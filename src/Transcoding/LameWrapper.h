/***************************************************************************
 *            LameWrapper.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
#ifndef _LAMEWRAPPER_H
#define _LAMEWRAPPER_H

#include <lame/lame.h>
#include <dlfcn.h>
#include <string>


extern "C"
{ 
  typedef lame_global_flags LameGlobalFlags;
  
  /* lame_global_flags* lame_init() */
  typedef lame_global_flags* (*LameInit_t)();
  /* const char* get_lame_version() */  
  typedef const char*        (*LameGetVersion_t)();
  /* void lame_init_params(lame_global_flags*) */
  typedef void               (*LameInitParams_t)(lame_global_flags*);
  /* void lame_print_config(lame_global_flags*) */
  typedef void               (*LamePrintConfig_t)(lame_global_flags*);
  
  typedef void               (*LameSetCompressionRatio_t)(lame_global_flags*, float);
  
  /* int lame_encode_buffer(lame_global_flags* gf, 
                            short int leftpcm[],
                            short int rightpcm[],
                            int num_samples,
                            char *mp3buffer,
                            int mp3buffer_size) */
  
  typedef int (*LameEncodeBufferInterleaved_t)(lame_global_flags*, short int[], int, char*, int);                    
               
  //lame_encode_flush(gf, mp3buffer, LAME_MAXMP3BUFFER);
  typedef int (*LameEncodeFlush_t)(lame_global_flags*, char*, int);

                      //lame_encode_buffer_interleaved(gf, pcmout, samplesRead, mp3buffer, LAME_MAXMP3BUFFER);
  
}

typedef enum tagLAME_BITRATE
{
  LAME_BITRATE_320 = 3,
  LAME_BITRATE_256 = 5,
  LAME_BITRATE_192 = 7,
  LAME_BITRATE_160 = 9,
  LAME_BITRATE_128 = 11
}LAME_BITRATE;

/*
  1. create
  2. call LoadLibrary()
  3. set compression ratio an other properties
  4. call Init()
  5. encoding using EncodeInterleaved() oder Encode()
  6. flush calling Flush()
  7. LameWrapper will clean p when it's deleted
*/ 

class CLameWrapper
{  
  public:
    CLameWrapper();
    bool LoadLibrary();
  
    void Init();
    void PrintConfig();
    std::string GetVersion();
    void SetBitrate(LAME_BITRATE p_nBitrate);
  
    int   EncodeInterleaved(short int p_PcmIn[], int p_nNumSamples);
    int   Flush();
    unsigned char* GetMp3Buffer() { return m_sMp3Buffer; }
    
    
  private:
    void*            m_LibHandle;
    LameGlobalFlags* m_LameGlobalFlags;
    unsigned char    m_sMp3Buffer[LAME_MAXMP3BUFFER];
  
    LameInit_t        m_LameInit;
    LameGetVersion_t  m_LameGetVersion;
    LameInitParams_t  m_LameInitParams; 
    LamePrintConfig_t m_LamePrintConfig;
  
    LameSetCompressionRatio_t     m_LameSetCompressionRatio;
  
    LameEncodeBufferInterleaved_t m_LameEncodeBufferInterleaved;
    LameEncodeFlush_t             m_LameEncodeFlush;
};

#endif /* _LAMEWRAPPER_H */