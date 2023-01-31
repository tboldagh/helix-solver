#pragma once
#ifdef USE_SYCL
    using Queue=std::unique_ptr<sycl::queue>;
#else
    // specimen Queue class for case when we do not use SYCL
    class PromptCPUQueue {
        public:
        void wait() {}
    };
    using Queue=PromptCPUQueue;
#endif
