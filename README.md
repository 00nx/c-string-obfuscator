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
