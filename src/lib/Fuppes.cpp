/***************************************************************************
 *            Fuppes.cpp
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 - 2007 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 *  Copyright (C) 2005 Thomas Schnitzler <tschnitzler@users.sourceforge.net>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
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
 
/*===============================================================================
 INCLUDES
===============================================================================*/

#include "Fuppes.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include "Common/Common.h"
#include "SharedLog.h"
#include "HTTP/HTTPMessage.h"
#include "MediaServer.h"
#include "UPnPDevice.h"
#include "GENA/SubscriptionMgr.h"

#include "GUI/NotificationMgr.h"

using namespace std;

const string LOGNAME = "FUPPES";

/** constructor
 *  @param  p_sIPAddress  IP-address of the network interface 
 *                        this instance should be started on
 *  @param  p_sUUID       UUID this instance should be started with
 *  @param  pPresentationRequestHandler pointer to an object implementing the request handler
 */
CFuppes::CFuppes(std::string p_sIPAddress, std::string p_sUUID)
{
  CSharedLog::Shared()->Log(L_EXTENDED, "starting UPnP subsystem", __FILE__, __LINE__);  
 
  /* set members */
  m_sIPAddress                  = p_sIPAddress;
  m_sUUID                       = p_sUUID;
  fuppesThreadInitMutex(&m_OnTimerMutex);
  fuppesThreadInitMutex(&m_RemoteDevicesMutex);
    
  /* init HTTP-server */
  try {
    m_pHTTPServer = new CHTTPServer(m_sIPAddress);
    m_pHTTPServer->SetReceiveHandler(this);
    m_pHTTPServer->Start();
  }
  catch(EException ex) {    
    throw;
  }
    
  /* init SSDP-controller */
  try {
    m_pSSDPCtrl = new CSSDPCtrl(m_sIPAddress, m_pHTTPServer->GetURL());
	  m_pSSDPCtrl->SetReceiveHandler(this);
    m_pSSDPCtrl->Start();
  }
  catch(EException ex) {    
    throw;
  }

  /* init SubscriptionMgr */
  try {
    CSubscriptionMgr::Shared();
  }
  catch(EException ex) {
    throw;
  }
  
  /* Create MediaServer */
  m_pMediaServer = new CMediaServer(m_pHTTPServer->GetURL(), this);	  
  
  /* create ContentDirectory */
  try {
    m_pContentDirectory = new CContentDirectory(m_pHTTPServer->GetURL());   
	  m_pMediaServer->AddUPnPService(m_pContentDirectory);
  }
  catch(EException ex) {
    throw;
  }    

  /* create ConnectionManager */
  try {
    m_pConnectionManager = new CConnectionManager(m_pHTTPServer->GetURL()); 
    m_pMediaServer->AddUPnPService(m_pConnectionManager);
  }
  catch(EException ex) {
    throw;
  }
	
	// XMSMediaReceiverRegistrar
	try {
	  m_pXMSMediaReceiverRegistrar = new CXMSMediaReceiverRegistrar();
		m_pMediaServer->AddUPnPService(m_pXMSMediaReceiverRegistrar);
	}
	catch(EException ex) {
	  throw;
	}
	
  CSharedLog::Shared()->Log(L_EXTENDED, "UPnP subsystem started", __FILE__, __LINE__);  
  
  /* if everything is up and running, multicast alive messages
     and search for other devices */       
  CSharedLog::Shared()->Log(L_EXTENDED, "multicasting alive messages", __FILE__, __LINE__);  
  m_pSSDPCtrl->send_alive();
  CSharedLog::Shared()->Log(L_EXTENDED, "multicasting m-search", __FILE__, __LINE__);    
  m_pSSDPCtrl->send_msearch();
  
  /* start alive timer */
  m_pMediaServer->GetTimer()->SetInterval(840);  // 900 sec = 15 min
  m_pMediaServer->GetTimer()->Start();
}

/** destructor
 */
CFuppes::~CFuppes()
{  
  CSharedLog::Shared()->Log(L_EXTENDED, "deleting FUPPES instance", __FILE__, __LINE__);    
  
  /* multicast notify-byebye */
  CSharedLog::Shared()->Log(L_EXTENDED, "multicasting byebye messages", __FILE__, __LINE__);
  m_pSSDPCtrl->send_byebye();  
  
  /* stop SSDP-controller */
	CSharedLog::Shared()->Log(L_EXTENDED, "stopping SSDP controller", __FILE__, __LINE__);
  m_pSSDPCtrl->Stop();

  /* stop HTTP-server */
	CSharedLog::Shared()->Log(L_EXTENDED, "stopping HTTP server", __FILE__, __LINE__);
  m_pHTTPServer->Stop();

  CleanupTimedOutDevices();  
  
  fuppesThreadDestroyMutex(&m_OnTimerMutex);
  fuppesThreadDestroyMutex(&m_RemoteDevicesMutex);
  
  /* destroy objects */
  delete m_pContentDirectory;
  delete m_pMediaServer;
  delete m_pSSDPCtrl;
  delete m_pHTTPServer;
}

void CFuppes::CleanupTimedOutDevices()
{  
  // iterate device list ...
  for(m_TimedOutDevicesIterator = m_TimedOutDevices.begin(); m_TimedOutDevicesIterator != m_TimedOutDevices.end(); m_TimedOutDevicesIterator++)
  {
    // ... and delete timed out devices
    CUPnPDevice* pTimedOutDevice = *m_TimedOutDevicesIterator;      
    m_TimedOutDevicesIterator = m_TimedOutDevices.erase(m_TimedOutDevicesIterator);
    delete pTimedOutDevice;
  }  
}

void CFuppes::OnTimer(CUPnPDevice* pSender)
{
  fuppesThreadLockMutex(&m_OnTimerMutex);
  
  CleanupTimedOutDevices();
  
  CSharedLog::Shared()->Log(L_EXTENDED, "OnTimer()", __FILE__, __LINE__);
  // local device must send alive message
  if(pSender->GetIsLocalDevice())
  {
    stringstream sLog;
    sLog << "device: " << pSender->GetUUID() << " send timed alive";
    CSharedLog::Shared()->ExtendedLog(LOGNAME, sLog.str());    
    m_pSSDPCtrl->send_alive();    
  }
  // remote device timed out
  else
  {
	  if(!pSender->GetFriendlyName().empty()) {
      stringstream sLog;
      sLog << "device: " << pSender->GetFriendlyName() << " timed out";
      CSharedLog::Shared()->Log(LOGNAME, sLog.str());
		}

    fuppesThreadLockMutex(&m_RemoteDevicesMutex);
    m_RemoteDeviceIterator = m_RemoteDevices.find(pSender->GetUUID());  
    // found device
    if(m_RemoteDeviceIterator != m_RemoteDevices.end()) { 
      // remove device from list of remote devices
      m_RemoteDevices.erase(pSender->GetUUID());			
      // stop the device's timer and HTTP client and ...
      pSender->GetTimer()->Stop();
      // ... push it to the list containing timed out devices
      m_TimedOutDevices.push_back(pSender);
    }
		fuppesThreadUnlockMutex(&m_RemoteDevicesMutex);

  }
  
  fuppesThreadUnlockMutex(&m_OnTimerMutex);
}

/** Returns URL of the HTTP member
 * @return std::string
 */
std::string CFuppes::GetHTTPServerURL()
{
  return m_pHTTPServer->GetURL();
}

/** Returns IP address 
 *  @return std::string
 */
std::string CFuppes::GetIPAddress()
{
  return m_sIPAddress;
}

/** Returns a UPnP device
 *  @return std::vector<CUPnPDevice*>
 */
std::vector<CUPnPDevice*> CFuppes::GetRemoteDevices()
{
  std::vector<CUPnPDevice*> vResult;
  
  /* Iterate devices */
  for(m_RemoteDeviceIterator = m_RemoteDevices.begin(); m_RemoteDeviceIterator != m_RemoteDevices.end(); m_RemoteDeviceIterator++)
  {
    vResult.push_back((*m_RemoteDeviceIterator).second);
  }
  
  return vResult; 
}


void CFuppes::OnSSDPCtrlReceiveMsg(CSSDPMessage* pMessage)
{
//  cout << inet_ntoa(pMessage->GetRemoteEndPoint().sin_addr) << ":" << ntohs(pMessage->GetRemoteEndPoint().sin_port) << endl;
  
  CSharedLog::Shared()->Log(L_EXTENDED, "OnSSDPCtrlReceiveMsg()", __FILE__, __LINE__);
	CSharedLog::Shared()->Log(L_DEBUG, pMessage->GetMessage(), __FILE__, __LINE__, false);
  
  if((m_sIPAddress.compare(inet_ntoa(pMessage->GetRemoteEndPoint().sin_addr)) != 0) || (pMessage->GetUUID().compare(m_sUUID) != 0))
  { 
    switch(pMessage->GetMessageType())
    {
      case SSDP_MESSAGE_TYPE_NOTIFY_ALIVE:
        CSharedLog::Shared()->ExtendedLog(LOGNAME, "SSDP_MESSAGE_TYPE_NOTIFY_ALIVE");
        HandleSSDPAlive(pMessage);
        break;
      case SSDP_MESSAGE_TYPE_M_SEARCH_RESPONSE:
        CSharedLog::Shared()->ExtendedLog(LOGNAME, "SSDP_MESSAGE_TYPE_M_SEARCH_RESPONSE");
        HandleSSDPAlive(pMessage);
        break;
      case SSDP_MESSAGE_TYPE_NOTIFY_BYEBYE:
        CSharedLog::Shared()->ExtendedLog(LOGNAME, "SSDP_MESSAGE_TYPE_NOTIFY_BYEBYE");
        HandleSSDPByeBye(pMessage);
        break;
      case SSDP_MESSAGE_TYPE_M_SEARCH:
        /* m-search is handled by SSDPCtrl */
        break;
      case SSDP_MESSAGE_TYPE_UNKNOWN:
        /* this should never happen :) */
        break;
    }
  }
}

bool CFuppes::OnHTTPServerReceiveMsg(CHTTPMessage* pMessageIn, CHTTPMessage* pMessageOut)
{
  bool fRet = true;

  // set HTTP version
  pMessageOut->SetVersion(pMessageIn->GetVersion());

  // handle message
  HTTP_MESSAGE_TYPE nMsgType = pMessageIn->GetMessageType();
  switch(nMsgType)
  {
    // HTTP request
    case HTTP_MESSAGE_TYPE_GET:
      fRet = HandleHTTPRequest(pMessageIn, pMessageOut);
      break;
    case HTTP_MESSAGE_TYPE_HEAD:
      fRet = HandleHTTPRequest(pMessageIn, pMessageOut);
      break;
    case HTTP_MESSAGE_TYPE_POST:
      fRet = HandleHTTPRequest(pMessageIn, pMessageOut);
      break;

    // SOAP
    case HTTP_MESSAGE_TYPE_POST_SOAP_ACTION:
      fRet = HandleHTTPPostSOAPAction(pMessageIn, pMessageOut);
      break;

		// HTTP response
    case HTTP_MESSAGE_TYPE_200_OK:
      break;
    case HTTP_MESSAGE_TYPE_404_NOT_FOUND:
      break;

    default:    
        fRet = false;
      break;
  }

  return fRet;
}

bool CFuppes::HandleHTTPRequest(CHTTPMessage* pMessageIn, CHTTPMessage* pMessageOut)
{
  // Get request
  std::string strRequest = pMessageIn->GetRequest();

  // Root description
  if(ToLower(strRequest).compare("/description.xml") == 0) {
    pMessageOut->SetMessage(HTTP_MESSAGE_TYPE_200_OK, "text/xml"); // HTTP_CONTENT_TYPE_TEXT_XML
    pMessageOut->SetContent(m_pMediaServer->GetDeviceDescription(pMessageIn));
    return true;
  }

  // ContentDirectory description
  else if(strRequest.compare("/UPnPServices/ContentDirectory/description.xml") == 0) {
    pMessageOut->SetMessage(HTTP_MESSAGE_TYPE_200_OK, "text/xml");
    pMessageOut->SetContent(m_pContentDirectory->GetServiceDescription());
    return true;
  }

  // ConnectionManager description
  else if(strRequest.compare("/UPnPServices/ConnectionManager/description.xml") == 0) {
    pMessageOut->SetMessage(HTTP_MESSAGE_TYPE_200_OK, "text/xml");    
    pMessageOut->SetContent(m_pConnectionManager->GetServiceDescription());
    return true;
  }

  return false;
}

bool CFuppes::HandleHTTPPostSOAPAction(CHTTPMessage* pMessageIn, CHTTPMessage* pMessageOut)
{     
  // get UPnP action
  CUPnPAction* pAction;
  pAction = pMessageIn->GetAction();  
  if(!pAction)
    return false;
  
  // handle UPnP action
  bool bRet = true;
  switch(pAction->GetTargetDeviceType())
  {
    case UPNP_SERVICE_CONTENT_DIRECTORY:
      m_pContentDirectory->HandleUPnPAction(pAction, pMessageOut);      
      break;
    case UPNP_SERVICE_CONNECTION_MANAGER:
      m_pConnectionManager->HandleUPnPAction(pAction, pMessageOut);
      break;    
		case UPNP_SERVICE_X_MS_MEDIA_RECEIVER_REGISTRAR:
      m_pXMSMediaReceiverRegistrar->HandleUPnPAction(pAction, pMessageOut);
      break;
    default:
      bRet = false;
      break;
  }
  return bRet;
}

void CFuppes::HandleSSDPAlive(CSSDPMessage* pMessage)
{
  fuppesThreadLockMutex(&m_RemoteDevicesMutex);

  m_RemoteDeviceIterator = m_RemoteDevices.find(pMessage->GetUUID());  

  // known device
  if(m_RemoteDeviceIterator != m_RemoteDevices.end())
  {
    m_RemoteDevices[pMessage->GetUUID()]->GetTimer()->Reset();
    
    std::stringstream sMsg;
    sMsg << "received \"Notify-Alive\" from known device id: " << pMessage->GetUUID();    
    CSharedLog::Shared()->Log(L_EXTENDED, sMsg.str(), __FILE__, __LINE__);
  }

  // new device
  else
  {
    std::stringstream sMsg;
    sMsg << "received \"Notify-Alive\" from unknown device id: " << pMessage->GetUUID();      
    CSharedLog::Shared()->Log(L_EXTENDED, sMsg.str(), __FILE__, __LINE__);
    sMsg.str("");
        
    if((pMessage->GetLocation().compare("")) == 0)
      return;
      
    CUPnPDevice* pDevice = new CUPnPDevice(this, pMessage->GetUUID());
		m_RemoteDevices[pMessage->GetUUID()] = pDevice;
		pDevice->BuildFromDescriptionURL(pMessage->GetLocation());
		pDevice->GetTimer()->SetInterval(10);  // wait max. 30 sec. for description
		pDevice->GetTimer()->Start();
  }

  fuppesThreadUnlockMutex(&m_RemoteDevicesMutex);
}

void CFuppes::OnNewDevice(CUPnPDevice* pSender)
{
  fuppesThreadLockMutex(&m_RemoteDevicesMutex);

  m_RemoteDeviceIterator = m_RemoteDevices.find(pSender->GetUUID());  
  if(m_RemoteDeviceIterator != m_RemoteDevices.end())
	{
	  stringstream sMsg;
		sMsg << "new device: " << pSender->GetFriendlyName() << " :: " << pSender->GetUPnPDeviceTypeAsString();
	  CSharedLog::Shared()->Log(L_NORMAL, sMsg.str(), __FILE__, __LINE__);
	  sMsg.str("");
	
	  pSender->GetTimer()->SetInterval(900); // 900 sec = 15 min
			
	  sMsg << "new UPnP device:" << endl << pSender->GetFriendlyName() << " (" << pSender->GetUPnPDeviceTypeAsString() << ")";
	  CNotificationMgr::Shared()->Notify(pSender->GetFriendlyName(), sMsg.str());
	}
	
  fuppesThreadUnlockMutex(&m_RemoteDevicesMutex);
}

void CFuppes::HandleSSDPByeBye(CSSDPMessage* pMessage)
{
  std::stringstream sLog;
  sLog << "received \"Notify-ByeBye\" from device: " << pMessage->GetUUID();  
  CSharedLog::Shared()->Log(L_EXTENDED, sLog.str(), __FILE__, __LINE__);
  sLog.str("");

  fuppesThreadLockMutex(&m_RemoteDevicesMutex);

  m_RemoteDeviceIterator = m_RemoteDevices.find(pMessage->GetUUID());  
  // found device
  if(m_RemoteDeviceIterator != m_RemoteDevices.end())
  {
    sLog << "received byebye from " << m_RemoteDevices[pMessage->GetUUID()]->GetFriendlyName() << " :: " << m_RemoteDevices[pMessage->GetUUID()]->GetUPnPDeviceTypeAsString();    
    CSharedLog::Shared()->Log(L_NORMAL, sLog.str(), __FILE__, __LINE__);

    stringstream sMsg;
    sMsg << "UPnP device gone:" << endl << m_RemoteDevices[pMessage->GetUUID()]->GetFriendlyName() << " (" << m_RemoteDevices[pMessage->GetUUID()]->GetUPnPDeviceTypeAsString() << ")";
    CNotificationMgr::Shared()->Notify(m_RemoteDevices[pMessage->GetUUID()]->GetFriendlyName(), sMsg.str());

    delete m_RemoteDevices[pMessage->GetUUID()];
    m_RemoteDevices.erase(pMessage->GetUUID());
  }
  
  fuppesThreadUnlockMutex(&m_RemoteDevicesMutex);
}