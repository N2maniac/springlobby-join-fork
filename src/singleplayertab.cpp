/* Copyright (C) 2007 The SpringLobby Team. All rights reserved. */


#include <wx/intl.h>
#include <wx/sizer.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/panel.h>
#include <wx/statline.h>
#include <wx/stattext.h>
#include <wx/msgdlg.h>
#include <stdexcept>

#include "singleplayertab.h"
#include "mapctrl.h"
#include "utils.h"
#include "uiutils.h"
#include "ui.h"
#include "iunitsync.h"
#include "addbotdialog.h"


BEGIN_EVENT_TABLE(SinglePlayerTab, wxPanel)

  EVT_CHOICE( SP_MAP_PICK, SinglePlayerTab::OnMapSelect )
  EVT_CHOICE( SP_MOD_PICK, SinglePlayerTab::OnModSelect )
  EVT_BUTTON( SP_ADD_BOT , SinglePlayerTab::OnAddBot )
  EVT_BUTTON( SP_START , SinglePlayerTab::OnStart )

END_EVENT_TABLE()


SinglePlayerTab::SinglePlayerTab(wxWindow* parent, Ui& ui, MainSinglePlayerTab& msptab):
  wxPanel( parent, -1 ),
  m_ui( ui ),
  m_battle( ui, msptab )
{
  try {
    if ( usync()->GetNumMods() > 0 ) m_battle.SetMod( usync()->GetMod( 0 ) ); // TODO load latest used mod.
  } catch (...) {}

  wxBoxSizer* m_main_sizer = new wxBoxSizer( wxVERTICAL );

  m_minimap = new MapCtrl( this, 100, &m_battle, ui, false, false, true, true );
  m_main_sizer->Add( m_minimap, 1, wxALL|wxEXPAND, 5 );

  wxBoxSizer* m_ctrl_sizer = new wxBoxSizer( wxHORIZONTAL );

  m_map_lbl = new wxStaticText( this, -1, _("Map:") );
  m_ctrl_sizer->Add( m_map_lbl, 0, wxALL, 5 );

  m_map_pick = new wxChoice( this, SP_MAP_PICK );
  m_ctrl_sizer->Add( m_map_pick, 1, wxALL, 5 );

  m_select_btn = new wxButton( this, SP_BROWSE_MAP, _("..."), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
  m_ctrl_sizer->Add( m_select_btn, 0, wxBOTTOM|wxRIGHT|wxTOP, 5 );

  m_mod_lbl = new wxStaticText( this, -1, _("Mod:") );
  m_ctrl_sizer->Add( m_mod_lbl, 0, wxALL, 5 );

  m_mod_pick = new wxChoice( this, SP_MOD_PICK );
  m_ctrl_sizer->Add( m_mod_pick, 1, wxALL, 5 );


//  m_ctrl_sizer->Add( 0, 0, 1, wxEXPAND, 0 );

  m_addbot_btn = new wxButton( this, SP_ADD_BOT, _("Add bot..."), wxDefaultPosition, wxSize(80, CONTROL_HEIGHT), 0 );
  m_ctrl_sizer->Add( m_addbot_btn, 0, wxALL, 5 );

  m_main_sizer->Add( m_ctrl_sizer, 0, wxEXPAND, 5 );

  m_buttons_sep = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
  m_main_sizer->Add( m_buttons_sep, 0, wxLEFT|wxRIGHT|wxEXPAND, 5 );

  wxBoxSizer* m_buttons_sizer = new wxBoxSizer( wxHORIZONTAL );

  m_reset_btn = new wxButton( this, SP_RESET, _("Reset"), wxDefaultPosition, wxSize(80, CONTROL_HEIGHT), 0 );
  m_buttons_sizer->Add( m_reset_btn, 0, wxALL, 5 );

  m_buttons_sizer->Add( 0, 0, 1, wxEXPAND, 0 );

  m_start_btn = new wxButton( this, SP_START, _("Start"), wxDefaultPosition, wxSize(80, CONTROL_HEIGHT), 0 );
  m_buttons_sizer->Add( m_start_btn, 0, wxALL, 5 );

  m_main_sizer->Add( m_buttons_sizer, 0, wxEXPAND, 5 );

  this->SetSizer( m_main_sizer );
  this->Layout();

  ReloadMaplist();
  ReloadModlist();

/*  if ( m_map_pick->GetCount() > 0 ) {
    m_map_pick->SetSelection(0);
    UnitSyncMap map = usync()->GetMap( 0, true );
    m_battle.SetMap( map );
    m_minimap->UpdateMinimap();
  }*/

}


SinglePlayerTab::~SinglePlayerTab()
{

}


void SinglePlayerTab::UpdateMinimap()
{
  m_minimap->UpdateMinimap();
}


void SinglePlayerTab::ReloadMaplist()
{
  m_map_pick->Clear();
  for ( int i = 0; i < usync()->GetNumMaps(); i++ ) {
    try {
      m_map_pick->Insert( RefineMapname( WX_STRING(usync()->GetMap( i, false ).name) ), i );
    } catch(...) {}
  }
}


void SinglePlayerTab::ReloadModlist()
{
  m_mod_pick->Clear();
  for ( int i = 0; i < usync()->GetNumMods(); i++ ) {
    try {
      m_mod_pick->Insert( RefineModname( WX_STRING(usync()->GetMod( i ).name) ), i );
    } catch(...) {}
  }

  m_mod_pick->SetStringSelection( RefineModname( WX_STRING(m_battle.GetModName()) ) );
}


bool SinglePlayerTab::ValidSetup()
{
  int numBots = 0;
  int first = -1;
  for ( unsigned int i = 0; i < (unsigned int)m_battle.Map().info.posCount; i++ ) {

    BattleBot* bot = m_battle.GetBotByStartPosition( i );

    if ( bot == 0 ) {
      if ( first == -1 ) first = i;
    } else {
      numBots++;
    }

  }

  if ( ( numBots < (int)m_battle.GetNumBots() ) || ( ( first != (int)m_battle.GetNumBots() ) && ( first != -1 ) ) ) {
    if ( numBots < (int)m_battle.GetNumBots() ) {
      wxMessageBox( _("You have bots that are not assingled to startpositions. In the current version of spring you are only allowed to use start positions positioning them freely is not allowed.\n\nThis will be fixed in next version of Spring."), _("Gamesetup error") );
    } else {
      wxMessageBox( _("You are not using consecutive start position numbers.\n\nIn the current version of spring you are not allowed to skip any startpositions. You have to use all consecutive position.\n\nExample: if you have 2 bots + yourself you have to use start positions 1,2,3 not 1,3,4.\n\nThis will be fixed in next version of Spring."), _("Gamesetup error") );
    }
    return false;
  }
  return true;
}


void SinglePlayerTab::OnMapSelect( wxCommandEvent& event )
{
  int index = m_map_pick->GetCurrentSelection();

  UnitSyncMap map = usync()->GetMap( index, true );
  m_battle.SetMap( map );

  m_minimap->UpdateMinimap();
}


void SinglePlayerTab::OnModSelect( wxCommandEvent& event )
{
  int index = m_mod_pick->GetCurrentSelection();

  UnitSyncMod mod = usync()->GetMod( index );
  m_battle.SetMod( mod );

  m_minimap->UpdateMinimap();
}


void SinglePlayerTab::OnAddBot( wxCommandEvent& event )
{
  AddBotDialog dlg( this, m_battle, true );
  if ( dlg.ShowModal() == wxID_OK ) {
    int x = 0, y = 0, r, g, b;
    m_battle.GetFreePosition( x, y );
    m_battle.GetFreeColour( r, g, b, false );
    int i = m_battle.AddBot( m_battle.GetFreeAlly(), x, y, dlg.GetAI() );
    BattleBot* bot = m_battle.GetBot( i );
    ASSERT_LOGIC( bot != 0, "bot == 0" );
    bot->bs.color_r = r;
    bot->bs.color_g = g;
    bot->bs.color_b = b;
    m_minimap->UpdateMinimap();
  }
}


void SinglePlayerTab::OnStart( wxCommandEvent& event )
{
  if ( ValidSetup() ) m_ui.StartSinglePlayerGame( m_battle );
}
