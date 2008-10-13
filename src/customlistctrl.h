#ifndef CUSTOMLISTITEM_H_
#define CUSTOMLISTITEM_H_

#ifndef __WXMSW__
    #include <wx/listctrl.h>
    typedef wxListCtrl ListBaseType;
#else
    #include "Helper/listctrl.h"
    typedef SL_Extern::wxGenericListCtrl ListBaseType;
#endif

#if wxUSE_TIPWINDOW
    #include <wx/tipwin.h>
#endif

#include <wx/timer.h>
#define IDD_TIP_TIMER 696

#include <vector>
#include <utility>

#include "useractions.h"

#if wxUSE_TIPWINDOW
class SLTipWindow : public wxTipWindow{
    public:
        SLTipWindow(wxWindow *parent, const wxString &text)
            :wxTipWindow(parent,text){};
        void Cancel(wxMouseEvent& event);

        DECLARE_EVENT_TABLE()
};
#endif

/** \brief Used as base class for all ListCtrls throughout SL
 * Provides generic functionality, such as column tooltips, possiblity to prohibit coloumn resizing and selection modifiers. \n
 * Some of the provided functionality only makes sense for single-select lists (see grouping) \n
 * Note: Tooltips are a bitch and anyone shoudl feel to revise them (koshi)
 */
class CustomListCtrl : public ListBaseType
{
protected:
    typedef UserActions::ActionType ActionType;
    //! used to display tooltips for a certain amount of time
    wxTimer m_tiptimer;
    //! always set to the currrently displayed tooltip text
    wxString m_tiptext;
    #if wxUSE_TIPWINDOW
    //! some wx implementations do not support this yet
    SLTipWindow* m_tipwindow;
    SLTipWindow** controlPointer;
    #endif
    int coloumnCount;

    typedef std::pair<wxString,bool> colInfo;
    typedef std::vector<colInfo> colInfoVec;

    /** global Tooltip thingies (ms)
     */
    static const unsigned int m_tooltip_delay    = 1000;
    static const unsigned int m_tooltip_duration = 2000;

/*** these are only meaningful in single selection lists ***/
    //! curently selected data
    long m_selected;
    //! index of curently selected data
    long m_selected_index;
    //! previously selected data
    long m_prev_selected;
    //! index of previously selected data
    long m_prev_selected_index;
/***********************************************************/

    //! stores info about the columns (wxString name,bool isResizable) - pairs
    colInfoVec m_colinfovec;
    //! primarily used to get coulumn index in mousevents (from cur. mouse pos)
    int getColoumnFromPosition(wxPoint pos);

    wxPoint m_last_mouse_pos;

    //! used as label for saving column widths
    wxString m_name;

    //!controls if highlighting should be considered
    bool m_highlight;

    //! which action should be considered?
    ActionType m_highlightAction;

    wxColor m_bg_color;

    //! list should be sorted
    bool m_dirty_sort;

    virtual void SetTipWindowText( const long item_hit, const wxPoint position);

public:
    CustomListCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pt,
                    const wxSize& sz,long style, wxString name, bool highlight = true,
                    UserActions::ActionType hlaction = UserActions::ActHighlight);

    virtual ~CustomListCtrl(){}

    void OnSelected( wxListEvent& event );
    void OnDeselected( wxListEvent& event );
    /** @name Single Selection methods
     * using these funcs in a multi selection list is meaingless at best, harmful in the worst case
     * \todo insert debug asserts to catch that
     * @{
     */
    long GetSelectedIndex();
    void SetSelectedIndex(const long newindex);
    long GetSelectedData();
    long GetIndexFromData( const unsigned long data );
    //! call this before example before sorting, inserting, etc
    void SetSelectionRestorePoint();
    void ResetSelection();
    //! and this afterwards
    void RestoreSelection();
    /** @}
     */

    //! intermediate function to add info to m_colinfovec after calling base class function
    void InsertColumn(long i, wxListItem item, wxString tip, bool = true);
    //! this event is triggered when delay timer (set in mousemotion) ended
    virtual void OnTimer(wxTimerEvent& event);
    //! prohibits resizin if so set in columnInfo
    void OnStartResizeCol(wxListEvent& event);
    //! we use this to automatically save column width after resizin
    virtual void OnEndResizeCol(wxListEvent& event);
    //! starts timer, sets tooltiptext
    virtual void OnMouseMotion(wxMouseEvent& event);
    //! does nothing
    void noOp(wxMouseEvent& event);
    //! automatically get saved column width if already saved, otherwise use parameter and save new width
    virtual bool SetColumnWidth(int col, int width);

    // funcs that should make things easier for group highlighting
    ///all that needs to be implemented in child class for UpdateHighlights to work
    virtual void HighlightItem( long item ) = 0;
    void HighlightItemUser( long item, const wxString& name );
    void UpdateHighlights();
    void SetHighLightAction( UserActions::ActionType action );


    /** @name Multi Selection methods
     * using these funcs in a single selection list is meaingless at best, harmful in the worst case
     * \todo insert debug asserts to catch that
     * @{
     */
    void SelectAll();
    void SelectInverse();
    /** @}
     */

    //! sets selected index to -1
    void SelectNone();

    //! marks the items in the control to be sorted
    void MarkDirtySort();

    DECLARE_EVENT_TABLE()
};


#endif /*CUSTOMLISTITEM_H_*/
