-dontobfuscate

-assumenosideeffects class android.util.Log {
    public static int v(...);
    public static int d(...);
}

-keep class org.qtproject.qt5.android.** { *; }
-keep class org.krita.android.** { *; }
