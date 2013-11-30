// FolderShaper
// Jonas Sundström, jonas@kirilla.com

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <fs_attr.h>

#include <Alert.h>
#include <Application.h>
#include <Beep.h>
#include <Directory.h>
#include <Entry.h>
#include <FindDirectory.h>
#include <MenuItem.h>
#include <Messenger.h>
#include <Node.h>
#include <Path.h>
#include <PopUpMenu.h>
#include <Roster.h>
#include <String.h>
#include <SupportDefs.h>
#include <TrackerAddOn.h>
#include <Volume.h>
#include <VolumeRoster.h>

#include "FSApp.h"
#include "FSDefs.h"
#include "KMenuItem.h"

#define DEBUG 1
#include <Debug.h>

extern "C" void 
process_refs(entry_ref dir_ref, BMessage * msg, void *)
{
	msg->PrintToStream();

	msg->AddRef("dir_ref", & dir_ref); 
	be_roster->Launch (FS_APP_SIGNATURE, msg);
}

int main()
{
	new FolderShaper();
	be_app	-> Run();
	delete	be_app;
	return (0);
}

FolderShaper::FolderShaper	(void)
:	BApplication	(FS_APP_SIGNATURE)
{
//	default values
	m_settings_file 	=	new BFile();
	m_tmpl_dir			=	new BDirectory();
	m_fs_window			=	NULL;
	m_got_refs			=	false;
	m_stay_open 		=	false;
	m_do_quit			=	false;
	m_do_move			=	true;
	m_do_open			=	FS_IF_ORIGINAL_OPEN;
	m_do_clobber		=	false;
	m_do_keep_position	=	true;
	m_do_clean_up		=	false;
	m_winloc.Set(200,200);
	m_messenger = new BMessenger();
	 
	status_t  status;
	
	if ((status = CreateTemplatesFolder())	!= B_OK)	// find templates dir
	{
		beep();
		ErrorMessage("CreateTemplatesFolder()", status);
		SeriousError();
	}

	if ((status = CreateSettingsFile()) != B_OK)	// find settings file
	{
		beep();
		ErrorMessage("CreateSettingsFile()", status);
		SeriousError();
	}
	
	ReadSettings();								// read settings
	
	if ((m_tmpl_dir->	CountEntries()) < 1)	// no templates ?
	{
		OpenTemplatesFolder();
		snooze(500000);
		AddTemplatesMessage();
	}
	
	// create window
	m_fs_window = new FolderShaperWindow(m_winloc, m_do_move, m_do_open, 	
		m_do_clobber, m_do_keep_position, m_do_clean_up);
										 
}

FolderShaper::~FolderShaper	(void)
{
}

void
FolderShaper::ReadyToRun	(void)
{
	if (m_do_quit == true)
	{
		Quit();
	}
	else
	{
		if (m_got_refs == false || m_stay_open == true)
		{
			// m_fs_window = new FolderShaperWindow(m_winloc, m_do_move);
			m_fs_window->	Show();
		}
		else Quit();
	}
}

void 
FolderShaper::MessageReceived	(BMessage * a_message)
{
	switch(a_message->what)
	{
		case 'sett':									// Save settings
				SaveSettings(a_message);
			break;
		case 'drpt':									// Template drop
				MoveCopyTemplates	(a_message);
				break;	
		case 'drpf':									// Folder drop
				ProcessRefs(a_message);
				break;	
		case 'tmpl':									// OpenTemplatesFolder button
				OpenTemplatesFolder();
			break;
		case 'help':									// open Documentation
				Help();
				break;
		default:
				BApplication::MessageReceived(a_message);
			break;			
	}
}

void 
FolderShaper::RefsReceived	(BMessage * a_message)
{
	if (m_do_quit)
		return;
	
	m_got_refs = true;
	
	ProcessRefs(a_message);
}

bool
FolderShaper::QuitRequested	(void)
{
	return BApplication::QuitRequested();
}

void
FolderShaper::AboutRequested	(void)
{
	BAlert * alert = new BAlert("About FolderShaper", 
			"This is FolderShaper " FS_APP_VERSION "\n\n"
			B_UTF8_COPYRIGHT " Jonas Sundström, jonas@kirilla.com\n\n"
			"What do you think?\n",
			"Open website", "Awful!", "Awesome!", 
			B_WIDTH_FROM_WIDEST, B_OFFSET_SPACING, B_IDEA_ALERT);

	int32 reply = alert->Go();
	
	if (reply == 0)
	{
		BMessage msg(B_ARGV_RECEIVED); 
		msg.AddString("argv", "app"); 
		msg.AddString("argv", "http://www.kirilla.com/");  

		msg.AddInt32("argc", 2); 
		be_roster->Launch ("application/x-vnd.Be.URL.http", &msg ); 	
	}
	else if (reply == 1)
	{
		beep();
	}	
}

void
FolderShaper::ErrorMessage	(const char * a_text, const int32 a_status) const
{
	PRINT(("%s: %s\n", a_text, strerror(a_status)));
}

void
FolderShaper::NameSafe	(BString * a_string)
{
	int32	position = 0;
	while(position < a_string->	Length())
	{
		if      (a_string-> ByteAt (position) == '\"') { a_string-> Insert ("\\",position); position++; }
		else if (a_string-> ByteAt (position) == '$')  { a_string-> Insert ("\\",position); position++; }
		else if (a_string-> ByteAt (position) == '`')  { a_string-> Insert ("\\",position); position++; }
		position++;
	}

	a_string->Prepend("\"");
	a_string->Append("\"");
}

status_t
FolderShaper::CreateSettingsFile	()
{
	status_t status;
	BPath p;
	if ((status = find_directory(B_USER_SETTINGS_DIRECTORY, &p)) == B_OK)
		if ((status = p.SetTo(p.Path(),"Kirilla/FolderShaper/settings")) == B_OK)
		{
			status =	m_settings_file->	SetTo(p.Path(),	B_READ_WRITE | B_CREATE_FILE);
						
			if (status == B_OK || status == B_FILE_EXISTS)
				return B_OK;
			else
				return status;
		}
		else
			return status;
	else
		return status;
}

status_t
FolderShaper::CreateTemplatesFolder	()
{
	status_t status;
	BPath p;
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &p) != B_OK)	return B_ERROR;
	
	BDirectory temp_dir;
	
	if ((status = m_tmpl_dir->	SetTo(p.Path())) != B_OK) 	return status;
	status	=	m_tmpl_dir->	CreateDirectory("Kirilla", &temp_dir);
	if (status != B_OK && status != B_FILE_EXISTS)	return status;

	p.SetTo(p.Path(),"Kirilla");
	if ((status = m_tmpl_dir->	SetTo(p.Path())) != B_OK)	return status;
	status	=	m_tmpl_dir->CreateDirectory("FolderShaper", &temp_dir);
	if (status != B_OK && status != B_FILE_EXISTS)	return status;

	p.SetTo(p.Path(),"FolderShaper");
	if ((status = m_tmpl_dir->	SetTo(p.Path())) != B_OK)	return status;
	status	=	m_tmpl_dir->CreateDirectory("Templates", &temp_dir);
	if (status != B_OK && status != B_FILE_EXISTS)	return status;
	
	p.SetTo(p.Path(),"Templates");
	if ((status = m_tmpl_dir->	SetTo(p.Path())) != B_OK)	return status;
		
	BEntry entry(p.Path());
	if (entry.Exists()	&&	entry.IsDirectory())	return B_OK;
	else											return B_ERROR;
}

void
FolderShaper::OpenTemplatesFolder	()
{
	BPath p;
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &p) != B_OK);
	if (p.SetTo(p.Path(),"Kirilla/FolderShaper/Templates") != B_OK);

	BString command(p.Path());
	NameSafe(& command);
	command.Prepend("/boot/system/Tracker ");
	system(command.String());
}

void
FolderShaper::AddTemplatesMessage	()
{
	BPath p;
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &p) != B_OK);
	if (p.SetTo(p.Path(),"Kirilla/FolderShaper/Templates") != B_OK);

	BString text("Put your template folders in this folder (");
	text.Append(p.Path());
	text.Append("), then try again.");

	BAlert *box = new BAlert("FolderShaper", text.String(),
							 "I will", NULL, NULL, B_WIDTH_AS_USUAL, B_IDEA_ALERT);
	box->Go();
}

void
FolderShaper::ProcessRefs	(BMessage *msg)
{
	status_t status = B_OK;

//	filter out bad refs

	entry_ref	ref;
	int32		ref_count	=	0;

	while(msg->FindRef("refs", ref_count, &ref) == B_OK)
	{
		BEntry entry(& ref, true);
		entry.GetRef(& ref);	// if Tracker is ever changed to -not- resolve symlinks
		
		if (entry.IsDirectory() == false)
		{
			BString text = "'";
			text.Append(ref.name);
			text.Append("' is not a folder.\nFolderShaper can only handle folders.");
			(new BAlert("FolderShaper Error", text.String(),
							 "I see", NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT))->Go();
			return;
		}
		
		if (IsFolderUntouchable('drpf', & ref))
			return;	
		
/*		if (IsFolderOpen(& ref))
		{
			(new BAlert("FolderShaper Error", "Please close all windows you want to shape.",
			 "Ok", NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT))->Go();
			
			PostMessage(B_QUIT_REQUESTED);
			return;
		}
*/		
		ref_count++;	
	}
	
	msg->FindRef("dir_ref", &ref);
	
	if (ref_count == 0 && IsFolderUntouchable('drpf', & ref))
		return;
	
// find templates  &&  build popup menu
	
	BPopUpMenu*	aPopUp	=	new BPopUpMenu("templates");
	BMenuItem*	aItem	=	NULL;
	aItem = new BMenuItem("Shape as", NULL);
	aItem->		SetEnabled(false);
	aPopUp->	AddItem(aItem);	

	int32		template_count	=	0;
	BEntry		template_entry;
	
	m_tmpl_dir->	Rewind();
	while(m_tmpl_dir->	GetNextEntry(&template_entry) == B_OK)
	{
		if ((status = template_entry.GetRef(&ref)) != B_OK)
		{	ErrorMessage("templates_entry.GetRef() ", status);	return;	}
	
		if ((status = template_entry.SetTo(&ref, true)) != B_OK)
		{	ErrorMessage("templates_entry.SetTo() ", status);	return;	}
	
		if (template_entry.IsDirectory())
		{
			template_count++;
			
			BNode	template_node (& template_entry);
			aItem = new KMenuItem(ref.name, NULL, 0, 0, & template_node);
						// char a_shortcut, uint32 a_modifiers, BNode * a_node_ptr)

			aPopUp->AddItem(aItem);
		}
		else PRINT(("entry %s is not a directory!\n", ref.name));
	}
	
	if (template_count < 1)
	{
		OpenTemplatesFolder();
		sleep(1);
		(new BAlert("FolderShaper Error", "You need some templates first.\n"
									"Come back when you've created them.",
							 "I will", NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT))->Go();
		return;
	}

	aPopUp->AddSeparatorItem();
	if (ref_count >1)
	{
		aItem = new BMenuItem("Add as Templates" B_UTF8_ELLIPSIS, NULL);
		aPopUp->AddItem(aItem);
	}
	else
	{
		aItem = new BMenuItem("Add as Template" B_UTF8_ELLIPSIS, NULL);
		aPopUp->AddItem(aItem);
	}
	
	aItem = new BMenuItem("Settings" B_UTF8_ELLIPSIS, NULL);	aPopUp->AddItem(aItem);	
	aItem = new BMenuItem("Templates" B_UTF8_ELLIPSIS, NULL);	aPopUp->AddItem(aItem);	

//	mouse location
	BPoint 	point;
	m_fs_window->	Lock();
	point 	=	m_fs_window->GetMouse();
	m_fs_window->	Unlock();
	
//	Popup
	BMenuItem * chosen = NULL;
	BString	label;

	chosen	=	aPopUp->Go(point, false, true);

//	handle popup
	if (chosen)
		label = chosen->Label();
	else 	{	delete aPopUp;		return;	}
	delete aPopUp;

	if ((label == "Add as Templates" B_UTF8_ELLIPSIS)
	||	(label == "Add as Template" B_UTF8_ELLIPSIS))
	{
		MoveCopyTemplates(msg);
		return;
	}

	if (label == "Settings" B_UTF8_ELLIPSIS)
	{
		m_stay_open	= true;
		return;
	}

	if (label == "Templates" B_UTF8_ELLIPSIS)
	{
		OpenTemplatesFolder();
		return;
	}
	
	ref_count	=	0;

	// 
	// Tracker add-on, one or more entry_refs
	// 
	while(msg->FindRef("refs", ref_count, &ref) == B_OK)
	{
		if ((status = template_entry.SetTo(&ref, true)) != B_OK)
		{
			ErrorMessage("!!!!!!!!!! SetTo() ", status);
			return;
		}
		
		if ((status = template_entry.GetRef(&ref)) != B_OK)
		{
			ErrorMessage("!!!!!!!!!! GetRef() ", status);
			return;
		}
	
		PRINT(("ShapeRef(%s, %s)\n", ref.name, label.String()));	// remove
	
		ShapeRef(& ref, label.String(), false);
		
		ref_count++;	
	}
	
	//
	// Tracker add-on, folder selected, no entry_refs
	//
	if (ref_count <= 0)		
	{
		if ((status = msg->FindMessenger("TrackerViewToken", m_messenger)) != B_OK)
		{															// Getting messenger to Tracker folder window
			ErrorMessage("msg->FindMessenger() ", status);			// need it later to close original window
		}
		
		if (msg->FindRef("dir_ref", &ref) == B_OK)
		{
			ShapeRef(& ref, label.String(), true);
		}
	}
}

void
FolderShaper::SeriousError	(void)
{
	BAlert * alert = new BAlert("FolderShaper Error", 
			"FolderShaper bumped into some unforseen\n"
			"circumstances that it's not fit to handle.\n\n"
			"Fire and brimstone will fall from the sky,\n"
			"and then, FolderShaper will die.\n\n",
			"Goddammit!", NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);

	alert->Go();
}

void
FolderShaper::SaveSettings(BMessage * a_message)
{
	BPoint loc;
	if (a_message->	FindPoint("winloc",&loc) == B_OK)	m_winloc = loc;
	
	bool a;
	int8 b;
	if (a_message->	FindBool("do_move", &a) == B_OK)			m_do_move = a;
	if (a_message->	FindInt8("do_open", &b) == B_OK)			m_do_open = b;
	if (a_message->	FindBool("do_clobber", &a) == B_OK)			m_do_clobber = a;
	if (a_message->	FindBool("do_keep_position", &a) == B_OK)	m_do_keep_position = a;
	if (a_message->	FindBool("do_clean_up", &a) == B_OK)		m_do_clean_up = a;
	
	status_t	status;
	
	m_settings_file->	Seek(0, SEEK_SET);
	
	status	=	a_message->	Flatten(m_settings_file);
	if (status != B_OK)
	{
		ErrorMessage("Flatten()", status);
		SeriousError();
	}
}

void
FolderShaper::ReadSettings()
{
	status_t	status;
	BMessage	settings;
	status	=	settings.Unflatten(m_settings_file);
	if (status != B_OK)
		ErrorMessage("Unflatten()", status);
	
	BPoint loc;
	if (settings.FindPoint("winloc",&loc) == B_OK)	m_winloc = loc;
	
	bool a;
	int8 b;
	if (settings.FindBool("do_move", &a) == B_OK)			m_do_move = a;
	if (settings.FindInt8("do_open", &b) == B_OK)			m_do_open = b;
	if (settings.FindBool("do_clobber", &a) == B_OK)		m_do_clobber = a;
	if (settings.FindBool("do_keep_position", &a) == B_OK)	m_do_keep_position = a;
	if (settings.FindBool("do_clean_up", &a) == B_OK)		m_do_clean_up = a;
}

bool 
FolderShaper::IsFolderOpen(const entry_ref * ref)	const
{
	status_t	status = B_OK;
	int32		index = 0;
	
	BMessenger	messenger(FS_TRACKER_SIGNATURE);
	BMessage	message(B_GET_PROPERTY);
	BMessage	reply;
	entry_ref 	window;

	
	if (! messenger.IsValid())
	{	
		PRINT(("Error: invalid BMessenger.\n"));
		return false; 
	}
	
	for(index = 1; index < 500; index++)
	{
		message.MakeEmpty();
		message.what = B_GET_PROPERTY;
		message.AddSpecifier("Path");
		message.AddSpecifier("Poses");
		message.AddSpecifier("Window", index);
		
		reply.MakeEmpty();
		
		if((status = messenger.SendMessage(&message, &reply)) != B_OK)
		{
			ErrorMessage("Error sending message to Tracker: ", status);
			return false;
		}
	
		if((status = reply.FindRef("result", &window)) != B_OK)
			return false;
		
		if(*ref == window)
			return true;
	}
	
	return false;
}

bool 
FolderShaper::IsFolderUntouchable(uint32 purpose, const entry_ref * a_ref)
{
	// system directories
	if	(IsItThisFolder(a_ref, B_DESKTOP_DIRECTORY))				return SysDirMessage(a_ref);
	if	(IsItThisFolder(a_ref, B_TRASH_DIRECTORY))					return SysDirMessage(a_ref);
	if	(IsItThisFolder(a_ref, B_APPS_DIRECTORY))					return SysDirMessage(a_ref);
	if	(IsItThisFolder(a_ref, B_PREFERENCES_DIRECTORY))			return SysDirMessage(a_ref);
	if	(IsItThisFolder(a_ref, B_BEOS_DIRECTORY))					return SysDirMessage(a_ref);
	if	(IsItThisFolder(a_ref, B_BEOS_SYSTEM_DIRECTORY))			return SysDirMessage(a_ref);
	if	(IsItThisFolder(a_ref, B_BEOS_ADDONS_DIRECTORY))			return SysDirMessage(a_ref);
	if	(IsItThisFolder(a_ref, B_BEOS_BOOT_DIRECTORY))				return SysDirMessage(a_ref);
	if	(IsItThisFolder(a_ref, B_BEOS_FONTS_DIRECTORY))				return SysDirMessage(a_ref);
	if	(IsItThisFolder(a_ref, B_BEOS_LIB_DIRECTORY))				return SysDirMessage(a_ref);
	if	(IsItThisFolder(a_ref, B_BEOS_SERVERS_DIRECTORY))			return SysDirMessage(a_ref);
	if	(IsItThisFolder(a_ref, B_BEOS_APPS_DIRECTORY))				return SysDirMessage(a_ref);
	if	(IsItThisFolder(a_ref, B_BEOS_BIN_DIRECTORY))				return SysDirMessage(a_ref);
	if	(IsItThisFolder(a_ref, B_BEOS_ETC_DIRECTORY))				return SysDirMessage(a_ref);
	if	(IsItThisFolder(a_ref, B_BEOS_DOCUMENTATION_DIRECTORY))		return SysDirMessage(a_ref);
	if	(IsItThisFolder(a_ref, B_BEOS_PREFERENCES_DIRECTORY))		return SysDirMessage(a_ref);
	if	(IsItThisFolder(a_ref, B_SYSTEM_DIRECTORY))					return SysDirMessage(a_ref);
	if	(IsItThisFolder(a_ref, B_SYSTEM_ADDONS_DIRECTORY))			return SysDirMessage(a_ref);
	if	(IsItThisFolder(a_ref, B_SYSTEM_BOOT_DIRECTORY))			return SysDirMessage(a_ref);
	if	(IsItThisFolder(a_ref, B_SYSTEM_FONTS_DIRECTORY))			return SysDirMessage(a_ref);
	if	(IsItThisFolder(a_ref, B_SYSTEM_LIB_DIRECTORY))				return SysDirMessage(a_ref);
	if	(IsItThisFolder(a_ref, B_SYSTEM_SERVERS_DIRECTORY))			return SysDirMessage(a_ref);
	if	(IsItThisFolder(a_ref, B_SYSTEM_BIN_DIRECTORY))				return SysDirMessage(a_ref);
	if	(IsItThisFolder(a_ref, B_SYSTEM_ETC_DIRECTORY))				return SysDirMessage(a_ref);
	if	(IsItThisFolder(a_ref, B_SYSTEM_DOCUMENTATION_DIRECTORY))	return SysDirMessage(a_ref);
	if	(IsItThisFolder(a_ref, B_SYSTEM_SETTINGS_DIRECTORY))		return SysDirMessage(a_ref);
	if	(IsItThisFolder(a_ref, B_SYSTEM_DEVELOP_DIRECTORY))			return SysDirMessage(a_ref);
	if	(IsItThisFolder(a_ref, B_SYSTEM_LOG_DIRECTORY))				return SysDirMessage(a_ref);
	if	(IsItThisFolder(a_ref, B_SYSTEM_SPOOL_DIRECTORY))			return SysDirMessage(a_ref);
	if	(IsItThisFolder(a_ref, B_SYSTEM_TEMP_DIRECTORY))			return SysDirMessage(a_ref);
	if	(IsItThisFolder(a_ref, B_SYSTEM_VAR_DIRECTORY))				return SysDirMessage(a_ref);
	if	(IsItThisFolder(a_ref, B_USER_DIRECTORY))					return SysDirMessage(a_ref);
	if	(IsItThisFolder(a_ref, B_USER_CONFIG_DIRECTORY))			return SysDirMessage(a_ref);
	if	(IsItThisFolder(a_ref, B_USER_ADDONS_DIRECTORY))			return SysDirMessage(a_ref);
	if	(IsItThisFolder(a_ref, B_USER_BOOT_DIRECTORY))				return SysDirMessage(a_ref);
	if	(IsItThisFolder(a_ref, B_USER_FONTS_DIRECTORY))				return SysDirMessage(a_ref);
	if	(IsItThisFolder(a_ref, B_USER_LIB_DIRECTORY))				return SysDirMessage(a_ref);
	if	(IsItThisFolder(a_ref, B_USER_SETTINGS_DIRECTORY))			return SysDirMessage(a_ref);
	if	(IsItThisFolder(a_ref, B_USER_DESKBAR_DIRECTORY))			return SysDirMessage(a_ref);
	
	// volume roots
	BVolumeRoster	vol_roster;
	BVolume			volume;
	BDirectory		vol_dir;
	BEntry			vol_entry;
	entry_ref		vol_ref;
	
	
	while (vol_roster.GetNextVolume(& volume) == B_OK)
	{
		volume.GetRootDirectory(& vol_dir);
		
		vol_dir.GetEntry(& vol_entry);
		
		vol_entry.GetRef(& vol_ref);
		
		if (*a_ref == vol_ref)
			return VolRootMessage(a_ref);
	}
	
	// target does not support attributes
	if (volume.SetTo(a_ref->device) != B_OK)
	{
		// message
		return true;
	}

	if(! volume.KnowsAttr())
	{
		char volname[B_FILE_NAME_LENGTH];
		volume.GetName(volname);
	
		return NoAttributesMessage(purpose, a_ref, volname);
	}

	// and perhaps returns something for the BAlert
	
	return false;
}

bool 
FolderShaper::IsItThisFolder(const entry_ref * a_ref, directory_which which)
{
	status_t	status;
	BPath		path;
	entry_ref	system_ref;
	
	status	=	find_directory (which, & path);
	BEntry		entry(path.Path());
	entry.GetRef(& system_ref);

	if (status != B_OK)
	{
		ErrorMessage("Error: Could not find system dir: ", status);
		SeriousError();
		exit (-1);
	}
		
	if (*a_ref == system_ref)
		return true;
	else
		return false;
}

status_t 
FolderShaper::ShapeRef	(entry_ref * a_ref, const char * a_template, bool a_do_open)
{
	PRINT(("ShapeRef(%s, %s)\n", a_ref->name, a_template));

	status_t	status;

// store original_name
	
	BEntry	original_entry	(a_ref, true);
	original_entry.	GetRef	(a_ref);
	BString original_name 	= a_ref->name;
	BPath	original_path	(& original_entry);
	
	BString original_path_string = original_path.Path();
	NameSafe(& original_path_string);

	BPath	original_parent_path;
	original_path.GetParent(& original_parent_path);

	BDirectory parent_dir(original_parent_path.Path());

//	temp name
	BString new_name	=	a_ref->name;
	new_name.Prepend("New ");

	int32	counter	=	2;
	while(parent_dir.Contains(new_name.String()))
	{
		new_name = a_ref->name;
		new_name.Append(" (temporary ");
		new_name << counter;
		new_name.Append(")");		
		counter	++;
	}

	PRINT(("original_name: %s\n", original_name.String()));
	PRINT(("new_name: %s\n", new_name.String()));	
	
	PRINT(("original_path: %s\n", original_path.Path()));	
	PRINT(("original_path_string: %s\n", original_path_string.String()));	
	PRINT(("original_parent_path: %s\n", original_parent_path.Path()));	

	
	BPath	template_path	(m_tmpl_dir, a_template);

	BEntry	tmp_entry;		// traverse link to template
	if ((status = tmp_entry.SetTo(template_path.Path(), true)) != B_OK)
	{
		// message
		return status;
	}
	template_path.SetTo(& tmp_entry);
	
	BString	template_path_string 	=	template_path.Path();
	NameSafe(& template_path_string);
	PRINT(("template_path_string: %s\n", template_path_string.String()));		

	BPath	new_path	(original_parent_path.Path(), new_name.String());
	BString	new_path_string	=	new_path.Path();
	NameSafe(& new_path_string);
	PRINT(("new_path_string: %s\n", new_path_string.String()));			

	BString command;

	if(m_do_keep_position)
	{
		//	copy X/Y position attribute from Original Folder

		command = "copyattr -n _trk/pinfo_le ";
		command.Append(original_path_string);
		command.Append(" ");
		command.Append(template_path_string);
		
		PRINT(("copy X/Y position: \n%s\n", command.String()));
	
		system(command.String());
	}
	else
	{
		BNode template_node(& tmp_entry);
		template_node.RemoveAttr("_trk/pinfo_le");
	}
		
	
//	copy template -> "New name" (temp)

	command = "copyattr -d -r ";
	command.Append(template_path_string);
	command.Append(" ");
	command.Append(new_path_string);
	
	PRINT(("copy template -> temp: \n%s\n", command.String()));
	
	system(command.String());
	
//	New folder exists?

	BDirectory	new_directory;
	if((status = new_directory.SetTo(new_path.Path())) != B_OK)
	{
		ErrorMessage("new_directory.SetTo() ", status);	// ... probably want a BAlert
		return status;
	}
	
	BDirectory original_dir;
	if((status = original_dir.SetTo(original_path.Path())) != B_OK)
	{
		ErrorMessage("original_dir.SetTo() ", status);	// ... probably want a BAlert
		return status;
	}
	
//	Move stuff from original folder to new folder	
	BEntry	original_content_entry;
	while(original_dir.GetNextEntry(&original_content_entry) == B_OK)
	{
		if ((status = original_content_entry
			.MoveTo(& new_directory, NULL, ! m_do_clobber))	!= B_OK 
			&& (status != B_FILE_EXISTS || (! m_do_clobber) == true))
		{	
			ErrorMessage("original_content_entry.MoveTo() ", status);
			return status;							// ... definitely needs a fat BAlert
		}
	}

//	Move original folder to Trash
//	alt.
//  Delete

	bool was_open = false;
	
	if (m_do_move)
	{
		//	Is Folder Open ?
		was_open =	IsFolderOpen(a_ref);
		
		//	Close Original Folder	
		status	=	MoveRefToTrash(a_ref);	// This calls Tracker to move the ref.
											// If the folder is open, Tracker will close it.

		for (int j = 0; j < 250; j++)
		{
			PRINT(("loop: %d\n", j));
		
			if (original_entry.Exists() == false)
				break;
			
			snooze(20000);		// give Tracker some Time to Move
			// .... consider using node-monitoring instead to 
			// catch when Tracker has moved the folder to Trash
		}
	}
	else
	{
		status = original_entry.Remove();
		if (status != B_OK)
		{
			ErrorMessage("original_content_entry.MoveTo() ", status);
			// BAlert.. Could not remove original folder
			// return status;
		}
	}	
	
//	Rename New Folder -> Original Name
	BEntry	new_entry(new_path.Path());
	status = new_entry.Rename(original_name.String());
	if (status != B_OK)
		ErrorMessage("new_entry.Rename() ", status);
	
	entry_ref	new_ref;
	status = new_entry.GetRef(& new_ref);
	if (status != B_OK)
		ErrorMessage("new_entry.GetRef() ", status);
	
//	Open New Folder
	if (was_open) PRINT(("was_open == true\n"));
	if (! was_open) PRINT(("was_open == false\n"));
		
	if (((m_do_open == FS_IF_ORIGINAL_OPEN) && was_open) || m_do_open == FS_ALWAYS_OPEN)
	{
		// open folder
		command =	original_path_string;
		command.Prepend("/boot/system/Tracker ");
		command.Append(" &");
		
		system(command.String());
		
		// wait for new folder to open
		WaitForOpeningFolder(& new_ref);
		
		//	Clean Up All	
		if (m_do_clean_up)
			CleanUpWindow(& new_ref);
	}
	
	return B_OK;
}

status_t
FolderShaper::MoveRefToTrash	(const entry_ref * a_ref)
{
	BMessenger trak_messgr(FS_TRACKER_SIGNATURE);

	if (trak_messgr.IsValid())
	{
		BMessage msg(B_DELETE_PROPERTY);

		BMessage specifier('sref');
		specifier.AddRef("refs", a_ref);
		specifier.AddString("property", "Entry");
		msg.AddSpecifier(& specifier);

		msg.AddSpecifier("Poses");
		msg.AddSpecifier("Window", 1);

		BMessage reply;
		status_t status;
		status = trak_messgr.SendMessage(&msg, &reply);
		
		return status;
	}
	else
		PRINT(("Tracker BMessenger not valid.\n"));
		
	return B_ERROR;
}

void
FolderShaper::MoveCopyTemplates	(BMessage *	a_msg)
{
	PRINT(("MoveCopyTemplates()\n"));
	
//	check for non-Folders in refs-message

	entry_ref	ref;
	int32		ref_count	=	0;

	while(a_msg->FindRef("refs", ref_count, &ref) == B_OK)
	{
		PRINT(("ref.name: %s\n", ref.name));	// remove
		
		BEntry entry(& ref, true);
		entry.GetRef(& ref);	// if Tracker is ever changed to -not- resolve link
		
		if (entry.IsDirectory() == false)
		{
			BString text = "'";
			text.Append(ref.name);
			text.Append("' is not a folder.\nFolderShaper can only use folders as templates.");
			(new BAlert("FolderShaper Error", text.String(),
							 "I see", NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT))->Go();
			return;
		}
		
		if (IsFolderUntouchable('drpt', & ref))
			return;	
		
		ref_count++;	
	}
	
	a_msg->FindRef("dir_ref", &ref);
	
	if (ref_count == 0 && IsFolderUntouchable('drpt', & ref))
		return;

//	Move or Copy ?	
	BAlert * alert = new BAlert("FolderShaper", "Would you like to Move or Copy\nto the Templates folder?",
							 "Cancel", "Move", "Copy", B_WIDTH_AS_USUAL, B_WARNING_ALERT);
	
	int choice = alert->Go();

//	Handle refs

	ref_count = 0;
		
	while(a_msg->FindRef("refs", ref_count, &ref) == B_OK)
	{
		if (choice == 1)	// Move
			MoveCopyTemplate	(& ref, true);
			
		if (choice == 2)	// Copy
			MoveCopyTemplate	(& ref, false);
		
		ref_count++;	
	}
	
	a_msg->FindRef("dir_ref", &ref);
	
	if (ref_count == 0)
	{
		if (choice == 1)	// Move
			MoveCopyTemplate	(& ref, true);
			
		if (choice == 2)	// Copy
			MoveCopyTemplate	(& ref, false);
	}
}


void
FolderShaper::MoveCopyTemplate	(entry_ref * a_template_ref, bool a_move)
{
	BEntry		template_entry;
	
	template_entry.SetTo	(a_template_ref, true);			// traverse, if link (future proof)
	template_entry.GetRef	(a_template_ref);
	BString template_name =	a_template_ref->name;				// name of dropped template
	BPath	template_path	(& template_entry);
	
	BString template_path_string = template_path.Path();		// path of dropped template
	NameSafe(& template_path_string);

	BPath	templates_folder_path	(m_tmpl_dir, NULL);			// path of template folder
	BString	templates_folder_string	=	templates_folder_path.Path();
	//NameSafe(& templates_folder_string);	// not yet, we want the new name in first

	// create a new name that is unique in the Template folder
	BString new_target_name	=	a_template_ref->name;			

	int32	counter	=	2;
	m_tmpl_dir->Rewind();
	
	while(m_tmpl_dir->Contains(new_target_name.String()))
	{
		new_target_name = a_template_ref->name;
		new_target_name.Append(" ");
		new_target_name << counter;
		counter	++;
	}

	BString	target_template_path_string	=	templates_folder_string;
	target_template_path_string.	Append("/");
	target_template_path_string.	Append(new_target_name);
	NameSafe(& target_template_path_string);
	
	//	 DEBUG
	PRINT(("template_name: %s\n", template_name.String()));
	PRINT(("new_target_name: %s\n", new_target_name.String()));	
	
	PRINT(("template_path_string: %s\n", template_path_string.String()));	
	PRINT(("templates_folder_string: %s\n", templates_folder_string.String()));	
	PRINT(("target_template_path_string: %s\n", target_template_path_string.String()));	

	if (a_move)	// Move
	{
		// find a temp name that's in neither of the source/target folders
		BString unique_name = a_template_ref->name;	 
		BDirectory	source_dir(templates_folder_string.String());
		
		counter = 2;
		while(	source_dir.Contains(unique_name.String())
				||
				m_tmpl_dir->Contains(unique_name.String())	)
		{
			unique_name = a_template_ref->name;
			unique_name.Append(" ");
			unique_name << counter;
			counter	++;
		}
		
		PRINT(("unique_name: %s\n", unique_name.String()));
		
		template_entry.Rename(unique_name.String());
		template_entry.MoveTo(m_tmpl_dir);
		
	}
	else		// Copy
	{
		BString command = "copyattr -d -r ";
		command.Append(template_path_string);
		command.Append(" ");
		command.Append(target_template_path_string);
		system(command.String());
	}
}

void 
FolderShaper::Help	(void)
{
	// Goto website info on FolderShaper
	
	BMessage msg(B_ARGV_RECEIVED); 
	msg.AddString("argv", "app"); 
	msg.AddString("argv", "http://www.kirilla.com/redirect/help/foldershaper.html");  

	msg.AddInt32("argc", 2); 
	be_roster->Launch ("application/x-vnd.Be.URL.http", &msg ); 	
}

void 
FolderShaper::CleanUpWindow(entry_ref * a_dir_ref)
{
	PRINT(("CleanUpWindow()\n"));
	
	status_t	status;
	int32		count = 0;
	
	BMessenger tracker(FS_TRACKER_SIGNATURE);

	BMessage get_win_msg(B_GET_PROPERTY);
	BMessage reply;
	
	entry_ref windowRef;
	BDirectory dir;
	
	for(count = 1; count < 1000; count++)
	{
		get_win_msg.MakeEmpty();
		get_win_msg.what = B_GET_PROPERTY;
		get_win_msg.AddSpecifier("Path");
		get_win_msg.AddSpecifier("Poses");
		get_win_msg.AddSpecifier("Window", count);
		
		reply.MakeEmpty();
		if((status = tracker.SendMessage(&get_win_msg, &reply)) != B_OK)
		{	ErrorMessage("Error: Invalid Reply From Tracker!\n", status);	return; }
	
		if((status = reply.FindRef("result", &windowRef)) != B_OK)
		{	ErrorMessage("Error -- no windowRef ", status);	}		
		
		if(*a_dir_ref == windowRef)
		{
			//PRINT(("That's the window!\n"));
			dir.SetTo(&windowRef);
			break;
		}
	}
	
// Clean up
	reply.MakeEmpty();
	BMessage editor;
	editor.what = 'Tcla';
	editor.AddSpecifier("Poses");
	editor.AddSpecifier("Window", count);

	if ((status = tracker.SendMessage(&editor, &reply)) != B_OK)
		PRINT(("Error: Invalid Reply From Tracker!\n"));
}

void
FolderShaper::WaitForOpeningFolder(const entry_ref * a_ref)	
{
	PRINT(("WaitForOpeningFolder()\n"));

	status_t	status;
//	int32		count = 0;
		
	BMessenger	tracker_messgr(FS_TRACKER_SIGNATURE);
	
//	snooze loop for thread
	PRINT(("## snooze loop for thread ##\n"));
	
	BString thread_name;
	BString ref_name;

	int32	cookie = 0;
	bool 	got_thread = false;
			
	team_id  my_team_id  =	tracker_messgr.Team();
	if (my_team_id != B_NAME_NOT_FOUND)
	{
		thread_info	info;
		
		for (int j = 0; j < 250; j++)
		{
			PRINT(("**************************************\n"));
			cookie = 0;
			
			while (get_next_thread_info(my_team_id, &cookie, &info) == B_OK)
			{
				thread_name	= 	info.name;
				ref_name	=	a_ref->name;
				ref_name.Prepend("w>");
				
				PRINT(("thread: %s\n", thread_name.String()));
				
				if (thread_name == ref_name)
				{
					got_thread = true;
					PRINT(("got thread %s\n", thread_name.String()));
					break;
				}	
			}
			
			if (got_thread)
				break;
			snooze(20000);
		}
	}
	else
	{
		PRINT(("Error: Could not set up BMessenger to Tracker!\n"));
		snooze(1000000);	// if no contact, snooze 1/10 sec
		return;				// then return
	}

//	snooze loop for window (Tr. scripting)
	PRINT(("## snooze loop for Tracker window ##\n"));
	
	bool got_window = false;
	
	for (int j = 0; j < 250; j++)
	{
		PRINT(("**************************************\n"));
		PRINT(("window snooze (%d)\n", j));
		
		if (IsFolderOpen(a_ref))
		{
			PRINT(("Tracker window is open.\n"));
			got_window = true;
			break;
		}
			
		snooze(20000);
	}

	if (! got_window)
	{
		PRINT(("Error: Tracker window did not open even though we snoozed quite some time!\n"));
		snooze(1000000);	// if no contact, snooze 1/10 sec
		return;				// then return
	}
	
//	snooze loop for contents loading
	PRINT(("## snooze loop for window contents ##\n"));

	BMessage	get_win_msg(B_GET_PROPERTY);
	BMessage	reply;
	
	entry_ref 	windowRef;
	BDirectory	window_dir;

	int32		dir_entry_count		=	0;
	int32		window_poses_count	=	0;
		
	int32	win_count = 0;
	for(win_count = 1; win_count < 1000; win_count++)
	{
		get_win_msg.MakeEmpty();
		get_win_msg.what = B_GET_PROPERTY;
		get_win_msg.AddSpecifier("Path");
		get_win_msg.AddSpecifier("Poses");
		get_win_msg.AddSpecifier("Window", win_count);
		
		reply.MakeEmpty();
		if((status = tracker_messgr.SendMessage(&get_win_msg, &reply)) != B_OK)
		{	ErrorMessage("Error: Invalid Reply From Tracker!\n", status);	return; }
	
		if((status = reply.FindRef("result", &windowRef)) != B_OK)
		{	PRINT(("(%ld) Error -- no windowRef: %s)\n", win_count, strerror(status))); }		
		
		if(*a_ref == windowRef)
		{
			PRINT(("That's the window! (win_count: %ld)\n", win_count));
			window_dir.SetTo(&windowRef);
			break;
		}
	}

// snooze loop for matching BDir entry count and BWindow poses count

	BMessage count_msg	(B_COUNT_PROPERTIES);
	count_msg.AddSpecifier("Entry");
	count_msg.AddSpecifier("Poses");
	count_msg.AddSpecifier("Window", win_count);

	dir_entry_count	=	window_dir.CountEntries();

	bool got_match = false;
	for (int j = 0; j < 200; j++)
	{
		PRINT(("**************************************\n"));
		PRINT(("try match (%d)\n", j));

		reply.MakeEmpty();	

		if ((status = tracker_messgr.SendMessage(&count_msg, &reply)) != B_OK)
		PRINT(("Error: Invalid Reply From Tracker!\n"));

		if (reply.FindInt32("result", & window_poses_count) == B_OK
			&& window_poses_count	==	dir_entry_count)
		{
			PRINT((" woohoo !\n"));
			got_match = true;
			break;
		}
		
		PRINT(("dir_entry_count: %ld\n", dir_entry_count));
		PRINT(("window_poses_count: %ld\n", window_poses_count));
			
		snooze(100000);
	}

	if (! got_match)
	{
		PRINT(("Error: BDir entries and BWindow poses did not match up.\n"));
		snooze(1000000);	// if no match, snooze 1/10 sec
		return;				// then return
	}
}

bool
FolderShaper::SysDirMessage	(const entry_ref * a_ref)
{
	BString	string = "'";
	string.Append(a_ref->name);
	string.Append("' is a vital system folder. FolderShaper is on parole "
				"and wants to stay out of trouble. Ok?"); 
			
	
	(new BAlert("FolderShaper Error", string.String(),
							 " Ok, I won't push you ", NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT))->Go();
	
	return true;	
}

bool
FolderShaper::VolRootMessage	(const entry_ref * a_ref)
{
	BString	string = "'";
	string.Append(a_ref->name);
	string.Append(	"' is a volume, not a pure folder.\n"
					"FolderShaper wimps out."); 
			
	
	(new BAlert("FolderShaper Error", string.String(),
							 " I'll try with a folder ", NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT))->Go();
	
	return true;	
}

bool
FolderShaper::NoAttributesMessage	(uint32 purpose, const entry_ref * a_ref, const char * a_volname)
{
	BString	string;
	BString button;
	
	if (purpose == 'drpf')
	{
		string = "'";
		string.Append(a_ref->name);
		string.Append("' can not be shaped, since it lives on a non-Be volume (");
		string.Append(a_volname);
		string.Append("), that does not support attributes.");
		button = "I see";
	}
	else 
	{
		string.Append("'");		
		string.Append(a_ref->name);
		string.Append("' lives on the non-Be, non-attribute ");
		string.Append(a_volname);				
		string.Append("-volume, and would be rather useless as a template, "
					  "at least in its current form. Totally uncool. :}");
		button = "Right";
	}
	
	(new BAlert("FolderShaper Error", string.String(), button.String(), 
				NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT))->Go();
	
	return true;	
}


