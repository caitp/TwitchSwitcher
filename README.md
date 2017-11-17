## TwitchSwitcher

A plugin for [obs-studio](https://github.com/jp9000/obs-studio), which enables automatically Switching the Twitch.tv channel status and game when switching scenes.

## Build

1. Install [CMake](https://cmake.org/)
2. Check out and build [obs-studio sources](https://github.com/jp9000/obs-studio) [instructions](https://github.com/jp9000/obs-studio/wiki/Install-Instructions)
3. Get libcurl development files (such as from https://curl.haxx.se/download.html)
4. Run [tools/update-dependencies.sh](tools/update-dependencies.sh) (or .bat on windows) to update git submodules
5. configure and generate your CMake project (easiest using cmake-gui)
6. open your project file, or `make -j32` or `ninja -C out/Release` whatever it is you like to do on your platform
7. Copy the output files into your obs-studio installation
8. Try it out

TwitchSwitcher depends on modern C++ features, such as lambdas and std::promise. Remember to use a modern libc++ implementation, or a modern version of Visual Studio, to build this thing.

## Usage

This plugin adds a new Scene Item to obs-studio, which contains properties specifying a game name and channel status (or title). When switching to a scene with such a scene item, a request is sent to the Twitch API to update the channel with those fields. The fields are optional, and if blank, will not be included in the request to the server.

So, change scenes, change your twitch status, basically. As the obs-studio API progresses, more useful ways to use this may present themselves and make for a better plugin.

## License

Licensed under the Apache 2.0 license. See [LICENSE](LICENSE) for details.

Third party libraries used by this project are licensed under the terms of their various license documentation, which is included in their respective project folders.
