#pragma once

#define NETLOADER_PORT 17491

extern char *netloadedPath;

int netloader_activate(void);
int netloader_deactivate(void);
int netloader_init(void);
int netloader_loop(void);
int netloader_exit(void);
int netloader_draw_error(void);
