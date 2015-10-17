#include <cstdlib>
#include <iostream>
#include <functional>
#include <tuple>

namespace bind_test
{


    // Stores a tuple of indices.  Used by tuple and pair, and by bind() to
    // extract the elements in a tuple.
    template<size_t... _Indexes>
    struct _Index_tuple
    {
        constexpr _Index_tuple()
        {}

        typedef _Index_tuple<_Indexes..., sizeof...(_Indexes)> __next;
    };

    // Builds an _Index_tuple<0, 1, 2, ..., _Num-1>.
    template<size_t _Num>
    struct _Build_index_tuple
    {
        typedef typename _Build_index_tuple<_Num - 1>::__type::__next __type;
    };

    template<>
    struct _Build_index_tuple<0>
    {
        typedef _Index_tuple<> __type;
    };

    template <size_t N>
    struct arg
    {
        constexpr arg()
        {}

        template <typename A0, typename ... Args>
        auto operator()(A0 const&, Args const& ... args) const
        {
            constexpr const arg<N - 1> next;
            return next(args...);
        }
    };

    template <>
    struct arg<0>
    {
        constexpr arg()
        {}

        template <typename A0, typename ... Args>
        A0 operator()(A0 const& a0, Args const& ...) const
        {
            return a0;
        }
    };

    template <typename T>
    struct const_
    {
        const_(T val)
            : val(std::move(val))
        {}

        template <typename ... Args>
        T operator()(Args const& ...) const
        {
            return val;
        }

    private:
        T val;
    };

    template <typename P>
    struct holder
    {
        typedef const_<P> type;
    };

    template <size_t N>
    struct holder<arg<N>>
    {
        typedef arg<N> type;
    };

    template <typename F, typename ... P>
    struct bind_t;

    template <typename F, typename ... P>
    struct holder<bind_t<F, P...>>
    {
        typedef bind_t<F, P...> type;
    };

    template <typename F, typename ... P>
    struct bind_t
    {
        bind_t(F func, P&& ... p)
            : func(std::move(func))
            , p(std::forward<P>(p)...)
        {}

        template <typename ... Args>
        auto operator()(Args const& ... args) const
        {
            constexpr typename _Build_index_tuple<sizeof...(P)>::__type indices;
            return call(indices, args...);
        }

    private:
        template <typename ... Args, size_t ... Indices>
        auto call(_Index_tuple<Indices...>, Args const& ... args) const
        {
            return func(std::get<Indices>(p)(args...)...);
        }

    private:
        F func;
        std::tuple<typename holder<P>::type...> p;
    };

    template <typename F, typename P0, typename P1>
    bind_t<F, P0, P1> bind(F func, P0 p0, P1 p1)
    {
        return bind_t<F, P0, P1>(std::move(func), std::move(p0), std::move(p1));
    }

    constexpr arg<0> _1;
    constexpr arg<1> _2;
}

int sum(int a, int b)
{
    return a + b;
}

int main()
{
    using namespace bind_test;

    std::cout << (bind(&sum, 5, -5))() << std::endl;
    std::cout << (bind(&sum, _1, 5))(-5) << std::endl;
    std::cout << (bind(&sum, _1, _1))(0) << std::endl;
    std::cout << (bind(&sum, _2, _2))(10, 0) << std::endl;
    std::cout << (bind(&sum, _2, _1))(5, -5) << std::endl;
    std::cout << (bind(&sum, bind(&sum, _2, 10), _1))(5, -15) << std::endl;
    return 0;
}

