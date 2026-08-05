adb devices 
adb logcat 
adb logcat | findstr /i ARTR_TS: 

gradlew.bat clean
gradlew.bat build

adb -d install -r app\build\outputs\apk\debug\app-debug.apk

