#include <gtest/gtest.h>

#include <functools.h>

#include <vector>
#include <set>

#if !defined(native_REALISATION)

using namespace NFuncTools;

class TestFunctools : public ::testing::Test
{
protected:
	void SetUp() {
	}
	void TearDown() {
	}
};



template <typename TContainer>
auto ToVector(TContainer&& container) {
    return std::vector{container.begin(), container.end()};
}

template <typename TContainerObjOrRef>
void TestViewCompileability(TContainerObjOrRef&& container) {
    using TContainer = std::decay_t<TContainerObjOrRef>;
    using TIterator = typename TContainer::iterator;

    static_assert(std::is_same_v<decltype(container.begin()), TIterator>);

    using difference_type = typename std::iterator_traits<TIterator>::difference_type;
    using value_type = typename std::iterator_traits<TIterator>::value_type;
    using reference = typename std::iterator_traits<TIterator>::reference;
    using pointer = typename std::iterator_traits<TIterator>::pointer;

    {
        // operator assignment
        auto it = container.begin();
        it = container.end();
        it = std::move(container.begin());
        // operator copying
        auto it2 = it;
        auto it3 = std::move(it);
    }

    auto it = container.begin();

    // sanity check for types
    using TConstReference = const std::decay_t<reference>&;
    TConstReference ref = *it;
    (void) static_cast<value_type>(*it);
    (void) static_cast<difference_type>(1);
    if constexpr (std::is_reference_v<decltype(*it)>) {
        pointer ptr = &*it;
    }

    // std compatibility
    ToVector(container);
}

struct TTestSentinel {};
struct TTestIterator {
    int operator*() {
        return X;
    }
    void operator++() {
        ++X;
    }
    bool operator!=(const TTestSentinel& other) const {
        return X < 3;
    }

    int X;
};

auto MakeMinimalisticContainer() {
    return MakeIteratorRange(TTestIterator{}, TTestSentinel{});
}


#if !defined(think_cell_REALISATION)
TEST_F(TestFunctools, Range1) {
    std::vector a = {0, 1, 4};
    std::vector<std::vector<int>> b = {
        {},
        {0},
        {0, 1, 2, 3},
    };
    for (int i = 0; i < a.size(); ++i) {
        std::vector<int> range;
        for (auto i : Range(a[i])) {
            ASSERT_EQ(i, (int)range.size());
            range.push_back(i);
        }
        ASSERT_EQ(range, b[i]);
    }
}
#endif

#if !defined(think_cell_REALISATION)
TEST_F(TestFunctools, Range2) {
    std::vector from = {0, 0, 2, 2};
    std::vector to = {0, 1, 4, 6};
    std::vector<std::vector<int>> b = {
        {},
        {0},
        {2, 3},
        {2, 3, 4, 5},
    };
    for (int i = 0; i < from.size(); ++i) {
        std::vector<int> range;
        for (auto j : Range(from[i], to[i])) {
            ASSERT_EQ(j, (int)range.size() + from[i]);
            range.push_back(j);
        }
        ASSERT_EQ(range, b[i]);
    }
}
#endif


#if !defined(range_v3_REALISATION) && !defined(think_cell_REALISATION)
TEST_F(TestFunctools, Range3) {
    std::vector<int> a;
    for (int i : Range(3, 9, 2)) {
        a.push_back(i);
    }
    ASSERT_EQ(a, (std::vector{3, 5, 7}));
}
#endif

#if !defined(baseline_REALISATION) && !defined(baseline_copy_REALISATION) && !defined(range_v3_REALISATION) && !defined(think_cell_REALISATION)
TEST_F(TestFunctools, CompileRange) {
    TestViewCompileability(Range(19));
    TestViewCompileability(Range(10, 19));
    TestViewCompileability(Range(10, 19, 2));
}
#endif


#if !defined(boost_range_REALISATION) && !defined(think_cell_REALISATION)
TEST_F(TestFunctools, Enumerate) {
    std::vector a = {1, 2, 4};
    std::vector<int> b;
    std::vector c = {1};
    for (auto& v : {a, b, c}) {
        int j = 0;
        for (auto [i, x] : Enumerate(v)) {
            ASSERT_EQ(v[i], x);
            ASSERT_EQ(i, j++);
            ASSERT_TRUE(i < v.size());
        }
        ASSERT_EQ(j, v.size());
    }

    std::vector d = {0, 0, 0};
    for (auto [i, x] : Enumerate(d)) {
        x = i;
    }
    ASSERT_TRUE((d == std::vector{0, 1, 2}));
}
#endif

#if !defined(boost_range_REALISATION) && !defined(range_v3_REALISATION) && !defined(think_cell_REALISATION)
TEST_F(TestFunctools, EnumerateTemporary) {
    std::vector a = {1, 2, 4};
    std::vector<int> b;
    std::vector c = {1};
    for (auto& v : {a, b, c}) {
        int j = 0;
        for (auto [i, x] : Enumerate(std::vector(v))) {
            ASSERT_EQ(v[i], x);
            ASSERT_EQ(i, j++);
            ASSERT_TRUE(i < v.size());
        }
        ASSERT_EQ(j, v.size());
    }

    for (auto [i, x] : Enumerate(std::vector{1, 2, 3})) {
        ASSERT_EQ(i + 1, x);
    }
}
#endif

#if !defined(boost_range_REALISATION) && !defined(think_cell_REALISATION) && !defined(range_v3_REALISATION) && !defined(baseline_REALISATION) && !defined(baseline_copy_REALISATION)
TEST_F(TestFunctools, CompileEnumerate) {
    auto container = std::vector{1, 2, 3};
    TestViewCompileability(Enumerate(container));

    std::vector<std::pair<int, int>> res;
    for (auto [i, x] : Enumerate(MakeMinimalisticContainer())) {
        res.push_back({i, x});
    }
    ASSERT_EQ(res, (std::vector<std::pair<int, int>>{
        {0, 0}, {1, 1}, {2, 2},
    }));

}
#endif

TEST_F(TestFunctools, Zip) {
    std::vector<std::pair<std::vector<int32_t>, std::vector<int32_t>>> ts = {
        {{1, 2, 3}, {4, 5, 6}},
        #if !defined(boost_range_REALISATION) && !defined(think_cell_REALISATION)
        {{1, 2, 3}, {4, 5, 6, 7}},
        {{1, 2, 3, 4}, {4, 5, 6}},
        {{1, 2, 3, 4}, {}},
        #endif
    };

    for (auto [a, b] : ts) {
        #if !defined(boost_range_REALISATION) && !defined(range_v3_REALISATION)
        {
            int k = 0;
            for (const auto& [i, j] : Zip(a, std::vector<int32_t>(b))) {
                ASSERT_EQ(++k, i);
                ASSERT_EQ(i + 3, j);
            }
        }
        {
            int k = 0;
            for (const auto& [i, j] : Zip(std::set<int32_t>(a.begin(), a.end()), std::set<int32_t>(b.begin(), b.end()))) {
                ASSERT_EQ(++k, i);
                ASSERT_EQ(i + 3, j);
            }
        }
        #endif

        int k = 0;
        #if !defined(boost_range_REALISATION)
        for (const auto& [i, j] : Zip(a, b)) {
            ASSERT_EQ(++k, i);
            ASSERT_EQ(i + 3, j);
        }
        #else
        auto zipResult = Zip(a, b);
        for (const auto t : zipResult) {
            int32_t i, j;
            boost::tie(i, j) = t;
            ASSERT_EQ(++k, i);
            ASSERT_EQ(i + 3, j);
        }
        #endif
        ASSERT_EQ(k, std::min(a.size(), b.size()));
    }
}


#if !defined(boost_range_REALISATION)
TEST_F(TestFunctools, ZipReference) {
    std::vector a = {0, 1, 2};
    std::vector b = {2, 1, 0, -1};
    for (auto [ai, bi] : Zip(a, b)) {
        ai = bi;
    }
    ASSERT_TRUE((a == std::vector{2, 1, 0}));
}
#endif

#if !defined(boost_range_REALISATION)
TEST_F(TestFunctools, Zip3) {
    std::vector<std::tuple<std::vector<int32_t>, std::vector<int32_t>, std::vector<int32_t>>> ts = {
        {{1, 2, 3}, {4, 5, 6}, {11, 3, 9}},
        #if !defined(think_cell_REALISATION)
        {{1, 2, 3}, {4, 5, 6}, {11, 3}},
        {{1, 2, 3}, {4, 5, 6, 7}, {9, 0}},
        {{1, 2, 3, 4}, {9}, {4, 5, 6}},
        {{1, 2, 3, 4}, {1}, {}},
        {{}, {1}, {1, 2, 3, 4}},
        #endif
    };

    for (auto [a, b, c] : ts) {
        std::vector<std::tuple<int32_t, int32_t, int32_t>> e;
        for (size_t j = 0; j < a.size() && j < b.size() && j < c.size(); ++j) {
            e.push_back({a[j], b[j], c[j]});
        }

        std::vector<std::tuple<int32_t, int32_t, int32_t>> f;
        for (auto [ai, bi, ci] : Zip(a, b, c)) {
            f.push_back({ai, bi, ci});
        }

        ASSERT_EQ(e, f);
    }
}
#endif

#if !defined(baseline_REALISATION) && !defined(baseline_copy_REALISATION) && !defined(range_v3_REALISATION) && !defined(think_cell_REALISATION)
TEST_F(TestFunctools, CompileZip) {
    auto container = std::vector{1, 2, 3};
    TestViewCompileability(Zip(container));
    TestViewCompileability(Zip(container, container, container));

    std::vector<std::pair<int, int>> res;
    for (auto [a, b] : Zip(MakeMinimalisticContainer(), container)) {
        res.push_back({a, b});
    }
    ASSERT_EQ(res, (std::vector<std::pair<int, int>>{
        {0, 1}, {1, 2}, {2, 3},
    }));

}
#endif

TEST_F(TestFunctools, Filter) {
    std::vector<std::vector<int32_t>> ts = {
        {},
        {1},
        {2},
        {1, 2},
        {2, 1},
        {1, 2, 3, 4, 5, 6, 7},
    };

    auto pred = [](int32_t x) -> bool { return x & 1; };

    for (auto& a : ts) {
        std::vector<int32_t> b;
        for (int32_t x : a) {
            if (pred(x)) {
                b.push_back(x);
            }
        }

        std::vector<int32_t> c;
        for (int32_t x : Filter(pred, a)) {
            c.push_back(x);
        }

        ASSERT_EQ(b, c);
    }
}


#if !defined(baseline_REALISATION) && !defined(baseline_copy_REALISATION) && !defined(range_v3_REALISATION) && !defined(think_cell_REALISATION)
TEST_F(TestFunctools, CompileFilter) {
    auto container = std::vector{1, 2, 3};
    auto isOdd = [](int x) { return bool(x & 1); };
    TestViewCompileability(Filter(isOdd, container));
}
#endif

TEST_F(TestFunctools, Map) {
    std::vector<std::vector<int32_t>> ts = {
        {},
        {1},
        {1, 2},
        {1, 2, 3, 4, 5, 6, 7},
    };

    auto f = [](int32_t x) { return x * x; };

    for (auto& a : ts) {
        std::vector<int32_t> b;
        for (int32_t x : a) {
            b.push_back(f(x));
        }

        std::vector<int32_t> c;
        for (int32_t x : Map(f, a)) {
            c.push_back(x);
        }

        ASSERT_EQ(b, c);
    }

    std::vector floats = {1.4, 4.1, 13.9};
    std::vector ints = {1, 4, 13};
    std::vector<float> roundedFloats = {1, 4, 13};
    std::vector<int> res;
    std::vector<float> resFloat;
    for (auto i : Map<int>(floats)) {
        res.push_back(i);
    }
    for (auto i : Map<float>(Map<int>(floats))) {
        resFloat.push_back(i);
    }
    ASSERT_EQ(ints, res);
    ASSERT_EQ(roundedFloats, resFloat);
}

#if !defined(baseline_REALISATION) && !defined(baseline_copy_REALISATION) && !defined(range_v3_REALISATION) && !defined(think_cell_REALISATION)
TEST_F(TestFunctools, CompileMap) {
    auto container = std::vector{1, 2, 3};
    auto sqr = [](int x) { return x * x; };
    TestViewCompileability(Map(sqr, container));
}
#endif

#if !defined(boost_range_REALISATION) && !defined(think_cell_REALISATION)
TEST_F(TestFunctools, CartesianProduct) {
    std::vector<std::pair<std::vector<int32_t>, std::vector<int32_t>>> ts = {
        {{1, 2, 3}, {4, 5, 6}},
        {{1, 2, 3}, {4, 5, 6, 7}},
        {{1, 2, 3, 4}, {4, 5, 6}},
        {{1, 2, 3, 4}, {}},
        {{}, {1, 2, 3, 4}},
    };

    for (auto [a, b] : ts) {
        std::vector<std::pair<int32_t, int32_t>> c;
        for (auto ai : a) {
            for (auto bi : b) {
                c.push_back({ai, bi});
            }
        }

        std::vector<std::pair<int32_t, int32_t>> d;
        for (auto [ai, bi] : CartesianProduct(a, b)) {
            d.push_back({ai, bi});
        }

        ASSERT_EQ(c, d);
    }

    {
        std::vector<std::vector<int>> g = {{}, {}};
        std::vector h = {10, 11, 12};
        for (auto [gi, i] : CartesianProduct(g, h)) {
            gi.push_back(i);
        }
        ASSERT_EQ(g[0], h);
        ASSERT_EQ(g[1], h);
    }
}
#endif

#if !defined(boost_range_REALISATION) && !defined(think_cell_REALISATION)
TEST_F(TestFunctools, CartesianProduct3) {
    std::vector<std::tuple<std::vector<int32_t>, std::vector<int32_t>, std::vector<int32_t>>> ts = {
        {{1, 2, 3}, {4, 5, 6}, {11, 3}},
        {{1, 2, 3}, {4, 5, 6, 7}, {9}},
        {{1, 2, 3, 4}, {9}, {4, 5, 6}},
        {{1, 2, 3, 4}, {1}, {}},
        {{}, {1}, {1, 2, 3, 4}},
    };

    for (auto [a, b, c] : ts) {
        std::vector<std::tuple<int32_t, int32_t, int32_t>> e;
        for (auto ai : a) {
            for (auto bi : b) {
                for (auto ci : c) {
                    e.push_back({ai, bi, ci});
                }
            }
        }

        std::vector<std::tuple<int32_t, int32_t, int32_t>> f;
        for (auto [ai, bi, ci] : CartesianProduct(a, b, c)) {
            f.push_back({ai, bi, ci});
        }

        ASSERT_EQ(e, f);
    }
}
#endif

#if !defined(baseline_REALISATION) && !defined(baseline_copy_REALISATION) && !defined(range_v3_REALISATION) && !defined(think_cell_REALISATION)
TEST_F(TestFunctools, CompileCartesianProduct) {
    auto container = std::vector{1, 2, 3};
    TestViewCompileability(CartesianProduct(container, container));

    std::vector<std::pair<int, int>> res;
    for (auto [a, b] : CartesianProduct(MakeMinimalisticContainer(), MakeMinimalisticContainer())) {
        res.push_back({a, b});
    }
    ASSERT_EQ(res, (std::vector<std::pair<int, int>>{
        {0, 0}, {0, 1}, {0, 2},
        {1, 0}, {1, 1}, {1, 2},
        {2, 0}, {2, 1}, {2, 2},
    }));
}
#endif

#if !defined(boost_range_REALISATION)
TEST_F(TestFunctools, Concatenate2) {
    std::vector<std::pair<std::vector<int32_t>, std::vector<int32_t>>> ts = {
        {{1, 2, 3}, {4, 5, 6}},
        {{1, 2, 3}, {4, 5, 6, 7}},
        {{1, 2, 3, 4}, {4, 5, 6}},
        {{1, 2, 3, 4}, {}},
        {{}, {1, 2, 3, 4}},
    };

    for (auto [a, b] : ts) {
        std::vector<int32_t> c;
        for (auto ai : a) {
            c.push_back(ai);
        }
        for (auto bi : b) {
            c.push_back(bi);
        }

        std::vector<int32_t> d;
        for (auto x : Concatenate(a, b)) {
            d.push_back(x);
        }

        ASSERT_EQ(c, d);
    }

    #if !defined(range_v3_REALISATION) && !defined(think_cell_REALISATION)
    {
        std::vector<int32_t> a = {1, 2, 3, 4};
        std::vector<int32_t> c;
        for (auto x : Concatenate(a, std::vector<int32_t>{5, 6})) {
            c.push_back(x);
        }
        ASSERT_EQ(c, (std::vector<int32_t>{1, 2, 3, 4, 5, 6}));
    }
    #endif
}
#endif

#if !defined(baseline_REALISATION) && !defined(baseline_copy_REALISATION) && !defined(range_v3_REALISATION) && !defined(think_cell_REALISATION)
TEST_F(TestFunctools, CompileConcatenate) {
    auto container = std::vector{1, 2, 3};
    TestViewCompileability(Concatenate(container, container));
    std::vector<int> res;
    for (auto a : Concatenate(MakeMinimalisticContainer(), MakeMinimalisticContainer())) {
        res.push_back(a);
    }
    ASSERT_EQ(res, (std::vector{0, 1, 2, 0, 1, 2}));
}
#endif


#if !defined(boost_range_REALISATION) && !defined(range_v3_REALISATION) && !defined(think_cell_REALISATION)
TEST_F(TestFunctools, Flatten) {
    {
        std::vector <int> a = {1, 2, 3};
        std::vector <int> b;
        for (auto [x] : Flatten(a)) {
            b.push_back(x);
        }
        ASSERT_EQ(a, b);
    }
    {
        std::vector <int> a = {1, 2, 3};
        std::vector <int> b;
        for (auto [i, x] : Flatten(Enumerate(a))) {
            ASSERT_EQ(a[i], x);
            b.push_back(x);
        }
        ASSERT_EQ(a, b);
    }
    {
        std::vector <int> a = {1, 2, 3};
        std::vector <int> b;
        for (auto [i, j, x] : Flatten(Enumerate(Enumerate(a)))) {
            ASSERT_EQ(i, j);
            ASSERT_EQ(a[i], x);
            b.push_back(x);
        }
        ASSERT_EQ(a, b);
    }
}
#endif

#if !defined(boost_range_REALISATION) && !defined(range_v3_REALISATION) && !defined(think_cell_REALISATION)
TEST_F(TestFunctools, Combo) {
    for (auto [i, j] : Enumerate(Range(10))) {
        ASSERT_EQ(i, j);
    }

    for (auto [i, j, k] : Flatten(Enumerate(Enumerate(Range(10))))) {
        ASSERT_EQ(i, j);
        ASSERT_EQ(j, k);
    }

    std::vector a = {0, 1, 2};
    for (auto [i, j] : Enumerate(NFuncTools::Reversed(a))) {
        ASSERT_EQ(i, 2 - j);
    }

    for (auto [i, j] : Enumerate(Map<float>(a))) {
        ASSERT_EQ(i, (int)j);
    }

    for (auto [i, j] : Zip(a, Map<float>(a))) {
        ASSERT_EQ(i, (int)j);
    }

    auto mapper = [](auto&& x) {
        auto& [u, v] = x;
        return u + v;
    };
    for (auto [i, j] : Zip(a, Map(mapper, Zip(a, a)))) {
        ASSERT_EQ(j, 2 * i);
    }

    for (auto [i, j, k, l] : Flatten(Zip(Zip(a, a), Zip(a, a)))) {
        ASSERT_EQ(i, j);
        ASSERT_EQ(i, k);
        ASSERT_EQ(i, l);
    }

    {
        // silly quadratic sort
        std::vector a = {5, 3, 4, 1 ,2};
        for (auto [i, j] : CartesianProduct(Range(a.size()), Range(a.size()))) {
            if (i < j && a[i] > a[j]) {
                std::swap(a[i], a[j]);
            }
        }
        for (auto i : Range(a.size() - 1)) {
            ASSERT_TRUE(a[i] < a[i + 1]);
        }
    }

    {
        // silly check for palindromness
        auto check = [](const auto& c) {
            for (auto [x, rx] : Zip(c, NFuncTools::Reversed(c))) {
                if (x != rx) {
                    return false;
                }
            }
            return true;
        };

        ASSERT_TRUE(!check(std::vector{5, 3, 4, 1 ,2}));
        ASSERT_TRUE(check(std::vector{5, 3, 4, 3 ,5}));
    }
}
#endif

#if !defined(boost_range_REALISATION) && !defined(range_v3_REALISATION) && !defined(think_cell_REALISATION)
TEST_F(TestFunctools, CopyIterator) {
    std::vector a = {1, 2, 3, 4};
    std::vector b = {4, 5, 6, 7};

    // calls f on 2nd, 3d and 4th positions (numeration from 1st)
    auto testIterator = [](auto it, auto f) {
        ++it;
        auto it2 = it;
        ++it2;
        ++it2;
        auto it3 = it;
        ++it3;
        f(*it, *it3, *it2);
    };

    {
        auto iterable = Enumerate(a);
        testIterator(std::begin(iterable),
            [](auto p2, auto p3, auto p4) {
                ASSERT_EQ(std::get<0>(p2), 1);
                ASSERT_EQ(std::get<1>(p2), 2);
                ASSERT_EQ(std::get<0>(p3), 2);
                ASSERT_EQ(std::get<1>(p3), 3);
                ASSERT_EQ(std::get<0>(p4), 3);
                ASSERT_EQ(std::get<1>(p4), 4);
            });
    }

    {
        auto iterable = Map([](int32_t x) { return x*x; }, a);
        testIterator(std::begin(iterable),
            [](auto p2, auto p3, auto p4) {
                ASSERT_EQ(p2, 4);
                ASSERT_EQ(p3, 9);
                ASSERT_EQ(p4, 16);
            });
    }

    {
        auto iterable = Zip(a, b);
        testIterator(std::begin(iterable),
            [](auto p2, auto p3, auto p4) {
                ASSERT_EQ(std::get<0>(p2), 2);
                ASSERT_EQ(std::get<1>(p2), 5);
                ASSERT_EQ(std::get<0>(p3), 3);
                ASSERT_EQ(std::get<1>(p3), 6);
                ASSERT_EQ(std::get<0>(p4), 4);
                ASSERT_EQ(std::get<1>(p4), 7);
            });
    }

    {
        auto c = {1, 2, 3, 4, 5, 6, 7, 8};
        auto iterable = Filter([](int32_t x) { return !(x & 1); }, c);
        testIterator(std::begin(iterable),
            [](auto p2, auto p3, auto p4) {
                ASSERT_EQ(p2, 4);
                ASSERT_EQ(p3, 6);
                ASSERT_EQ(p4, 8);
            });
    }

    {
        auto iterable = CartesianProduct(std::vector{0, 1}, std::vector{2, 3});
        // (0, 2), (0, 3), (1, 2), (1, 3)
        testIterator(std::begin(iterable),
            [](auto p2, auto p3, auto p4) {

                ASSERT_EQ(std::get<0>(p2), 0);
                ASSERT_EQ(std::get<1>(p2), 3);
                ASSERT_EQ(std::get<0>(p3), 1);
                ASSERT_EQ(std::get<1>(p3), 2);
                ASSERT_EQ(std::get<0>(p4), 1);
                ASSERT_EQ(std::get<1>(p4), 3);
            });
    }
}
#endif

#endif // #if !defined(native_REALISATION)

int main(int argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
