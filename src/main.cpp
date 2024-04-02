#include "AudioRecordInterface.h"

#include <stdio.h>
#include <stdlib.h>

using namespace Streaming;

void onPcmDataCallback(FILE *fp_pcm, uint8_t *data, size_t size)
{
	printf("pcm size=%zu\n", size);
	if (fp_pcm != nullptr)
	{
		fwrite(data, size, 1, fp_pcm);
	}
}

int main(void)
{
	uint16_t sampleRate = 48000;

    FILE* fp_pcm = fopen("/sdcard/audio_dump.pcm", "wb");
    if (fp_pcm == nullptr) {
        printf("fopen failed\n");
    }

    AudioRecordInterface *audioRecord = AudioRecordInterface::CreateAudioRecordInstance();
	if (audioRecord->initAudioRecord(sampleRate) < 0)
	{
		printf("initAacEncoder failed\n");
		return -1;
	}
	system("service call audio 10 i32 3 i32 30 i32 0");

#if 1
	audioRecord->setPcmDataCallback([&](uint8_t *data, size_t size) {
		printf("pcm size=%zu\n", size);

        if (fp_pcm != nullptr) {
            fwrite(data, size, 1, fp_pcm);
        }
	});
#else
	audioRecord->setPcmDataCallback(std::bind(&onPcmDataCallback, fp_pcm, std::placeholders::_1, std::placeholders::_2));
#endif

	printf("Enter to exit: \n");
	getchar();

	audioRecord->closeAudioRecord();
	printf("closeAudioRecord success\n");

	sleep(1);

    if (fp_pcm != nullptr) {
        fclose(fp_pcm);
        fp_pcm = nullptr;
    }

	printf("fclose success\n");
	return 0;
}