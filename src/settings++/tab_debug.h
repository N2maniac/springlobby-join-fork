/**
    This file is part of springsettings,
    Copyright (C) 2007
    Original work by Kloot
    cross-plattform/UI adaptation and currently maintained by koshi (Ren� Milk)
    visit http://spring.clan-sy.com/phpbb/viewtopic.php?t=12104
    for more info/help

    springsettings is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    springsettings is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with springsettings.  If not, see <http://www.gnu.org/licenses/>.
**/

#ifndef __TAB_DEBUG_h__
#define __TAB_DEBUG_h__

#include "tab_abstract.h"

class wxCheckBox;
class wxSlider;
class wxStaticBoxSizer;
class wxString;
class wxWindow;
class wxPoint;
class wxSize;
class wxCloseEvent;

class debug_panel : public abstract_panel
{

	public:
		debug_panel(wxWindow *parent, wxWindowID id = 1, const wxString &title = wxT("Project2"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0);
		virtual ~debug_panel();

		void initDebugSizer(wxStaticBoxSizer*);
		void updateControls(int);
    protected:
        void OnClose(wxCloseEvent& event);
		void CreateGUIControls();
		wxCheckBox* checkBox0 ;
		wxCheckBox* checkBox1 ;
		wxSlider* slider;
		
		wxSizer* parentSizer ;	// main window sizer (three columns)
			wxSizer* childLSizer ;		// main window left column sizer

			// sizers for static boxes containing sliders, checkboxes, radiobuttons
			wxStaticBoxSizer* debugSizer;
		
		DECLARE_EVENT_TABLE()
};

#endif