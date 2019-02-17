#include <functools.h>
#include <json/json.h>

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
Json::Value Measure(TMeasuredFunction&& fun) {
    double startProcessorTime = GetProcessorTime();
    int hash = fun();
    Json::Value res;
    res["UserProcessorTime"] = GetProcessorTime() - startProcessorTime;
    res["ResultHash"] = hash;
    return res;
}

int metaIterations = 3;
std::vector<int> a, b, c, d;

void InitData() {
    for (int i : Range(1000000)) {
        a.push_back(i * i * i ^ i);
        b.push_back(i * i * i | i);
    }
    for (int i : Range(1000)) {
        c.push_back(i * i * i ^ i);
        d.push_back(i * i * i | i);
    }
}

#if !defined(boost_range_REALISATION)
int BenchEnumerate() {
    int res = 0;
    for (int i : Range(metaIterations)) {
        #ifdef USE_FUNCTOOLS
            for (auto [j, aj] : Enumerate(a)) {
                res += i ^ j * aj;
            }
        #endif
        #ifdef USE_NATIVE
            for (size_t j = 0; j < a.size(); ++j) {
                res += i ^ j * a[j];
            }
        #endif
    }
    return res;
}
#endif

int BenchZip() {
    int res = 0;
    for (int i : Range(metaIterations)) {
        #ifdef USE_FUNCTOOLS
            #if !defined(boost_range_REALISATION)
                for (auto [aj, bj] : Zip(a, b)) {
                    res += i ^ aj * bj;
                }
            #else
                for (auto t : Zip(a, b)) {
                    int aj, bj;
                    boost::tie(aj, bj) = t;
                    res += i ^ aj * bj;
                }
            #endif
        #endif
        #ifdef USE_NATIVE
            for (size_t j = 0; j < a.size() && j < b.size(); ++j) {
                res += i ^ a[j] * b[j];
            }
        #endif
    }
    return res;
}

int BenchFilter() {
    int res = 0;
    auto pred = [](auto x) {
        return bool(x & 1);
    };
    for (int i : Range(metaIterations)) {
        #ifdef USE_FUNCTOOLS
            for (auto aj : Filter(pred, a)) {
                res += i ^ aj;
            }
        #endif
        #ifdef USE_NATIVE
            for (size_t j = 0; j < a.size(); ++j) {
                if (a[j] & 1) {
                    res += i ^ a[j];
                }
            }
        #endif
    }
    return res;
}


#if !defined(boost_range_REALISATION)
int BenchCartesianProduct() {
    int res = 0;
    for (int i : Range(metaIterations)) {
        #ifdef USE_FUNCTOOLS
            for (auto [aj, bj] : CartesianProduct(c, d)) {
                res += i ^ aj * bj;
            }
        #endif
        #ifdef USE_NATIVE
            for (size_t j = 0; j < c.size(); ++j) {
                for (size_t k = 0; k < d.size(); ++k) {
                    res += i ^ c[j] * d[k];
                }
            }
        #endif
    }
    return res;
}
#endif


#if !defined(boost_range_REALISATION)
int BenchConcatenate() {
    int res = 0;
    for (int i : Range(metaIterations)) {
        #ifdef USE_FUNCTOOLS
            for (auto x : Concatenate(a, b)) {
                res += i ^ x;
            }
        #endif
        #ifdef USE_NATIVE
            for (size_t j = 0; j < a.size(); ++j) {
                res += i ^ a[j];
            }
            for (size_t j = 0; j < b.size(); ++j) {
                res += i ^ b[j];
            }
        #endif
    }
    return res;
}
#endif

int main() {
    InitData();
    Json::Value report;
    auto& result = report["Benchmarks"];

    BenchFilter();

    #define MEASURE(func) result[#func] = Measure(&func);
        MEASURE(BenchZip);
        MEASURE(BenchFilter);
        #if !defined(boost_range_REALISATION)
            MEASURE(BenchEnumerate);
            MEASURE(BenchConcatenate);
            MEASURE(BenchCartesianProduct);
        #endif
    #undef MEASURE

    std::cout << report << std::endl;
    return 0;
}
