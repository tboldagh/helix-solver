#include "HelixSolver/AdaptiveHoughGpuKernel.h"
#include "HelixSolver/AccumulatorHelper.h"


namespace HelixSolver
{
    AdaptiveHoughGpuKernel::AdaptiveHoughGpuKernel(sycl::accessor<float, 1, sycl::access::mode::read, sycl::access::target::device> rs,
            sycl::accessor<float, 1, sycl::access::mode::read, sycl::access::target::device> phis,
            sycl::accessor<SolutionCircle, 1, sycl::access::mode::write, sycl::access::target::device> solutions)
    : rs(rs)
    , phis(phis)
    , solutions(solutions) {}

    void AdaptiveHoughGpuKernel::operator()(sycl::id<2> idx) const
    {
        constexpr uint8_t MAX_STUB_LISTS_NUM = MAX_DIVISION_LEVEL - ADAPTIVE_KERNEL_INITIAL_DIVISION_LEVEL + 2;
        constexpr uint32_t MAX_STUB_LIST_ELEMENTS_NUM = MAX_STUB_NUM * MAX_STUB_LISTS_NUM;

        uint16_t xBegin = ACC_WIDTH * idx[0] / ADAPTIVE_KERNEL_INITIAL_DIVISIONS;
        uint16_t yBegin = ACC_HEIGHT * idx[1] / ADAPTIVE_KERNEL_INITIAL_DIVISIONS;

        uint32_t stubListSizes[MAX_STUB_LISTS_NUM];
        for(uint32_t i = 0; i < MAX_STUB_LISTS_NUM; ++i) stubListSizes[i] = 0;

        uint32_t stubLists[MAX_STUB_LIST_ELEMENTS_NUM];
        uint32_t stubsNum = rs.size();
        stubListSizes[0] = stubsNum;
        for(uint32_t i = 0; i < stubsNum; ++i) stubLists[i] = i;

        constexpr uint8_t MAX_SECTIONS_HEIGHT = (MAX_DIVISION_LEVEL - ADAPTIVE_KERNEL_INITIAL_DIVISION_LEVEL) * 4;
        AccumulatorSection sections[MAX_SECTIONS_HEIGHT];
        uint8_t sectionsHeight = 1;

        sections[0] = AccumulatorSection(ACC_WIDTH / ADAPTIVE_KERNEL_INITIAL_DIVISIONS, ACC_HEIGHT / ADAPTIVE_KERNEL_INITIAL_DIVISIONS, xBegin, yBegin);
        while (sectionsHeight) fillAccumulatorSection(sections, sectionsHeight, stubLists, stubListSizes);
    }


    void AdaptiveHoughGpuKernel::fillAccumulatorSection(AccumulatorSection* sections, uint8_t& sectionsHeight, uint32_t* stubLists, uint32_t* stubListSizes) const
    {
        sectionsHeight--;
        AccumulatorSection section = sections[sectionsHeight];

        uint8_t divisionLevel = MAX_DIVISION_LEVEL - ADAPTIVE_KERNEL_INITIAL_DIVISION_LEVEL - static_cast<uint8_t>(round(log2(section.width > section.height ? section.width : section.height)));
        fillHits(stubLists, stubListSizes, divisionLevel, section);
        if (stubListSizes[divisionLevel + 1] - stubListSizes[divisionLevel] < THRESHOLD) return;
        
        if (section.width > 1)
        {
            uint32_t newWidth = section.width / 2;
            if(section.height > 1)
            {
                uint32_t newHeight = section.height / 2;
                sections[sectionsHeight] = AccumulatorSection(newWidth, newHeight, section.xBegin, section.yBegin);
                sections[sectionsHeight + 1] = AccumulatorSection(newWidth, newHeight, section.xBegin, section.yBegin + newHeight);
                sections[sectionsHeight + 2] = AccumulatorSection(newWidth, newHeight, section.xBegin + newWidth, section.yBegin);
                sections[sectionsHeight + 3] = AccumulatorSection(newWidth, newHeight, section.xBegin + newWidth, section.yBegin + newHeight);
                sectionsHeight += 4;
            }
            else
            {
                sections[sectionsHeight] = AccumulatorSection(newWidth, section.height, section.xBegin, section.yBegin);
                sections[sectionsHeight + 1] = AccumulatorSection(newWidth, section.height, section.xBegin + newWidth, section.yBegin);
                sectionsHeight += 2;
            }
        }
        else
        {
            if(section.height > 1)
            {
                uint32_t newHeight = section.height / 2;
                sections[sectionsHeight] = AccumulatorSection(section.width, newHeight, section.xBegin, section.yBegin);
                sections[sectionsHeight + 1] = AccumulatorSection(section.width, newHeight, section.xBegin, section.yBegin + newHeight);
                sectionsHeight += 2;   
            }
            else
            {
                addSolution(section.xBegin, section.yBegin);
            }
        }
    }

    void AdaptiveHoughGpuKernel::fillHits(uint32_t* stubLists, uint32_t* stubListSizes, uint8_t divisionLevel, const AccumulatorSection& section) const
    {
        float x = linspaceElement(Q_OVER_P_BEGIN, Q_OVER_P_END, ACC_WIDTH, section.xBegin);
        float y = linspaceElement(PHI_BEGIN, PHI_END, ACC_HEIGHT, section.yBegin);
        float xLeft = x - ACC_CELL_WIDTH * 0.5;
        float xRight = x + ACC_CELL_WIDTH * (section.width - 0.5);
        float yBottom = y - ACC_CELL_HEIGHT * 0.5;
        float yTop = y + ACC_CELL_HEIGHT * (section.height - 0.5);

        stubListSizes[divisionLevel + 1] = stubListSizes[divisionLevel];
        uint32_t startStubIndex = divisionLevel ? stubListSizes[divisionLevel - 1] : 0;
        for(uint32_t stubIndex = startStubIndex; stubIndex < stubListSizes[divisionLevel]; ++stubIndex)
        {
            float yLeft = -rs[stubLists[stubIndex]] * xLeft + phis[stubLists[stubIndex]];
            float yRight = -rs[stubLists[stubIndex]] * xRight + phis[stubLists[stubIndex]];

            if (yLeft >= yBottom && yRight <= yTop)
            {
                stubLists[stubListSizes[divisionLevel + 1]] = stubLists[stubIndex];
                stubListSizes[divisionLevel + 1]++;
            }            
        }
    }

    void AdaptiveHoughGpuKernel::addSolution(uint32_t qOverPtIndex, uint32_t phiIndex) const
    {
        float qOverPt = linspaceElement(Q_OVER_P_BEGIN, Q_OVER_P_END, ACC_WIDTH, qOverPtIndex);
        float phi_0 = linspaceElement(PHI_BEGIN, Q_OVER_P_END, ACC_HEIGHT, phiIndex);
        
        uint32_t index = phiIndex * ACC_WIDTH + qOverPtIndex;
        solutions[index].isValid = true;
        solutions[index].r = ((1 / qOverPt) / B) * 1000;
        solutions[index].phi = phi_0 + M_PI_2;
    }

    AdaptiveHoughGpuKernel::AccumulatorSection::AccumulatorSection(uint16_t width, uint16_t height, uint16_t xBegin, uint16_t yBegin)
    : width(width)
    , height(height)
    , xBegin(xBegin)
    , yBegin(yBegin) {}
} // namespace HelixSolver
