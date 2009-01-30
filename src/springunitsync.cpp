/* Copyright (C) 2007 The SpringLobby Team. All rights reserved. */

#include <algorithm>
#include <wx/dynlib.h>
#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/msgdlg.h>
#include <wx/mstream.h>
#include <wx/stdpaths.h>
#include <wx/string.h>
#include <wx/file.h>
#include <wx/tokenzr.h>
#include <wx/log.h>
//#include <wx/txtstrm.h>
//#include <wx/wfstream.h>
#include <wx/textfile.h>
#include <cmath>
#include <stdexcept>
#include <clocale>

#include "springunitsync.h"
#include "settings.h"
#include "springunitsynclib.h"
#include "settings++/custom_dialogs.h"
#include "unitsyncthread.h"
#include "globalsmanager.h"


#define LOCK_UNITSYNC wxCriticalSectionLocker lock_criticalsection(m_lock)

const wxEventType UnitSyncAsyncOperationCompletedEvt = wxNewEventType();


IUnitSync& usync()
{
  static GlobalObjectHolder<SpringUnitSync> m_sync;
  return m_sync;
}


SpringUnitSync::SpringUnitSync()
  : m_map_image_cache( 3 )      // may take about 3M per image ( 1024x1024 24 bpp minimap )
  , m_tiny_minimap_cache( 200 ) // takes at most 30k per image (   100x100 24 bpp minimap )
  , m_mapinfo_cache( 1000000 )  // this one is just misused as thread safe std::map ...
{
  m_cache_thread.Create();
  m_cache_thread.SetPriority( WXTHREAD_MIN_PRIORITY );
  m_cache_thread.Run();
}


SpringUnitSync::~SpringUnitSync()
{
  m_cache_thread.Wait();
  FreeUnitSyncLib();
}


bool SpringUnitSync::LoadUnitSyncLib( const wxString& unitsyncloc )
{
   LOCK_UNITSYNC;
   wxLogDebugFunc( _T("") );
   bool ret = _LoadUnitSyncLib( unitsyncloc );
   if (ret)
   {
      m_cache_path = sett().GetCachePath();
      PopulateArchiveList();
   }
   return ret;
}


void SpringUnitSync::PopulateArchiveList()
{
  m_maps_list.clear();
  m_mods_list.clear();
  m_mod_array.Empty();
  m_map_array.Empty();
  m_map_image_cache.Clear();
  m_mapinfo_cache.Clear();

  int numMaps = susynclib().GetMapCount();
  for ( int i = 0; i < numMaps; i++ )
  {
    wxString name, hash;
    try
    {
     name = susynclib().GetMapName( i );
     hash = susynclib().GetMapChecksum( i );
     //PrefetchMap( name ); // DEBUG
    } catch (...) { continue; }
    try
    {
      m_maps_list[name] = hash;
      m_map_array.Add( name );
    } catch (...)
    {
      wxLogError( _T("Found map with hash collision: ") + name + _T(" hash: ") + hash );
    }
  }
  int numMods = susynclib().GetPrimaryModCount();
  for ( int i = 0; i < numMods; i++ )
  {
    wxString name, hash;
    try
    {
     name = susynclib().GetPrimaryModName( i );
     hash = i2s(susynclib().GetPrimaryModChecksum( i ));
    } catch (...) { continue; }
    try
    {
      m_mods_list[name] = hash;
      m_mod_array.Add( name );
    } catch (...)
    {
      wxLogError( _T("Found mod with hash collision: ") + name + _T(" hash: ") + hash );
    }
  }
}



bool SpringUnitSync::_LoadUnitSyncLib( const wxString& unitsyncloc )
{
  ActivityNoticeContainer act_notice( NULL, _("Loading unitsync"), _T("%s") );
  try {
    susynclib().Load( unitsyncloc, sett().GetForcedSpringConfigFilePath() );
  } catch (...) {
    return false;
  }
  return true;
}


void SpringUnitSync::FreeUnitSyncLib()
{
  LOCK_UNITSYNC;
  wxLogDebugFunc( _T("") );
  susynclib().Unload();
}


bool SpringUnitSync::IsLoaded()
{
  return susynclib().IsLoaded();
}


wxString SpringUnitSync::GetSpringVersion()
{
  wxLogDebugFunc( _T("") );
  wxString ret;
  try
  {
    ret = susynclib().GetSpringVersion();
  }
  catch (...){}
  return ret;
}


bool SpringUnitSync::VersionSupports( GameFeature feature )
{
  return susynclib().VersionSupports( feature );
}


int SpringUnitSync::GetNumMods()
{
  wxLogDebugFunc( _T("") );
  return m_mod_array.GetCount();
}


wxArrayString SpringUnitSync::GetModList()
{
  return m_mod_array;
}


int SpringUnitSync::GetModIndex( const wxString& name )
{
  wxLogDebugFunc( _T("name = \"") + name + _T("\"") );
  try
  {
    return susynclib().GetModIndex( name );
  } catch (...){}
  return -1;
}


bool SpringUnitSync::ModExists( const wxString& modname )
{
  return (m_mods_list.find(modname) != m_mods_list.end());
}


bool SpringUnitSync::ModExists( const wxString& modname, const wxString& hash )
{
  LocalArchivesVector::iterator itor = m_mods_list.find(modname);
  if ( itor == m_mods_list.end() ) return false;
  return itor->second == hash;
}

bool SpringUnitSync::ModExistsCheckHash( const wxString& hash ) const
{
    LocalArchivesVector::const_iterator itor = m_mods_list.begin();
    for ( ; itor != m_mods_list.end(); ++itor ) {
        if ( itor->second == hash )
            return true;
    }
    return false;
}

UnitSyncMod SpringUnitSync::GetMod( const wxString& modname )
{
  wxLogDebugFunc( _T("modname = \"") + modname + _T("\"") );
  UnitSyncMod m;

  m.name = modname;
  m.hash = m_mods_list[modname];

  return m;
}


UnitSyncMod SpringUnitSync::GetMod( int index )
{
  wxLogDebugFunc( _T("") );
  UnitSyncMod m;
  susynclib().GetPrimaryModCount();
  m.name = susynclib().GetPrimaryModName( index );
  m.hash = i2s( susynclib().GetPrimaryModChecksum( index ) );

  return m;
 }

int SpringUnitSync::GetNumMaps()
{
  wxLogDebugFunc( _T("") );
  return m_map_array.GetCount();
}


wxArrayString SpringUnitSync::GetMapList()
{
  return m_map_array;
}


wxArrayString SpringUnitSync::GetModValidMapList( const wxString& modname )
{
  wxArrayString ret;
  try
  {
    unsigned int mapcount = susynclib().GetValidMapCount( modname );
    for ( unsigned int i = 0; i < mapcount; i++ ) ret.Add( susynclib().GetValidMapName( i ) );
  } catch ( assert_exception& e ) {}
  return ret;
}


bool SpringUnitSync::MapExists( const wxString& mapname )
{
  return (m_maps_list.find(mapname) != m_maps_list.end());
}


bool SpringUnitSync::MapExists( const wxString& mapname, const wxString& hash )
{
  LocalArchivesVector::iterator itor = m_maps_list.find(mapname);
  if ( itor == m_maps_list.end() ) return false;
  return itor->second == hash;
}


UnitSyncMap SpringUnitSync::GetMap( const wxString& mapname )
{
  wxLogDebugFunc( _T("") );
  UnitSyncMap m;

  m.name = mapname;
  m.hash = m_maps_list[mapname];

  return m;
}


UnitSyncMap SpringUnitSync::GetMap( int index )
{
  wxLogDebugFunc( _T("") );
  UnitSyncMap m;

  m.name = m_map_array[index];
  m.hash = m_maps_list[m.name];

   return m;
 }


UnitSyncMap SpringUnitSync::GetMapEx( int index )
{
  UnitSyncMap m;

  if ( index < 0 ) return m;

  m.name = m_map_array[index];

  m.hash = m_maps_list[m.name];

  m.info = _GetMapInfoEx( m.name );

  return m;
}

void GetOptionCachefileEntry( const int i, wxArrayString& entry, GameOptions& ret)
{
    //all section values for options are converted to lower case
    //since usync returns the key of section type keys lower case
    //otherwise comapring would be a real hassle
    wxString key = susynclib().GetOptionKey(i);
      entry.Add( key );
      wxString name = susynclib().GetOptionName(i);
      entry.Add( TowxString( susynclib().GetOptionType(i) ) );
      switch (susynclib().GetOptionType(i))
      {
      case opt_float:
      {
        ret.float_map[key] = mmOptionFloat( name, key,
            susynclib().GetOptionDesc(i), susynclib().GetOptionNumberDef(i),
            susynclib().GetOptionNumberStep(i),
            susynclib().GetOptionNumberMin(i), susynclib().GetOptionNumberMax(i),
            susynclib().GetOptionSection(i).Lower(), susynclib().GetOptionStyle(i) );

        entry.Add( ret.float_map[key].name );
        wxString descr = ret.float_map[key].description;
        descr.Replace( _T("\n"), _T("") );
        entry.Add( descr );
        entry.Add( TowxString( ret.float_map[key].def ) );
        entry.Add( TowxString( ret.float_map[key].stepping ) );
        entry.Add( TowxString( ret.float_map[key].min ) );
        entry.Add( TowxString( ret.float_map[key].max) );
        entry.Add( ret.float_map[key].section );
        entry.Add( ret.float_map[key].ct_type_string );
        break;
      }
      case opt_bool:
      {
        ret.bool_map[key] = mmOptionBool( name, key,
            susynclib().GetOptionDesc(i), susynclib().GetOptionBoolDef(i),
            susynclib().GetOptionSection(i).Lower(), susynclib().GetOptionStyle(i) );

        entry.Add( susynclib().GetOptionName(i) );
        wxString descr = ret.bool_map[key].description;
        descr.Replace( _T("\n"), _T("") );
        entry.Add( descr );
        entry.Add( TowxString( ret.bool_map[key].def ) );
        entry.Add( ret.bool_map[key].section );
        entry.Add( ret.bool_map[key].ct_type_string );
        break;
      }
      case opt_string:
      {
        ret.string_map[key] = mmOptionString( name, key,
            susynclib().GetOptionDesc(i), susynclib().GetOptionStringDef(i),
            susynclib().GetOptionStringMaxLen(i),
            susynclib().GetOptionSection(i).Lower(), susynclib().GetOptionStyle(i) );

        entry.Add( ret.string_map[key].name );
        wxString descr = ret.string_map[key].description;
        descr.Replace( _T("\n"), _T("") );
        entry.Add( descr );
        entry.Add( ret.string_map[key].def );
        entry.Add( TowxString( ret.string_map[key].max_len ) );
        entry.Add( ret.string_map[key].section );
        entry.Add( ret.string_map[key].ct_type_string );
        break;
      }
      case opt_list:
      {
         ret.list_map[key] = mmOptionList(name,key,
            susynclib().GetOptionDesc(i),susynclib().GetOptionListDef(i),
            susynclib().GetOptionSection(i).Lower(),susynclib().GetOptionStyle(i));

        entry.Add( ret.list_map[key].name );
        wxString descr = ret.list_map[key].description;
        descr.Replace( _T("\n"), _T("") );
        entry.Add( descr );
        entry.Add( ret.list_map[key].def );
        int listItemCount = susynclib().GetOptionListCount(i);
        entry.Add( TowxString( listItemCount ) );
         for (int j = 0; j < listItemCount; ++j)
         {
           wxString descr = susynclib().GetOptionListItemDesc(i,j);
           descr.Replace( _T("\n"), _T("") );
           ret.list_map[key].addItem(susynclib().GetOptionListItemKey(i,j),susynclib().GetOptionListItemName(i,j), descr);
           entry.Add( susynclib().GetOptionListItemKey(i,j) );
           entry.Add( susynclib().GetOptionListItemName(i,j) );
           entry.Add( descr );
         }
        entry.Add( ret.list_map[key].section );
        entry.Add( ret.list_map[key].ct_type_string );
        break;
      }
      case opt_section:
      {
        ret.section_map[key] = mmOptionSection( name, key, susynclib().GetOptionDesc(i),
            susynclib().GetOptionSection(i).Lower(), susynclib().GetOptionStyle(i) );
        wxString section = susynclib().GetOptionSection(i);

        entry.Add( ret.section_map[key].name );
        wxString descr = ret.section_map[key].description;
        descr.Replace( _T("\n"), _T("") );
        entry.Add( descr );
        entry.Add( ret.section_map[key].section );
        entry.Add( ret.section_map[key].ct_type_string );
      }
      }
}

void ParseOptionCacheFile( wxArrayString& cache, GameOptions& ret )
{
    unsigned int count = cache.GetCount();
    for (unsigned int i = 0; i < count; ++i)
    {
      // key  type
      wxArrayString params = wxStringTokenize( cache[i], _T("\t"), wxTOKEN_RET_EMPTY);
      wxString key = params[0];
      switch ( s2l( params[1] ) )
      {
      case opt_float:
        // name description default_value step_size min_value max_value
        ret.float_map[key] = mmOptionFloat( params[2], key,
            params[3], (float)s2d( params[4] ), (float)s2d( params[5] ),
            (float)s2d( params[6] ), (float)s2d( params[7] ), params[8],params[9] );
        break;
      case opt_bool:
        // name description default_value
        ret.bool_map[key] = mmOptionBool( params[2], key,
            params[3], (bool)s2l( params[4] ), params[5],params[6] );
        break;
      case opt_string:
        // name description default_value max_lenght
        ret.string_map[key] = mmOptionString( params[2], key,
            params[3], params[4], (unsigned int) s2l( params[5] ), params[6],params[7] );
        break;
      case opt_list:
      {
         unsigned int last_item_index = ( (unsigned int)s2l(params[5]) * 3 + 6);
         //be sure to check index alignement
         ret.list_map[key] = mmOptionList( params[2],key,
            params[3], params[4], params[last_item_index],params[last_item_index +1] );
            //there's 3 members per item, therefor the 3 + params[5] (which is listitemcount)
         for (unsigned int j = 6; j < last_item_index; j = j + 3)
         {
           ret.list_map[key].addItem( params[j], params[j+1], params[j+2] );
         }
         break;
      }
      case opt_section:
        ret.section_map[key] = mmOptionSection( params[2], key,
            params[3], params[4], params[5] );
        break;
      }
    }
}

GameOptions SpringUnitSync::GetMapOptions( const wxString& name )
{
  wxLogDebugFunc( name );
  GameOptions ret;
  wxArrayString cache;
  try
  {
    cache = GetCacheFile( GetFileCachePath( name, _T(""), false ) + _T(".mapoptions") );
    ParseOptionCacheFile( cache, ret );
  }
  catch (...)
  {
    int count = susynclib().GetMapOptionCount(name);
    for (int i = 0; i < count; ++i)
    {
      wxArrayString entry;
      GetOptionCachefileEntry( i, entry, ret );
      wxString optiontoken;

      unsigned int entrycount = entry.GetCount();
      for ( unsigned int pos = 0; pos < entrycount; pos++ )
        optiontoken << entry[pos] << _T('\t');

      cache.Add( optiontoken );
    }
    SetCacheFile( GetFileCachePath( name, _T(""), false ) + _T(".mapoptions"), cache );
  }
  return ret;
}

wxArrayString SpringUnitSync::GetMapDeps( const wxString& mapname )
{
	wxArrayString ret;
	try
	{
		ret = susynclib().GetMapDeps( GetMapIndex( mapname ) );
	}
	catch( unitsync_assert ) {}
	return ret;
}


UnitSyncMap SpringUnitSync::GetMapEx( const wxString& mapname )
{
  wxLogDebugFunc( _T("") );
  int i = GetMapIndex( mapname );
  ASSERT_LOGIC( i >= 0, _T("Map does not exist") );
  return GetMapEx( i );
}


int SpringUnitSync::GetMapIndex( const wxString& name )
{
  int result = m_map_array.Index( name );
  if ( result == wxNOT_FOUND ) result = -1;
  return result;
}


wxString SpringUnitSync::GetModArchive( int index )
{
  wxLogDebugFunc( _T("") );

  return susynclib().GetPrimaryModArchive( index );
}


wxString SpringUnitSync::GetMapArchive( int index )
{
  wxLogDebugFunc( _T("") );

  int count = susynclib().GetMapArchiveCount( index );

  if ( count > 0 )
    return susynclib().GetMapArchiveName( 0 );
  else
    return _T("");
}


GameOptions SpringUnitSync::GetModOptions( const wxString& name )
{
  wxLogDebugFunc( name );
  GameOptions ret;
  wxArrayString cache;
  try
  {
    cache = GetCacheFile( GetFileCachePath( name, _T(""), true ) + _T(".modoptions") );
    ParseOptionCacheFile( cache, ret );
  }
  catch (...)
  {
    int count = susynclib().GetModOptionCount(name);
    for (int i = 0; i < count; ++i)
    {
      wxArrayString entry;
      GetOptionCachefileEntry( i, entry, ret );
      wxString optiontoken;

      unsigned int entrycount = entry.GetCount();
      for ( unsigned int pos = 0; pos < entrycount; pos++ )
        optiontoken << entry[pos] << _T('\t');

      cache.Add( optiontoken );
    }
    SetCacheFile( GetFileCachePath( name, _T(""), true ) + _T(".modoptions"), cache );
  }
  return ret;
}

wxArrayString SpringUnitSync::GetModDeps( const wxString& modname )
{
	wxArrayString ret;
	try
	{
		ret = susynclib().GetModDeps( GetModIndex( modname ) );
	}
	catch( unitsync_assert ) {}
	return ret;
}

wxArrayString SpringUnitSync::GetSides( const wxString& modname )
{
	wxArrayString ret;
	try
	{
		ret = susynclib().GetSides( modname );
	}
	catch( unitsync_assert ) {}
	return ret;
}


wxImage SpringUnitSync::GetSidePicture( const wxString& modname, const wxString& SideName )
{
  wxLogDebugFunc( _T("") );

  wxImage cache;

  if ( !cache.LoadFile( GetFileCachePath( modname, _T(""), true ) + _T("-") + SideName + _T(".sidepicture.png") ) )
  {
    susynclib().SetCurrentMod( modname );
    wxLogDebugFunc( _T("SideName = \"") + SideName + _T("\"") );
    wxString ImgName = _T("SidePics");
    ImgName += _T("/");
    ImgName += SideName.Upper();
    ImgName += _T(".bmp");

    int ini = susynclib().OpenFileVFS (ImgName );
    ASSERT_EXCEPTION( ini, _T("cannot find side image") );

    int FileSize = susynclib().FileSizeVFS(ini);
    if (FileSize == 0) {
      susynclib().CloseFileVFS(ini);
      ASSERT_EXCEPTION( FileSize, _T("side image has size 0") );
    }

    uninitialized_array<char> FileContent(FileSize);
    susynclib().ReadFileVFS(ini, FileContent, FileSize);
    wxMemoryInputStream FileContentStream( FileContent, FileSize );

    cache.LoadFile( FileContentStream, wxBITMAP_TYPE_ANY, -1);
    cache.InitAlpha();
    for ( int x = 0; x < cache.GetWidth(); x++ )
      for ( int y = 0; y < cache.GetHeight(); y++ )
        if ( cache.GetBlue( x, y ) == 255 && cache.GetGreen( x, y ) == 255 && cache.GetRed( x, y ) == 255 ) cache.SetAlpha( x, y, 0 ); /// set pixel to be transparent
    cache.SaveFile( GetFileCachePath( modname, _T(""), true ) + _T("-") + SideName + _T(".sidepicture.png"), wxBITMAP_TYPE_PNG );
  }
  return cache;
}


wxArrayString SpringUnitSync::GetAIList( const wxString& modname )
{
  wxLogDebugFunc( _T("") );

	wxArrayString ret;

	if ( usync().VersionSupports( USYNC_GetSkirmishAI ) )
	{
		int total = susynclib().GetSkirmishAICount( modname );
		for ( int i = 0; i < total; i++ )
		{
			wxArrayString infos = susynclib().GetAIInfo( i );
			int namepos = infos.Index( _T("shortName") ) + 1;
			int versionpos = infos.Index( _T("version") ) + 1;
			wxString ainame;
			if ( namepos > 0 ) ainame += infos[namepos];
			if ( versionpos > 0 ) ainame += _T(" ") + infos[versionpos];
			ret.Add( ainame );
		}
	}
	else
	{
		// list dynamic link libraries
		int dllini = susynclib().InitFindVFS(  wxDynamicLibrary::CanonicalizeName(_T("AI/Bot-libs/*"), wxDL_MODULE) );

		wxString FileName;
		dllini = susynclib().FindFilesVFS( dllini, FileName );
		while ( dllini )
		{
			if ( ret.Index( FileName.BeforeLast( '/') ) == wxNOT_FOUND ) ret.Add ( FileName ); // don't add duplicates
			dllini = susynclib().FindFilesVFS( dllini, FileName );
		}
		// list jar files (java AIs)
		int jarini = susynclib().InitFindVFS(  _T("AI/Bot-libs/*.jar") );

		jarini = susynclib().FindFilesVFS( jarini, FileName );
		while ( jarini )
		{
			if ( ret.Index( FileName.BeforeLast( '/') ) == wxNOT_FOUND ) ret.Add ( FileName ); // don't add duplicates
			jarini = susynclib().FindFilesVFS( jarini, FileName );
		}

		// luaai
		try
		{
			const int LuaAICount = susynclib().GetLuaAICount( modname );
			for ( int i = 0; i < LuaAICount; i++ )
			{
				 ret.Add( _T( "LuaAI:" ) +  susynclib().GetLuaAIName( i ) );
			}
		} catch (...) {}
	}

  return ret;
}

wxArrayString SpringUnitSync::GetAIInfos( int index )
{
	wxArrayString ret;
	try
	{
		ret = susynclib().GetAIInfo( index );
	}
	catch ( unitsync_assert ) {}
	return ret;
}

int SpringUnitSync::GetNumUnits( const wxString& modname )
{
  wxLogDebugFunc( _T("") );

  susynclib().AddAllArchives( susynclib().GetPrimaryModArchive( susynclib().GetModIndex( modname ) ) );
  susynclib().ProcessUnitsNoChecksum();

  return susynclib().GetUnitCount();
}


wxArrayString SpringUnitSync::GetUnitsList( const wxString& modname )
{
  wxLogDebugFunc( modname );

  wxArrayString cache;
  try
  {
    cache = GetCacheFile( GetFileCachePath( modname, _T(""), true ) + _T(".units") );
  } catch(...)
  {
    susynclib().SetCurrentMod( modname );
    while ( susynclib().ProcessUnitsNoChecksum() );
    unsigned int unitcount = susynclib().GetUnitCount();
    for ( unsigned int i = 0; i < unitcount; i++ )
    {
      cache.Add( susynclib().GetFullUnitName(i) << _T(" (") << susynclib().GetUnitName(i) << _T(")") );
    }

    SetCacheFile( GetFileCachePath( modname, _T(""), true ) + _T(".units"), cache );

  }

  return cache;
}

wxSize MakeFit(const wxSize &original, const wxSize &bounds){
  if(bounds.GetWidth()<=0 || bounds.GetHeight()<=0)return wxSize(0,0);
  int sizex=(original.GetWidth()*bounds.GetHeight())/original.GetHeight();
  if(sizex<=bounds.GetWidth()){
    return wxSize(sizex,bounds.GetHeight());
  }else{
    int sizey=(original.GetHeight()*bounds.GetWidth())/original.GetWidth();
    return wxSize(bounds.GetWidth(),sizey);
  }
}


wxImage SpringUnitSync::GetMinimap( const wxString& mapname )
{
  wxLogDebugFunc( mapname );
  return _GetMapImage( mapname, _T(".minimap.png"), &SpringUnitSyncLib::GetMinimap );
}


wxImage SpringUnitSync::GetMinimap( const wxString& mapname, int width, int height )
{
  wxLogDebugFunc( mapname + _T(" size: ") + TowxString( width ) + _T("x") + TowxString( height ) );

  const bool tiny = ( width <= 100 && height <= 100 );
  wxImage img;

  if ( tiny && m_tiny_minimap_cache.TryGet( mapname, img ) )
  {
    wxSize image_size = MakeFit(wxSize(img.GetWidth(), img.GetHeight()), wxSize(width, height));

    if ( image_size.GetWidth() != img.GetWidth() || image_size.GetHeight() != img.GetHeight() )
      img.Rescale( image_size.GetWidth(), image_size.GetHeight() );

    return img;
  }

  img = GetMinimap( mapname );

  // special resizing code because minimap is always square,
  // and we need to resize it to the correct aspect ratio.
  if (img.GetWidth() > 1 && img.GetHeight() > 1)
  {
    try {
      MapInfo mapinfo = _GetMapInfoEx( mapname );

      wxSize image_size = MakeFit(wxSize(mapinfo.width, mapinfo.height), wxSize(width, height));
      img.Rescale( image_size.GetWidth(), image_size.GetHeight() );
    }
    catch (...) {
      img = wxImage( 1, 1 );
    }
  }

  if ( tiny ) m_tiny_minimap_cache.Add( mapname, img );

  return img;
}


wxImage SpringUnitSync::GetMetalmap( const wxString& mapname )
{
  wxLogDebugFunc( mapname );
  return _GetMapImage( mapname, _T(".metalmap.png"), &SpringUnitSyncLib::GetMetalmap );
}


wxImage SpringUnitSync::GetMetalmap( const wxString& mapname, int width, int height )
{
  wxLogDebugFunc( mapname + _T(" size: ") + TowxString( width ) + _T("x") + TowxString( height ) );
  return _GetScaledMapImage( mapname, &SpringUnitSync::GetMetalmap, width, height );
}


wxImage SpringUnitSync::GetHeightmap( const wxString& mapname )
{
  wxLogDebugFunc( mapname );
  return _GetMapImage( mapname, _T(".heightmap.png"), &SpringUnitSyncLib::GetHeightmap );
}


wxImage SpringUnitSync::GetHeightmap( const wxString& mapname, int width, int height )
{
  wxLogDebugFunc( mapname + _T(" size: ") + TowxString( width ) + _T("x") + TowxString( height ) );
  return _GetScaledMapImage( mapname, &SpringUnitSync::GetHeightmap, width, height );
}


wxImage SpringUnitSync::_GetMapImage( const wxString& mapname, const wxString& imagename, wxImage (SpringUnitSyncLib::*loadMethod)(const wxString&) )
{
  wxImage img;

  if ( m_map_image_cache.TryGet( mapname + imagename, img ) ) return img;

  wxString originalsizepath = GetFileCachePath( mapname, _T(""), false ) + imagename;

  try
  {
    ASSERT_EXCEPTION( wxFileExists( originalsizepath ), _T("File cached image does not exist") );

    img = wxImage( originalsizepath, wxBITMAP_TYPE_PNG );
    ASSERT_EXCEPTION( img.Ok(), _T("Failed to load cache image") );
  }
  catch (...)
  {
    try
    {
      img = (susynclib().*loadMethod)( mapname );

      img.SaveFile( originalsizepath, wxBITMAP_TYPE_PNG );
    }
    catch (...)
    {
      img = wxImage( 1, 1 );
    }
  }

  m_map_image_cache.Add( mapname + imagename, img );

  return img;
}


wxImage SpringUnitSync::_GetScaledMapImage( const wxString& mapname, wxImage (SpringUnitSync::*loadMethod)(const wxString&), int width, int height )
{
	ActivityNoticeContainer act_notice( NULL, mapname, _T("Loading metalmap %s") );
  wxImage img = (this->*loadMethod) ( mapname );

  if (img.GetWidth() > 1 && img.GetHeight() > 1)
  {
    wxSize image_size = MakeFit(wxSize(img.GetWidth(), img.GetHeight()), wxSize(width, height));
    img.Rescale( image_size.GetWidth(), image_size.GetHeight() );
  }

  return img;
}


MapInfo SpringUnitSync::_GetMapInfoEx( const wxString& mapname )
{
  MapInfo info;

  if ( m_mapinfo_cache.TryGet( mapname, info ) ) return info;

  wxArrayString cache;
  try
  {
    cache = GetCacheFile( GetFileCachePath( mapname, _T(""), false ) + _T(".infoex") );

    ASSERT_EXCEPTION( cache.GetCount() >= 11, _T("not enough lines found in cache info ex") );
    info.author = cache[0];
    info.tidalStrength =  s2l( cache[1] );
    info.gravity = s2l( cache[2] );
    info.maxMetal = s2d( cache[3] );
    info.extractorRadius = s2d( cache[4] );
    info.minWind = s2l( cache[5] );
    info.maxWind = s2l( cache[6] );
    info.width = s2l( cache[7] );
    info.height = s2l( cache[8] );
    info.posCount = s2l( cache[9] );

    wxArrayString posinfo = wxStringTokenize( cache[10], _T(' '), wxTOKEN_RET_EMPTY );
    for ( int i = 0; i < info.posCount; i++)
    {
       StartPos position;
       position.x = s2l( posinfo[i].BeforeFirst( _T('-') ) );
       position.y = s2l( posinfo[i].AfterFirst( _T('-') ) );
       info.positions[i] = position;
    }

    unsigned int LineCount = cache.GetCount();
    for ( unsigned int i = 11; i < LineCount; i++ ) info.description << cache[i] << _T('\n');

  }
  catch (...)
  {
    info = susynclib().GetMapInfoEx( mapname, 1 );

    cache.Add ( info.author );
    cache.Add( TowxString( info.tidalStrength ) );
    cache.Add( TowxString( info.gravity ) );
    cache.Add( TowxString( info.maxMetal ) );
    cache.Add( TowxString( info.extractorRadius ) );
    cache.Add( TowxString( info.minWind ) );
    cache.Add( TowxString( info.maxWind )  );
    cache.Add( TowxString( info.width ) );
    cache.Add( TowxString( info.height ) );
    cache.Add( TowxString( info.posCount ) );

    wxString postring;
    for ( int i = 0; i < info.posCount; i++)
    {
       postring << TowxString( info.positions[i].x ) << _T('-') << TowxString( info.positions[i].y ) << _T(' ');
    }
    cache.Add( postring );

    wxArrayString descrtoken = wxStringTokenize( info.description, _T('\n') );
    unsigned int desclinecount = descrtoken.GetCount();
    for ( unsigned int count = 0; count < desclinecount; count++ ) cache.Add( descrtoken[count] );

    SetCacheFile( GetFileCachePath( mapname, _T(""), false ) + _T(".infoex"), cache );
  }

  m_mapinfo_cache.Add( mapname, info );

  return info;
}


bool SpringUnitSync::ReloadUnitSyncLib()
{
  return LoadUnitSyncLib( sett().GetCurrentUsedUnitSync() );
}


void SpringUnitSync::SetSpringDataPath( const wxString& path )
{
  susynclib().SetSpringConfigString( _T("SpringData"), path );
}


wxString SpringUnitSync::GetFileCachePath( const wxString& name, const wxString& hash, bool IsMod )
{
  LOCK_UNITSYNC;

  wxString ret = m_cache_path;
  if ( !name.IsEmpty() ) ret << name;
  else return wxEmptyString;
  if ( !hash.IsEmpty() ) ret << hash;
  else
  {
    if ( IsMod ) ret <<  _T("-") << m_mods_list[name];
    else
    {
       ret << _T("-") << m_maps_list[name];
    }
  }
  return ret;
}


wxArrayString SpringUnitSync::GetCacheFile( const wxString& path )
{
  wxArrayString ret;
  wxTextFile file( path );
  file.Open();
  ASSERT_EXCEPTION( file.IsOpened() , wxString::Format( _T("cache file( %s ) not found"), path.c_str() ) );
  unsigned int linecount = file.GetLineCount();
  for ( unsigned int count = 0; count < linecount; count ++ )
  {
    ret.Add( file[count] );
  }
  return ret;
}


void SpringUnitSync::SetCacheFile( const wxString& path, const wxArrayString& data )
{
  wxTextFile file( path );
  unsigned int arraycount = data.GetCount();
  for ( unsigned int count = 0; count < arraycount; count++ )
  {
    file.AddLine( data[count] );
  }
  file.Write();
  file.Close();
}

void SpringUnitSync::GetReplayList(std::vector<wxString> &ret)
{
  ret.clear();
  wxLogDebug( _T("") );

  if ( !IsLoaded() ) return;

  ActivityNoticeContainer act_notice( 0, _("Loading replaylist"), _T("%s") );

  int ini = susynclib().InitFindVFS( _T("demos/*.sdf") );

  wxString FilePath ;
  //wxArrayString ret;
  do
  {
    ini = susynclib().FindFilesVFS ( ini, FilePath );
    wxString FileName = wxString ( FilePath, wxConvUTF8 );
    //ret.Add ( FileName );
    ret.push_back(FileName);
  } while (ini != 0);

    int kol = ret.size();

  std::sort(ret.begin(),ret.end());
  std::vector<wxString>::iterator i=std::unique(ret.begin(),ret.end());
  ret.resize(i - ret.begin());

  kol = ret.size();
}

bool SpringUnitSync::FileExists( const wxString& name )
{
  int handle = susynclib().OpenFileVFS(name);
  if ( handle == 0 ) return false;
  susynclib().CloseFileVFS(handle);
  return true;
}


wxString SpringUnitSync::GetArchivePath( const wxString& name )
{
  wxLogDebugFunc( name );

  return susynclib().GetArchivePath( name );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////// Unitsync prefetch/background thread code

namespace
{
  typedef wxImage (SpringUnitSync::*LoadMethodPtr)(const wxString&);
  typedef wxImage (SpringUnitSync::*ScaledLoadMethodPtr)(const wxString&, int, int);

  class CacheMapWorkItem : public WorkItem
  {
    public:
      SpringUnitSync* m_usync;
      wxString m_mapname;
      LoadMethodPtr m_loadMethod;

      void Run()
      {
        (m_usync->*m_loadMethod)( m_mapname );
      }

      CacheMapWorkItem( SpringUnitSync* usync, const wxString& mapname, LoadMethodPtr loadMethod )
        : m_usync(usync), m_mapname(mapname.c_str()), m_loadMethod(loadMethod) {}
  };

  class CacheMinimapWorkItem : public WorkItem
  {
    public:
      wxString m_mapname;

      void Run()
      {
        // Fetch rescaled minimap using this specialized class instead of
        // CacheMapWorkItem with a pointer to SpringUnitSync::GetMinimap,
        // to ensure SpringUnitSync::_GetMapInfoEx will be called too, and
        // hence it's data cached.

        // This reduces main thread blocking while waiting for WorkerThread
        // to release it's lock while e.g. scrolling through battle list.

        // 98x98 because battle list map preview is 98x98
        usync().GetMinimap( m_mapname, 98, 98 );
      }

      CacheMinimapWorkItem( const wxString& mapname )
        : m_mapname(mapname.c_str()) {}
  };

  class GetMapImageAsyncResult : public WorkItem // TODO: rename
  {
    public:
      void Run()
      {
        try
		{
          RunCore();
        }
        catch (...)
        {
          // Event without mapname means some async job failed.
          // This is sufficient for now, we just need symmetry between
          // number of initiated async jobs and number of finished/failed
          // async jobs.
          m_mapname = wxEmptyString;
        }
        PostEvent();
      }

    protected:
      SpringUnitSync* m_usync;
      wxString m_mapname;
      int m_evtHandlerId;
      int m_evtId;

      void PostEvent()
      {
        wxCommandEvent evt( UnitSyncAsyncOperationCompletedEvt, m_evtId );
        evt.SetString( m_mapname );
        m_usync->PostEvent( m_evtHandlerId, evt );
      }

      virtual void RunCore() = 0;

      GetMapImageAsyncResult( SpringUnitSync* usync, const wxString& mapname, int evtHandlerId, int evtId )
        : m_usync(usync), m_mapname(mapname.c_str()), m_evtHandlerId(evtHandlerId), m_evtId(evtId) {}
  };

  class GetMapImageAsyncWorkItem : public GetMapImageAsyncResult
  {
    public:
      void RunCore()
      {
		(m_usync->*m_loadMethod)( m_mapname );
      }

      LoadMethodPtr m_loadMethod;

      GetMapImageAsyncWorkItem( SpringUnitSync* usync, const wxString& mapname, int evtHandlerId, LoadMethodPtr loadMethod )
        : GetMapImageAsyncResult( usync, mapname, evtHandlerId, 1 ), m_loadMethod(loadMethod) {}
  };

  class GetScaledMapImageAsyncWorkItem : public GetMapImageAsyncResult
  {
    public:
      void RunCore()
      {
        (m_usync->*m_loadMethod)( m_mapname, m_width, m_height );
      }

      int m_width;
      int m_height;
      ScaledLoadMethodPtr m_loadMethod;

      GetScaledMapImageAsyncWorkItem( SpringUnitSync* usync, const wxString& mapname, int w, int h, int evtHandlerId, ScaledLoadMethodPtr loadMethod )
        : GetMapImageAsyncResult( usync, mapname, evtHandlerId, 2 ), m_width(w), m_height(h), m_loadMethod(loadMethod) {}
  };

  class GetMapExAsyncWorkItem : public GetMapImageAsyncResult
  {
    public:
      void RunCore()
      {
        m_usync->GetMapEx( m_mapname );
      }

      GetMapExAsyncWorkItem( SpringUnitSync* usync, const wxString& mapname, int evtHandlerId )
        : GetMapImageAsyncResult( usync, mapname, evtHandlerId, 3 ) {}
  };
};


void SpringUnitSync::PrefetchMap( const wxString& mapname )
{
  wxLogDebugFunc( mapname );

  // Use a simple hash based on 3 characters from the mapname
  // (without '.smf') as negative priority for the WorkItems.
  // This ensures WorkItems for the same map are put together,
  // which improves caching performance.

  // Measured improvement: 60% more cache hits while populating replay tab.
  // 50% hits without, 80% hits with this code.  (cache size 20 images)

  const int length = std::max(0, int(mapname.length()) - 4);
  const int hash = (mapname[length * 1/4] << 16)
                 | (mapname[length * 2/4] << 8)
                 | (mapname[length * 3/4]);
  const int priority = -hash;

  {
    CacheMinimapWorkItem* work;

    work = new CacheMinimapWorkItem( mapname );
    m_cache_thread.DoWork( work, priority );
  }
  {
    CacheMapWorkItem* work;

    work = new CacheMapWorkItem( this, mapname, &SpringUnitSync::GetMetalmap );
    m_cache_thread.DoWork( work, priority );

    work = new CacheMapWorkItem( this, mapname, &SpringUnitSync::GetHeightmap );
    m_cache_thread.DoWork( work, priority );
  }
}

int SpringUnitSync::RegisterEvtHandler( wxEvtHandler* evtHandler )
{
  return m_evt_handlers.Add( evtHandler );
}

void SpringUnitSync::UnregisterEvtHandler( int evtHandlerId )
{
  m_evt_handlers.Remove( evtHandlerId );
}

void SpringUnitSync::PostEvent( int evtHandlerId, wxEvent& evt )
{
  m_evt_handlers.PostEvent( evtHandlerId, evt );
}

void SpringUnitSync::_GetMapImageAsync( const wxString& mapname, wxImage (SpringUnitSync::*loadMethod)(const wxString&), int evtHandlerId )
{
  GetMapImageAsyncWorkItem* work;

  work = new GetMapImageAsyncWorkItem( this, mapname, evtHandlerId, loadMethod );
  m_cache_thread.DoWork( work, 100 );
}

void SpringUnitSync::GetMinimapAsync( const wxString& mapname, int evtHandlerId )
{
  wxLogDebugFunc( mapname );
  _GetMapImageAsync( mapname, &SpringUnitSync::GetMinimap, evtHandlerId );
}

void SpringUnitSync::GetMinimapAsync( const wxString& mapname, int width, int height, int evtHandlerId )
{
  wxLogDebugFunc( mapname + _T(" size: ") + TowxString(width) + _T("x") + TowxString(height) );

  GetScaledMapImageAsyncWorkItem* work;

  work = new GetScaledMapImageAsyncWorkItem( this, mapname, width, height, evtHandlerId, &SpringUnitSync::GetMinimap );
  m_cache_thread.DoWork( work, 100 );
}

void SpringUnitSync::GetMetalmapAsync( const wxString& mapname, int evtHandlerId )
{
  wxLogDebugFunc( mapname );
  _GetMapImageAsync( mapname, &SpringUnitSync::GetMetalmap, evtHandlerId );
}

void SpringUnitSync::GetMetalmapAsync( const wxString& mapname, int width, int height, int evtHandlerId )
{
  GetMetalmapAsync( mapname, evtHandlerId );
}

void SpringUnitSync::GetHeightmapAsync( const wxString& mapname, int evtHandlerId )
{
  wxLogDebugFunc( mapname );
  _GetMapImageAsync( mapname, &SpringUnitSync::GetHeightmap, evtHandlerId );
}

void SpringUnitSync::GetHeightmapAsync( const wxString& mapname, int width, int height, int evtHandlerId )
{
  GetHeightmapAsync( mapname, evtHandlerId );
}

void SpringUnitSync::GetMapExAsync( const wxString& mapname, int evtHandlerId )
{
  wxLogDebugFunc( mapname );

  GetMapExAsyncWorkItem* work;

  work = new GetMapExAsyncWorkItem( this, mapname, evtHandlerId );
  m_cache_thread.DoWork( work, 200 /* higher prio then GetMinimapAsync */ );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////// EvtHandlerCollection code

int EvtHandlerCollection::Add( wxEvtHandler* evtHandler )
{
  wxCriticalSectionLocker lock(m_lock);
  ++m_last_id;
  m_items[m_last_id] = evtHandler;
  return m_last_id;
}

void EvtHandlerCollection::Remove( int evtHandlerId )
{
  wxCriticalSectionLocker lock(m_lock);
  EvtHandlerMap::iterator it = m_items.find( evtHandlerId );
  if ( it != m_items.end() ) m_items.erase( it );
}

void EvtHandlerCollection::PostEvent( int evtHandlerId, wxEvent& evt )
{
  wxCriticalSectionLocker lock(m_lock);
  EvtHandlerMap::iterator it = m_items.find( evtHandlerId );
  if ( it != m_items.end() ) wxPostEvent( it->second, evt );
}
