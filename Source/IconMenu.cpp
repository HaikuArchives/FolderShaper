//------------------------------------------------------------------------------
// IconMenu.cpp
//------------------------------------------------------------------------------
// A menu item implementation that displays an icon as its label.
//
// IconMenu implementation Copyright (C) 1998 Tyler Riti <fizzboy@mail.utexas.edu>
// Based on code Copyright (C) 1997 Jens Kilian
// This code is free to use in any way so long as the credits above remain intact.
// This code carries no warranties or guarantees of any kind. Use at your own risk.
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// I N C L U D E S
//------------------------------------------------------------------------------

#include "IconMenu.h"
#include <Bitmap.h>
#include <MenuItem.h>
#include <Menu.h>
#include <Rect.h>
#include <Application.h>
#include <Roster.h>
#include <AppFileInfo.h>

//------------------------------------------------------------------------------
// I M P L E M E N T A T I O N
//------------------------------------------------------------------------------

IconMenu::IconMenu(BBitmap* icon, BMenu* menu) : 
	BMenuItem(menu),
	bounds(),
	iconLabel(NULL)
{
	if (icon) {
		bounds = icon->Bounds();
		iconLabel = new BBitmap(bounds, B_COLOR_8_BIT);
		iconLabel->SetBits(icon->Bits(), icon->BitsLength(), 0, B_COLOR_8_BIT);
	}
}

IconMenu::IconMenu(BMenu* menu) : 
	BMenuItem(menu),
	bounds(0.0, 0.0, 15.0, 15.0),
	iconLabel(NULL)
{
	app_info info;
	if (be_app->GetAppInfo(&info) == B_NO_ERROR) {
		BFile appFile(&(info.ref), O_RDONLY);
		BAppFileInfo appFileInfo(&appFile);
		
		iconLabel = new BBitmap(bounds, B_COLOR_8_BIT);
		
		if (appFileInfo.GetIcon(iconLabel, B_MINI_ICON) != B_NO_ERROR) {
			delete iconLabel;
			iconLabel = NULL;
		}
	}
}

IconMenu::~IconMenu()
{
	delete iconLabel;
	iconLabel = NULL;
}

void IconMenu::GetContentSize(float* width, float* height)
{
	if (iconLabel) {
		*width = bounds.Width();
		*height = bounds.Height();
	}
	else
		BMenuItem::GetContentSize(width, height);
}

void IconMenu::DrawContent()
{
	if (iconLabel) {
		Menu()->SetDrawingMode(B_OP_OVER);
		
		float width, height;
		
		Menu()->GetPreferredSize(&width, &height);
		
		BRect destBounds = bounds;
		destBounds.OffsetBy(8.0, ((height - bounds.Height()) * 0.5) - 1);
		
		// Scaling the icon is left as an exercise for the reader :)
		Menu()->DrawBitmap(iconLabel, bounds, destBounds);
	}
	else
		BMenuItem::DrawContent();
}


