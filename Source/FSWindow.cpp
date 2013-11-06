// FolderShaper
// Jonas Sundström, jonas@kirilla.com

#include <stdio.h>
#include <stdlib.h>

#include <Application.h>
#include <Beep.h>
#include <File.h>
#include <FindDirectory.h>
#include <InterfaceKit.h>
#include <Path.h>
#include <Roster.h>
#include <String.h>

#include "FSDefs.h"
#include "FSWindow.h"
#include "FSDropViews.h"
#include "IconMenu.h"

#define DEBUG 0
#include <Debug.h>

FolderShaperWindow::FolderShaperWindow(BPoint a_winloc, bool a_do_move, int8 a_do_open,
									bool a_do_clobber, bool a_do_keep_position, bool a_do_clean_up)
:	BWindow(BRect(300,200,495,435), "FolderShaper", B_TITLED_WINDOW, B_NOT_ZOOMABLE | B_NOT_RESIZABLE)
{
	m_menubar		= new BMenuBar(BRect(0,0,195,20), "menubar");
	m_appmenu		= new BMenu("app");
	m_settingsmenu	= new BMenu("settings");
	
	m_iconmenu		= new IconMenu(m_appmenu);
	
	m_menubar->		AddItem(m_iconmenu);
	m_menubar->		AddItem(m_settingsmenu);
	
	m_aboutitem 	= new BMenuItem("About" B_UTF8_ELLIPSIS, new BMessage('abou'), 'A');
	m_help_item		= new BMenuItem("Help" B_UTF8_ELLIPSIS, new BMessage('help'), 'H');
	m_quititem 		= new BMenuItem("Quit", new BMessage(B_QUIT_REQUESTED), 'Q');
	m_appmenu->		AddItem(m_aboutitem);
	m_appmenu->		AddItem(m_help_item);
	m_appmenu->		AddSeparatorItem();
	m_appmenu->		AddItem(m_quititem);

	m_moveitem 			= new BMenuItem("Move original folder to Trash", new BMessage('move'));
	m_deleitem 			= new BMenuItem("Delete original folder", new BMessage('dele'));
	m_settingsmenu->	AddItem(m_moveitem);
	m_settingsmenu->	AddItem(m_deleitem);

	m_settingsmenu->	AddSeparatorItem();
	m_keep_pos_item		= new BMenuItem("Keep position", new BMessage('kpos'));
	m_remove_pos_item	= new BMenuItem("New position", new BMessage('rpos'));
	m_settingsmenu->	AddItem(m_keep_pos_item);
	m_settingsmenu->	AddItem(m_remove_pos_item);

	m_settingsmenu->	AddSeparatorItem();
	m_clobber_item		= new BMenuItem("Replace original contents", new BMessage('clob'));
	m_settingsmenu->	AddItem(m_clobber_item);

	m_settingsmenu->	AddSeparatorItem();
	m_clean_up_item		= new BMenuItem("Clean Up All", new BMessage('clup'));
	m_settingsmenu->	AddItem(m_clean_up_item);

	m_settingsmenu->	AddSeparatorItem();
	m_alwaysitem 		= new BMenuItem("Open", new BMessage('alwa'));
	m_keepitem			= new BMenuItem("Keep open", new BMessage('keep'));
	m_neveritem 		= new BMenuItem("Close", new BMessage('neve'));
	m_settingsmenu->	AddItem(m_alwaysitem);
	m_settingsmenu->	AddItem(m_keepitem);
	m_settingsmenu->	AddItem(m_neveritem);

	m_settingsmenu->	AddSeparatorItem();
	m_tmplitem 			= new BMenuItem("Open templates folder" B_UTF8_ELLIPSIS, new BMessage('tmpl'), 'T');
	m_settingsmenu->		AddItem(m_tmplitem);
		
	float width, height;
	m_menubar->	GetPreferredSize(& width, & height);
	
	topbox	=	new BBox(BRect(1,height+2,Bounds().right,Bounds().bottom), "view", B_FOLLOW_ALL_SIDES, B_NAVIGABLE);

	topbox	->AddChild(new	FSDropTemplateBox());
	topbox	->AddChild(new	FSDropFolderBox());

	AddChild(m_menubar);
	AddChild(topbox);

	MoveTo(a_winloc);
	
	m_do_move 			=	a_do_move;
	m_do_open 			=	a_do_open;
	m_do_clobber		=	a_do_clobber;
	m_do_keep_position	=	a_do_keep_position;
	m_do_clean_up		=	a_do_clean_up;
	
	if (m_do_move)	m_moveitem	->SetMarked(true);	
	else			m_deleitem	->SetMarked(true);

	if (m_do_clobber)	m_clobber_item	->SetMarked(true);	

	if (m_do_keep_position)	m_keep_pos_item		->SetMarked(true);	
	else					m_remove_pos_item	->SetMarked(true);
	
	if (m_do_open == FS_ALWAYS_OPEN)		m_alwaysitem	->SetMarked(true);
	if (m_do_open == FS_NEVER_OPEN)			m_neveritem		->SetMarked(true);
	if (m_do_open == FS_IF_ORIGINAL_OPEN)	m_keepitem		->SetMarked(true);

	if (m_do_clean_up)	m_clean_up_item	->SetMarked(true);
	
	ResizeBy(0, height+2);
}

FolderShaperWindow::~FolderShaperWindow()	{/*nothing*/}

void 
FolderShaperWindow::MessageReceived(BMessage * a_message)
{
	switch(a_message->what)
	{
		case 'move':										// Move
					m_do_move = true;
					m_moveitem->SetMarked(true);	
					m_deleitem->SetMarked(false);
					Save();
				break;
		case 'dele':										// Delete 
					m_do_move = false;
					m_moveitem->SetMarked(false);	
					m_deleitem->SetMarked(true);
					Save();
				break;
		case 'alwa':										// Do open
					m_do_open = FS_ALWAYS_OPEN;
					m_alwaysitem->SetMarked(true);
					m_neveritem->SetMarked(false);
					m_keepitem->SetMarked(false);
					Save();
				break;
		case 'neve':										// Don't open
					m_do_open = FS_NEVER_OPEN;
					m_alwaysitem->SetMarked(false);
					m_neveritem->SetMarked(true);
					m_keepitem->SetMarked(false);
					Save();
				break;
		case 'keep':										// Open if Original was open
					m_do_open = FS_IF_ORIGINAL_OPEN;
					m_alwaysitem->SetMarked(false);
					m_neveritem->SetMarked(false);
					m_keepitem->SetMarked(true);
					Save();
				break;
		case 'kpos':										// Use the X/Y position of the original
					m_do_keep_position = true;
					m_keep_pos_item->SetMarked(true);
					m_remove_pos_item->SetMarked(false);
					Save();
				break;			
		case 'rpos':										// Remove X/Y position from template folder
					m_do_keep_position = false;
					m_keep_pos_item->SetMarked(false);
					m_remove_pos_item->SetMarked(true);
					Save();
				break;
		case 'clob':										// Template contents takes precedence
					if (m_do_clobber == false)
					{
						m_do_clobber = true;
						m_clobber_item->SetMarked(true);
					}
					else
					{
						m_do_clobber = false;
						m_clobber_item->SetMarked(false);
					}
					Save();
				break;
		case 'clup':										// Remove all X/Y pos. info of New folder contents
					if (m_do_clean_up == false)
					{
						m_do_clean_up = true;
						m_clean_up_item->SetMarked(true);
					}
					else
					{
						m_do_clean_up = false;
						m_clean_up_item->SetMarked(false);
					}
					Save();
				break;
		case 'help':										// Help -- documentation, settings
				be_app->PostMessage('help');
				break; 
		case 'tmpl':										// Open Templates Folder -button
				be_app->PostMessage('tmpl');
				break;
		case 'abou':										// About -button
				be_app->PostMessage(B_ABOUT_REQUESTED);
				break;
		case 'quit':										// Quit, Alt-Q, whatever
				Quit();
				break;	
		case 'drpt':										// Template drop
				be_app->PostMessage(a_message);
				break;	
		case 'drpf':										// Folder drop
				be_app->PostMessage(a_message);
				break;	
		default:
				BWindow::MessageReceived(a_message);
			break;			
	}
}

void
FolderShaperWindow::ErrorMessage(const char * a_text, int32 a_status)
{
	PRINT(("%s: %s\n", a_text, strerror(a_status)));
}

void
FolderShaperWindow::Quit()
{
	Save();
	be_app->PostMessage(B_QUIT_REQUESTED);
	BWindow::Quit();
}

void
FolderShaperWindow::Save()
{
	BMessage msg('sett');
	msg.AddPoint("winloc",Frame().LeftTop());
	msg.AddBool	("do_move", m_do_move);
	msg.AddInt8	("do_open", m_do_open);
	msg.AddBool	("do_clobber", m_do_clobber);
	msg.AddBool	("do_keep_position", m_do_keep_position);
	msg.AddBool	("do_clean_up", m_do_clean_up);
	
	be_app->PostMessage(& msg);
}

BPoint
FolderShaperWindow::GetMouse()
{
	BPoint point;
	uint32 buttons;	
	topbox->GetMouse(&point, &buttons);
	point = ConvertToScreen(point);
	
	float width, height;
	m_menubar->	GetPreferredSize(&width, &height);

	point.Set(point.x, point.y+height+2);
		
	return point;
}

