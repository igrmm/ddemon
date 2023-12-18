#!/bin/sh

SDL_COMMIT=tags/release-2.28.3
SDL_IMAGE_COMMIT=tags/release-2.6.3
SDL_TTF_COMMIT=tags/release-2.20.2
ANDROID_PROJECT=com.igrmm.isgt
APP_NAME=ISGT

mkdir -p build

if [ ! -d "build/android" ]; then
    cd build
    mkdir -p android && cd android
    git clone https://github.com/libsdl-org/SDL/
    cd SDL && git checkout $SDL_COMMIT && cd build-scripts
    ./androidbuild.sh $ANDROID_PROJECT /dev/null
    cd ../ && mv build/$ANDROID_PROJECT ../ && cd ../
    cd $ANDROID_PROJECT/app/jni/
    git clone https://github.com/libsdl-org/SDL_image.git
    cd SDL_image && git checkout $SDL_IMAGE_COMMIT && cd ../
    git clone https://github.com/libsdl-org/SDL_ttf.git
    cd SDL_ttf && git checkout $SDL_TTF_COMMIT && git submodule update --init
    cd ../

    #scaping $ and /
    sed -i "/LOCAL_C_INCLUDES.*/ s/$/ \$(LOCAL_PATH)\/..\/SDL_image\/ \$(LOCAL_PATH)\/..\/SDL_ttf\//" src/Android.mk
    sed -i "s/LOCAL_SRC_FILES.*/LOCAL_SRC_FILES := \$(wildcard \$(LOCAL_PATH)\/..\/..\/..\/..\/..\/..\/src\/*.c)/" src/Android.mk
    sed -i "/LOCAL_SHARED_LIBRARIES.*/ s/$/ SDL2_image SDL2_ttf/" src/Android.mk

    cd ../
    sed -i "s/Game/$APP_NAME/" src/main/res/values/strings.xml

    sed -i "/<activity android:name/a \            android:screenOrientation=\"landscape\"" src/main/AndroidManifest.xml

    mkdir src/main/res/values-v27/
    echo '<?xml version="1.0" encoding="utf-8"?>
    <resources>
    <style name="Theme">
    <item name="android:windowLayoutInDisplayCutoutMode">
    shortEdges
    </item>
    </style>
    </resources>' > src/main/res/values-v27/styles.xml
    sed -i "s/android:theme=.*/android:theme=\"@style\/Theme\"/" src/main/AndroidManifest.xml

    cd ../../../../
    ln -s $(pwd)/assets build/android/$ANDROID_PROJECT/app/src/main/
fi
cd build/android/$ANDROID_PROJECT/
./gradlew installDebug
cd ../../../
