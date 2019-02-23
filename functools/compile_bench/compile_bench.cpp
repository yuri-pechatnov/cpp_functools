#include <functools.h>
#include <vector>


#if !defined(native_REALISATION)
using namespace NFuncTools;
#endif


#if defined(BenchEnumerate_BENCH)
int BenchEnumerate(const std::vector<int>& a) {
    int res = 0;
    #if !defined(native_REALISATION)
        for (auto [j, aj] : Enumerate(a)) {
            res += j * aj;
        }
    #else
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
    #if !defined(native_REALISATION)
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
    #else
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
    #if !defined(native_REALISATION)
        for (auto aj : Filter(pred, a)) {
            res += aj;
        }
    #else
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
    #if !defined(native_REALISATION)
        for (auto [aj, bj] : CartesianProduct(a, b)) {
            res += aj * bj;
        }
    #else
        for (size_t j = 0; j < a.size(); ++j) {
            for (size_t k = 0; k < b.size(); ++k) {
                res += a[j] * b[k];
            }
        }
    #endif
    return res;
}
#endif


#if defined(BenchConcatenate_BENCH)
int BenchConcatenate(const std::vector<int>& a, const std::vector<int>& b) {
    int res = 0;
    #if !defined(native_REALISATION)
        for (auto x : Concatenate(a, b)) {
            res += x;
        }
    #else
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
