/* DO NOT EDIT THIS FILE - it is machine generated */
#include "jri.h"

/* Header for class netscape/plugin/Plugin */

#ifndef _netscape_plugin_Plugin_H_
#define _netscape_plugin_Plugin_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct netscape_javascript_JSObject;
struct java_lang_Class;

/*******************************************************************************
 * Class netscape/plugin/Plugin
 ******************************************************************************/

typedef struct netscape_plugin_Plugin netscape_plugin_Plugin;

#define classname_netscape_plugin_Plugin	"netscape/plugin/Plugin"

#define class_netscape_plugin_Plugin(env) \
	((struct java_lang_Class*)JRI_FindClass(env, classname_netscape_plugin_Plugin))

/*******************************************************************************
 * Public Methods
 ******************************************************************************/

#ifdef DEBUG

/*** public getPeer ()I ***/
extern JRI_PUBLIC_API(jint)
netscape_plugin_Plugin_getPeer(JRIEnv* env, struct netscape_plugin_Plugin* self);

/*** public init ()V ***/
extern JRI_PUBLIC_API(void)
netscape_plugin_Plugin_init(JRIEnv* env, struct netscape_plugin_Plugin* self);

/*** public destroy ()V ***/
extern JRI_PUBLIC_API(void)
netscape_plugin_Plugin_destroy(JRIEnv* env, struct netscape_plugin_Plugin* self);

/*** public isActive ()Z ***/
extern JRI_PUBLIC_API(jbool)
netscape_plugin_Plugin_isActive(JRIEnv* env, struct netscape_plugin_Plugin* self);

/*** public getWindow ()Lnetscape/javascript/JSObject; ***/
extern JRI_PUBLIC_API(struct netscape_javascript_JSObject *)
netscape_plugin_Plugin_getWindow(JRIEnv* env, struct netscape_plugin_Plugin* self);

/*** public <init> ()V ***/
extern JRI_PUBLIC_API(struct netscape_plugin_Plugin*)
netscape_plugin_Plugin_new(JRIEnv* env, struct java_lang_Class* clazz);

#else /* !DEBUG */

/*** public getPeer ()I ***/
#define netscape_plugin_Plugin_getPeer(env, obj) \
	(JRI_CallMethodInt(env)(env, JRI_CallMethod_op, obj, methodID_netscape_plugin_Plugin_getPeer))

/*** public init ()V ***/
#define netscape_plugin_Plugin_init(env, obj) \
	((void)JRI_CallMethod(env)(env, JRI_CallMethod_op, obj, methodID_netscape_plugin_Plugin_init))

/*** public destroy ()V ***/
#define netscape_plugin_Plugin_destroy(env, obj) \
	((void)JRI_CallMethod(env)(env, JRI_CallMethod_op, obj, methodID_netscape_plugin_Plugin_destroy))

/*** public isActive ()Z ***/
#define netscape_plugin_Plugin_isActive(env, obj) \
	(JRI_CallMethodBoolean(env)(env, JRI_CallMethod_op, obj, methodID_netscape_plugin_Plugin_isActive))

/*** public getWindow ()Lnetscape/javascript/JSObject; ***/
#define netscape_plugin_Plugin_getWindow(env, obj) \
	((struct netscape_javascript_JSObject *)JRI_CallMethod(env)(env, JRI_CallMethod_op, obj, methodID_netscape_plugin_Plugin_getWindow))

/*** public <init> ()V ***/
#define netscape_plugin_Plugin_new(env, clazz)	\
	((struct netscape_plugin_Plugin*)JRI_NewObject(env)(env, JRI_NewObject_op, clazz, methodID_netscape_plugin_Plugin_new))

#endif /* !DEBUG*/

/*** public getPeer ()I ***/
extern JRIMethodID FAR methodID_netscape_plugin_Plugin_getPeer;
#define name_netscape_plugin_Plugin_getPeer	"getPeer"
#define sig_netscape_plugin_Plugin_getPeer 	"()I"
#define use_netscape_plugin_Plugin_getPeer(env, clazz)	\
	(methodID_netscape_plugin_Plugin_getPeer =	\
		JRI_GetMethodID(env, clazz,	\
			name_netscape_plugin_Plugin_getPeer,	\
			sig_netscape_plugin_Plugin_getPeer))
#define unuse_netscape_plugin_Plugin_getPeer(env, clazz)	\
	(methodID_netscape_plugin_Plugin_getPeer = JRIUninitialized)

/*** public init ()V ***/
extern JRIMethodID FAR methodID_netscape_plugin_Plugin_init;
#define name_netscape_plugin_Plugin_init	"init"
#define sig_netscape_plugin_Plugin_init 	"()V"
#define use_netscape_plugin_Plugin_init(env, clazz)	\
	(methodID_netscape_plugin_Plugin_init =	\
		JRI_GetMethodID(env, clazz,	\
			name_netscape_plugin_Plugin_init,	\
			sig_netscape_plugin_Plugin_init))
#define unuse_netscape_plugin_Plugin_init(env, clazz)	\
	(methodID_netscape_plugin_Plugin_init = JRIUninitialized)

/*** public destroy ()V ***/
extern JRIMethodID FAR methodID_netscape_plugin_Plugin_destroy;
#define name_netscape_plugin_Plugin_destroy	"destroy"
#define sig_netscape_plugin_Plugin_destroy 	"()V"
#define use_netscape_plugin_Plugin_destroy(env, clazz)	\
	(methodID_netscape_plugin_Plugin_destroy =	\
		JRI_GetMethodID(env, clazz,	\
			name_netscape_plugin_Plugin_destroy,	\
			sig_netscape_plugin_Plugin_destroy))
#define unuse_netscape_plugin_Plugin_destroy(env, clazz)	\
	(methodID_netscape_plugin_Plugin_destroy = JRIUninitialized)

/*** public isActive ()Z ***/
extern JRIMethodID FAR methodID_netscape_plugin_Plugin_isActive;
#define name_netscape_plugin_Plugin_isActive	"isActive"
#define sig_netscape_plugin_Plugin_isActive 	"()Z"
#define use_netscape_plugin_Plugin_isActive(env, clazz)	\
	(methodID_netscape_plugin_Plugin_isActive =	\
		JRI_GetMethodID(env, clazz,	\
			name_netscape_plugin_Plugin_isActive,	\
			sig_netscape_plugin_Plugin_isActive))
#define unuse_netscape_plugin_Plugin_isActive(env, clazz)	\
	(methodID_netscape_plugin_Plugin_isActive = JRIUninitialized)

/*** public getWindow ()Lnetscape/javascript/JSObject; ***/
extern JRIMethodID FAR methodID_netscape_plugin_Plugin_getWindow;
#define name_netscape_plugin_Plugin_getWindow	"getWindow"
#define sig_netscape_plugin_Plugin_getWindow 	"()Lnetscape/javascript/JSObject;"
#define use_netscape_plugin_Plugin_getWindow(env, clazz)	\
	(methodID_netscape_plugin_Plugin_getWindow =	\
		JRI_GetMethodID(env, clazz,	\
			name_netscape_plugin_Plugin_getWindow,	\
			sig_netscape_plugin_Plugin_getWindow))
#define unuse_netscape_plugin_Plugin_getWindow(env, clazz)	\
	(methodID_netscape_plugin_Plugin_getWindow = JRIUninitialized)

/*** public <init> ()V ***/
extern JRIMethodID FAR methodID_netscape_plugin_Plugin_new;
#define name_netscape_plugin_Plugin_new	"<init>"
#define sig_netscape_plugin_Plugin_new 	"()V"
#define use_netscape_plugin_Plugin_new(env, clazz)	\
	(methodID_netscape_plugin_Plugin_new =	\
		JRI_GetMethodID(env, clazz,	\
			name_netscape_plugin_Plugin_new,	\
			sig_netscape_plugin_Plugin_new))
#define unuse_netscape_plugin_Plugin_new(env, clazz)	\
	(methodID_netscape_plugin_Plugin_new = JRIUninitialized)

/*******************************************************************************
 * IMPLEMENTATION SECTION: 
 * Define the IMPLEMENT_netscape_plugin_Plugin symbol 
 * if you intend to implement the native methods of this class. This 
 * symbol makes the private and protected methods available. You should 
 * also call the register_netscape_plugin_Plugin routine 
 * to make your native methods available.
 ******************************************************************************/

extern JRI_PUBLIC_API(struct java_lang_Class*)
use_netscape_plugin_Plugin(JRIEnv* env);

extern JRI_PUBLIC_API(void)
unuse_netscape_plugin_Plugin(JRIEnv* env);

extern JRI_PUBLIC_API(struct java_lang_Class*)
register_netscape_plugin_Plugin(JRIEnv* env);

extern JRI_PUBLIC_API(void)
unregister_netscape_plugin_Plugin(JRIEnv* env);

#ifdef IMPLEMENT_netscape_plugin_Plugin

/*******************************************************************************
 * Implementation Field Accessors: 
 * You should only use these from within the native method definitions.
 ******************************************************************************/

#ifdef DEBUG

/*** peer I ***/
extern JRI_PUBLIC_API(jint)
get_netscape_plugin_Plugin_peer(JRIEnv* env, netscape_plugin_Plugin* obj);
extern JRI_PUBLIC_API(void)
set_netscape_plugin_Plugin_peer(JRIEnv* env, netscape_plugin_Plugin* obj, jint v);

/*** window Lnetscape/javascript/JSObject; ***/
extern JRI_PUBLIC_API(struct netscape_javascript_JSObject *)
get_netscape_plugin_Plugin_window(JRIEnv* env, netscape_plugin_Plugin* obj);
extern JRI_PUBLIC_API(void)
set_netscape_plugin_Plugin_window(JRIEnv* env, netscape_plugin_Plugin* obj, struct netscape_javascript_JSObject * v);

#else /* !DEBUG */

/*** peer I ***/
#define get_netscape_plugin_Plugin_peer(env, obj) \
	(JRI_GetFieldInt(env, obj, fieldID_netscape_plugin_Plugin_peer))
#define set_netscape_plugin_Plugin_peer(env, obj, v) \
	JRI_SetFieldInt(env, obj, fieldID_netscape_plugin_Plugin_peer, v)

/*** window Lnetscape/javascript/JSObject; ***/
#define get_netscape_plugin_Plugin_window(env, obj) \
	((struct netscape_javascript_JSObject *)JRI_GetField(env, obj, fieldID_netscape_plugin_Plugin_window))
#define set_netscape_plugin_Plugin_window(env, obj, v) \
	JRI_SetField(env, obj, fieldID_netscape_plugin_Plugin_window, v)

#endif /* !DEBUG*/

extern JRIFieldID FAR fieldID_netscape_plugin_Plugin_peer;
#define name_netscape_plugin_Plugin_peer	"peer"
#define sig_netscape_plugin_Plugin_peer 	"I"
#define use_netscape_plugin_Plugin_peer(env, clazz)	\
	(fieldID_netscape_plugin_Plugin_peer =	\
		JRI_GetFieldID(env, clazz,	\
			name_netscape_plugin_Plugin_peer,	\
			sig_netscape_plugin_Plugin_peer))
#define unuse_netscape_plugin_Plugin_peer(env, clazz)	\
		(fieldID_netscape_plugin_Plugin_peer = JRIUninitialized)

extern JRIFieldID FAR fieldID_netscape_plugin_Plugin_window;
#define name_netscape_plugin_Plugin_window	"window"
#define sig_netscape_plugin_Plugin_window 	"Lnetscape/javascript/JSObject;"
#define use_netscape_plugin_Plugin_window(env, clazz)	\
	(fieldID_netscape_plugin_Plugin_window =	\
		JRI_GetFieldID(env, clazz,	\
			name_netscape_plugin_Plugin_window,	\
			sig_netscape_plugin_Plugin_window))
#define unuse_netscape_plugin_Plugin_window(env, clazz)	\
		(fieldID_netscape_plugin_Plugin_window = JRIUninitialized)

#endif /* IMPLEMENT_netscape_plugin_Plugin */

#ifdef __cplusplus
} /* extern "C" */

/*******************************************************************************
 * C++ Definitions
 ******************************************************************************/

#include "java_lang_Object.h"

struct netscape_plugin_Plugin : virtual public java_lang_Object {

	static struct java_lang_Class* _use(JRIEnv* env) {
		return use_netscape_plugin_Plugin(env);
	}

	static void _unuse(JRIEnv* env) {
		unuse_netscape_plugin_Plugin(env);
	}

	static struct java_lang_Class* _register(JRIEnv* env) {
		return register_netscape_plugin_Plugin(env);
	}

	static void _unregister(JRIEnv* env) {
		unregister_netscape_plugin_Plugin(env);
	}

	static struct java_lang_Class* _class(JRIEnv* env) {
		return class_netscape_plugin_Plugin(env);
	}

	/* Public Methods */
	/*** public getPeer ()I ***/
	jint getPeer(JRIEnv* env) {
		return netscape_plugin_Plugin_getPeer(env, this);
	}

	/*** public init ()V ***/
	void init(JRIEnv* env) {
		netscape_plugin_Plugin_init(env, this);
	}

	/*** public destroy ()V ***/
	void destroy(JRIEnv* env) {
		netscape_plugin_Plugin_destroy(env, this);
	}

	/*** public isActive ()Z ***/
	jbool isActive(JRIEnv* env) {
		return netscape_plugin_Plugin_isActive(env, this);
	}

	/*** public getWindow ()Lnetscape/javascript/JSObject; ***/
	struct netscape_javascript_JSObject * getWindow(JRIEnv* env) {
		return netscape_plugin_Plugin_getWindow(env, this);
	}

	/*** public <init> ()V ***/
	static netscape_plugin_Plugin* _new(JRIEnv* env, struct java_lang_Class* clazz) {
		return netscape_plugin_Plugin_new(env, clazz);
	}

#ifdef IMPLEMENT_netscape_plugin_Plugin

#endif /* IMPLEMENT_netscape_plugin_Plugin */
};

#endif /* __cplusplus */

#endif /* Class netscape/plugin/Plugin */
