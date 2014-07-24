CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

file(GLOB common_sources ./*.cpp)
file(GLOB fields_sources ./fields/*.cpp)
file(GLOB extended_sources ./fields/extended/*.cpp)
file(GLOB stl_sources ./fields/extended/stl/*.cpp)
file(GLOB jsonxx_sources ./utils/jsonxx/*.cc)
file(GLOB trex_sources ./utils/trex/*.c)
file(GLOB utils_sources ./utils/*.cpp)

include_directories( ./ )
include_directories( ./utils/ )

SET(CC_COVERAGE_COMPILE_FLAGS "-fPIC -O2 -g")
SET(CC_COVERAGE_LINK_FLAGS    "")

SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${CC_COVERAGE_COMPILE_FLAGS} ${CC_COVERAGE_LINK_FLAGS}" )
SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${CC_COVERAGE_LINK_FLAGS}" ) 

add_library (cjson SHARED ${common_sources} ${fields_sources} ${extended_sources} ${stl_sources} ${jsonxx_sources} ${trex_sources} ${utils_sources} )
target_link_libraries ( cjson ${CMAKE_DL_LIBS} )
