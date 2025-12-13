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
|Thread|Multi-threading framework|🧪 Unstable|

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

### Usage

#### Object Lifetime

Objects that require manua lifetime management have associated `_construct()` and `_destroy()` functions to manage their lifetime. Some do not have a destructor, when only initialisation is required. Many constructors return a result value. When this value is not `MS_RESULT_SUCCESS`, an error occurred and the object must be destroyed by calling its destructor function (if existing).

When creating a composite object, it's recommended to fail the composite object creation upon construction failure of one of the sub-objects and delegate clean-up to the destructor. This way, a hierarchy of destructors will always be called, avoiding complicated, error-prone, clean-up logic in mulitple points.

All manual lifetime structures provided are designed to follow this strategy.

## License

See [LICENSE](./LICENSE).



