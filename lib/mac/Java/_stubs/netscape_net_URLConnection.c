/* DO NOT EDIT THIS FILE - it is machine generated */
#include "StubPreamble.h"
/* Stubs for class netscape/net/URLConnection */

/* SYMBOL: "netscape/net/URLConnection/pCreate(Ljava/lang/String;Ljava/lang/String;)V", Java_netscape_net_URLConnection_pCreate_stub */
JRI_PUBLIC_API(stack_item*)
Java_netscape_net_URLConnection_pCreate_stub(stack_item* _P_, struct execenv* _EE_) {
	extern void netscape_net_URLConnection_pCreate(void *, void *, void *);
	(void) netscape_net_URLConnection_pCreate(_P_[0].p,((_P_[1].p)),((_P_[2].p)));
	return _P_;
}

/* SYMBOL: "netscape/net/URLConnection/finalize()V", Java_netscape_net_URLConnection_finalize_stub */
JRI_PUBLIC_API(stack_item*)
Java_netscape_net_URLConnection_finalize_stub(stack_item* _P_, struct execenv* _EE_) {
	extern void netscape_net_URLConnection_finalize(void *);
	(void) netscape_net_URLConnection_finalize(_P_[0].p);
	return _P_;
}

/* SYMBOL: "netscape/net/URLConnection/getContentLength0()I", Java_netscape_net_URLConnection_getContentLength0_stub */
JRI_PUBLIC_API(stack_item*)
Java_netscape_net_URLConnection_getContentLength0_stub(stack_item* _P_, struct execenv* _EE_) {
	extern long netscape_net_URLConnection_getContentLength0(void *);
	_P_[0].i = netscape_net_URLConnection_getContentLength0(_P_[0].p);
	return _P_ + 1;
}

/* SYMBOL: "netscape/net/URLConnection/getContentType0()Ljava/lang/String;", Java_netscape_net_URLConnection_getContentType0_stub */
JRI_PUBLIC_API(stack_item*)
Java_netscape_net_URLConnection_getContentType0_stub(stack_item* _P_, struct execenv* _EE_) {
	extern void* netscape_net_URLConnection_getContentType0(void *);
	_P_[0].p = netscape_net_URLConnection_getContentType0(_P_[0].p);
	return _P_ + 1;
}

/* SYMBOL: "netscape/net/URLConnection/getHeaderField0(Ljava/lang/String;)Ljava/lang/String;", Java_netscape_net_URLConnection_getHeaderField0_stub */
JRI_PUBLIC_API(stack_item*)
Java_netscape_net_URLConnection_getHeaderField0_stub(stack_item* _P_, struct execenv* _EE_) {
	extern void* netscape_net_URLConnection_getHeaderField0(void *, void *);
	_P_[0].p = netscape_net_URLConnection_getHeaderField0(_P_[0].p,((_P_[1].p)));
	return _P_ + 1;
}

/* SYMBOL: "netscape/net/URLConnection/getHeaderFieldKey0(I)Ljava/lang/String;", Java_netscape_net_URLConnection_getHeaderFieldKey0_stub */
JRI_PUBLIC_API(stack_item*)
Java_netscape_net_URLConnection_getHeaderFieldKey0_stub(stack_item* _P_, struct execenv* _EE_) {
	extern void* netscape_net_URLConnection_getHeaderFieldKey0(void *, long);
	_P_[0].p = netscape_net_URLConnection_getHeaderFieldKey0(_P_[0].p,((_P_[1].i)));
	return _P_ + 1;
}

/* SYMBOL: "netscape/net/URLConnection/close()V", Java_netscape_net_URLConnection_close_stub */
JRI_PUBLIC_API(stack_item*)
Java_netscape_net_URLConnection_close_stub(stack_item* _P_, struct execenv* _EE_) {
	extern void netscape_net_URLConnection_close(void *);
	(void) netscape_net_URLConnection_close(_P_[0].p);
	return _P_;
}

