#pragma once
#include <cstddef>
#include <cstdint>

namespace obff_internal {


constexpr uint8_t xor_key(std::size_t seed) {
    return static_cast<uint8_t>((seed * 31 + 97) % 256);
}

template <std::size_t N, std::size_t Seed>
class XorString {
private:
    char data[N];
    const uint8_t key;

public:
    constexpr XorString(const char (&input)[N])
        : data{}, key(xor_key(Seed)) {
        for (std::size_t i = 0; i < N; ++i)
            data[i] = input[i] ^ key;
    }

    char* decrypt() {
        for (std::size_t i = 0; i < N; ++i)
            data[i] ^= key;
        return data;
    }

    void zeroize() {
        for (std::size_t i = 0; i < N; ++i)
            data[i] = 0;
    }
};

template <std::size_t N, std::size_t Seed>
class XorWString {
private:
    wchar_t data[N];
    const uint8_t key;

public:
    constexpr XorWString(const wchar_t (&input)[N])
        : data{}, key(xor_key(Seed)) {
        for (std::size_t i = 0; i < N; ++i)
            data[i] = input[i] ^ key;
    }

    wchar_t* decrypt() {
        for (std::size_t i = 0; i < N; ++i)
            data[i] ^= key;
        return data;
    }

    void zeroize() {
        for (std::size_t i = 0; i < N; ++i)
            data[i] = 0;
    }
};

} 
#define __OBF_UNIQUE_ID_IMPL(lineno) _obf_##lineno
#define __OBF_UNIQUE_ID(lineno) __OBF_UNIQUE_ID_IMPL(lineno)
#define obfuscate(str) []() { \
    static constexpr auto __OBF_UNIQUE_ID(__LINE__) = \
        obff_internal::XorString<sizeof(str), __LINE__>(str); \
    static auto val = __OBF_UNIQUE_ID(__LINE__); \
    return val.decrypt(); \
}()


#define obfuscate_w(str) []() { \
    static constexpr auto __OBF_UNIQUE_ID(__LINE__) = \
        obff_internal::XorWString<sizeof(str) / sizeof(wchar_t), __LINE__>(str); \
    static auto val = __OBF_UNIQUE_ID(__LINE__); \
    return val.decrypt(); \
}()
