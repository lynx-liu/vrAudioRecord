#define LOG_TAG "VrAudioRecord"

#include "AudioRecordInterface.h"
#include <utils/Timers.h>

#include <sys/un.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/stat.h>
#include <malloc.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>

using namespace Streaming;

void onPcmDataCallback(FILE *fp_pcm, uint8_t *data, size_t size)
{
	ALOGD("%d: pcm size=%zu", __LINE__, size);
	if (fp_pcm != nullptr)
	{
		fwrite(data, size, 1, fp_pcm);
	}
}

// logcat -s VrAudioRecord
int main(void)
{
	uint16_t sampleRate = 48000;

    FILE* fp_pcm = fopen("/sdcard/audio.pcm", "wb");
    if (fp_pcm == nullptr) {
        ALOGE("fopen failed\n");
    }

    AudioRecordInterface *audioRecord = AudioRecordInterface::CreateAudioRecordInstance();
	if (audioRecord->initAudioRecord(sampleRate) < 0)
	{
		ALOGE("initAacEncoder failed\n");
		return -1;
	}
	system("service call audio 10 i32 3 i32 30 i32 0");

#if 1
	audioRecord->setPcmDataCallback([&](uint8_t *data, size_t size) {
		static nsecs_t start = 0;
		if (start == 0)
		{
			start = systemTime(SYSTEM_TIME_MONOTONIC);
		}
		else
		{
			nsecs_t end = systemTime(SYSTEM_TIME_MONOTONIC);
			nsecs_t delta = end - start;
			start = end;
			ALOGD("%d: pcm size=%zu, interval time=%ld ms", __LINE__, size, long(ns2ms(delta)));
		}

        if (fp_pcm != nullptr) {
            fwrite(data, size, 1, fp_pcm);
        }
	});
#else
	audioRecord->setPcmDataCallback(std::bind(&onPcmDataCallback, fp_pcm, std::placeholders::_1, std::placeholders::_2));
#endif

	ALOGD("Enter to exit: \n");
	getchar();

	audioRecord->closeAudioRecord();
	ALOGD("closeAudioRecord success\n");

	sleep(1);

    if (fp_pcm != nullptr) {
        fclose(fp_pcm);
        fp_pcm = nullptr;
    }

	ALOGD("fclose success\n");
	return 0;
}