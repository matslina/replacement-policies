cmake_minimum_required (VERSION 2.6)
project (replacement-policies C)

enable_testing()
add_test(slru test/slru_test)

set(CMAKE_C_FLAGS -g)

add_executable(bench bench.c htable.c fifo.c rnd.c clk.c gclk.c lru.c slru.c)

find_library(check check)

enable_testing()

add_executable(htable_test htable_test.c    htable.c)
add_executable(fifo_test   fifo_test.c fifo.c htable.c)
add_executable(rnd_test    rnd_test.c  rnd.c  htable.c)
add_executable(clk_test    clk_test.c  clk.c  htable.c)
#add_executable(gclk_test   gclk_test.c gclk.c htable.c)
add_executable(lru_test    lru_test.c  lru.c  htable.c)
add_executable(slru_test   slru_test.c slru.c htable.c)

target_link_libraries(htable_test check)
target_link_libraries(fifo_test   check)
target_link_libraries(rnd_test    check)
target_link_libraries(clk_test    check)
#target_link_libraries(gclk_test   check)
target_link_libraries(lru_test    check)
target_link_libraries(slru_test   check)

add_test(htable htable_test)
add_test(fifo fifo_test)
add_test(rnd rnd_test)
add_test(clk clk_test)
#add_test(gclk gclk_test)
add_test(lru lru_test)
add_test(slru slru_test)
