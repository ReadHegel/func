#ifndef FUNCLIST_H
#define FUNCLIST_H

#include <functional>
#include <sstream>
#include <string>

namespace flist {

namespace detail {

template <typename F, typename A>
using L = std::function<A(F, A)>;

// const auto of_range_creator = []<>(this const auto& self, T t,
//                                                Args... args) {
//     if constexpr (sizeof...(args) != 0) {
//         return cons(t, self(args...));
//     }
//     else {
//         return cons(t, empty);
//     }
// };

}  // namespace detail

const auto empty = []([[maybe_unused]] auto f, auto a) {
    return a;
};

const auto cons = [](auto x, auto l) {
    return [x, l](auto f, auto a) {
        return f(x, l(f, a));
    };
};

// TODO upiększyć
const auto rev = [](auto l) {
    return [l](auto f, auto a) {
        using A = decltype(a);
        using ACC = std::function<A(A)>;

        ACC acc = [](A aa) {
            return aa;
        };

        auto func = [f](auto x, auto acc) -> ACC {
            return [f, x, acc](auto aa) {
                return acc(f(x, aa));
            };
        };

        return l(func, acc)(a);
    };
};

const auto as_string = [](const auto& l) -> std::string {
    // auto acc = [](auto x, std::reference_wrapper<std::ostringstream> os)
    //     -> std::reference_wrapper<std::ostringstream> {
    //     os.get() << x << ";";
    //     return os;
    // };

    auto acc = [](auto x, auto os) {
        os.get() << x << ";";
        return os;
    };

    std::ostringstream oss;
    oss << "[";

    auto result = rev(l)(acc, std::ref(oss)).get().str();

    if (result.size() != 1) {
        result.pop_back();
    }

    return result += "]";
};

const auto create = []<typename T, typename... Args>(this const auto& self, T t,
                                                     Args... args) {
    if constexpr (sizeof...(args) != 0) {
        return cons(t, self(args...));
    }
    else {
        return cons(t, empty);
    }
};

const auto concat = [](auto l, auto k) {
    return [l, k](auto f, auto a) {
        return l(f, k(f, a));
    };
};

// const auto of_range_creator = [](this const auto& self, auto it, auto end,
//                                  auto l) {
//     auto it_new = ++it;
//     if constexpr (it_new == end) {
//         return l;
//     }
//     return self(it_new, end, cons(*(it_new), l));
// };

const auto of_range = [](auto r) { 
    return [r](auto f, auto a) {
        auto&& container = [&]() -> decltype(auto) {
            if constexpr (std::is_same_v<
                              std::remove_cvref_t<decltype(r)>,
                              std::reference_wrapper<
                                  std::ranges::range_value_t<decltype(r)>>>) {
                return r.get();
            }
            else {
                return r;
            }
        }();

        for (auto it = std::ranges::begin(container);
             it != std::ranges::end(container); ++it) {
            a = f(*it, a);
        }
        return a;
    };
    // return [r](auto f, auto a) {
    //     for (auto it = r.begin(); it != r.end(); it++) {
    //         a = f(*it, a);
    //     }
    //     return a;
    // };
};

// auto of_range(auto r) {
//     auto create_from_range = [](this const auto& self, auto begin, auto end)
//     {
//         if (begin == end) {
//             return empty;
//         }
//         auto prev = end;
//         --prev;
//         return cons(*prev, self(begin, prev));
//     };

//     return create_from_range(std::ranges::begin(r), std::ranges::end(r));
// }

const auto map = [](auto m, auto l) {
    auto get_new_f = [m](auto old_f) {
        return [m, old_f](auto x, auto a) {
            return old_f(m(x), a);
        };
    };

    return [get_new_f, l](auto f, auto a) {
        return l(get_new_f(f), a);
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

}  // namespace flist

#endif  // FUNCLIST_H