/* DO NOT EDIT THIS FILE - it is machine generated */
#include "native.h"
/* Header for class sun_awt_image_Image */

#ifndef _Included_sun_awt_image_Image
#define _Included_sun_awt_image_Image
struct Hjava_awt_image_ImageProducer;
struct Hsun_awt_image_InputStreamImageSource;
struct Hsun_awt_image_ImageInfoGrabber;
struct Hjava_util_Hashtable;

typedef struct Classsun_awt_image_Image {
/* Inaccessible static: UndefinedProperty */
    struct Hjava_awt_image_ImageProducer *source;
    struct Hsun_awt_image_InputStreamImageSource *src;
    struct Hsun_awt_image_ImageInfoGrabber *info;
    long width;
    long height;
    struct Hjava_util_Hashtable *properties;
    long availinfo;
    struct Hjava_util_Hashtable *representations;
} Classsun_awt_image_Image;
HandleTo(sun_awt_image_Image);

#endif
