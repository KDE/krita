# Building Krita for Android

First of all, I use linux to do my builds and testing. Although,
they _should_ work on Windows/macOS, I cannott give any guarentee 
that it will.

## Setting up Android SDK and NDK

We right now use android ndk version `r18b` to do our builds. So,
I would recommend to use that. Download it from [google's website](https://developer.android.com/ndk/downloads/older_releases.html)
then extract it.

Next, Android SDK. You can either download Android Studio or just
the `sdk-tools`. Both could be downloaded from [google's website](https://developer.android.com/studio).

If you downloaded Android Studio then open sdk manager and download
`Android SDK Build-Tools`.
(more info: https://developer.android.com/studio/intro/update#sdk-manager)

If you download just `sdk-tools`, then, extract it and run:

```shell
cd <extracted-android-sdk-tools>/tools/bin
./sdkmanager --licenses
./sdkmanager platform-tools
./sdkmanager "platforms;android-21"
./sdkmanager "platforms;android-28"    # for androiddeployqt
./sdkmanager "build-tools;28.0.2"
```

If you get some `ClasNotFoundException` it might be because `java`
version is set to `11`. For `sdkmanager` to work, set it to `8` and
then run it again.

That's the only dependency we have to manage manually!

## Building Krita

Now, to build krita, run `<krita-source>/packaging/android/androidbuild.sh --help`
and pass the required arguments.

Example:

```shell
./androidbuild.sh -p=all --src=/home/sh_zam/workspace/krita --build-type=Debug --build-root=/home/sh_zam/workspace/build-krita-android --ndk-path=/home/sh_zam/Android/Sdk/ndk-bundle --sdk-path=/home/sh_zam/Android/Sdk --api-level=21 --android-abi=armeabi-v7a
```

That's all!

## Installing Krita APK

To install run `adb install -d -r <build-root>/krita_build_apk/build/outputs/apk/debug/krita_build_apk-debug.apk`.

`adb` should be in `<sdk-root>/platform-tools/`

## Crash

If Krita crashes you can look up the logs using `adb logcat`
