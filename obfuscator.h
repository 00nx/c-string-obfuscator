#pragma once

#include <cstddef>
#include <cstdint>
#include <array>
#include <cstring>      // std::fill
#include <utility>      // std::forward

namespace obff_internal {

// ───────────────────────────────────────────────
//  Better compile-time hash mixer (inspired by splitmix64 / wyhash style)
// ───────────────────────────────────────────────
constexpr uint64_t mix_seed(uint64_t z) noexcept {
    z ^= z >> 30;
    z *= 0xbf58476d1ce4e5b9ull;
    z ^= z >> 27;
    z *= 0x94d049bb133111ebull;
    z ^= z >> 31;
    return z;
}

constexpr uint8_t derive_key(std::size_t seed) noexcept {
    uint64_t mixed = mix_seed(static_cast<uint64_t>(seed));
    return static_cast<uint8_t>(mixed ^ (mixed >> 56));
}

// ───────────────────────────────────────────────
//  Generate small rolling key stream from seed
// ───────────────────────────────────────────────
template<std::size_t KeyLen = 16>
constexpr auto make_rolling_key(std::size_t seed) noexcept {
    std::array<uint8_t, KeyLen> key{};
    uint64_t z = mix_seed(seed);

    for (std::size_t i = 0; i < KeyLen; ++i) {
        z ^= z >> 13;
        z *= 0xff51afd7ed558ccdull;
        z ^= z >> 33;
        key[i] = static_cast<uint8_t>(z);
    }
    return key;
}

// ───────────────────────────────────────────────
//  Core obfuscated string storage (char / wchar_t agnostic)
// ───────────────────────────────────────────────
template<typename CharT, std::size_t N, std::size_t Seed>
struct XorStringBase {
    static constexpr std::size_t KeyLen = 16;

    alignas(16) std::array<CharT, N> data{};
    bool decrypted = false;

    static constexpr auto key_stream = make_rolling_key<KeyLen>(Seed);

    constexpr XorStringBase(const CharT (&input)[N]) {
        for (std::size_t i = 0; i < N; ++i) {
            data[i] = input[i] ^ static_cast<CharT>(key_stream[i % KeyLen]);
        }
    }

    CharT* decrypt() noexcept {
        if (!decrypted) {
            // Main hot path — hopefully gets unrolled / vectorized
            CharT* p = data.data();
            const CharT* end = p + N;

            // We can hint compiler that N is small/medium in many cases
            while (p < end) {
                std::size_t idx = static_cast<std::size_t>(p - data.data());
                *p++ ^= static_cast<CharT>(key_stream[idx % KeyLen]);
            }

            decrypted = true;
        }
        return data.data();
    }

    void zeroize() noexcept {
        volatile CharT* p = data.data();
        // Modern compilers understand this pattern better than hand-written loop
        std::fill(data.begin(), data.end(), CharT{0});

        // Extra barrier (helps against aggressive optimizer reordering)
        asm volatile ("" : : "r,m"(p) : "memory");
    }

    ~XorStringBase() { zeroize(); }  // ← added RAII zero on destruction
};

// ───────────────────────────────────────────────
//  char / wchar_t specializations
// ───────────────────────────────────────────────
template<std::size_t N, std::size_t Seed = __LINE__>
struct XorString : XorStringBase<char, N, Seed> {
    using base = XorStringBase<char, N, Seed>;
    constexpr XorString(const char (&s)[N]) : base(s) {}
};

template<std::size_t N, std::size_t Seed = __LINE__>
struct XorWString : XorStringBase<wchar_t, N, Seed> {
    using base = XorStringBase<wchar_t, N, Seed>;
    constexpr XorWString(const wchar_t (&s)[N]) : base(s) {}
};

// ───────────────────────────────────────────────
//  RAII helper – decrypt + auto zero when leaving scope
// ───────────────────────────────────────────────
template<typename XorStr>
class AutoDecryptZero {
    XorStr& str;
public:
    explicit AutoDecryptZero(XorStr& s) noexcept : str(s) {
        str.decrypt();
    }

    ~AutoDecryptZero() {
        str.zeroize();
    }

    auto get() const noexcept { return str.data.data(); }
    operator const typename XorStr::base::CharT*() const noexcept { return get(); }
};

// ───────────────────────────────────────────────
//  Usable macros
// ───────────────────────────────────────────────
#define OBF(str) []() -> const char* {                  \
    static obff_internal::XorString<sizeof(str)> xs(str); \
    return xs.decrypt();                                \
}()

#define OBF_W(str) []() -> const wchar_t* {             \
    static obff_internal::XorWString<(sizeof(str)/sizeof(wchar_t))> xs(str); \
    return xs.decrypt();                                \
}()

#define OBF_AUTO(str) []() -> const char* {             \
    static obff_internal::XorString<sizeof(str)> xs(str); \
    static obff_internal::AutoDecryptZero<decltype(xs)> az(xs); \
    return az.get();                                    \
}()

#define OBF_W_AUTO(str) []() -> const wchar_t* {        \
    static obff_internal::XorWString<(sizeof(str)/sizeof(wchar_t))> xs(str); \
    static obff_internal::AutoDecryptZero<decltype(xs)> az(xs); \
    return az.get();                                    \
}()

} 
