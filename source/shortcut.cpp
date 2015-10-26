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
	s->name = NULL;
	s->description = NULL;
	s->author = NULL;
}

Result createShortcut(shortcut_s* s, char* path)
{
	if(!s || !path)return -1;

	initShortcut(s);
	
	return loadShortcut(s, path);
}

void loadXmlString(char** out, XMLElement* in, const char* key)
{
	if(!out || !in || !key)return;

	XMLElement* node = in->FirstChildElement(key);
	if(node)
	{
		const char* str = node->GetText();
		if(str)
		{
			*out = (char*)malloc(strlen(str) + 1);
			if(*out) strcpy(*out, str);
		}
	}
}

// TODO : error checking
Result loadShortcut(shortcut_s* s, char* path)
{
	if(!s || !path)return -1;

	XMLDocument doc;
	if(doc.LoadFile(path))return -2;

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
		if(!s->executable) return -3;

		XMLElement* descriptor = shortcut->FirstChildElement("descriptor");
		const char* descriptor_path = path;
		if(descriptor) descriptor_path = descriptor->GetText();
		if(descriptor_path)
		{
			s->descriptor = (char*)malloc(strlen(descriptor_path) + 1);
			if(s->descriptor) strcpy(s->descriptor, descriptor_path);
		}

		loadXmlString(&s->icon, shortcut, "icon");
		loadXmlString(&s->arg, shortcut, "arg");
		loadXmlString(&s->name, shortcut, "name");
		loadXmlString(&s->description, shortcut, "description");
		loadXmlString(&s->author, shortcut, "author");
	}else return -4;

	return 0;
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
