#ifndef DESCRIPTOR_H
#define DESCRIPTOR_H

#include <3ds.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "scanner.h"

typedef struct
{
	u64 tid;
	u8 mediatype;
}targetTitle_s;

typedef struct
{
	char name[9];
	int priority;
}serviceRequest_s;

typedef struct
{
	targetTitle_s* targetTitles;
	u32 numTargetTitles;
	
	serviceRequest_s *requestedServices;
	u32 numRequestedServices;

	bool selectTargetProcess;
	bool autodetectServices;

	executableMetadata_s executableMetadata;
}descriptor_s;

void initDescriptor(descriptor_s* d);
void freeDescriptor(descriptor_s* d);
void loadDescriptor(descriptor_s* d, char* path);

#ifdef __cplusplus
}
#endif

#endif
