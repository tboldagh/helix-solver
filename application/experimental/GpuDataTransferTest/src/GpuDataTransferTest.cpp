#include <iostream>
#include <numeric>
#include <random>
#include <CL/sycl.hpp>
#include <cmath>
#include <stdexcept>

#define USE_SYCL // Must be defined before Debug/Debug.h include
#include "Debug/Debug.h"

void devInfo(const sycl::queue &q);

class ComputationHeavyKernel
{
    using FloatBufferReadAccessor = sycl::accessor<float, 1, sycl::access::mode::read, sycl::access::target::device>;
    using FloatBufferWriteAccessor = sycl::accessor<float, 1, sycl::access::mode::write, sycl::access::target::device>;

    constexpr static u_int64_t Iterations = 1e5;

public:
    ComputationHeavyKernel(FloatBufferReadAccessor input, FloatBufferWriteAccessor output)
        : input(input), output(output) {}

    void operator()(sycl::id<1> idx) const
    {
        int index = idx[0];
        float result = 0;
        for (u_int64_t i = 0; i < Iterations; ++i)
        {
            result += input[index] * std::sin(static_cast<float>(index * i));
        }
        output[index] = result;
    }

private:
    FloatBufferReadAccessor input;
    FloatBufferWriteAccessor output;
};

class TransferHeavyKernel
{
    using FloatBufferReadAccessor = sycl::accessor<float, 1, sycl::access::mode::read, sycl::access::target::device>;
    using FloatBufferWriteAccessor = sycl::accessor<float, 1, sycl::access::mode::write, sycl::access::target::device>;

    public:
    TransferHeavyKernel(FloatBufferReadAccessor input, FloatBufferWriteAccessor output)
        : input(input), output(output) {}

    void operator()(sycl::id<1> idx) const
    {
        output[idx] = input[idx];
    }

private:
    FloatBufferReadAccessor input;
    FloatBufferWriteAccessor output;
};

void runSequential(sycl::queue& queue, unsigned iterations, std::vector<float>& computationHeavyExecutionTimes, std::vector<float>& transferHeavyExecutionTimes)
{
    for (unsigned i = 0; i < iterations; ++i)
    {
        constexpr unsigned inputSize = 1000;
        std::vector<float> input(inputSize);
        std::vector<float> output(inputSize);
        for (int i = 0; i < inputSize; ++i)
        {
            input[i] = i;
        }
        sycl::buffer<float, 1> inputBuffer(input.data(), sycl::range<1>(input.size()));
        sycl::buffer<float, 1> outputBuffer(output.data(), sycl::range<1>(output.size()));

        constexpr unsigned largeInputSize = 1e8;
        std::vector<float> largeInput(largeInputSize);
        std::vector<float> largeOutput(largeInputSize);
        for (int i = 0; i < largeInputSize; ++i)
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

            handler.parallel_for(sycl::range<1>(inputSize), ComputationHeavyKernel(inputAccessor, outputAccessor));
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

            handler.parallel_for(sycl::range<1>(largeInputSize), TransferHeavyKernel(inputAccessor, outputAccessor));
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

void runParallel(sycl::queue& queue, unsigned iterations, std::vector<float>& executionTimes)
{
    for (unsigned i = 0; i < iterations; ++i)
    {
        constexpr unsigned inputSize = 1000;
        std::vector<float> input(inputSize);
        std::vector<float> output(inputSize);
        for (int i = 0; i < inputSize; ++i)
        {
            input[i] = i;
        }
        sycl::buffer<float, 1> inputBuffer(input.data(), sycl::range<1>(input.size()));
        sycl::buffer<float, 1> outputBuffer(output.data(), sycl::range<1>(output.size()));

        constexpr unsigned largeInputSize = 1e8;
        std::vector<float> largeInput(largeInputSize);
        std::vector<float> largeOutput(largeInputSize);
        for (int i = 0; i < largeInputSize; ++i)
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

            handler.parallel_for(sycl::range<1>(inputSize), ComputationHeavyKernel(inputAccessor, outputAccessor));
        });
        queue.submit([&](sycl::handler& handler) {
            sycl::accessor<float, 1, sycl::access::mode::read, sycl::access::target::device> inputAccessor(largeInputBuffer, handler, sycl::read_only);
            sycl::accessor<float, 1, sycl::access::mode::write, sycl::access::target::device> outputAccessor(largeOutputBuffer, handler, sycl::write_only);

            handler.parallel_for(sycl::range<1>(largeInputSize), TransferHeavyKernel(inputAccessor, outputAccessor));
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

int main()
{
    sycl::queue queue(sycl::gpu_selector_v);

    devInfo(queue);
    std::cout << std::endl;

    std::vector<float> computationHeavyExecutionTimes;
    std::vector<float> transferHeavyExecutionTimes;
    runSequential(queue, 10, computationHeavyExecutionTimes, transferHeavyExecutionTimes);

    std::cout << "Sequential execution times:" << std::endl;
    std::cout << "iteration\texecution time\tcomputation\ttransfer" << std::endl;
    for (unsigned i = 0; i < 10; ++i)
    {
        std::cout << i << "\t\t" << computationHeavyExecutionTimes[i] + transferHeavyExecutionTimes[i] << "\t\t" << computationHeavyExecutionTimes[i] << "\t\t" << transferHeavyExecutionTimes[i] << std::endl;
    }

    std::cout << std::endl;

    std::vector<float> parallelExecutionTimes;
    runParallel(queue, 10, parallelExecutionTimes);

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
