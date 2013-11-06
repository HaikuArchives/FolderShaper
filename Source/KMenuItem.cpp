#include <MenuItem.h>
#include <fs_attr.h>
#include <Bitmap.h>
#include <String.h>
#include <Node.h>
#include <NodeInfo.h>

#include "KMenuItem.h"

KMenuItem::KMenuItem(const char * a_label, BMessage * a_message, char a_shortcut,
										 uint32 a_modifiers, const entry_ref *a_ref)
: BMenuItem(a_label, a_message, a_shortcut, a_modifiers),
	m_string		(NULL),
	m_icon_bitmap	(NULL)
{
	m_string =  new BString;
	m_icon_bitmap = new BBitmap(BRect(0,0,15,15), B_COLOR_8_BIT);
	
	BNodeInfo::GetTrackerIcon(a_ref, m_icon_bitmap, B_MINI_ICON);
	
	return;
}

KMenuItem::KMenuItem(const char * a_label, BMessage * a_message, char a_shortcut,
										 uint32 a_modifiers, BNode * a_node_ptr)
: BMenuItem(a_label, a_message, a_shortcut, a_modifiers),
	m_string		(NULL),
	m_icon_bitmap	(NULL)
{
	m_string =  new BString;
	m_icon_bitmap = new BBitmap(BRect(0,0,15,15), B_COLOR_8_BIT);

	BNodeInfo	node_info(a_node_ptr);
	node_info.GetTrackerIcon(m_icon_bitmap, B_MINI_ICON);
	
	return;
}

KMenuItem::KMenuItem(const char *a_label, BMessage *a_message, char a_shortcut,
										uint32 a_modifiers, const BBitmap * a_icon_bitmap)
: BMenuItem(a_label, a_message, a_shortcut, a_modifiers),
	m_string		(NULL),
	m_icon_bitmap	(NULL)
{
	m_string =  new BString;

	m_icon_bitmap = new BBitmap(a_icon_bitmap->Bounds(), B_COLOR_8_BIT);
	if (!m_icon_bitmap)
		return;
	
	m_icon_bitmap->SetBits(a_icon_bitmap->Bits(),
							a_icon_bitmap->BitsLength(),
							0,
							a_icon_bitmap->ColorSpace());
	return;
}

KMenuItem::KMenuItem(const char *a_label, BMessage *a_message, char a_shortcut,
						uint32 a_modifiers, const char * a_icon_bytes)
: BMenuItem(a_label, a_message, a_shortcut, a_modifiers),
	m_string		(NULL),
	m_icon_bitmap	(NULL)
{
	m_string =  new BString;
	
	if (a_icon_bytes)
		MakeBitmap(a_icon_bytes, 256);
	
	return;
}

KMenuItem::~KMenuItem()
{
	delete m_icon_bitmap;

	return;
}

void KMenuItem::MakeBitmap(const char * a_bytes, size_t a_len)
{
	m_icon_bitmap = new BBitmap(BRect(0,0,15,15), B_COLOR_8_BIT);
	if (! m_icon_bitmap)
		return;
		
	m_icon_bitmap->SetBits(a_bytes, 256, 0, B_COLOR_8_BIT);
	
	return;
}

void KMenuItem::Highlight(bool a_isHi)
{
	BMenuItem::Highlight(a_isHi);
	
	// Antialiased text looks bad when highlighted.
	// Don't know what to do about it.
	//DrawContent();

	return;
}

void KMenuItem::DrawContent()
{
	Menu()->SetDrawingMode(B_OP_OVER);
	
	BPoint point = ContentLocation();
	
	point.y -= 1;
	point.x -= 7;
	
	if (m_icon_bitmap)
		Menu()->DrawBitmap(m_icon_bitmap, point);
	
	BPoint str_loc;
	
	//str_loc.x = point.x + 20;
	str_loc.x = point.x + 21;
	
	float width;
	float height;
	
	GetContentSize(&width, &height);
	
	str_loc.y = point.y + height - 3;
			
	Menu()->DrawString(Label(), str_loc);
	
	return;
}

