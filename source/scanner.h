#ifndef SCANNER_H
#define SCANNER_H

typedef struct
{
	bool scanned;
	u32 sectionSizes[3];
	bool servicesThatMatter[2];
}executableMetadata_s;

void initMetadata(executableMetadata_s* em);

Result scan3dsx(char* path, char** patterns, int num_patterns, u32* sectionSizes, bool* patternsFound);
void scanExecutable(executableMetadata_s* em, char* path);

#endif
