🔒 C++ String Obfuscator (XOR-Based)

✨ Features
- Compile-time XOR obfuscation of char[] and wchar_t[]
- Simple macro usage with automatic seed based on source line number
- Easy integration (just include one header)
- Runtime decryption + memory zeroization support


⚙️ How It Works
Strings are XOR-encoded at compile time using a seed derived from the line number, then decoded at runtime using the same key. This helps evade static analysis and basic reverse engineering.

## example usage ( c++ )
```cpp
#include "obfuscator.h"

int main() {
    const char* secret = obfuscate("Hello, World!");
    const wchar_t* wide_secret = obfuscate_w(L"Sensitive WString");
    
    printf("Decrypted: %s\n", secret);
}
```


🔐 Overview
```obfuscate("string")```
Accepts a ```const char[]``` literal

- Obfuscates it using an XOR key derived from the line number

- Decrypts lazily at runtime when used

```obfuscate_w(L"wstring")```
- Accepts a ```const wchar_t[]``` wide string literal
- XOR-obfuscated at compile time
- Decrypted on demand

**Internals (Header)**

- ```XorString<N, Seed>``` : Template for narrow strings
- ```XorWString<N, Seed>``` : Template for wide strings
- ```xor_key(seed)``` : Generates a pseudo-random XOR key from the seed

```cpp
const char* hidden = obfuscate("Secret123!");
```
