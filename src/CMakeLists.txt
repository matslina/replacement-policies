add_library(replacement-policies STATIC
            htable.c linkmap.c fifo.c rnd.c clk.c gclk.c lru.c slru.c)
add_executable(bench bench.c)
target_link_libraries(bench replacement-policies)
