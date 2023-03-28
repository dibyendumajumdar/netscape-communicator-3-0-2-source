/* DO NOT EDIT THIS FILE - it is machine generated */
#include "jri.h"

/* Header for class netscape/javascript/JSObject */

#ifndef _netscape_javascript_JSObject_H_
#define _netscape_javascript_JSObject_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct java_lang_String;
struct java_lang_Object;
struct java_applet_Applet;
struct netscape_javascript_JSObject;
struct java_lang_Class;

/*******************************************************************************
 * Class netscape/javascript/JSObject
 ******************************************************************************/

typedef struct netscape_javascript_JSObject netscape_javascript_JSObject;

#define classname_netscape_javascript_JSObject	"netscape/javascript/JSObject"

#define class_netscape_javascript_JSObject(env) \
	((struct java_lang_Class*)JRI_FindClass(env, classname_netscape_javascript_JSObject))

/*******************************************************************************
 * Public Methods
 ******************************************************************************/

#ifdef DEBUG

/*** public native getMember (Ljava/lang/String;)Ljava/lang/Object; ***/
extern JRI_PUBLIC_API(struct java_lang_Object *)
netscape_javascript_JSObject_getMember(JRIEnv* env, struct netscape_javascript_JSObject* self, struct java_lang_String *a);

/*** public native getSlot (I)Ljava/lang/Object; ***/
extern JRI_PUBLIC_API(struct java_lang_Object *)
netscape_javascript_JSObject_getSlot(JRIEnv* env, struct netscape_javascript_JSObject* self, jint a);

/*** public native setMember (Ljava/lang/String;Ljava/lang/Object;)V ***/
extern JRI_PUBLIC_API(void)
netscape_javascript_JSObject_setMember(JRIEnv* env, struct netscape_javascript_JSObject* self, struct java_lang_String *a, struct java_lang_Object *b);

/*** public native setSlot (ILjava/lang/Object;)V ***/
extern JRI_PUBLIC_API(void)
netscape_javascript_JSObject_setSlot(JRIEnv* env, struct netscape_javascript_JSObject* self, jint a, struct java_lang_Object *b);

/*** public native removeMember (Ljava/lang/String;)V ***/
extern JRI_PUBLIC_API(void)
netscape_javascript_JSObject_removeMember(JRIEnv* env, struct netscape_javascript_JSObject* self, struct java_lang_String *a);

/*** public native call (Ljava/lang/String;[Ljava/lang/Object;)Ljava/lang/Object; ***/
extern JRI_PUBLIC_API(struct java_lang_Object *)
netscape_javascript_JSObject_call(JRIEnv* env, struct netscape_javascript_JSObject* self, struct java_lang_String *a, jobjectArray b);

/*** public native eval (Ljava/lang/String;)Ljava/lang/Object; ***/
extern JRI_PUBLIC_API(struct java_lang_Object *)
netscape_javascript_JSObject_eval(JRIEnv* env, struct netscape_javascript_JSObject* self, struct java_lang_String *a);

/*** public native toString ()Ljava/lang/String; ***/
extern JRI_PUBLIC_API(struct java_lang_String *)
netscape_javascript_JSObject_toString(JRIEnv* env, struct netscape_javascript_JSObject* self);

/*** public static native getWindow (Ljava/applet/Applet;)Lnetscape/javascript/JSObject; ***/
extern JRI_PUBLIC_API(struct netscape_javascript_JSObject *)
netscape_javascript_JSObject_getWindow(JRIEnv* env, struct java_lang_Class* clazz, struct java_applet_Applet *a);

#else /* !DEBUG */

/*** public native getMember (Ljava/lang/String;)Ljava/lang/Object; ***/
#define netscape_javascript_JSObject_getMember(env, obj, a) \
	((struct java_lang_Object *)JRI_CallMethod(env)(env, JRI_CallMethod_op, obj, methodID_netscape_javascript_JSObject_getMember, a))

/*** public native getSlot (I)Ljava/lang/Object; ***/
#define netscape_javascript_JSObject_getSlot(env, obj, a) \
	((struct java_lang_Object *)JRI_CallMethod(env)(env, JRI_CallMethod_op, obj, methodID_netscape_javascript_JSObject_getSlot, a))

/*** public native setMember (Ljava/lang/String;Ljava/lang/Object;)V ***/
#define netscape_javascript_JSObject_setMember(env, obj, a, b) \
	((void)JRI_CallMethod(env)(env, JRI_CallMethod_op, obj, methodID_netscape_javascript_JSObject_setMember, a, b))

/*** public native setSlot (ILjava/lang/Object;)V ***/
#define netscape_javascript_JSObject_setSlot(env, obj, a, b) \
	((void)JRI_CallMethod(env)(env, JRI_CallMethod_op, obj, methodID_netscape_javascript_JSObject_setSlot, a, b))

/*** public native removeMember (Ljava/lang/String;)V ***/
#define netscape_javascript_JSObject_removeMember(env, obj, a) \
	((void)JRI_CallMethod(env)(env, JRI_CallMethod_op, obj, methodID_netscape_javascript_JSObject_removeMember, a))

/*** public native call (Ljava/lang/String;[Ljava/lang/Object;)Ljava/lang/Object; ***/
#define netscape_javascript_JSObject_call(env, obj, a, b) \
	((struct java_lang_Object *)JRI_CallMethod(env)(env, JRI_CallMethod_op, obj, methodID_netscape_javascript_JSObject_call, a, b))

/*** public native eval (Ljava/lang/String;)Ljava/lang/Object; ***/
#define netscape_javascript_JSObject_eval(env, obj, a) \
	((struct java_lang_Object *)JRI_CallMethod(env)(env, JRI_CallMethod_op, obj, methodID_netscape_javascript_JSObject_eval, a))

/*** public native toString ()Ljava/lang/String; ***/
#define netscape_javascript_JSObject_toString(env, obj) \
	((struct java_lang_String *)JRI_CallMethod(env)(env, JRI_CallMethod_op, obj, methodID_netscape_javascript_JSObject_toString))

/*** public static native getWindow (Ljava/applet/Applet;)Lnetscape/javascript/JSObject; ***/
#define netscape_javascript_JSObject_getWindow(env, clazz, a) \
	((struct netscape_javascript_JSObject *)JRI_CallStaticMethod(env)(env, JRI_CallStaticMethod_op, clazz, methodID_netscape_javascript_JSObject_getWindow, a))

#endif /* !DEBUG*/

/*** public native getMember (Ljava/lang/String;)Ljava/lang/Object; ***/
extern JRIMethodID FAR methodID_netscape_javascript_JSObject_getMember;
#define name_netscape_javascript_JSObject_getMember	"getMember"
#define sig_netscape_javascript_JSObject_getMember 	"(Ljava/lang/String;)Ljava/lang/Object;"
#define use_netscape_javascript_JSObject_getMember(env, clazz)	\
	(methodID_netscape_javascript_JSObject_getMember =	\
		JRI_GetMethodID(env, clazz,	\
			name_netscape_javascript_JSObject_getMember,	\
			sig_netscape_javascript_JSObject_getMember))
#define unuse_netscape_javascript_JSObject_getMember(env, clazz)	\
	(methodID_netscape_javascript_JSObject_getMember = JRIUninitialized)

/*** public native getSlot (I)Ljava/lang/Object; ***/
extern JRIMethodID FAR methodID_netscape_javascript_JSObject_getSlot;
#define name_netscape_javascript_JSObject_getSlot	"getSlot"
#define sig_netscape_javascript_JSObject_getSlot 	"(I)Ljava/lang/Object;"
#define use_netscape_javascript_JSObject_getSlot(env, clazz)	\
	(methodID_netscape_javascript_JSObject_getSlot =	\
		JRI_GetMethodID(env, clazz,	\
			name_netscape_javascript_JSObject_getSlot,	\
			sig_netscape_javascript_JSObject_getSlot))
#define unuse_netscape_javascript_JSObject_getSlot(env, clazz)	\
	(methodID_netscape_javascript_JSObject_getSlot = JRIUninitialized)

/*** public native setMember (Ljava/lang/String;Ljava/lang/Object;)V ***/
extern JRIMethodID FAR methodID_netscape_javascript_JSObject_setMember;
#define name_netscape_javascript_JSObject_setMember	"setMember"
#define sig_netscape_javascript_JSObject_setMember 	"(Ljava/lang/String;Ljava/lang/Object;)V"
#define use_netscape_javascript_JSObject_setMember(env, clazz)	\
	(methodID_netscape_javascript_JSObject_setMember =	\
		JRI_GetMethodID(env, clazz,	\
			name_netscape_javascript_JSObject_setMember,	\
			sig_netscape_javascript_JSObject_setMember))
#define unuse_netscape_javascript_JSObject_setMember(env, clazz)	\
	(methodID_netscape_javascript_JSObject_setMember = JRIUninitialized)

/*** public native setSlot (ILjava/lang/Object;)V ***/
extern JRIMethodID FAR methodID_netscape_javascript_JSObject_setSlot;
#define name_netscape_javascript_JSObject_setSlot	"setSlot"
#define sig_netscape_javascript_JSObject_setSlot 	"(ILjava/lang/Object;)V"
#define use_netscape_javascript_JSObject_setSlot(env, clazz)	\
	(methodID_netscape_javascript_JSObject_setSlot =	\
		JRI_GetMethodID(env, clazz,	\
			name_netscape_javascript_JSObject_setSlot,	\
			sig_netscape_javascript_JSObject_setSlot))
#define unuse_netscape_javascript_JSObject_setSlot(env, clazz)	\
	(methodID_netscape_javascript_JSObject_setSlot = JRIUninitialized)

/*** public native removeMember (Ljava/lang/String;)V ***/
extern JRIMethodID FAR methodID_netscape_javascript_JSObject_removeMember;
#define name_netscape_javascript_JSObject_removeMember	"removeMember"
#define sig_netscape_javascript_JSObject_removeMember 	"(Ljava/lang/String;)V"
#define use_netscape_javascript_JSObject_removeMember(env, clazz)	\
	(methodID_netscape_javascript_JSObject_removeMember =	\
		JRI_GetMethodID(env, clazz,	\
			name_netscape_javascript_JSObject_removeMember,	\
			sig_netscape_javascript_JSObject_removeMember))
#define unuse_netscape_javascript_JSObject_removeMember(env, clazz)	\
	(methodID_netscape_javascript_JSObject_removeMember = JRIUninitialized)

/*** public native call (Ljava/lang/String;[Ljava/lang/Object;)Ljava/lang/Object; ***/
extern JRIMethodID FAR methodID_netscape_javascript_JSObject_call;
#define name_netscape_javascript_JSObject_call	"call"
#define sig_netscape_javascript_JSObject_call 	"(Ljava/lang/String;[Ljava/lang/Object;)Ljava/lang/Object;"
#define use_netscape_javascript_JSObject_call(env, clazz)	\
	(methodID_netscape_javascript_JSObject_call =	\
		JRI_GetMethodID(env, clazz,	\
			name_netscape_javascript_JSObject_call,	\
			sig_netscape_javascript_JSObject_call))
#define unuse_netscape_javascript_JSObject_call(env, clazz)	\
	(methodID_netscape_javascript_JSObject_call = JRIUninitialized)

/*** public native eval (Ljava/lang/String;)Ljava/lang/Object; ***/
extern JRIMethodID FAR methodID_netscape_javascript_JSObject_eval;
#define name_netscape_javascript_JSObject_eval	"eval"
#define sig_netscape_javascript_JSObject_eval 	"(Ljava/lang/String;)Ljava/lang/Object;"
#define use_netscape_javascript_JSObject_eval(env, clazz)	\
	(methodID_netscape_javascript_JSObject_eval =	\
		JRI_GetMethodID(env, clazz,	\
			name_netscape_javascript_JSObject_eval,	\
			sig_netscape_javascript_JSObject_eval))
#define unuse_netscape_javascript_JSObject_eval(env, clazz)	\
	(methodID_netscape_javascript_JSObject_eval = JRIUninitialized)

/*** public native toString ()Ljava/lang/String; ***/
extern JRIMethodID FAR methodID_netscape_javascript_JSObject_toString;
#define name_netscape_javascript_JSObject_toString	"toString"
#define sig_netscape_javascript_JSObject_toString 	"()Ljava/lang/String;"
#define use_netscape_javascript_JSObject_toString(env, clazz)	\
	(methodID_netscape_javascript_JSObject_toString =	\
		JRI_GetMethodID(env, clazz,	\
			name_netscape_javascript_JSObject_toString,	\
			sig_netscape_javascript_JSObject_toString))
#define unuse_netscape_javascript_JSObject_toString(env, clazz)	\
	(methodID_netscape_javascript_JSObject_toString = JRIUninitialized)

/*** public static native getWindow (Ljava/applet/Applet;)Lnetscape/javascript/JSObject; ***/
extern JRIMethodID FAR methodID_netscape_javascript_JSObject_getWindow;
#define name_netscape_javascript_JSObject_getWindow	"getWindow"
#define sig_netscape_javascript_JSObject_getWindow 	"(Ljava/applet/Applet;)Lnetscape/javascript/JSObject;"
#define use_netscape_javascript_JSObject_getWindow(env, clazz)	\
	(methodID_netscape_javascript_JSObject_getWindow =	\
		JRI_GetStaticMethodID(env, clazz,	\
			name_netscape_javascript_JSObject_getWindow,	\
			sig_netscape_javascript_JSObject_getWindow))
#define unuse_netscape_javascript_JSObject_getWindow(env, clazz)	\
	(methodID_netscape_javascript_JSObject_getWindow = JRIUninitialized)

/*******************************************************************************
 * IMPLEMENTATION SECTION: 
 * Define the IMPLEMENT_netscape_javascript_JSObject symbol 
 * if you intend to implement the native methods of this class. This 
 * symbol makes the private and protected methods available. You should 
 * also call the register_netscape_javascript_JSObject routine 
 * to make your native methods available.
 ******************************************************************************/

extern JRI_PUBLIC_API(struct java_lang_Class*)
use_netscape_javascript_JSObject(JRIEnv* env);

extern JRI_PUBLIC_API(void)
unuse_netscape_javascript_JSObject(JRIEnv* env);

extern JRI_PUBLIC_API(struct java_lang_Class*)
register_netscape_javascript_JSObject(JRIEnv* env);

extern JRI_PUBLIC_API(void)
unregister_netscape_javascript_JSObject(JRIEnv* env);

#ifdef IMPLEMENT_netscape_javascript_JSObject

/*******************************************************************************
 * Native Methods: 
 * These are the signatures of the native methods which you must implement.
 ******************************************************************************/

/*** private static native initClass ()V ***/
extern JRI_PUBLIC_API(void)
native_netscape_javascript_JSObject_initClass(JRIEnv* env, struct java_lang_Class* clazz);

/*** public native getMember (Ljava/lang/String;)Ljava/lang/Object; ***/
extern JRI_PUBLIC_API(struct java_lang_Object *)
native_netscape_javascript_JSObject_getMember(JRIEnv* env, struct netscape_javascript_JSObject* self, struct java_lang_String *a);

/*** public native getSlot (I)Ljava/lang/Object; ***/
extern JRI_PUBLIC_API(struct java_lang_Object *)
native_netscape_javascript_JSObject_getSlot(JRIEnv* env, struct netscape_javascript_JSObject* self, jint a);

/*** public native setMember (Ljava/lang/String;Ljava/lang/Object;)V ***/
extern JRI_PUBLIC_API(void)
native_netscape_javascript_JSObject_setMember(JRIEnv* env, struct netscape_javascript_JSObject* self, struct java_lang_String *a, struct java_lang_Object *b);

/*** public native setSlot (ILjava/lang/Object;)V ***/
extern JRI_PUBLIC_API(void)
native_netscape_javascript_JSObject_setSlot(JRIEnv* env, struct netscape_javascript_JSObject* self, jint a, struct java_lang_Object *b);

/*** public native removeMember (Ljava/lang/String;)V ***/
extern JRI_PUBLIC_API(void)
native_netscape_javascript_JSObject_removeMember(JRIEnv* env, struct netscape_javascript_JSObject* self, struct java_lang_String *a);

/*** public native call (Ljava/lang/String;[Ljava/lang/Object;)Ljava/lang/Object; ***/
extern JRI_PUBLIC_API(struct java_lang_Object *)
native_netscape_javascript_JSObject_call(JRIEnv* env, struct netscape_javascript_JSObject* self, struct java_lang_String *a, jobjectArray b);

/*** public native eval (Ljava/lang/String;)Ljava/lang/Object; ***/
extern JRI_PUBLIC_API(struct java_lang_Object *)
native_netscape_javascript_JSObject_eval(JRIEnv* env, struct netscape_javascript_JSObject* self, struct java_lang_String *a);

/*** public native toString ()Ljava/lang/String; ***/
extern JRI_PUBLIC_API(struct java_lang_String *)
native_netscape_javascript_JSObject_toString(JRIEnv* env, struct netscape_javascript_JSObject* self);

/*** public static native getWindow (Ljava/applet/Applet;)Lnetscape/javascript/JSObject; ***/
extern JRI_PUBLIC_API(struct netscape_javascript_JSObject *)
native_netscape_javascript_JSObject_getWindow(JRIEnv* env, struct java_lang_Class* clazz, struct java_applet_Applet *a);

/*** protected native finalize ()V ***/
extern JRI_PUBLIC_API(void)
native_netscape_javascript_JSObject_finalize(JRIEnv* env, struct netscape_javascript_JSObject* self);

/*******************************************************************************
 * Implementation Field Accessors: 
 * You should only use these from within the native method definitions.
 ******************************************************************************/

#ifdef DEBUG

/*** private internal I ***/
extern JRI_PUBLIC_API(jint)
get_netscape_javascript_JSObject_internal(JRIEnv* env, netscape_javascript_JSObject* obj);
extern JRI_PUBLIC_API(void)
set_netscape_javascript_JSObject_internal(JRIEnv* env, netscape_javascript_JSObject* obj, jint v);

#else /* !DEBUG */

/*** private internal I ***/
#define get_netscape_javascript_JSObject_internal(env, obj) \
	(JRI_GetFieldInt(env, obj, fieldID_netscape_javascript_JSObject_internal))
#define set_netscape_javascript_JSObject_internal(env, obj, v) \
	JRI_SetFieldInt(env, obj, fieldID_netscape_javascript_JSObject_internal, v)

#endif /* !DEBUG*/

extern JRIFieldID FAR fieldID_netscape_javascript_JSObject_internal;
#define name_netscape_javascript_JSObject_internal	"internal"
#define sig_netscape_javascript_JSObject_internal 	"I"
#define use_netscape_javascript_JSObject_internal(env, clazz)	\
	(fieldID_netscape_javascript_JSObject_internal =	\
		JRI_GetFieldID(env, clazz,	\
			name_netscape_javascript_JSObject_internal,	\
			sig_netscape_javascript_JSObject_internal))
#define unuse_netscape_javascript_JSObject_internal(env, clazz)	\
		(fieldID_netscape_javascript_JSObject_internal = JRIUninitialized)

/*******************************************************************************
 * Implementation Methods: 
 * You should only use these from within the native method definitions.
 ******************************************************************************/

#ifdef DEBUG

/*** private static native initClass ()V ***/
extern JRI_PUBLIC_API(void)
netscape_javascript_JSObject_initClass(JRIEnv* env, struct java_lang_Class* clazz);

/*** private <init> ()V ***/
extern JRI_PUBLIC_API(struct netscape_javascript_JSObject*)
netscape_javascript_JSObject_new(JRIEnv* env, struct java_lang_Class* clazz);

/*** protected native finalize ()V ***/
extern JRI_PUBLIC_API(void)
netscape_javascript_JSObject_finalize(JRIEnv* env, struct netscape_javascript_JSObject* self);

/*** static <clinit> ()V ***/
extern JRI_PUBLIC_API(void)
netscape_javascript_JSObject__0003cclinit_0003e(JRIEnv* env, struct java_lang_Class* clazz);

#else /* !DEBUG */

/*** private static native initClass ()V ***/
#define netscape_javascript_JSObject_initClass(env, clazz) \
	((void)JRI_CallStaticMethod(env)(env, JRI_CallStaticMethod_op, clazz, methodID_netscape_javascript_JSObject_initClass))

/*** private <init> ()V ***/
#define netscape_javascript_JSObject_new(env, clazz)	\
	((struct netscape_javascript_JSObject*)JRI_NewObject(env)(env, JRI_NewObject_op, clazz, methodID_netscape_javascript_JSObject_new))

/*** protected native finalize ()V ***/
#define netscape_javascript_JSObject_finalize(env, obj) \
	((void)JRI_CallMethod(env)(env, JRI_CallMethod_op, obj, methodID_netscape_javascript_JSObject_finalize))

/*** static <clinit> ()V ***/
#define netscape_javascript_JSObject__0003cclinit_0003e(env, clazz) \
	((void)JRI_CallStaticMethod(env)(env, JRI_CallStaticMethod_op, clazz, methodID_netscape_javascript_JSObject__0003cclinit_0003e))

#endif /* !DEBUG*/

/*** private static native initClass ()V ***/
extern JRIMethodID FAR methodID_netscape_javascript_JSObject_initClass;
#define name_netscape_javascript_JSObject_initClass	"initClass"
#define sig_netscape_javascript_JSObject_initClass 	"()V"
#define use_netscape_javascript_JSObject_initClass(env, clazz)	\
	(methodID_netscape_javascript_JSObject_initClass =	\
		JRI_GetStaticMethodID(env, clazz,	\
			name_netscape_javascript_JSObject_initClass,	\
			sig_netscape_javascript_JSObject_initClass))
#define unuse_netscape_javascript_JSObject_initClass(env, clazz)	\
	(methodID_netscape_javascript_JSObject_initClass = JRIUninitialized)

/*** private <init> ()V ***/
extern JRIMethodID FAR methodID_netscape_javascript_JSObject_new;
#define name_netscape_javascript_JSObject_new	"<init>"
#define sig_netscape_javascript_JSObject_new 	"()V"
#define use_netscape_javascript_JSObject_new(env, clazz)	\
	(methodID_netscape_javascript_JSObject_new =	\
		JRI_GetMethodID(env, clazz,	\
			name_netscape_javascript_JSObject_new,	\
			sig_netscape_javascript_JSObject_new))
#define unuse_netscape_javascript_JSObject_new(env, clazz)	\
	(methodID_netscape_javascript_JSObject_new = JRIUninitialized)

/*** protected native finalize ()V ***/
extern JRIMethodID FAR methodID_netscape_javascript_JSObject_finalize;
#define name_netscape_javascript_JSObject_finalize	"finalize"
#define sig_netscape_javascript_JSObject_finalize 	"()V"
#define use_netscape_javascript_JSObject_finalize(env, clazz)	\
	(methodID_netscape_javascript_JSObject_finalize =	\
		JRI_GetMethodID(env, clazz,	\
			name_netscape_javascript_JSObject_finalize,	\
			sig_netscape_javascript_JSObject_finalize))
#define unuse_netscape_javascript_JSObject_finalize(env, clazz)	\
	(methodID_netscape_javascript_JSObject_finalize = JRIUninitialized)

/*** static <clinit> ()V ***/
extern JRIMethodID FAR methodID_netscape_javascript_JSObject__0003cclinit_0003e;
#define name_netscape_javascript_JSObject__0003cclinit_0003e	"<clinit>"
#define sig_netscape_javascript_JSObject__0003cclinit_0003e 	"()V"
#define use_netscape_javascript_JSObject__0003cclinit_0003e(env, clazz)	\
	(methodID_netscape_javascript_JSObject__0003cclinit_0003e =	\
		JRI_GetStaticMethodID(env, clazz,	\
			name_netscape_javascript_JSObject__0003cclinit_0003e,	\
			sig_netscape_javascript_JSObject__0003cclinit_0003e))
#define unuse_netscape_javascript_JSObject__0003cclinit_0003e(env, clazz)	\
	(methodID_netscape_javascript_JSObject__0003cclinit_0003e = JRIUninitialized)

#endif /* IMPLEMENT_netscape_javascript_JSObject */

#ifdef __cplusplus
} /* extern "C" */

/*******************************************************************************
 * C++ Definitions
 ******************************************************************************/

#include "java_lang_Object.h"

struct netscape_javascript_JSObject : public java_lang_Object {

	static struct java_lang_Class* _use(JRIEnv* env) {
		return use_netscape_javascript_JSObject(env);
	}

	static void _unuse(JRIEnv* env) {
		unuse_netscape_javascript_JSObject(env);
	}

	static struct java_lang_Class* _register(JRIEnv* env) {
		return register_netscape_javascript_JSObject(env);
	}

	static void _unregister(JRIEnv* env) {
		unregister_netscape_javascript_JSObject(env);
	}

	static struct java_lang_Class* _class(JRIEnv* env) {
		return class_netscape_javascript_JSObject(env);
	}

	/* Public Methods */
	/*** public native getMember (Ljava/lang/String;)Ljava/lang/Object; ***/
	struct java_lang_Object * getMember(JRIEnv* env, struct java_lang_String *a) {
		return netscape_javascript_JSObject_getMember(env, this, a);
	}

	/*** public native getSlot (I)Ljava/lang/Object; ***/
	struct java_lang_Object * getSlot(JRIEnv* env, jint a) {
		return netscape_javascript_JSObject_getSlot(env, this, a);
	}

	/*** public native setMember (Ljava/lang/String;Ljava/lang/Object;)V ***/
	void setMember(JRIEnv* env, struct java_lang_String *a, struct java_lang_Object *b) {
		netscape_javascript_JSObject_setMember(env, this, a, b);
	}

	/*** public native setSlot (ILjava/lang/Object;)V ***/
	void setSlot(JRIEnv* env, jint a, struct java_lang_Object *b) {
		netscape_javascript_JSObject_setSlot(env, this, a, b);
	}

	/*** public native removeMember (Ljava/lang/String;)V ***/
	void removeMember(JRIEnv* env, struct java_lang_String *a) {
		netscape_javascript_JSObject_removeMember(env, this, a);
	}

	/*** public native call (Ljava/lang/String;[Ljava/lang/Object;)Ljava/lang/Object; ***/
	struct java_lang_Object * call(JRIEnv* env, struct java_lang_String *a, jobjectArray b) {
		return netscape_javascript_JSObject_call(env, this, a, b);
	}

	/*** public native eval (Ljava/lang/String;)Ljava/lang/Object; ***/
	struct java_lang_Object * eval(JRIEnv* env, struct java_lang_String *a) {
		return netscape_javascript_JSObject_eval(env, this, a);
	}

	/*** public native toString ()Ljava/lang/String; ***/
	struct java_lang_String * toString(JRIEnv* env) {
		return netscape_javascript_JSObject_toString(env, this);
	}

	/*** public static native getWindow (Ljava/applet/Applet;)Lnetscape/javascript/JSObject; ***/
	static struct netscape_javascript_JSObject * getWindow(JRIEnv* env, struct java_lang_Class* clazz, struct java_applet_Applet *a) {
		return netscape_javascript_JSObject_getWindow(env, clazz, a);
	}

#ifdef IMPLEMENT_netscape_javascript_JSObject

	/* Protected Methods */
	/*** protected native finalize ()V ***/
	void finalize(JRIEnv* env) {
		netscape_javascript_JSObject_finalize(env, this);
	}

	/* Private Field Accessors */
	/*** private internal I ***/
	jint internal(JRIEnv* env) {
		return get_netscape_javascript_JSObject_internal(env, this);
	}
	void internal(JRIEnv* env, jint v) {
		set_netscape_javascript_JSObject_internal(env, this, v);
	}

	/* Private Methods */
	/*** private static native initClass ()V ***/
	static void initClass(JRIEnv* env, struct java_lang_Class* clazz) {
		netscape_javascript_JSObject_initClass(env, clazz);
	}

	/*** private <init> ()V ***/
	static netscape_javascript_JSObject* _new(JRIEnv* env, struct java_lang_Class* clazz) {
		return netscape_javascript_JSObject_new(env, clazz);
	}

#endif /* IMPLEMENT_netscape_javascript_JSObject */
};

#endif /* __cplusplus */

#endif /* Class netscape/javascript/JSObject */
