include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.11.0
  )

FetchContent_MakeAvailable(googletest)
add_library(GTest::GTest INTERFACE IMPORTED)
target_link_libraries(GTest::GTest INTERFACE gtest_main)



add_executable(
  AccumulatorSectionTest
  AccumulatorSectionTestSuite.cpp  
)

target_link_libraries(AccumulatorSectionTest
 PRIVATE
  GTest::GTest)
target_include_directories(AccumulatorSectionTest PRIVATE ../include)

add_test(AccumulatorSectionTest_getst AccumulatorSectionTest)  


add_executable(
  SorterTest
  SorterTestSuite.cpp  
)

target_link_libraries(SorterTest
 PRIVATE
  GTest::GTest)
target_include_directories(SorterTest PRIVATE ../include)

add_test(SorterTest_getst SorterTest)  
