#ifndef FUNCLIST_H
#define FUNCLIST_H

#include <algorithm>
#include <functional>
#include <ranges>
#include <sstream>
#include <string>

namespace flist {

namespace detail {}  // namespace detail

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

// Lambda recursively creating a list from arguments.
const auto create = []<typename T, typename... Args>(this const auto& self, T t,
                                                     Args... args) {
    if constexpr (sizeof...(args) != 0) {
        return cons(t, self(args...));
    }
    else {
        return cons(t, empty);
    }
};

// Lambda returning a list from a range.
const auto of_range = []<typename Con>(Con r) {
    return [r](auto f, auto a) {
        auto reference = std::cref(r);

        auto begin = std::ranges::begin(reference.get());
        auto end = std::ranges::end(reference.get());

        return std::ranges::fold_right(begin, end, a, f);
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
        ACC accumulator = [](A acc) {
            return acc;
        };

        // Function 'f' but insted of returning a value it updates 'accumulator'
        auto create_accumulator = [f](auto x, auto acc) -> ACC {
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
    // Lambda returning a composition of 'm' and 'f'.
    auto get_mapped_f = [m](auto f) {
        return [m, f](auto x, auto a) {
            return f(m(x), a);
        };
    };

    return [get_mapped_f, l](auto f, auto a) {
        return l(get_mapped_f(f), a);
    };
};

const auto filter = [](auto p, auto l) {
    return [l, p](auto f, auto a) {
        auto filtered_f = [p, f](auto x, auto acc) {
            return p(x) ? f(x, acc) : acc;
        };

        return l(filtered_f, a);
    };
};

const auto flatten = [](auto l) {
    return l(concat, empty);
};

const auto as_string = [](const auto& l) -> std::string {
    // Accumulator lambda.
    auto acc = [](auto x, auto os) {
        os.get() << x << ";";
        return os;
    };

    std::ostringstream oss;
    oss << "[";

    // Append all list elements to the accumulator. And then get the string.
    auto result = rev(l)(acc, std::ref(oss)).get().str();

    // Remove the last semicolon.
    if (result.size() != 1) {
        result.pop_back();
    }

    return result += "]";
};

}  // namespace flist

#endif  // FUNCLIST_H