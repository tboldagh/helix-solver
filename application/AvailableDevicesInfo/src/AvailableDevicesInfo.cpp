#include <sycl/sycl.hpp>
#include <iostream>

int main()
{
    std::vector<sycl::device> devices = sycl::device::get_devices();
    for (sycl::device device : devices)
    {
        std::cout << device.get_info<sycl::info::device::name>() << std::endl;
    }
}