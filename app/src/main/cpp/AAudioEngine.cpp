//
// Created by dahaimian on 2023/2/4.
//

#include "AAudioEngine.h"

#define BUFFER_SIZE 2048
#define TIMEOUT_NANO 800000000L
namespace aaudiodemo {
#if AAUDIO_CALLBACK

    /**
 * Every time the playback stream requires data this method will be called.
 *
 * @param stream the audio stream which is requesting data, this is the playStream_ object
 * @param userData the context in which the function is being called, in this case it will be the
 * PlayAudioEngine instance
 * @param audioData an empty buffer into which we can write our audio data
 * @param numFrames the number of audio frames which are required
 * @return Either AAUDIO_CALLBACK_RESULT_CONTINUE if the stream should continue requesting data
 * or AAUDIO_CALLBACK_RESULT_STOP if the stream should stop.
 *
 * @see PlayAudioEngine#dataCallback
 */
    aaudio_data_callback_result_t dataCallback(AAudioStream *stream, void *userData,
                                               void *audioData, int32_t numFrames) {
        assert(userData && audioData);
        auto *audioEngine = reinterpret_cast<AAudioEngine *>(userData);
        return audioEngine->dataCallback(stream, audioData, numFrames);
    }

/**
 * If there is an error with a stream this function will be called. A common example of an error
 * is when an audio device (such as headphones) is disconnected. In this case you should not
 * restart the stream within the callback, instead use a separate thread to perform the stream
 * recreation and restart.
 *
 * @param stream the stream with the error
 * @param userData the context in which the function is being called, in this case it will be the
 * PlayAudioEngine instance
 * @param error the error which occured, a human readable string can be obtained using
 * AAudio_convertResultToText(error);
 *
 * @see PlayAudioEngine#errorCallback
 */
    void errorCallback(AAudioStream *stream,
                       void *userData,
                       aaudio_result_t error) {
        assert(userData);
        auto *audioEngine = reinterpret_cast<AAudioEngine *>(userData);
        audioEngine->errorCallback(stream, error);
    }

    aaudio_data_callback_result_t AAudioEngine::dataCallback(AAudioStream *stream,
                                                             void *audioData,
                                                             int32_t numFrames) {
        assert(stream == mAudioStream);
        int size = AAsset_read(mAsset, audioData,
                               numFrames * mChannel * (mFormat == AAUDIO_FORMAT_PCM_I16 ? 2 : 1));
        LOGI("AAudioEngine::dataCallback, size:%d, numFrames:%d ", size, numFrames);
        if (size <= 0) {
            LOGI("AAudioEngine::dataCallback, file reach eof!!");
            return AAUDIO_CALLBACK_RESULT_STOP;
        }
        return AAUDIO_CALLBACK_RESULT_CONTINUE;
    }

    void AAudioEngine::errorCallback(AAudioStream *stream,
                                     aaudio_result_t  __unused error) {
        assert(stream == mAudioStream);
        LOGD("errorCallback result: %s", AAudio_convertResultToText(error));

        aaudio_stream_state_t streamState = AAudioStream_getState(mAudioStream);
        if (streamState == AAUDIO_STREAM_STATE_DISCONNECTED) {

            // Handle stream restart on a separate thread
            std::function<void(void)> restartStream = std::bind(&AAudioEngine::restartStream,
                                                                this);
            std::thread streamRestartThread(restartStream);
            streamRestartThread.detach();
        }
    }

#endif

    void AAudioEngine::restartStream() {

        LOGI("Restarting stream");

        if (mRestartingLock.try_lock()) {
            Stop();
            createPlaybackStream();
            Start();
            mRestartingLock.unlock();
        } else {
            LOGW("Restart stream operation already in progress - ignoring this request");
            // We were unable to obtain the restarting lock which means the restart operation is currently
            // active. This is probably because we received successive "stream disconnected" events.
            // Internal issue b/63087953
        }
    }

    AAudioEngine::AAudioEngine(AAssetManager *assetManager, const std::string &filePath,
                               uint32_t sampleRate, uint16_t channel,
                               uint32_t format) {
        mSampleRate = sampleRate;
        mChannel = channel;
        mFormat = format == 2 ? AAUDIO_FORMAT_PCM_I16 : AAUDIO_FORMAT_UNSPECIFIED;
        mAsset = AAssetManager_open(assetManager, filePath.c_str(), 0);
        LOGI("AAudioEngine::AAudioEngine, filePath:%s", filePath.c_str());
    }

    AAudioEngine::~AAudioEngine() {
        destroy();
    }


    bool AAudioEngine::Init() {
        createPlaybackStream();
#if  !AAUDIO_CALLBACK
        mPlayThread = std::thread(&AAudioEngine::workFunc, this);
#endif
        mValid = true;
        return true;
    }

    void AAudioEngine::Start() {
        if (mAudioStream) {
#if  !AAUDIO_CALLBACK
            std::unique_lock<std::mutex> lock(mPlayMutex);
#endif
            aaudio_result_t result = AAudioStream_requestStart(mAudioStream);
            if (result != AAUDIO_OK) {
                LOGE("Error starting stream. %s", AAudio_convertResultToText(result));
                return;
            }
            hasPlay = true;
#if  !AAUDIO_CALLBACK
            mPlayCV.notify_all();
#endif
        }
    }

    void AAudioEngine::Pause() {
        if (mAudioStream) {
#if  !AAUDIO_CALLBACK
            std::unique_lock<std::mutex> lock(mPlayMutex);
#endif
            aaudio_result_t result = AAudioStream_requestPause(mAudioStream);
            if (result != AAUDIO_OK) {
                LOGE("Error starting stream. %s", AAudio_convertResultToText(result));
                return;
            }
            hasPlay = false;
#if  !AAUDIO_CALLBACK
            mPlayCV.notify_all();
#endif
        }
    }

    void AAudioEngine::Stop() {
        if (mAudioStream && mValid) {
            LOGI("AAudioEngine::Stop, stop begin");
#if  !AAUDIO_CALLBACK
            std::unique_lock<std::mutex> lock(mPlayMutex);
#endif
            aaudio_result_t result = AAudioStream_requestStop(mAudioStream);
            if (result != AAUDIO_OK) {
                LOGE("Error starting stream. %s", AAudio_convertResultToText(result));
                return;
            }
            if (mAudioStream) {
                AAudioStream_close(mAudioStream);
                mAudioStream = nullptr;
            }
            hasPlay = false;
            mValid = false;
#if  !AAUDIO_CALLBACK
            mPlayCV.notify_all();
#endif
            LOGI("AAudioEngine::Stop, stop finish");
        }
    }

    void AAudioEngine::destroy() {
        LOGI("AAudioEngine::destroy, %d", mValid);
        if (mValid) {
            Stop();
        }
#if  !AAUDIO_CALLBACK
        mPlayThread.join();
#endif
        if (mAsset) {
            AAsset_close(mAsset);
            mAsset = nullptr;
        }

#if  !AAUDIO_CALLBACK
        if (mBufferData) {
            free(mBufferData);
            mBufferData = nullptr;
        }
#endif
    }

#if  !AAUDIO_CALLBACK
    void AAudioEngine::workFunc() {
        while (mValid) {
            {
                std::unique_lock<std::mutex> lock(mPlayMutex);
                while (!hasPlay && mValid) {
                    LOGI("1111111");
                    mPlayCV.wait(lock);
                    LOGI("2222222");
                }
            }
            if (!mValid) {
                break;
            }
            if (mBufferData == nullptr) {
                mBufferData = static_cast<uint8_t *>(calloc(1, BUFFER_SIZE));
            }
            int realSize = AAsset_read(mAsset, mBufferData, BUFFER_SIZE);
            if (realSize <= 0) {
                LOGI("AAudioEngine::workFunc, file eof!!!");
                break;
            }
            aaudio_result_t result = AAudioStream_write(mAudioStream, mBufferData,
                                                        realSize / (mChannel == 2 ? 2 : 1) /
                                                        (mFormat == AAUDIO_FORMAT_PCM_I16 ? 2
                                                                                          : 1),
                                                        TIMEOUT_NANO);
            LOGI("AAudioEngine::workFunc, result:%d", result);
        }
        LOGI("AAudioEngine::workFunc, Play thread exit!!");
    }
#endif

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
#if AAUDIO_CALLBACK
        AAudioStreamBuilder_setDataCallback(builder, aaudiodemo::dataCallback, this);
        AAudioStreamBuilder_setErrorCallback(builder, aaudiodemo::errorCallback, this);
#endif
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

    bool AAudioEngine::createPlaybackStream() {
        AAudioStreamBuilder *builder = createStreamBuilder();
        aaudio_result_t result = AAUDIO_ERROR_BASE;
        if (builder != nullptr) {
            setupPlaybackStreamParameters(builder);
            result = AAudioStreamBuilder_openStream(builder, &mAudioStream);
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
        return result == AAUDIO_OK && mAudioStream;
    }

}


