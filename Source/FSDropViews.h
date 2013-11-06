#ifndef _FSDROPVIEWS_H
#define _FSDROPVIEWS_H

class FSDropTemplateBox : public BBox
{
public:
					FSDropTemplateBox	();
					~FSDropTemplateBox	();
	virtual void	MessageReceived 	(BMessage * a_message);
};

#endif

#ifndef __FS_DROPFOLDER_BOX_H__
#define __FS_DROPFOLDER_BOX_H__

class FSDropFolderBox : public BBox
{
public:
					FSDropFolderBox	();
					~FSDropFolderBox	();
	virtual void	MessageReceived 	(BMessage * a_message);
};

#endif

