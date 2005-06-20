/***************************************************************************
 *            ContentDirectory.cpp
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *  Copyright (C) 2005 Ulrich Völkel
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
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
#include "ContentDirectory.h" 
#include "UPnPItem.h"
#include "AudioItem.h"
#include "../UPnPActions/UPnPBrowse.h"
#include "../win32.h"
#include "../SharedConfig.h"
 
#include <iostream>
#include <sstream>
#include <libxml/xmlwriter.h>
#include <cstdio>
#include <dirent.h>
#include <sys/types.h>
using namespace std;
 
CContentDirectory::CContentDirectory(): CUPnPService(udtContentDirectory)
{
  CStorageFolder* pBaseDir = new CStorageFolder();
  m_ObjectList["0"] = pBaseDir;
  
  BuildObjectList();
}

CContentDirectory::~CContentDirectory()
{
}
 
CHTTPMessage* CContentDirectory::HandleUPnPAction(CUPnPAction* pUPnPAction)
{
  string sContent = HandleUPnPBrowse((CUPnPBrowse*)pUPnPAction);  
  CHTTPMessage* pResult = new CHTTPMessage(http_200_ok, text_xml);
  pResult->SetContent(sContent);
  
  return pResult;
}

std::string CContentDirectory::GetFileNameFromObjectID(std::string p_sObjectID)
{ 
  m_ListIterator = m_ObjectList.find(p_sObjectID.c_str());
  if(m_ListIterator != m_ObjectList.end())      
    return ((CUPnPObject*)m_ObjectList[p_sObjectID.c_str()])->GetFileName();
  else
    return "";
}
 
std::string CContentDirectory::HandleUPnPBrowse(CUPnPBrowse* pUPnPBrowse)
{
  xmlTextWriterPtr writer;
	xmlBufferPtr buf;
	std::stringstream sTmp;	
	
	buf    = xmlBufferCreate();   
	writer = xmlNewTextWriterMemory(buf, 0);    
	xmlTextWriterStartDocument(writer, NULL, "UTF-8", NULL);
  
  // root  
  xmlTextWriterStartElementNS(writer, BAD_CAST "s", BAD_CAST "Envelope", NULL);    
  xmlTextWriterWriteAttributeNS(writer, BAD_CAST "s", 
    BAD_CAST "encodingStyle", 
    BAD_CAST  "http://schemas.xmlsoap.org/soap/envelope/", 
    BAD_CAST "http://schemas.xmlsoap.org/soap/encoding/");
   
    // body
    xmlTextWriterStartElementNS(writer, BAD_CAST "s", BAD_CAST "Body", NULL);    
  
      // browse response
      xmlTextWriterStartElementNS(writer, BAD_CAST "u",        
        BAD_CAST "BrowseResponse", 
        BAD_CAST "urn:schemas-upnp-org:service:ContentDirectory:1");
  
        // result
        xmlTextWriterStartElement(writer, BAD_CAST "Result");
      
        unsigned int nNumberReturned = 0;
        unsigned int nTotalMatches   = 0;
        m_ListIterator = m_ObjectList.find(pUPnPBrowse->m_sObjectID);
        if(m_ListIterator != m_ObjectList.end())
        {
          xmlTextWriterWriteString(writer, 
            BAD_CAST ((CStorageFolder*)m_ObjectList[pUPnPBrowse->m_sObjectID])->GetContentAsString(pUPnPBrowse, &nNumberReturned, &nTotalMatches).c_str());        
        }
      
        // end result
        xmlTextWriterEndElement(writer);
        
        // number returned
        xmlTextWriterStartElement(writer, BAD_CAST "NumberReturned");
        sTmp << nNumberReturned;
        xmlTextWriterWriteString(writer, BAD_CAST sTmp.str().c_str());
        sTmp.str("");
        xmlTextWriterEndElement(writer);
        
        // total matches
        xmlTextWriterStartElement(writer, BAD_CAST "TotalMatches");
        sTmp << nTotalMatches;
        xmlTextWriterWriteString(writer, BAD_CAST sTmp.str().c_str());
        sTmp.str("");
        xmlTextWriterEndElement(writer);
        
        // update id
        xmlTextWriterStartElement(writer, BAD_CAST "UpdateID");
        xmlTextWriterWriteString(writer, BAD_CAST "0");
        xmlTextWriterEndElement(writer);
  
      // end browse response
      xmlTextWriterEndElement(writer);
      
    // end body
    xmlTextWriterEndElement(writer);
   
	// end root
	xmlTextWriterEndElement(writer);
  xmlTextWriterEndDocument(writer);
	xmlFreeTextWriter(writer);
	
	std::stringstream output;
	output << (const char*)buf->content;
	
	xmlBufferFree(buf);
	return output.str();
}

void CContentDirectory::BuildObjectList()
{
  int nCount = 1;
  ScanDirectory(CSharedConfig::Shared()->GetBaseDir(), &nCount, ((CStorageFolder*)m_ObjectList["0"]));
}

void CContentDirectory::ScanDirectory(std::string p_sDirectory, int* p_nCount, CStorageFolder* pParentFolder)
{
  /*
   * ACHTUNG: Der Kram hier ist uebelst hingeschmiert :)
   * Da ich noch keine Ahnung habe, wie man unter C++ sauber einen
   * Verzeichnisbaum erstellt, erstmal diese Variante.
   */
  
  DIR*    pDir;
  DIR*    pTmp;
  dirent* pDirEnt;
  stringstream sTmp;
    
  if(p_sDirectory.substr(p_sDirectory.length()-1).compare(upnpPathDelim) != 0)
  {
    sTmp << p_sDirectory << upnpPathDelim;
    p_sDirectory = sTmp.str();
    sTmp.str("");
  }
  
  if((pDir = opendir(p_sDirectory.c_str())) != NULL)
  {
    while((pDirEnt = readdir(pDir)))
    {
      if(((string(".").compare(pDirEnt->d_name) != 0) && 
         (string("..").compare(pDirEnt->d_name) != 0)))
      {        
        sTmp << p_sDirectory << pDirEnt->d_name;        
        
        stringstream sObjId;
        
        string sExt = sTmp.str().substr(sTmp.str().length() - 3);
        if(sExt.compare("mp3") == 0)
        {
          CAudioItem* pTmpItem = new CAudioItem();
          char szObjId[10];                            
          sprintf(szObjId, "%010X", *p_nCount);
            
          pTmpItem->SetObjectID(szObjId);            
          pTmpItem->SetParent(pParentFolder);
          pTmpItem->SetName(pDirEnt->d_name);
          pTmpItem->SetFileName(sTmp.str());
        
          // add folder to list and parent folder
          m_ObjectList[szObjId] = pTmpItem;
          pParentFolder->AddUPnPObject(pTmpItem);
          
          // increment counter
          int nTmp = *p_nCount;
          nTmp++;
          *p_nCount = nTmp;         
        }
        else
        {
          if((pTmp = opendir(sTmp.str().c_str())) != NULL)
          {
            closedir(pTmp);
            
            // create folder object
            CStorageFolder* pTmpFolder = new CStorageFolder();
            
            char szObjId[10];                            
            sprintf(szObjId, "%010X", *p_nCount);            
            
            pTmpFolder->SetObjectID(szObjId);            
            pTmpFolder->SetParent(pParentFolder);
            pTmpFolder->SetName(pDirEnt->d_name);
            pTmpFolder->SetFileName(sTmp.str());
            
            // add folder to list and parent folder
            m_ObjectList[szObjId] = pTmpFolder;
            pParentFolder->AddUPnPObject(pTmpFolder);
            
            // increment counter
            int nTmp = *p_nCount;
            nTmp++;
            *p_nCount = nTmp;
            
            // scan subdirectories
            ScanDirectory(sTmp.str(), p_nCount, pTmpFolder); 
          }
        }
        sTmp.str("");
      }
    }    
    closedir(pDir);
  }
}