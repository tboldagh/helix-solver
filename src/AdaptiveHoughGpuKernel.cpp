#include "HelixSolver/AdaptiveHoughGpuKernel.h"


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
        constexpr uint16_t INITIAL_SECTION_WIDTH = ACC_WIDTH / ADAPTIVE_KERNEL_INITIAL_DIVISIONS;
        constexpr uint16_t INITIAL_SECTION_HEIGHT = ACC_HEIGHT / ADAPTIVE_KERNEL_INITIAL_DIVISIONS;

        const uint16_t xBegin = ACC_WIDTH * idx[0] / ADAPTIVE_KERNEL_INITIAL_DIVISIONS;
        const uint16_t yBegin = ACC_HEIGHT * idx[1] / ADAPTIVE_KERNEL_INITIAL_DIVISIONS;

        uint32_t stubListSizes[MAX_STUB_LISTS_NUM];
        for(uint32_t i = 0; i < MAX_STUB_LISTS_NUM; ++i) stubListSizes[i] = 0;

        uint32_t stubLists[MAX_STUB_LIST_ELEMENTS_NUM];
        const uint32_t stubsNum = rs.size();
        stubListSizes[0] = stubsNum;
        for(uint32_t i = 0; i < stubsNum; ++i) stubLists[i] = i;

        constexpr uint8_t MAX_SECTIONS_HEIGHT = (MAX_DIVISION_LEVEL - ADAPTIVE_KERNEL_INITIAL_DIVISION_LEVEL) * 4;
        AccumulatorSection sections[MAX_SECTIONS_HEIGHT];
        uint8_t sectionsHeight = 1;
        sections[0] = AccumulatorSection(INITIAL_SECTION_WIDTH, INITIAL_SECTION_HEIGHT, xBegin, yBegin);
        while (sectionsHeight) fillAccumulatorSection(sections, sectionsHeight, stubLists, stubListSizes);
    }

    void AdaptiveHoughGpuKernel::fillAccumulatorSection(AccumulatorSection* sections, uint8_t& sectionsHeight, uint32_t* stubLists, uint32_t* stubListSizes) const
    {
        sectionsHeight--;
        const AccumulatorSection section = sections[sectionsHeight];

        const uint8_t divisionLevel = MAX_DIVISION_LEVEL - ADAPTIVE_KERNEL_INITIAL_DIVISION_LEVEL - static_cast<uint8_t>(round(log2(section.width > section.height ? section.width : section.height)));
        fillHits(stubLists, stubListSizes, divisionLevel, section);
        if (stubListSizes[divisionLevel + 1] - stubListSizes[divisionLevel] < THRESHOLD) return;
        
        if (section.width > 1)
        {
            const uint16_t newWidth = section.width / 2;
            if(section.height > 1)
            {
                const uint16_t newHeight = section.height / 2;
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
                const uint16_t newHeight = section.height / 2;
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
        const float x = linspaceElement(Q_OVER_P_BEGIN, Q_OVER_P_END, ACC_WIDTH, section.xBegin);
        const float y = linspaceElement(PHI_BEGIN, PHI_END, ACC_HEIGHT, section.yBegin);
        const float xLeft = x - ACC_CELL_WIDTH * 0.5f;
        const float xRight = x + ACC_CELL_WIDTH * (section.width - 0.5f);
        const float yBottom = y - ACC_CELL_HEIGHT * 0.5f;
        const float yTop = y + ACC_CELL_HEIGHT * (section.height - 0.5f);

        stubListSizes[divisionLevel + 1] = stubListSizes[divisionLevel];
        const uint32_t startStubIndexInList = divisionLevel ? stubListSizes[divisionLevel - 1] : 0;
        for(uint32_t stubIndexInList = startStubIndexInList; stubIndexInList < stubListSizes[divisionLevel]; ++stubIndexInList)
        {
            const uint32_t stubIndex = stubLists[stubIndexInList];
            const float yLeft = -rs[stubIndex] * xLeft + phis[stubIndex];
            const float yRight = -rs[stubIndex] * xRight + phis[stubIndex];

            if (yLeft >= yBottom && yRight <= yTop)
            {
                stubLists[stubListSizes[divisionLevel + 1]] = stubIndex;
                stubListSizes[divisionLevel + 1]++;
            }            
        }
    }

    void AdaptiveHoughGpuKernel::addSolution(uint32_t qOverPtIndex, uint32_t phiIndex) const
    {
        const float qOverPt = linspaceElement(Q_OVER_P_BEGIN, Q_OVER_P_END, ACC_WIDTH, qOverPtIndex);
        const float phi_0 = linspaceElement(PHI_BEGIN, Q_OVER_P_END, ACC_HEIGHT, phiIndex);
        
        const uint32_t index = phiIndex * ACC_WIDTH + qOverPtIndex;
        solutions[index].isValid = true;
        solutions[index].r = 1000.0f / (qOverPt * B);
        solutions[index].phi = phi_0 + float(M_PI_2); // float czy double
    }

    AdaptiveHoughGpuKernel::AccumulatorSection::AccumulatorSection(uint16_t width, uint16_t height, uint16_t xBegin, uint16_t yBegin)
    : width(width)
    , height(height)
    , xBegin(xBegin)
    , yBegin(yBegin) {}
} // namespace HelixSolver
