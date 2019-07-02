#pragma once

#include <memory>
#include <functional>
#include <string>
#include <optional>
#include <any>
#include <tuple>
#include <type_traits>
#include <vector>


template <typename T>
using UPtr = std::unique_ptr<T>;

template <typename T>
using UPtrV = std::vector<std::unique_ptr<T>>;

template <typename T>
using FuncT = std::function<T>;

template <typename T>
using OptT = std::optional<T>;

template <typename ...T>
using TupleT = std::tuple<T...>;

template <typename T>
OptT<T> GetAny(const std::any & any) {
    try {
        return OptT<T>(std::any_cast<T>(any));
    }
    catch(std::bad_any_cast &e) {
    }

    return OptT<T>();
}

template <typename T>
auto UnderlyingT(T val)
{
    static_assert(std::is_enum<T>::value, "UnderlyingT requires an enum argument");
    return static_cast<typename std::underlying_type<T>::type>(val);
}

template <class T> constexpr std::string_view type_name()
{
    using namespace std;
#ifdef __clang__
    string_view p = __PRETTY_FUNCTION__;
    return string_view(p.data() + 34, p.size() - 34 - 1);
#elif defined(__GNUC__)
    string_view p = __PRETTY_FUNCTION__;
#if __cplusplus < 201402
    return string_view(p.data() + 36, p.size() - 36 - 1);
#else
    return string_view(p.data() + 49, p.find(';', 49) - 49);
#endif
#elif defined(_MSC_VER)
    string_view p = __FUNCSIG__;
    return string_view(p.data() + 84, p.size() - 84 - 7);
#endif
}

template<typename T> const T& constant(T& _) { return const_cast<const T&>(_); }
template<typename T> T& variable(const T& _) { return const_cast<T&>(_); }

template <typename T> int sign(T val) {
    return (val > T(0)) - (val < T(0));
}