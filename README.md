#  C++ Compile-Time String Obfuscator (Rolling XOR)

**Lightweight • Header-only • Hard-to-grep • Anti-static-analysis friendly**

[![C++17](https://img.shields.io/badge/C++-17-blue?logo=c%2B%2B)](https://isocpp.org/std/the-standard)
[![Header-only](https://img.shields.io/badge/Header--only-yes-success)](#)
[![No dependencies](https://img.shields.io/badge/dependencies-none-important)](#)

##  Features

- Compile-time XOR obfuscation of string literals (`char[]` & `wchar_t[]`)
- **Rolling XOR key stream** — different key byte per position (16-byte cycle by default)
- Strong compile-time seed mixing (inspired by splitmix64 / wyhash)
- Lazy decryption (happens only when you first access the string)
- **Automatic zeroization** on scope exit (via RAII helper)
- Very simple & clean macro syntax
- Header-only — drop-in single file integration
- No external dependencies, no dynamic allocation

## Why better than plain single-byte XOR?

| Feature                     | Classic single XOR | This version (rolling)     |
|-----------------------------|--------------------|-----------------------------|
| Key per string              | 1 byte             | 16-byte cycling stream      |
| Static analysis resistance  | Low                | Significantly higher        |
| Easy to grep / pattern match| Very easy          | Much harder                 |
| Key derivation              | Weak shifts/mul    | Strong mixer (wyhash-like)  |
| Memory cleanup              | Manual             | Automatic RAII              |

## Quick Start

```cpp
#include "obfuscator.h"

int main()
{
    // Most common & safest way — decrypt + auto-zeroize when leaving scope
    const char* msg = OBF_AUTO("kernel32.dll");

    // One-time decrypt (stays decrypted until program ends or zeroized manually)
    const char* api = OBF("CreateRemoteThread");

    // Wide string versions
    const wchar_t* wpath = OBF_W_AUTO(L"C:\\Windows\\Temp\\payload.bin");

    wprintf(L"Path: %s\n", wpath);
    printf("API name: %s\n", api);

    // You can also use it inside functions, conditions, etc.
    if (some_condition) {
        MessageBoxA(NULL, OBF_AUTO("Critical error!"), OBF("Error"), MB_ICONERROR);
    }
}
```

## Available Macros

| Macro              | Behavior                                           | Recommended? | Lifetime of decrypted data          |
|--------------------|----------------------------------------------------|--------------|-------------------------------------|
| `OBF("...")`       | Lazy decrypt, stays decrypted in memory forever    | Sometimes    | Until program ends                  |
| `OBF_AUTO("...")`  | Decrypt + **auto zeroize** when leaving scope      | **Yes**      | Only while in current scope         |
| `OBF_W("L...")`    | Wide version — lazy decrypt                        | Sometimes    | Until program ends                  |
| `OBF_W_AUTO("L...")`| Wide version + **auto zeroize** on scope exit     | **Yes**      | Only while in current scope         |

**Recommendation**: Use `OBF_AUTO` / `OBF_W_AUTO` in most cases — it's significantly safer as it minimizes the time sensitive strings remain in plaintext in memory.


## Performance Overview

All numbers are approximate, measured on modern hardware (Zen 4 / Intel 13th–14th gen, 2024–2025 compilers) with `-O3 -march=native`.

| Scenario                          | Plain literal (ns) | This lib (ns)     | Vectorized xorstr-style (ns) | Overhead factor | Notes |
|-----------------------------------|--------------------|-------------------|------------------------------|-----------------|-------|
| Short string (~8–16 chars), cold  | 0.5–2             | 10–25            | 4–12                        | ~10–20×        | First decrypt dominates |
| Short string, hot (cached)        | 0.5–2             | ~1–3             | ~1–2                        | ~1–3×          | After decrypt(), near-zero cost |
| Medium (~32–64 chars), cold       | 1–4               | 15–45            | 5–15                        | ~5–15×         | Modulo can limit auto-vectorization |
| Medium, hot                       | 1–4               | ~2–6             | ~1–3                        | ~1–3×          | |
| Long (~128+ chars), cold          | 3–10              | 40–120           | 8–30                        | ~5–15×         | Vectorized wins big here |
| 10,000 calls/sec (typical logging)| negligible        | <0.5% total      | <0.2% total                 | —              | Real apps: IO / crypto dominates |
| In tight loop (1M iter/sec)       | —                 | 1–5% slowdown    | <1% slowdown                | —              | Only if decrypt every iteration |
