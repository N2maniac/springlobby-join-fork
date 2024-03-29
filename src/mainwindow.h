#ifndef SPRINGLOBBY_HEADERGUARD_MAINWINDOW_H
#define SPRINGLOBBY_HEADERGUARD_MAINWINDOW_H

#include <wx/frame.h>
#include "gui/windowattributespickle.h"

class Ui;
class Channel;
class User;
class wxCommandEvent;
class wxListbookEvent;
class wxAuiNotebookEvent;
class MainChatTab;
class BattleListTab;
class MainJoinBattleTab;
class MainSinglePlayerTab;
#ifndef NO_TORRENT_SYSTEM
class MainTorrentTab;
#endif
class wxBoxSizer;
class SLNotebook;
class wxListbook;
class MainOptionsTab;
class wxBitmap;
class wxImageList;
class ChatPanel;
class settings_frame;
class wxMenuItem;
class wxMenuBar;
class wxMenu;
class ChannelChooserDialog;
class ReplayTab;
class AutojoinChannelDialog;
class WidgetDownloadDialog;
class wxLogWindow;
class wxLogChain;
class wxCloseEvent;
class Statusbar;
class OptionsDialog;
class ReplayTraits;
template < class Traits >
class PlaybackTab;
class wxFocusEvent;

//class SavegameTraits;
//template < class Traits >
//class SavegameTab;

//! @brief wxFrame that contains the main window of the client.
class MainWindow : public wxFrame, public WindowAttributesPickle
{
  public:
    MainWindow( );
    virtual ~MainWindow();

    typedef PlaybackTab<ReplayTraits>
        ReplayTab;
//    typedef PlaybackTab<SavegameTraits>
//        SavegameTab;

    // MainWindow interface
    void OpenChannelChat( Channel& channel, bool doFocus = true );
    void OpenPrivateChat( const User& user, bool doFocus = false );

    void ShowConfigure( const unsigned int page = OPT_PAGE_SPRING );
    void ShowTab( const unsigned int idx );
    void ShowSingleplayer();

    /** Show the channel list dialog. */
    void ShowChannelChooser();

	//! cannot be const while using static event tables (function pointer sig don't match)
	void OnMenuAbout( wxCommandEvent& event );
    void OnMenuJoin( wxCommandEvent& event );
    void OnMenuChat( wxCommandEvent& event );
    void OnMenuConnect( wxCommandEvent& event );
    void OnMenuDisconnect( wxCommandEvent& event );
	void OnMenuServerTab( wxCommandEvent& event );
    void OnMenuSaveOptions( wxCommandEvent& event );
    void OnMenuQuit( wxCommandEvent& event );
	void OnMenuPathInfo( wxCommandEvent& event );
    void OnMenuVersion ( wxCommandEvent& event );
    void OnMenuSaveLayout( wxCommandEvent& event );
    void OnMenuLoadLayout( wxCommandEvent& event );
    void OnMenuResetLayout( wxCommandEvent& event );
	void OnMenuPreferences( wxCommandEvent& event );
	void OnMenuRename( wxCommandEvent& event );
	void OnMenuFirstStart( wxCommandEvent& event );
    void OnUnitSyncReload( wxCommandEvent& event );
    void OnMenuAutojoinChannels( wxCommandEvent& event );
	void OnReportBug( wxCommandEvent& event );
	void OnShowDocs( wxCommandEvent& event );
    void OnShowSettingsPP( wxCommandEvent& event );
    void OnMenuSelectLocale( wxCommandEvent& event );
    void OnShowChannelChooser( wxCommandEvent& event );
	void OnShowScreenshots( wxCommandEvent& event );
    void forceSettingsFrameClose();
    void OnUnitSyncReloaded();
    void OnChannelList( const wxString& channel, const int& numusers, const wxString& topic );
    void OnChannelListStart( );
    void OnClose( wxCloseEvent& );
	void OnSetFocus(wxFocusEvent&);
	void OnKillFocus(wxFocusEvent&);

    void OnTabsChanged( wxAuiNotebookEvent& event );
    MainChatTab& GetChatTab();
    BattleListTab& GetBattleListTab();
    MainJoinBattleTab& GetJoinTab();
    MainSinglePlayerTab& GetSPTab();
    ReplayTab& GetReplayTab();
//    SavegameTab& GetSavegameTab();
    #ifndef NO_TORRENT_SYSTEM
    MainTorrentTab& GetTorrentTab();
    #endif
    ChatPanel* GetActiveChatPanel();
    ChatPanel* GetChannelChatPanel( const wxString& channel );

    void SetTabIcons();

    void SetLogWin( wxLogWindow* log, wxLogChain* logchain );

    void LoadPerspectives( const wxString& perspective_name = wxEmptyString );
    void SavePerspectives( const wxString& perspective_name = wxEmptyString );

    void FocusBattleRoomTab();

	bool HasFocus() const;



  protected:

    wxMenuItem* m_settings_menu;
    wxMenuBar* m_menubar;
    wxMenu* m_menuTools;
	wxMenu* m_menuEdit;
	Statusbar* m_statusbar;

    wxBoxSizer* m_main_sizer;
    SLNotebook* m_func_tabs;

    MainChatTab* m_chat_tab;
    BattleListTab* m_list_tab;
    MainJoinBattleTab* m_join_tab;
    MainSinglePlayerTab* m_sp_tab;
    #ifndef NO_TORRENT_SYSTEM
    MainTorrentTab* m_torrent_tab;
    #endif
	OptionsDialog* m_opts_dialog;

    AutojoinChannelDialog* m_autojoin_dialog;
    settings_frame* se_frame;
    friend class settings_frame;
    bool se_frame_active;
    ChannelChooserDialog* m_channel_chooser;

    ReplayTab* m_replay_tab;
//    SavegameTab* m_savegame_tab;

	wxBitmap GetTabIcon( const unsigned char* data, size_t size  ) const;
	wxString AddPerspectivePostfix( const wxString& pers_name ) const;

    wxLogWindow* m_log_win;
    wxLogChain* m_log_chain;

	bool m_has_focus;

    enum {
        MENU_ABOUT = wxID_ABOUT,
        MENU_QUIT = wxID_EXIT,

        MENU_CONNECT = wxID_HIGHEST,
		MENU_SETTINGSPP,
        MENU_DISCONNECT,
		MENU_SERVER_TAB,
        MENU_SAVE_OPTIONS,
        MENU_JOIN,
        MENU_USYNC,
        MENU_TRAC,
        MENU_DOC,
        MENU_CHAT,
        MAIN_TABS,
        MENU_VERSION,
        MENU_START_TORRENT,
        MENU_STOP_TORRENT,
        MENU_AUTOJOIN_CHANNELS,
        MENU_SELECT_LOCALE,
        MENU_CHANNELCHOOSER,
        MENU_SAVE_LAYOUT,
        MENU_LOAD_LAYOUT,
        MENU_RESET_LAYOUT,
        MENU_DEFAULT_LAYOUT,
		MENU_SCREENSHOTS,
		MENU_PREFERENCES,
		MENU_RENAME,
		MENU_GENERAL_HELP,
		MENU_PATHINFO
    };

        class TabNames : public wxArrayString
        {
            public:
                TabNames ()
                {
                    Add( _("Chat") );
                    Add( _("Battlelist") );
                    Add( _("Battleroom") );
                    Add( _("Singleplayer") );
//                    Add( _("Savegames") );
                    Add( _("Replays") );
                #ifndef NO_TORRENT_SYSTEM
                    Add( _("Downloads") );
                #endif
                }
        };
        static TabNames m_tab_names;

    public:
        // Page indexes
        static const unsigned int PAGE_CHAT    = 0;
        static const unsigned int PAGE_LIST    = 1;
        static const unsigned int PAGE_JOIN    = 2;
        static const unsigned int PAGE_SINGLE  = 3;
        static const unsigned int PAGE_REPLAY  = 4;
//        static const unsigned int PAGE_SAVEGAME = 4;

        #ifndef NO_TORRENT_SYSTEM
        static const unsigned int PAGE_TORRENT = 5;
        #endif

        static const unsigned int OPT_PAGE_SPRING   = 0;
        static const unsigned int OPT_PAGE_CHAT     = 1;
        #ifndef NO_TORRENT_SYSTEN
        static const unsigned int OPT_PAGE_TORRENT  = 2;
        static const unsigned int OPT_PAGE_GENERAL  = 3;
        static const unsigned int OPT_PAGE_GROUPS   = 4;
        #else
        static const unsigned int OPT_PAGE_GENERAL  = 2;
        static const unsigned int OPT_PAGE_GROUPS   = 3;
        #endif

        static const TabNames& GetTabNames();

    protected:

        DECLARE_EVENT_TABLE()
};

//ChatPanel& servwin();

// wxWidget IDs


#endif // SPRINGLOBBY_HEADERGUARD_MAINWINDOW_H

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

