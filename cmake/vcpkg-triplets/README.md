# CI vcpkg triplets

Overlay triplets used by the `ci-*` presets. They mirror the corresponding
vcpkg builtin triplets and restrict dependency builds to Release:

```cmake
set(VCPKG_BUILD_TYPE release)
```

By default vcpkg builds every port twice, debug and release. CI only ever ships
Release (see the `ci-*` presets in `CMakePresets.json`), so the debug half was
pure waste — it roughly doubled the dependency build time.

These are wired in via `VCPKG_OVERLAY_TRIPLETS` in `CMakePresets.json`, so they
apply to a local `cmake --preset ci-macos` too, not just to CI.

The macOS triplets also set `VCPKG_OSX_DEPLOYMENT_TARGET` to match their CMake
presets. `arm64-osx-release.cmake` targets macOS 15.0 for the compatibility
package, while `arm64-osx26-release.cmake` targets macOS 26.0 for the dedicated
Liquid Glass package. This ensures each application and all of its statically
linked dependencies use the same minimum macOS version.

There is no Linux triplet: the `ci-linux` preset does not use vcpkg at all. Its
wxWidgets comes from apt, because vcpkg's wxwidgets port depends on
`gtk3`/`cairo`/`pango` and builds the whole GTK+X11 stack from source.

Keep the non-`VCPKG_BUILD_TYPE` lines in sync with the vcpkg builtin triplets
they copy (`<vcpkg>/triplets/`); they change very rarely.
