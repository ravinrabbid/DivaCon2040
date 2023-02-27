file(GLOB pico_ssd1306_SOURCES src/*.c)

add_library(pico_ssd1306 STATIC ${pico_ssd1306_SOURCES})

target_include_directories(
  pico_ssd1306
  PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include/ssd1306)

target_link_libraries(pico_ssd1306 PUBLIC pico_stdlib hardware_i2c hardware_dma)
