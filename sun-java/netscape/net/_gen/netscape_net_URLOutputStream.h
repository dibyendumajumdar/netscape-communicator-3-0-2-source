/* DO NOT EDIT THIS FILE - it is machine generated */
#include "native.h"
/* Header for class netscape_net_URLOutputStream */

#ifndef _Included_netscape_net_URLOutputStream
#define _Included_netscape_net_URLOutputStream
struct Hnetscape_net_URLConnection;

typedef struct Classnetscape_net_URLOutputStream {
    struct Hnetscape_net_URLConnection *connection;
} Classnetscape_net_URLOutputStream;
HandleTo(netscape_net_URLOutputStream);

extern void netscape_net_URLOutputStream_open(struct Hnetscape_net_URLOutputStream* self);
extern void netscape_net_URLOutputStream_write(struct Hnetscape_net_URLOutputStream* self,long);
extern void netscape_net_URLOutputStream_writeBytes(struct Hnetscape_net_URLOutputStream* self,HArrayOfByte *,long,long);
extern void netscape_net_URLOutputStream_pClose(struct Hnetscape_net_URLOutputStream* self);
#endif
