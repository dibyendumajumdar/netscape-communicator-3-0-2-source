/* DO NOT EDIT THIS FILE - it is machine generated */
#include "native.h"
/* Header for class java_net_PlainSocketImpl */

#ifndef _Included_java_net_PlainSocketImpl
#define _Included_java_net_PlainSocketImpl
struct Hjava_io_FileDescriptor;
struct Hjava_net_InetAddress;

typedef struct Classjava_net_PlainSocketImpl {
    struct Hjava_io_FileDescriptor *fd;
    struct Hjava_net_InetAddress *address;
    long port;
    long localport;
} Classjava_net_PlainSocketImpl;
HandleTo(java_net_PlainSocketImpl);

extern void java_net_PlainSocketImpl_socketCreate(struct Hjava_net_PlainSocketImpl* self,/*boolean*/ long);
extern void java_net_PlainSocketImpl_socketConnect(struct Hjava_net_PlainSocketImpl* self,struct Hjava_net_InetAddress *,long);
extern void java_net_PlainSocketImpl_socketBind(struct Hjava_net_PlainSocketImpl* self,struct Hjava_net_InetAddress *,long);
extern void java_net_PlainSocketImpl_socketListen(struct Hjava_net_PlainSocketImpl* self,long);
struct Hjava_net_SocketImpl;
extern void java_net_PlainSocketImpl_socketAccept(struct Hjava_net_PlainSocketImpl* self,struct Hjava_net_SocketImpl *);
extern long java_net_PlainSocketImpl_socketAvailable(struct Hjava_net_PlainSocketImpl* self);
extern void java_net_PlainSocketImpl_socketClose(struct Hjava_net_PlainSocketImpl* self);
#endif
