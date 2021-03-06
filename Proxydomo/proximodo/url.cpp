//------------------------------------------------------------------
//
//this file is part of Proximodo
//Copyright (C) 2004-2005 Antony BOUCHER ( kuruden@users.sourceforge.net )
//                        Paul Rupe      ( prupe@users.sourceforge.net )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//------------------------------------------------------------------
// Modifications: (date, author, description)
//
//------------------------------------------------------------------


//#include "stdafx.h"
//#include "settings.h"
#include "url.h"
#include <windows.h>
#include "..\Settings.h"
#include "util.h"

using namespace std;

/* Constructor with parsing
 */
CUrl::CUrl(const wstring& str) {
    parseUrl(str);
}


/* Parses the URL string and stores the URL and each part in
 * different strings
 */
void CUrl::parseUrl(const wstring& str) {

    size_t pos1 = str.find(L"://");
    if (pos1 == wstring::npos) 
		pos1 = 0;
	else
		pos1 += 3;
    url       = str;
    protocol  = (pos1 ? str.substr(0, pos1-3) : wstring(L"http"));

    bypassIn = bypassOut = bypassText = debug = source = false;

	if (str.substr(pos1, CSettings::s_urlCommandPrefix.length()) == CSettings::s_urlCommandPrefix) {
        bool foundUrlCmd = false;        
        for (;;) {
			if (str.length() <= pos1 + CSettings::s_urlCommandPrefix.length())
				break;
			pos1 += CSettings::s_urlCommandPrefix.length();
            wstring begin = str.substr(pos1);
            if (CUtil::noCaseBeginsWith(L"bin.", begin)) {
                bypassIn = true;
                pos1 += 4;
            } else if (CUtil::noCaseBeginsWith(L"bout.", begin)) {
                bypassOut = true;
                pos1 += 5;
            } else if (CUtil::noCaseBeginsWith(L"bweb.", begin)) {
                bypassText = true;
                pos1 += 5;
            } else if (CUtil::noCaseBeginsWith(L"bypass.", begin)) {
                bypassIn = bypassOut = bypassText = true;
                pos1 += 7;
            } else if (CUtil::noCaseBeginsWith(L"dbug.", begin)) {
                debug = true;
                pos1 += 5;
            } else if (CUtil::noCaseBeginsWith(L"src.", begin)) {
                source = true;
                pos1 += 4;
            } else {
				if (foundUrlCmd)
					pos1 -= CSettings::s_urlCommandPrefix.length();
                break;
            }
            foundUrlCmd = true;
        }
        if (foundUrlCmd == false)
			pos1 -= CSettings::s_urlCommandPrefix.length();
    }
	debug	|= CSettings::s_WebFilterDebug;

    size_t pos2 = str.find_first_of(L"/?#", pos1);
    if (pos2 == string::npos) pos2 = str.length();
    size_t pos3 = str.find_first_of(L"?#", pos2);
    if (pos3 == string::npos) pos3 = str.length();
    size_t pos4 = str.find_first_of(L"#", pos3);
    if (pos4 == string::npos) pos4 = str.length();

    fromhost  = str.substr(pos1);
    afterhost = str.substr(pos2);
    host      = str.substr(pos1, pos2 - pos1);
	hostport = host;
	if (hostport.find(L":") == string::npos)
		hostport += L':' + protocol;
	size_t pos5 = host.find(L':');
	if (pos5 != string::npos) {
		wstring port = host.substr(pos5 + 1);
		const wchar_t* kHttpPort = L"80";
		const wchar_t* kHttpsPort = L"443";
		if ((protocol == L"http" && port == kHttpPort) || (protocol == L"https" && port == kHttpsPort)) {
			host = host.substr(0, pos5);
		}
	}
    path      = str.substr(pos2, pos3 - pos2);
    query     = str.substr(pos3, pos4 - pos3);
    anchor    = str.substr(pos4);
}
// vi:ts=4:sw=4:et
