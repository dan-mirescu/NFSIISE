NFSIISE
=======

Cross-platform wrapper for the Need For Speed™ II SE game with 3D acceleration and TCP protocol!

**Fork features:**
- **Big-endian** support on Linux (still WIP)
- **x86_64** support for Linux
- **ARM64** (AArch64) support for Linux
- **Android arm64** APK build (SDL3, GLES2)
- **SDL3** port

## OpenGL

* By default OpenGL 2 is used (except Android - it always uses GLES2).
* To use OpenGL 1, pass `-DOPENGL1X=ON` to CMake.
* To use OpenGL ES 2, pass `-DGLES2=ON` to CMake.

![Screenshot](https://raw.githubusercontent.com/zaps166/GitHubCommonContents/master/Screenshots/NFSIISE.png)

## Build with CMake

### Requirements

* C++14 compiler (Clang or GCC)
* SDL3 development libraries
* OpenGL development libraries
* (optional) Yasm assembler, for x86 32-bit assembly build (use -DUSE_ASM=ON to compile)

### C++ native build (x86_64, ARM64, PowerPC, etc.) — default

Release build:

```sh
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

Debug build:

```sh
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build .
```

The binary will be placed in the `Need For Speed II SE` directory.

#### OpenGL 1.x backend

```sh
cmake .. -DOPENGL1X=ON -DCMAKE_BUILD_TYPE=Release
```

#### OpenGL ES 2 backend

```sh
cmake .. -DGLES2=ON -DCMAKE_BUILD_TYPE=Release
```

### x86 32-bit assembly build

For 32-bit x86 systems, the hand-tuned assembly can be used instead of the C++ translation:

```sh
mkdir build && cd build
cmake .. -DUSE_ASM=ON -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

On Debian/Ubuntu, install 32-bit dependencies first:

```sh
dpkg --add-architecture i386
apt-get update
apt-get install libsdl3-dev:i386 gcc-multilib yasm
```

### Windows cross-compilation (MinGW)

Use a MinGW toolchain file with CMake, for example:

```sh
cmake .. -DCMAKE_TOOLCHAIN_FILE=/path/to/mingw-toolchain.cmake -DCMAKE_BUILD_TYPE=Release
```

## Run

* Copy `fedata` and `gamedata` directories from the Need For Speed™ II SE original CD-ROM into `Need For Speed II SE` directory.
* This game **needs** data from Need For Speed 2 **Special Edition**, otherwise you'll see a 'MOVIE FILE NOT FOUND' message!
* You can delete unnecessary files, e.g. `fedata/pc/text/text.*`, because TCP version uses new files in root directory.
* All files and directories copied from CD-ROM **must** have *small letters* on Unix-like systems!!!
  * Please use the `Need For Speed II SE/convert_to_lowercase` script if you have UPPERCASE names.
* If you want to change the language, edit `install.win` file (with text editor which doesn't modify last line or line endings!) and change the first line. Leave `4nn` as is and modify only language name. Possible languages are:
  * english,
  * french,
  * german,
  * italian,
  * spanish,
  * swedish.
* Run the game.
* The game settings files are located in `~/.nfs2se` (`%AppData%\.nfs2se` on Windows). At the first run, the `nfs2se.conf.template` will be copied there. You can modify the file if you want to configure the game. On Windows, you can use `open_config.bat` to open the config file in notepad.
* On Android devices, copy `gamedata`, `fedata`, `install.win`, `text.*`, `nfs2se.conf` (rename `nfs2se.conf.template` to `nfs2se.conf`) into `/sdcard/NFSIISE` or `/storage/emulated/0/NFSIISE` (create `NFSIISE` directory first).
* For Batocera Linux devices (an open-source and completely free retro-gaming distribution), tested with version 39:
  * take Windows version (e.g `nfs2se-win32-v1.4.0.zip` under Releases), unpack it,
  * copy original `fedata` and `gamedata` directories into `Need For Speed II SE` directory,
  * in `Need For Speed II SE` directory create new file `autorun.cmd` with content `CMD=nfs2se.exe`,
  * rename directory `Need For Speed II SE` to `Need For Speed II SE.pc`,
  * place directory `Need For Speed II SE.pc` into your Batocera Linux directory `/userdata/roms/windows/`,
  * to update the game list in Batocera using your controller, press `START` and go to `GAME SETTINGS` → `UPDATE GAMELISTS`, the game is listed under `Windows` section and will be successfully emulated via Wine

## Android

The Android build produces an **arm64-v8a** APK (SDL3 + GLES2 backend).

### Requirements

* Android SDK: platform 35, build-tools, CMake, NDK **28.2.13676358**
  (the version SDL3 3.4.x is built with),
* JDK 17 or newer (the JBR bundled with Android Studio works),
* SDL3 **source** code (3.4.x), unpacked somewhere.

### Setup

* Set `ANDROID_HOME` (or write `sdk.dir=` into `src/Android/local.properties`),
* Create a symlink to the unpacked `SDL3-*` directory at `src/Android/app/jni/SDL`:
  ```sh
  ln -s /path/to/SDL3-3.4.20 src/Android/app/jni/SDL
  ```
  On Windows use a junction instead:
  ```cmd
  mklink /J src\Android\app\jni\SDL C:\path\to\SDL3-3.4.20
  ```
  SDL3 is built from source by CMake and its Java glue (`org.libsdl.app.*`) is
  compiled straight out of `jni/SDL/android-project`, so the symlink is the only
  place the SDL version is configured.

### Build

```sh
cd src/Android
./gradlew assembleDebug        # app/build/outputs/apk/debug/app-debug.apk
./gradlew assembleRelease      # needs your own signing config
```

To bake the game data into the APK, point `gameDataDir` at your copy of the
CD-ROM content (the directory holding `fedata`, `gamedata`, `install.win` and
the `text.*` files, in any letter case):

```sh
./gradlew assembleRelease -PgameDataDir="/path/to/Need For Speed II SE"
```

The files are added as assets (lower-cased, and stored uncompressed since they
are compressed formats already), which makes the APK about 530 MB.  On the
first run the game unpacks them into
`/sdcard/Android/data/org.zaps166.NFSIISE/files` — the game reads its data with
plain stdio, which cannot read assets — and shows a message box while it does.
Unpacking needs roughly as much free space as the APK and only happens once;
a `/sdcard/NFSIISE` directory with game data in it still takes precedence.

To build for other ABIs, change `abiFilters` in `src/Android/app/build.gradle`.
Note that only 32-bit ABIs (`armeabi-v7a`) and `arm64-v8a` make sense: the
wrapper truncates every pointer it hands to the translated game code to 32 bits.

### How the APK is laid out

* `libSDL3.so` – built from the SDL3 sources,
* `libnfs2se.so` – the game and the wrapper (all of `src/` plus the
  `src/Cpp/NFS2SE.cpp` unity build),
* `libmain.so` – the shared object SDL looks `SDL_main` up in. It is only a
  loader stub (`src/Android/app/jni/src/Loader.c`): it reserves 64 MB at the
  first free low address (`0x10000000`, `0x14000000`, … – which one is free
  differs per device) and asks the bionic linker to map `libnfs2se.so` there
  (`ANDROID_DLEXT_RESERVED_ADDRESS`) before calling `nfs2se_main`.
  This is needed because on 64-bit Android every library is mapped far above
  4 GB, while the wrapper stores all pointers truncated to 32 bits (`push32`,
  `PTR32`); on Linux the same thing is achieved with `-no-pie`.
  `libnfs2se.so` must therefore never be loaded with `System.loadLibrary`.

### On-screen touch controls

Seven translucent buttons are drawn over the game (steer left/right,
accelerate, brake, `Esc`, `C` for the camera view and `F11` to reset the car).
A press pushes the matching key event,
so the game sees an ordinary keyboard; a finger that misses every button still
works as before (drag moves the pointer, a tap is Enter).  They are enabled by
default on Android only and are configured in `nfs2se.conf`:

```
TouchControls=1
TouchControlsOpacity=40
TouchControlsScale=100
TouchButtons=left:9,76,10;right:27,76,10;down:73,78,8.5;up:91,76,11;esc:5,11,5.5;c:87,11,4.5;f11:95,11,5.5
```

Each button is `key:centerX,centerY,radius`, where `centerX` is a percentage of
the window width and `centerY`/`radius` are percentages of its height.  Usable
keys are `left`, `right`, `up`, `down`, `esc`, `enter`, `space`, `tab`,
`shift`, `ctrl`, `f1`-`f12` and single letters or digits.  The overlay is drawn
by the OpenGL 2 / GLES2 backend; the legacy OpenGL 1 backend does not draw it.

### Game data

Copy `gamedata`, `fedata`, `install.win`, `text.*` and `nfs2se.conf` (renamed
from `nfs2se.conf.template`) into either

* `/sdcard/NFSIISE` – needs "All files access" (`MANAGE_EXTERNAL_STORAGE`) to be
  granted in the system settings on Android 11+, or
* `/sdcard/Android/data/org.zaps166.NFSIISE/files` – the app-specific directory,
  which needs no permission and is used automatically when the first one cannot
  be opened.

## What works

* Game controllers (reconnected game controllers should be the same),
* Force Feedback (tested on Linux),
* TCP and UDP connection,
* Serial port connection,
* Brightness,
* Sound.

## What does not work

* Modem connection (it will never work again, this feature has been removed from assembly code),

## Function keys in-game

* F1  - toggle rain
* F2  - car detail
* F3  - view distance
* F4  - toggle horizon
* F5  - toggle HUD (player 1)
* F6  - toggle HUD (player 2)
* F7  - toggle mirror
* F8  - toggle music
* F9  - toggle sound effects
* F10 - brightness
* F11 - reset car (player 1)
* F12 - reset car (player 2)

## Additional information

* Cockpit view and night driving are unavailable (original 3D-accelerated version doesn't have this), see [NFSIISEN](https://github.com/zaps166/NFSIISEN) repository.
* OpenGL 1.x only: if the game crashes it might not restore the gamma properly. In this case run: `xgamma -gamma 1.0`.
* There is Docker based build environment available on [GitHub](https://github.com/thomas-mc-work/nfsiise-build-env).

## License

This project is licensed under the **MIT License** – see the [LICENSE](LICENSE) file for details.

This project is **not affiliated with or endorsed by Electronic Arts Inc.** "Need for Speed" is a registered trademark of Electronic Arts Inc. All game assets (data files, graphics, sounds) remain the property of their respective owners. The wrapper source code is the only portion covered by the MIT license.
