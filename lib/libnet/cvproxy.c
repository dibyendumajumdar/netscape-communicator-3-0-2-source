#include "mkutils.h"
#include "mkstream.h"
#include "mkgeturl.h"
#include "xp.h"

typedef struct _ProxyObj {
    Bool  past_first_line;
	Bool  definately_send_headers;
	char    *content_type;
	char    *content_encoding;
} ProxyObj;

PRIVATE int net_proxy_write (ProxyObj * obj, CONST char* s, int32 len)
{
    /* send HTTP headers if not already getting an HTTP doc
     */
    if(!obj->past_first_line)
      {
		if(obj->definately_send_headers || strncmp(s, "HTTP/", len >= 5 ? 5 : len))
		  {
            write(1, "HTTP/1.0 200 OK\r\n",17);
            write(1, "Content-type: ",14);
            write(1, obj->content_type, XP_STRLEN(obj->content_type));
            if(obj->content_encoding)
              {
                write(1, "\r\nContent-encoding: ",18);
                write(1, obj->content_encoding, XP_STRLEN(obj->content_encoding));
              }
            write(1, "\r\nServer: MKLib proxy agent\r\n",29);
            write(1, "\r\n", 2);  /* finish it */
		  }

		obj->past_first_line = TRUE;
      }

    write(1, s, len); 
    return(1);
}

PRIVATE unsigned int net_proxy_WriteReady (void * obj)
{
   return(MAX_WRITE_READY);  /* always ready for writing */
}


PRIVATE void net_proxy_complete (ProxyObj * obj)
{
	FREEIF(obj->content_type);
	FREEIF(obj->content_encoding);
	FREE(obj);
    return;
}

PRIVATE void net_proxy_abort (ProxyObj * obj, int status)
{
	FREEIF(obj->content_type);
	FREEIF(obj->content_encoding);
	FREE(obj);
    return;
}



PUBLIC NET_StreamClass * 
NET_ProxyConverter(int         format_out,
                   void       *data_obj,
                   URL_Struct *URL_s,
                   MWContext  *window_id)
{
    NET_StreamClass* stream;
    ProxyObj * obj;
    
    TRACEMSG(("Setting up display stream. Have URL: %s \n%s\n", 
									URL_s->address, URL_s->content_type));

    stream = XP_NEW(NET_StreamClass);
    if(stream == NULL) 
        return(NULL);

	obj = XP_NEW(ProxyObj);
    if(obj == NULL) 
	  {
		FREE(stream);
        return(NULL);
	  }

	XP_MEMSET(obj, 0, sizeof(ProxyObj));

	stream->data_object = obj;
	

    stream->name           = "ProxyWriter";
    stream->complete       = (MKStreamCompleteFunc) net_proxy_complete;
    stream->abort          = (MKStreamAbortFunc) net_proxy_abort;
    stream->put_block      = (MKStreamWriteFunc) net_proxy_write;
    stream->is_write_ready = (MKStreamWriteReadyFunc) net_proxy_WriteReady;
    stream->window_id      = window_id;

    TRACEMSG(("Returning stream from display_converter\n"));

    /* send HTTP headers if not already getting an HTTP doc 
	 */
    if(strncasecomp(URL_s->address,"http:",5))
      {
		obj->definately_send_headers = TRUE;
      }
	StrAllocCopy(obj->content_type, URL_s->content_type);
	StrAllocCopy(obj->content_encoding, URL_s->content_encoding);

    return stream;
}
