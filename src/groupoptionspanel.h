
#ifndef SPRINGLOBBY_HEADERGUARD_GROUPOPTIONSPANEL_H
#define SPRINGLOBBY_HEADERGUARD_GROUPOPTIONSPANEL_H

#include <wx/panel.h>
#include <wx/string.h>


class ColorButton;
class GroupUserDialog;
class wxCheckBox;
class wxStaticText;
class wxListBox;
class wxButton;

class GroupOptionsPanel : public wxPanel
{
  DECLARE_EVENT_TABLE()

  protected:
    enum
    {
      REMOVE_GROUP = 1000,
      RENAME_GROUP,
      ADD_GROUP,
      GROUPS_LIST,
      NOTIFY_LOGIN,
      IGNORE_CHAT,
      NOTIFY_HOST,
      IGNORE_PM,
      NOTIFY_STATUS,
      AUTOCKICK,
      ALLOW_JUGGLER,
      NOTIFY_HIGHLIGHT,
      HIGHLIGHT_COLOR,
      USERS_LIST,
      ADD_USER,
	  REMOVE_USER
    };

    wxListBox* m_group_list;
    wxButton* m_remove_group_button;
    wxButton* m_rename_group_button;

    wxButton* m_add_group_button;
    wxPanel* m_group_panel;
    wxCheckBox* m_login_notify_check;
    wxCheckBox* m_ignore_chat_check;
    wxCheckBox* m_notify_host_check;
    wxCheckBox* m_ignore_pm_check;
    wxCheckBox* m_notify_status_check;
    wxCheckBox* m_autokick_check;
    wxCheckBox* m_allow_juggler_check;
    wxCheckBox* m_highlight_check;
    wxStaticText* m_highlight_colorstaticText;
    ColorButton* m_highlight_color_button;
    wxListBox* m_user_list;
    wxButton* m_add_user_button;
    wxButton* m_remove_user_button;

    wxString m_current_group;
    GroupUserDialog* m_user_dialog;

    void OnRemoveGroup( wxCommandEvent& event );
    void OnRenameGroup( wxCommandEvent& event );
    void OnAddNewGroup( wxCommandEvent& event );
    void OnGroupListSelectionChange( wxCommandEvent& event );
    void OnGroupActionsChange( wxCommandEvent& event );
    void OnHighlightColorClick( wxCommandEvent& event );
    void OnUsersListSelectionChange( wxCommandEvent& event );
    void OnAddUsers( wxCommandEvent& event );
    void OnRemoveUser( wxCommandEvent& event );

    void Initialize();

    void ShowGroup( const wxString& group );
    void ReloadUsersList();
    void ReloadGroupsList();
    wxString GetFirstGroupName();

  public:
    GroupOptionsPanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 656,537 ), long style = wxTAB_TRAVERSAL );
    ~GroupOptionsPanel();
    void Update();
};

#endif

/**
    This file is part of SpringLobby,
    Copyright (C) 2007-2011

    SpringLobby is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as published by
    the Free Software Foundation.

    SpringLobby is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SpringLobby.  If not, see <http://www.gnu.org/licenses/>.
**/

