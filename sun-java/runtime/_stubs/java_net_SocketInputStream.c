/* DO NOT EDIT THIS FILE - it is machine generated */
#include "StubPreamble.h"
/* Stubs for class java/net/SocketInputStream */

/* SYMBOL: "java/net/SocketInputStream/socketRead([BII)I", Java_java_net_SocketInputStream_socketRead_stub */
JRI_PUBLIC_API(stack_item*)
Java_java_net_SocketInputStream_socketRead_stub(stack_item* _P_, struct execenv* _EE_) {
	extern long java_net_SocketInputStream_socketRead(void *, void *, long, long);
	_P_[0].i = java_net_SocketInputStream_socketRead(_P_[0].p,((_P_[1].p)),((_P_[2].i)),((_P_[3].i)));
	return _P_ + 1;
}

