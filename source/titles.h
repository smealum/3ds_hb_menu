#ifndef TITLES_H
#define TITLES_H

#include <3ds.h>

#include "smdh.h"
#include "menu.h"

typedef struct
{
	u8 mediatype;
	u64 title_id;
	smdh_s* icon;
}titleInfo_s;

typedef bool (*titleFilter_callback)(u64 tid);

typedef struct
{
	u8 mediatype;
	u32 num;
	titleInfo_s* titles;
	titleFilter_callback filter;
}titleList_s;

typedef struct
{
	titleList_s lists[3];
	u32 total;
	u64 nextCheck;
	int selectedId;
	titleInfo_s* selected;
	menuEntry_s selectedEntry;
}titleBrowser_s;

void titlesInit();
void titlesExit();

void initTitleInfo(titleInfo_s* ti, u8 mediatype, u64 title_id);
void freeTitleInfo(titleInfo_s* ti);

void initTitleList(titleList_s* tl, titleFilter_callback filter, u8 mediatype);
void freeTitleList(titleList_s* tl);
int populateTitleList(titleList_s* tl);

void initTitleBrowser(titleBrowser_s* tb, titleFilter_callback filter);
void updateTitleBrowser(titleBrowser_s* tb);
void drawTitleBrowser(titleBrowser_s* tb);
titleInfo_s* findTitleBrowser(titleBrowser_s* tb, u8 mediatype, u64 tid);

#endif
