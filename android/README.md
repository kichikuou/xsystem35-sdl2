# xsystem35 for Android

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
1. Create a ZIP file containing all the game files (`*.ALD`) and BGM files (for example `Track02.mp3`, `Track03.mp3`, ...), and transfer it to your device.
2. Open the app. A list of installed games is displayed. Since nothing has been installed yet, only the "Install from ZIP" button is displayed. Tap it.
3. Select the zip you created in 1.
4. The game starts. Two-finger touch is treated as a right click.

To uninstall a game, long-tap the title in the game list.

## TODO
- Improve launcher
- MIDI BGM support
