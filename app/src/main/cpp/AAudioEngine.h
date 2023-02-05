//
// Created by dahaimain on 2023/2/4.
//

#ifndef AAUDIODEMO_AAUDIOENGINE_H
#define AAUDIODEMO_AAUDIOENGINE_H

#include <aaudio/AAudio.h>
#include <assert.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <string>
#include "logging_macros.h"
#include <thread>
#include <mutex>
#include <condition_variable>

namespace aaudiodemo {
    class AAudioEngine {
    public:
        AAudioEngine(AAssetManager *assetManager, const std::string &filePath, uint32_t sampleRate,
                     uint16_t channel, uint32_t format);

        ~AAudioEngine();

        AAudioEngine(const AAudioEngine &) = delete;

        AAudioEngine &operator=(const AAudioEngine &) = delete;

    public:
        /**
         * 初始化播放引擎
         * @return 初始化结果
         */
        bool Init();

        /**
         * 开始播放
         */
        void Start();

        /**
         * 暂停播放
         */
        void Pause();

        /**
         * 停止播放
         */
        void Stop();


    private:
        bool hasPlay{false};
        bool mValid{false};
        int32_t mSampleRate{44100};
        int16_t mChannel{2};
        aaudio_format_t mFormat{AAUDIO_FORMAT_PCM_I16};
        ///pcm file path
        AAudioStream *mAudioStream{nullptr};

        AAsset *mAsset{nullptr};
        uint8_t *mBufferData{nullptr};
        std::condition_variable mPlayCV;
        std::mutex mPlayMutex;
        std::thread mPlayThread;
    private:
        void workFunc();

        AAudioStreamBuilder *createStreamBuilder();

        bool createPlaybackStream();

        void setupPlaybackStreamParameters(AAudioStreamBuilder *builder);

        void destroy();


    };

}
#endif //AAUDIODEMO_AAUDIOENGINE_H
