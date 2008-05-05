#ifndef SPRINGLOBBY_HEADERGUARD_IBATTLE_H
#define SPRINGLOBBY_HEADERGUARD_IBATTLE_H


#include <wx/string.h>
#include <list>
#include <vector>

#include "iunitsync.h"
#include "user.h"
#include "mmoptionswrapper.h"


typedef int HostInfo;

typedef int StartType;
enum {
  ST_Fixed = 0,
  ST_Random = 1,
  ST_Choose = 2,
  ST_Pick = 3
};


typedef int GameType;
enum  {
  GT_ComContinue = 0,
  GT_ComEnds = 1,
  GT_Lineage = 2
};


typedef int BattleType;
enum {
  BT_Unknown = 0,
  BT_Multi = 1,
  BT_Single = 2
};


struct BattleStartRect
{
  BattleStartRect() { local = true; updated = false; deleted = false; }
  bool local;
  bool updated;
  bool deleted;

  int ally;
  int top;
  int left;
  int right;
  int bottom;
};


struct BattleBot {
  UserBattleStatus bs;
  int posx;
  int posy;
  int handicap;
  wxString name;
  wxString owner;
  wxString aidll;

};


class IBattle
{
  public:

    IBattle();
    virtual ~IBattle();

    virtual void SetMap( const wxString& mapname, const wxString& hash );
    virtual void SetMap( const UnitSyncMap& map );
    virtual const UnitSyncMap& Map();
    virtual wxString GetMapName();
    virtual wxString GetMapHash();

    virtual void SetMod( const wxString& modname, const wxString& hash );
    virtual void SetMod( const UnitSyncMod& mod );
    virtual const UnitSyncMod& Mod();
    virtual wxString GetModName();
    virtual wxString GetModHash();

    virtual bool MapExists();
    virtual bool ModExists();

    virtual wxColour GetFreeColour( User *for_whom ) = 0;

    virtual BattleStartRect* GetStartRect( int allyno ) { return 0; };
    virtual void AddStartRect( int allyno, int left, int top, int right, int bottom ) {};
    virtual void RemoveStartRect( int allyno ) {};
    virtual void UpdateStartRect( int allyno ) {};
    virtual void ClearStartRects(){};

    virtual int GetMyAlly() = 0;
    virtual void SetMyAlly( int ally ) = 0;

    virtual bool IsFounderMe() =0;

    virtual void SendHostInfo( HostInfo update ) = 0;
    virtual void SendHostInfo( const wxString& Tag ) = 0;

    virtual BattleBot* GetBotByStartPosition( unsigned int startpos ) { return 0; };
    virtual BattleBot* GetBot( unsigned int index ) = 0;
    virtual BattleBot* GetBot( const wxString& name ) { return 0; };
    virtual unsigned int GetNumBots() = 0;
    virtual unsigned int AddBot( int ally, int posx, int posy, int handicap, const wxString& aidll );
    virtual void RemoveBot( unsigned int index ) {};

    virtual void GetFreePosition( int& x, int& y ) {}
    virtual int GetFreeAlly() { return 0; }

    virtual void DisableUnit( const wxString& unitname );
    virtual void EnableUnit( const wxString& unitname );
    virtual void EnableAllUnits();
    virtual wxArrayString DisabledUnits();

    virtual void OnUnitSyncReloaded();

    virtual std::vector<BattleStartRect*>::size_type GetNumRects() =0;

    virtual mmOptionsWrapper* CustomBattleOptions() =0;

  protected:

    bool m_map_loaded;
    bool m_mod_loaded;
    bool m_map_exists;
    bool m_mod_exists;
    UnitSyncMap m_map;
    UnitSyncMod m_mod;
    wxString m_map_name;
    wxString m_mod_name;

    wxArrayString m_units;
};

#endif // SPRINGLOBBY_HEADERGUARD_IBATTLE_H
