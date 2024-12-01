file(GLOB is31se5117a_SOURCES src/*.cpp)

add_library(is31se5117a STATIC ${is31se5117a_SOURCES})

target_include_directories(
  is31se5117a
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include/is31se5117a)

target_link_libraries(is31se5117a PUBLIC pico_stdlib hardware_i2c)
