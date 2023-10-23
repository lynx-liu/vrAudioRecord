#define LOG_TAG "VrAudioRecord"

#include "VrAudioRecord.h"

#include <thread>

using namespace Streaming;
using namespace android;

int VrAudioRecord::setAudioParams(int sampleRate)
{
    m_sampleRateInHz = sampleRate;
    m_audioFormat = AUDIO_FORMAT_PCM_16_BIT;
    m_channelConfig = AUDIO_CHANNEL_IN_STEREO;
    m_inputSource = AUDIO_SOURCE_REMOTE_SUBMIX;
    return 0;
}

void VrAudioRecord::AudioRecordCallback(int event, void *user, void *info)
{
    if (event == android::AudioRecord::EVENT_NEW_POS)
    {

    }
    else if (event == android::AudioRecord::EVENT_MORE_DATA)
    {
        android::AudioRecord::Buffer *pBuff = (android::AudioRecord::Buffer *)info;
        pBuff->size = 0;
    }
    else if (event == android::AudioRecord::EVENT_OVERRUN)
    {
        ALOGE("audio EVENT_OVERRUN\n");
    }
}

int VrAudioRecord::initAudioRecord(uint16_t sampleRate)
{
    size_t minFrameCount = 0;

    m_audioFormat = AUDIO_FORMAT_PCM_16_BIT;
    m_channelConfig = AUDIO_CHANNEL_IN_STEREO;
    m_inputSource = AUDIO_SOURCE_REMOTE_SUBMIX;


    status_t status = AudioRecord::getMinFrameCount(
        &minFrameCount, m_sampleRateInHz, m_audioFormat, m_channelConfig);
    if (status != android::NO_ERROR)
    {
        ALOGE("%s  AudioRecord.getMinFrameCount fail \n", __FUNCTION__);
        return -1;
    }
    m_audio_bufferSizeInBytes = minFrameCount * m_audio_frameSize;
    ALOGI("minFrameCount=%zu, m_audio_bufferSizeInBytes=%d\n", minFrameCount, m_audio_bufferSizeInBytes);

    m_iNotificationPeriodInFrames = m_sampleRateInHz / 10;
    m_AudioRecord = new android::AudioRecord(String16("Vrviu_audio_pcm"));
    if (nullptr == m_AudioRecord)
    {
        ALOGE("create native AudioRecord failed!\n");
        return -1;
    }

    m_AudioRecord->set(m_inputSource,
                       m_sampleRateInHz,
                       m_audioFormat,
                       m_channelConfig,
                       0,
                       VrAudioRecord::AudioRecordCallback,
                       nullptr,
                       0,
                       true,
                       (audio_session_t)0);

    if (m_AudioRecord->initCheck() != android::NO_ERROR)
    {
        ALOGE("AudioTrack initCheck error!\n");
        if (m_AudioRecord != nullptr)
        {
            m_AudioRecord->stop();
            m_AudioRecord = nullptr;
        }
        return -1;
    }

    if (m_AudioRecord->setPositionUpdatePeriod(m_iNotificationPeriodInFrames) != android::NO_ERROR)
    {
        ALOGE("AudioTrack setPositionUpdatePeriod error!\n");
        return -3;
    }

    mPcmOutputBuf.resize(m_audio_bufferSizeInBytes, 0);

    std::thread t(&VrAudioRecord::threadLoop, this);
    t.detach();

    ALOGD("initRecord success, m_sampleRateInHz=%d, m_channelConfig=0x%x\n", m_sampleRateInHz, m_channelConfig);

    return 0;
}

void VrAudioRecord::closeAudioRecord(void)
{
    mState = STOPPING;

    if (m_AudioRecord != nullptr)
    {
        m_AudioRecord->stop();
    }

    mPcmOutputBuf.clear();
}

bool VrAudioRecord::isValidVoice(const unsigned char *pcmdata, size_t size)
{
    int db = 0;
    short int value = 0;
    double sum = 0;
    for (int i = 0; i < size; i += 2)
    {
        memcpy(&value, pcmdata + i, 2);
        sum += abs(value);
    }
    sum = sum / (size / 2);

    if (sum > 0)
    {
        db = (int)(20.0 * log10(sum));
    }

    if (db >= 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool VrAudioRecord::threadLoop(void)
{
    size_t readSize = 0;
    mState = RUNNING;

    ALOGD("into threadLoop\n");

    if (m_AudioRecord == nullptr)
    {
        return false;
    }

    if (m_AudioRecord->start() != android::NO_ERROR)
    {
        ALOGE("AudioTrack start error!\n");
        return false;
    }

    while (mState == RUNNING)
    {
        readSize = m_AudioRecord->read(&mPcmOutputBuf[0], m_audio_bufferSizeInBytes);
        // ALOGD("readSize=%zu, m_audio_bufferSizeInBytes=%d\n", readSize, m_audio_bufferSizeInBytes);
        if (readSize == WOULD_BLOCK || readSize == BAD_VALUE)
        {
            ALOGE("read error, readSize=%zu\n", readSize);
            continue;
        }

        if (!isValidVoice(reinterpret_cast<uint8_t *>(&mPcmOutputBuf[0]), readSize))
        {
            continue;//mute
        }

        if (mDataCallback != nullptr)
        {
            mDataCallback(reinterpret_cast<uint8_t *>(&mPcmOutputBuf[0]), readSize);
        }
    }

    mState = STOPPED;
    ALOGD("exit threadLoop\n");
    return false; // stop
}

AudioRecordInterface *AudioRecordInterface::CreateAudioRecordInstance(void)
{
    static VrAudioRecord sVrAudioRecord;
    return static_cast<AudioRecordInterface *>(&sVrAudioRecord);
}