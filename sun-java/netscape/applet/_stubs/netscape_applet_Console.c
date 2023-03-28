/* DO NOT EDIT THIS FILE - it is machine generated */
#include "jri.h"

/* Stubs for class netscape/applet/Console */

#ifdef IMPLEMENT_netscape_applet_Console
#define _implementing_netscape_applet_Console
#endif /* IMPLEMENT_netscape_applet_Console */
#define IMPLEMENT_netscape_applet_Console
#include "netscape_applet_Console.h"

#ifndef UNUSED_netscape_applet_Console_frame
JRIFieldID FAR fieldID_netscape_applet_Console_frame = JRIUninitialized;
#ifdef DEBUG
struct netscape_applet_ConsoleFrame;
JRI_PUBLIC_API(struct netscape_applet_ConsoleFrame *)
get_netscape_applet_Console_frame(JRIEnv* env, struct netscape_applet_Console* obj) {
	if (fieldID_netscape_applet_Console_frame == JRIUninitialized) {
		assert(!"Forgot to call use_netscape_applet_Console(env) before accessing field frame Lnetscape/applet/ConsoleFrame;");
	}
	return (struct netscape_applet_ConsoleFrame *)
		JRI_GetField(env, obj, fieldID_netscape_applet_Console_frame);
}
JRI_PUBLIC_API(void)
set_netscape_applet_Console_frame(JRIEnv* env, struct netscape_applet_Console* obj, struct netscape_applet_ConsoleFrame * v) {
	if (fieldID_netscape_applet_Console_frame == JRIUninitialized) {
		assert(!"Forgot to call use_netscape_applet_Console(env) before accessing field frame Lnetscape/applet/ConsoleFrame;");
	}
	JRI_SetField(env, obj, fieldID_netscape_applet_Console_frame, v);
}
#endif /* DEBUG */
#endif /* UNUSED_netscape_applet_Console_frame */

#ifndef UNUSED_netscape_applet_Console_new
JRIMethodID FAR methodID_netscape_applet_Console_new = JRIUninitialized;
#ifdef DEBUG
JRI_PUBLIC_API(struct netscape_applet_Console*)
netscape_applet_Console_new(JRIEnv* env, struct java_lang_Class* clazz) {
	if (methodID_netscape_applet_Console_new == JRIUninitialized) {
		assert(!"Forgot to call use_netscape_applet_Console(env) before calling method <init> ()V");
	}
	return (struct netscape_applet_Console*)JRI_NewObject(env)(env, JRI_NewObject_op, clazz, methodID_netscape_applet_Console_new);
}
#endif /* DEBUG */
#endif /* UNUSED_netscape_applet_Console_new */

#ifndef UNUSED_netscape_applet_Console_setSystemIO
JRIMethodID FAR methodID_netscape_applet_Console_setSystemIO = JRIUninitialized;
#ifdef DEBUG
JRI_PUBLIC_API(void)
netscape_applet_Console_setSystemIO(JRIEnv* env, struct java_lang_Class* clazz, struct java_io_InputStream *a, struct java_io_PrintStream *b, struct java_io_PrintStream *c) {
	if (methodID_netscape_applet_Console_setSystemIO == JRIUninitialized) {
		assert(!"Forgot to call use_netscape_applet_Console(env) before calling method setSystemIO (Ljava/io/InputStream;Ljava/io/PrintStream;Ljava/io/PrintStream;)V");
	}
	(void)JRI_CallStaticMethod(env)(env, JRI_CallStaticMethod_op, clazz, methodID_netscape_applet_Console_setSystemIO, a, b, c);
}
#endif /* DEBUG */
#endif /* UNUSED_netscape_applet_Console_setSystemIO */

#ifndef UNUSED_netscape_applet_Console_dumpNSPRInfo
JRIMethodID FAR methodID_netscape_applet_Console_dumpNSPRInfo = JRIUninitialized;
#ifdef DEBUG
JRI_PUBLIC_API(void)
netscape_applet_Console_dumpNSPRInfo(JRIEnv* env, struct java_lang_Class* clazz) {
	if (methodID_netscape_applet_Console_dumpNSPRInfo == JRIUninitialized) {
		assert(!"Forgot to call use_netscape_applet_Console(env) before calling method dumpNSPRInfo ()V");
	}
	(void)JRI_CallStaticMethod(env)(env, JRI_CallStaticMethod_op, clazz, methodID_netscape_applet_Console_dumpNSPRInfo);
}
#endif /* DEBUG */
#endif /* UNUSED_netscape_applet_Console_dumpNSPRInfo */

#ifndef UNUSED_netscape_applet_Console_dumpMemory
JRIMethodID FAR methodID_netscape_applet_Console_dumpMemory = JRIUninitialized;
#ifdef DEBUG
JRI_PUBLIC_API(void)
netscape_applet_Console_dumpMemory(JRIEnv* env, struct java_lang_Class* clazz, jbool a) {
	if (methodID_netscape_applet_Console_dumpMemory == JRIUninitialized) {
		assert(!"Forgot to call use_netscape_applet_Console(env) before calling method dumpMemory (Z)V");
	}
	(void)JRI_CallStaticMethod(env)(env, JRI_CallStaticMethod_op, clazz, methodID_netscape_applet_Console_dumpMemory, a);
}
#endif /* DEBUG */
#endif /* UNUSED_netscape_applet_Console_dumpMemory */

#ifndef UNUSED_netscape_applet_Console_dumpMemorySummary
JRIMethodID FAR methodID_netscape_applet_Console_dumpMemorySummary = JRIUninitialized;
#ifdef DEBUG
JRI_PUBLIC_API(void)
netscape_applet_Console_dumpMemorySummary(JRIEnv* env, struct java_lang_Class* clazz) {
	if (methodID_netscape_applet_Console_dumpMemorySummary == JRIUninitialized) {
		assert(!"Forgot to call use_netscape_applet_Console(env) before calling method dumpMemorySummary ()V");
	}
	(void)JRI_CallStaticMethod(env)(env, JRI_CallStaticMethod_op, clazz, methodID_netscape_applet_Console_dumpMemorySummary);
}
#endif /* DEBUG */
#endif /* UNUSED_netscape_applet_Console_dumpMemorySummary */

#ifndef UNUSED_netscape_applet_Console_checkpointMemory
JRIMethodID FAR methodID_netscape_applet_Console_checkpointMemory = JRIUninitialized;
#ifdef DEBUG
JRI_PUBLIC_API(void)
netscape_applet_Console_checkpointMemory(JRIEnv* env, struct java_lang_Class* clazz) {
	if (methodID_netscape_applet_Console_checkpointMemory == JRIUninitialized) {
		assert(!"Forgot to call use_netscape_applet_Console(env) before calling method checkpointMemory ()V");
	}
	(void)JRI_CallStaticMethod(env)(env, JRI_CallStaticMethod_op, clazz, methodID_netscape_applet_Console_checkpointMemory);
}
#endif /* DEBUG */
#endif /* UNUSED_netscape_applet_Console_checkpointMemory */

#ifndef UNUSED_netscape_applet_Console_dumpApplicationHeaps
JRIMethodID FAR methodID_netscape_applet_Console_dumpApplicationHeaps = JRIUninitialized;
#ifdef DEBUG
JRI_PUBLIC_API(void)
netscape_applet_Console_dumpApplicationHeaps(JRIEnv* env, struct java_lang_Class* clazz) {
	if (methodID_netscape_applet_Console_dumpApplicationHeaps == JRIUninitialized) {
		assert(!"Forgot to call use_netscape_applet_Console(env) before calling method dumpApplicationHeaps ()V");
	}
	(void)JRI_CallStaticMethod(env)(env, JRI_CallStaticMethod_op, clazz, methodID_netscape_applet_Console_dumpApplicationHeaps);
}
#endif /* DEBUG */
#endif /* UNUSED_netscape_applet_Console_dumpApplicationHeaps */

#ifndef UNUSED_netscape_applet_Console_reset
JRIMethodID FAR methodID_netscape_applet_Console_reset = JRIUninitialized;
#ifdef DEBUG
JRI_PUBLIC_API(void)
netscape_applet_Console_reset(JRIEnv* env, struct netscape_applet_Console* self) {
	if (methodID_netscape_applet_Console_reset == JRIUninitialized) {
		assert(!"Forgot to call use_netscape_applet_Console(env) before calling method reset ()V");
	}
	(void)JRI_CallMethod(env)(env, JRI_CallMethod_op, self, methodID_netscape_applet_Console_reset);
}
#endif /* DEBUG */
#endif /* UNUSED_netscape_applet_Console_reset */

#ifndef UNUSED_netscape_applet_Console_show
JRIMethodID FAR methodID_netscape_applet_Console_show = JRIUninitialized;
#ifdef DEBUG
JRI_PUBLIC_API(void)
netscape_applet_Console_show(JRIEnv* env, struct netscape_applet_Console* self) {
	if (methodID_netscape_applet_Console_show == JRIUninitialized) {
		assert(!"Forgot to call use_netscape_applet_Console(env) before calling method show ()V");
	}
	(void)JRI_CallMethod(env)(env, JRI_CallMethod_op, self, methodID_netscape_applet_Console_show);
}
#endif /* DEBUG */
#endif /* UNUSED_netscape_applet_Console_show */

#ifndef UNUSED_netscape_applet_Console_hide
JRIMethodID FAR methodID_netscape_applet_Console_hide = JRIUninitialized;
#ifdef DEBUG
JRI_PUBLIC_API(void)
netscape_applet_Console_hide(JRIEnv* env, struct netscape_applet_Console* self) {
	if (methodID_netscape_applet_Console_hide == JRIUninitialized) {
		assert(!"Forgot to call use_netscape_applet_Console(env) before calling method hide ()V");
	}
	(void)JRI_CallMethod(env)(env, JRI_CallMethod_op, self, methodID_netscape_applet_Console_hide);
}
#endif /* DEBUG */
#endif /* UNUSED_netscape_applet_Console_hide */

#ifndef UNUSED_use_netscape_applet_Console

static jglobal _globalclass_netscape_applet_Console = NULL;

JRI_PUBLIC_API(struct java_lang_Class*)
use_netscape_applet_Console(JRIEnv* env)
{
	if (_globalclass_netscape_applet_Console == NULL) {
		struct java_lang_Class* clazz = JRI_FindClass(env, classname_netscape_applet_Console);
		if (clazz == NULL) {
			JRI_ThrowNew(env, JRI_FindClass(env, "java/lang/ClassNotFoundException"), classname_netscape_applet_Console);
			return NULL;
		}
		use_netscape_applet_Console_frame(env, clazz);
		use_netscape_applet_Console_new(env, clazz);
		use_netscape_applet_Console_setSystemIO(env, clazz);
		use_netscape_applet_Console_dumpNSPRInfo(env, clazz);
		use_netscape_applet_Console_dumpMemory(env, clazz);
		use_netscape_applet_Console_dumpMemorySummary(env, clazz);
		use_netscape_applet_Console_checkpointMemory(env, clazz);
		use_netscape_applet_Console_dumpApplicationHeaps(env, clazz);
		use_netscape_applet_Console_reset(env, clazz);
		use_netscape_applet_Console_show(env, clazz);
		use_netscape_applet_Console_hide(env, clazz);
		_globalclass_netscape_applet_Console = JRI_NewGlobalRef(env, clazz);
		return clazz;
	}
	else {
		return (struct java_lang_Class*)JRI_GetGlobalRef(env, _globalclass_netscape_applet_Console);
	}
}

JRI_PUBLIC_API(void)
unuse_netscape_applet_Console(JRIEnv* env)
{
	if (_globalclass_netscape_applet_Console != NULL) {
		struct java_lang_Class* clazz = (struct java_lang_Class*)JRI_GetGlobalRef(env, _globalclass_netscape_applet_Console);
		unuse_netscape_applet_Console_frame(env, clazz);
		unuse_netscape_applet_Console_new(env, clazz);
		unuse_netscape_applet_Console_setSystemIO(env, clazz);
		unuse_netscape_applet_Console_dumpNSPRInfo(env, clazz);
		unuse_netscape_applet_Console_dumpMemory(env, clazz);
		unuse_netscape_applet_Console_dumpMemorySummary(env, clazz);
		unuse_netscape_applet_Console_checkpointMemory(env, clazz);
		unuse_netscape_applet_Console_dumpApplicationHeaps(env, clazz);
		unuse_netscape_applet_Console_reset(env, clazz);
		unuse_netscape_applet_Console_show(env, clazz);
		unuse_netscape_applet_Console_hide(env, clazz);
		JRI_DisposeGlobalRef(env, _globalclass_netscape_applet_Console);
		_globalclass_netscape_applet_Console = NULL;
		clazz = NULL;	/* prevent unused variable 'clazz' warning */
	}
}

#endif /* UNUSED_use_netscape_applet_Console */

#ifdef _implementing_netscape_applet_Console

JRI_PUBLIC_API(struct java_lang_Class*)
register_netscape_applet_Console(JRIEnv* env)
{
	char* nativeNamesAndSigs[] = {
		"setSystemIO(Ljava/io/InputStream;Ljava/io/PrintStream;Ljava/io/PrintStream;)V",
		"dumpNSPRInfo()V",
		"dumpMemory(Z)V",
		"dumpMemorySummary()V",
		"checkpointMemory()V",
		"dumpApplicationHeaps()V",
		NULL
	};
	void* nativeProcs[] = {
		(void*)native_netscape_applet_Console_setSystemIO,
		(void*)native_netscape_applet_Console_dumpNSPRInfo,
		(void*)native_netscape_applet_Console_dumpMemory,
		(void*)native_netscape_applet_Console_dumpMemorySummary,
		(void*)native_netscape_applet_Console_checkpointMemory,
		(void*)native_netscape_applet_Console_dumpApplicationHeaps,
		NULL
	};
	struct java_lang_Class* clazz = JRI_FindClass(env, classname_netscape_applet_Console);
	if (clazz == NULL) {
		JRI_ThrowNew(env, JRI_FindClass(env, "java/lang/ClassNotFoundException"), classname_netscape_applet_Console);
		return NULL;
	}
	JRI_RegisterNatives(env, clazz, nativeNamesAndSigs, nativeProcs);
	use_netscape_applet_Console(env);
	return clazz;
}

JRI_PUBLIC_API(void)
unregister_netscape_applet_Console(JRIEnv* env)
{
	struct java_lang_Class* clazz = JRI_FindClass(env, classname_netscape_applet_Console);
	JRI_UnregisterNatives(env, clazz);
	unuse_netscape_applet_Console(env);
}

#endif /* _implementing_netscape_applet_Console */

/* These stub routines are generated for compatibility with the JDK: */

#ifndef NO_JDK

/* SYMBOL: "netscape/applet/Console/setSystemIO(Ljava/io/InputStream;Ljava/io/PrintStream;Ljava/io/PrintStream;)V", Java_netscape_applet_Console_setSystemIO_stub */
JRI_PUBLIC_API(JRI_JDK_stack_item*)
Java_netscape_applet_Console_setSystemIO_stub(JRI_JDK_stack_item* _P_, JRIEnv* _EE_) {
	(void) native_netscape_applet_Console_setSystemIO(_EE_,NULL,((_P_[0].p)),((_P_[1].p)),((_P_[2].p)));
	return _P_;
}

/* SYMBOL: "netscape/applet/Console/dumpNSPRInfo()V", Java_netscape_applet_Console_dumpNSPRInfo_stub */
JRI_PUBLIC_API(JRI_JDK_stack_item*)
Java_netscape_applet_Console_dumpNSPRInfo_stub(JRI_JDK_stack_item* _P_, JRIEnv* _EE_) {
	(void) native_netscape_applet_Console_dumpNSPRInfo(_EE_,NULL);
	return _P_;
}

/* SYMBOL: "netscape/applet/Console/dumpMemory(Z)V", Java_netscape_applet_Console_dumpMemory_stub */
JRI_PUBLIC_API(JRI_JDK_stack_item*)
Java_netscape_applet_Console_dumpMemory_stub(JRI_JDK_stack_item* _P_, JRIEnv* _EE_) {
	(void) native_netscape_applet_Console_dumpMemory(_EE_,NULL,((_P_[0].i)));
	return _P_;
}

/* SYMBOL: "netscape/applet/Console/dumpMemorySummary()V", Java_netscape_applet_Console_dumpMemorySummary_stub */
JRI_PUBLIC_API(JRI_JDK_stack_item*)
Java_netscape_applet_Console_dumpMemorySummary_stub(JRI_JDK_stack_item* _P_, JRIEnv* _EE_) {
	(void) native_netscape_applet_Console_dumpMemorySummary(_EE_,NULL);
	return _P_;
}

/* SYMBOL: "netscape/applet/Console/checkpointMemory()V", Java_netscape_applet_Console_checkpointMemory_stub */
JRI_PUBLIC_API(JRI_JDK_stack_item*)
Java_netscape_applet_Console_checkpointMemory_stub(JRI_JDK_stack_item* _P_, JRIEnv* _EE_) {
	(void) native_netscape_applet_Console_checkpointMemory(_EE_,NULL);
	return _P_;
}

/* SYMBOL: "netscape/applet/Console/dumpApplicationHeaps()V", Java_netscape_applet_Console_dumpApplicationHeaps_stub */
JRI_PUBLIC_API(JRI_JDK_stack_item*)
Java_netscape_applet_Console_dumpApplicationHeaps_stub(JRI_JDK_stack_item* _P_, JRIEnv* _EE_) {
	(void) native_netscape_applet_Console_dumpApplicationHeaps(_EE_,NULL);
	return _P_;
}

#endif /* NO_JDK */

