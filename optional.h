#pragma once

struct nullopt_t;

struct in_place_t;

template <typename T>
class optional {
public:
    constexpr optional() noexcept;
    constexpr optional(nullopt_t) noexcept;

    constexpr optional(optional const&);
    constexpr optional(optional&&);

    optional& operator=(optional const&);
    optional& operator=(optional&&);

    constexpr optional(T value);

    template <typename... Args>
    explicit constexpr optional(in_place_t, Args&&... args);

    optional& operator=(nullopt_t) noexcept;

    constexpr explicit operator bool() const noexcept;

    constexpr T& operator*() noexcept;
    constexpr T const& operator*() const noexcept;

    constexpr T* operator->() noexcept;
    constexpr T const* operator->() const noexcept;

    template <typename... Args>
    void emplace(Args&&... args);

    void reset();
};

template<typename T>
constexpr bool operator==(optional<T> const &a, optional<T> const &b);

template<typename T>
constexpr bool operator!=(optional<T> const &a, optional<T> const &b);

template<typename T>
constexpr bool operator<(optional<T> const &a, optional<T> const &b);

template<typename T>
constexpr bool operator<=(optional<T> const &a, optional<T> const &b);

template<typename T>
constexpr bool operator>(optional<T> const &a, optional<T> const &b);

template<typename T>
constexpr bool operator>=(optional<T> const &a, optional<T> const &b);
