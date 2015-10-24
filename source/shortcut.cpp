#include <3ds.h>

#include "shortcut.h"
#include "tinyxml2.h"

using namespace tinyxml2;

void initShortcut(shortcut_s* s)
{
	if(!s)return;

	s->executable = NULL;
	s->descriptor = NULL;
	s->icon = NULL;
	s->arg = NULL;
}

// TODO : error checking
void loadShortcut(shortcut_s* s, char* path)
{
	if(!s || !path)return;

	XMLDocument doc;
	if(doc.LoadFile(path))return;

	XMLElement* shortcut = doc.FirstChildElement("shortcut");
	if(shortcut)
	{
		XMLElement* executable = shortcut->FirstChildElement("executable");
		if(executable)
		{
			const char* str = executable->GetText();
			if(str)
			{
				s->executable = (char*)malloc(strlen(str) + 1);
				if(s->executable) strcpy(s->executable, str);
			}
		}
		if(!s->executable) return;

		XMLElement* descriptor = shortcut->FirstChildElement("descriptor");
		const char* descriptor_path = path;
		if(descriptor) descriptor_path = descriptor->GetText();
		if(descriptor_path)
		{
			s->descriptor = (char*)malloc(strlen(descriptor_path) + 1);
			if(s->descriptor) strcpy(s->descriptor, descriptor_path);
		}

		XMLElement* icon = shortcut->FirstChildElement("icon");
		if(icon)
		{
			const char* str = icon->GetText();
			if(str)
			{
				s->icon = (char*)malloc(strlen(str) + 1);
				if(s->icon) strcpy(s->icon, str);
			}
		}

		XMLElement* arg = shortcut->FirstChildElement("arg");
		if(arg)
		{
			const char* str = arg->GetText();
			if(str)
			{
				s->arg = (char*)malloc(strlen(str) + 1);
				if(s->arg) strcpy(s->arg, str);
			}
		}

	}
}

void freeShortcut(shortcut_s* s)
{
	if(!s)return;

	free(s->executable);
	s->executable = NULL;

	free(s->descriptor);
	s->descriptor = NULL;

	free(s->icon);
	s->icon = NULL;

	free(s->arg);
	s->arg = NULL;
}
