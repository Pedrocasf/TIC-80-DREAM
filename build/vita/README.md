# PS Vita build

## Requirements

* [VitaSDK](https://vitasdk.org) with `$VITASDK` pointing at it and
  `$VITASDK/bin` in your `$PATH`
* a host toolchain (`cmake`, `gcc`, `ruby`), the build compiles a few tools that
  run on the build machine

SDL2 is built from `vendor/sdl2`, no extra vdpm package is needed.

## Building instructions

```
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=$VITASDK/share/vita.toolchain.cmake \
  -DCMAKE_BUILD_TYPE=MinSizeRel -DBUILD_WITH_ALL=ON \
  -DBUILD_WITH_JS=OFF -DBUILD_WITH_SCHEME=OFF -DBUILD_WITH_RUBY=OFF -DBUILD_WITH_YUE=OFF
cmake --build build --parallel
```

You should now be able to find `tic80.vpk` in `build/`, install it with
VitaShell.

`BUILD_WITH_ALL` turns on every script runtime, four of them are left out above
because they do not build against the newlib the toolchain ships:

* `BUILD_WITH_JS` — quickjs wants `malloc_usable_size` and `tm_gmtoff`
* `BUILD_WITH_SCHEME` — s7 wants `sigsetjmp` / `siglongjmp`
* `BUILD_WITH_RUBY` — mruby needs a cross build configuration of its own
* `BUILD_WITH_YUE` — yuescript needs more C++ than the toolchain provides

That leaves Lua, Moonscript, Fennel, Wren, Squirrel, Python, Janet and WASM.

## Installing

Copy `tic80.vpk` to the Vita and install it from VitaShell. The bubble asks for
unsafe homebrew permissions, they are needed to reach `ux0:/data` and to raise
the clocks.

## Runtime notes

* Carts, `config.tic` and everything else the studio writes live in
  `ux0:/data/tic80`. Drop your `.tic` files there and they show up in the
  console and in SURF.
* The physical controls are gamepad #1, the PlayStation layout matches the
  TIC-80 one: cross is `A`, circle is `B`, square is `X`, triangle is `Y`.
  `SELECT` opens the game menu.
* The front touchscreen drives the mouse, the rear pad is left alone.
* The software keyboard is drawn over the lower half of the screen in the
  console, in the editors and in carts that ask for `input: keyboard`. SURF, the
  menu and regular games keep the whole screen and are driven with the buttons.
  A USB or Bluetooth keyboard (and mouse) works too, SDL2 picks them up through
  SceHid.
* There is no networking: the online cart browser in SURF and
  `CHECK_NEW_VERSION` do nothing on this platform.
