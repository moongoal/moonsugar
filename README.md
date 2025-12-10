# Moonsugar Core Library

Moonsugar is a library of core functionality written in C99.

## Modules

|Module|Description|API Stability|
|------|-----------|-------------|
|Coroutine|Stackless coroutine framework|🧪 Unstable|
|Path|Path manipulation|🧪 Unstable|
|Compress|Data compression|🧪 Unstable|
|Memory|Memory allocation|🧪 Unstable|
|URI|Uniform Resource Identifier|🧪 Unstable|
|Config|Text-based configuration|🧪 Unstable|
|Sys|System information queries|🧪 Unstable|
|Log|Logging|🧪 Unstable|
|Containers|Structured data containers|🧪 Unstable|
|Hash|Data hashing functions|🧪 Unstable|
|Handle|Handle interface|🧪 Unstable|
|File|File IO|🧪 Unstable|

## Development

Requirements:

* CMake
* Ninja
* LLVM

Dependencies:

* Zlib

### Building

```
# Configure
cmake --preset Debug

# Build
cmake --build --preset Debug

# Test
ctest --preset Debug
```

## License

See [LICENSE](./LICENSE).



