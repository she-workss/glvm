#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#if __has_include(<stdfloat>)
#include <stdfloat>
#endif

namespace rusty::types {
using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;
using isize = std::ptrdiff_t;

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using usize = std::size_t;

#ifdef __STDCPP_FLOAT16_T__
using f16 = std::float16_t;
#endif

// float/double on purpose: std::float32_t/float64_t can be distinct extended
// types on some targets (e.g. MinGW), which breaks plain float overloads.
using f32 = float;
using f64 = double;

#ifdef __STDCPP_FLOAT128_T__
using f128 = std::float128_t;
#endif

using str = std::string_view;
using String = std::string;

template<typename T>
using Vec = std::vector<T>;

template<typename T>
using Slice = std::span<T>;

template<typename T, usize N>
using Array = std::array<T, N>;

template<typename T>
using Box = std::unique_ptr<T>;

template<typename T>
using Rc = std::shared_ptr<T>;

template<typename T>
using Weak = std::weak_ptr<T>;

template<typename K, typename V, typename Hash = std::hash<K>>
using HashMap = std::unordered_map<K, V, Hash>;

template<typename K, typename Hash = std::hash<K>>
using HashSet = std::unordered_set<K, Hash>;

template<typename K, typename V>
using BTreeMap = std::map<K, V>;

template<typename K>
using BTreeSet = std::set<K>;

using byte = std::byte;

inline constexpr auto None = std::nullopt;

template<typename... Ts>
using Enum = std::variant<Ts...>;

using Unit = std::monostate;

using Mutex = std::mutex;
template<typename Mutex>
using MutexGuard = std::lock_guard<Mutex>;

using AtomicBool = std::atomic_bool;
using AtomicI8 = std::atomic_int8_t;
using AtomicI16 = std::atomic_int16_t;
using AtomicI32 = std::atomic_int32_t;
using AtomicI64 = std::atomic_int64_t;
using AtomicIsize = std::atomic_ptrdiff_t;

using AtomicU8 = std::atomic_uint8_t;
using AtomicU16 = std::atomic_uint16_t;
using AtomicU32 = std::atomic_uint32_t;
using AtomicU64 = std::atomic_uint64_t;
using AtomicUsize = std::atomic_size_t;

template<typename Tp>
using VecDeque = std::deque<Tp>;
} // namespace rusty::types
