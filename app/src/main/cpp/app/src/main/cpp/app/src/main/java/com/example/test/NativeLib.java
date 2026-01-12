package com.example.test;

public class NativeLib {
    static {
        System.loadLibrary("native-lib");
    }

    // Native methods - signatures must match the C++ exported names
    public static native void tick(float dt);
    public static native void touch(float x, float y, boolean down);
}
