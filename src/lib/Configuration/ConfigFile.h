/***************************************************************************
 *            ConfigFile.h
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2007 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as 
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef _CONFIGFILE_H
#define _CONFIGFILE_H

#include "../Common/XMLParser.h"
#include <vector>

const std::string NEEDED_CONFIGFILE_VERSION = "0.7.2";

typedef enum CONFIG_FILE_ERROR {
  CF_OK,
  CF_FILE_NOT_FOUND,
  CF_PARSE_ERROR,
  CF_CONFIG_DEPRECATED  
} CONFIG_FILE_ERROR;

class CConfigFile
{
  public:
    CConfigFile();
    ~CConfigFile();

    int Load(std::string p_sFileName, std::string* p_psErrorMsg);
  
    // shared dir
    int SharedDirCount() { return m_lSharedDirs.size(); }
    std::string SharedDir(int p_nIdx) { return m_lSharedDirs[p_nIdx]; }
    void AddSharedDir(std::string p_sDirName);
    void RemoveSharedDir(int p_nIdx);
  
    // shared iTunes
    int SharedITunesCount() { return m_lSharedITunes.size(); }
    std::string SharedITunes(int p_nIdx) { return m_lSharedITunes[p_nIdx]; }
    void AddSharedITunes(std::string p_sITunesName);
    void RemoveSharedITunes(int p_nIdx);
  
    // network
    std::string NetInterface() { return m_sNetInterface; }
    void        NetInterface(std::string p_sNetInterface);
    int         HttpPort() { return m_nHttpPort; }
    void        HttpPort(int p_nHttpPort);
  
    // allowed ip
    int         AllowedIpsCount() { return m_lAllowedIps.size(); }
    std::string AllowedIp(int p_nIdx) { return m_lAllowedIps[p_nIdx]; }
    void AddAllowedIp(std::string p_sIpAddress);
    void RemoveAllowedIp(int p_nIdx);
    
    // content_directory
    std::string LocalCharset() { return m_sLocalCharset; }
    void        LocalCharset(std::string p_sLocalCharset);
    bool        UseImageMagick() { return m_bUseImageMagick; }
    bool        UseTaglib() { return m_bUseTaglib; }
    bool        UseLibAvFormat() { return m_bUseLibAvFormat; }
  
    // transcoding  
    std::string  AudioEncoder() { return m_sAudioEncoder; }
    bool         TranscodeVorbis() { return m_bTranscodeVorbis; }
    bool         TranscodeMusePack() { return m_bTranscodeMusePack; }
    bool         TranscodeFlac() { return m_bTranscodeFlac; }
  
    bool WriteDefaultConfig(std::string p_sFileName);
  
  private:
    CXMLDocument* m_pDoc;
  
    void ReadSharedObjects();
    void ReadNetworkSettings();
    void SetupDeviceIdentificationMgr(CXMLNode* pDeviceSettingsNode);
  
    // shared objects
    std::vector<std::string>  m_lSharedDirs;
    std::vector<std::string>  m_lSharedITunes;
  
    // network
    std::string               m_sNetInterface;    
    int                       m_nHttpPort;
    std::vector<std::string>  m_lAllowedIps;
  
    // content_directory
    std::string               m_sLocalCharset;
    bool                      m_bUseImageMagick;
    bool                      m_bUseTaglib;
    bool                      m_bUseLibAvFormat;

    // transcoding
    std::string               m_sAudioEncoder;
    bool                      m_bTranscodeVorbis;
    bool                      m_bTranscodeMusePack;
    bool                      m_bTranscodeFlac;  
};

#endif // _CONFIGFILE_H
