/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            UPnPActionFactory.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005-2010 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
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

#include "UPnPActionFactory.h"
#include "../Common/Common.h"
#include "../Common/RegEx.h"
#include "../SharedLog.h"


#include <iostream>
#include <libxml/parser.h>
#include <libxml/tree.h>

using namespace std;

CUPnPAction* CUPnPActionFactory::buildActionFromString(std::string p_sContent, CDeviceSettings* pDeviceSettings, std::string vfolderLayout)
{
  xmlDocPtr pDoc = NULL;
  pDoc = xmlReadMemory(p_sContent.c_str(), p_sContent.length(), "", NULL, 0);
  if(!pDoc) {
    cout << "error parsing action" << endl;
		return NULL;
  }
	
  xmlNode* pRootNode = NULL;  
  xmlNode* pTmpNode  = NULL;   
  pRootNode = xmlDocGetRootElement(pDoc);
    
  pTmpNode = NULL;
	
  // get first first-level child element
  for(pTmpNode = pRootNode->children; pTmpNode; pTmpNode = pTmpNode->next) {
    if(pTmpNode->type == XML_ELEMENT_NODE)
      break;
  }
	
	if(!pTmpNode) {
		xmlFreeDoc(pDoc);
		return NULL;
	}

  // get first second-level element
  for(pTmpNode = pTmpNode->children; pTmpNode; pTmpNode = pTmpNode->next) {
    if(pTmpNode->type == XML_ELEMENT_NODE)
      break;
  }

  
  CUPnPAction* pAction = NULL;
	if(!pTmpNode->nsDef) {
		cout << "malformed xml" << endl;
		return NULL;
	}
  string sNs   = (char*)pTmpNode->nsDef->href;
	string sName = (char*)pTmpNode->name;


	// ContentDirectory
	if(sNs.compare("urn:schemas-upnp-org:service:ContentDirectory:1") == 0)	{
    if(sName.compare("Browse") == 0) {
      pAction = new CUPnPBrowse(p_sContent);
      pAction->DeviceSettings(pDeviceSettings);
      parseBrowseAction((CUPnPBrowse*)pAction);
    }
	  else if(sName.compare("Search") == 0) {
	    pAction = new CUPnPSearch(p_sContent);
      pAction->DeviceSettings(pDeviceSettings);
		  parseSearchAction((CUPnPSearch*)pAction);
	  }
    else if(sName.compare("GetSearchCapabilities") == 0) {
      pAction = new CUPnPAction(UPNP_SERVICE_CONTENT_DIRECTORY, UPNP_GET_SEARCH_CAPABILITIES, p_sContent);      
      pAction->DeviceSettings(pDeviceSettings);
    }
    else if(sName.compare("GetSortCapabilities") == 0) {
      pAction = new CUPnPAction(UPNP_SERVICE_CONTENT_DIRECTORY, UPNP_GET_SORT_CAPABILITIES, p_sContent);
      pAction->DeviceSettings(pDeviceSettings);
    }
		else if(sName.compare("GetSortExtensionCapabilities") == 0) {
      pAction = new CUPnPAction(UPNP_SERVICE_CONTENT_DIRECTORY, UPNP_GET_SORT_EXTENSION_CAPABILITIES, p_sContent);
      pAction->DeviceSettings(pDeviceSettings);
    }
    else if(sName.compare("GetSystemUpdateID") == 0) {
      pAction = new CUPnPAction(UPNP_SERVICE_CONTENT_DIRECTORY, UPNP_GET_SYSTEM_UPDATE_ID, p_sContent);
      pAction->DeviceSettings(pDeviceSettings);
    }  
    /*else if(sName.compare("GetProtocolInfo") == 0) {
      pAction = new CUPnPAction(UPNP_SERVICE_CONTENT_DIRECTORY, UPNP_GET_PROTOCOL_INFO, p_sContent);
      pAction->DeviceSettings(pDeviceSettings);
    }*/
    else if(sName.compare("DestroyObject") == 0) {
      pAction = new CUPnPAction(UPNP_SERVICE_CONTENT_DIRECTORY, UPNP_DESTROY_OBJECT, p_sContent);
      pAction->DeviceSettings(pDeviceSettings);
      parseDestroyObjectAction(pAction);
    }
    else {
      cout << "unhandled ContentDirectory ACTION: " << sName << endl;
      pAction = NULL;
		}
    
	}
	
	// Connection Manager
	else if(sNs.compare("urn:schemas-upnp-org:service:ConnectionManager:1") == 0)
	{
		if(sName.compare("GetProtocolInfo") == 0) {
		  pAction = new CUPnPAction(UPNP_SERVICE_CONNECTION_MANAGER, CMA_GET_PROTOCOL_INFO, p_sContent);
		}
		/*else if(sName.compare("PrepareForConnection") == 0) {
			pAction = new CUPnPAction(UPNP_SERVICE_CONNECTION_MANAGER, CMA_PREPARE_FOR_CONNECTION, p_sContent);
		}
		else if(sName.compare("ConnectionComplete") == 0) {
			pAction = new CUPnPAction(UPNP_SERVICE_CONNECTION_MANAGER, CMA_CONNECTION_COMPLETE, p_sContent);
		}*/
		else if(sName.compare("GetCurrentConnectionIDs") == 0) {
			pAction = new CUPnPAction(UPNP_SERVICE_CONNECTION_MANAGER, CMA_GET_CURRENT_CONNECTION_IDS, p_sContent);
		}
		else if(sName.compare("GetCurrentConnectionInfo") == 0) {
			pAction = new CUPnPAction(UPNP_SERVICE_CONNECTION_MANAGER, CMA_GET_CURRENT_CONNECTION_INFO, p_sContent);
		}
		else {	
			pAction = new CUPnPAction(UPNP_SERVICE_CONNECTION_MANAGER, CMA_UNKNOWN, p_sContent);			
		}

	}
	
	// XMSMediaReceiverRegistrar
	else if(sNs.compare("urn:microsoft.com:service:X_MS_MediaReceiverRegistrar:1") == 0)
	{
	  if(sName.compare("IsAuthorized") == 0) {
	    pAction = new CUPnPAction(UPNP_SERVICE_X_MS_MEDIA_RECEIVER_REGISTRAR, UPNP_IS_AUTHORIZED, p_sContent);      
	  }
	  else if(sName.compare("IsValidated") == 0) {
	    pAction = new CUPnPAction(UPNP_SERVICE_X_MS_MEDIA_RECEIVER_REGISTRAR, UPNP_IS_VALIDATED, p_sContent);      
	  }
		else if(sName.compare("RegisterDevice") == 0) {
	    pAction = new CUPnPAction(UPNP_SERVICE_X_MS_MEDIA_RECEIVER_REGISTRAR, UPNP_REGISTER_DEVICE, p_sContent);      
	  }
		else {
      cout << "unhandled XMS ACTION: " << sName << endl;
      pAction = NULL;
		}

	}

  // fuppes soap control
 	else if(sNs.compare("urn:fuppesControl") == 0) {

    cout << "FUPPES SOAP CONTROL ACTION: " << sName << endl;

    if(sName.compare("DatabaseRebuild") == 0) {
	    pAction = new FuppesCtrlAction(FUPPES_CTRL_DATABASE_REBUILD, p_sContent);
	  }
    else if(sName.compare("DatabaseUpdate") == 0) {
	    pAction = new FuppesCtrlAction(FUPPES_CTRL_DATABASE_UPDATE, p_sContent);
	  }
    else if(sName.compare("VfolderRebuild") == 0) {
	    pAction = new FuppesCtrlAction(FUPPES_CTRL_VFOLDER_REBUILD, p_sContent);
	  }


    else if(sName.compare("GetDir") == 0) {
	    pAction = new FuppesCtrlAction(FUPPES_CTRL_GET_DIR, p_sContent);
	  }
    else if(sName.compare("GetSharedObjects") == 0) {
	    pAction = new FuppesCtrlAction(FUPPES_CTRL_GET_SHARED_OBJECTS, p_sContent);
	  }
    else if(sName.compare("AddSharedObject") == 0) {
	    pAction = new FuppesCtrlAction(FUPPES_CTRL_ADD_SHARED_OBJECT, p_sContent);
	  }
    else if(sName.compare("DelSharedObject") == 0) {
	    pAction = new FuppesCtrlAction(FUPPES_CTRL_DEL_SHARED_OBJECT, p_sContent);
	  }

/* 
  FUPPES_CTRL_MOD_SHARED_OBJECT,*/     
     
    else if(sName.compare("Test") == 0) {
	    pAction = new FuppesCtrlAction(FUPPES_CTRL_TEST, p_sContent);
	  }
    else {
	    pAction = new FuppesCtrlAction(FUPPES_CTRL_UNKNOWN, p_sContent);
    }

	}
  
	
	if(!pAction) {
		CSharedLog::Log(L_DBG, __FILE__, __LINE__, "unhandled UPnP Action \"%s\"", sName.c_str());
	}
  else {
    pAction->DeviceSettings(pDeviceSettings);
    pAction->setVirtualFolderLayout(vfolderLayout);
  }
	
  xmlFreeDoc(pDoc);
  return pAction;
}


bool CUPnPActionFactory::parseBrowseAction(CUPnPBrowse* pAction)
{
/*<?xml version="1.0" encoding="utf-8"?>
  <s:Envelope s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" xmlns:s="http://schemas.xmlsoap.org/soap/envelope/">
  <s:Body>
  <u:Browse xmlns:u="urn:schemas-upnp-org:service:ContentDirectory:1">
  <ObjectID>0</ObjectID>
  <BrowseFlag>BrowseDirectChildren</BrowseFlag>
  <Filter>*</Filter>
  <StartingIndex>0</StartingIndex>
  <RequestedCount>10</RequestedCount>
  <SortCriteria />
  </u:Browse>
  </s:Body>
  </s:Envelope>*/
  
  /* Object ID */
  string sRxObjId;
  if(!pAction->DeviceSettings()->Xbox360Support()) {
    sRxObjId = "<ObjectID.*>(.+)</ObjectID>";
  }
  else {
    // Xbox 360 does a browse using ContainerID instead of ObjectID for Pictures
    sRxObjId = "<ContainerID.*>(.+)</ContainerID>";
  }    
  RegEx rxObjId(sRxObjId);
  if(rxObjId.Search(pAction->GetContent())) {
		if(rxObjId.Match(1).length() > 10)
			return false;

    pAction->m_sObjectId = rxObjId.Match(1);
  }
		
  /* Browse flag */
  pAction->m_nBrowseFlag = UPNP_BROWSE_FLAG_UNKNOWN;    
  RegEx rxBrowseFlag("<BrowseFlag.*>(.+)</BrowseFlag>");
  if(rxBrowseFlag.Search(pAction->GetContent()))
  {
    string sMatch = rxBrowseFlag.Match(1);
    if(sMatch.compare("BrowseMetadata") == 0)
      pAction->m_nBrowseFlag = UPNP_BROWSE_FLAG_METADATA;
    else if(sMatch.compare("BrowseDirectChildren") == 0)
      pAction->m_nBrowseFlag = UPNP_BROWSE_FLAG_DIRECT_CHILDREN;
  }  
  
  /* Filter */
  RegEx rxFilter("<Filter.*>(.+)</Filter>");
  if(rxFilter.Search(pAction->GetContent()))  
    pAction->m_sFilter = rxFilter.Match(1);  
  else
    pAction->m_sFilter = "";

  /* Starting index */
  RegEx rxStartIdx("<StartingIndex.*>(.+)</StartingIndex>");
  if(rxStartIdx.Search(pAction->GetContent()))  
    pAction->m_nStartingIndex = atoi(rxStartIdx.Match(1).c_str());

  /* Requested count */
  RegEx rxReqCnt("<RequestedCount.*>(.+)</RequestedCount>");
  if(rxReqCnt.Search(pAction->GetContent()))  
    pAction->m_nRequestedCount = atoi(rxReqCnt.Match(1).c_str());  
  
	parseSortCriteria(pAction);

  return true;     
}

bool CUPnPActionFactory::parseSearchAction(CUPnPSearch* pAction)
{
  /*
	<?xml version="1.0" encoding="utf-8"?>
	<s:Envelope s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" xmlns:s="http://schemas.xmlsoap.org/soap/envelope/">
	<s:Body>
	  <u:Search xmlns:u="urn:schemas-upnp-org:service:ContentDirectory:1">
		  <ContainerID>0</ContainerID>
			<SearchCriteria>(upnp:class contains "object.item.imageItem") and (dc:title contains "")</SearchCriteria>
			<Filter>*</Filter>
			<StartingIndex>0</StartingIndex>
			<RequestedCount>7</RequestedCount>
			<SortCriteria></SortCriteria>
		</u:Search>
	</s:Body>
	</s:Envelope> */
  
  // Container ID
  RegEx rxContainerID("<ContainerID.*>(.+)</ContainerID>");
  if (rxContainerID.Search(pAction->GetContent()))
    pAction->m_sObjectId = rxContainerID.Match(1);
	
	// Search Criteria
	RegEx rxSearchCriteria("<SearchCriteria.*>(.+)</SearchCriteria>");
	if(rxSearchCriteria.Search(pAction->GetContent()))
	  pAction->m_sSearchCriteria = rxSearchCriteria.Match(1);
	
  // Filter
  RegEx rxFilter("<Filter.*>(.+)</Filter>");
  if(rxFilter.Search(pAction->GetContent()))  
    pAction->m_sFilter = rxFilter.Match(1);  
  else
    pAction->m_sFilter = "";

  // Starting index
  RegEx rxStartIdx("<StartingIndex.*>(.+)</StartingIndex>");
  if(rxStartIdx.Search(pAction->GetContent()))  
    pAction->m_nStartingIndex = atoi(rxStartIdx.Match(1).c_str());

  // Requested count
  RegEx rxReqCnt("<RequestedCount.*>(.+)</RequestedCount>");
  if(rxReqCnt.Search(pAction->GetContent()))  
    pAction->m_nRequestedCount = atoi(rxReqCnt.Match(1).c_str()); 

  // sort cirteria
	parseSortCriteria(pAction);
	
	return true;
}

bool CUPnPActionFactory::parseSortCriteria(CUPnPBrowseSearchBase* action)
{
#warning todo: error handling as defined by the upnp forum
	action->m_isSupportedSort = true;
	
	// sort criteria
  RegEx rxSort("<SortCriteria.*>(.+)</SortCriteria>");
	if(!rxSort.Search(action->GetContent().c_str())) {
	
		// sort by title if no sort criteria found
		action->m_sortCriteriaSQL = " A_TRACK_NUMBER, TITLE asc ";
		return false;
	}
	
	string tmp = rxSort.Match(1);
	action->m_sortCriteria = tmp;
	
	tmp += ",";
	string part;
	string ext;
	string::size_type pos;
	
	// split the criterias
	// e.g. +upnp:artist,-dc:date,+dc:title
	while((pos = tmp.find(",")) != string::npos) {
		
		part	= tmp.substr(0, pos);
		part	= TrimWhiteSpace(part);
		
		tmp		= tmp.substr(pos + 1);		
		tmp		= TrimWhiteSpace(tmp);
		
		// asc, desc
		ext   = part.substr(0, 1);
		part  = part.substr(1);
		
		if(ext.compare("+") == 0) {
			ext = "asc";
		}
		else if(ext.compare("-") == 0) {
			ext = "desc";
		}
		
		// replace fields
		if(part.compare("upnp:class") == 0) {
			part = " TYPE " + ext;
		}
		else if(part.compare("upnp:artist") == 0) {
			part = " AV_ARTIST " + ext;
		}
		else if(part.compare("upnp:genre") == 0) {
			part = " AV_GENRE " + ext;
		}
		else if(part.compare("upnp:album") == 0) {
			part = " AV_ALBUM " + ext;
		}
    else if(part.compare("dc:title") == 0) {
			part = " TITLE " + ext;
		}
		else if(part.compare("upnp:originalTrackNumber") == 0) {
			part = " A_TRACK_NUMBER " + ext;
		}
		// unhandled
		else {
			action->m_isSupportedSort = false;
			  cout << 
				"unhandled sort order" << endl <<
				"your device requested a sort order that is currently not supported " <<
				"by FUPPES. This is not a bug as FUPPES correctly specifies it's sorting " <<
				"capabilities but your device seems to ignore it." << endl <<
				"please file a feature request containing the following lines: " << endl << endl <<
        "=== CUT ===" << endl <<
        action->m_sortCriteria << endl <<
        "=== CUT ===" << endl;

			return false;
		}		

		action->m_sortCriteriaSQL +=	(part + ", ");
	} // while
	
	// remove trailing ","
	action->m_sortCriteriaSQL = TrimWhiteSpace(action->m_sortCriteriaSQL);
	if(action->m_sortCriteriaSQL.substr(action->m_sortCriteriaSQL.length()-1, 1).compare(",") == 0) {
		action->m_sortCriteriaSQL = action->m_sortCriteriaSQL.substr(0, action->m_sortCriteriaSQL.length()-1);
	}
	
	action->m_isSupportedSort = true;
	return action->m_isSupportedSort;	
}

bool CUPnPActionFactory::parseDestroyObjectAction(CUPnPAction* action)
{
/*<?xml version="1.0" encoding="utf-8"?>
  <s:Envelope s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" xmlns:s="http://schemas.xmlsoap.org/soap/envelope/">
  <s:Body>
  <u:DestroyObject xmlns:u="urn:schemas-upnp-org:service:ContentDirectory:1">
  <ObjectID>0</ObjectID>
  </u:DestroyObject>
  </s:Body>
  </s:Envelope>*/
  
  
  // object ID
  RegEx rxObjectID("<ObjectID.*>(.+)</ObjectID>");
  if (rxObjectID.Search(action->GetContent().c_str()))
    action->m_sObjectId = rxObjectID.Match(1);
  else
    return false;
    
    
  return true;  
}

