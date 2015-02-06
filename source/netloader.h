#pragma once

#define NETLOADER_TMPFILE "/netloader.3dsx"
#define NETLOADER_PORT 9000

int netloader_activate(void);
int netloader_deactivate(void);
int netloader_init(void);
int netloader_loop(void);
int netloader_exit(void);
int netloader_draw_error(void);
