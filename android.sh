#!/bin/sh

SDL_COMMIT=tags/release-3.2.0
ANDROID_PROJECT=com.igrmm.ddemon
APP_NAME=DDEMON

if [ "$1" = "install" ]; then
    mkdir -p build
    if [ ! -d "build/android" ]; then
        cd build
        mkdir -p android && cd android
        git clone https://github.com/libsdl-org/SDL/
        cd SDL && git checkout $SDL_COMMIT && cd build-scripts
        ./create-android-project.py $ANDROID_PROJECT /dev/null 2> /dev/null
        cd ../ && mv build/$ANDROID_PROJECT ../ && cd ../
        ln -s $(pwd)/SDL $ANDROID_PROJECT/app/jni/
        cd $ANDROID_PROJECT/app/jni/

        #scaping $ and /
        sed -i "s/null//" src/Android.mk
        sed -i "s/LOCAL_SRC_FILES.*/LOCAL_SRC_FILES := \$(wildcard \$(LOCAL_PATH)\/..\/..\/..\/..\/..\/..\/src\/*.c)/" src/Android.mk
        sed -i "s/-lGLESv2/-lGLESv3/" src/Android.mk

        cd ../
        sed -i "s/Game/$APP_NAME/" src/main/res/values/strings.xml

        sed -i "/<activity android:name/a \            android:screenOrientation=\"landscape\"" src/main/AndroidManifest.xml

        cd ../../../../
        ln -s $(pwd)/assets build/android/$ANDROID_PROJECT/app/src/main/
    fi
    cd build/android/$ANDROID_PROJECT/
    ./gradlew installDebug
    cd ../../../
fi

# remember to pair and connect phone to adb
if [ "$1" = "debug" ]; then
    adb shell run-as $ANDROID_PROJECT logcat
fi
