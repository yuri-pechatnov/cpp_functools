#include <functools.h>

#include <vector>

using namespace NFuncTools;


#if defined(BenchEnumerate_BENCH)
int BenchEnumerate(const std::vector<int>& a) {
    int res = 0;
    #ifdef USE_FUNCTOOLS
        for (auto [j, aj] : Enumerate(a)) {
            res += j * aj;
        }
    #endif
    #ifdef USE_NATIVE
        for (size_t j = 0; j < a.size(); ++j) {
            res += j * a[j];
        }
    #endif
    return res;
}
#endif

#if defined(BenchZip_BENCH)
int BenchZip(const std::vector<int>& a, const std::vector<int>& b) {
    int res = 0;
    #ifdef USE_FUNCTOOLS
        #if !defined(boost_range_REALISATION)
            for (auto [aj, bj] : Zip(a, b)) {
                res += aj * bj;
            }
        #else
            for (auto t : Zip(a, b)) {
                int aj, bj;
                boost::tie(aj, bj) = t;
                res += aj * bj;
            }
        #endif
    #endif
    #ifdef USE_NATIVE
        for (size_t j = 0; j < a.size() && j < b.size(); ++j) {
            res += a[j] * b[j];
        }
    #endif
    return res;
}
#endif

#if defined(BenchFilter_BENCH)
int BenchFilter(const std::vector<int>& a) {
    int res = 0;
    auto pred = [](auto x) {
        return bool(x & 1);
    };
    #ifdef USE_FUNCTOOLS
        for (auto aj : Filter(pred, a)) {
            res += aj;
        }
    #endif
    #ifdef USE_NATIVE
        for (size_t j = 0; j < a.size(); ++j) {
            if (a[j] & 1) {
                res += a[j];
            }
        }
    #endif
    return res;
}
#endif

#if defined(BenchCartesianProduct_BENCH)
int BenchCartesianProduct(const std::vector<int>& a, const std::vector<int>& b) {
    int res = 0;
    #ifdef USE_FUNCTOOLS
        for (auto [aj, bj] : CartesianProduct(a, b)) {
            res += aj * bj;
        }
    #endif
    #ifdef USE_NATIVE
        for (size_t j = 0; j < a.size(); ++j) {
            for (size_t k = 0; k < b.size(); ++k) {
                res += c[j] * d[k];
            }
        }
    #endif
}
#endif


#if defined(BenchConcatenate_BENCH)
int BenchConcatenate(const std::vector<int>& a, const std::vector<int>& b) {
    int res = 0;
    #ifdef USE_FUNCTOOLS
        for (auto x : Concatenate(a, b)) {
            res += x;
        }
    #endif
    #ifdef USE_NATIVE
        for (size_t j = 0; j < a.size(); ++j) {
            res += a[j];
        }
        for (size_t j = 0; j < b.size(); ++j) {
            res += b[j];
        }
    #endif
    return res;
}
#endif
