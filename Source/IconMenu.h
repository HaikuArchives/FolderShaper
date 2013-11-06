//------------------------------------------------------------------------------
// IconMenu.h
//------------------------------------------------------------------------------
// A menu item implementation that displays an icon as its label.
// 
// IconMenu implementation Copyright (C) 1998 Tyler Riti <fizzboy@mail.utexas.edu>
// Based on code Copyright (C) 1997 Jens Kilian
// This code is free to use in any way so long as the credits above remain intact.
// This code carries no warranties or guarantees of any kind. Use at your own risk.
//------------------------------------------------------------------------------

#ifndef _ICONMENU_H
#define _ICONMENU_H

#include <MenuItem.h>

class IconMenu : public BMenuItem
{
	public:
		// Constructor
		// Description:	Creates a menu item with the provided icon as its
		//				label and menu as its content.
		// Requires:		Both icon and menu must be valid.
		// Result:		If icon is valid, then the icon it points to will
		//				be used as the label for the menu. If icon is NULL
		//				then the menu's name will be used instead. If icon
		//				is invalid, a crash occurs. If menu is invalid or
		//				NULL, a crash occurs.
		// Notes:			Client code is still responsible for deleting the
		//				data icon points to. IconMenu only copies the data,
		//				it does not take posession of it.
		IconMenu(BBitmap* icon, BMenu* menu);
		
		// Constructor
		// Description:	Creates a menu item with the application's mini
		//				icon as its label and menu as its content.
		// Requires:		A valid BMenu pointer. The application should also
		//				have a mini icon.
		// Result:		If the application's mini icon is found then the
		//				menu will be created with the icon as the label.
		//				Otherwise, the name of menu will be used as the
		//				label instead. If menu is invalid or NULL, a
		//				crash will occur.
		IconMenu(BMenu* menu);
		
		// Destructor
		virtual ~IconMenu();
		
	protected:
		// Overridden Hook Functions
		virtual void GetContentSize(float* width, float* height);
		virtual void DrawContent();
		
	private:
		// Disabled Copy Constructor
		IconMenu(const IconMenu& iconMenu);
		
		// Disabled Assignment Operator
		IconMenu& operator=(const IconMenu& iconMenu);
		
		BRect	bounds;
		BBitmap* iconLabel;
};

#endif
