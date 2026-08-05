// As JNI is more inline with CPP thus we are using CPP
#include<stdio.h>
#include<android/log.h>
#include<android_native_app_glue.h>

void android_main(struct android_app* state)
{
    // Code
    __android_log_print(ANDROID_LOG_INFO,"ARTR_TS:", "Hello, World!!!\n");
}
