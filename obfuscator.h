#pragma once
#include <cstddef>
#include <cstdint>
#include <array>
#include <algorithm>

namespace obff_internal {

constexpr uint64_t mix_seed(uint64_t z) noexcept {
    z ^= z >> 30;
    z *= 0xbf58476d1ce4e5b9ull;
    z ^= z >> 27;
    z *= 0x94d049bb133111ebull;
    z ^= z >> 31;
    return z;
}

template<std::size_t KeyLen = 32>
constexpr auto make_rolling_key(std::size_t seed) noexcept {
    std::array<uint8_t, KeyLen> key{};
    uint64_t z = mix_seed(static_cast<uint64_t>(seed));
    for (std::size_t i = 0; i < KeyLen; ++i) {
        z ^= z >> 13;
        z *= 0xff51afd7ed558ccdull;
        z ^= z >> 33;
        key[i] = static_cast<uint8_t>(z);
        z ^= static_cast<uint64_t>(i) << 32;
    }
    return key;
}


template<typename CharT, std::size_t N, std::size_t Seed>
struct XorStringBase {
    static constexpr std::size_t KeyLen = 32;
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
            for (std::size_t i = 0; i < N; ++i) {
                data[i] ^= static_cast<CharT>(key_stream[i % KeyLen]);
            }
            decrypted = true;
        }
        return data.data();
    }

    const CharT* c_str() const noexcept {
        return decrypted ? data.data() : nullptr;
    }

    void zeroize() noexcept {

        volatile auto* p = data.data();
        std::fill(data.begin(), data.end(), CharT{0});
        __builtin___clear_cache(reinterpret_cast<char*>(p), reinterpret_cast<char*>(p) + sizeof(CharT) * N);
    }

    ~XorStringBase() { zeroize(); }
};


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


#define OBF(str) []() -> const char* { \
    static obff_internal::XorString<sizeof(str)> xs(str); \
    return xs.decrypt(); \
}()

#define OBF_SEED(str, seed) []() -> const char* { \
    static obff_internal::XorString<sizeof(str), (seed)> xs(str); \
    return xs.decrypt(); \
}()

#define OBF_W(str) []() -> const wchar_t* { \
    static obff_internal::XorWString<sizeof(str)/sizeof(wchar_t)> xs(str); \
    return xs.decrypt(); \
}()

#define OBF_W_SEED(str, seed) []() -> const wchar_t* { \
    static obff_internal::XorWString<(sizeof(str)/sizeof(wchar_t)), (seed)> xs(str); \
    return xs.decrypt(); \
}()
}
