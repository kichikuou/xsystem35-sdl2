# xsystem35 for Android

## Download
You can download prebuilt APKs
[here](https://github.com/kichikuou/xsystem35-sdl2/releases).

## Build
Prerequisites:
- Android SDK >=28
- Android NDK >=r15c

### Using Android Studio
Open this directory as an Android Studio project.

### Command Line Build
Set environment variables and run the `gradlew` script in this directory.

Example build instructions (for Debian bookworm):
```sh
# Install necessary packages
sudo apt install git wget unzip default-jdk-headless ninja-build

# Install Android SDK / NDK
export ANDROID_SDK_ROOT=$HOME/android-sdk
mkdir -p $ANDROID_SDK_ROOT/cmdline-tools
wget https://dl.google.com/android/repository/commandlinetools-linux-10406996_latest.zip
unzip commandlinetools-linux-10406996_latest.zip -d $ANDROID_SDK_ROOT/cmdline-tools
mv $ANDROID_SDK_ROOT/cmdline-tools/cmdline-tools $ANDROID_SDK_ROOT/cmdline-tools/tools
yes | $ANDROID_SDK_ROOT/cmdline-tools/tools/bin/sdkmanager --licenses
$ANDROID_SDK_ROOT/cmdline-tools/tools/bin/sdkmanager ndk-bundle 'cmake;3.22.1'
export ANDROID_NDK_HOME=$ANDROID_SDK_ROOT/ndk-bundle

# Clone and build xsystem35
git clone https://github.com/kichikuou/xsystem35-sdl2.git
cd xsystem35-sdl2/android
./gradlew build  # or ./gradlew installDebug if you have a connected device
```

## Usage
### Basic Usage
1. Create a ZIP file containing all the game files and BGM files (see
   [below](#preparing-a-zip) for details), and transfer it to your device.
2. Open the app. A list of installed games will be displayed. Since no games
   have been installed yet, only the "Install from ZIP" button will be visible.
   Tap it.
3. Select the ZIP file you created in step 1.
4. The game will start. To simulate a right-click, tap the black bars on either
   the left or right, or top or bottom of the screen.

### Preparing a ZIP
- Include all files from the `GAMEDATA` folder (such as `.ALD` files and
  others). `.EXE` and `.DLL` files are not necessary, but you can include them
  if you want.
- Music files (`.mp3`, `.ogg`, or `.wav`) whose filenames end with a number
  will be recognized as BGM files. For example:
  - `Track2.mp3`
  - `15.ogg`
  - `rance4_03.wav` (Note: The filename shouldn't be `rance403.wav`, as it
    would be treated as the 403rd track.)

Note: This ZIP format is also compatible with
[Kichikuou on Web](http://kichikuou.github.io/web/).

### Miscellaneous
- You can export or import save files via the game list's option menu.
- To uninstall a game, long-tap its title in the game list.

## Known Issues
- Android versions older than 7.0 cannot handle ZIP files containing Shift-JIS
  filenames. This issue occurs with some ZIP files distributed on
  [retroc.net](http://retropc.net/alice/). If you encounter the error message
  "This type of ZIP is not supported," unzip the file on your PC and re-archive
  it using modern ZIP creation software.
