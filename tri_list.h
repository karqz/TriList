#ifndef TRI_LIST_H
#define TRI_LIST_H

#include <functional>
#include <ranges>
#include <vector>
#include <tuple>
#include "tri_list_concepts.h"

template <typename T>
auto identity = [](T x){return x;};

template <typename T, modifier<T> F, modifier<T> G>
auto compose(F f, G g) {
    return [=](T x){return f(g(x));};
};


template <typename T1, typename T2, typename T3>
class tri_list {
public:
    std::vector<std::variant<T1, T2, T3>> list;
    std::tuple<std::function<T1(T1)>,std::function<T2(T2)>,std::function<T3(T3)>> modifiers;

    class iterator {
        public:
            using iterator_category = std::bidirectional_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = std::variant<T1, T2, T3>;
            using pointer = value_type*;
            using reference = const value_type&;

            iterator() = default;
            iterator(std::vector<value_type>::iterator _ptr, const std::tuple<std::function<T1(T1)>,std::function<T2(T2)>,std::function<T3(T3)>> &_modifiers):ptr(_ptr),modifiers(_modifiers) {};

            value_type operator*() const {
                return std::visit([&](auto &&v) -> value_type {
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::same_as<T, T1>) {
                        return std::invoke(std::get<0>(modifiers), v);
                    }
                    else if constexpr(std::same_as<T, T2>) {
                        return std::invoke(std::get<1>(modifiers), v);
                    }    
                    else {
                        return std::invoke(std::get<2>(modifiers), v);
                    } 
                }, *ptr);
            }

            iterator& operator++() {
                ptr++;
                return *this;
            };

            iterator operator++(int) {
                iterator tmp = *this;
                ptr++;
                return tmp;
            };
            
            iterator& operator--() {
                ptr--;
                return *this;
            };

            iterator operator--(int) {
                iterator tmp = *this;
                ptr--;
                return tmp;
            };

            friend bool operator==(const iterator& that, const iterator& other) {
                return that.ptr == other.ptr;
            };

            friend bool operator!=(const iterator& that, const iterator& other) {
                return that.ptr != other.ptr;
            };
            
        private:
            std::vector<value_type>::iterator ptr;
            std::tuple<std::function<T1(T1)>,std::function<T2(T2)>,std::function<T3(T3)>> modifiers;

    };

public:
    tri_list() = default;

    tri_list(std::initializer_list<std::variant<T1, T2, T3>> _list):list(_list), modifiers({identity<T1>, identity<T2>, identity<T3>}) {};

    template <typename T> void push_back(const T& t) {
        list.push_back(t);
    };

    template <typename T, modifier<T> F> void modify_only(F m = F{}) {
        auto modifier = std::get<std::function<T(T)>>(modifiers);
        std::get<std::function<T(T)>>(modifiers) = compose<T>(m, modifier);
    };

    template <typename T> void reset() {
        std::get<std::function<T(T)>>(modifiers) = identity<T>;
    };

    template <typename T> auto range_over() const {
        auto result = list 
                    | std::views::filter([](auto x){return std::holds_alternative<T>(x);})
                    | std::views::transform([](auto x){return std::get<T>(x);})
                    | std::views::transform(std::get<std::function<T(T)>>(modifiers));
        return result;
    };

    auto begin() {
        return iterator(list.begin(), modifiers);
    };

    auto end() {
        return iterator(list.end(), modifiers);
    };
};

#endif //TRI_LIST_H