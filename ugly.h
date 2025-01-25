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
        return std::ranges::fold_right(std::ranges::begin(std::cref(r).get()),
                                       std::ranges::end(std::cref(r).get()), a,
                                       f);
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

        return l(
            [f](auto x, auto acc) -> ACC {
                return [f, x, acc](auto acc_value) {
                    return acc(f(x, acc_value));
                };
            },
            [](A acc) {
                return acc;
            })(a);
    };
};

const auto map = [](auto m, auto l) {
    return [m, l](auto f, auto a) {
        return l(
            [m](auto f) {
                return [m, f](auto x, auto a) {
                    return f(m(x), a);
                };
            }(f),
            a);
    };
};

const auto filter = [](auto p, auto l) {
    return [l, p](auto f, auto a) {
        return l(
            [p, f](auto x, auto acc) {
                return p(x) ? f(x, acc) : acc;
            },
            a);
    };
};

const auto flatten = [](auto l) {
    return l(concat, empty);
};

const auto as_string = [](const auto& l) -> std::string {
    std::ostringstream oss.str('[');
    auto result = rev(l)(
                      [](auto x, auto os) {
                          os.get() << x << ";";
                          return os;
                      };
                      , std::ref(oss))
                      .get()
                      .str();

    // Remove the last semicolon.
    if (result.size() != 1) {
        result.pop_back();
    }

    return result += "]";
};

}  // namespace flist

#endif  // FUNCLIST_H