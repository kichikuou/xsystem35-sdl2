# xsytem35 for Android

## Build
Prerequisites:
- CMake >=3.13
- Android SDK >=26
- Android NDK >=r15c

### Using Android Studio
Open this directory as an Android Studio project.

### Command line build
Configure environment variables and run the `gradlew` script in this folder.

Example build instructions (for Debian Stretch):
```sh
# Install necessary packages
sudo apt install git wget unzip default-jdk-headless ninja-build
sudo apt -t stretch-backports install cmake

# Install Android SDK / NDK
export ANDROID_HOME=$HOME/android-sdk
export PATH=$PATH:$ANDROID_HOME/tools
export PATH=$PATH:$ANDROID_HOME/platform-tools
wget https://dl.google.com/android/repository/sdk-tools-linux-4333796.zip
unzip sdk-tools-linux-4333796.zip -d $ANDROID_HOME
yes |$ANDROID_HOME/tools/bin/sdkmanager --licenses
$ANDROID_HOME/tools/bin/sdkmanager ndk-bundle

# Check out and build xsystem35
git clone https://github.com/kichikuou/xsystem35-sdl2.git
cd xsystem35-sdl2/android
./gradlew build  # or ./gradlew installDebug if you have a connected device
```

## How to use
1. Create a ZIP file containing all the game files (`*.ALD`) and [configuration files](https://haniwa.technology/games/preparing-a-game-directory.html), and transfer it to your device.
2. When you start the app, a file chooser opens. Select the zip file.
3. The game starts.
4. Game data persists in the app's internal storage. Use the device's Settings app if you want to clear it (eg. before installing another game).

Note that this Android port is still in early development; see below.

## TODO
- Right-click emulation
- Save / Load support
- Better launcher
- MP3 / MIDI BGM support (currently only ogg is supported)
- App icon
