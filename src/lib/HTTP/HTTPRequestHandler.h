/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            HTTPRequestHandler.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2006-2010 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
 
#ifndef _HTTPREQUESTHANDLER_H
#define _HTTPREQUESTHANDLER_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include "HTTPMessage.h"


class CHTTPRequestHandler
{
  public:
    CHTTPRequestHandler(std::string p_sHTTPServerURL);
  
    bool HandleRequest(CHTTPMessage* pRequest, CHTTPMessage* pResponse);

  private:
    bool HandleHTTPRequest(CHTTPMessage* pRequest, CHTTPMessage* pResponse);
    //bool HandleHTTPResponse();
    bool HandleSOAPAction(CHTTPMessage* pRequest, CHTTPMessage* pResponse);    

    bool HandleGENAMessage(CHTTPMessage* pRequest, CHTTPMessage* pResponse);
  
    bool handleAVItemRequest(std::string p_sObjectId, CHTTPMessage* pRequest, CHTTPMessage* pResponse, bool audio, std::string requestExt);
    
		bool handleImageRequest(std::string p_sObjectId, CHTTPMessage* pRequest, CHTTPMessage* pResponse);
			
    std::string m_sHTTPServerURL;
};

#endif // _HTTPREQUESTHANDLER_H
