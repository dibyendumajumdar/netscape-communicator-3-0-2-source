/* DO NOT EDIT THIS FILE - it is machine generated */
#include "native.h"
/* Header for class java_lang_Double */

#ifndef _Included_java_lang_Double
#define _Included_java_lang_Double

typedef struct Classjava_lang_Double {
#define java_lang_Double_POSITIVE_INFINITY	infD
#define java_lang_Double_NEGATIVE_INFINITY	-infD
#define java_lang_Double_NaN	nan0x7fffffffD
#define java_lang_Double_MAX_VALUE	1.79769e+308D
#define java_lang_Double_MIN_VALUE	2.22507e-308D
    double value;
} Classjava_lang_Double;
HandleTo(java_lang_Double);

struct Hjava_lang_String;
extern struct Hjava_lang_String *java_lang_Double_toString(struct Hjava_lang_Double* self,double);
struct Hjava_lang_Double;
extern struct Hjava_lang_Double *java_lang_Double_valueOf(struct Hjava_lang_Double* self,struct Hjava_lang_String *);
extern int64_t java_lang_Double_doubleToLongBits(struct Hjava_lang_Double* self,double);
extern double java_lang_Double_longBitsToDouble(struct Hjava_lang_Double* self,int64_t);
#endif
