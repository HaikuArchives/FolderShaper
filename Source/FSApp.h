#include <Application.h>
#include <Entry.h>
#include <FindDirectory.h>
#include <Messenger.h>
#include <Window.h>

#include "FSWindow.h"

#ifndef _FSAPP_H
#define _FSAPP_H

class FolderShaper : public BApplication
{
public:
	FolderShaper (void);
	~FolderShaper (void);

	virtual void	ReadyToRun		(void);
	virtual bool	QuitRequested	(void);
	virtual void	MessageReceived	(BMessage	*	a_message);
	virtual void	RefsReceived	(BMessage	*	a_message);
	virtual void	AboutRequested	(void);
	void			Help			(void);

	void			ReadSettings	();		
	void			SaveSettings	(BMessage * a_message);

	void			NameSafe			(BString * a_string);
	void			ErrorMessage		(const char * a_text, const int32 a_status) const;
	void			SeriousError		(void);

	status_t		CreateSettingsFile		(void);
	status_t		CreateTemplatesFolder	(void);
	void			OpenTemplatesFolder		(void);
	void			AddTemplatesMessage		(void);
	
	void			ProcessRefs			(BMessage *	msg);
	void			MoveCopyTemplates	(BMessage *	msg);
	void			MoveCopyTemplate	(entry_ref * a_template_ref, bool a_move);
	
	status_t 		ShapeRef		(entry_ref * a_ref, const char * a_template, bool a_do_open);
	status_t		MoveRefToTrash			(const entry_ref * a_ref);
	void 			CleanUpWindow			(entry_ref * a_dir_ref);
	void			WaitForOpeningFolder	(const entry_ref * a_ref);
	
	bool 			IsFolderOpen		(const entry_ref * a_ref) const;
	bool 			IsFolderUntouchable	(uint32 purpose, const entry_ref * a_ref);
	bool			SysDirMessage		(const entry_ref * a_ref);
	bool			VolRootMessage		(const entry_ref * a_ref);
	bool			NoAttributesMessage	(uint32 purpose, 
										const entry_ref * a_ref, const char * a_volname);
	
	bool			IsItThisFolder		(const entry_ref * a_ref, directory_which which);
	
	
		
private:

	bool					m_got_refs;
	bool					m_stay_open;
	bool					m_do_quit;
	FolderShaperWindow	*	m_fs_window;
	BDirectory			*	m_tmpl_dir;
	
	BPoint 					m_winloc;
	bool					m_do_move;
	int8					m_do_open;
	bool					m_do_clobber;
	bool					m_do_keep_position;
	bool					m_do_clean_up;
	
	BFile				*	m_settings_file;
	
	BMessenger			*	m_messenger;
};

#endif
