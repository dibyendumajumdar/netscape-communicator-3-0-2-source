/* DO NOT EDIT THIS FILE - it is machine generated */
#include "native.h"
/* Header for class sun_awt_motif_MFileDialogPeer */

#ifndef _Included_sun_awt_motif_MFileDialogPeer
#define _Included_sun_awt_motif_MFileDialogPeer
struct Hjava_awt_Component;
struct Hjava_awt_Insets;

typedef struct Classsun_awt_motif_MFileDialogPeer {
    struct Hjava_awt_Component *target;
    long pData;
    struct Hjava_awt_Insets *insets;
/* Inaccessible static: allDialogs */
} Classsun_awt_motif_MFileDialogPeer;
HandleTo(sun_awt_motif_MFileDialogPeer);

struct Hsun_awt_motif_MComponentPeer;
extern void sun_awt_motif_MFileDialogPeer_create(struct Hsun_awt_motif_MFileDialogPeer* self,struct Hsun_awt_motif_MComponentPeer *);
extern void sun_awt_motif_MFileDialogPeer_pReshape(struct Hsun_awt_motif_MFileDialogPeer* self,long,long,long,long);
extern void sun_awt_motif_MFileDialogPeer_pShow(struct Hsun_awt_motif_MFileDialogPeer* self);
extern void sun_awt_motif_MFileDialogPeer_pHide(struct Hsun_awt_motif_MFileDialogPeer* self);
struct Hjava_lang_String;
extern void sun_awt_motif_MFileDialogPeer_setFileEntry(struct Hsun_awt_motif_MFileDialogPeer* self,struct Hjava_lang_String *,struct Hjava_lang_String *);
#endif
