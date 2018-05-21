# ffprobe-android
A project for using ffprobe on android based on [ijkplayer](https://github.com/Bilibili/ijkplayer)&[ffmpeg](http://ffmpeg.org)

After using ijkplayer for a long time we add some logic for using ffprobe features on the android platform by simply invoke java methods.

this lib including sample code for getting video duration & video format.

```
public static VideoDuration probeVideoDuration(String url);

public static VideoDuration probeVideoDuration
(String url, Map<String, String> headers);

public static VideoDuration probeVideoDuration
(String url, Map<String, String> headers,int timeoutSec);

public static VideoFormat probeVideoFormat(String url);

public static VideoFormat probeVideoFormat
(String url, Map<String, String> headers);

public static VideoFormat probeVideoFormat
(String url, Map<String, String> headers,int timeoutSec);
```

The build scripts including in this porject are all based on the project [ijkplayer](https://github.com/Bilibili/ijkplayer).If you have some trouble building the project also check the [ijkplayer](https://github.com/Bilibili/ijkplayer) project's `README` for help.

### Before Build
```
# install homebrew, git, yasm
ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
brew install git
brew install yasm

# add these lines to your ~/.bash_profile or ~/.profile
# export ANDROID_SDK=<your sdk path>
# export ANDROID_NDK=<your ndk path>

# on Cygwin (unmaintained)
# install git, make, yasm
```

- If you prefer more codec/format
```
cd config
rm module.sh
ln -s module-default.sh module.sh
cd android/contrib
# cd ios
sh compile-ffmpeg.sh clean
```

- If you prefer less codec/format for smaller binary size (include hevc function)
```
cd config
rm module.sh
ln -s module-lite-hevc.sh module.sh
cd android/contrib
# cd ios
sh compile-ffmpeg.sh clean
```

- If you prefer less codec/format for smaller binary size (by default)
```
cd config
rm module.sh
ln -s module-lite.sh module.sh
cd android/contrib
# cd ios
sh compile-ffmpeg.sh clean
```

- For Ubuntu/Debian users.
```
# choose [No] to use bash
sudo dpkg-reconfigure dash
```

### Build Android
```
git clone https://github.com/george5613/ffprobe-android.git ffprobe-android
cd ffprobe-android
git checkout -B latest master

./init-android.sh

cd android/contrib
./compile-ffmpeg.sh clean
./compile-ffmpeg.sh all

cd ..
./compile-ffprobe.sh all

# Android Studio:
#     Open an existing Android Studio project
#     Select android/probe/ and import
#
#     define ext block in your root build.gradle
#     ext {
#       compileSdkVersion = 23       // depending on your sdk version
#       buildToolsVersion = "23.0.0" // depending on your build tools version
#
#       targetSdkVersion = 23        // depending on your sdk version
#     }
#
# Gradle
#     cd probe
#     gradle

```

