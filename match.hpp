#pragma once
#include <stdexcept>
#include <tuple>
#include <type_traits>

namespace zoid {
    class bad_match : public std::runtime_error {
    public:
        bad_match(char const* msg) : std::runtime_error(msg) {}
        bad_match(std::string const& msg) : std::runtime_error(msg) {}
    };

    template<class T, class F>
    class predicate {
    public:
        explicit predicate(T const& t, F const& f = F())
        : value(t)
        , pred(f) {}

        bool match(T const& value2) {
            return pred(value, value2);
        }

        template<class U>
        predicate<T ,F>& operator=(U const&) {
            return *this;
        }

    private:
        T value;
        F pred;
    };

    namespace _internal {
        template<class T>
        struct is_predicate : std::integral_constant<bool, false> { };

        template<class T, class F>
        struct is_predicate<predicate<T, F>> : std::integral_constant<bool, true> { };

        template<class... Ts>
        class matcher {
        public:
            template<class... Us>
            matcher(Us&&... argz) : args{argz...} { }

            template<class U, class... Us>
            matcher<Ts...>& operator=(std::tuple<U, Us...> tup) {
                match<0>(tup);
                args = tup;
                return *this;
            }

        private:
            std::tuple<Ts&...> args;

            template<std::size_t I, class... Us>
            typename std::enable_if<I == sizeof...(Us) - 1>::type
            match(std::tuple<Us...> const&) { }

            template<std::size_t I, class... Us>
            typename std::enable_if<I < sizeof...(Us) - 1 && is_predicate<typename std::remove_reference<decltype(std::get<I>(args))>::type>::value>::type
            match(std::tuple<Us...> const& tup) {
                if (!std::get<I>(args).match(std::get<I>(tup))) throw bad_match{"bad_match"};
                match<I + 1>(tup);
            }

            template<std::size_t I, class... Us>
            typename std::enable_if<I < sizeof...(Us) - 1 && !is_predicate<typename std::remove_reference<decltype(std::get<I>(args))>::type>::value>::type
            match(std::tuple<Us...> const& tup) {
                match<I + 1>(tup);
            }
        };
    }

    template<class... Ts>
    _internal::matcher<Ts...> match_tie(Ts&&... ts) {
        return _internal::matcher<Ts...>{ts...};
    }
}
