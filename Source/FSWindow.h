#include <MenuBar.h>
#include <Bitmap.h>
#include <Menu.h>
#include <MenuItem.h>

#include "IconMenu.h"

#ifndef _FSWINDOW_H
#define _FSWINDOW_H

class FolderShaperWindow : public BWindow
{
public:
					FolderShaperWindow	(BPoint a_winloc, bool a_do_move, int8 a_do_open, 
										bool a_do_clobber, bool a_do_keep_position, bool a_do_clean_up);
					~FolderShaperWindow	();
	virtual void	MessageReceived 	(BMessage * a_message);
	virtual void	Quit				();
			void	Save				();
			void	ErrorMessage		(const char * a_text, int32 a_status);
			BPoint	GetMouse			();
			
private:
	bool	m_do_move;
	int8	m_do_open;
	bool	m_do_clobber;
	bool	m_do_keep_position;
	bool	m_do_clean_up;
	
	BBox		*	topbox;
	
	BMenuBar	*	m_menubar;
	IconMenu	*	m_iconmenu;
	BBitmap		*	m_bitmap;
	
	BMenu		*	m_appmenu;
	BMenu		*	m_settingsmenu;

	BMenuItem	*	m_aboutitem;
	BMenuItem	*	m_quititem;
	BMenuItem	*	m_help_item;

	BMenuItem	*	m_moveitem;
	BMenuItem	*	m_deleitem;
	BMenuItem	*	m_tmplitem;

	BMenuItem	*	m_neveritem;
	BMenuItem	*	m_alwaysitem;
	BMenuItem	*	m_keepitem;
	
	BMenuItem	*	m_keep_pos_item;
	BMenuItem	*	m_remove_pos_item;
	
	BMenuItem	*	m_clobber_item;

	BMenuItem	*	m_clean_up_item;

};

#endif

