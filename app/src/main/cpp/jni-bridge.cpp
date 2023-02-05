#include <jni.h>
#include <string>
#include "AAudioEngine.h"

using namespace aaudiodemo;
extern "C"
JNIEXPORT jlong JNICALL
Java_com_example_aaudiodemo_MainActivity_nativeCreateAAudioEngine(JNIEnv *env, jobject thiz,
                                                                  jobject assetManager,
                                                                  jstring file_path,
                                                                  jint sampleRate,
                                                                  jint audioChannel,
                                                                  jint audioFormat) {
    jboolean isCopy;
    const char *convertedValue = (env)->GetStringUTFChars(file_path, &isCopy);
    std::string string = convertedValue;
    auto playEngine = new AAudioEngine(AAssetManager_fromJava(env, assetManager), string,
                                       sampleRate, audioChannel, audioFormat);
    env->ReleaseStringUTFChars(file_path, convertedValue);
    bool ret = playEngine->Init();
    LOGI("nativeCreateAAudioEngine, init ret:%d", ret);
    if (ret) {
        return reinterpret_cast<jlong> (playEngine);
    } else {
        return 0;
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_aaudiodemo_MainActivity_nativeDestroyAAudioEngine(JNIEnv *env, jobject thiz,
                                                                   jlong engine_handle) {
    LOGI("nativeDestroyAAudioEngine");
    auto playEngine = reinterpret_cast<AAudioEngine *>(engine_handle);
    delete playEngine;
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_aaudiodemo_MainActivity_nativeAAudioEnginePlay(JNIEnv *env, jobject thiz,
                                                                jlong engine_handle) {
    LOGI("nativeAAudioEnginePlay");
    auto playEngine = reinterpret_cast<AAudioEngine *>(engine_handle);
    playEngine->Start();
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_aaudiodemo_MainActivity_nativeAAudioEnginePause(JNIEnv *env, jobject thiz,
                                                                 jlong engine_handle) {
    LOGI("nativeAAudioEnginePause");
    auto playEngine = reinterpret_cast<AAudioEngine *>(engine_handle);
    playEngine->Pause();
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_aaudiodemo_MainActivity_nativeAAudioEngineStop(JNIEnv *env, jobject thiz,
                                                                jlong engine_handle) {
    LOGI("nativeAAudioEngineStop");
    auto playEngine = reinterpret_cast<AAudioEngine *>(engine_handle);
    playEngine->Stop();
}