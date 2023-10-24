// #pragma once
#ifndef __VR_AUDIOR_RECORD__
#define __VR_AUDIOR_RECORD__

#include <media/AudioRecord.h>

#include <math.h>

#include <errno.h>
#include <vector>
#include <iostream>
#include <functional>
#include <utils/String16.h>
#include "AudioRecordInterface.h"

using namespace android;
using android::String16;
namespace Streaming
{

    class VrAudioRecord : public AudioRecordInterface
    {
    public:
        VrAudioRecord(void){};

        virtual ~VrAudioRecord()
        {
            ALOGD("~VrAudioRecord");
        }

        int initAudioRecord(uint16_t sampleRate) override;

        void closeAudioRecord(void) override;

        bool threadLoop(void);

        void setPcmDataCallback(const DataCallback &cb) override
        {
            mDataCallback = cb;
        };

        void setPcmDataCallback(const DataCallback &&cb) override
        {
            mDataCallback = std::move(cb);
        };

    private:
        static void AudioRecordCallback(int event, void *user, void *info);

        bool isValidVoice(const unsigned char *pcmdata, size_t size);

    private:
        enum
        {
            UNINITIALIZED,
            INIT,
            RUNNING,
            STOPPING,
            STOPPED
        } mState;

        android::AudioRecord *m_AudioRecord;
        audio_source_t m_inputSource = AUDIO_SOURCE_REMOTE_SUBMIX;
        audio_format_t m_audioFormat = AUDIO_FORMAT_PCM_16_BIT;
        audio_channel_mask_t m_channelConfig = AUDIO_CHANNEL_IN_STEREO;

        int m_sampleRateInHz = 48000;
        int m_audio_frameSize = 1;
        int m_audio_bufferSizeInBytes = 0;
        int m_iNotificationPeriodInFrames = 0;

        std::vector<char> mPcmOutputBuf;
        DataCallback mDataCallback;
    };

};

#endif