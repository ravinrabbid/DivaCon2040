file(GLOB mpr121_SOURCES src/*.cpp)

add_library(mpr121 STATIC ${mpr121_SOURCES})

target_include_directories(
  mpr121
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include/mpr121)

target_link_libraries(mpr121 PUBLIC pico_stdlib hardware_i2c)
