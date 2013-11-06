// FolderShaper
// Jonas Sundström, jonas@kirilla.com

#include <Box.h>
#include <Message.h>
#include <Window.h>

#include "FSDropViews.h"

#define DEBUG 0
#include <Debug.h>

/* FSDropTemplateBox */

FSDropTemplateBox::FSDropTemplateBox()
: BBox(BRect(5,5,190,115), "DropTemplates", B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP)
{
	SetLabel("Drop Template folders here");
}

FSDropTemplateBox::~FSDropTemplateBox()	{/*nothing*/}

void 
FSDropTemplateBox::MessageReceived(BMessage * a_message)
{
	switch(a_message->what)
	{
		case 'DATA':
				a_message->what = 'drpt';
				Window()->MessageReceived(a_message);
				break;
		default:
				a_message->PrintToStream();
				BBox::MessageReceived(a_message);
			break;			
	}
}

/* FSDropFolderBox */

FSDropFolderBox::FSDropFolderBox()
: BBox(BRect(5,120,190,230), "DropTemplates", B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP)
{
	SetLabel("Drop Folders here");
}

FSDropFolderBox::~FSDropFolderBox()	{/*nothing*/}

void 
FSDropFolderBox::MessageReceived(BMessage * a_message)
{
	switch(a_message->what)
	{
		case 'DATA':
				a_message->what = 'drpf';
				Window()->MessageReceived(a_message);
				break;
		default:
				BBox::MessageReceived(a_message);
			break;		
	}
}
