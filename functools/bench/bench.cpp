#include <functools.h>

#include <sys/resource.h>
#include <sys/times.h>

#include <vector>
#include <iostream>

using namespace NFuncTools;

struct TResources {
    double UserProcessorTime;
};


double GetProcessorTime() {
    struct rusage rusage;
    if ( getrusage( RUSAGE_SELF, &rusage ) != -1 )
        return (double)rusage.ru_utime.tv_sec + (double)rusage.ru_utime.tv_usec / 1000000.0;
    return 0;
}

template <typename TMeasuredFunction>
TResources Measure(TMeasuredFunction&& fun) {
    double startProcessorTime = GetProcessorTime();
    fun();
    return TResources{
        GetProcessorTime() - startProcessorTime
    };
}

int main() {
    std::cout << Measure([](){
        int ret = 100;
        for (int i : Range(0, 100000, 3)) {
            ret += i * i * i;
        }
        return ret;
    }).UserProcessorTime << std::endl;


}
