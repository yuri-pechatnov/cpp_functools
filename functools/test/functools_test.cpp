#include <gtest/gtest.h>

#include <functools.h>

#include <vector>

using namespace NFuncTools;

class TestFunctools : public ::testing::Test
{
protected:
	void SetUp() {
	}
	void TearDown() {
	}
};

#if !defined(boost_range_REALISATION)
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

#if !defined(boost_range_REALISATION)
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

TEST_F(TestFunctools, Zip) {
    std::vector<std::pair<std::vector<int32_t>, std::vector<int32_t>>> ts = {
        {{1, 2, 3}, {4, 5, 6}},
#if !defined(boost_range_REALISATION)
        {{1, 2, 3}, {4, 5, 6, 7}},
        {{1, 2, 3, 4}, {4, 5, 6}},
        {{1, 2, 3, 4}, {}},
#endif
    };

    for (auto [a, b] : ts) {
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
        {{1, 2, 3}, {4, 5, 6}, {11, 3}},
        {{1, 2, 3}, {4, 5, 6, 7}, {9, 0}},
        {{1, 2, 3, 4}, {9}, {4, 5, 6}},
        {{1, 2, 3, 4}, {1}, {}},
        {{}, {1}, {1, 2, 3, 4}},
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

#if !defined(boost_range_REALISATION)
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

#if !defined(boost_range_REALISATION)
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

    {
        std::vector<int32_t> a = {1, 2, 3, 4};
        std::vector<int32_t> c;
        for (auto x : Concatenate(a, std::vector<int32_t>{5, 6})) {
            c.push_back(x);
        }
        ASSERT_EQ(c, (std::vector<int32_t>{1, 2, 3, 4, 5, 6}));
    }
}
#endif


#if !defined(boost_range_REALISATION)
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

#if !defined(boost_range_REALISATION)
TEST_F(TestFunctools, Combo) {
    for (auto [i, j] : Enumerate(Range(10))) {
        ASSERT_EQ(i, j);
    }

    for (auto [i, j, k] : Flatten(Enumerate(Enumerate(Range(10))))) {
        ASSERT_EQ(i, j);
        ASSERT_EQ(j, k);
    }

    std::vector a = {0, 1, 2};
    for (auto [i, j] : Enumerate(Reversed(a))) {
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
            for (auto [x, rx] : Zip(c, Reversed(c))) {
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

#if !defined(boost_range_REALISATION)
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



int main(int argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}