extern volatile int BenchWithGdbCounter = 0;
extern volatile int BenchWithGdbBenchActive = 0;

void BenchWithGdbStartLabel() {
    BenchWithGdbBenchActive = 1;
}

void BenchWithGdbFinishLabel() {
    BenchWithGdbBenchActive = 0;
    ++BenchWithGdbCounter;
}
