/*
 * done thanks to,
 * http://www.gamedev.net/page/resources/_/technical/game-programming/loading-a-wave-file-r709
 * &
 * http://www-mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html
 */

 #include "waveLoader.hpp"

 #include <fstream>
 #include <iostream>
 #include <string>
 #include <cstring>

 #include "Types.hpp"

u16 readBytesAsU16(std::ifstream & stream);
u32 readBytesAsU32(std::ifstream & stream);
bool readMasterChunk(std::ifstream & stream);
neededChunk readChunkId(std::ifstream & stream);
bool readFMTChunk(std::ifstream & stream, FMT_Chunk & cunkInfo);
bool readDataChunk(std::ifstream & stream, DATA_Chunk & chunkInfo);

 u16 readBytesAsU16(std::ifstream & stream)
 {
     char in[2] = {0,0};
     stream.read(in, sizeof(u16));
     return (u16)( in[0] + (u16)(in[1] << 8) );
 }

 u32 readBytesAsU32(std::ifstream & stream)
 {
     char in[4] = {0,0,0,0};
     stream.read(in, sizeof(u32));
     return (u32)( in[0] + (in[1] << 8) + (in[2] << 16) + (in[3] << 24) );
 }

neededChunk readChunkId(std::ifstream & stream)
{
  char chunkID[5] = {0,0,0,0,0};
  stream.read(chunkID, sizeof(char) * 4);

  if( strcmp(chunkID, "fmt ") == 0 )
  {
    return FMT_CHUNK;
  }
  else if( strcmp(chunkID, "data") == 0 )
  {
    return DATA_CHUNK;
  }
  else
  {
    return IGNORE_CHUNK;
  }
}

bool readMasterChunk(std::ifstream & stream)
{
  char ckID[5] = {0,0,0,0,0};
  stream.read(ckID, sizeof(char)*4);
  if( strcmp(ckID, "RIFF") != 0 ) { return false; }

  stream.ignore(sizeof(u32));

  stream.read(ckID, sizeof(char) * 4);
  if( strcmp(ckID, "WAVE") != 0 ) {return false;}

   return true;
}

bool readFMTChunk(std::ifstream & stream, FMT_chunk & chunkInfo)
{
  u32 ckSize = readBytesAsU32(stream);

  u16 wFormatTag = readBytesAsU16(stream);

  if( wFormatTag != WAVE_FORMAT_PCM ) { return false; }

  chunkInfo.nChannels = readBytesAsU16(stream);
  chunkInfo.nSamplesPerSec = readBytesAsU32(stream);
  chunkInfo.nAvgBytesPerSec = readBytesAsU32(stream);
  chunkInfo.nBlockAlign = readBytesAsU16(stream);
  chunkInfo.wBitsPerSample = readBytesAsU16(stream);

  if (ckSize == 16)
  {
    return true;
  }

  chunkInfo.cbSize = readBytesAsU16(stream);

  if(ckSize == 18)
  {
    return true;
  }

  chunkInfo.wValidBitsPerSample = readBytesAsU16(stream);
  chunkInfo.wValidBitsPerSample = readBytesAsU16(stream);
  chunkInfo.dwChannelMask = readBytesAsU32(stream);
  stream.read(chunkInfo.SubFormat, sizeof(char) * 4);

  return true;
}

bool readDataChunk(std::ifstream & stream, DATA_Chunk & chunkInfo)
{
  chunkInfo.cksize = readBytesAsU32(stream);
  chunkInfo.streamPointer = stream.tellg();
  return true;
}

bool readWaveFile(std::string filePath, std::ifstream & out, waveInfo & info)
{

  out.open(filePath, std::ios::binary);

  if(!out.is_open()) { return false; }

  if( !readMasterChunk(out) ) { return false; }

  FMT_Chunk fmtChunkInfo = {};
  DATA_Chunk dataChunkInfo = {};

  bool fmtChunkRead = false, dataChunkRead = false;

  neededChunk chunkType = IGNORE_CHUNK;

  // read until we found the data chunk
  do
  {
    chunkType = readChunkId(out);

    switch (chunkType)
    {
      case FMT_CHUNK:
      {
        if( readFMTChunk(out, fmtChunkInfo) ){ fmtChunkRead = true; }
        else { return false; }
      }
      break;

      case DATA_CHUNK:
      {
        if( readDataChunk(out, dataChunkInfo) ){ dataChunkRead = true;}
        else { return false; }
      }
      break;

      case IGNORE_CHUNK:
      default:
      {
        u32 chunkSizeToIgnore = readBytesAsU32(out);
        out.ignore(chunkSizeToIgnore);
      }
      break;
    }
  }while( (chunkType != DATA_CHUNK) && (!out.eof()) );

  if( fmtChunkRead && dataChunkRead )
  {
    if( fmtChunkInfo.wBitsPerSample != 0 )
        { info.bytesPerSample = fmtChunkInfo.wBitsPerSample >> 3; }
    else
      { return false; }

    info.dataStart = dataChunkInfo.streamPointer;
    info.numberOfChannel = fmtChunkInfo.nChannels;
    info.sampleRate = fmtChunkInfo.nSamplesPerSec;
    info.samplesPerBuff = info.sampleRate / 30;
    info.dataSize = dataChunkInfo.cksize;
    info.blockPerChannel = fmtChunkInfo.nBlockAlign;

    return true;
  }
  else
  {
    return false;
  }
}
