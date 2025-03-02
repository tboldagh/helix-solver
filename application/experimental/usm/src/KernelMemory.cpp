#include "EventUsm/KernelMemory.h"

#include <sycl/sycl.hpp>


KernelMemory::KernelMemory(sycl::queue& queue)
: queue_(queue) {}
