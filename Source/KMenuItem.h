#ifndef _KMENUITEM_H
#define _KMENUITEM_H

class KMenuItem: public BMenuItem
{
	public:
		KMenuItem(const char *a_label, BMessage *a_message, char a_shortcut = 0,
						uint32 a_modifiers = 0, const entry_ref *a_ref = NULL);
						
		KMenuItem(const char *a_label, BMessage *a_message, char a_shortcut = 0,
						uint32 a_modifiers = 0, BNode *a_node_ptr = NULL);
						
		KMenuItem(const char *a_label, BMessage *a_message, char a_shortcut = 0,
						uint32 a_modifiers = 0, const BBitmap *a_icon_bitmap = NULL);
						
		KMenuItem(const char *a_label, BMessage *a_message, char a_shortcut = 0,
						uint32 a_modifiers = 0, const char *a_icon_bytes = NULL);

		virtual ~KMenuItem();

	
		BString *	m_string;

	protected:
		BBitmap	*	m_icon_bitmap;
		
		virtual	void DrawContent();
		virtual void Highlight(bool a_isHi);
		void MakeBitmap(const char *a_bytes, size_t a_len);
};

#endif
