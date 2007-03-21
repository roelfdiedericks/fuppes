/***************************************************************************
 *            UPnPSearch.cpp
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
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
#include "UPnPSearch.h"
#include "../Common/Common.h"
#include "../Common/RegEx.h"

#include <sstream>
#include <iostream>

using namespace std;

CUPnPSearch::CUPnPSearch(std::string p_sMessage):
  CUPnPAction(UPNP_SERVICE_CONTENT_DIRECTORY, UPNP_SEARCH, p_sMessage)
{
}                                     

CUPnPSearch::~CUPnPSearch()
{
}

/*unsigned int CUPnPSearch::GetObjectIDAsInt()
{
  return HexToInt(m_sObjectID);
}*/

std::string StringReplace(std::string p_sIn, std::string p_sSearch, std::string p_sReplace)
{
  std::string p_sResult;
	if(p_sIn.find(p_sSearch) == std::string::npos)
	  return p_sIn;
	
  while(p_sIn.find(p_sSearch) != std::string::npos)
	{
	  p_sResult += p_sIn.substr(0, p_sIn.find(p_sSearch)) + p_sReplace;
		p_sIn      = p_sIn.substr(p_sIn.find(p_sSearch) + p_sSearch.length(), p_sIn.length());
	}
	p_sResult += p_sIn;
	
	return p_sResult;
}

std::string CUPnPSearch::BuildSQL(bool p_bLimit)
{
  /*std::string sTest;
	sTest = "(upnp:class contains \"object.item.imageItem\") and (dc:title = \"test \\\"dahhummm\\\" [xyz] §$%&(abc) titel\") or author exists true and (title exists false and (author = \"test\" or author = \"dings\"))";
*/

  string sOpenBr;
	string sProp;
	string sOp;
	string sVal;
	string sCloseBr;
	string sLogOp = "where";
	bool   bNumericProp = false;
	bool   bLikeOp  = false;
  bool   bBuildOK = false;

  stringstream sSql;


  // xbox 360 uses &quot; instead of "
	if(GetDeviceSettings()->m_bXBox360Support) {
	  m_sSearchCriteria = StringReplace(m_sSearchCriteria, "&quot;", "\"");
	}


  cout << m_sSearchCriteria << endl;

  RegEx rxSearch("(\\(*) *([\\w+:*\\w*]+) ([=|!=|<|<=|>|>=|contains|doesNotContain|derivedfrom|exists]+) (\".*?[^\\\\]\"|true|false) *(\\)*) *([and|or]*)");
	if(rxSearch.Search(m_sSearchCriteria.c_str())) {
	  do {
		  cout <<  rxSearch.Match(1) << " X " << rxSearch.Match(2) << " X " << rxSearch.Match(3) << " X " << rxSearch.Match(4) << " X " << rxSearch.Match(5) << " X " << rxSearch.Match(6) << endl;
		
		  // contains "where" on first loop
			// so we just append it when criterias can be found
		  sSql << " " << sLogOp << endl;
		
		  sOpenBr  = rxSearch.Match(1);
			sProp    = rxSearch.Match(2);
		  sOp      = rxSearch.Match(3);
			sVal     = rxSearch.Match(4);
			sCloseBr = rxSearch.Match(5);
			sLogOp   = rxSearch.Match(6);
			
			if(sOp.compare("exists") == 0) {
			}
			else {
				
				
				bBuildOK = true;
				
				// replace property
				if(sProp.compare("upnp:class") == 0) {
				  sProp = "o.TYPE";
					bNumericProp = true;
				}
				else if(sProp.compare("dc:title") == 0) {
				  sProp = "o.TITLE";
					bNumericProp = false;
				}
				else {
				  bBuildOK = false;
				}
				
				
				// replace operator
				bLikeOp = false;
				if(sOp.compare("contains") == 0) {
				  if(bNumericProp)
					  sOp = "in";
					else
					  sOp = "like";
						
					bLikeOp = true;
				}
				else if (sOp.compare("derivedfrom") == 0) {
				  sOp = "in";
				} 
				else {
				  bBuildOK = false;
				}
				
				
				// trim value
				cout << "Val: " << sVal << " => ";
			  sVal = sVal.substr(1, sVal.length() - 2);
				cout << sVal << endl;
				
				// replace value
				if(sProp.compare("o.TYPE") == 0) { 
				  if(sVal.compare("object.item.imageItem") == 0)
					  sVal = "(1, 100)";
					else if(sVal.compare("object.item.audioItem") == 0)
					  sVal = "(2, 200, 201)";	
					else if(sVal.compare("object.item.videoItem") == 0)
					  sVal = "(3, 300, 301)";
						
					else
					  bBuildOK = false;
				} 
				else if (!bNumericProp) {
				  if(bLikeOp)
				    sVal = "'%" + sVal + "%'";
					else
						sVal = "'" + sVal + "'";
				}
				
			} // != exists
		
		  if(bBuildOK)
  			sSql << sOpenBr << sProp << " " << sOp << " " << sVal << sCloseBr << " ";
			else
			  cout << "error parsing search request (part): " << rxSearch.Match(0) << endl;
			  
			
		}	while (rxSearch.SearchAgain());
	}
	else {
	  //cout << "no match" << endl;
	}
	
	
	/* XBox 360
	
	Request type	ContainerID	SearchCriteria
List all albums								7																	(upnp:class = "object.container.album.musicAlbum")
List contents of an album			The id of the album container			(upnp:class derivedfrom "object.item.audioItem"
List all artists							6																	(upnp:class = "object.container.person.musicArtist")
List all albums by an artist	1																	(upnp:class = "object.container.album.musicAlbum") and (upnp:artist = "artistname")
List all playlists						F																	(upnp:class = "object.container.playlistContainer")
List contents of a playlist		The id of the playlist						(upnp:class derivedfrom "object.item.audioItem")
List all songs								4																	(upnp:class derivedfrom "object.item.audioItem")
List all genres								5																	(upnp:class = "object.container.genre.musicGenre")
List contents of a genre			The id of the genre								(upnp:class derivedfrom "object.item.audioItem")
*/
	
	
	
	if(p_bLimit) {
		if((m_nRequestedCount > 0) || (m_nStartingIndex > 0)) {
      sSql << " limit " << m_nStartingIndex << ", ";
      if(m_nRequestedCount == 0)
        sSql << "-1";
      else
        sSql << m_nRequestedCount;
    }
	}

  sSql << ";";

  //cout << sSql.str() << endl;	

  return sSql.str();
}
