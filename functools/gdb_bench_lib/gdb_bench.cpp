#include "gdb_bench.h"

volatile int BenchWithGdbCounter = 0;
volatile int BenchWithGdbBenchActive = 0;

void BenchWithGdbStartLabel() {
    BenchWithGdbBenchActive = 1;
}

void BenchWithGdbFinishLabel() {
    BenchWithGdbBenchActive = 0;
    ++BenchWithGdbCounter;
}
