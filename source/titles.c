#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <3ds.h>

#include "titles.h"
#include "error.h"

extern int debugValues[100];

void titlesInit()
{
	amInit();
}

void titlesExit()
{
	amExit();
}

void initTitleInfo(titleInfo_s* ti, u8 mediatype, u64 title_id)
{
	if(!ti)return;

	ti->mediatype = mediatype;
	ti->title_id = title_id;
	ti->icon = NULL;
}

void freeTitleInfo(titleInfo_s* ti)
{
	if(!ti)return;

	ti->title_id = 0x0;
	if(ti->icon)free(ti->icon);
	ti->icon = NULL;
}

Result loadTitleInfoIcon(titleInfo_s* ti)
{
	if(!ti || ti->icon)return -1;

	ti->icon = malloc(sizeof(smdh_s));
	if(!ti->icon)return -1;

	Handle fileHandle;
	u32 archivePath[] = {ti->title_id & 0xFFFFFFFF, (ti->title_id >> 32) & 0xFFFFFFFF, ti->mediatype, 0x00000000};
	static const u32 filePath[] = {0x00000000, 0x00000000, 0x00000002, 0x6E6F6369, 0x00000000};	
	Result ret = FSUSER_OpenFileDirectly(&fileHandle, (FS_Archive){ARCHIVE_SAVEDATA_AND_CONTENT, (FS_Path){PATH_BINARY, 0x10, (u8*)archivePath}}, (FS_Path){PATH_BINARY, 0x14, (u8*)filePath}, FS_OPEN_READ, 0);

	if(ret)
	{
		free(ti->icon);
		ti->icon = NULL;
		return ret;
	}

	u32 bytesRead;	
	ret = FSFILE_Read(fileHandle, &bytesRead, 0x0, ti->icon, sizeof(smdh_s));

	if(ret)
	{
		free(ti->icon);
		ti->icon = NULL;
	}

	FSFILE_Close(fileHandle);

	return ret;
}

bool application_filter(u64 tid)
{
	u32 tid_high = tid >> 32;
	return (tid_high == 0x00040010 || tid_high == 0x00040000 || tid_high == 0x00040002);
}

void initTitleList(titleList_s* tl, titleFilter_callback filter, u8 mediatype)
{
	if(!tl)return;

	tl->mediatype = mediatype;

	if(tl->filter) tl->filter = filter;
	else tl->filter = &application_filter;

	tl->num = 0;
	tl->titles = NULL;

	populateTitleList(tl);
}

void freeTitleList(titleList_s* tl)
{
	if(!tl)return;

	int i;
	for(i=0; i<tl->num; i++)freeTitleInfo(&tl->titles[i]);

	tl->num = 0;
	if(tl->titles)free(tl->titles);
	tl->titles = NULL;
}

int populateTitleList(titleList_s* tl)
{
	if(!tl)return 0;
	Result ret;

	u32 old_num = tl->num;

	u32 num;
	ret = AM_GetTitleCount(tl->mediatype, &num);

	if(ret)
	{
		freeTitleList(tl);
	}else if(num){
		// temp buffer is not ideal, but i like modularity
		u64* tmp = (u64*)malloc(sizeof(u64) * num);

		if(!tmp)
		{
			tl->num = 0;
			return 1;
		}

		ret = AM_GetTitleIdList(tl->mediatype, num, tmp);

		if(!ret)
		{
			int i;

			// apply tid filter
			for(i=0; i<num; i++)
			{
				if(!tl->filter(tmp[i]))
				{
					num--;
					tmp[i] = tmp[num];
					i--;
				}
			}

			if(tl->num != num || tl->mediatype == 2)
			{
				freeTitleList(tl);

				tl->num = num;

				if(tl->num) tl->titles = malloc(sizeof(titleInfo_s) * tl->num);

				if(tl->titles)
				{
					for(i=0; i<tl->num; i++)
					{
						initTitleInfo(&tl->titles[i], tl->mediatype, tmp[i]);
					}
				}
			}
		}else tl->num = 0;

		free(tmp);
	}

	return old_num != tl->num;
}

titleInfo_s* findTitleList(titleList_s* tl, u64 tid)
{
	if(!tl)return NULL;

	// special case : gamecard mediatype with 0 tid
	if(!tid && tl->mediatype == 2 && tl->num)return &tl->titles[0];

	int i;
	for(i=0; i<tl->num; i++)
	{
		if(tl->titles[i].title_id == tid)return &tl->titles[i];
	}

	return NULL;
}

titleInfo_s* findTitleBrowser(titleBrowser_s* tb, u8 mediatype, u64 tid)
{
	if(!tb || mediatype > 2)return NULL;

	return findTitleList(&tb->lists[2-mediatype], tid);
}

void initTitleBrowser(titleBrowser_s* tb, titleFilter_callback filter)
{
	if(!tb)return;

	int i;
	for(i=0; i<3; i++)
	{
		initTitleList(&tb->lists[i], filter, (u8)2-i);
	}

	tb->total = 0;
	tb->nextCheck = 0;
	tb->selectedId = 0;
	tb->selected = NULL;
}

void updateTitleBrowser(titleBrowser_s* tb)
{
	if(!tb)return;

	int i;

	if (osGetTime() > tb->nextCheck)
	{
		bool updated = false;

		tb->total = 0;

		for(i=0; i<3; i++)
		{
			updated = populateTitleList(&tb->lists[i]) || updated;
			tb->total += tb->lists[i].num;
		}

		if(updated)
		{
			tb->selectedId = 0;
		}

		tb->nextCheck = osGetTime() + 250;
	}

	tb->selected = NULL;

	if(!tb->total)return;

	u32 padDown = hidKeysDown();

	int move = 0;
	if(padDown & KEY_LEFT)move--;
	if(padDown & KEY_RIGHT)move++;

	tb->selectedId += move;

	while(tb->selectedId < 0) tb->selectedId += tb->total;
	while(tb->selectedId >= tb->total) tb->selectedId -= tb->total;

	int id = tb->selectedId;
	for(i=0; i<3; i++)
	{
		const titleList_s* tl = &tb->lists[i];
		if(id >= 0 && id < tl->num)
		{
			tb->selected = &tl->titles[id];
			break;
		}else id -= tl->num;
	}

	if(tb->selected)
	{
		if(!tb->selected->icon)loadTitleInfoIcon(tb->selected);
		if(tb->selected->icon)extractSmdhData(tb->selected->icon, tb->selectedEntry.name, tb->selectedEntry.description, tb->selectedEntry.author, tb->selectedEntry.iconData);
		else
		{
			tb->selected = NULL;
			if(!move)tb->selectedId++;
			else tb->selectedId += move;
		}
	}
}

void drawTitleBrowser(titleBrowser_s* tb)
{
	if(!tb)return;

	if(tb->selected && tb->selected->icon)
	{
		drawError(GFX_BOTTOM,
			"Target title selector",
			"    The application you're trying to run requires that you select a target.\n\n"
			"                                                                                                        A : Select target\n"
			"                                                                                                        B : Cancel\n",
			10-drawMenuEntry(&tb->selectedEntry, GFX_BOTTOM, 240, 9, true));
	}else{
		drawError(GFX_BOTTOM,
			"Target title selector",
			"    The application you're trying to run requires that you select a target.\n"
			"    No adequate target title could be found. :(\n\n"
			"                                                                                            B : Cancel\n",
			0);
	}
}
