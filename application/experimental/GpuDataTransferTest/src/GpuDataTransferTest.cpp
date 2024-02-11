#include <CL/sycl.hpp>
#include <chrono>
#include <iostream>
#include <vector>

#define USE_SYCL // Must be defined before Debug/Debug.h include

#include "GpuDataTransferTest/ComputationHeavyKernel.h"
#include "GpuDataTransferTest/TransferHeavyKernel.h"


void devInfo(const sycl::queue &q);

template <u_int64_t InputSize, u_int64_t ComputationHeavyIterations, u_int64_t LargeInputSize>
void runSequential(sycl::queue& queue, unsigned iterations, std::vector<float>& computationHeavyExecutionTimes, std::vector<float>& transferHeavyExecutionTimes);

template <u_int64_t InputSize, u_int64_t ComputationHeavyIterations, u_int64_t LargeInputSize>
void runParallel(sycl::queue& queue, unsigned iterations, std::vector<float>& executionTimes);

int main()
{
    sycl::queue queue(sycl::gpu_selector_v);

    devInfo(queue);
    std::cout << std::endl;

    constexpr u_int64_t inputSize = 1e3;
    constexpr u_int64_t computationHeavyIterations = 1e5;
    constexpr u_int64_t largeInputSize = 1e8;
    unsigned iterations = 10;

    std::vector<float> computationHeavyExecutionTimes;
    std::vector<float> transferHeavyExecutionTimes;
    runSequential<inputSize, computationHeavyIterations, largeInputSize>(queue, iterations, computationHeavyExecutionTimes, transferHeavyExecutionTimes);

    std::cout << "Sequential execution times:" << std::endl;
    std::cout << "iteration\texecution time\tcomputation\ttransfer" << std::endl;
    for (unsigned i = 0; i < 10; ++i)
    {
        std::cout << i << "\t\t" << computationHeavyExecutionTimes[i] + transferHeavyExecutionTimes[i] << "\t\t" << computationHeavyExecutionTimes[i] << "\t\t" << transferHeavyExecutionTimes[i] << std::endl;
    }

    std::cout << std::endl;

    std::vector<float> parallelExecutionTimes;
    runParallel<inputSize, computationHeavyIterations, largeInputSize>(queue, iterations, parallelExecutionTimes);

    std::cout << "Parallel execution times:" << std::endl;
    std::cout << "iteration\texecution time" << std::endl;
    for (unsigned i = 0; i < 10; ++i)
    {
        std::cout << i << "\t\t" << parallelExecutionTimes[i] << std::endl;
    }
}

void devInfo(const sycl::queue &q)
{
    auto dev = q.get_device();
    std::cout << "Device " << (dev.is_cpu() ? "cpu" : "")
              << (dev.is_gpu() ? "gpu" : "") << std::endl;
    std::cout << "Vendor " << dev.get_info<sycl::info::device::vendor>()
              << std::endl;

    std::cout << "Max Comp units "
              << dev.get_info<sycl::info::device::max_compute_units>()
              << std::endl;
    std::cout << "Max WI DI "
              << dev.get_info<sycl::info::device::max_work_item_dimensions>()
              << std::endl;
    std::cout << "Max WI SZ "
              << dev.get_info<sycl::info::device::max_work_group_size>()
              << std::endl;

    std::cout << "GLOB Mem kB "
              << dev.get_info<sycl::info::device::global_mem_size>() / 1024
              << std::endl;
    std::cout << "GLOB Mem cache kB "
              << dev.get_info<sycl::info::device::global_mem_cache_size>() / 1024
              << std::endl;
    // std::cout << "GLOB Mem cache line kB " <<
    // dev.get_info<sycl::info::device::global_mem_cacheline_size>()/1024 <<
    // std::endl; std::cout << "GLOB Mem cache type " <<
    // dev.get_info<sycl::info::device::global_mem_cache_type>() << std::endl;
    std::cout << "LOC Mem kB "
              << dev.get_info<sycl::info::device::local_mem_size>() / 1024
              << std::endl;
    std::cout << "LOC Mem type "
              << (dev.get_info<sycl::info::device::local_mem_type>() == sycl::info::local_mem_type::local ? "local" : "")
              << (dev.get_info<sycl::info::device::local_mem_type>() == sycl::info::local_mem_type::global ? "global" : "")
              << std::endl;
}

template <u_int64_t InputSize, u_int64_t ComputationHeavyIterations, u_int64_t LargeInputSize>
void runSequential(sycl::queue& queue, unsigned iterations, std::vector<float>& computationHeavyExecutionTimes, std::vector<float>& transferHeavyExecutionTimes)
{
    for (unsigned i = 0; i < iterations; ++i)
    {
        std::vector<float> input(InputSize);
        std::vector<float> output(InputSize);
        for (int i = 0; i < InputSize; ++i)
        {
            input[i] = i;
        }
        sycl::buffer<float, 1> inputBuffer(input.data(), sycl::range<1>(input.size()));
        sycl::buffer<float, 1> outputBuffer(output.data(), sycl::range<1>(output.size()));

        std::vector<float> largeInput(LargeInputSize);
        std::vector<float> largeOutput(LargeInputSize);
        for (int i = 0; i < LargeInputSize; ++i)
        {
            largeInput[i] = i;
        }
        sycl::buffer<float, 1> largeInputBuffer(largeInput.data(), sycl::range<1>(largeInput.size()));
        sycl::buffer<float, 1> largeOutputBuffer(largeOutput.data(), sycl::range<1>(largeOutput.size()));

        std::chrono::steady_clock::time_point computationHeavyTaskStart;
        std::chrono::steady_clock::time_point computationHeavyTaskEnd;
        std::chrono::steady_clock::time_point transferHeavyTaskStart;
        std::chrono::steady_clock::time_point transferHeavyTaskEnd;

        computationHeavyTaskStart = std::chrono::steady_clock::now();
        queue.submit([&](sycl::handler& handler) {
            sycl::accessor<float, 1, sycl::access::mode::read, sycl::access::target::device> inputAccessor(inputBuffer, handler, sycl::read_only);
            sycl::accessor<float, 1, sycl::access::mode::write, sycl::access::target::device> outputAccessor(outputBuffer, handler, sycl::write_only);

            using Kernel = ComputationHeavyKernel<ComputationHeavyIterations>;
            handler.parallel_for(sycl::range<1>(InputSize), Kernel(inputAccessor, outputAccessor));
        });
        {
            auto hostOutputAccessor = outputBuffer.get_host_access();
            output = std::vector<float>(hostOutputAccessor.begin(), hostOutputAccessor.end());
        }
        queue.wait();
        computationHeavyTaskEnd = std::chrono::steady_clock::now();

        transferHeavyTaskStart = std::chrono::steady_clock::now();
        queue.submit([&](sycl::handler& handler) {
            sycl::accessor<float, 1, sycl::access::mode::read, sycl::access::target::device> inputAccessor(largeInputBuffer, handler, sycl::read_only);
            sycl::accessor<float, 1, sycl::access::mode::write, sycl::access::target::device> outputAccessor(largeOutputBuffer, handler, sycl::write_only);

            handler.parallel_for(sycl::range<1>(LargeInputSize), TransferHeavyKernel(inputAccessor, outputAccessor));
        });
        {
            auto hostOutputAccessor = largeOutputBuffer.get_host_access();
            largeOutput = std::vector<float>(hostOutputAccessor.begin(), hostOutputAccessor.end());
        }
        queue.wait();
        transferHeavyTaskEnd = std::chrono::steady_clock::now();

        computationHeavyExecutionTimes.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(computationHeavyTaskEnd - computationHeavyTaskStart).count());
        transferHeavyExecutionTimes.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(transferHeavyTaskEnd - transferHeavyTaskStart).count());
    }
}

template <u_int64_t InputSize, u_int64_t ComputationHeavyIterations, u_int64_t LargeInputSize>
void runParallel(sycl::queue& queue, unsigned iterations, std::vector<float>& executionTimes)
{
    for (unsigned i = 0; i < iterations; ++i)
    {
        std::vector<float> input(InputSize);
        std::vector<float> output(InputSize);
        for (int i = 0; i < InputSize; ++i)
        {
            input[i] = i;
        }
        sycl::buffer<float, 1> inputBuffer(input.data(), sycl::range<1>(input.size()));
        sycl::buffer<float, 1> outputBuffer(output.data(), sycl::range<1>(output.size()));

        std::vector<float> largeInput(LargeInputSize);
        std::vector<float> largeOutput(LargeInputSize);
        for (int i = 0; i < LargeInputSize; ++i)
        {
            largeInput[i] = i;
        }
        sycl::buffer<float, 1> largeInputBuffer(largeInput.data(), sycl::range<1>(largeInput.size()));
        sycl::buffer<float, 1> largeOutputBuffer(largeOutput.data(), sycl::range<1>(largeOutput.size()));

        std::chrono::steady_clock::time_point taskStart;
        std::chrono::steady_clock::time_point taskEnd;

        taskStart = std::chrono::steady_clock::now();
        queue.submit([&](sycl::handler& handler) {
            sycl::accessor<float, 1, sycl::access::mode::read, sycl::access::target::device> inputAccessor(inputBuffer, handler, sycl::read_only);
            sycl::accessor<float, 1, sycl::access::mode::write, sycl::access::target::device> outputAccessor(outputBuffer, handler, sycl::write_only);

            using Kernel = ComputationHeavyKernel<static_cast<u_int64_t>(ComputationHeavyIterations)>;
            handler.parallel_for(sycl::range<1>(InputSize), Kernel(inputAccessor, outputAccessor));
        });
        queue.submit([&](sycl::handler& handler) {
            sycl::accessor<float, 1, sycl::access::mode::read, sycl::access::target::device> inputAccessor(largeInputBuffer, handler, sycl::read_only);
            sycl::accessor<float, 1, sycl::access::mode::write, sycl::access::target::device> outputAccessor(largeOutputBuffer, handler, sycl::write_only);

            handler.parallel_for(sycl::range<1>(LargeInputSize), TransferHeavyKernel(inputAccessor, outputAccessor));
        });

        {
            auto hostOutputAccessor = outputBuffer.get_host_access();
            output = std::vector<float>(hostOutputAccessor.begin(), hostOutputAccessor.end());
        }
        {
            auto hostOutputAccessor = largeOutputBuffer.get_host_access();
            largeOutput = std::vector<float>(hostOutputAccessor.begin(), hostOutputAccessor.end());
        }

        queue.wait();
        taskEnd = std::chrono::steady_clock::now();

        executionTimes.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(taskEnd - taskStart).count());
    }
}
