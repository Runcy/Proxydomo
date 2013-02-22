/**
*	@file	LogViewWindow.cpp
*	@brief	ログ表示ウィンドウクラス
*/
/**
	this file is part of Proxydomo
	Copyright (C) amate 2013-

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either
	version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#include "stdafx.h"
#include "LogViewWindow.h"
#include <fstream>
#include <algorithm>
#include <boost\property_tree\ini_parser.hpp>
#include <boost\property_tree\ptree.hpp>
#include "Misc.h"

#define  LOG_COLOR_BACKGROUND  RGB(255, 255, 255)
#define  LOG_COLOR_DEFAULT     RGB(140, 140, 140)
#define  LOG_COLOR_FILTER      RGB(140, 140, 140)
#define  LOG_COLOR_REQUEST     RGB(240, 100,   0)
#define  LOG_COLOR_RESPONSE    RGB(  0, 150,   0)
#define  LOG_COLOR_PROXY       RGB(  0,   0,   0)


CLogViewWindow::CLogViewWindow()
{
}


CLogViewWindow::~CLogViewWindow()
{
}


void	CLogViewWindow::ShowWindow()
{
	CLog::RegisterLogTrace(this);
	__super::ShowWindow(TRUE);
}


// ILogTrace

void CLogViewWindow::ProxyEvent(LogProxyEvent Event, const IPv4Address& addr)
{
	CString msg;
	msg.Format(_T(">>> ポート %d : "), addr.GetPortNumber());
	switch (Event) {
	case kLogProxyNewRequest:
		msg	+= _T("新しいリクエストを受信しました\n");
		{
			CCritSecLock	lock(m_csActiveRequestLog);
			m_vecActiveRequestLog.emplace_back(new EventLog(addr.GetPortNumber(), msg, LOG_COLOR_PROXY));
		}
		break;

	case kLogProxyEndRequest:
		msg += _T("リクエストが完了しました\n");
		{
			CCritSecLock	lock(m_csActiveRequestLog);
			uint16_t port = addr.GetPortNumber();
			auto result = std::remove_if(m_vecActiveRequestLog.begin(), m_vecActiveRequestLog.end(), 
				[port](const std::unique_ptr<EventLog>& log) { return log->port == port; }
			);
			m_vecActiveRequestLog.erase(result, m_vecActiveRequestLog.end());
		}
		break;

	default:
		ATLASSERT( FALSE );
		return ;
	}

	CString title;
	title.Format(_T("ログ - Active[ %d ]"), CLog::GetActiveRequestCount());
	SetWindowText(title);

	_AppendText(msg, LOG_COLOR_PROXY);
}

void CLogViewWindow::HttpEvent(LogHttpEvent Event, const IPv4Address& addr, int RequestNumber, const std::string& text)
{
	CString msg;
	msg.Format(_T(">>> ポート %d #%d : "), addr.GetPortNumber(), RequestNumber);
	switch (Event) {
	case kLogHttpRecvOut:
		msg += _T("ブラウザ ⇒ Proxy(this)");
		break;

	case kLogHttpSendOut:
		msg += _T("Proxy(this) ⇒ サイト");
		break;

	case kLogHttpRecvIn:
		msg += _T("サイト ⇒ Proxy(this)");
		break;

	case kLogHttpSendIn:
		msg += _T("Proxy(this) ⇒ ブラウザ");
		break;

	default:
		return ;
	}
	msg += _T("\n");
	msg += text.c_str();
	msg += _T("\n");

    // Colors depends on Outgoing or Incoming
	COLORREF color = LOG_COLOR_REQUEST;
	if (Event == kLogHttpRecvIn || Event == kLogHttpSendIn)
		color = LOG_COLOR_RESPONSE;
	
	{
		CCritSecLock	lock(m_csActiveRequestLog);
		m_vecActiveRequestLog.emplace_back(new EventLog(addr.GetPortNumber(), msg, color));
	}
	_AppendText(msg, color);
}


BOOL CLogViewWindow::OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
{
	m_editLog = GetDlgItem(IDC_RICHEDIT_LOG);

	m_editLog.SetBackgroundColor(LOG_COLOR_BACKGROUND);

    // ダイアログリサイズ初期化
    DlgResize_Init(true, false, WS_THICKFRAME | WS_MAXIMIZEBOX | WS_CLIPCHILDREN);

	using namespace boost::property_tree;
	
	CString settingsPath = Misc::GetExeDirectory() + _T("settings.ini");
	std::ifstream fs(settingsPath);
	if (fs) {
		ptree pt;
		read_ini(fs, pt);
		CRect rcWindow;
		rcWindow.top	= pt.get("LogWindow.top", 0);
		rcWindow.left	= pt.get("LogWindow.left", 0);
		rcWindow.right	= pt.get("LogWindow.right", 0);
		rcWindow.bottom	= pt.get("LogWindow.bottom", 0);
		if (rcWindow != CRect()) {
			MoveWindow(&rcWindow);
		}
		if (pt.get("LogWindow.ShowWindow", false))
			ShowWindow();
	}

	return 0;
}

void CLogViewWindow::OnDestroy()
{
	CLog::RemoveLogTrace();

	using namespace boost::property_tree;
	
	std::string settingsPath = CT2A(Misc::GetExeDirectory() + _T("settings.ini"));
	ptree pt;
	read_ini(settingsPath, pt);

	CRect rcWindow;
	GetWindowRect(&rcWindow);
		
	pt.put("LogWindow.top", rcWindow.top);
	pt.put("LogWindow.left", rcWindow.left);
	pt.put("LogWindow.right", rcWindow.right);
	pt.put("LogWindow.bottom", rcWindow.bottom);
	bool bVisible = IsWindowVisible() != 0;
	pt.put("LogWindow.ShowWindow", bVisible);

	write_ini(settingsPath, pt);
}

void CLogViewWindow::OnCancel(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	CLog::RemoveLogTrace();

	{
		CCritSecLock	lock(m_csActiveRequestLog);
		m_vecActiveRequestLog.clear();
	}

	__super::ShowWindow(FALSE);
}


void CLogViewWindow::OnClear(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	m_editLog.SetWindowText(_T(""));
	m_editLog.Invalidate();
}

void CLogViewWindow::OnShowActiveRequestLog(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	OnClear(0, 0, NULL);

	CString title;
	title.Format(_T("ログ - Active[ %d ]"), CLog::GetActiveRequestCount());
	SetWindowText(title);

	CCritSecLock	lock(m_csActiveRequestLog);
	for (auto& log : m_vecActiveRequestLog) {
		_AppendText(log->text, log->textColor);
	}

}

void	CLogViewWindow::_AppendText(const CString& text, COLORREF textColor)
{
	CCritSecLock lock(m_csLog);

	CHARFORMAT cfmt = { sizeof(CHARFORMAT) };
	cfmt.dwMask = CFM_COLOR;
	cfmt.crTextColor	= textColor;
	
	m_editLog.SetSel(-1, -1);
	long start = 0;
	long end = 0;
	m_editLog.GetSel(start, end);

	m_editLog.AppendText(text);

	m_editLog.SetSel(end, -1);
	m_editLog.SetSelectionCharFormat(cfmt);
	m_editLog.SetSel(-1, -1);

	m_editLog.PostMessage(WM_VSCROLL, SB_BOTTOM);
}


