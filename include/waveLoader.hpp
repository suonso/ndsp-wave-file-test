#pragma once

#include <fstream>
#include <string>

#include "Types.hpp"

// all we care is WAVE_FORMAT_PCM
typedef enum WAVE_FORMAT
{
  WAVE_FORMAT_PCM        = 0x0001,
  WAVE_FORMAT_IEEE_FLOAT = 0x0003,
  WAVE_FORMAT_ALAN       = 0x0006,
  WAVE_FORMAT_MULAW      = 0x0007,
  WAVE_FORMAT_EXTENSIBLE = 0xFFFE
} WAVE_FORMAT;

typedef enum neededChunk
{
  FMT_CHUNK,
  DATA_CHUNK,
  IGNORE_CHUNK,
} neededChunk;

// we don't need all of this but i'll throw it away later
typedef struct FMT_Chunk
{
  u32 cksize;// Chunk size: 16, 18 or 40
  u16 wFormatTag; // Format code
  u16 nChannels;// Number of interleaved channels
  u16 nSamplesPerSec;// Sampling rate (blocks per second)
  u16 nAvgBytesPerSec;// Data rate
  u16 nBlockAlign;// Data block size (bytes)
  u16 wBitsPerSample;// Bits per sample
  u16 cbSize;// Size of the extension (0 or 22)
  u16 wValidBitsPerSample;// Number of valid bits
  u16 dwChannelMask;// Speaker position mask
  char SubFormat[16];// GUID, including the data format code
} FMT_chunk;

typedef struct DATA_Chunk
{
  u32 cksize;// Chunk size = data + ckID - 4(bytes)
  size_t streamPointer;// not in spec, used for mark the begining of data stream
} DATA_Chunk;

bool readWaveFile(std::string filePath, std::ifstream & out, waveInfo & info);
