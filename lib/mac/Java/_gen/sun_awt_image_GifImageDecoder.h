/* DO NOT EDIT THIS FILE - it is machine generated */
#include "native.h"
/* Header for class sun_awt_image_GifImageDecoder */

#ifndef _Included_sun_awt_image_GifImageDecoder
#define _Included_sun_awt_image_GifImageDecoder
struct Hsun_awt_image_InputStreamImageSource;
struct Hjava_io_InputStream;
struct Hsun_awt_image_PixelStore8;
struct Hjava_awt_image_IndexColorModel;
struct Hjava_util_Hashtable;

typedef struct Classsun_awt_image_GifImageDecoder {
    struct Hsun_awt_image_InputStreamImageSource *source;
    struct Hjava_io_InputStream *input;
    /*boolean*/ long verbose;
#define sun_awt_image_GifImageDecoder_IMAGESEP	44L
#define sun_awt_image_GifImageDecoder_EXBLOCK	33L
#define sun_awt_image_GifImageDecoder_EX_GRAPHICS_CONTROL	249L
#define sun_awt_image_GifImageDecoder_EX_COMMENT	254L
#define sun_awt_image_GifImageDecoder_EX_APPLICATION	255L
#define sun_awt_image_GifImageDecoder_TERMINATOR	59L
#define sun_awt_image_GifImageDecoder_INTERLACEMASK	64L
#define sun_awt_image_GifImageDecoder_COLORMAPMASK	128L
    struct Hsun_awt_image_PixelStore8 *store;
    long num_colors;
    struct HArrayOfByte *colormap;
    struct Hjava_awt_image_IndexColorModel *model;
    struct Hjava_util_Hashtable *props;
#define sun_awt_image_GifImageDecoder_normalflags	30L
#define sun_awt_image_GifImageDecoder_interlaceflags	29L
} Classsun_awt_image_GifImageDecoder;
HandleTo(sun_awt_image_GifImageDecoder);

extern /*boolean*/ long sun_awt_image_GifImageDecoder_parseImage(struct Hsun_awt_image_GifImageDecoder* self,long,long,/*boolean*/ long,long,HArrayOfByte *,HArrayOfByte *);
#endif
