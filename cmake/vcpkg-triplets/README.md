# CI vcpkg triplets

Overlay triplets used by the `ci-*` presets. Each one mirrors the vcpkg builtin
triplet of the same base name and adds a single line:

```cmake
set(VCPKG_BUILD_TYPE release)
```

By default vcpkg builds every port twice, debug and release. CI only ever ships
Release (see the `ci-*` presets in `CMakePresets.json`), so the debug half was
pure waste — it roughly doubled the dependency build time.

These are wired in via `VCPKG_OVERLAY_TRIPLETS` in `CMakePresets.json`, so they
apply to a local `cmake --preset ci-linux` too, not just to CI.

Keep the non-`VCPKG_BUILD_TYPE` lines in sync with the vcpkg builtin triplets
they copy (`<vcpkg>/triplets/`); they change very rarely.
