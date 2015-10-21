#ifndef SCANNER_H
#define SCANNER_H

#define NUM_SERVICESTHATMATTER 5

typedef struct
{
	bool scanned;
	u32 sectionSizes[3];
	u8 servicesThatMatter[NUM_SERVICESTHATMATTER];
}executableMetadata_s;

void initMetadata(executableMetadata_s* em);

Result scan3dsx(char* path, char** patterns, int num_patterns, u32* sectionSizes, bool* patternsFound);
void scanExecutable(executableMetadata_s* em, char* path);

#endif
