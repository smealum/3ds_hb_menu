#include "descriptor.h"
#include "tinyxml2.h"

using namespace tinyxml2;

void initDescriptor(descriptor_s* d)
{
	if(!d)return;

	d->targetTitles = NULL;
	d->numTargetTitles = 0;
	
	d->requestedServices = NULL;
	d->numRequestedServices = 0;

	d->selectTargetProcess = false;
}

// TODO : error checking
void loadDescriptor(descriptor_s* d, char* path)
{
	if(!d || !path)return;

    XMLDocument doc;
    if(doc.LoadFile(path))return;

    XMLElement* targets = doc.FirstChildElement("targets");
    if(targets)
    {
    	// grab selectable target flag
    	{
    		if(targets->QueryBoolAttribute("selectable", &d->selectTargetProcess)) d->selectTargetProcess = false;
    	}

    	// grab preferred target titles
    	{
			d->numTargetTitles = 0;
			for (tinyxml2::XMLElement* child = targets->FirstChildElement(); child != NULL; child = child->NextSiblingElement())
			{
				if(!strcmp(child->Name(), "title"))
				{
					d->numTargetTitles++;
				}
			}

			d->targetTitles = (targetTitle_s*)malloc(sizeof(targetTitle_s) * d->numTargetTitles);
			d->numTargetTitles = 0;

			for (tinyxml2::XMLElement* child = targets->FirstChildElement(); child != NULL; child = child->NextSiblingElement())
			{
				if(!strcmp(child->Name(), "title"))
				{
					// SD is default mediatype
					int mediatype;
					if(child->QueryIntAttribute("mediatype", &mediatype))mediatype = 1;

					d->targetTitles[d->numTargetTitles].tid = strtoull(child->GetText(), NULL, 16);
					d->targetTitles[d->numTargetTitles].mediatype = mediatype;

					d->numTargetTitles++;
				}
			}
    	}
    }
}
