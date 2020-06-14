
#define NDEBUG
#include <iostream>
#include <cstddef>
#include <vector>
#include <chrono>
#include <cassert>

#ifdef __GNUC__
#define ALLWAYS_INLINE __attribute__((always_inline))
#elif _MSC_VER
#define ALLWAYS_INLINE __forceinline
#else
#define ALLWAYS_INLINE
#endif

template<typename T, typename Rep = std::vector<T>>
class Vec {

    Rep expr_rep;

public:

    explicit Vec(std::size_t s) : expr_rep(s) { }

    Vec(const Rep& rb) : expr_rep(rb) { }

    std::size_t size() const { return expr_rep.size(); }

    Vec& operator=(const Vec& b) {

        assert(size() == b.size());

        for (std::size_t i = 0 ; i < b.size() ; ++i)
            expr_rep[i] = b[i];

        return *this;
    }

    template<typename T2, typename Rep2>
    ALLWAYS_INLINE Vec& operator=(const Vec<T2, Rep2>& b) {

        assert(size() == b.size());

        for (std::size_t i = 0 ; i < b.size() ; ++i)
            expr_rep[i] = b[i];

        return *this;
    }

    decltype(auto) operator[](std::size_t i) const {

        assert(i < size());

        return expr_rep[i];
    }

    T& operator[](std::size_t i) {

        assert(i < size());

        return expr_rep[i];
    }

    const Rep& rep() const { return expr_rep; }

    Rep& rep() { return expr_rep; }
};

template<typename T>
class Scalar {
    const T& s;

public:

    constexpr Scalar(const T& v) : s(v) {}

    constexpr const T& operator[](std::size_t) const { return s; }

    constexpr std::size_t size() const { return 0; }
};

template<typename T>
struct VecTraits {
    using ExprRef = const T&;
};

template<typename T>
struct VecTraits<Scalar<T>> {
    using ExprRef = Scalar<T>;
};

template<typename T, typename OP1, typename OP2>
class AddV {

    typename VecTraits<OP1>::ExprRef op1;
    typename VecTraits<OP2>::ExprRef op2;

public:

    AddV(const OP1& a, const OP2& b) : op1(a), op2(b) {}

    T operator[](std::size_t i) const { return op1[i] + op2[i]; }

    std::size_t size() const {
        assert(op1.size() == 0 || op2.size() == 0 || op1.size() == op2.size());

        return op1.size() != 0 ? op1.size() : op2.size();
    }
};

template<typename T, typename OP1, typename OP2>
class MultV {

    typename VecTraits<OP1>::ExprRef op1;
    typename VecTraits<OP2>::ExprRef op2;

public:

    MultV(const OP1& a, const OP2& b) : op1(a), op2(b) {}

    T operator[](std::size_t i) const { return op1[i] * op2[i]; }

    std::size_t size() const {
        assert(op1.size() == 0 || op2.size() == 0 || op1.size() == op2.size());

        return op1.size() != 0 ? op1.size() : op2.size();
    }
};

template<typename T, typename Rep1, typename Rep2>
inline auto operator+(const Vec<T, Rep1>& a, const Vec<T, Rep2>& b)
{
    using Rep = AddV<T, Rep1, Rep2>;

    return Vec<T, Rep>{ Rep{a.rep(), b.rep()} };
}

template<typename T, typename Rep2>
inline auto operator*(const T& a, const Vec<T, Rep2>& b)
{
    using Rep = MultV<T, Scalar<T>, Rep2>;

    return Vec<T, Rep>{ Rep{Scalar<T>(a), b.rep()} };
}

template<typename T, typename Rep1>
inline auto operator*(const Vec<T, Rep1>& a, const T& b)
{
    return b*a;
}

template<typename T, typename Rep1, typename Rep2>
inline auto operator*(const Vec<T, Rep1>& a, const Vec<T, Rep2>& b)
{
    using Rep = MultV<T, Rep1, Rep2>;

    return Vec<T, Rep>{ Rep{a.rep(), b.rep()} };
}

class Double {

    double d;

public:

    inline static int count = 0;

    Double() { }

    Double(double dd) : d(dd) { count++; }

    Double(const Double& dd) : d(dd.d) { count++; }

    Double& operator=(const Double& dd) { d = dd.d; count++; return *this; }

    friend Double operator+(const Double& a, const Double& b) {
        return Double{a.d + b.d};
    }

    friend Double operator*(const Double& a, const Double& b) {
        return Double{a.d * b.d};
    }
};

int main()
{
    constexpr int N = 1000*1000*100;

    Vec<double> v1(N);
    Vec<double> v2(N);
    Vec<double> v3(N);

    std::cout << v1.size() << "\n";

    for (std::size_t i = 0 ; i < v1.size() ; ++i)
    {
        v1[i] = 1.0;
        v2[i] = 2.0;
    }

    Double::count = 0;

    double a{1.5};
    double b{1.25};

    auto t0 = std::chrono::steady_clock::now();

    v3 = a*v1*b + v2*v2;

    /*
    using Rep1 = MultV<double, Scalar<double>, Vec<double>>;
    using Rep2 = MultV<double, Rep1, Scalar<double>>;
    using Rep3 = MultV<double, Vec<double>, Vec<double>>;
    using Rep4 = AddV<double, Rep2, Rep3>;

    v3 = Vec<double, Rep4>(Rep4(Rep2(Rep1(a,v1),b), Rep3(v2,v2)));
    */
    auto t1 = std::chrono::steady_clock::now();

    std::chrono::duration<double> dur = t1 - t0;

    std::cout << dur.count() << "\n";

    t0 = std::chrono::steady_clock::now();

    for (std::size_t i = 0 ; i < v1.size() ; ++i)
        v3[i] = a*v1[i]*b + v2[i]*v2[i];

    t1 = std::chrono::steady_clock::now();

    dur = t1 - t0;

    std::cout << dur.count() << "\n";

    std::cout << Double::count << "\n";

    return 0;
}
