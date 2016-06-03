#include <3ds.h>
#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>
#include <fstream>
#include <cstdio>

#include "waveLoader.hpp"
#include "Types.hpp"

void fillBuffer(u8 * bufferToFill, size_t size, std::ifstream & stream);

void fillBuffer(u8 * bufferToFill, size_t size, std::ifstream & stream)
{
	stream.read(reinterpret_cast<char *>(bufferToFill), size);
	DSP_FlushDataCache(bufferToFill, size);
}

int main(int argc, char* argv[])
{
	gfxInitDefault();

	consoleInit(GFX_TOP, nullptr);

	// The dsp channel number
	constexpr int channel = 0;

	u8 fillBlock = 0;

	// Initialize ndsp
	ndspInit();

  std::ifstream fileStream;
  waveInfo audioInfo;

  bool success = readWaveFile(std::string("test.wav"), fileStream, audioInfo);

  if( !success )
  {
    gfxExit();
  	ndspExit();
    return 1;
  }

	// 512KB (or sighltly less) for each buffer so about 1Mo for the whole buffer
	u32 samplesPerBuff = 512000 / audioInfo.bytesPerSample;
	size_t halfBufferSize = samplesPerBuff * audioInfo.bytesPerSample;

  u8 * data = static_cast<u8 *>(linearAlloc(halfBufferSize * 2));

	if( data == NULL || data == nullptr )
	{
		std::cout << "allocation failed" << std::endl;
		gfxExit();
		ndspExit();
		return 1;
	}

  u16 ndspFormat;

	if(audioInfo.bytesPerSample == 1)
	{
		ndspFormat = (audioInfo.numberOfChannel == 1) ?
			NDSP_FORMAT_MONO_PCM8 :
			NDSP_FORMAT_STEREO_PCM8;
	}
	else
	{
		ndspFormat = (audioInfo.numberOfChannel == 1) ?
			NDSP_FORMAT_MONO_PCM16 :
			NDSP_FORMAT_STEREO_PCM16;
	}

	ndspChnReset(channel);
	ndspSetOutputMode(NDSP_OUTPUT_STEREO);
	ndspChnSetInterp(channel, NDSP_INTERP_NONE);
	ndspChnSetRate(channel, float(audioInfo.sampleRate));
	ndspChnSetFormat(channel, ndspFormat);

	// set the volume for output
  float mix[12];
  memset(mix, 0, sizeof(mix));
  mix[0] = 1.0;
  mix[1] = 1.0;
  ndspChnSetMix(channel, mix);

	// Create and play a wav buffer
	ndspWaveBuf waveBuf[2];
	std::memset(&waveBuf, 0, sizeof(ndspWaveBuf));

	waveBuf[0].data_vaddr = reinterpret_cast<u32 *>(&data[0]);
	waveBuf[0].nsamples = samplesPerBuff;
	waveBuf[0].looping = false;
	waveBuf[1].data_vaddr = reinterpret_cast<u32 *>(&data[samplesPerBuff]);
	waveBuf[1].nsamples = samplesPerBuff;
	waveBuf[1].looping = false;

	size_t readedSize = 0;

	fillBuffer(data, halfBufferSize * 2, fileStream);
	readedSize += (halfBufferSize * 2);

	ndspChnWaveBufAdd(channel, &waveBuf[0]);
	ndspChnWaveBufAdd(channel, &waveBuf[1]);

	while(aptMainLoop())
	{
		hidScanInput();

		u32 keys = hidKeysDown();

		if(keys & KEY_START)
			break;

		if(waveBuf[fillBlock].status == NDSP_WBUF_DONE)
		{
			double sizeleft = (double)audioInfo.dataSize - (double)readedSize;
			size_t toRead;
			if(sizeleft > 0 && sizeleft < halfBufferSize)
			{
				toRead = sizeleft;
			}
			else if(sizeleft > 0)
			{
				toRead = halfBufferSize;
			}
			else
			{
				toRead = 0;
			}

			if(toRead > 0)
			{
				fillBuffer(reinterpret_cast<u8 *>(waveBuf[fillBlock].data_pcm16), toRead, fileStream);
				ndspChnWaveBufAdd(channel, &waveBuf[fillBlock]);
				readedSize += toRead;
			}

			// http://www.catonmat.net/blog/low-level-bit-hacks-you-absolutely-must-know/
			// bit hack number 5
			// tldr : 0 xor 1 = 1, 1 xor 1 = 0
			// HOW I DIDN'T TOUGHT ABOUT THAT !!
			// can be simplified to fillBlock ^= 1;
			fillBlock = fillBlock ^ 1;
		}

		gfxFlushBuffers();
		gfxSwapBuffers();
		gspWaitForVBlank();
	}

	ndspChnWaveBufClear(channel);

	linearFree(data);

	gfxExit();
	ndspExit();

	return 0;
}
