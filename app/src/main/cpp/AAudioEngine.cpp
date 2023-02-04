//
// Created by yu mryang on 2023/2/4.
//

#include "AAudioEngine.h"

namespace aaudiodemo {
    AAudioEngine::AAudioEngine(const std::string &filePath) {
        file = fopen(filePath.c_str(), "r");

    }

    AAudioEngine::~AAudioEngine() {
        if (file) {
            fclose(file);
            file = nullptr;
        }
    }

    void AAudioEngine::Start() {

    }

    void AAudioEngine::Pause() {

    }

    void AAudioEngine::Stop() {

    }

    void AAudioEngine::Destroy() {

    }

    void AAudioEngine::workFunc() {

    }

    AAudioStreamBuilder *AAudioEngine::createStreamBuilder() {
        AAudioStreamBuilder *builder = nullptr;
        aaudio_result_t result = AAudio_createStreamBuilder(&builder);
        if (result != AAUDIO_OK) {
            LOGE("AAudioEngine::createStreamBuilder Fail: %s", AAudio_convertResultToText(result));
        }
        return builder;
    }

    void AAudioEngine::setupPlaybackStreamParameters(AAudioStreamBuilder *builder) {
        AAudioStreamBuilder_setDeviceId(builder, AAUDIO_UNSPECIFIED);
        AAudioStreamBuilder_setFormat(builder, mFormat);
        AAudioStreamBuilder_setChannelCount(builder, mChannel);
        AAudioStreamBuilder_setSampleRate(builder, mSampleRate);

        // We request EXCLUSIVE mode since this will give us the lowest possible latency.
        // If EXCLUSIVE mode isn't available the builder will fall back to SHARED mode.
        AAudioStreamBuilder_setSharingMode(builder, AAUDIO_SHARING_MODE_EXCLUSIVE);
        AAudioStreamBuilder_setPerformanceMode(builder, AAUDIO_PERFORMANCE_MODE_LOW_LATENCY);
        AAudioStreamBuilder_setDirection(builder, AAUDIO_DIRECTION_OUTPUT);
//    AAudioStreamBuilder_setDataCallback(builder, ::dataCallback, this);
//    AAudioStreamBuilder_setErrorCallback(builder, ::errorCallback, this);
    }

    void static PrintAudioStreamInfo(const AAudioStream *stream) {
#define STREAM_CALL(c) AAudioStream_##c((AAudioStream*)stream)
        LOGI("StreamID: %p", stream);

        LOGI("BufferCapacity: %d", STREAM_CALL(getBufferCapacityInFrames));
        LOGI("BufferSize: %d", STREAM_CALL(getBufferSizeInFrames));
        LOGI("FramesPerBurst: %d", STREAM_CALL(getFramesPerBurst));
        LOGI("XRunCount: %d", STREAM_CALL(getXRunCount));
        LOGI("SampleRate: %d", STREAM_CALL(getSampleRate));
        LOGI("SamplesPerFrame: %d", STREAM_CALL(getChannelCount));
        LOGI("DeviceId: %d", STREAM_CALL(getDeviceId));
        LOGI("Format: %d", STREAM_CALL(getFormat));
        LOGI("SharingMode: %s", (STREAM_CALL(getSharingMode)) == AAUDIO_SHARING_MODE_EXCLUSIVE ?
                                "EXCLUSIVE" : "SHARED");

        aaudio_performance_mode_t perfMode = STREAM_CALL(getPerformanceMode);
        std::string perfModeDescription;
        switch (perfMode) {
            case AAUDIO_PERFORMANCE_MODE_NONE:
                perfModeDescription = "NONE";
                break;
            case AAUDIO_PERFORMANCE_MODE_LOW_LATENCY:
                perfModeDescription = "LOW_LATENCY";
                break;
            case AAUDIO_PERFORMANCE_MODE_POWER_SAVING:
                perfModeDescription = "POWER_SAVING";
                break;
            default:
                perfModeDescription = "UNKNOWN";
                break;
        }
        LOGI("PerformanceMode: %s", perfModeDescription.c_str());

        aaudio_direction_t dir = STREAM_CALL(getDirection);
        LOGI("Direction: %s", (dir == AAUDIO_DIRECTION_OUTPUT ? "OUTPUT" : "INPUT"));
        if (dir == AAUDIO_DIRECTION_OUTPUT) {
            LOGI("FramesReadByDevice: %d", (int32_t) STREAM_CALL(getFramesRead));
            LOGI("FramesWriteByApp: %d", (int32_t) STREAM_CALL(getFramesWritten));
        } else {
            LOGI("FramesReadByApp: %d", (int32_t) STREAM_CALL(getFramesRead));
            LOGI("FramesWriteByDevice: %d", (int32_t) STREAM_CALL(getFramesWritten));
        }
#undef STREAM_CALL
    }

    void AAudioEngine::createPlaybackStream() {
        AAudioStreamBuilder *builder = createStreamBuilder();

        if (builder != nullptr) {
            setupPlaybackStreamParameters(builder);
            aaudio_result_t result = AAudioStreamBuilder_openStream(builder, &mAudioStream);
            if (result == AAUDIO_OK && mAudioStream != nullptr) {

                // check that we got PCM_FLOAT format
                if (mFormat != AAudioStream_getFormat(mAudioStream)) {
                    LOGW("Sample format is not PCM_FLOAT");
                }

                int framesPerBurst = AAudioStream_getFramesPerBurst(mAudioStream);

                // Set the buffer size to the burst size - this will give us the minimum possible latency
                AAudioStream_setBufferSizeInFrames(mAudioStream, framesPerBurst);

                PrintAudioStreamInfo(mAudioStream);

            } else {
                LOGE("Failed to create stream. Error: %s", AAudio_convertResultToText(result));
            }
            AAudioStreamBuilder_delete(builder);

        } else {
            LOGE("Unable to obtain an AAudioStreamBuilder object");
        }
    }

}


