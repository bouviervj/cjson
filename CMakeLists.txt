CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(CJson C CXX)

INCLUDE(find_cmake/FindXercesC.cmake)

include_directories( ./ )
include_directories( ./lib )
include_directories( ./lib/utils )

add_subdirectory (lib)

SET(CC_COVERAGE_COMPILE_FLAGS "-fPIC -g -O3")
SET(CC_COVERAGE_LINK_FLAGS    "")

SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${CC_COVERAGE_COMPILE_FLAGS} ${CC_COVERAGE_LINK_FLAGS}" )
SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${CC_COVERAGE_LINK_FLAGS}" ) 

file(GLOB main_sources ./main.cc)
file(GLOB unittest_sources ./unittest.cc)
file(GLOB test_sources ./test.cc)

add_executable(main ${main_sources} )
target_link_libraries(main cjson ${XERCESC_LIBRARY})

add_executable(unittest ${unittest_sources} )
target_link_libraries(unittest cjson ${XERCESC_LIBRARY})

add_executable(test ${test_sources} )
target_link_libraries(test cjson ${XERCESC_LIBRARY})

