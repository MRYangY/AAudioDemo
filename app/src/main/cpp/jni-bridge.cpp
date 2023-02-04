#include <jni.h>
#include <string>

extern "C"
JNIEXPORT jlong JNICALL
Java_com_example_aaudiodemo_MainActivity_nativeCreateAAudioEngine(JNIEnv *env, jobject thiz,
                                                                  jstring file_path) {
    // TODO: implement nativeCreateAAudioEngine()
    return 0;
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_aaudiodemo_MainActivity_nativeDestroyAAudioEngine(JNIEnv *env, jobject thiz,
                                                                   jlong engine_handle) {
    // TODO: implement nativeDestroyAAudioEngine()
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_aaudiodemo_MainActivity_nativeAAudioEnginePlay(JNIEnv *env, jobject thiz,
                                                                jlong engine_handle) {
    // TODO: implement nativeAAudioEnginePlay()
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_aaudiodemo_MainActivity_nativeAAudioEnginePause(JNIEnv *env, jobject thiz,
                                                                 jlong engine_handle) {
    // TODO: implement nativeAAudioEnginePause()
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_aaudiodemo_MainActivity_nativeAAudioEngineStop(JNIEnv *env, jobject thiz,
                                                                jlong engine_handle) {
    // TODO: implement nativeAAudioEngineStop()
}