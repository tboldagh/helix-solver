#include <iostream>
#include <numeric>
#include <random>

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
static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_real_distribution<> zDistribution(-3000, 3000);
static std::uniform_real_distribution<> phiDistribution(-M_PI, M_PI);
static std::uniform_real_distribution<> rDistribution(200, 1000);

void fill(std::vector<float> &x, std::vector<float> &y, std::vector<float> &z ) {
  for ( int i = 0; i < x.size(); ++i ) {
    z[i]=zDistribution(gen);
    const double r = rDistribution(gen);
    const double phi = phiDistribution(gen);
    
  }
  std::uniform_int_distribution<> distrib(1, 6);
  std::cout << "FILLES dummy data of size " << x.size() << std::endl;
}

constexpr size_t inDataSize = 50e3;
// constexpr short phiSplit = 16;
// constexpr short etaSplit = 51;
constexpr short phiSplit = 2;
constexpr short etaSplit = 7;
constexpr short totSplit = etaSplit*phiSplit;


// int belongs_to(float value, int phi_group_id) {
//   constexpr int range = inDataSize/phiSplit;
//   return (phi_group_id*range < value && value < (phi_group_id+1)*range) ? 1 : 0;
// }

// int index(int a) {
//   return a * etaSplit + b;
// }

int main() {
  std::cout << "---\n";

  std::vector<float> x(inDataSize, 0);
  std::vector<float> y(inDataSize, 0);
  std::vector<float> z(inDataSize, 0);
  std::vector<float> r(inDataSize, 0);
  std::vector<float> phi(inDataSize, 0);

  std::vector<int> count(totSplit, 0);
  fill(x,y,z);
  sycl::buffer<float, 1> xBuffer(x.data(), x.size());
  sycl::buffer<float, 1> yBuffer(y.data(), y.size());
  sycl::buffer<float, 1> zBuffer(z.data(), z.size());
  sycl::buffer<float, 1> rBuffer(r.data(), r.size());
  sycl::buffer<float, 1> phiBuffer(phi.data(), phi.size());

  sycl::buffer<int, 1> countBuffer(count.data(), count.size());
  sycl::queue q;
  devInfo(q);
  auto t1 = std::chrono::steady_clock::now();   // Start timing

  q.submit([&](sycl::handler &cgh) {
    // // Getting write only access to the buffer on a device
    auto x = xBuffer.get_access<sycl::access::mode::read>(cgh);
    auto y = yBuffer.get_access<sycl::access::mode::read>(cgh);
    auto r = rBuffer.get_access<sycl::access::mode::write>(cgh);
    auto phi = phiBuffer.get_access<sycl::access::mode::write>(cgh);
    const int size = x.size();
    cgh.parallel_for(sycl::range{46}, [=](sycl::item<1> it){
      for ( int i = it.get_id()[0]; i < size; i += it.get_range()[0]) {
          r[i] = sycl::hypot(x[i], y[i]);
          phi[i] = sycl::atan2(y[i], x[i]);
      }
    });
  });


  q.submit([&](sycl::handler &cgh) {
    // // Getting write only access to the buffer on a device
    auto r = rBuffer.get_access<sycl::access::mode::read>(cgh);
    auto phi = phiBuffer.get_access<sycl::access::mode::read>(cgh);
   
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
      region.parallel_for_work_item( sycl::range<1>(256), [&](sycl::h_item<1> item) {
        // out << "WI ID region " << " " << region[0] << " localID " << item.get_logical_local_id()[0] << " " << item.get_logical_local_range()[0] << "\n";
        const int shift = item.get_logical_local_id()[0];
        const int step = item.get_logical_local_range()[0];
        for ( int i = shift; i < r.size(); i += step) {
          //  lcount += belongs_to(x[i]+y[i]+z[i], region[0]);
          lcount += item.get_logical_local_id()[0];
        }
      });
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