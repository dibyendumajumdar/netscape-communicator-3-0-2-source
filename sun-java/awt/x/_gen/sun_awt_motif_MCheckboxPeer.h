/* DO NOT EDIT THIS FILE - it is machine generated */
#include "native.h"
/* Header for class sun_awt_motif_MCheckboxPeer */

#ifndef _Included_sun_awt_motif_MCheckboxPeer
#define _Included_sun_awt_motif_MCheckboxPeer
struct Hjava_awt_Component;

typedef struct Classsun_awt_motif_MCheckboxPeer {
    struct Hjava_awt_Component *target;
    long pData;
} Classsun_awt_motif_MCheckboxPeer;
HandleTo(sun_awt_motif_MCheckboxPeer);

struct Hsun_awt_motif_MComponentPeer;
extern void sun_awt_motif_MCheckboxPeer_create(struct Hsun_awt_motif_MCheckboxPeer* self,struct Hsun_awt_motif_MComponentPeer *);
struct Hjava_lang_String;
extern void sun_awt_motif_MCheckboxPeer_setLabel(struct Hsun_awt_motif_MCheckboxPeer* self,struct Hjava_lang_String *);
extern void sun_awt_motif_MCheckboxPeer_setState(struct Hsun_awt_motif_MCheckboxPeer* self,/*boolean*/ long);
struct Hjava_awt_CheckboxGroup;
extern void sun_awt_motif_MCheckboxPeer_setCheckboxGroup(struct Hsun_awt_motif_MCheckboxPeer* self,struct Hjava_awt_CheckboxGroup *);
#endif
