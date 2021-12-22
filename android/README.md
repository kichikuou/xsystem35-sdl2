# xsystem35 for Android

## Download
Prebuilt APKs are [here](https://github.com/kichikuou/xsystem35-sdl2/releases).

## Build
Prerequisites:
- CMake >=3.20
- Android SDK >=28
- Android NDK >=r15c

### Using Android Studio
Open this directory as an Android Studio project.

### Command line build
Configure environment variables and run the `gradlew` script in this folder.

Example build instructions (for Debian bullseye):
```sh
# Install necessary packages
sudo apt install git wget unzip default-jdk-headless ninja-build
sudo apt install -t bullseye-backports cmake

# Install Android SDK / NDK
export ANDROID_SDK_ROOT=$HOME/android-sdk
mkdir -p $ANDROID_SDK_ROOT/cmdline-tools
wget https://dl.google.com/android/repository/commandlinetools-linux-7583922_latest.zip
unzip commandlinetools-linux-7583922_latest.zip -d $ANDROID_SDK_ROOT/cmdline-tools
mv $ANDROID_SDK_ROOT/cmdline-tools/cmdline-tools $ANDROID_SDK_ROOT/cmdline-tools/tools
yes |$ANDROID_SDK_ROOT/cmdline-tools/tools/bin/sdkmanager --licenses
$ANDROID_SDK_ROOT/cmdline-tools/tools/bin/sdkmanager ndk-bundle
export ANDROID_NDK_HOME=$ANDROID_SDK_ROOT/ndk-bundle

# Check out and build xsystem35
git clone https://github.com/kichikuou/xsystem35-sdl2.git
cd xsystem35-sdl2/android
./gradlew build  # or ./gradlew installDebug if you have a connected device
```

## Use
### Basic Usage
1. Create a ZIP file containing all the game files and BGM files (see [below](#preparing-a-zip) for details), and transfer it to your device.
2. Open the app. A list of installed games is displayed. Since nothing has been installed yet, only the "Install from ZIP" button is displayed. Tap it.
3. Select the ZIP file you created in 1.
4. The game starts. Two-finger touch is treated as a right click.

### Preparing a ZIP
- Include all files in the `GAMEDATA` folder (`.ALD` files and others). `.EXE` and `.DLL` are not really needed, but you can include them as well.
- Music files (`.mp3`, `.ogg` or `.wav`) whose file names end with a number are recognized as BGM files. For example:
  - `Track2.mp3`
  - `15.ogg`
  - `rance4_03.wav` (This shouldn't be `rance403.wav`, because it would be treated as the 403rd track)

Note: This form of ZIP can be used in [Kichikuou on Web](http://kichikuou.github.io/web/) as well.

### Miscellaneous
- You can export / import saved files using the option menu of the game list.
- To uninstall a game, long-tap the title in the game list.

## Known Issues
- Android versions older than 7.0 cannot handle ZIPs containing Shift-JIS file names. This is the case with some ZIPs distributed on [retroc.net](http://retropc.net/alice/). If you get the error "This type of ZIP is not supported.", unzip the ZIP file on your PC and re-archive it with a modern ZIP creation software.
