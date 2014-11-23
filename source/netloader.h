#pragma once

#define NETLOADER_TMPFILE "/netloader.3dsx"
#define NETLOADER_PORT 9000

extern bool netloader_active;
extern bool netloader_boot;

void netloader_activate();
void netloader_deactivate();
void netloader_init();
int netloader_loop();
void netloader_exit();

