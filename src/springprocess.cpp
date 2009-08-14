/* Copyright (C) 2007 The SpringLobby Team. All rights reserved. */

#include "springprocess.h"
#include "spring.h"
#include "utils/debug.h"
#include "utils/conversion.h"
#include <wx/log.h>


DEFINE_LOCAL_EVENT_TYPE( wxEVT_SPRING_EXIT )


SpringProcess::SpringProcess( Spring& sp ) :
		m_spring( sp ), m_exit_code( 0 )
{
	wxLogDebugFunc( _T( "" ) );
}


SpringProcess::~SpringProcess()
{
	wxLogDebugFunc( _T( "" ) );
}


void SpringProcess::SetCommand( const wxString& cmd )
{
	m_cmd = cmd;
}


void SpringProcess::OnExit()
{
	wxLogDebugFunc( _T( "" ) );
	wxCommandEvent event( wxEVT_SPRING_EXIT, PROC_SPRING );
	event.SetExtraLong( m_exit_code );
	m_spring.AddPendingEvent( event );
}


void* SpringProcess::Entry()
{
	wxLogDebugFunc( _T( "" ) );
	m_exit_code = system( m_cmd.mb_str( wxConvUTF8 ) );
	wxLogMessage( _T( "Spring closed." ) );
	return 0;
}

wxSpringProcess::wxSpringProcess( Spring& sp ) :
		m_spring( sp )
{
	wxLogDebugFunc( _T( "" ) );
    Redirect();
}


wxSpringProcess::~wxSpringProcess()
{
	wxLogDebugFunc( _T( "" ) );
}

void wxSpringProcess::OnTerminate( int pid, int status )
{
	wxCommandEvent event( wxEVT_SPRING_EXIT, PROC_SPRING );
	event.SetExtraLong( status );
	m_spring.AddPendingEvent( event );
}

