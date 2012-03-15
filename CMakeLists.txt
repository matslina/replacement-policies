cmake_minimum_required (VERSION 2.6)
project (replacement-policies C)

enable_testing()
add_test(slru test/slru_test)

set(CMAKE_C_FLAGS -g)

add_subdirectory(src bin)
add_subdirectory(test test)

find_library(check check)

enable_testing()

add_test(htable test/htable_test)
add_test(fifo test/fifo_test)
add_test(rnd test/rnd_test)
add_test(clk test/clk_test)
#add_test(gclk test/gclk_test)
add_test(lru test/lru_test)
add_test(slru test/slru_test)
