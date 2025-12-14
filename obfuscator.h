#pragma once

#include <cstddef>
#include <cstdint>
#include <array>

namespace obff_internal {

constexpr uint8_t simple_hash(std::size_t seed) noexcept {
    seed ^= seed >> 12;
    seed ^= seed >> 25;
    seed ^= seed >> 27;
    return static_cast<uint8_t>(seed * 31 + 97);
}

template <typename CharT, std::size_t N, std::size_t Seed>
struct XorStringBase {
protected:
    std::array<CharT, N> data;
    const uint8_t key;
    bool decrypted = false;

    constexpr XorStringBase(const CharT (&input)[N], uint8_t k)
        : key(k) {
        for (std::size_t i = 0; i < N; ++i)
            data[i] = input[i] ^ static_cast<CharT>(key);
    }

public:
    CharT* decrypt() noexcept {
        if (!decrypted) {
            for (std::size_t i = 0; i < N; ++i)
                data[i] ^= static_cast<CharT>(key);
            decrypted = true;
        }
        return data.data();
    }

    void zeroize() noexcept {
        for (std::size_t i = 0; i < N; ++i) {
            volatile CharT* p = &data[i];
            *p = CharT(0);
        }
    }
};

template <std::size_t N, std::size_t Seed = __LINE__>
class XorString : public XorStringBase<char, N, Seed> {
    using Base = XorStringBase<char, N, Seed>;
public:
    constexpr XorString(const char (&input)[N])
        : Base(input, simple_hash(Seed)) {}
};

template <std::size_t N, std::size_t Seed = __LINE__>
class XorWString : public XorStringBase<wchar_t, N, Seed> {
    using Base = XorStringBase<wchar_t, N, Seed>;
public:
    constexpr XorWString(const wchar_t (&input)[N])
        : Base(input, simple_hash(Seed)) {}
};

template <typename XorStr>
class AutoZero {
    XorStr& str;
public:
    explicit AutoZero(XorStr& s) : str(s) {}
    ~AutoZero() { str.zeroize(); }
    typename XorStr::CharT* get() const noexcept { return str.decrypt(); }
};

}

#define OBF(str) []() -> const char* { \
    static obff_internal::XorString<sizeof(str)> obfuscated(str); \
    return obfuscated.decrypt(); \
}()

#define OBF_W(str) []() -> const wchar_t* { \
    static obff_internal::XorWString<sizeof(str)/sizeof(wchar_t)> obfuscated(str); \
    return obfuscated.decrypt(); \
}()

#define OBF_AUTO(str) []() { \
    static obff_internal::XorString<sizeof(str)> obfuscated(str); \
    static obff_internal::AutoZero<decltype(obfuscated)> guard(obfuscated); \
    return guard.get(); \
}()
