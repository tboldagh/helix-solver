#include <iostream>
#include <numeric>
#include <random>
#include <CL/sycl.hpp>
#include <cmath>
#include <stdexcept>

#define USE_SYCL    // Must be defined before Debug/Debug.h include
#include "Debug/Debug.h"



// this functions assert whether point belongs to a z-phi-eta slice
// the definition of the region is fiven by:
// phi center,  delta phi, 
// z center, delta z - at the beam line (typically unchanged and covering all +- 20cm)
// eta center, delta eta

struct Reg { 
    float center;
    float width;
};

/**
 * @brief description of the wedge
 */
struct Wedge {
    /**
    * this is constructor in a form suitable fro SYCL
    */
    void setup( Reg Phi, Reg z, Reg eta);

    Reg m_phi;
    // definition of lines in r - z plane
    float m_aleft, m_bleft;
    float m_aright, m_bright;
    /**
     * @brief true if point is in the Wedge
     * 
     * @param x,y,z - cartesian coordinates
     */
    bool in_wedge_x_y_z( float x, float y, float z ) const;
    /**
     * @brief true if point is in the Wedge
     * 
     * @param p, phi, z  - polar coordinates
     */
    SYCL_EXTERNAL bool in_wedge_r_phi_z( float r, float phi, float z ) const;

    /**
     * @brief returns true if the the point given as an argument is below top line
     */
    bool below(float r, float z) const;
    /**
     * @brief returns true if the the point given as an argument is above bottom line
     */
    bool above(float r, float z) const;


};
/**
 * uniform split
 */
Reg uniform_split(float min, float max, short index, short splits );


float phi_wrap( float phi )
{
    return std::fmod(phi + M_PI, 2.0 * M_PI) + (-1.0 + 2.0 * static_cast<float>(phi + M_PI < 0.0)) * M_PI;
}

float phi_dist ( float phi1, float phi2 ) {
  float dphi = phi1 - phi2; // ? Is phi_dist supposed to be distance? If so, then why is not absolute?
  return phi_wrap(dphi);
}

void Wedge::setup(Reg p, Reg z, Reg eta) 
{
    ASSURE_THAT( p.width> 0.1, "Wedge is very narrow in phi < 0.1, not good idea")
    m_phi = p;
    ASSURE_THAT( eta.width > 0.1, "Wedge is very narrow in eta < 0.1, not good idea (also do not support negative widths)")
    ASSURE_THAT( std::fabs( eta.center + eta.width)> 0.01, "Wedge does not support eta + eta width == 0")
    ASSURE_THAT( std::fabs( eta.center - eta.width)> 0.01, "Wedge does not support eta - eta width == 0")
    m_aleft = std::tan( 2.0 * std::atan( sycl::exp( - (eta.center-eta.width))));
    m_aright = std::tan( 2.0 * std::atan( sycl::exp( - (eta.center+eta.width))));
    m_bleft = - m_aleft/(z.center - z.width);
    m_bright = - m_aright/(z.center + z.width);
}

bool Wedge::in_wedge_x_y_z( float x, float y, float z ) const
{
    return in_wedge_r_phi_z(std::hypot(x,y), std::atan2(y, x), z);
}

bool Wedge::in_wedge_r_phi_z( float r, float phi, float z ) const
{
    ASSURE_THAT( std::fabs(r) > 0.1, "Strange r, it needs to be larger than 0.1 (meaning larger than 0)");
    if ( std::fabs(phi_dist(phi, m_phi.center)) > m_phi.width) return false;

    // 3 cases   
    if ( m_aleft > 0 && m_aright > 0  ) { //both left & right are  right tilted like this / /
        // std::cout << " here r" << r << " z " << z << " " << m_aleft * z + m_bleft << " " <<  m_aright * z + m_bright << "\n";
        return m_aleft * z + m_bleft > r && r > m_aright * z + m_bright;
    } else if ( m_aleft < 0 && m_aright > 0 ) { // this lind of wedge \ /
        return m_aleft * z + m_bleft < r && r > m_aright * z + m_bright;
    } else { // left tilt situation
        return m_aleft * z + m_bleft < r && r < m_aright * z + m_bright;
    }
    ASSURE_THAT( true, "Strange position of the point, does not fit any wedge definition");
    return false;
}


 Reg uniform_split(float min, float max, short index, short splits )
 {
   float width = (max-min)/splits;
   return {min + width*index + 0.5f*width, 0.5f*width};
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
            << (dev.get_info<sycl::info::device::local_mem_type>() == sycl::info::local_mem_type::local ? "local":"")
            << (dev.get_info<sycl::info::device::local_mem_type>() == sycl::info::local_mem_type::global ? "global":"")
            << std::endl;
}


static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_real_distribution<> zDistribution(-3000, 3000);
static std::uniform_real_distribution<> phiDistribution(-M_PI, M_PI);
static std::uniform_real_distribution<> rDistribution(200, 1000);

void fill(std::vector<float> &x, std::vector<float> &y, std::vector<float> &z )
{
  for ( int i = 0; i < x.size(); ++i ) {
    z[i]=zDistribution(gen);
    const double r = rDistribution(gen);
    const double phi = phiDistribution(gen);
    x[i] = r * std::cos(phi);
    y[i] = r * std::sin(phi);
  }
  std::uniform_int_distribution<> distrib(1, 6);
  std::cout << "FILLES dummy data of size " << x.size() << std::endl;
}

constexpr size_t inDataSize = 50e3;
constexpr short phiSplit = 16;
constexpr short etaSplit = 51;
// constexpr short phiSplit = 2;
// constexpr short etaSplit = 7;
constexpr short totSplit = etaSplit*phiSplit;

int main()
{
  std::cout << "---\n";

  std::vector<float> x(inDataSize, 0);
  std::vector<float> y(inDataSize, 0);
  std::vector<float> z(inDataSize, 0);
  std::vector<float> rinv(inDataSize, 0);
  std::vector<float> phi(inDataSize, 0);

  std::vector<int> count(totSplit, 0);
  fill(x,y,z);
  sycl::buffer<float, 1> xBuffer(x.data(), x.size());
  sycl::buffer<float, 1> yBuffer(y.data(), y.size());
  sycl::buffer<float, 1> zBuffer(z.data(), z.size());
  sycl::buffer<float, 1> rinvBuffer(rinv.data(), rinv.size());
  sycl::buffer<float, 1> phiBuffer(phi.data(), phi.size());

  sycl::buffer<int, 1> countBuffer(count.data(), count.size());
//   sycl::queue q;
  sycl::queue q(sycl::gpu_selector_v);
  devInfo(q);
  auto t1 = std::chrono::steady_clock::now();   // Start timing

  q.submit([&](sycl::handler &handler)
  {
    // Getting write only access to the buffer on a device
    auto x = xBuffer.get_access<sycl::access::mode::read>(handler);
    auto y = yBuffer.get_access<sycl::access::mode::read>(handler);
    auto rinv = rinvBuffer.get_access<sycl::access::mode::write>(handler);
    auto phi = phiBuffer.get_access<sycl::access::mode::write>(handler);
    const int size = x.size();
    // sycl::stream out(1024*48, 1024, handler);

    handler.parallel_for(sycl::range{46}, [=](sycl::item<1> it)
    {
      for ( int i = it.get_id()[0]; i < size; i += it.get_range()[0])
      {
          rinv[i] = sycl::rsqrt(x[i]*x[i] + y[i]*y[i]);
          phi[i] = sycl::atan2(y[i], x[i]);
          // out << "r: " << 1.0f/rinv[i] << " phi: " << phi[i] << "\n";
      }
    });
  });


  q.submit([&](sycl::handler &handler)
  {
    auto rinv = rinvBuffer.get_access<sycl::access::mode::read>(handler);
    auto phi = phiBuffer.get_access<sycl::access::mode::read>(handler);
    auto z = zBuffer.get_access<sycl::access::mode::read>(handler);
   
    // auto countA = countBuffer.get_access<sycl::access::mode::write>(handler);
    constexpr int maxlocal = 256;
    auto rinvFragment = sycl::local_accessor<float, 1>(sycl::range{maxlocal}, handler); 
    auto phiFragment = sycl::local_accessor<float, 1>(sycl::range{maxlocal}, handler); 

    // sycl::stream out(1024*48, 1024, handler);


    handler.parallel_for_work_group(sycl::range{totSplit}, sycl::range{1}, [=](sycl::group<1> region){
      int _lcount=0;
      using local_atomic_int_ref = sycl::atomic_ref<int, sycl::memory_order::relaxed, sycl::memory_scope::work_group, sycl::access::address_space::local_space>;
      // out << "ID " << " " << region[0] << "\n";
      local_atomic_int_ref lcount(_lcount);
      const int etaIndex = region[0]%etaSplit;
      const int phiIndex = region[0]/etaSplit;
      // this fails to compile
      Wedge wedge;
      wedge.setup(uniform_split(-M_PI, M_PI, phiIndex, phiSplit),
          Reg({0, 15}),
          uniform_split(-4, 4, etaIndex, etaSplit)
        );
      // out << "Data size to process " <<  rinv.size() << "\n";      
      // out << "wedge: " << phiIndex << " " << wedge.m_phi.center 
      //     << " eta index:" << etaIndex << " " << wedge.m_aleft << " " << wedge.m_aright <<"\n";
      region.parallel_for_work_item( sycl::range<1>(256), [&](sycl::h_item<1> item) {
        // out << "WI ID region " << " " << region[0] << " localID " << item.get_logical_local_id()[0] << " " << item.get_logical_local_range()[0] << "\n";
        const int shift = item.get_logical_local_id()[0];
        const int step = item.get_logical_local_range()[0];

        for ( int i = shift; i < rinv.size(); i += step) {
          // out << "r: " << 1.0f/rinv[i] << " phi: " << phi[i] << " z: " << z[i] << "\n";
          if ( wedge.in_wedge_r_phi_z(1.0f/rinv[i], phi[i], z[i]) ){
            rinvFragment[lcount] = rinv[i];
            phiFragment[lcount] = phi[i];
            lcount++; 
          } else {
            // out <<"-";
          }
        }
        // countA[region[0]] = lcount;
        // here is the palce for real agorithm

      });
      // out << "count in ID " << " " << region[0] << " " << static_cast<int>(lcount) << "\n";
    }); // EOF parallel_for_work_group
  }).wait();
  auto t2 = std::chrono::steady_clock::now(); 
  std::cout << "TIME ms " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << std::endl;

  // auto countBack = countBuffer.get_access<sycl::access::mode::read>();
  // std::cout << "counts size"  << countBack.size() << " " << count.size() << std::endl;
  // for ( int i = 0; i < count.size(); ++i) {
  //   std::cout << " " << count[i];
  // }
  std::cout << "\n---\n";
}