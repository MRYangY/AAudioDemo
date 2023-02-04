//
// Created by dahaimain on 2023/2/4.
//

#ifndef AAUDIODEMO_AAUDIOENGINE_H
#define AAUDIODEMO_AAUDIOENGINE_H

#include <aaudio/AAudio.h>
#include <string>
#include "logging_macros.h"
#include <thread>
#include <mutex>
#include <condition_variable>

namespace aaudiodemo {
    class AAudioEngine {
    public:
        AAudioEngine(const std::string &filePath);

        ~AAudioEngine();

        AAudioEngine(const AAudioEngine &) = delete;

        AAudioEngine &operator=(const AAudioEngine &) = delete;

    public:
        void Start();

        void Pause();

        void Stop();

        void Destroy();

    private:
        int32_t mSampleRate{44100};
        int16_t mChannel{2};
        aaudio_format_t mFormat{AAUDIO_FORMAT_PCM_I16};
        ///pcm file path
        std::string mFilePath;
        AAudioStream *mAudioStream{nullptr};
    private:
        void workFunc();

        AAudioStreamBuilder *createStreamBuilder();

        void createPlaybackStream();

        void setupPlaybackStreamParameters(AAudioStreamBuilder *builder);

    private:
        FILE *file{nullptr};
        std::condition_variable mPlayCV;
        std::mutex mPlayMutex;
        std::thread mPlayThread;
    };

}
#endif //AAUDIODEMO_AAUDIOENGINE_H
