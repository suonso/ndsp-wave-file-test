#pragma once

#include <3ds.h>

struct waveInfo
{
  size_t dataStart;
  size_t dataSize;
  u32 sampleRate;
  u16 numberOfChannel;
  u32 samplesPerBuff;
  u32 bytesPerSample;
  u16 blockPerChannel;
};
