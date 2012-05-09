find_library(check check)
find_library(replacement-policies replacement-policies)

include_directories(../src)

add_executable(htable_test htable_test.c)
add_executable(lrutable_test lrutable_test.c)
add_executable(fifo_test   fifo_test.c)
add_executable(rnd_test    rnd_test.c)
add_executable(clk_test    clk_test.c)
#add_executable(gclk_test   gclk_test.c)
add_executable(lru_test    lru_test.c)
add_executable(slru_test   slru_test.c)

target_link_libraries(htable_test check)
target_link_libraries(lrutable_test check)
target_link_libraries(fifo_test   check)
target_link_libraries(rnd_test    check)
target_link_libraries(clk_test    check)
#target_link_libraries(gclk_test   check)
target_link_libraries(lru_test    check)
target_link_libraries(slru_test   check)

target_link_libraries(htable_test replacement-policies)
target_link_libraries(lrutable_test replacement-policies)
target_link_libraries(fifo_test   replacement-policies)
target_link_libraries(rnd_test    replacement-policies)
target_link_libraries(clk_test    replacement-policies)
#target_link_libraries(gclk_test   replacement-policies)
target_link_libraries(lru_test    replacement-policies)
target_link_libraries(slru_test   replacement-policies)


