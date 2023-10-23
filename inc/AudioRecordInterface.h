#pragma once

#include <utils/Log.h>
#include <utils/Errors.h>
#include <iostream>

namespace Streaming
{

    using DataCallback = std::function<void(uint8_t *data, size_t size)>;

    class AudioRecordInterface
    {
    public:
        AudioRecordInterface(){};

        virtual ~AudioRecordInterface(){};

        virtual int initAudioRecord(uint16_t sampleRate) = 0;

        virtual void closeAudioRecord(void) = 0;

        virtual void setPcmDataCallback(const DataCallback &cb) = 0;

        virtual void setPcmDataCallback(const DataCallback &&cb) = 0;

        static AudioRecordInterface *CreateAudioRecordInstance(void);
    };

};