#pragma once

#include <type_traits>
struct nullopt_t {};

inline constexpr nullopt_t nullopt = {};

struct in_place_t {};

inline constexpr in_place_t in_place = {};

struct dummy_t {};

template<typename T, bool is_trivial>
struct optional_destructor_base {

  constexpr optional_destructor_base() noexcept : has_value(false), dummy() {}

  constexpr optional_destructor_base(T value_) : has_value(true), value(std::move(value_)) {}

  template<typename... Args>
  constexpr optional_destructor_base(in_place_t, Args&&... args): has_value(true), value(std::forward<Args>(args)...) {}

  ~optional_destructor_base() {
    if (has_value) {
      value.~T();
    }
  }

  bool has_value;
  union {
    T value;
    dummy_t dummy;
  };
};

template<typename T>
struct optional_destructor_base<T, true> {
  constexpr optional_destructor_base() noexcept : has_value(false), dummy() {}

  constexpr optional_destructor_base(T value_) noexcept(std::is_nothrow_move_constructible_v<T>) : has_value{true}, value(std::move(value_)) {}

  template<typename... Args>
  constexpr optional_destructor_base(in_place_t, Args&&... args): has_value(true), value(std::forward<Args>(args)...) {}

  ~optional_destructor_base() = default;

  bool has_value;
  union {
    T value;
    dummy_t dummy;
  };
};


template<typename T, bool is_trivial>
struct optional_move_base : optional_destructor_base<T, std::is_trivially_destructible_v<T>> {
  using base = optional_destructor_base<T, std::is_trivially_destructible_v<T>>;

  using base::base;

  constexpr optional_move_base(optional_move_base&& other) noexcept(std::is_nothrow_move_constructible_v<T>) : base() {
    if (other.has_value) {
      this->has_value = true;
      new(&this->value) T(std::move(other.value));
    }
  }
};

template<typename T>
struct optional_move_base<T, true> : optional_destructor_base<T, std::is_trivially_destructible_v<T>> {
  using base = optional_destructor_base<T, std::is_trivially_destructible_v<T>>;

  using base::base;

  constexpr optional_move_base(optional_move_base&&) noexcept(std::is_nothrow_move_constructible_v<T>) = default;
};

template<typename T, bool is_trivial>
struct optional_copy_base : optional_move_base<T, std::is_trivially_move_constructible_v<T>> {
  using base = optional_move_base<T, std::is_trivially_move_constructible_v<T>>;

  using base::base;

  constexpr optional_copy_base(optional_copy_base&& other) noexcept(std::is_nothrow_move_constructible_v<T>) = default;

  constexpr optional_copy_base(optional_copy_base const& other) noexcept(std::is_nothrow_copy_constructible_v<T>) : base() {
    if (other.has_value) {
      this->has_value = true;
      new(&this->value) T(other.value);
    }
  }
};

template<typename T>
struct optional_copy_base<T, true> : optional_move_base<T, std::is_trivially_move_constructible_v<T>> {
  using base = optional_move_base<T, std::is_trivially_move_constructible_v<T>>;

  using base::base;

  constexpr optional_copy_base(optional_copy_base&& other) noexcept(std::is_nothrow_move_constructible_v<T>) = default;

  constexpr optional_copy_base(optional_copy_base const& other) noexcept(std::is_nothrow_copy_constructible_v<T>) = default;
};

template<typename T, bool is_trivial>
struct optional_move_assign_base : optional_copy_base<T, std::is_trivially_copy_constructible_v<T>> {
  using base = optional_copy_base<T, std::is_trivially_copy_constructible_v<T>>;

  using base::base;

  optional_move_assign_base(optional_move_assign_base const&) noexcept(std::is_nothrow_copy_constructible_v<T>) = default;

  optional_move_assign_base(optional_move_assign_base&&) noexcept(std::is_nothrow_move_constructible_v<T>) = default;

  constexpr optional_move_assign_base& operator=(optional_move_assign_base&& other)
      noexcept(std::is_nothrow_move_assignable_v<T> && std::is_nothrow_move_constructible_v<T>) {
    if (this != &other) {
      if (this->has_value && !other.has_value) {
        this->value.~T();
      } else if (this->has_value && other.has_value) {
        this->value = std::move(other.value);
      } else if (other.has_value) {
        new(&this->value) T(std::move(other.value));
      }
      this->has_value = other.has_value;
    }
    return *this;
  }
};

template<typename T>
struct optional_move_assign_base<T, true> : optional_copy_base<T, std::is_trivially_copy_constructible_v<T>> {
  using base = optional_copy_base<T, std::is_trivially_copy_constructible_v<T>>;

  using base::base;

  optional_move_assign_base(optional_move_assign_base const&) noexcept(std::is_nothrow_copy_constructible_v<T>) = default;

  optional_move_assign_base(optional_move_assign_base&&) noexcept(std::is_nothrow_move_constructible_v<T>) = default;

  constexpr optional_move_assign_base& operator=(optional_move_assign_base&& other)
      noexcept(std::is_nothrow_move_assignable_v<T> && std::is_nothrow_move_constructible_v<T>) = default;
};

template<typename T, bool is_trivial>
struct optional_copy_assign_base : optional_move_assign_base<T, std::is_trivially_move_assignable_v<T>> {
  using base = optional_move_assign_base<T, std::is_trivially_move_assignable_v<T>>;

  using base::base;

  optional_copy_assign_base(optional_copy_assign_base const&) noexcept(std::is_nothrow_copy_constructible_v<T>) = default;

  optional_copy_assign_base(optional_copy_assign_base&&) noexcept(std::is_nothrow_move_constructible_v<T>) = default;

  optional_copy_assign_base& operator=(optional_copy_assign_base&&)
      noexcept(std::is_nothrow_move_assignable_v<T> && std::is_nothrow_move_constructible_v<T>) = default;

  constexpr optional_copy_assign_base& operator=(optional_copy_assign_base const& other)
      noexcept(std::is_nothrow_copy_assignable_v<T> && std::is_nothrow_copy_constructible_v<T>) {
    if (this != &other) {
      if (this->has_value && !other.has_value) {
        this->value.~T();
      } else if (this->has_value && other.has_value) {
        this->value = other.value;
      } else if (other.has_value) {
        new(&this->value) T(other.value);
      }
      this->has_value = other.has_value;
    }
    return *this;
  }
};

template<typename T>
struct optional_copy_assign_base<T, true> : optional_move_assign_base<T, std::is_trivially_move_assignable_v<T>> {
  using base = optional_move_assign_base<T, std::is_trivially_move_assignable_v<T>>;

  using base::base;

  optional_copy_assign_base(optional_copy_assign_base const&) noexcept(std::is_nothrow_copy_constructible_v<T>) = default;

  optional_copy_assign_base(optional_copy_assign_base&&) noexcept(std::is_nothrow_move_constructible_v<T>) = default;

  optional_copy_assign_base& operator=(optional_copy_assign_base&&)
      noexcept(std::is_nothrow_move_assignable_v<T> && std::is_nothrow_move_constructible_v<T>) = default;

  constexpr optional_copy_assign_base& operator=(optional_copy_assign_base const&)
      noexcept(std::is_nothrow_copy_assignable_v<T> && std::is_nothrow_copy_constructible_v<T>) = default;
};

template <typename T>
struct optional : optional_copy_assign_base<T, std::is_trivially_copy_assignable_v<T>> {
  using base = optional_copy_assign_base<T, std::is_trivially_copy_assignable_v<T>>;

  using base::base;

  constexpr optional(nullopt_t) noexcept : base() {}

  constexpr optional(optional const& other) noexcept(std::is_nothrow_copy_constructible_v<T>) = default;

  constexpr optional(optional&& other) noexcept(std::is_nothrow_move_constructible_v<T>) = default;

  optional& operator=(optional const& other)
      noexcept(std::is_nothrow_copy_assignable_v<T> && std::is_nothrow_copy_constructible_v<T>) = default;

  optional& operator=(optional&& other)
      noexcept(std::is_nothrow_move_assignable_v<T> && std::is_nothrow_move_constructible_v<T>) = default;

  optional& operator=(nullopt_t) noexcept {
    reset();
    return *this;
  }

  constexpr explicit operator bool() const noexcept {
    return this->has_value;
  }

  constexpr T& operator*() noexcept {
    return this->value;
  }

  constexpr T const& operator*() const noexcept {
    return this->value;
  }

  constexpr T* operator->() noexcept {
    return &this->value;
  }

  constexpr T const* operator->() const noexcept {
    return &this->value;
  }

  template <typename... Args>
  void emplace(Args&&... args) {
    reset();
    new(&this->value) T(std::forward<Args>(args)...);
    this->has_value = true;
  }

  void reset() {
    if (this->has_value) {
      this->value.~T();
    }
    this->has_value = false;
  }
};

template<typename T>
constexpr bool operator==(optional<T> const &a, optional<T> const &b) {
  return !(a.has_value || b.has_value)
             ? true
             : a.has_value ? (b.has_value ? a.value == b.value : false) : false;
}

template<typename T>
constexpr bool operator!=(optional<T> const &a, optional<T> const &b) {
  return !(a == b);
}

template<typename T>
constexpr bool operator<(optional<T> const &a, optional<T> const &b) {
  return !(a.has_value || b.has_value)
             ? false
             : a.has_value ? (b.has_value ? a.value < b.value : false) : true;
}

template<typename T>
constexpr bool operator<=(optional<T> const &a, optional<T> const &b) {
  return !(a > b);
}

template<typename T>
constexpr bool operator>(optional<T> const &a, optional<T> const &b) {
  return (b < a);
}

template<typename T>
constexpr bool operator>=(optional<T> const &a, optional<T> const &b) {
  return !(a < b);
}
