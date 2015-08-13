#ifndef DESCRIPTOR_H
#define DESCRIPTOR_H

#include <3ds.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct
{
	u64 tid;
	u8 mediatype;
}targetTitle_s;

typedef struct
{
	targetTitle_s* targetTitles;
	u32 numTargetTitles;
	
	char** requestedServices;
	u32 numRequestedServices;

	bool selectTargetProcess;
}descriptor_s;

void initDescriptor(descriptor_s* d);
void loadDescriptor(descriptor_s* d, char* path);

#ifdef __cplusplus
}
#endif

#endif
