# Moonsugar Core Library

Moonsugar is a library of core functionality written in C99.

## Modules

|Module|Description|API Stability|
|------|-----------|-------------|
|Coroutine|Stackless coroutine framework|🧪 Unstable|
|Path|Path manipulation|🧪 Unstable|

## Development

Requirements:

* CMake
* Ninja
* LLVM

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



