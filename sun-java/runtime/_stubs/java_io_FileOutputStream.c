/* DO NOT EDIT THIS FILE - it is machine generated */
#include "StubPreamble.h"
/* Stubs for class java/io/FileOutputStream */

/* SYMBOL: "java/io/FileOutputStream/open(Ljava/lang/String;)V", Java_java_io_FileOutputStream_open_stub */
JRI_PUBLIC_API(stack_item*)
Java_java_io_FileOutputStream_open_stub(stack_item* _P_, struct execenv* _EE_) {
	extern void java_io_FileOutputStream_open(void *, void *);
	(void) java_io_FileOutputStream_open(_P_[0].p,((_P_[1].p)));
	return _P_;
}

/* SYMBOL: "java/io/FileOutputStream/write(I)V", Java_java_io_FileOutputStream_write_stub */
JRI_PUBLIC_API(stack_item*)
Java_java_io_FileOutputStream_write_stub(stack_item* _P_, struct execenv* _EE_) {
	extern void java_io_FileOutputStream_write(void *, long);
	(void) java_io_FileOutputStream_write(_P_[0].p,((_P_[1].i)));
	return _P_;
}

/* SYMBOL: "java/io/FileOutputStream/writeBytes([BII)V", Java_java_io_FileOutputStream_writeBytes_stub */
JRI_PUBLIC_API(stack_item*)
Java_java_io_FileOutputStream_writeBytes_stub(stack_item* _P_, struct execenv* _EE_) {
	extern void java_io_FileOutputStream_writeBytes(void *, void *, long, long);
	(void) java_io_FileOutputStream_writeBytes(_P_[0].p,((_P_[1].p)),((_P_[2].i)),((_P_[3].i)));
	return _P_;
}

/* SYMBOL: "java/io/FileOutputStream/close()V", Java_java_io_FileOutputStream_close_stub */
JRI_PUBLIC_API(stack_item*)
Java_java_io_FileOutputStream_close_stub(stack_item* _P_, struct execenv* _EE_) {
	extern void java_io_FileOutputStream_close(void *);
	(void) java_io_FileOutputStream_close(_P_[0].p);
	return _P_;
}

