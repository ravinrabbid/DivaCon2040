file(GLOB cap1188_SOURCES src/*.cpp)

add_library(cap1188 STATIC ${cap1188_SOURCES})

target_include_directories(
  cap1188
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include/cap1188)

target_link_libraries(cap1188 PUBLIC pico_stdlib hardware_i2c)
