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


//
// Class: Ui
#ifndef _UI_H_
#define _UI_H_

#include <wx/msgdlg.h>
#include "mainwindow.h"
#include "connectwindow.h"


struct UiChannelData {
  ChatPanel* panel;
};


//! @brief UI main class
class Ui
{
  public:
    Ui();
    virtual ~Ui();
  
    // Ui interface
  
    void ShowMainWindow();
    void ShowConnectWindow();
    void Connect();
  
    void Quit();
  
    bool Ask( wxString heading, wxString question );
  
    MainWindow& mw();
  
    void OnConnected( string server_name, string server_ver, bool supported );
  
    void OnJoinedChannelSuccessful( Channel& chan );
    void OnUserJoinedChannel( Channel& chan, User& user );
    void OnUserLeftChannel( Channel& chan, User& user, const string& reason );
    
    void OnChannelTopic( Channel& channel , User& user, const string& topic );
    void OnChannelSaid( Channel& channel , User& user, const string& message );
    void OnChannelDidAction( Channel& channel , User& user, const string& action );
    void OnLeaveChannel( Channel& channel );
  
    void OnUserOnline( User& user );
    void OnUserOffline( User& user );
    void OnUserStatusChanged( User& user );

  protected:
    // Ui variables
  
    MainWindow* m_main_win;
    ConnectWindow* m_con_win;
};

Ui& ui();


#endif  //_UI_H_
