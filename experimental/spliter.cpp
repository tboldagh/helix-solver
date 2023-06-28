#include <iostream>
#include <numeric>

#include <sycl/sycl.hpp>

void devInfo(const sycl::queue &q) {
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
            << (dev.get_info<sycl::info::device::local_mem_type>() == sycl::info::local_mem_type::local ? "local":"")
            << (dev.get_info<sycl::info::device::local_mem_type>() == sycl::info::local_mem_type::global ? "global":"")
            << std::endl;
}

void fill(std::vector<float> &x, float start=0) {
  std::iota(x.begin(), x.end(), start);
  std::cout << "FILLES dummy data of size " << x.size() << std::endl;
}

constexpr size_t inDataSize = 50e3;
// constexpr short phiSplit = 16;
// constexpr short etaSplit = 51;
constexpr short phiSplit = 2;
constexpr short etaSplit = 7;
constexpr short totSplit = etaSplit*phiSplit;


int belongs_to(float value, int phi_group_id) {
  constexpr int range = inDataSize/phiSplit;
  return (phi_group_id*range < value && value < (phi_group_id+1)*range) ? 1 : 0;
}

// int index(int a) {
//   return a * etaSplit + b;
// }

int main() {
  std::cout << "---\n";

  std::vector<float> x(inDataSize, 0);
  std::vector<float> y(inDataSize, 0);
  std::vector<float> z(inDataSize, 0);
  std::vector<int> count(totSplit, 0);
  fill(x);
  sycl::buffer<float, 1> xBuffer(x.data(), x.size());
  sycl::buffer<float, 1> yBuffer(y.data(), y.size());
  sycl::buffer<float, 1> zBuffer(z.data(), z.size());
  sycl::buffer<int, 1> countBuffer(count.data(), count.size());
  sycl::queue q;
  devInfo(q);
  auto t1 = std::chrono::steady_clock::now();   // Start timing

  q.submit([&](sycl::handler &cgh) {
    // // Getting write only access to the buffer on a device
    auto x = xBuffer.get_access<sycl::access::mode::read>(cgh);
    auto y = yBuffer.get_access<sycl::access::mode::read>(cgh);
    auto z = zBuffer.get_access<sycl::access::mode::read>(cgh);
    auto countA = countBuffer.get_access<sycl::access::mode::write>(cgh);
    constexpr int maxlocal = 1024;
    auto fragment = sycl::local_accessor<float, 1>(sycl::range{maxlocal}, cgh); 

    // sycl::stream out(1024*16, 1024, cgh);

    // // Executing kernel

    cgh.parallel_for_work_group(sycl::range{totSplit}, sycl::range{1}, [=](sycl::group<1> region){
      int _lcount=0;
      using local_atomic_int_ref = sycl::atomic_ref<int, sycl::memory_order::relaxed, sycl::memory_scope::work_group, sycl::access::address_space::local_space>;
      // out << "ID " << " " << region[0] << "\n";
      local_atomic_int_ref lcount(_lcount);
      float a = x[0];
      float b = y[0];
      float c = z[0];

      // region.parallel_for_work_item( sycl::range<1>(1024), [&](sycl::h_item<1> item) {
      //   // out << "WI ID region " << " " << region[0] << " localID " << item.get_logical_local_id()[0] << " " << item.get_logical_local_range()[0] << "\n";
      //   const int shift = item.get_logical_local_id()[0];
      //   const int step = item.get_logical_local_range()[0];
      //   for ( int i = shift; i < x.size(); i += step) {
      //     //  lcount += belongs_to(x[i]+y[i]+z[i], region[0]);
      //     lcount += item.get_logical_local_id()[0];
      //   }
      // });
      countA[region[0]] =  lcount;
      // out << "count in ID " << " " << region[0] << " " << static_cast<int>(lcount) << "\n";
    }); // EOF parallel_for_work_group
  }).wait();
  auto t2 = std::chrono::steady_clock::now(); 
  std::cout << "TIME ms " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << std::endl;

  auto countBack = countBuffer.get_access<sycl::access::mode::read>();
  std::cout << "counts size"  << countBack.size() << " " << count.size() << std::endl;
  for ( int i = 0; i < count.size(); ++i) {
    std::cout << " " << count[i];
  }
  std::cout << "\n---\n";


}