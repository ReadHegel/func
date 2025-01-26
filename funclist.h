#ifndef FUNCLIST_H
#define FUNCLIST_H

#include <algorithm>
#include <functional>
#include <ranges>
#include <sstream>
#include <string>

namespace flist {

namespace detail {

    using str_stream_ref = std::reference_wrapper<std::ostringstream>;

    // Helper lambda for appending a value to a stream with a semicolon
    // in as_string function.
    const auto stream_append_with_semicolon =
        [](auto x, str_stream_ref os) -> str_stream_ref {
        os.get() << x << ";";
        return os;
    };

}  // namespace detail

// Lambda returning an empty list.
const auto empty = []([[maybe_unused]] auto f, auto a) {
    return a;
};

// Lambda returning a list with x appended to l.
const auto cons = [](auto x, auto l) {
    return [x, l](auto f, auto a) {
        return f(x, l(f, a));
    };
};

// // Lambda recursively creating a list from arguments.
const auto create = []<typename... Args>(this const auto& self, Args... args) {
    if constexpr (sizeof...(args) == 0) {
        return empty;
    }
    else {
        return [self]<typename T, typename... Rest>(T t, Rest... rest) {
            return cons(t, self(rest...));
        }(args...);
    }
};

// Lambda returning a list from a range.
const auto of_range = [](auto r) {
    return [r](auto f, auto a) {
        const auto reference = std::cref(r);

        // Reverse iterator range.
        const auto rbegin = std::ranges::rbegin(reference.get());
        const auto rend = std::ranges::rend(reference.get());

        using A = decltype(a);

        // Recursive lambda just evaluating f on list elements.
        const auto call_recursive = [](this const auto& self, auto beg,
                                       auto end, auto f, auto a) -> A {
            return (beg == end) ? a : self(std::next(beg), end, f, f(*beg, a));
        };

        return call_recursive(rbegin, rend, f, a);
    };
};

// Lambda returning a list that is a concatenation of l and k.
const auto concat = [](auto l, auto k) {
    return [l, k](auto f, auto a) {
        return l(f, k(f, a));
    };
};

// Lambda returning a list that is a reverse of l.
const auto rev = [](auto l) {
    return [l](auto f, auto a) {
        using A = decltype(a);
        using ACC = std::function<A(A)>;

        // We are constructing function(A) -> A st. evaluated on 'a' will
        // calculate evaluation of 'f' on list 'l', but from the back.

        // Initial accumulator function.
        const ACC accumulator = [](A acc) {
            return acc;
        };

        // Function 'f' but insted of returning a value it updates 'accumulator'
        const auto create_accumulator = [f](auto x, auto acc) -> ACC {
            return [f, x, acc](auto acc_value) {
                return acc(f(x, acc_value));
            };
        };

        // l(create_accumulator, accumulator) will return a function that
        // evaluated on 'a' will return the result of 'f' evaluated on 'l' from
        // the back.
        return l(create_accumulator, accumulator)(a);
    };
};

const auto map = [](auto m, auto l) {
    return [m, l](auto f, auto a) {
        const auto mapped_f = [m, f](auto x, auto a) {
            return f(m(x), a);
        };

        return l(mapped_f, a);
    };
};

const auto filter = [](auto p, auto l) {
    return [l, p](auto f, auto a) {
        const auto filtered_f = [p, f](auto x, auto acc) {
            return p(x) ? f(x, acc) : acc;
        };

        return l(filtered_f, a);
    };
};

const auto flatten = [](auto l) {
    return l(concat, empty);
};

const auto as_string = [](const auto& l) -> std::string {
    std::ostringstream acc;

    // Append all list elements to the accumulator. And then get the string.
    std::string result =
        rev(l)(detail::stream_append_with_semicolon, std::ref(acc)).get().str();

    // Remove the last semicolon.
    if (!result.empty()) {
        result.pop_back();
    }

    return "[" + result + "]";
};

}  // namespace flist

#endif  // FUNCLIST_H