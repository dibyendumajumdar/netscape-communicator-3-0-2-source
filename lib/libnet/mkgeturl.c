/* -*- Mode: C; tab-width: 4 -*-
 */
#include "mkutils.h"
#include "mkgeturl.h"
#include "shist.h"
#include "mkparse.h"
#include "mkformat.h"

#include "mkhttp.h"
#include "mkftp.h"
#include "mkgopher.h"
#include "mkremote.h"
#include "mknews.h"
#include "mkpop3.h"
#include "mkfile.h"
#include "mksmtp.h"
#include "mkcache.h"
#include "mkmemcac.h"
#include "glhist.h"
#include "mkaccess.h"
#include "mkmailbx.h"
#include "mkautocf.h"
#include "ssl.h"

#include "mktcp.h"  /* for NET_InGetHostByName semaphore */
#include "cert.h"
#ifdef MOZILLA_CLIENT
#include "secnav.h"
#ifdef MOCHA
#include "libmocha.h"
#endif
#endif
#include "il.h"
#include "libi18n.h"
#include "xp_sec.h"
#include "mkmocha.h"
#include "htmldlgs.h"

#include "np.h"
#include "prefapi.h"

#include "sslerr.h"
#include "xp_error.h"
#include "merrors.h"

#ifdef EDITOR
#include "edt.h"
#endif

#ifdef MOCHA
#include "libmocha.h"
#endif

#ifdef XP_UNIX
#include "prnetdb.h"
#else
#define PRHostEnt struct hostent
#endif

#include "xplocale.h"

/* for XP_GetString() */
#include "xpgetstr.h"
extern int MK_CONNECTION_REFUSED;
extern int MK_CONNECTION_TIMED_OUT;
extern int MK_MALFORMED_URL_ERROR;
extern int MK_NO_WAIS_PROXY;
extern int MK_OUT_OF_MEMORY;
extern int MK_TIMEBOMB_MESSAGE;
extern int MK_TIMEBOMB_URL_PROHIBIT;
extern int MK_TIMEBOMB_WARNING_MESSAGE;
extern int MK_UNABLE_TO_CONNECT;
extern int MK_UNABLE_TO_CREATE_SOCKET;
extern int MK_UNABLE_TO_LOCATE_HOST;
extern int MK_UNABLE_TO_LOCATE_PROXY;
#ifndef NO_SSL /* added by jwz */
extern int SSL_ERROR_BAD_CERTIFICATE;
#endif /* NO_SSL -- added by jwz */
extern int XP_ALERT_CONNECTION_LESSZERO;
extern int XP_ALERT_INTERRUPT_WINDOW;
extern int XP_ALERT_URLS_LESSZERO;
extern int XP_ALERT_URN_USEHTTP;
extern int XP_CONFIRM_REPOST_FORMDATA;
extern int XP_CONSULT_SYS_ADMIN;
extern int XP_HTML_MISSING_REPLYDATA;
extern int XP_SOCKS_NS_ENV_VAR;
extern int XP_SOME_HOSTS_UNREACHABLE;
extern int XP_THIS_IS_DNS_VERSION;
extern int XP_THIS_IS_YP_VERSION;
extern int XP_UNKNOWN_HOST;
extern int XP_UNKNOWN_HOSTS;
extern int XP_UNKNOWN_HTTP_PROXY;
extern int XP_UNKNOWN_SOCKS_HOST;
extern int XP_URLS_WAITING_FOR_AN_OPEN_SOCKET;
extern int XP_URLS_WAITING_FOR_FEWER_ACTIVE_URLS;
extern int XP_CONNECTIONS_OPEN;
extern int XP_ACTIVE_URLS;
extern int XP_USING_PREVIOUSLY_CACHED_COPY_INSTEAD;
extern int XP_BAD_AUTOCONFIG_NO_FAILOVER;

extern int XP_SOCK_CON_SOCK_PROTOCOL;			
extern int XP_URL_NOT_FOUND_IN_CACHE;			
extern int XP_PARTIAL_CACHE_ENTRY;
extern int XP_CHECKING_SERVER__FORCE_RELOAD;
extern int XP_OBJECT_HAS_EXPIRED;
extern int XP_CHECKING_SERVER_CACHE_ENTRY;
extern int XP_CHECKING_SERVER__LASTMOD_MISS;
extern int XP_NETSITE_	;
extern int XP_LOCATION_	;
extern int XP_FILE_MIME_TYPE_	;
extern int XP_CURRENTLY_UNKNOWN;
extern int XP_SOURCE_ ;
extern int XP_CURRENTLY_IN_DISK_CACHE	;
extern int XP_CURRENTLY_IN_MEM_CACHE; 
extern int XP_CURRENTLY_NO_CACHE	;
extern int XP_LOCAL_CACHE_FILE_ ;
extern int XP_NONE	;
extern int XP_LOCAL_TIME_FMT ;
extern int XP_LAST_MODIFIED ;
extern int XP_GMT_TIME_FMT	;
extern int XP_CONTENT_LENGTH_	;
extern int XP_NO_DATE_GIVEN;
extern int XP_EXPIRES_	;
extern int XP_MAC_TYPE_	;
extern int XP_MAC_CREATOR_	;
extern int XP_CHARSET_	;
extern int XP_STATUS_UNKNOWN ;
extern int XP_SECURITY_ ;
extern int XP_CERTIFICATE_	;
extern int XP_UNTITLED_DOCUMENT ;
extern int XP_HAS_THE_FOLLOWING_STRUCT	;
extern int XP_DOCUMENT_INFO ;
extern int XP_THE_WINDOW_IS_NOW_INACTIVE ;
extern int XP_MSG_UNKNOWN ;
extern int XP_EDIT_NEW_DOC_NAME;
extern int XP_CONFIRM_MAILTO_POST_1;
extern int XP_CONFIRM_MAILTO_POST_2;
extern int XP_CONFIRM_MAILTO_POST_3;

#if defined(JAVA)
#include "nslocks.h"
#include "prlog.h"
#include <stdarg.h>


PRMonitor* libnet_asyncIO;

#if defined(SOLARIS)
extern int gethostname(char *, int);
#endif /* SOLARIS */

/*
** Map netlib trace messages into nspr logger so that they are
** thread safe.
*/
#if defined(DEBUG)

PR_LOG_DEFINE(NETLIB);

PRIVATE
void net_Trace(char *msg)
{
#ifdef XP_WIN32
	if(MKLib_trace_flag)
		XP_Trace(msg);
#endif
	PR_LOG(NETLIB, out, ("libnet: %s", msg));
}

void NET_NTrace(char *msg, int32 length)
{
	char * new_string = XP_ALLOC(length+1);

	if(!new_string)
		return;

	strncpy(new_string, msg, length);
	new_string[length] = '\0';

	net_Trace(new_string);

	FREE(new_string);
}

void _MK_TraceMsg(char *fmt, ...)
{
	va_list ap;
	char buf[200];

	va_start(ap, fmt);
	PR_vsnprintf(buf, sizeof(buf), fmt, ap);

	PR_LOG(NETLIB, out, ("libnet: %s", buf));
}
#endif /* DEBUG */

#endif /* JAVA */

#define LIBNET_UNLOCK_AND_RETURN(value) \
	do { int rv = (value); LIBNET_UNLOCK(); return rv; } while (0)

/*
** Don't ever forget about this!!!
**
** Define TIMEBOMB_ON for beta builds.
** Undef TIMEBOMB_ON for release builds.
*/
/* #define TIMEBOMB_ON */
#undef TIMEBOMB_ON
/*
** After this date all hell breaks loose
*/
#define TIME_BOMB_TIME		848131203	/* 11/16/96 + 3 secs */
#define TIME_BOMB_WARNING_TIME	846835203	/* 11/01/96 + 3 secs */

#ifndef XP_UNIX
#ifdef NADA_VERSION
static char *https_security = "SECURITY_VERSION: -https";
#else
static char *https_security = "SECURITY_VERSION: +https";
#endif
#endif

#ifdef EDITOR
/* 
 * Strings used for new, empty document creation
 * Kluge we use to load a new document is resource with one space.
 * These strings are loaded from XP_MSG.I on first GetURL call
 * (this allows changing "Untitled" for other languages)
*/
PUBLIC char *XP_NEW_DOC_URL = NULL; /* "about:editfilenew" */
/* 
 * This is the name we want to appear as the URL address
 * You can load a new doc using this - it will be changed 
 *  to XP_NEW_DOC_URL. 
 * When done, XP_NEW_DOC_URL will be changed to XP_NEW_DOC_NAME
*/
PUBLIC char *XP_NEW_DOC_NAME = NULL; /* "Untitled" */
#endif

#ifdef PROFILE
#pragma profile on
#endif

PRIVATE void add_slash_to_URL (URL_Struct *URL_s);
PRIVATE Bool override_proxy (CONST char * URL);
PRIVATE int net_output_about_url(ActiveEntry * this_entry);
PRIVATE int net_output_security_url(ActiveEntry * this_entry, MWContext *cx);
PRIVATE Bool net_about_kludge(URL_Struct *URL_s);
PRIVATE void net_FreeURLAllHeaders(URL_Struct * URL_s);

PRIVATE NET_TimeBombActive = FALSE;

typedef struct _WaitingURLStruct {
	int                 type;
	URL_Struct 		   *URL_s;
	FO_Present_Types    format_out;
    MWContext          *window_id;
    Net_GetUrlExitFunc *exit_routine;
} WaitingURLStruct;

/* Pointers to proxy servers
 * They are pointers to a host if
 * a proxy environment variable
 * was found or they are zero if not.
 */
PRIVATE char * MKftp_proxy=0;
PRIVATE char * MKgopher_proxy=0;
PRIVATE char * MKhttp_proxy=0;
PRIVATE char * MKhttps_proxy=0;
PRIVATE char * MKwais_proxy=0;
PRIVATE char * MKnews_proxy=0;
PRIVATE char * MKno_proxy=0;
PRIVATE char * MKproxy_ac_url=0;
PRIVATE NET_ProxyStyle MKproxy_style = PROXY_STYLE_UNSET;

PRIVATE XP_List * net_EntryList=0;

MODULE_PRIVATE CacheUseEnum NET_CacheUseMethod=CU_CHECK_PER_SESSION;

#define MAX_NUMBER_OF_PROCESSING_URLS 15

#define INITIAL_MAX_ALL_HEADERS 10
#define MULT_ALL_HEADER_COUNT   2
#define MAX_ALL_HEADER_COUNT    16000

MODULE_PRIVATE time_t NET_StartupTime=0;

MODULE_PRIVATE XP_Bool NET_ProxyAcLoaded = FALSE;
MODULE_PRIVATE int NET_TotalNumberOfOpenConnections=0;
MODULE_PRIVATE int NET_MaxNumberOfOpenConnections=100;
MODULE_PRIVATE int NET_MaxNumberOfOpenConnectionsPerContext=4;
MODULE_PRIVATE int NET_TotalNumberOfProcessingURLs=0;
PRIVATE XP_List  * net_waiting_for_actives_url_list=0;
PRIVATE XP_List  * net_waiting_for_connection_url_list=0;

typedef struct NETExitCallbackNode {
	struct NETExitCallbackNode *next;
	void *arg;
	void (* func)(void *arg);
} NETExitCallbackNode;

PRIVATE NETExitCallbackNode *NETExitCallbackHead = NULL;

PUBLIC void
NET_AddExitCallback( void (* func)(void *arg), void *arg)
{
	NETExitCallbackNode *node;
	
	/* alloc node */
	node = XP_ALLOC(sizeof(NETExitCallbackNode));
	if ( node != NULL ) {
		/* fill it in */
		node->func = func;
		node->arg = arg;
		/* link it into the list */
		node->next = NETExitCallbackHead;
		NETExitCallbackHead = node;
	}
	
	return;
}

PRIVATE void
NET_ProcessExitCallbacks(void)
{
	NETExitCallbackNode *node, *freenode;

	node = NETExitCallbackHead;
	
	while ( node != NULL ) {
		(* node->func)(node->arg);
		freenode = node;
		node = node->next;
		XP_FREE(freenode);
	}
	
	NETExitCallbackHead = NULL;
	
	return;
}

/* initialize the netlibrary and set the socket buffer size
 */
PUBLIC int
NET_InitNetLib(int socket_buffer_size, int max_number_of_connections)
{
    int status;

	TRACEMSG(("Initializing Network library"));

	NET_StartupTime = time(NULL);

	/* seed the random generator
	 */
	XP_SRANDOM((unsigned int) time(NULL));

	if(max_number_of_connections < 1)
		max_number_of_connections = 1;

#ifdef XP_UNIX
	signal(SIGPIPE, SIG_IGN);
#endif
	

    status = NET_ChangeSocketBufferSize(socket_buffer_size);

	NET_ChangeMaxNumberOfConnections(max_number_of_connections);
	net_waiting_for_actives_url_list = XP_ListNew();
	net_waiting_for_connection_url_list = XP_ListNew();

    net_EntryList = XP_ListNew();

#ifdef MOZILLA_CLIENT
    NET_ReadCacheFAT("", TRUE);

	NET_ReadCookies("");

	GH_InitGlobalHistory();
#endif /* MOZILLA_CLIENT */

	NET_TotalNumberOfProcessingURLs=0; /* reset */

#ifdef XP_UNIX
	{
		char *ac_url = getenv("AUTOCONF_URL");
		if (ac_url) {
			NET_SetProxyServer(PROXY_AUTOCONF_URL, ac_url);
			NET_SelectProxyStyle(PROXY_STYLE_AUTOMATIC);
		}
	}
#endif

#ifdef JAVA
    libnet_asyncIO = PR_NewNamedMonitor(0, "libnet");
#endif

    return(status);
}

/* set the maximum allowable number of open connections at any one
 * time regardless of the context
 */
PUBLIC void
NET_ChangeMaxNumberOfConnections(int max_number_of_connections)
{

	/* if already equal return */
	if(NET_MaxNumberOfOpenConnections == max_number_of_connections)
		return;

	if(max_number_of_connections < 1)
	    max_number_of_connections = 1;

	/* make sure that # conns per context is less than or equal
	 * to this Max
	 */
	if(NET_MaxNumberOfOpenConnectionsPerContext > max_number_of_connections)
	    NET_MaxNumberOfOpenConnectionsPerContext = max_number_of_connections;

	NET_MaxNumberOfOpenConnections = max_number_of_connections;

	/* close any open connections to prevent deadlock
	 */
	NET_CleanupFTP();
#ifdef MOZILLA_CLIENT
	NET_CleanupNews();
#endif /* MOZILLA_CLIENT */
}

/* set the maximum allowable number of open connections at any one
 * time Per context
 */
PUBLIC void
NET_ChangeMaxNumberOfConnectionsPerContext(int max_number_of_connections)
{

	/* if already equal return */
	if(NET_MaxNumberOfOpenConnectionsPerContext == max_number_of_connections)
		return;

	/* gate the number of max connections to be within 1 and 4
	 */
	if(max_number_of_connections < 1)
		max_number_of_connections = 1;

	if(max_number_of_connections > 6)
		max_number_of_connections = 6;

	NET_MaxNumberOfOpenConnectionsPerContext = max_number_of_connections;

	/* close any open connections to prevent deadlock
	 */
	NET_CleanupFTP();
#ifdef MOZILLA_CLIENT
	NET_CleanupNews();
#endif /* MOZILLA_CLIENT */

}

/* check the date and set off the timebomb if necessary
 *
 * Calls FE_Alert with a message and then disables
 * all network calls to hosts not in our domains,
 * except for FTP connects.
 */
PUBLIC Bool
NET_CheckForTimeBomb(MWContext *context)
{
#ifdef TIMEBOMB_ON

	time_t cur_time;
	static Bool done = FALSE;
	time_t timebomb_time = TIME_BOMB_TIME;
#ifdef DEBUG
	time_t timebomb_warning_time = TIME_BOMB_WARNING_TIME;
#endif

	if(done)
		return(NET_TimeBombActive);

	done = TRUE;

	cur_time  = XP_TIME();

	TRACEMSG(("Timebomb time is: %ld %s", timebomb_time,
								ctime(&timebomb_time)));
#ifdef DEBUG
	TRACEMSG(("Timebomb warning time is: %ld %s", timebomb_warning_time,
								ctime(&timebomb_warning_time)));
#endif
	TRACEMSG(("current time is: %ld %s", cur_time,
								ctime(&cur_time)));

    if(cur_time > TIME_BOMB_TIME)
	  {
		char * alert = NET_ExplainErrorDetails(MK_TIMEBOMB_MESSAGE);

		FE_Alert(context, alert);
		FREE(alert);

		NET_TimeBombActive = TRUE;

		return(TRUE);
	  }
    else if(cur_time > TIME_BOMB_WARNING_TIME)
	  {
		char * alert = NET_ExplainErrorDetails( MK_TIMEBOMB_WARNING_MESSAGE,
												INTL_ctime(context, &timebomb_time));
		FE_Alert(context, alert);
		FREE(alert);

		NET_TimeBombActive = FALSE;

		return(FALSE);
	  }
#endif /* TIMEBOMB_ON */

	return(FALSE);
}

/* set the way the cache should be used
 */
PUBLIC void
NET_SetCacheUseMethod(CacheUseEnum method)
{
    NET_CacheUseMethod = method;
}

PRIVATE void
net_CallExitRoutine(Net_GetUrlExitFunc *exit_routine,
					URL_Struct 		   *URL_s,
					int                 status,
					FO_Present_Types	format_out,
					MWContext          *window_id)
{
#ifdef EDITOR
    /* History_entry * his; */
    /* Change all references to "about:editfilenew" into "file:///Untitled" */
    /* Do this for both Browser and Editor windows */
    if( /* EDT_IS_EDITOR(window_id) && */
        URL_s && URL_s->address &&
        0 == XP_STRCMP(URL_s->address, XP_NEW_DOC_URL) )
    {
        XP_FREE(URL_s->address);
        URL_s->address = XP_STRDUP(XP_NEW_DOC_NAME);
       
        /* Not sure if this is the best place to do this,
         * but it needs to go someplace!        
        */
        LO_SetBaseURL(window_id, XP_NEW_DOC_NAME);

        /* Setting context title to NULL will trigger
         * Windows front end to use the URL address and set 
         * doc and window title to "Untitled"
         * Do all front ends do this?
         * If not, they will have to use EDT_IS_NEW_DOCUMENT
         *  to test for new doc and update title (in exit_routine)
        */
        if ( window_id->title ) {
            XP_FREE(window_id->title);
            window_id->title = NULL;
        }
        /* Note: We replace "about:editfilenew" with "file:///Untitled"
         * in SHIST_AddDocument(), so we don't need to mess with
         * History data here.
        */

        /* Set  window_id->is_new_document flag appropriately 
         * This flag allows quicker response for often-called 
         * status queries at front end
        */
        if ( EDT_IS_EDITOR(window_id) ) {
            EDT_NEW_DOCUMENT(window_id, TRUE);
        }
    }
#endif /* EDITOR */
#if defined(XP_WIN) || defined (XP_MAC)
	FE_URLEcho(URL_s, status, window_id);
#endif /* XP_WIN/MAC */

	if (!URL_s->load_background)
	  FE_EnableClicking(window_id);

#ifdef MOZILLA_CLIENT
	if(URL_s->refresh_url && status != MK_INTERRUPTED)
		FE_SetRefreshURLTimer(window_id, URL_s->refresh, URL_s->refresh_url);
#endif /* MOZILLA_CLIENT */

    if (URL_s->pre_exit_fn)
	  {
		Net_GetUrlExitFunc *per = URL_s->pre_exit_fn;
		URL_s->pre_exit_fn = 0;
		(*per) (URL_s, status, window_id);
	  }

    (*exit_routine) (URL_s, status, window_id);

#ifdef MOZILLA_CLIENT
#ifdef EDITOR
#ifdef XP_MAC
    /* If URL is not loaded correctly at startup,
     * there is no history entry to fall back on,
     * so editor doesn't have a document, which is very very bad!
     * Load our "new document" if this happens
    */
    if( EDT_IS_EDITOR(window_id) &&
        status < 0 )
    {
        History_entry * his = SHIST_GetCurrent(&window_id->hist);
        if ( his == NULL || his->address == NULL ) {
            URL_Struct *pNewDocURL = NET_CreateURLStruct (XP_NEW_DOC_URL, NET_NORMAL_RELOAD );
            NET_GetURL (pNewDocURL, FO_CACHE_AND_PRESENT, window_id, exit_routine);
        }
    }
#endif /* XP_MAC */
#endif /* EDITOR */
#endif /* MOZILLA_CLIENT */
}

PRIVATE XP_Bool
net_does_url_require_socket_limit(int url_type)
{
	if( url_type == HTTP_TYPE_URL
        || url_type == SECURE_HTTP_TYPE_URL
        || url_type == FTP_TYPE_URL
        || url_type == NEWS_TYPE_URL
        || url_type == GOPHER_TYPE_URL)
		return TRUE;

	return(FALSE);
}


PRIVATE XP_Bool
net_is_one_url_allowed_to_run(MWContext *context, int url_type)
{
    /* put a limit on the total number of open connections
     */
    if(NET_TotalNumberOfOpenConnections < NET_MaxNumberOfOpenConnectionsPerContext
        || !net_does_url_require_socket_limit(url_type))
      {
		return(TRUE);
	  }
	else if(NET_TotalNumberOfOpenConnections >= NET_MaxNumberOfOpenConnections)
	  {
		return(FALSE);
	  }
	else
	  {
        XP_List * list_ptr;
        ActiveEntry * tmpEntry;
        int32 cur_win_id = FE_GetContextID(context);
        int real_number_of_connections = 0;


		/* run through the list of running URLs and check
		 * for open connections only for this context.
		 * This will allow for the Total number of open
		 * connections to only apply to each window not
		 * to the whole program
		 */
    	list_ptr = net_EntryList;
    	while((tmpEntry = (ActiveEntry *) XP_ListNextObject(list_ptr)) != NULL)
      	  {
        	if(cur_win_id == FE_GetContextID(tmpEntry->window_id)
				&& net_does_url_require_socket_limit(url_type))
			  {
				real_number_of_connections++;
          	  }
      	  } /* end while */

		/* pause it if the number of connections per context
		 * is higher than the per context limit or
		 */
    	if(real_number_of_connections >= NET_MaxNumberOfOpenConnectionsPerContext)

		  {
			return(FALSE);
		  }
      }

	return(TRUE);
}
 
static int
net_push_url_on_wait_queue(int                 url_type,
						   URL_Struct         *URL_s,
						   FO_Present_Types    format_out,
						   MWContext          *context,
						   Net_GetUrlExitFunc  exit_routine)
{

	WaitingURLStruct * wus = XP_NEW(WaitingURLStruct);

	TRACEMSG(("Pushing URL on wait queue with %d open connections",
					 NET_TotalNumberOfOpenConnections ));
	if(!wus)
	  {
		net_CallExitRoutine(exit_routine,
							URL_s,
							MK_OUT_OF_MEMORY,
							format_out,
							context);
		return(MK_OUT_OF_MEMORY);
	  }

	wus->type         = url_type;
	wus->URL_s        = URL_s;
	wus->format_out   = format_out;
	wus->window_id    = context;
	wus->exit_routine = exit_routine;

	/* add "text/ *" to beginning of list so it gets processed first */
	if (CLEAR_CACHE_BIT(format_out) == FO_INTERNAL_IMAGE)
		/* low priority */
		XP_ListAddObjectToEnd(net_waiting_for_connection_url_list, wus);
	else
		/* higher priority */
		XP_ListAddObject(net_waiting_for_connection_url_list, wus);

	return(0);
}

/* returns a malloc'd string that has a bunch of netlib
 * status info in it.
 *
 * please free the pointer when done.
 */
PUBLIC char *
NET_PrintNetlibStatus()
{
	char small_buf[128];
    XP_List * list_ptr;
	ActiveEntry *tmpEntry;
	char *rv=0;

	LIBNET_LOCK();
	list_ptr = net_EntryList;

	sprintf(small_buf, XP_GetString( XP_URLS_WAITING_FOR_AN_OPEN_SOCKET ),
						XP_ListCount(net_waiting_for_connection_url_list),
						NET_MaxNumberOfOpenConnectionsPerContext);
    StrAllocCat(rv, small_buf);

	sprintf(small_buf, XP_GetString( XP_URLS_WAITING_FOR_FEWER_ACTIVE_URLS ),
					XP_ListCount(net_waiting_for_actives_url_list));
    StrAllocCat(rv, small_buf);

	sprintf(small_buf, XP_GetString( XP_CONNECTIONS_OPEN ),
										NET_TotalNumberOfOpenConnections);
    StrAllocCat(rv, small_buf);

	sprintf(small_buf, XP_GetString( XP_ACTIVE_URLS ),
	NET_TotalNumberOfProcessingURLs);
    StrAllocCat(rv, small_buf);

    while((tmpEntry = (ActiveEntry *)XP_ListNextObject(list_ptr)) != 0)
	 {
	   sprintf(small_buf, "------------------------------------\nURL:");
       StrAllocCat(rv, small_buf);
       StrAllocCat(rv, tmpEntry->URL_s->address);
       StrAllocCat(rv, "\n");

	   sprintf(small_buf, XP_GetString(XP_SOCK_CON_SOCK_PROTOCOL),
					tmpEntry->socket, tmpEntry->con_sock, tmpEntry->protocol);
       StrAllocCat(rv, small_buf);
	 }

#if defined(DEBUG) && defined(JAVA)
	{
		static int loggingOn = 0;
		NETLIBLog.level = (loggingOn ? PRLogLevel_none : PRLogLevel_debug);
		NETLIBLog.depth = 0;	/* keep it from auto-initializing */
		loggingOn = !loggingOn;
	}
#endif /*  defined(DEBUG) && defined(JAVA) */

	LIBNET_UNLOCK();
	return(rv);
}

PRIVATE void
net_release_urls_for_processing(XP_Bool release_prefered)
{
	XP_List * list_ptr = net_waiting_for_connection_url_list;
    WaitingURLStruct * wus;

	while((wus = (WaitingURLStruct *)
						XP_ListNextObject(list_ptr)) != NULL)
	  {

		if(!release_prefered
		   || ( (wus->format_out != FO_INTERNAL_IMAGE
			    && wus->format_out != FO_CACHE_AND_INTERNAL_IMAGE)
#ifdef MOZILLA_CLIENT
			    || IL_PreferredStream(wus->URL_s) ) )
#else
		  ) )
#endif /* MOZILLA_CLIENT */
		  {
			XP_ListRemoveObject(net_waiting_for_connection_url_list, wus);
        	NET_GetURL(wus->URL_s,
				   	   wus->format_out,
				   	   wus->window_id,
				   	   wus->exit_routine);
        	FREE(wus);
			break;
		  }
	  }
}

/*
 * Must be called under the cover of the LIBNET_LOCK!
 * 
 * The was_background parameter is used to determine when to call
 * FE_AllConnectionsComplete. It should only be called if we've just
 * completed processing a URL that's *not* a background loading URL.
 * A background URL is one that loads without UI activity e.g. no
 * status bar and thermo activity.
 * This ensures that looping animations don't call 
 * FE_AllConnectionsComplete on every loop cycle.
*/
PRIVATE void
net_CheckForWaitingURL(MWContext * window_id, int protocol, Bool was_background)
{
#ifdef NSPR
	XP_ASSERT(LIBNET_IS_LOCKED());
#endif

	/* decrement here since this function is called
	 * after every exit_routine
	 */
	NET_TotalNumberOfProcessingURLs--;

    if(NET_TotalNumberOfProcessingURLs < 0)
      {
        FE_Alert(window_id, XP_GetString(XP_ALERT_URLS_LESSZERO));
        NET_TotalNumberOfProcessingURLs = 0;
      }

	if(NET_TotalNumberOfOpenConnections < 0)
	  {
		FE_Alert(window_id, XP_GetString(XP_ALERT_CONNECTION_LESSZERO));
	    NET_TotalNumberOfOpenConnections = 0;
	  }

	TRACEMSG(("In: net_CheckForWaitingURL with %d connection waiting URL's, %d active waiting URL's and %d open connections",
	      XP_ListCount(net_waiting_for_connection_url_list),
	      XP_ListCount(net_waiting_for_actives_url_list),
	      NET_TotalNumberOfOpenConnections));

	/* release preferred streams first */
	net_release_urls_for_processing(TRUE);

	/* release non-prefered streams */
	net_release_urls_for_processing(FALSE);


    if(!NET_AreThereActiveConnectionsForWindow(window_id) && 
       !was_background)
	  {
#ifdef MOCHA
		LM_SendLoadEvent(window_id, LM_XFER_DONE);
#endif
		FE_AllConnectionsComplete(window_id);
	  }

}

PRIVATE int
net_AbortWaitingURL(MWContext * window_id, Bool all, XP_List *list)
{
	XP_List * list_ptr = list->next;
	XP_List * prev_list_ptr = list;
	XP_List * tmp_list_ptr;
	WaitingURLStruct * wus;
    int32 cur_win_id;
	int number_killed = 0;

	if(!all && window_id)
		cur_win_id = FE_GetContextID(window_id);

	while(list_ptr)
	  {
		wus = (WaitingURLStruct *)list_ptr->object;

		if(all || (window_id && cur_win_id == FE_GetContextID(wus->window_id)))
		  {
			TRACEMSG(("killing waiting URL"));

			/* call exit routine since we are done */
            net_CallExitRoutine(wus->exit_routine,
								wus->URL_s,
								MK_INTERRUPTED,
								wus->format_out,
								wus->window_id);

			number_killed += 1;

			tmp_list_ptr = list_ptr;
			list_ptr = list_ptr->next;

			/* remove the element from the list
			 *
			 * we have to use the function since it does
			 * funky doubly linked list shit.
			 */
			XP_ListRemoveObject(list, wus);

			FREE(wus);
		  }
		else
		  {
			prev_list_ptr = list_ptr;
			list_ptr = list_ptr->next;
		  }
	  }

	return(number_killed);
}


/* Helper function for NET_SanityCheckDNS() */
PRIVATE Bool
net_IsHostResolvable (CONST char *hostname, MWContext *context)
{
#ifdef XP_UNIX
	int rv;
	PRHostEnt hpbuf;
	char dbbuf[PR_NETDB_BUF_SIZE];
	rv = (PR_gethostbyname(hostname, &hpbuf, dbbuf, sizeof(dbbuf), 0)
		  ? TRUE : FALSE);
	return rv;
#else
	return(FALSE);
#endif
}


#if defined(__sun) && !defined(__svr4__)
/* From xfe/dns-stub.o or xfe/nis-stub.o, depending.  Gag. */
extern int fe_HaveDNS;
#endif


PUBLIC void
NET_SanityCheckDNS (MWContext *context)
{
#ifdef XP_UNIX

  char *proxy = MKhttp_proxy;
  char *socks = NET_SocksHostName;
  char *test_host_1 = "home.netscape.com";
  char *test_host_2 = "home6.netscape.com";
  char *test_host_3 = "internic.net";
  char *message;
#if defined(__sun) && !defined(__svr4__)
  char temp[1000];
#endif

  static Bool done;
  if (done) return;
  done = TRUE;

  message = (char *) XP_ALLOC (3000);
  if (! message)
	return;
  *message = 0;

  if (proxy)
	proxy = XP_STRDUP (proxy);
  if (socks)
	socks = XP_STRDUP (socks);

  /* Strip off everything after last colon. */
  {
    char *s;
    if (proxy && (s = XP_STRRCHR (proxy, ':')))
	  *s = 0;
    if (socks && (s = XP_STRRCHR (socks, ':')))
	  *s = 0;
  }

  if (proxy)
	proxy = XP_StripLine (proxy); /* in case it was "hostname:  80" */
  if (socks)
	socks = XP_StripLine (socks);

  /* If the hosts are specified as IP numbers, don't try and resolve them.
     (Yes, hostnames can't begin with digits.) */
  if (proxy && proxy[0] >= '0' && proxy[0] <= '9')
    XP_FREE (proxy), proxy = 0;
  if (socks && socks[0] >= '0' && socks[0] <= '9')
    XP_FREE (socks), socks = 0;

  if (proxy && *proxy)
    {
      /* If there is an HTTP proxy, then we shouldn't try to resolve any
         other hosts at all, because they might legitimately be unresolvable.
         The HTTP proxy will do all lookups.
       */
      if (!net_IsHostResolvable (proxy, context))
        {
		  sprintf(message, XP_GetString(XP_UNKNOWN_HTTP_PROXY), proxy);
        }
    }
  else
    {
      /* There is not an HTTP proxy specified.  So check the other hosts.
       */

      if (socks && *socks && !net_IsHostResolvable (socks, context))
        {
		  sprintf(message, XP_GetString(XP_UNKNOWN_SOCKS_HOST), socks);
#ifdef XP_UNIX
			XP_STRCAT (message, XP_GetString(XP_SOCKS_NS_ENV_VAR));
                  /* Only Unix has the $SOCKS_NS environment variable. */
#endif /* XP_UNIX */
			XP_STRCAT (message, XP_GetString(XP_CONSULT_SYS_ADMIN));
        }
      else
        {
          /* At this point, we're not using a proxy, and either we're not
             using a SOCKS host or we're using a resolvable SOCKS host.
             So that means that all the usual host names should be resolvable,
             and the world is broken if they're not.
           */
          char *losers [10];
          int loser_count = 0;

#ifdef XP_UNIX
          char local [255], *local2;
          PRHostEnt *hent, hpbuf;
		  char dbbuf[PR_NETDB_BUF_SIZE];

          if (gethostname (local, sizeof (local) - 1))
            local [0] = 0;

          /* gethostname() and gethostbyname() often return different data -
             on many systems, the former is basename, the latter is FQDN. */
          if (local &&
              (hent = PR_gethostbyname (local, &hpbuf, dbbuf, sizeof(dbbuf), 0)) &&
              XP_STRCMP (local, hent->h_name))
            local2 = XP_STRDUP (hent->h_name);
          else
            local2 = 0;

          if (local && *local && !net_IsHostResolvable (local, context))
            losers [loser_count++] = local;
          if (local2 && *local2 && !net_IsHostResolvable (local2, context))
            losers [loser_count++] = local2;
#endif /* XP_UNIX */

          if (!net_IsHostResolvable (test_host_1, context))
            losers [loser_count++] = test_host_1;
          if (!net_IsHostResolvable (test_host_2, context))
            losers [loser_count++] = test_host_2;
          if (!net_IsHostResolvable (test_host_3, context))
            losers [loser_count++] = test_host_3;

          if (loser_count > 0)
            {
              if (loser_count > 1)
                {
                  int i;
                  sprintf(message, XP_GetString(XP_UNKNOWN_HOSTS));
                  for (i = 0; i < loser_count; i++)
                    {
                      XP_STRCAT (message, "                    ");
                      XP_STRCAT (message, losers [i]);
                      XP_STRCAT (message, "\n");
                    }
                }
              else
                {
				  sprintf(message, XP_GetString(XP_UNKNOWN_HOST), losers[0]);
                }
              XP_STRCAT (message, XP_GetString(XP_SOME_HOSTS_UNREACHABLE));
#ifdef XP_UNIX
# if defined(__sun) && !defined(__svr4__)	/* compiled on SunOS 4.1.3 */
			  if (fe_HaveDNS)
				/* Don't talk about $SOCKS_NS in the YP/NIS version. */
# endif
				XP_STRCAT (message, XP_GetString(XP_SOCKS_NS_ENV_VAR));

#if defined(__sun) && !defined(__svr4__)	/* compiled on SunOS 4.1.3 */
			  assert (XP_AppName);
			  if (fe_HaveDNS)
				{
					sprintf(temp, XP_GetString(XP_THIS_IS_DNS_VERSION),
						XP_AppName);
				}
			  else
				{
					sprintf(temp, XP_GetString(XP_THIS_IS_YP_VERSION),
						XP_AppName);
				}
			  XP_STRCAT(message, temp);
# endif /*  SunOS 4.* */
#endif /* XP_UNIX */
              XP_STRCAT (message, XP_GetString(XP_CONSULT_SYS_ADMIN));
            }

#ifdef XP_UNIX
          if (local2) XP_FREE (local2);
#endif /* XP_UNIX */
        }
    }

  if (proxy) XP_FREE (proxy);
  if (socks) XP_FREE (socks);

  if (*message)
    FE_Alert (context, message);
  XP_FREE (message);
#endif /* XP_UNIX full function wrap */
}


/* shutdown the netlibrary, cancel all
 * conections and free all
 * memory
 */
PUBLIC void
NET_ShutdownNetLib(void)
{
    ActiveEntry *tmpEntry;

#ifdef XP_WIN32
	if (libnet_asyncIO) LIBNET_LOCK();
#else
    LIBNET_LOCK();
#endif

    if(net_waiting_for_actives_url_list) {
	    net_AbortWaitingURL(0, TRUE, net_waiting_for_actives_url_list);
    	XP_ListDestroy(net_waiting_for_actives_url_list);
    	net_waiting_for_actives_url_list = 0;
    }

    if(net_waiting_for_connection_url_list) {
    	net_AbortWaitingURL(0, TRUE, net_waiting_for_connection_url_list);
    	XP_ListDestroy(net_waiting_for_connection_url_list);
    	net_waiting_for_connection_url_list = 0;
    }

    /* run through list of
     * connections
     */
    while((tmpEntry = (ActiveEntry *)XP_ListRemoveTopObject(net_EntryList)) != 0)
      {
        switch(tmpEntry->protocol)
          {

#ifdef MOZILLA_CLIENT
        	case MEMORY_CACHE_TYPE_URL:
           		NET_InterruptMemoryCache(tmpEntry);
           		break;

        	case MAILTO_TYPE_URL:
           		NET_InterruptMailto(tmpEntry);
           		break;

        	case POP3_TYPE_URL:
           		NET_InterruptPop3(tmpEntry);
           		break;

			case MAILBOX_TYPE_URL:
				NET_InterruptMailbox(tmpEntry);
				break;
#endif /* MOZILLA_CLIENT */

            case HTTP_TYPE_URL:
            case SECURE_HTTP_TYPE_URL:
                NET_InterruptHTTP(tmpEntry);
                break;

            case GOPHER_TYPE_URL:
                NET_InterruptGopher(tmpEntry);
                break;

            case FTP_TYPE_URL:
                NET_InterruptFTP(tmpEntry);
                break;

#ifdef MOZILLA_CLIENT
            case NEWS_TYPE_URL:
            case INTERNAL_NEWS_TYPE_URL:
                NET_InterruptNews(tmpEntry);
                break;

            case FILE_TYPE_URL:
            case FILE_CACHE_TYPE_URL:
                NET_InterruptFile(tmpEntry);
#endif /* MOZILLA_CLIENT */

          } /* end switch */

         TRACEMSG(("End of transfer, entry (soc=%d, con=%d) being removed from list with %d status: %s",
				   tmpEntry->socket, tmpEntry->con_sock, tmpEntry->status, tmpEntry->URL_s->address));

         /* call exit routine since we know we are done */
         net_CallExitRoutine(tmpEntry->exit_routine,
							 tmpEntry->URL_s,
							 tmpEntry->status,
							 tmpEntry->format_out,
							 tmpEntry->window_id);
         XP_FREE(tmpEntry);  /* free the no longer active entry */
      }

	XP_ListDestroy(net_EntryList);
	net_EntryList = 0;

    /* free any memory in the protocol modules
     */
    NET_CleanupHTTP();
    NET_CleanupGopher();
    NET_CleanupFTP();

#ifdef MOZILLA_CLIENT
    NET_CleanupNews();
    NET_CleanupFile();
    NET_CleanupMailto();

	NET_SaveCookies("");
	GH_SaveGlobalHistory();
#endif /* MOZILLA_CLIENT */

	/* purge existing cache files */

    /* free memory in the tcp routines
     */
    NET_CleanupTCP();

#ifdef MOZILLA_CLIENT
    NET_CleanupCache("");
	NET_SetMemoryCacheSize(0); /* free memory cache */

#endif /* MOZILLA_CLIENT */

#ifdef XP_UNIX
	NET_CleanupFileFormat(NULL);
#else
	NET_CleanupFileFormat();
#endif

#ifdef XP_WIN32
    if (libnet_asyncIO) LIBNET_UNLOCK();
#else
    LIBNET_UNLOCK();
#endif
}

static PRBool warn_on_mailto_post = PR_TRUE;

extern void
NET_WarnOnMailtoPost(PRBool warn)
{
	warn_on_mailto_post = warn;
	return;
}

/* register and begin a transfer.
 *
 * returns negative if the transfer errored or finished during the call
 * otherwise returns 0 or greater.
 *
 * A URL, an output format, a window context pointer, and a callback routine
 * should be passed in.
 */

PUBLIC int
NET_GetURL (URL_Struct *URL_s,
            FO_Present_Types output_format,
            MWContext *window_id,
            Net_GetUrlExitFunc*	exit_routine)
{
    int          status=MK_MALFORMED_URL_ERROR;
    int          pacf_status=TRUE;
    int          type;
    int          cache_method=0;
    ActiveEntry *this_entry=0;  /* a new entry */
	char        *new_address;
	char        *colon;
	int processcallbacks = 0;
	Bool confirm;
	Bool load_background;
	char *confirmstring;
	
	TRACEMSG(("Entering NET_GetURL"));
	LIBNET_LOCK();

	XP_ASSERT (URL_s && URL_s->address);
#ifdef MOZILLA_CLIENT
	if ( URL_s->mailto_post && warn_on_mailto_post) {
		confirmstring = NULL;
		confirm = FALSE;
		
		StrAllocCopy(confirmstring, XP_GetString(XP_CONFIRM_MAILTO_POST_1));
		StrAllocCat(confirmstring, XP_GetString(XP_CONFIRM_MAILTO_POST_2));
		StrAllocCat(confirmstring, XP_GetString(XP_CONFIRM_MAILTO_POST_3));
		
		if ( confirmstring ) {
			confirm = FE_Confirm(window_id, confirmstring);
			XP_FREE(confirmstring);
		}
		
		if ( !confirm ) {
			/* abort url
			 */
			net_CallExitRoutine(exit_routine,
								URL_s,
								MK_INTERRUPTED,
								output_format,
								window_id);
			LIBNET_UNLOCK_AND_RETURN(MK_INTERRUPTED);
		}
	}
#endif /* MOZILLA_CLIENT */	
#ifdef EDITOR
    /* First time here - get our strings out of XP_MSG.I */
    if (XP_NEW_DOC_URL == NULL) {
	
        StrAllocCopy(XP_NEW_DOC_URL, "about:editfilenew");
        /* Added strings to allxpstr.h and sllxpstr.rc, 
           but still get "undefined" WHERE ARE MSG IDs DEFINED? */
        /* StrAllocCopy(XP_NEW_DOC_URL, XP_GetString(XP_EDIT_NEW_DOC_URL) ); */
    }
    if (XP_NEW_DOC_NAME == NULL) {
/*        StrAllocCopy(XP_NEW_DOC_NAME, "file:///Untitled" );  */
         StrAllocCopy(XP_NEW_DOC_NAME, XP_GetString(XP_EDIT_NEW_DOC_NAME) );
    }

    /* Test for "Untitled" URL */
    if( 0 == XP_STRCMP(URL_s->address, XP_NEW_DOC_NAME) ) {
/*        if ( EDT_IS_EDITOR(window_id) ) { */
        /* Change request to load "file:///Untitled" into "about:editfilenew"  */
            XP_FREE(URL_s->address);
            URL_s->address = XP_STRDUP(XP_NEW_DOC_URL);
            /* Set flag so FE can quickly detect new doc */
            window_id->is_new_document = TRUE;
/*        } */
    }
    else if( 0 == XP_STRCMP(URL_s->address, XP_NEW_DOC_URL) ) {
        window_id->is_new_document = TRUE;
    }
#endif
#ifdef MOZILLA_CLIENT
	if (MKproxy_ac_url && !NET_ProxyAcLoaded &&
		(!MKproxy_style || MKproxy_style == PROXY_STYLE_AUTOMATIC) &&
		(!(type = NET_URL_Type(URL_s->address))
		 || type == HTTP_TYPE_URL
		 || type == SECURE_HTTP_TYPE_URL
		 || type == GOPHER_TYPE_URL
		 || type == FTP_TYPE_URL
		 || type == FILE_TYPE_URL
		 || type == WAIS_TYPE_URL
		 || type == URN_TYPE_URL
		 || (type == NEWS_TYPE_URL && !strncasecomp(URL_s->address, "snews:", 6)))) {

		int status;

		NET_ProxyAcLoaded = TRUE;
		status = NET_LoadProxyConfig(MKproxy_ac_url,
									 URL_s, output_format, window_id,
									 exit_routine);
		if (status != -1) {
		
			/*
			 * Proxy autoconf load started -- the originally
			 * requested URL will be loaded by the pre_exit_fn()
			 * in the proxy autoconf URL_Struct.
			 *
			 */
			LIBNET_UNLOCK_AND_RETURN(status);
		}
		/* else no proxy autoconfig -- continue normally */
	}

#endif	/* MOZILLA_CLIENT */

    if(!URL_s || !URL_s->address || !*URL_s->address 
#if defined MOZILLA_CLIENT && defined MOCHA
       || !LM_CheckGetURL(window_id, URL_s)
#endif
       )
      {
        /* we need at least an address
         * if we don't have it exit this routine
         */
        net_CallExitRoutine(exit_routine,
							URL_s,
							MK_MALFORMED_URL_ERROR,
							output_format,
							window_id);
		/* increment since it will get decremented */
	    NET_TotalNumberOfProcessingURLs++;
		net_CheckForWaitingURL(window_id, 0, FALSE);

		LIBNET_UNLOCK_AND_RETURN(MK_MALFORMED_URL_ERROR);
      }

	load_background = URL_s->load_background;

	/* kill leading and trailing spaces in the URL address
	 */
	new_address = XP_StripLine(URL_s->address);
	if(new_address != URL_s->address)
	  {
		XP_MEMMOVE(URL_s->address, new_address, XP_STRLEN(new_address)+1);
	  }

	/* get the protocol type
	 */
    type = NET_URL_Type(URL_s->address);

	if (URL_s->method == URL_HEAD_METHOD &&
		type != HTTP_TYPE_URL && type != SECURE_HTTP_TYPE_URL && type != FILE_TYPE_URL) {
	  /* We can only do HEAD on http connections. */
	  net_CallExitRoutine(exit_routine,
						  URL_s,
						  MK_MALFORMED_URL_ERROR, /* Is this right? ### */
						  output_format,
						  window_id);
	  /* increment since it will get decremented */
	  NET_TotalNumberOfProcessingURLs++;
	  net_CheckForWaitingURL(window_id, 0, load_background);

	  LIBNET_UNLOCK_AND_RETURN(MK_MALFORMED_URL_ERROR);
	}

	TRACEMSG(("Called NET_GetURL with FO: %d URL %.5000s ---------------------------", output_format, URL_s->address));
	TRACEMSG(("with method: %d, and post headers: %s", URL_s->method,
				URL_s->post_headers ? URL_s->post_headers : "none"));

	/* put a limit on the total number of active urls
	 */
    if((NET_TotalNumberOfProcessingURLs >= MAX_NUMBER_OF_PROCESSING_URLS) && 
       ((output_format & FO_ONLY_FROM_CACHE) == 0))
	  {
        LIBNET_UNLOCK_AND_RETURN(net_push_url_on_wait_queue(type,
								   							URL_s,
								   							output_format,
								   							window_id,
								   							exit_routine));
	  }

	/* if we get this far then we should add the URL
	 * to the number of processing URL's
	 */
	NET_TotalNumberOfProcessingURLs++;

	if(type == VIEW_SOURCE_TYPE_URL)
	  {
		/* this is a view-source: URL
		 * strip off the front stuff 
		 */
		char *new_address=0;
		/* the colon is guarenteed to be there */
		StrAllocCopy(new_address, XP_STRCHR(URL_s->address, ':')+1);
		FREE(URL_s->address);
		URL_s->address = new_address;

		type = NET_URL_Type(URL_s->address);

		/* remap the format out for the fo_present type
		 */
		if(CLEAR_CACHE_BIT(output_format) == FO_PRESENT)
			output_format = FO_VIEW_SOURCE;
	  }

#ifdef MOZILLA_CLIENT
	if(
#if defined(XP_WIN) || defined(XP_MAC)
	   FE_UseExternalProtocolModule(window_id, output_format, URL_s, exit_routine) ||
#endif
	   NPL_HandleURL(window_id, output_format, URL_s, exit_routine))
      {
		/* don't call the exit routine since the
		 * External protocol module will call it
		 */
        net_CheckForWaitingURL(window_id, 0, load_background);
 
        LIBNET_UNLOCK_AND_RETURN(-1); /* all done */
      }

	if(NET_TimeBombActive)
	  {
		/* Timebomb has gone off!!
		 *
		 * limit URL's to FTP and anything in our domains
		 */
		if(type != FTP_TYPE_URL
			&& type != ABOUT_TYPE_URL
			 && type != FILE_TYPE_URL
			  && type != MAILBOX_TYPE_URL
			   && type != POP3_TYPE_URL
			    && type != MOCHA_TYPE_URL
		         && !strcasestr(URL_s->address, "mcom.com")
			      &&  !strcasestr(URL_s->address, "netscape.com"))
		  {
			char * alert = NET_ExplainErrorDetails(MK_TIMEBOMB_URL_PROHIBIT);

			FE_Alert(window_id, alert);
			FREE(alert);

        	/* we need at least an address
         	 * if we don't have it exit this routine
             */
        	net_CallExitRoutine(exit_routine,
                            URL_s,
                            MK_TIMEBOMB_URL_PROHIBIT,
							output_format,
                            window_id);

			net_CheckForWaitingURL(window_id, 0, load_background);

        	LIBNET_UNLOCK_AND_RETURN(MK_TIMEBOMB_URL_PROHIBIT);
		  }
	  }

	/* put up security dialog boxes to tell the user
	 * about transitions
	 *
	 * only do this for FO_PRESENT objects and objects
	 * not coming out of history
	 * and also make sure that the object is not being asked
	 * for twice via the "304 use local copy" method
	 */
	if(output_format == FO_CACHE_AND_PRESENT
		&& !URL_s->history_num
		 && !URL_s->use_local_copy)
	  {
		Bool continue_loading_url = TRUE;
		History_entry * h = SHIST_GetCurrent(&window_id->hist);

		/* this is some protection against the "mail document"
		 * feature popping up a dialog warning about an insecure
		 * post.
		 * If the type is MAILTO and the first post header
		 * begins with "Content-type" then it is a post from
		 * inside a form, otherwise it is just a normal MAILTO
		 *
		 * And, of course, anything not a MAILTO link will fall
		 * into the if as well.
		 */
		if(type != MAILTO_TYPE_URL
			|| !URL_s->post_headers
			 || !XP_STRNCMP("Content-type", URL_s->post_headers, 12))
		  {
#ifndef NO_SSL /* added by jwz */
		    if(h && h->security_on)
		      {
				/* if this is not a secure transaction */
			    if(type != SECURE_HTTP_TYPE_URL
					&& !(type == NEWS_TYPE_URL &&
						 toupper(*URL_s->address) == 'S') )
			      {
				    if(URL_s->method == URL_POST_METHOD)
				      {
					    continue_loading_url = (Bool)SECNAV_SecurityDialog(window_id,
									           SD_INSECURE_POST_FROM_SECURE_DOC);
				      }
				    else if(!URL_s->redirecting_url
							 && type != MAILTO_TYPE_URL
								&& type != ABOUT_TYPE_URL
							       && type != MOCHA_TYPE_URL)
				      {
					    /* don't put up in case of redirect or mocha
					     */
					    continue_loading_url = (Bool)SECNAV_SecurityDialog(window_id,
									           SD_LEAVING_SECURE_SPACE);
				      }
			      }
		      }
		    else
#endif /* NO_SSL -- added by jwz */
		      {
				/* put up a dialog for all insecure posts
				 * except news and mail posts
				 */
			    if(URL_s->method == URL_POST_METHOD
					&& type != SECURE_HTTP_TYPE_URL
					 && type != INTERNAL_NEWS_TYPE_URL
					 && type != NEWS_TYPE_URL
					  && type != HTML_DIALOG_HANDLER_TYPE_URL
					   && type != HTML_PANEL_HANDLER_TYPE_URL
					   && type != INTERNAL_SECLIB_TYPE_URL
					    && type != MAILTO_TYPE_URL)
                  {
#ifndef NO_SSL /* added by jwz */
				    continue_loading_url = (Bool)SECNAV_SecurityDialog(window_id,
                                           SD_INSECURE_POST_FROM_INSECURE_DOC);
#else
					continue_loading_url = TRUE; /* #### ???? */
#endif /* NO_SSL -- added by jwz */
                  }
		      }

		    if(!continue_loading_url)
		      {
			    /* abort url
                 */
                net_CallExitRoutine(exit_routine,
									URL_s,
									MK_INTERRUPTED,
									output_format,
									window_id);
                net_CheckForWaitingURL(window_id, 0, load_background);

                LIBNET_UNLOCK_AND_RETURN(MK_INTERRUPTED);
		      }
		  }
      }
#endif /* MOZILLA_CLIENT */

	/*
	 * if the string is not a valid URL we are just going
     * to punt, so we may as well try it as http
     *
	 * if the address has a colon not followed by a digit then
	 * don't do this.  This prevents things like Ftp://test
	 * from getting treated badly
	 */
	if(type==0 && URL_s->address &&
		(!(colon = strrchr(URL_s->address, ':')) || isdigit(*(colon+1))) )
	{
		char *munged;

#define INT_SEARCH_URL "http://cgi.netscape.com/cgi-bin/url_search.cgi?search="
#define INT_SEARCH_URL_TYPE HTTP_TYPE_URL

		if(!(munged = (char*) XP_ALLOC(XP_STRLEN(URL_s->address)+20)))
		{
           net_CallExitRoutine(exit_routine,
								URL_s,
								MK_OUT_OF_MEMORY,
								output_format,
								window_id);
			
			LIBNET_UNLOCK_AND_RETURN(MK_OUT_OF_MEMORY);
		}	

#ifdef URL_SEARCH_FEATURE
		if(XP_STRCHR(URL_s->address, ' '))
		  {
			/* URL contains spaces.  Treat it as search text. */
			char *escaped = NET_Escape(URL_s->address, URL_XPALPHAS);

			if(escaped)
			  {
				FREE(munged);
				munged = PR_smprintf("%s%s", INT_SEARCH_URL, escaped);
				FREE(escaped);
			    type = INT_SEARCH_URL_TYPE;
			  }
		  }
		else
#endif /* URL_SEARCH_FEATURE */
		  {
			if(*URL_s->address == '/')
		  	  {
			    XP_STRCPY(munged, "file:");
			    type = FILE_TYPE_URL;
		      }
		    else if(!strncasecomp(URL_s->address, "ftp", 3))
		      {
			    XP_STRCPY(munged, "ftp://");
			    type = FTP_TYPE_URL;
		      }
		    else if(!strncasecomp(URL_s->address, "gopher", 6))
		      {
			    XP_STRCPY(munged, "gopher://");
			    type = GOPHER_TYPE_URL;
		      }
		    else if(!strncasecomp(URL_s->address, "news", 4)
				    || !strncasecomp(URL_s->address, "nntp", 4))
		      {
			    XP_STRCPY(munged, "news://");
			    type = NEWS_TYPE_URL;
		      }
		    else
		      {
			    XP_STRCPY(munged, "http://");
			    type = HTTP_TYPE_URL;
		      }
    
		    XP_STRCAT(munged, URL_s->address);
		  }

		XP_FREE(URL_s->address);
		URL_s->address = munged;

#ifdef MOZILLA_CLIENT
		/*	Try the external protocol handlers again, since with the
		 *		newly appended protocol, they might work this time.
		 */
		if(
	#if defined(XP_WIN) || defined(XP_MAC)
		   FE_UseExternalProtocolModule(window_id, output_format, URL_s, exit_routine) ||
	#endif
		   NPL_HandleURL(window_id, output_format, URL_s, exit_routine))
	      {
			/* don't call the exit routine since the
			 * External protocol module will call it
			 */
	        net_CheckForWaitingURL(window_id, 0, load_background);
 
	        LIBNET_UNLOCK_AND_RETURN(-1); /* all done */
	      }
#endif /* MOZILLA_CLIENT */
	}

    if(type == HTTP_TYPE_URL || type == FILE_TYPE_URL ||
		type == SECURE_HTTP_TYPE_URL || type == GOPHER_TYPE_URL)
        add_slash_to_URL(URL_s);

#ifdef MOZILLA_CLIENT
	/*
	 * If this is a resize-reload OR a view-source URL, try to use the
	 * URL's wysiwyg: counterpart.
	 */
	URL_s->resize_reload = (URL_s->force_reload == NET_RESIZE_RELOAD);
	if(URL_s->wysiwyg_url &&
	   (URL_s->resize_reload == TRUE ||
		output_format == FO_VIEW_SOURCE))
	  {
		StrAllocCopy(URL_s->address, URL_s->wysiwyg_url);
		type = WYSIWYG_TYPE_URL;
	  }

	/* the FE's were screwing up the use of force_reload
     * to get around a bug in the scroll to named anchor
	 * code.  So I gave them an enum NET_RESIZE_RELOAD
	 * that they could use instead.  NET_RESIZE_RELOAD
	 * should be treated just like NET_DONT_RELOAD within
	 * the netlib
	 */
	if((URL_s->force_reload == NET_RESIZE_RELOAD) ||
       (URL_s->force_reload == NET_CACHE_ONLY_RELOAD))
		URL_s->force_reload = NET_DONT_RELOAD;

    /* check for the url in the cache
     */
    cache_method = NET_FindURLInCache(URL_s, window_id);

	if (!cache_method)
	  {
		/* cache testing stuff */
		if(NET_CacheTraceOn)
		  {
			char *buf = 0;
			StrAllocCopy(buf, XP_GetString(XP_URL_NOT_FOUND_IN_CACHE));
			StrAllocCat(buf, URL_s->address);
			FE_Alert(window_id, buf);
			FREE(buf);
		  }

		/* cache miss: we must clear this flag so scripts rerun */
		URL_s->resize_reload = FALSE;

#ifdef MOCHA /* added by jwz */
		/* if wysiwyg, there must be a cache entry or we retry the real url */
		if(type == WYSIWYG_TYPE_URL)
		  {
			const char *real_url = LM_SkipWysiwygURLPrefix(URL_s->address);

			/* XXX can't use StrAllocCopy because it frees dest first */
			if (real_url && (new_address = XP_STRDUP(real_url)))
			  {
				XP_FREE(URL_s->address);
				URL_s->address = new_address;
				FREE_AND_CLEAR(URL_s->wysiwyg_url);
				LIBNET_UNLOCK_AND_RETURN(
					NET_GetURL(URL_s, output_format, window_id, exit_routine)
				);
			  }
		  }
#endif /* MOCHA -- added by jwz */
	  }

    /* if cache_method is non zero then we have this URL cached.  Use
     * the bogus cache url type
 	 *
	 * This nasty bit of logic figures out if we really
	 * need to reload it or if we can use the cached copy
     */
    else
      {
		if(URL_s->use_local_copy || output_format & FO_ONLY_FROM_CACHE)
		  {
            /* the cached file is valid so use it unilaterally
             */
            type = cache_method;
		  }
		else if(URL_s->real_content_length > URL_s->content_length)
		  {
			/* cache testing stuff */
			if(NET_CacheTraceOn)
	  		  {
				char *buf = 0;
				StrAllocCopy(buf, XP_GetString(XP_PARTIAL_CACHE_ENTRY));
				StrAllocCat(buf, URL_s->address);
				FE_Alert(window_id, buf);
				FREE(buf);
	  		  }

            URL_s->memory_copy = 0;
            cache_method = 0;
            TRACEMSG(("Getting the rest of a partial cache file"));
		  }
        else if (URL_s->force_reload != NET_DONT_RELOAD)
          {
            TRACEMSG(("Force reload flag set.  Checking server to see if modified"));
            /* cache testing stuff */
            if(NET_CacheTraceOn)
              {
                char *buf = 0;
                StrAllocCopy(buf, XP_GetString(XP_CHECKING_SERVER__FORCE_RELOAD));
                StrAllocCat(buf, URL_s->address);
                FE_Alert(window_id, buf);
                FREE(buf);
              }

            /* strip the cache file and
             * memory pointer
             */
	    if (type == WYSIWYG_TYPE_URL)
		type = cache_method;
	    else
	      {
		FREE_AND_CLEAR(URL_s->cache_file);
		URL_s->memory_copy = 0;
		cache_method = 0;
	      }
          }
        else if(URL_s->expires)
          {
            time_t cur_time = time(NULL);

            if(cur_time > URL_s->expires)
              {
                FREE_AND_CLEAR(URL_s->cache_file);
                URL_s->memory_copy = 0;
                URL_s->expires = 0;  /* remove cache reference */
                cache_method = 0;

                /* cache testing stuff */
                if(NET_CacheTraceOn)
                  {
                    char *buf = 0;
                    StrAllocCopy(buf, XP_GetString(XP_OBJECT_HAS_EXPIRED));
                    StrAllocCat(buf, URL_s->address);
                    FE_Alert(window_id, buf);
                    FREE(buf);
                  }
              }
			else
			  {
		    	/* the cached file is valid so use it unilaterally
		     	 */
            	type = cache_method;
			  }
		  }
		else if((NET_CacheUseMethod == CU_NEVER_CHECK || URL_s->history_num)
				  && !URL_s->expires)
		  {
            type = cache_method;
		  }
		else if(NET_CacheUseMethod == CU_CHECK_ALL_THE_TIME
			    && CLEAR_CACHE_BIT(output_format) == FO_PRESENT
				  && (type == HTTP_TYPE_URL || type == SECURE_HTTP_TYPE_URL)
					  && !URL_s->expires)
		  {
			/* cache testing stuff */
			if(NET_CacheTraceOn)
	  		  {
				char *buf = 0;
				StrAllocCopy(buf, XP_GetString(XP_CHECKING_SERVER_CACHE_ENTRY));
				StrAllocCat(buf, URL_s->address);
				FE_Alert(window_id, buf);
				FREE(buf);
	  		  }

			 /* if it's an HTTP URL and its not an Internal image
			  * and it's not being asked for with ONLY FROM CACHE
			  * and it's not coming out of the history
			  * and it doesn't have an expiration date...
			  * FORCE Reload it
			  */
            TRACEMSG(("Non history http file found. Force reloading it "));
            /* strip the cache file and
             * memory pointer
             */
            FREE_AND_CLEAR(URL_s->cache_file);
            URL_s->memory_copy = 0;
            cache_method = 0;

		  }
		else if(!URL_s->last_modified
				&& CLEAR_CACHE_BIT(output_format) == FO_PRESENT
				&& (type == HTTP_TYPE_URL || type == SECURE_HTTP_TYPE_URL))
		  {
			TRACEMSG(("Non history cgi script found (probably)."
					  " Force reloading it "));
			/* cache testing stuff */
			if(NET_CacheTraceOn)
	  		  {
				char *buf = 0;
				StrAllocCopy(buf, XP_GetString(XP_CHECKING_SERVER__LASTMOD_MISS));
				StrAllocCat(buf, URL_s->address);
				FE_Alert(window_id, buf);
				FREE(buf);
	  		  }

            /* strip the cache file and
             * memory pointer
             */
            FREE_AND_CLEAR(URL_s->cache_file);
            URL_s->memory_copy = 0;
           	URL_s->expires = 0;  /* remove cache reference */
            cache_method = 0;
		  }
		else
		  {
		    /* the cached file is valid so use it unilaterally
		     */
            type = cache_method;
		  }
      }
#endif /* MOZILLA_CLIENT */

	/*
 	 * If the object should only come out of the cache
	 * we should abort now if there is not a cache object
	 */
    if(output_format & FO_ONLY_FROM_CACHE)
      {
		TRACEMSG(("object called with ONLY_FROM_CACHE format out"));

		if(!cache_method)
		  {
            net_CallExitRoutine(exit_routine,
								URL_s,
								MK_OBJECT_NOT_IN_CACHE,
								output_format,
								window_id);
			net_CheckForWaitingURL(window_id, type, load_background);

            LIBNET_UNLOCK_AND_RETURN(MK_OBJECT_NOT_IN_CACHE);
		  }
		else
		  {
			/* remove the ONLY_FROM_CACHE designation and replace
		 	 * it with CACHE_AND.. so that we go through the cacheing
			 * module again so that we can memory cache disk objects
			 * and do the right thing in general
			 */
			output_format = CLEAR_PRESENT_BIT(output_format,FO_ONLY_FROM_CACHE);
			output_format = SET_PRESENT_BIT(output_format,FO_CACHE_ONLY);
		  }
      }

    /* put a limit on the total number of open connections
     */
	if(!net_is_one_url_allowed_to_run(window_id, type))
	  {
		NET_TotalNumberOfProcessingURLs--; /* waiting not processing */
        LIBNET_UNLOCK_AND_RETURN(net_push_url_on_wait_queue(type,
								   							URL_s,
								   							output_format,
								   							window_id,
								   							exit_routine));
	  }

    /* start a new entry */
    this_entry = XP_NEW(ActiveEntry);
    if(!this_entry)
	  {
       net_CallExitRoutine(exit_routine,
					URL_s,
					MK_OUT_OF_MEMORY,
					output_format,
					window_id);

        LIBNET_UNLOCK_AND_RETURN(MK_OUT_OF_MEMORY);
	  }

    /* init new entry */
    XP_MEMSET(this_entry, 0, sizeof(ActiveEntry));
    this_entry->URL_s        = URL_s;
    this_entry->socket       = SOCKET_INVALID;
    this_entry->con_sock     = SOCKET_INVALID;
    this_entry->exit_routine = exit_routine;
    this_entry->window_id    = window_id;
	this_entry->protocol     = type;

    /* set the format out for the entry
     */
    this_entry->format_out = output_format;

	/* set it busy */
	this_entry->busy = TRUE;

	/* add it to the processing list now
	 */
    XP_ListAddObjectToEnd(net_EntryList, this_entry);

	/* this will protect against multiple posts unknown to the
	 * user
 	 *
	 * check if the method is post and it came from the
	 * history and it isn't coming from the cache
	 */
	if(URL_s->method == URL_POST_METHOD
		&& !URL_s->expires
		 && !cache_method
		  && URL_s->history_num)
	  {
		if(URL_s->force_reload != NET_DONT_RELOAD)
	 	  {
			if(!FE_Confirm(window_id, XP_GetString(XP_CONFIRM_REPOST_FORMDATA)))
			  {
    			XP_ListRemoveObject(net_EntryList, this_entry);
                net_CallExitRoutine(exit_routine,
									URL_s,
									MK_INTERRUPTED,
									output_format,
									window_id);
				/* will get decremented */
                net_CheckForWaitingURL(window_id, this_entry->protocol, 
									   load_background);

        		XP_FREE(this_entry);  /* not needed any more */

                LIBNET_UNLOCK_AND_RETURN(MK_INTERRUPTED);
			  }
			/* otherwise fall through and repost
			 */
		  }
		else
		  {
			NET_StreamClass * stream;
			char buffer[1000];

			StrAllocCopy(URL_s->content_type, TEXT_HTML);
 			stream =  NET_StreamBuilder(CLEAR_CACHE_BIT(output_format),
										URL_s,
										window_id);
            if(stream)
              {
                XP_STRCPY(buffer, XP_GetString(XP_HTML_MISSING_REPLYDATA));
                (*stream->put_block)(stream->data_object,
								     buffer,
								     XP_STRLEN(buffer));
				(*stream->complete)(stream->data_object);
              }

    		XP_ListRemoveObject(net_EntryList, this_entry);

			net_CallExitRoutine(exit_routine,
								URL_s,
								MK_INTERRUPTED,
								output_format,
								window_id);
            net_CheckForWaitingURL(window_id, this_entry->protocol,
								   load_background);

        	XP_FREE(this_entry);  /* not needed any more */

            LIBNET_UNLOCK_AND_RETURN(MK_INTERRUPTED);

		  }
	  }

redo_load_switch:   /* come here on file/ftp retry */

  ABOUT_KLUDGE:

#ifdef MOZILLA_CLIENT
        pacf_status = TRUE;
	if ((NET_ProxyAcLoaded) &&
		(this_entry->protocol == HTTP_TYPE_URL
		 || this_entry->protocol == SECURE_HTTP_TYPE_URL
		 || this_entry->protocol == GOPHER_TYPE_URL
		 || this_entry->protocol == FTP_TYPE_URL
		 || this_entry->protocol == WAIS_TYPE_URL
		 || this_entry->protocol == URN_TYPE_URL
		 || (this_entry->protocol == NEWS_TYPE_URL 
			 && !strncasecomp(URL_s->address, "snews:", 6)
			)
		)
		&& (this_entry->proxy_conf =
		     pacf_find_proxies_for_url(window_id, URL_s->address)) &&
		(pacf_status = pacf_get_proxy_addr(window_id,
							 this_entry->proxy_conf,
							 &this_entry->proxy_addr,
							 &this_entry->socks_host,
							 &this_entry->socks_port)) &&
		(this_entry->proxy_addr))
	  {
		  /* Secure protocols need to be kept in their own protocol
		   * to make SSL tunneling happen.
		   */
		  if (this_entry->protocol != SECURE_HTTP_TYPE_URL &&
			  this_entry->protocol != NEWS_TYPE_URL)
			{
				this_entry->protocol = HTTP_TYPE_URL;
			}

		  /* Everything else except SNEWS gets loaded by HTTP loader,
		   * including HTTPS.
		   */
		  if (this_entry->protocol != NEWS_TYPE_URL)
			{
				status = NET_HTTPLoad(this_entry);
			}
		  else
			{
				status = NET_NewsLoad(this_entry, this_entry->proxy_addr);
			}
	  }
    else 
      if ( pacf_status == FALSE && NET_GetNoProxyFailover() == TRUE ) {
        status = MK_UNABLE_TO_LOCATE_PROXY;
        FE_Alert(window_id, XP_GetString(XP_BAD_AUTOCONFIG_NO_FAILOVER));
      } else
#endif	/* ! MOZILLA_CLIENT */
	  switch(type) {

#ifdef MOZILLA_CLIENT
		case MAILBOX_TYPE_URL:
            status = NET_MailboxLoad(this_entry);
            break;

        case MEMORY_CACHE_TYPE_URL:
            status = NET_MemoryCacheLoad(this_entry);
            break;

#ifdef MOCHA
        case MOCHA_TYPE_URL:
			if (URL_s)
				StrAllocCopy(URL_s->charset, INTL_ResourceCharSet());
			status = net_OpenMochaURL(this_entry);
            break;
#endif /* MOCHA */
#endif /* MOZILLA_CLIENT */

        case FILE_TYPE_URL:
        case FILE_CACHE_TYPE_URL:
            status = NET_FileLoad(this_entry);
            break;

        case FTP_TYPE_URL:
            if(MKftp_proxy && !override_proxy(this_entry->URL_s->address))
			  {
				this_entry->protocol = HTTP_TYPE_URL;
				this_entry->proxy_addr = MKftp_proxy;
                status = NET_HTTPLoad(this_entry);
			  }
            else
			  {
                status = NET_FTPLoad(this_entry);
			  }
            break;

        case SECURE_HTTP_TYPE_URL:
#ifdef NADA_VERSION
			/* change the url to point to our page that
			 * says security doesn't work in this version
			 */
			this_entry->protocol = HTTP_TYPE_URL;
			StrAllocCopy(this_entry->URL_s->address,
							"http://home.netscape.com/home/no-https.html");
#endif
            if(MKhttps_proxy && !override_proxy(this_entry->URL_s->address))
			  {
				this_entry->proxy_addr = MKhttps_proxy;
				status = NET_HTTPLoad(this_entry);
			  }
			else
			  {
				status = NET_HTTPLoad(this_entry);
			  }
			break;

        case HTTP_TYPE_URL:
            if(MKhttp_proxy && !override_proxy(this_entry->URL_s->address))
			  {
				this_entry->protocol = HTTP_TYPE_URL;
				this_entry->proxy_addr = MKhttp_proxy;
                status = NET_HTTPLoad(this_entry);
	          }
            else
			  {
                status = NET_HTTPLoad(this_entry);
			  }
            break;

	    case URN_TYPE_URL:
            if(MKhttp_proxy)
			  {
				this_entry->protocol = HTTP_TYPE_URL;
				this_entry->proxy_addr = MKhttp_proxy;
                status = NET_HTTPLoad(this_entry);
			  }
            else
			  {
                char buffer[256];
                XP_STRCPY(buffer, XP_GetString(XP_ALERT_URN_USEHTTP));
                XP_STRNCAT(buffer, this_entry->URL_s->address, 150);
				buffer[255] = '\0'; /* in case strncat doesn't add one */
                FE_Alert(window_id, buffer);
 			  }
            break;

        case GOPHER_TYPE_URL:
            if(MKgopher_proxy && !override_proxy(this_entry->URL_s->address))
			  {
				this_entry->protocol = HTTP_TYPE_URL;
				this_entry->proxy_addr = MKgopher_proxy;
                status = NET_HTTPLoad(this_entry);
			  }
            else
			  {
                status = NET_GopherLoad(this_entry);
			  }
            break;

#ifdef MOZILLA_CLIENT
        case MAILTO_TYPE_URL:
            status = NET_MailtoLoad(this_entry);
            break;
    
        case INTERNAL_NEWS_TYPE_URL:
			if(!URL_s->internal_url)
				goto invalid_url;
        case NEWS_TYPE_URL:
            if(MKnews_proxy && !override_proxy(this_entry->URL_s->address))
			  {
				this_entry->protocol = HTTP_TYPE_URL;
				this_entry->proxy_addr = MKnews_proxy;
                status = NET_HTTPLoad(this_entry);
			  }
            else
			  {
                status = NET_NewsLoad(this_entry, MKhttps_proxy);
			  }
            break;
#endif /* MOZILLA_CLIENT */

        case ABOUT_TYPE_URL:
#ifdef MOZILLA_CLIENT
		  /* Force embedded textfile to be ResourceCharset if require translation */
		  if (URL_s && XP_STRCMP(URL_s->address, "about:license") == 0)
    	  	StrAllocCopy(URL_s->charset, INTL_ResourceCharSet());
#endif /* MOZILLA_CLIENT */
		  if (net_about_kludge (URL_s))
			{
			  type = this_entry->protocol = NET_URL_Type (URL_s->address);
			  goto ABOUT_KLUDGE;
			}
		  status = net_output_about_url(this_entry);
		  break;

#ifdef MOZILLA_CLIENT
        case INTERNAL_IMAGE_TYPE_URL:
            break;

        case RLOGIN_TYPE_URL:
        case TELNET_TYPE_URL:
        case TN3270_TYPE_URL:
            status = NET_RemoteHostLoad(this_entry);
            break;
#endif /* MOZILLA_CLIENT */

        case WAIS_TYPE_URL:
            if(MKwais_proxy && !override_proxy(this_entry->URL_s->address))
			  {
				this_entry->protocol = HTTP_TYPE_URL;
				this_entry->proxy_addr = MKwais_proxy;
                status = NET_HTTPLoad(this_entry);
			  }
            else
			  {
				char * alert = NET_ExplainErrorDetails(MK_NO_WAIS_PROXY);
				FE_Alert(window_id, alert);
				FREE(alert);
			  }
            break;

#ifdef MOZILLA_CLIENT
        case POP3_TYPE_URL:
			if(URL_s->internal_url)
			  {
            	status = NET_Pop3Load(this_entry);
            	break;
			  }
			else
			  {
				goto invalid_url;
			  }

#ifdef HAVE_SECURITY /* added by jwz */
        case SECURITY_TYPE_URL:
    	  if (URL_s)
    	  	StrAllocCopy(URL_s->charset, INTL_ResourceCharSet());
		  status = net_output_security_url(this_entry, window_id);
		  if (this_entry->status == MK_MALFORMED_URL_ERROR)
			/* uhh, is this right?  Should we go there whenever negative? */
			goto invalid_url;
		  break;
#endif /* !HAVE_SECURITY -- added by jwz */

#ifdef HAVE_SECURITY /* added by jwz */
        case HTML_DIALOG_HANDLER_TYPE_URL:
    	  if (URL_s)
    	  	StrAllocCopy(URL_s->charset, INTL_ResourceCharSet());
		  XP_HandleHTMLDialog(this_entry->URL_s);
		  processcallbacks = 1;
		  status = -1; /* XXX - why?? */
			
		  break;

        case HTML_PANEL_HANDLER_TYPE_URL:
    	  if (URL_s)
    	  	StrAllocCopy(URL_s->charset, INTL_ResourceCharSet());
		  XP_HandleHTMLPanel(this_entry->URL_s);
		  processcallbacks = 1;
		  status = -1; /* XXX - why?? */
			
		  break;
#endif /* !HAVE_SECURITY -- added by jwz */


#ifndef NO_SSL /* added by jwz */
        case INTERNAL_SECLIB_TYPE_URL:
    	  if (URL_s)
    	  	StrAllocCopy(URL_s->charset, INTL_ResourceCharSet());
		  SECNAV_HandleInternalSecURL(this_entry->URL_s, window_id);
		  processcallbacks = 1;
		  status = -1; /* XXX - why?? */
			
		  break;
#endif /* NO_SSL -- added by jwz */

#endif /* MOZILLA_CLIENT */

#ifdef MOCHA /* added by jwz */
		case WYSIWYG_TYPE_URL:
		  {
			const char *real_url = LM_SkipWysiwygURLPrefix(URL_s->address);

			/* XXX can't use StrAllocCopy because it frees dest first */
			if (real_url && (new_address = XP_STRDUP(real_url)) != NULL)
			  {
				XP_FREE(URL_s->address);
				URL_s->address = new_address;
				FREE_AND_CLEAR(URL_s->wysiwyg_url);
				XP_ListRemoveObject(net_EntryList, this_entry);
				XP_FREE(this_entry);
				LIBNET_UNLOCK_AND_RETURN(
					NET_GetURL(URL_s, output_format, window_id, exit_routine)
			    );
			  }
		  }
#endif /* MOCHA -- added by jwz */
		  /* FALL THROUGH */

        default:
invalid_url:  /* goto jump point */
            {
				URL_s->error_msg =
								NET_ExplainErrorDetails(MK_MALFORMED_URL_ERROR,
												   this_entry->URL_s->address);
				status = MK_MALFORMED_URL_ERROR;
				this_entry->status = MK_MALFORMED_URL_ERROR;
            }
        }

	/* the entry is no longer busy
	 */
	this_entry->busy = FALSE;

#ifdef MOZILLA_CLIENT
    /* OH NASTY A GOTO!
     * If we went into load file and what we really wanted to do
     * was use FTP, LoadFile will return MK_USE_FTP_INSTEAD so
     * we will go back up to the beginning of the switch and
     * use ftp instead
     */
    if(status == MK_USE_FTP_INSTEAD)
      {
        type = FTP_TYPE_URL;
	    this_entry->protocol = type; /* change protocol designator */
		this_entry->status = 0; /* reset */
        goto redo_load_switch;
      }
 	else 
#endif /* MOZILLA_CLIENT */
	if(this_entry->status == MK_USE_COPY_FROM_CACHE)
	  {
	    TRACEMSG(("304 Not modified recieved using cached entry\n"));
#ifdef MOZILLA_CLIENT
	    NET_RefreshCacheFileExpiration(this_entry->URL_s);
#endif /* MOZILLA_CLIENT */
		/* turn off force reload by telling it to use local copy
		 */
		this_entry->URL_s->use_local_copy = TRUE;

	    /* restart the transfer
	     */
		XP_ListRemoveObject(net_EntryList, this_entry);
	    status = NET_GetURL(this_entry->URL_s,
				   this_entry->format_out,
				   this_entry->window_id,
				   this_entry->exit_routine);

		net_CheckForWaitingURL(this_entry->window_id,
							   this_entry->protocol,
							   this_entry->URL_s->load_background);

		XP_FREE(this_entry);
		LIBNET_UNLOCK_AND_RETURN(0);
	  }
	else
 	if(this_entry->status == MK_DO_REDIRECT)
	  {
		/* this redirect should just call GetURL again
		 */
		int status;
		XP_ListRemoveObject(net_EntryList, this_entry);
		status =NET_GetURL(this_entry->URL_s, 
						   this_entry->format_out,
						   this_entry->window_id,
						   this_entry->exit_routine);
		net_CheckForWaitingURL(this_entry->window_id, 
							   this_entry->protocol,
							   load_background);
        XP_FREE(this_entry);  /* not needed any more */
		LIBNET_UNLOCK_AND_RETURN(0);
	  }
    else if(this_entry->status == MK_TOO_MANY_OPEN_FILES)
      {
        /* Queue this URL so it gets tried again
         */
		XP_ListRemoveObject(net_EntryList, this_entry);
	   	status = net_push_url_on_wait_queue(
									NET_URL_Type(this_entry->URL_s->address),
                           			this_entry->URL_s,
                           			this_entry->format_out,
                           			this_entry->window_id,
                           			this_entry->exit_routine);
        NET_TotalNumberOfProcessingURLs--;
		XP_FREE(this_entry);
		LIBNET_UNLOCK_AND_RETURN(status);
      }

    if(status < 0)  /* check for finished state */
      {
		  TRACEMSG(("End of transfer, entry (soc=%d, con=%d) being removed from list with %d status: %s",
					this_entry->socket, this_entry->con_sock, this_entry->status, this_entry->URL_s->address));

		XP_ListRemoveObject(net_EntryList, this_entry);

        net_CallExitRoutine(this_entry->exit_routine,
							this_entry->URL_s,
							this_entry->status,
							this_entry->format_out,
							this_entry->window_id);
		net_CheckForWaitingURL(this_entry->window_id, 
							   this_entry->protocol,
							   load_background);
        XP_FREE(this_entry);  /* not needed any more */
      }

    TRACEMSG(("Leaving GetURL with %d items in list",
			  XP_ListCount(net_EntryList)));

	/* XXX - hack for chromeless windows - jsw 10/27/95 */
	LIBNET_UNLOCK();
	if ( processcallbacks ) {
		NET_ProcessExitCallbacks();
	}
	
	return status;

#ifdef NOTDEF /* this is the real code here */
    LIBNET_UNLOCK_AND_RETURN(status);
#endif
}

/* process_net is called from the client's main event loop and
 * causes connections to be read and processed.  Multiple
 * connections can be processed simultaneously.
 */
PUBLIC int NET_ProcessNet (NETSOCK ready_fd,  int fd_type)
{
    ActiveEntry * tmpEntry;
    XP_List * list_item;
    int rv;
	Bool load_background;

	TRACEMSG(("Entering ProcessNet!  ready_fd: %d", ready_fd));
	LIBNET_LOCK();

	if(XP_ListIsEmpty(net_EntryList))
	  {
		TRACEMSG(("Invalid call to NET_ProcessNet with fd: %d - No active entries\n", ready_fd));

#ifdef MOZILLA_CLIENT
#ifdef XP_UNIX
# if 0 /* jwz */
		XP_ASSERT(0);
# endif
#endif
		if (ready_fd == -1)
		  {
			MWContext *c = XP_FindContextOfType(0, MWContextBrowser);
			if (!c) c = XP_FindContextOfType(0, MWContextMail);
			if (!c) c = XP_FindContextOfType(0, MWContextNews);
			if (!c) c = XP_FindContextOfType(0, MWContextMessageComposition);
			if (c)
			  FE_ClearCallNetlibAllTheTime(c);
		  }
#endif /* MOZILLA_CLIENT */

        LIBNET_UNLOCK_AND_RETURN(0); /* no entries to process */
	  }

	if(NET_InGetHostByName)
	  {
		TRACEMSG(("call to processnet while doing gethostbyname call"));
		assert(0);
		LIBNET_UNLOCK_AND_RETURN(1);
	  }

    /*
     * if -1 is passed into ProcessNet use select to
     * figure out if a socket is ready
     */
    if(ready_fd == -1)
 	  {
		/* try and find a socket ready for reading
		 */
    	fd_set read_fds;
    	fd_set write_fds;
    	fd_set exps_fds;
		unsigned int fd_set_size=0;
    	struct timeval timeout;

		/* reorder the list so that we get a round robin effect */
		XP_ListMoveTopToBottom(net_EntryList);

    	timeout.tv_sec = 0;    /* polling */
    	timeout.tv_usec = 0;   /* polling */
		XP_MEMSET(&read_fds, 0, sizeof(fd_set));
		XP_MEMSET(&write_fds, 0, sizeof(fd_set));
		XP_MEMSET(&exps_fds, 0, sizeof(fd_set));


		fd_type = NET_SOCKET_FD;

    	/* process one socket ready for reading */
		list_item = net_EntryList;
    	while((tmpEntry = (ActiveEntry *) XP_ListNextObject(list_item)) != 0)
      	  {

			if(tmpEntry->busy)
				continue;

			if(!tmpEntry->local_file && !tmpEntry->memory_file)
			  {
				if (tmpEntry->socket != SOCKET_INVALID)
				  {
					/* check with SSL to see if it already
					 * had data pending.  If it does
					 * then we will bypass select
					 */
#ifndef NO_SSL /* added by jwz */
					if(SSL_DataPending(tmpEntry->socket))
					  {
						fd_type = NET_SOCKET_FD;
						ready_fd = tmpEntry->socket;
						break;  /* exit while */
					  }
#endif /* NO_SSL -- added by jwz */

					FD_SET(tmpEntry->socket, &read_fds);
					if(tmpEntry->socket >= fd_set_size)
						fd_set_size = tmpEntry->socket+1;
				  }
			  }
			else if(tmpEntry == 
					(ActiveEntry *) XP_ListGetObjectNum(net_EntryList, 1))
			  {
                /* if this is the very first object in the list
                 * and it's a local file or a memory cache copy
                 * then use this one entry and skip the select call
                 * we won't deadlock because we reorder the list
                 * with every call to net process net.
                 */
				fd_type = NET_LOCAL_FILE_FD;
				ready_fd = tmpEntry->socket;  /* ugly */
				break;  /* exit while */
			  }

			if(tmpEntry->con_sock != SOCKET_INVALID)
			  {
			    FD_SET(tmpEntry->con_sock, &write_fds);
			    FD_SET(tmpEntry->con_sock, &exps_fds);
			    if(tmpEntry->con_sock >= fd_set_size)
				    fd_set_size = tmpEntry->con_sock+1;
			  }
		  }

		if(ready_fd == -1) /* in case we set it for the local file type above */
		  {
            int ret = select(fd_set_size,
							 &read_fds,
							 &write_fds,
							 &exps_fds,
							 &timeout);

		    if(!ret)
			  {
			  	TRACEMSG(("Select returned no active sockets! "
						  "WASTED CALL TO PROCESS NET"));
    		    LIBNET_UNLOCK_AND_RETURN(XP_ListIsEmpty(net_EntryList) ? 0 : 1);
			  }

            fd_type = NET_SOCKET_FD;

            /* process one socket ready for reading,
		     * find the first one ready
	         */
			list_item = net_EntryList;
            while((tmpEntry=(ActiveEntry *) XP_ListNextObject(list_item)) != 0)
              {
                if(tmpEntry->busy)
                    continue;

                if(!tmpEntry->local_file && !tmpEntry->memory_file)
                  {
		            if(FD_ISSET(tmpEntry->socket, &read_fds)
					    || FD_ISSET(tmpEntry->socket, &write_fds)
					      || FD_ISSET(tmpEntry->socket, &exps_fds))
			          {
				        ready_fd = tmpEntry->socket;
				        break;
			          }
                  }
			  }

		    if(ready_fd == -1)
		      {
    		    LIBNET_UNLOCK_AND_RETURN(XP_ListIsEmpty(net_EntryList) ? 0 : 1);
		      }
		  }
      }

    list_item = net_EntryList;

    /* process one socket ready for reading
     *
	 * find the ready socket in the active entry list
     */
    while((tmpEntry = (ActiveEntry *) XP_ListNextObject(list_item)) != 0)
      {
        TRACEMSG(("Found item in Active Entry List. sock #%d  con_sock #%d",
                            tmpEntry->socket, tmpEntry->con_sock));

		if(tmpEntry->busy)
		  {
			/* I guess this is alright since one of the streams
			 * (Play audio for instance) may put up a modal dialog
			 * box which causes the main event loop to get called
			 * from within the stream
		 	 *
			 * FE_Alert(tmpEntry->window_id, "reentrant call to Process Net");
			 */
		  }
		else if( ((tmpEntry->memory_file && fd_type == NET_UNKNOWN_FD) ||
												/* always activate memory files */
			 (fd_type == NET_LOCAL_FILE_FD && tmpEntry->local_file && ready_fd == tmpEntry->socket) ||
			  (fd_type != NET_LOCAL_FILE_FD &&
						(ready_fd == tmpEntry->socket || ready_fd == tmpEntry->con_sock) ) ) )
	      {

			 tmpEntry->busy = TRUE;

	         TRACEMSG(("Item has data ready for read"));
	         switch(tmpEntry->protocol)
		        {
#ifdef MOZILLA_CLIENT
					case MAILBOX_TYPE_URL:
            			rv = NET_ProcessMailbox(tmpEntry);
            			break;

        			case MEMORY_CACHE_TYPE_URL:
            			rv = NET_ProcessMemoryCache(tmpEntry);
            			break;

        			case MOCHA_TYPE_URL:
						XP_ASSERT(0);
						break;
#endif /* MOZILLA_CLIENT */

        	        case HTTP_TYPE_URL:
        	        case SECURE_HTTP_TYPE_URL:
			            rv = NET_ProcessHTTP(tmpEntry);
            	    	break;

#ifdef MOZILLA_CLIENT
        			case MAILTO_TYPE_URL:
            			rv = NET_ProcessMailto(tmpEntry);
            			break;

        			case POP3_TYPE_URL:
            			rv = NET_ProcessPop3(tmpEntry);
            			break;
#endif /* MOZILLA_CLIENT */

        	        case GOPHER_TYPE_URL:
			            rv = NET_ProcessGopher(tmpEntry);
            	    	break;

        	        case FTP_TYPE_URL:
			            rv = NET_ProcessFTP(tmpEntry);
            	    	break;

#ifdef MOZILLA_CLIENT
        	        case NEWS_TYPE_URL:
        	        case INTERNAL_NEWS_TYPE_URL:
			            rv = NET_ProcessNews(tmpEntry);
            	    	break;
#endif /* MOZILLA_CLIENT */

                    case FILE_TYPE_URL:
            		case FILE_CACHE_TYPE_URL:
                        rv = NET_ProcessFile(tmpEntry);

            	 } /* end switch */

			 tmpEntry->busy = FALSE;

            /* check for done status on transfer and call
             * exit routine if done.
             */
            if(rv < 0)
              {

                XP_ListRemoveObject(net_EntryList, tmpEntry);

                if(tmpEntry->status == MK_USE_COPY_FROM_CACHE)
                  {
                    TRACEMSG(("304 Not modified recieved using cached entry\n"));
#ifdef MOZILLA_CLIENT
                    NET_RefreshCacheFileExpiration(tmpEntry->URL_s);
#endif /* MOZILLA_CLIENT */

					/* turn off force reload by telling it to use local copy
					 */
					tmpEntry->URL_s->use_local_copy = TRUE;

                    /* restart the transfer
                     */
                    NET_GetURL(tmpEntry->URL_s,
							   tmpEntry->format_out,
							   tmpEntry->window_id,
							   tmpEntry->exit_routine);

					net_CheckForWaitingURL(tmpEntry->window_id,
										   tmpEntry->protocol,
										   tmpEntry->URL_s->load_background);

                  }
                else if(tmpEntry->status == MK_DO_REDIRECT)
                  {
                    TRACEMSG(("Doing redirect part in ProcessNet"));

                    /* restart the whole transfer */
                    NET_GetURL(tmpEntry->URL_s,
                                   tmpEntry->format_out,
                                   tmpEntry->window_id,
                                   tmpEntry->exit_routine);

					net_CheckForWaitingURL(tmpEntry->window_id,
										   tmpEntry->protocol,
										   tmpEntry->URL_s->load_background);                  }
                else if(tmpEntry->status == MK_TOO_MANY_OPEN_FILES)
				  {
        			/* Queue this URL so it gets tried again
         			 */
        			LIBNET_UNLOCK_AND_RETURN(net_push_url_on_wait_queue(
                                    NET_URL_Type(tmpEntry->URL_s->address),
                                    tmpEntry->URL_s,
                                    tmpEntry->format_out,
                                    tmpEntry->window_id,
                                    tmpEntry->exit_routine));
				  }
                else if(tmpEntry->status < 0
						&& tmpEntry->URL_s->last_modified
						&& !tmpEntry->URL_s->use_local_copy
#ifndef NO_SSL /* added by jwz */
						&& XP_GetError() != SSL_ERROR_BAD_CERTIFICATE
#endif /* NO_SSL -- added by jwz */
                        && (tmpEntry->status == MK_CONNECTION_REFUSED
                            || tmpEntry->status == MK_CONNECTION_TIMED_OUT
                            || tmpEntry->status == MK_UNABLE_TO_CREATE_SOCKET
                            || tmpEntry->status == MK_UNABLE_TO_LOCATE_HOST
                            || tmpEntry->status == MK_UNABLE_TO_CONNECT))
				  {
					/* if the status is negative something went wrong
					 * with the load.  If last_modified is set
					 * then we probably have a cache file,
					 * but it might be a broken image so
					 * make sure "use_local_copy" is not
					 * set.
					 *
					 * Only do this when we can't connect to the
					 * server.
					 */

					/* if we had a cache file and got a load
                     * error, go ahead and use it anyways
                     */

                    /* turn off force reload by telling it to use local copy
                     */
                    tmpEntry->URL_s->use_local_copy = TRUE;

					if(CLEAR_CACHE_BIT(tmpEntry->format_out) == FO_PRESENT)
					  {
						StrAllocCat(tmpEntry->URL_s->error_msg,
						XP_GetString( XP_USING_PREVIOUSLY_CACHED_COPY_INSTEAD ) );

						FE_Alert(tmpEntry->window_id,
								 tmpEntry->URL_s->error_msg);
					  }

            		FREE_AND_CLEAR(tmpEntry->URL_s->error_msg);

                    /* restart the transfer
                     */
                    NET_GetURL(tmpEntry->URL_s,
                               tmpEntry->format_out,
                               tmpEntry->window_id,
                               tmpEntry->exit_routine);

                    net_CheckForWaitingURL(tmpEntry->window_id,
                                           tmpEntry->protocol,
										   tmpEntry->URL_s->load_background);
				  }
				else
				  {
					  TRACEMSG(("End of transfer, entry (soc=%d, con=%d) being removed from list with %d status: %s",
								tmpEntry->socket, tmpEntry->con_sock, tmpEntry->status, 
								(tmpEntry->URL_s->address ? tmpEntry->URL_s->address : "")));

					/* catch out of memory errors at the lowest
					 * level since we don't do it at all the out
					 * of memory condition spots
					 */
                    if(tmpEntry->status == MK_OUT_OF_MEMORY
						&& !tmpEntry->URL_s->error_msg)
                      {
                        tmpEntry->URL_s->error_msg =
                                NET_ExplainErrorDetails(MK_OUT_OF_MEMORY);
                      }

					load_background = tmpEntry->URL_s->load_background;
					/* run the exit routine
					 */
                    net_CallExitRoutine(tmpEntry->exit_routine,
										tmpEntry->URL_s,
										tmpEntry->status,
										tmpEntry->format_out,
										tmpEntry->window_id);

					net_CheckForWaitingURL(tmpEntry->window_id,
										   tmpEntry->protocol,
										   load_background);

#ifdef MILAN
					/* if the error was caused by a NETWORK DOWN
					 * then interrupt the window.  This
					 * could still be a problem since another
					 * window may be active, but it should get
					 * the same error
					 */
					if(SOCKET_ERRNO == XP_ERRNO_ENETDOWN)
					  {
						NET_SilentInterruptWindow(tmpEntry->window_id);
					  }
#endif /* MILAN */

                  }

                XP_FREE(tmpEntry);  /* free the now non active entry */

              } /* end if */

            TRACEMSG(("Leaving process net with %d items in list",
					  XP_ListCount(net_EntryList)));

    		LIBNET_UNLOCK_AND_RETURN(XP_ListIsEmpty(net_EntryList) ? 0 : 1); /* all done */

          } /* end if FD_ISSET */

      } /* end while */


	/* the active socket wasn't found in the list :(
     */
	TRACEMSG(("Invalid call to NET_ProcessNet: Active item with passed in fd: %d not found\n", ready_fd));

    LIBNET_UNLOCK_AND_RETURN(XP_ListIsEmpty(net_EntryList) ? 0 : 1);

}

/*
 *	net_InterruptActiveStream
 *	Kills a single stream given its ActiveEntry*
 *	*	FrontEndCancel -> NetInterruptWindow -> net_InterruptActiveStream
 *	*	LayoutNewDoc -> KillOtherLayouts -> net_InterruptActiveStream
 *
 *	Also removes the entry from the active list and frees it.
 *	Also allows waiting urls into the active list.
 */
PRIVATE int
net_InterruptActiveStream (ActiveEntry *entry)
{
	TRACEMSG(("Terminating transfer on port #%d, proto: %d",
								entry->socket, entry->protocol));

	/* remove it from the active list first to prevent
	 * reentrant problem
	 */
	XP_ListRemoveObject(net_EntryList, entry);

	switch(entry->protocol)
	{
#ifdef MOZILLA_CLIENT
		case MAILBOX_TYPE_URL:
			NET_InterruptMailbox(entry);
			break;

		case MEMORY_CACHE_TYPE_URL:
			NET_InterruptMemoryCache(entry);
			break;

		case MAILTO_TYPE_URL:
			NET_InterruptMailto(entry);
			break;

       	case POP3_TYPE_URL:
       		NET_InterruptPop3(entry);
       		break;

#ifdef MOCHA
        case MOCHA_TYPE_URL:
			net_InterruptMocha(entry);
			break;
#endif /* MOCHA */
#endif /* MOZILLA_CLIENT */


	    case HTTP_TYPE_URL:
	    case SECURE_HTTP_TYPE_URL:
	        NET_InterruptHTTP(entry);
	        break;

	    case GOPHER_TYPE_URL:
	        NET_InterruptGopher(entry);
	        break;

	    case FTP_TYPE_URL:
	        NET_InterruptFTP(entry);
	        break;

#ifdef MOZILLA_CLIENT
	    case INTERNAL_NEWS_TYPE_URL:
	    case NEWS_TYPE_URL:
	        NET_InterruptNews(entry);
	        break;

	    case FILE_TYPE_URL:
		case FILE_CACHE_TYPE_URL:
	        NET_InterruptFile(entry);
	        break;
#endif /* MOZILLA_CLIENT */

		default:
			XP_ASSERT(0);
	 } /* end switch */

	TRACEMSG(("End of transfer, entry (soc=%d, con=%d) being removed from list with %d status: %s",
			  entry->socket, entry->con_sock, entry->status, entry->URL_s->address));

	/* call exit routine since we know we are done */
	net_CallExitRoutine(entry->exit_routine,
						entry->URL_s,
						entry->status,
						entry->format_out,
						entry->window_id);

	/* decrement here since we don't call checkForWaitingURL's
	 */
	NET_TotalNumberOfProcessingURLs--;

	if(!NET_AreThereActiveConnectionsForWindow(entry->window_id))
		FE_AllConnectionsComplete(entry->window_id);

	/* free the no longer active entry */
	XP_FREE(entry);

	return 0;
}

/*
 *	NET_InterruptStream kills just stream associated with an URL.
 */
PUBLIC int
NET_InterruptStream (URL_Struct *nurl)
{
	/* Find the ActiveEntry structure for this URL */
	ActiveEntry *tmpEntry = NULL, *entryToKill = NULL;
    XP_List * iter;
	int status;
	
	TRACEMSG(("Entering NET_InterruptStream"));
	LIBNET_LOCK();
	iter = net_EntryList;

	if(NET_InGetHostByName)
	  {
		TRACEMSG(("call to InterrruptStream while doing gethostbyname call"));
		LIBNET_UNLOCK_AND_RETURN(1);
	  }

    while ((tmpEntry = (ActiveEntry *) XP_ListNextObject(iter)) != NULL) {
    	if (tmpEntry->URL_s == nurl) {
    		entryToKill = tmpEntry;
    		break;
   		}
    }
/*    assert (entryToKill);*/
    /* Kill it & free it */
	if (entryToKill)
	    status = net_InterruptActiveStream (entryToKill);
	else
	    status = -1;

	LIBNET_UNLOCK_AND_RETURN(status);
}


/* interrupt a LoadURL action by using a socket number as
 * an index.  NOTE: that this cannot interrupt non-socket
 * based operations such as file or memory cache loads
 *
 * returns -1 on error.
 */
PUBLIC int
NET_InterruptSocket (NETSOCK socket)
{
    /* Find the ActiveEntry structure for this URL */
    ActiveEntry *tmpEntry = NULL, *entryToKill = NULL;
    XP_List * iter;
    int status;

    TRACEMSG(("Entering NET_InterruptSocket"));
    LIBNET_LOCK();
    iter = net_EntryList;

    if(NET_InGetHostByName)
      {
        TRACEMSG(("call to InterrruptStream while doing gethostbyname call"));
        LIBNET_UNLOCK_AND_RETURN(1);
      }

    while ((tmpEntry = (ActiveEntry *) XP_ListNextObject(iter)) != NULL) {
        if (tmpEntry->con_sock == socket || tmpEntry->socket == socket) {
            entryToKill = tmpEntry;
            break;
        }
    }

	if (entryToKill)
	    status = net_InterruptActiveStream (entryToKill);
	else
	    status = -1;

	LIBNET_UNLOCK_AND_RETURN(status);
}

/*
 *  NET_SetNewContext changes the context associated with a URL Struct.
 *	can also change the url exit routine, which causes the old one to be
 *	called with a status of MK_CHANGING_CONTEXT
 */
PUBLIC int
NET_SetNewContext(URL_Struct *URL_s, MWContext * new_context, Net_GetUrlExitFunc *exit_routine)
{
    /* Find the ActiveEntry structure for this URL */
    ActiveEntry *tmpEntry = NULL;
    XP_List * iter;
	MWContext *old_window_id;

	LIBNET_LOCK();
	iter = net_EntryList;

    while ((tmpEntry = (ActiveEntry *) XP_ListNextObject(iter)) != NULL)
	  {
        if (tmpEntry->URL_s == URL_s)
		  {
			/* call the old exit routine now with the old context */
			/* call with MK_CHANGING_CONTEXT, FE shouldn't free URL_s */
			net_CallExitRoutine(tmpEntry->exit_routine,
								URL_s,
								MK_CHANGING_CONTEXT,
								0,
								tmpEntry->window_id);

		  	/*	see if we're changing exit routines */
			if(exit_routine != NULL)	{
				/*	okay to change now. */
				tmpEntry->exit_routine = exit_routine;
			}
			old_window_id = tmpEntry->window_id;
            tmpEntry->window_id = new_context;

#ifdef XP_WIN
			/*
			 *	Windows needs a way to know when a URL switches contexts,
			 *		so that it can keep the NCAPI progress messages
			 *		specific to a URL loading and not specific to the
			 *		context attempting to load.
			 *	So sue me.  GAB 10-16-95
			 */
			FE_UrlChangedContext(URL_s, old_window_id, new_context);
#endif /* XP_WIN */

			/*	if there are no more connections in the old context, we
			 *	need to call AllConnectionsComplete
			 */
    		if(!NET_AreThereActiveConnectionsForWindow(old_window_id))
				FE_AllConnectionsComplete(old_window_id);

			LIBNET_UNLOCK();
            return(0);
          }
      }

	/* couldn't find it :( */
    XP_ASSERT (0);

	LIBNET_UNLOCK();
    return(-1);
}

/* returns true if the stream is safe for setting up a new
 * context and using a separate window.
 *
 * This will return FALSE in multipart/mixed cases where
 * it is impossible to use separate windows for the
 * same stream.
 */
PUBLIC Bool
NET_IsSafeForNewContext(URL_Struct *URL_s)
{
	if(!URL_s || URL_s->is_active)
		return(FALSE);

	return(TRUE);
}


/* NET_InterruptTransfer
 *
 * Interrupts all transfers in progress that have the same window id as
 * the one passed in.
 */
PRIVATE int
net_InternalInterruptWindow(MWContext * window_id, Bool show_warning)
{
    int starting_list_count;
    ActiveEntry * tmpEntry;
    ActiveEntry * tmpEntry2;
    XP_List * list_item;
    int32 cur_win_id = FE_GetContextID(window_id);
	int number_killed=0;
	XP_Bool call_all_connections_complete = TRUE;

	TRACEMSG(("-------Interrupt Transfer called!"));

	LIBNET_LOCK();
	list_item = net_EntryList;
	starting_list_count = XP_ListCount(net_EntryList);

#ifdef DEBUG
	if(NET_InGetHostByName)
	  {
		TRACEMSG(("call to InterrruptWindow while doing gethostbyname call"));
		LIBNET_UNLOCK_AND_RETURN(1);
	  }
#endif /* DEBUG */

	number_killed = net_AbortWaitingURL(window_id,
										FALSE,
										net_waiting_for_actives_url_list);
	number_killed += net_AbortWaitingURL(window_id,
										FALSE,
										net_waiting_for_connection_url_list);

    /* run through the whole list of connections and
     * interrupt any of them that have a matching window
     * id.
     */
	tmpEntry = (ActiveEntry *) XP_ListNextObject(list_item);

    while(tmpEntry)
      {
      	/* advance to the next item NOW in case we free this one */
      	tmpEntry2 = (ActiveEntry *) XP_ListNextObject(list_item);

        if(FE_GetContextID(tmpEntry->window_id) == cur_win_id)
          {
		    if(tmpEntry->busy)
		      {
				if(show_warning)
				  {
			    	TRACEMSG(("AAAACK, reentrant protection kicking in.!!!"));
#ifdef DEBUG
			    	FE_Alert(tmpEntry->window_id, XP_GetString(XP_ALERT_INTERRUPT_WINDOW));
#endif /* DEBUG */
				  }
		      }
			else
		      {
				FE_EnableClicking(window_id);
				net_InterruptActiveStream (tmpEntry);
				number_killed += 1;

				/* unset call_all_connections_complete here
				 * since InterruptActiveStream will already
				 * call all_connections_complete when it detects
				 * that there are no more active connections
				 * going on for the window.
				 */
				call_all_connections_complete = FALSE;
		      }
          } /* end if cur_win_id */

		tmpEntry = tmpEntry2;
      } /* end while */

	/* If all_connection_complete was called, FE might have destroyed
	 * the context. So dont call FE_EnableClicking() if
	 * call_all_connections_complete was already called.
	 */
	if (call_all_connections_complete)
		FE_EnableClicking(window_id);

	/* we are sure that all connections are complete
	 * but only call it if we killed one.
	 */
	if(number_killed && call_all_connections_complete)
        FE_AllConnectionsComplete(window_id);


    TRACEMSG(("Leaving Interrupt transfer with %d items in list",
                                XP_ListCount(net_EntryList)));

    LIBNET_UNLOCK_AND_RETURN(starting_list_count - XP_ListCount(net_EntryList));

}

/*
 * Interrupts all transfers in progress that have the same window id as
 * the one passed in.
 */
PUBLIC int
NET_InterruptWindow(MWContext * window_id)
{
	return(net_InternalInterruptWindow(window_id, TRUE));
}

/*
 * Silently Interrupts all transfers in progress that have the same
 * window id as the one passed in.
 */
MODULE_PRIVATE int
NET_SilentInterruptWindow(MWContext * window_id)
{
    return(net_InternalInterruptWindow(window_id, FALSE));
}

/* check for any active URL transfers in progress for the given
 * window Id
 *
 * It is possible that there exist URL's on the wait queue that
 * will not be returned by this function.  It is recommended
 * that you call NET_InterruptWindow(MWContext * window_id)
 * when FALSE is returned to make sure that the wait queue is
 * cleared as well.
 * 
 * It is also possible that some URLs will be explicitly marked as
 * "background" URLs that are not identified as active, even though
 * they are.
 */
PUBLIC Bool
NET_AreThereActiveConnectionsForWindow(MWContext * window_id)
{
    ActiveEntry * tmpEntry;
    int32 cur_win_id = FE_GetContextID(window_id);
    XP_List * list_ptr;
    WaitingURLStruct * wus;

	LIBNET_LOCK();

    TRACEMSG(("-------NET_AreThereActiveConnectionsForWindow called!"));

	/* check for connections in the wait queue
	 */
    list_ptr = net_waiting_for_actives_url_list;
    while((wus = (WaitingURLStruct*) XP_ListNextObject(list_ptr)) != NULL)
      {
        if(cur_win_id == FE_GetContextID(wus->window_id) &&
           !wus->URL_s->load_background)
		  {
			LIBNET_UNLOCK();
			return(TRUE);
		  }
      }

    /* check for connections in the connections wait queue
     */
	list_ptr = net_waiting_for_connection_url_list;
    while((wus = (WaitingURLStruct*) XP_ListNextObject(list_ptr)) != NULL)
      {
        if(cur_win_id == FE_GetContextID(wus->window_id) &&
           !wus->URL_s->load_background)
          {
            LIBNET_UNLOCK();
            return(TRUE);
          }
      }

    /* run through the whole list of active connections and
     * return true if any of them have the passed in window id
     */
	list_ptr = net_EntryList;
    while((tmpEntry = (ActiveEntry *) XP_ListNextObject(list_ptr)) != NULL)
      {
        if(cur_win_id == FE_GetContextID(tmpEntry->window_id) &&
           !tmpEntry->URL_s->load_background)
		  {
			LIBNET_UNLOCK();
            return(TRUE);
		  }

      } /* end while */

	LIBNET_UNLOCK();
    return(FALSE);
}

/*
 *  GAB 04-23-96
 *  We're interested in knowing if there are any operations which
 *      will cause activity of the said type, in the said context.
 *  This is in the interests of PE, where they desire a world of
 *      dial on demand (hanging up the phone when we're not doing dick
 *      on the network).
 *
 *  Args:
 *      window_id   The said context to check.
 *                  If NULL, check them all (disregard).
 *      waiting     Include waiting queues in check if TRUE.
 *                  Note that waiting queues do not know thier type
 *                      yet (local, cache, or net), thus type can not
 *                      be correctly checked.  I can only assume that
 *                      we have the technology to mostly decide this
 *                      before the fact, but I see no facility available.
 *      background  Include background activity in the check if TRUE.
 *  Returns:
 *      Bool        TRUE, there is chance of activity of said type.
 *                  FALSE, that particular type of activity is not present.
 *
 */
PUBLIC Bool
NET_HasNetworkActivity(MWContext * window_id, Bool waiting, Bool background)
{
    ActiveEntry * tmpEntry;
    XP_List * list_ptr;
    WaitingURLStruct * wus;

	LIBNET_LOCK();

    TRACEMSG(("-------NET_HasNetworkActivity called!"));

	/* check for connections in the wait queue
	 */
    if(waiting)
      {
        list_ptr = net_waiting_for_actives_url_list;
        while((wus = (WaitingURLStruct*) XP_ListNextObject(list_ptr)) != NULL)
          {
            if((window_id == NULL ||
                FE_GetContextID(window_id) ==
                FE_GetContextID(wus->window_id)) &&
                (background || !wus->URL_s->load_background))
		      {
			    LIBNET_UNLOCK();
			    return(TRUE);
		      }
          }

        /* check for connections in the connections wait queue
         */
	    list_ptr = net_waiting_for_connection_url_list;
        while((wus = (WaitingURLStruct*) XP_ListNextObject(list_ptr)) != NULL)
          {
            if((window_id == NULL ||
                FE_GetContextID(window_id) ==
                FE_GetContextID(wus->window_id)) &&
                (background || !wus->URL_s->load_background))
              {
                LIBNET_UNLOCK();
                return(TRUE);
              }
          }
      }

    /* run through the whole list of active connections
     */
	list_ptr = net_EntryList;
    while((tmpEntry = (ActiveEntry *) XP_ListNextObject(list_ptr)) != NULL)
      {
        if((window_id == NULL ||
            FE_GetContextID(window_id) ==
            FE_GetContextID(tmpEntry->window_id)) &&
            (background || !tmpEntry->URL_s->load_background) &&
            !tmpEntry->memory_file &&
            !tmpEntry->local_file)
		  {
			LIBNET_UNLOCK();
            return(TRUE);
		  }

      } /* end while */

	LIBNET_UNLOCK();
    return(FALSE);
}


/* malloc, init, and set the URL structure used
 * for the network library
 */
PUBLIC URL_Struct * 
NET_CreateURLStruct (CONST char *url, NET_ReloadMethod force_reload)
{
    uint32 all_headers_size;
    URL_Struct * URL_s  = XP_NEW(URL_Struct);

    if(!URL_s)
	  {
    	return NULL;
	  }

    /* zap the whole structure */
    XP_MEMSET (URL_s, 0, sizeof (URL_Struct));

    /* we haven't modified the address at all (since we are just creating it)*/
    URL_s->address_modified=NO;

	URL_s->method = URL_GET_METHOD;

    URL_s->force_reload = force_reload;

	URL_s->load_background = FALSE;

    /* Allocate space for pointers to hold message (http, news etc) headers */
    all_headers_size = 
        INITIAL_MAX_ALL_HEADERS * sizeof (URL_s->all_headers.key[0]);
    URL_s->all_headers.key = (char **) XP_ALLOC(all_headers_size);
    if(!URL_s->all_headers.key)
      {
        NET_FreeURLStruct(URL_s);
    	return NULL;
      }
    XP_MEMSET (URL_s->all_headers.key, 0, all_headers_size);

    all_headers_size = 
        INITIAL_MAX_ALL_HEADERS * sizeof (URL_s->all_headers.value[0]);
    URL_s->all_headers.value = (char **) XP_ALLOC(all_headers_size);
    if(!URL_s->all_headers.value)
      {
        NET_FreeURLStruct(URL_s);
    	return NULL;
      }
    XP_MEMSET (URL_s->all_headers.value, 0, all_headers_size);

    URL_s->all_headers.max_index = INITIAL_MAX_ALL_HEADERS;

    /* set the passed in value */
    StrAllocCopy(URL_s->address, url);
	if (!URL_s->address) {
		/* out of memory, clean up previously allocated objects */
		NET_FreeURLStruct(URL_s);
		URL_s = NULL;
	}

    return(URL_s);
}

/* Increase the size of AllHeaders in URL_struct
 */
PRIVATE Bool 
net_EnlargeURLAllHeaders (URL_Struct * URL_s)
{
    uint32 number_of_entries = 
        URL_s->all_headers.empty_index * MULT_ALL_HEADER_COUNT;
    uint32 realloc_size = 
        number_of_entries * sizeof (URL_s->all_headers.key[0]);
    char **temp;

    if (number_of_entries < MAX_ALL_HEADER_COUNT) {

        temp = (char **) XP_REALLOC(URL_s->all_headers.key, realloc_size);
        if (URL_s->all_headers.key) {
            URL_s->all_headers.key = temp;

            temp = (char **) XP_REALLOC(URL_s->all_headers.value, realloc_size);
            if (URL_s->all_headers.value) {
                URL_s->all_headers.value = temp;

                URL_s->all_headers.max_index = number_of_entries;
                return(TRUE);
            }
        }
    }
    net_FreeURLAllHeaders(URL_s);
    return(FALSE);
}


/* Append Key, Value pair into AllHeaders list of URL_struct
 */
PUBLIC Bool 
NET_AddToAllHeaders(URL_Struct * URL_s, char *name, char *value)
{
    char *key_ptr;
    char *value_ptr;

    XP_ASSERT(URL_s);
    XP_ASSERT(name);
    XP_ASSERT(value);

    if ((URL_s->all_headers.empty_index >= URL_s->all_headers.max_index) &&
        (!net_EnlargeURLAllHeaders(URL_s))) {
        return(FALSE);
    }

    key_ptr = URL_s->all_headers.key[URL_s->all_headers.empty_index] =
        XP_STRDUP(name);
    if (!key_ptr) {
        net_FreeURLAllHeaders(URL_s);
        return(FALSE);
        }

    value_ptr = URL_s->all_headers.value[URL_s->all_headers.empty_index] = 
        XP_STRDUP(value);
    if (!value_ptr) {
        XP_FREE(key_ptr);
        URL_s->all_headers.key[URL_s->all_headers.empty_index] = NULL;
        net_FreeURLAllHeaders(URL_s);
        return(FALSE);
        }
    URL_s->all_headers.empty_index++;
    return(TRUE);
}

/* Set the option IPAddressString field
 * This field overrides the hostname when doing the connect, 
 * and allows Java to specify a spelling of an IP number.
 * Out of memory condition will cause the URL_struct to be destroyed.
 */
PUBLIC int
NET_SetURLIPAddressString (URL_Struct * URL_s, CONST char *ip_string)
{
    XP_ASSERT(URL_s);

    FREEIF(URL_s->IPAddressString);
    URL_s->IPAddressString = NULL;

    /* set the passed in value */
    StrAllocCopy(URL_s->IPAddressString, ip_string);
	return NULL == URL_s->IPAddressString;
}

/* free the contents and the URL structure when finished
 */
PUBLIC void
NET_FreeURLStruct (URL_Struct * URL_s)
{

#ifdef XP_WIN
	/*
	 *	Call the front end to have it free off any
	 *		special data it has hanging off the
	 *		structure.  In specific, the ncapi stuff.
	 */
	FE_DeleteUrlData(URL_s);
#endif /* XP_WIN */

    FREEIF(URL_s->address);
    FREEIF(URL_s->username);
    FREEIF(URL_s->password);
    FREEIF(URL_s->IPAddressString);
    FREEIF(URL_s->referer);
    FREEIF(URL_s->post_data);
    FREEIF(URL_s->post_headers);
    FREEIF(URL_s->content_type);
    FREEIF(URL_s->content_encoding);
    FREEIF(URL_s->x_mac_type);
    FREEIF(URL_s->x_mac_creator);
	FREEIF(URL_s->charset);
	FREEIF(URL_s->boundary);
    FREEIF(URL_s->redirecting_url);
	FREEIF(URL_s->authenticate);
	FREEIF(URL_s->protection_template);
    FREEIF(URL_s->http_headers);
	FREEIF(URL_s->cache_file);
    FREEIF(URL_s->window_target);
	FREEIF(URL_s->window_chrome);
	FREEIF(URL_s->refresh_url);
	FREEIF(URL_s->wysiwyg_url);
	FREEIF(URL_s->error_msg);

    FREEIF(URL_s->key_cipher);
    FREEIF(URL_s->key_issuer);
    FREEIF(URL_s->key_subject);
#ifdef HAVE_SECURITY /* added by jwz */
	if ( URL_s->certificate ) {
		CERT_DestroyCertificate(URL_s->certificate);
	}
	
	if ( URL_s->redirecting_cert ) {
		CERT_DestroyCertificate(URL_s->redirecting_cert);
	}
#endif /* !HAVE_SECURITY -- added by jwz */

    /* Free all memory associated with header information */
    net_FreeURLAllHeaders(URL_s);

	if(URL_s->files_to_post)
	  {
		/* free a null terminated array of filenames
		 */
		int i=0;
		for(i=0; URL_s->files_to_post[i] != NULL; i++)
		  {
			FREE(URL_s->files_to_post[i]); /* free the filenames */
		  }
		FREE(URL_s->files_to_post); /* free the array */
	  }

    XP_FREE(URL_s);

}

/* free the contents of AllHeader structure that is part of URL_Struct
 */
PRIVATE void
net_FreeURLAllHeaders (URL_Struct * URL_s)
{
    uint32 i=0;

    /* Free all memory associated with header information */
    for (i = 0; i < URL_s->all_headers.empty_index; i++) {
        if (URL_s->all_headers.key) {
            FREEIF(URL_s->all_headers.key[i]);
        }
        if (URL_s->all_headers.value) {
            FREEIF(URL_s->all_headers.value[i]);
        }
    }
    URL_s->all_headers.empty_index = URL_s->all_headers.max_index = 0;

    FREEIF(URL_s->all_headers.key);
    FREEIF(URL_s->all_headers.value);
    URL_s->all_headers.key = URL_s->all_headers.value = NULL;
}


/* if there is no slash in a URL besides the two that specify a host
 * then add one to the end.
 */
PRIVATE void add_slash_to_URL (URL_Struct *URL_s)
{
   	char *colon=XP_STRCHR(URL_s->address,':');  /* must find colon */
   	char *slash;

	/* make sure there is a hostname
	 */
	if(*(colon+1) == '/' && *(colon+2) == '/')
	    slash = XP_STRCHR(colon+3,'/');
	else
		return;

	if(slash==NULL)
      {
		TRACEMSG(("GetURL: URL %s changed to ",URL_s->address));

#ifdef MOZILLA_CLIENT
		/* add it now to the Global history since we can't get this
		 * name back :(  this is a bug because we are not sure
		 * that the link will actually work.
		 */
	    GH_UpdateGlobalHistory(URL_s);
#endif /* MOZILLA_CLIENT */

        StrAllocCat(URL_s->address, "/");
        URL_s->address_modified = YES;

        TRACEMSG(("%s",URL_s->address));
      }
}
 
#ifdef MOZILLA_CLIENT
PRIVATE void
net_OutputURLDocInfo(MWContext *ctxt, char *which, char **data, int32 *length)
{
	char *output=0;
	URL_Struct *URL_s = NET_CreateURLStruct(which, NET_DONT_RELOAD);
	struct tm *tm_struct_p;
	char buf[64];
	char *tmp=0;
	char *sec_msg, *il_msg;
	char *charset;

	NET_FindURLInCache(URL_s, ctxt);

#define CELL_TOP 							\
	StrAllocCat(output, 					\
				"</A></TD>"					\
				"<TR><TD VALIGN=BASELINE ALIGN=RIGHT><B>");	
#define CELL_TITLE(title)					\
	StrAllocCat(output, title);
#define CELL_MIDDLE							\
	StrAllocCat(output, 					\
				"</B></TD>"					\
				"<TD>");
#define CELL_BODY(body)						\
	StrAllocCat(output, body);
#define CELL_END							\
	StrAllocCat(output, 					\
				"</TD></TR>");
#define ADD_CELL(c_title, c_body)			\
	CELL_TOP;								\
	CELL_TITLE(c_title);					\
	CELL_MIDDLE;							\
	CELL_BODY(c_body);						\
	CELL_END;

	StrAllocCopy(output, "<TABLE>");

	StrAllocCopy(tmp, "<A HREF=\"");
	StrAllocCat(tmp, URL_s->address);
	StrAllocCat(tmp, "\">");
	StrAllocCat(tmp, URL_s->address);
	StrAllocCat(tmp, "</a>");
	if(URL_s->is_netsite)
	  {
		ADD_CELL(XP_GetString(XP_NETSITE_), tmp);
	  }
	else
	  {
		ADD_CELL(XP_GetString(XP_LOCATION_), tmp);
	  }
	FREE(tmp);

	ADD_CELL(XP_GetString(XP_FILE_MIME_TYPE_), 
			 URL_s->content_type ? URL_s->content_type : XP_GetString(XP_CURRENTLY_UNKNOWN)); 
	ADD_CELL(XP_GetString(XP_SOURCE_), URL_s->cache_file
							? XP_GetString(XP_CURRENTLY_IN_DISK_CACHE)
							  : URL_s->memory_copy ? 
								XP_GetString(XP_CURRENTLY_IN_MEM_CACHE) 
								: XP_GetString(XP_CURRENTLY_NO_CACHE));

	ADD_CELL(XP_GetString(XP_LOCAL_CACHE_FILE_), URL_s->cache_file 
								 ? URL_s->cache_file
									:  XP_GetString(XP_NONE));

	tm_struct_p = localtime(&URL_s->last_modified);

	if(tm_struct_p)
	  {
#ifdef XP_UNIX
		strftime(buf, 64, XP_GetString(XP_LOCAL_TIME_FMT),
			 						   tm_struct_p);
#else
       char tmpbuf[64];
       XP_StrfTime(ctxt,tmpbuf,64,XP_LONG_DATE_TIME_FORMAT,
                   tm_struct_p);
       /* NOTE: XP_LOCAL_TIME_FMT is different depend on platform */
       PR_snprintf(buf, 64, XP_GetString(XP_LOCAL_TIME_FMT), tmpbuf);
#endif
      }
	else
	  {
		buf[0] = 0;
	  }
	ADD_CELL(XP_GetString(XP_LAST_MODIFIED), URL_s->last_modified ? buf :  XP_GetString(XP_MSG_UNKNOWN));

	tm_struct_p = gmtime(&URL_s->last_modified);

	if(tm_struct_p)
	  {
#ifdef XP_UNIX
		strftime(buf, 64, XP_GetString(XP_GMT_TIME_FMT),
			 						   tm_struct_p);
#else
       char tmpbuf[64];
       XP_StrfTime(ctxt,tmpbuf,64,XP_LONG_DATE_TIME_FORMAT,
			 	   tm_struct_p);
       /* NOTE: XP_GMT_TIME_FMT is different depend on platform */
       PR_snprintf(buf, 64, XP_GetString(XP_GMT_TIME_FMT), tmpbuf);
#endif
	  }
	else
	  {
		buf[0] = 0;
	  }

	ADD_CELL(XP_GetString(XP_LAST_MODIFIED), URL_s->last_modified ? buf :  XP_GetString(XP_MSG_UNKNOWN));

#ifdef DEBUG
	PR_snprintf(buf, 64, "%lu seconds since 1-1-70 GMT", URL_s->last_modified);
	ADD_CELL("Last Modified:", URL_s->last_modified ? buf :  "Unknown");
#endif

	XP_SPRINTF(buf, "%lu", URL_s->content_length);
	ADD_CELL(XP_GetString(XP_CONTENT_LENGTH_), URL_s->content_length 
								 ? buf
									:  XP_GetString(XP_MSG_UNKNOWN));
	ADD_CELL(XP_GetString(XP_EXPIRES_), URL_s->expires > 0
								 ? INTL_ctime(ctxt, &URL_s->expires)
									:  XP_GetString(XP_NO_DATE_GIVEN));

	if (URL_s->x_mac_type && *URL_s->x_mac_type)
	  {
		ADD_CELL(XP_GetString(XP_MAC_TYPE_), URL_s->x_mac_type);
	  }
	if (URL_s->x_mac_creator && *URL_s->x_mac_creator)
	  {
		ADD_CELL(XP_GetString(XP_MAC_CREATOR_), URL_s->x_mac_creator);
	  }

	if (URL_s->charset)
	  {
		ADD_CELL(XP_GetString(XP_CHARSET_), URL_s->charset);
	  }
	else 
	  {
		charset = INTL_CharSetDocInfo(ctxt);
		ADD_CELL(XP_GetString(XP_CHARSET_), charset);
		FREE(charset);
	  }

	if(URL_s->cache_file || URL_s->memory_copy)
		sec_msg = XP_PrettySecurityStatus(URL_s->security_on, 
									  	  URL_s->key_cipher, 
									  	  URL_s->key_size, 
									  	  URL_s->key_secret_size);
	else
		sec_msg = XP_STRDUP(XP_GetString(XP_STATUS_UNKNOWN));

	if(sec_msg)
	  {
		ADD_CELL(XP_GetString(XP_SECURITY_), sec_msg);
		FREE(sec_msg);
	  }

#ifdef HAVE_SECURITY /* added by jwz */
	if(URL_s->certificate)
		sec_msg = CERT_HTMLCertInfo(URL_s->certificate, PR_TRUE);
	else
#endif /* !HAVE_SECURITY -- added by jwz */
		sec_msg = 0;

#ifndef NO_SSL /* added by jwz */
	if(sec_msg)
	  {
		char *extstring;
		extstring = SECNAV_MakeCertButtonString(URL_s->certificate);
		if ( extstring ) {
			StrAllocCat(sec_msg, extstring);
			XP_FREE(extstring);
		}
		ADD_CELL(XP_GetString(XP_CERTIFICATE_), sec_msg);
		FREE(sec_msg);
	  }
#endif /* NO_SSL -- added by jwz */
	StrAllocCat(output, "</TABLE>");

	if(URL_s->content_type 
		&& !strncasecomp(URL_s->content_type, "image", 5))
	  {
        /* Have a seat. Lie down.  Tell me about yourself. */
        il_msg = IL_HTMLImageInfo(URL_s->address);
        if (il_msg) {
          StrAllocCat(output, "<HR>\n");
          StrAllocCat(output, il_msg);
          XP_FREE(il_msg);
        }
	  }
		
	*data = output;
	*length = XP_STRLEN(output);

	NET_FreeURLStruct(URL_s);
	return;
}

/* prints information about a context usint HTML.
 *
 * The context passed in as arg2 is the context that
 * we are printing info about and the context that
 * is in the ActiveEntry is the window we are displaying
 * within
 */
PRIVATE
int
net_PrintContextInfo(ActiveEntry *cur_entry, MWContext *context)
{
	History_entry *his;
	NET_StreamClass *stream;
	char buf[64];
	char *tmp;

    /* get the current history entry and
     * extract the URL from it.  That has
     * the top level document URL and the title
     */
    his = SHIST_GetCurrent (&context->hist);
   
	cur_entry->format_out = CLEAR_CACHE_BIT(cur_entry->format_out);
	StrAllocCopy(cur_entry->URL_s->content_type, TEXT_HTML);
    stream = NET_StreamBuilder(cur_entry->format_out,
                                   cur_entry->URL_s, cur_entry->window_id);

    if(!stream)
        return(MK_UNABLE_TO_CONVERT);

    XP_STRCPY(buf, "<FONT SIZE=+2><b>");
	(*stream->put_block)(stream->data_object, buf, XP_STRLEN(buf));

    if(his && his->title)
		tmp = XP_STRDUP(his->title);
	else
		tmp = XP_STRDUP(XP_GetString(XP_UNTITLED_DOCUMENT));

	(*stream->put_block)(stream->data_object, tmp, XP_STRLEN(tmp));

    XP_STRCPY(buf, XP_GetString(XP_HAS_THE_FOLLOWING_STRUCT));
	(*stream->put_block)(stream->data_object, buf, XP_STRLEN(buf));

	LO_DocumentInfo(context, stream);

	(*stream->complete)(stream->data_object);
	cur_entry->status = MK_DATA_LOADED;
	cur_entry->status = MK_DATA_LOADED;

	return(-1);
}
#endif /* MOZILLA_CLIENT */

/* print out silly about type URL
 */
PRIVATE int net_output_about_url(ActiveEntry * cur_entry)
{
    NET_StreamClass * stream;
	char * data=0;
	char * content_type=0;
	int32 length;
	char * which = cur_entry->URL_s->address;
	char * colon = strchr (which, ':');
	void * fe_data = NULL;
	Bool   uses_fe_data=FALSE;

	
	if (colon)
	  which = colon + 1;

	if(0)
	  {
		/* placeholder to make ifdef's easier */
	  }
#ifdef MOZILLA_CLIENT
	else if(NET_URL_Type(which))
	  {
	    /* this url is asking for document info about this
		 * URL.
		 * Call a NET function to generate the HTML block
		 */
		net_OutputURLDocInfo(cur_entry->window_id, which, &data, &length);
		StrAllocCopy(content_type, TEXT_HTML);
	  }
	else if(!strcasecomp(which, "cache"))
	  {
		NET_DisplayCacheInfoAsHTML(cur_entry);
		return(-1);
	  }
	else if(!strcasecomp(which, "memory-cache"))
	  {
		NET_DisplayMemCacheInfoAsHTML(cur_entry);
		return(-1);
	  }
	else if(!strcasecomp(which, "image-cache"))
	  {
        IL_DisplayMemCacheInfoAsHTML(cur_entry->format_out, cur_entry->URL_s,
                                     cur_entry->window_id);
		return(-1);
	  }
	else if(!strcasecomp(which, "document"))
	  {
		char buf[64];
	  	History_entry *his;
		MWContext *context;

		/* output a Grid here with the first grid
		 * cell containing an about: url with the
		 * context pointer as the argument.
		 * 
		 * also put the context pointer in as the post data
		 * if it's not there already so that history will
		 * do the right thing.
		 *
		 * also output blank string for all FO's not view source
		 * and save
		 */

		/* if there is post data then a context ID is hiding
		 * in there.  Get it and verify it
		 */
		if(cur_entry->URL_s->post_data)
		  {
#if defined(__sun) && !defined(SVR4) /* sun 4.1.3 */
			sscanf(cur_entry->URL_s->post_data, "%lu", &context);
#else
			sscanf(cur_entry->URL_s->post_data, "%p", &context);
#endif
			if(!XP_IsContextInList(context))
				context = cur_entry->window_id;
		  }
		else
		  {
			context = cur_entry->window_id;
		  }
			
		/* get the current history entry and
		 * extract the URL from it.  That has
		 * the top level document URL 
		 */
	  	his = SHIST_GetCurrent (&context->hist);

		StrAllocCat(data, "<TITLE>");
		StrAllocCat(data, XP_GetString(XP_DOCUMENT_INFO));
		StrAllocCat(data, 	"</TITLE>"
							"<frameset rows=\"40%,60%\">"
							"<frame src=about:");

		StrAllocCopy(cur_entry->URL_s->window_target, "%DocInfoWindow");
		/* add a Chrome Struct so that we can control the window */
		cur_entry->URL_s->window_chrome = XP_NEW(Chrome);

		if(cur_entry->URL_s->window_chrome)
		  {
			XP_MEMSET(cur_entry->URL_s->window_chrome, 0, sizeof(Chrome));
			/* it should be MWContextDialog
			 * but X isn't ready for it yet...
			 */
#ifdef XP_UNIX
			cur_entry->URL_s->window_chrome->type = MWContextBrowser;
#else
			cur_entry->URL_s->window_chrome->type = MWContextDialog;
#endif
			cur_entry->URL_s->window_chrome->show_scrollbar = TRUE;
			cur_entry->URL_s->window_chrome->allow_resize = TRUE;
			cur_entry->URL_s->window_chrome->allow_close = TRUE;
		  }

		/* don't let people really view the source of this... */
		if(FO_PRESENT != CLEAR_CACHE_BIT(cur_entry->format_out))	
		  {
			StrAllocCat(data, "blank");
		  }
		else
		  {
			StrAllocCat(data, "FeCoNtExT=");

		    if(!cur_entry->URL_s->post_data)
		      {
#if defined(__sun) && !defined(SVR4) /* sun 4.1.3 */
			    XP_SPRINTF(buf, "%lu", context);
#else
			    XP_SPRINTF(buf, "%p", context);
#endif
		 	    StrAllocCat(data, buf);
		 	    StrAllocCopy(cur_entry->URL_s->post_data, buf);
		      }
		    else
		      {
		 	    StrAllocCat(data, cur_entry->URL_s->post_data);
		      }
		  }

		StrAllocCat(data, "><frame name=Internal_URL_Info src=about:");
		
		if(his && his->address)
			StrAllocCat(data, his->address);
		else
			StrAllocCat(data, "blank");

		StrAllocCat(data, "></frameset>");

		length = XP_STRLEN(data);
		StrAllocCopy(content_type, TEXT_HTML);
	  }
	else if(!strncasecomp(which, "Global", 6))
	  {
		cur_entry->status = NET_DisplayGlobalHistoryInfoAsHTML(
													cur_entry->window_id,
                                   					cur_entry->URL_s,
                                   					cur_entry->format_out);

		return(-1);

	  }
	else if(!XP_STRNCMP(which, "FeCoNtExT=", 10))
	  {
		MWContext *old_ctxt;
		
#if defined(__sun) && !defined(SVR4) /* sun 4.1.3 */
		sscanf(which+10, "%lu", &old_ctxt);
#else
		sscanf(which+10, "%p", &old_ctxt);
#endif

		/* verify that we have the right context 
		 *
		 */
		if(!XP_IsContextInList(old_ctxt))
		  {
			StrAllocCopy(data, XP_GetString(XP_THE_WINDOW_IS_NOW_INACTIVE));
		  }
		else	
		  {
			return(net_PrintContextInfo(cur_entry, old_ctxt));
		  }		
    
	    length = XP_STRLEN(data);
	    StrAllocCopy(content_type, TEXT_HTML);

      }
#endif /* MOZILLA_CLIENT */
	else if(!strncasecomp(which, "authors", 7))
	  {
		static const char *d =
		  ("<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html\" "
		   "CHARSET=\"iso-8859-1\"><TITLE>Mozilla &Uuml;ber Alles</TITLE>"
		   "<BODY BGCOLOR=\"#C0C0C0\" TEXT=\"#000000\">"
		   "<TABLE width=\"100%\" height=\"100%\"><TR><TD ALIGN=CENTER>"
		   "<FONT SIZE=5><FONT COLOR=\"#0000EE\">about:authors</FONT> "
		   "removed.</FONT></TD></TR></TABLE>");
	    data = XP_STRDUP(d);
		length = (d ? XP_STRLEN(d) : 0);
	    content_type = XP_STRDUP(TEXT_HTML);
	    uses_fe_data = FALSE;
	  }
    else
      {
	    fe_data = FE_AboutData(which, &data, &length, &content_type);
	    uses_fe_data=TRUE;
      }

	if (!data || !content_type)
	  {
		cur_entry->status = MK_MALFORMED_URL_ERROR;
	  }
	else
	  {
		int status;
		StrAllocCopy(cur_entry->URL_s->content_type, content_type);

		cur_entry->format_out = CLEAR_CACHE_BIT(cur_entry->format_out);

		stream = NET_StreamBuilder(cur_entry->format_out,
								   cur_entry->URL_s, cur_entry->window_id);

		if(!stream)
		  {
			cur_entry->status = MK_UNABLE_TO_CONVERT;
		  	return(MK_UNABLE_TO_CONVERT);
		  }

		status = (*stream->put_block)(stream->data_object, data, length);

		if (status >= 0)
		  (*stream->complete) (stream->data_object);
		else
		  (*stream->abort) (stream->data_object, status);

		cur_entry->status = status;

		FREE(stream);
	  }

	if(uses_fe_data)
	  {
#ifdef XP_MAC
		FE_FreeAboutData (fe_data, which);
#endif /* XP_MAC */
	  }
	else
	  {
		FREE(data);
		FREE(content_type);
	  }


    return(-1);
}

#ifdef MOZILLA_CLIENT
#ifndef NO_SSL /* added by jwz */
/* print out security URL
 */
PRIVATE int net_output_security_url(ActiveEntry * cur_entry, MWContext *cx)
{
    NET_StreamClass * stream;
	char * content_type;
	char * which = cur_entry->URL_s->address;
	char * colon = XP_STRCHR (which, ':');

	if (colon)
	  {
		/* found the first colon; now find the question mark
		   (as in "about:security?certs"). */
		which = colon + 1;
		colon = XP_STRCHR (which, '?');
		if (colon)
		  which = colon + 1;
		else
		  which = which + XP_STRLEN (which); /* give it "" */
	  }

	content_type = SECNAV_SecURLContentType(which);

	if (!content_type) {
		cur_entry->status = MK_MALFORMED_URL_ERROR;

	} else if (!strcasecomp(content_type, "advisor")) {
	    cur_entry->status = SECNAV_SecHandleSecurityAdvisorURL(cx, which);

	} else {
		int status;

		StrAllocCopy(cur_entry->URL_s->content_type, content_type);

		cur_entry->format_out = CLEAR_CACHE_BIT(cur_entry->format_out);

		stream = NET_StreamBuilder(cur_entry->format_out,
								   cur_entry->URL_s, cur_entry->window_id);

		if (!stream)
			return(MK_UNABLE_TO_CONVERT);

		status = SECNAV_SecURLData(which, stream, cx);

		if (status >= 0) {
			(*stream->complete) (stream->data_object);
		} else {
			(*stream->abort) (stream->data_object, status);
		}

		cur_entry->status = status;

		FREE(stream);
	}

    return(-1);
}
#endif /* NO_SSL -- added by jwz */
#endif /* MOZILLA_CLIENT */

PRIVATE Bool net_about_kludge(URL_Struct *URL_s)
{
  unsigned char *url = (unsigned char *) URL_s->address;
  unsigned char *user = (unsigned char *) XP_STRDUP ((char*)(url + 6));
  unsigned char *tmp;
 
  if (user == NULL)
  	return FALSE;

  for (tmp = user; *tmp; tmp++) *tmp += 23;

  if(!XP_STRCMP((char*)user, "\170\211\200") ||					/* ari */
	 !XP_STRCMP((char*)user, "\170\213\206\213\200\172") ||		/* atotic */
	 !XP_STRCMP((char*)user, "\171\203\220\213\177\174") ||		/* blythe */
	 !XP_STRCMP((char*)user, "\172\177\206\214\172\202") ||		/* chouck */
	 !XP_STRCMP((char*)user, "\172\204\170\205\212\202\174") ||	/* cmanske */
	 !XP_STRCMP((char*)user, "\173\202\170\211\203\213\206\205") ||     /* dkarlton */
	 !XP_STRCMP((char*)user, "\173\204\206\212\174") ||			/* dmose */
	 !XP_STRCMP((char*)user, "\173\207") ||						/* dp */
	 !XP_STRCMP((char*)user, "\174\171\200\205\170") ||			/* ebina */
	 !XP_STRCMP((char*)user, "\175\213\170\205\176") ||			/* ftang */
	 !XP_STRCMP((char*)user, "\174\211\200\202") ||				/* erik */
	 !XP_STRCMP((char*)user, "\172\217\214\174") ||				/* cxue */
	 !XP_STRCMP((char*)user, "\171\206\171\201") ||				/* bobj */
	 !XP_STRCMP((char*)user, "\175\214\171\200\205\200") ||     /* fubini */
	 !XP_STRCMP((char*)user, "\175\214\211") ||					/* fur */
	 !XP_STRCMP((char*)user, "\177\170\176\170\205") ||			/* hagan */
	 !XP_STRCMP((char*)user, "\201\170\211") ||					/* jar */
	 !XP_STRCMP((char*)user, "\201\174\175\175") ||				/* jeff */
	 !XP_STRCMP((char*)user, "\201\176") ||						/* jg */
	 !XP_STRCMP((char*)user, "\201\216\221") ||					/* jwz */
	 !XP_STRCMP((char*)user, "\201\212\216") ||					/* jsw */
	 !XP_STRCMP((char*)user, "\202\170\211\203\213\206\205") ||	/* karlton */
	 !XP_STRCMP((char*)user, "\202\200\207\207") ||				/* kipp */
	 !XP_STRCMP((char*)user, "\203\213\170\171\171") ||			/* ltabb */
	 !XP_STRCMP((char*)user, "\204\170\211\172\170") ||			/* marca */
	 !XP_STRCMP((char*)user, "\204\203\204") ||					/* mlm */
	 !XP_STRCMP((char*)user, "\204\206\205\213\214\203\203\200")|| /*montulli*/
	 !XP_STRCMP((char*)user, "\204\213\206\220") ||				/* mtoy */
	 !XP_STRCMP((char*)user, "\207\170\210\214\200\205") ||		/* paquin */
	 !XP_STRCMP((char*)user, "\207\170\203\174\215\200\172\177") ||	/* palevich */
	 !XP_STRCMP((char*)user, "\211\170\204\170\205") ||			/* raman */
	 !XP_STRCMP((char*)user, "\211\174\207\202\170") ||
/* repka */
	 !XP_STRCMP((char*)user, "\211\206\171\204") ||				/* robm */
     !XP_STRCMP((char*)user, "\211\206\174\171\174\211") ||     /* roeber */
	 !XP_STRCMP((char*)user, "\212\177\170\211\206\205\200") ||	/* sharoni */
	 !XP_STRCMP((char*)user, "\204\170\203\204\174\211") ||	/* malmer */
	 !XP_STRCMP((char*)user, "\212\202") ||						/* sk */
	 !XP_STRCMP((char*)user, "\212\207\174\205\172\174") ||		/* spence */
	 !XP_STRCMP((char*)user, "\213\174\211\211\220") ||			/* terry */
	 !XP_STRCMP((char*)user, "\213\200\204\204"))				/* timm */
	{
	  char *head = ("\177\213\213\207\121\106\106\177\206\204\174\105\205"
					"\174\213\212\172\170\207\174\105\172\206\204\106\207"
					"\174\206\207\203\174\106"
					/* "http://home.netscape.com/people/" */
					);
	  unsigned char *location = (unsigned char *)
		XP_ALLOC (XP_STRLEN ((char *) head) + XP_STRLEN ((char *) user) + 2);
	  if (! location)
		{
		  XP_FREE (user);
		  return FALSE;
		}
	  XP_STRCPY ((char *) location, (char *) head);
	  XP_STRCAT ((char *) location, (char *) user);
	  for (tmp = location; *tmp; tmp++) *tmp -= 23;
	  XP_STRCAT ((char *) location, "/");
	  XP_FREE (user);
	  XP_FREE (URL_s->address);
	  URL_s->address = (char *) location;
	  URL_s->address_modified = TRUE;
	  return TRUE;
	}
  else
	{
	  XP_FREE (user);
	  return FALSE;
	}
}


/*
 *  Check the no_proxy environment variable to get the list
 *  of hosts for which proxy server is not consulted.
 *
 *  no_proxy is a comma- or space-separated list of machine
 *  or domain names, with optional :port part.  If no :port
 *  part is present, it applies to all ports on that domain.
 *
 *  Example: "netscape.com,some.domain:8001"
 *
 */
PRIVATE Bool override_proxy (CONST char * URL)
{
    char * p = NULL;
    char * host = NULL;
    int port = 0;
    int h_len = 0;
    char *no_proxy = MKno_proxy;

    if(no_proxy==NULL)
        return(FALSE);

    if (!(host = NET_ParseURL(URL, GET_HOST_PART)))
        return NO;

    if (!*host)
      {
        XP_FREE(host);
        return NO;
      }

    p = XP_STRCHR(host, ':');
    if (p)     /* Port specified */
      {
        *p++ = 0;                       /* Chop off port */
        port = atoi(p);
      }
    else
      {                          /* Use default port */
        char * access = NET_ParseURL(URL, GET_PROTOCOL_PART);
        if (access) {
            if      (!XP_STRCMP(access,"http"))    port = 80;
            else if (!XP_STRCMP(access,"gopher"))  port = 70;
            else if (!XP_STRCMP(access,"ftp"))     port = 21;
                XP_FREE(access);
        }
      }
    if (!port) port = 80;           /* Default */
    h_len = XP_STRLEN(host);

    while (*no_proxy) {
        char * end;
        char * colon = NULL;
        int templ_port = 0;
        int t_len;

        while (*no_proxy && (isspace(*no_proxy) || *no_proxy==','))
        no_proxy++;                 /* Skip whitespace and separators */

        end = no_proxy;
        while (*end && !isspace(*end) && *end != ',')   /* Find separator */
		  {
            if (*end==':') colon = end;                 /* Port number given */
            end++;
          }

        if (colon)
		  {
            templ_port = atoi(colon+1);
            t_len = colon - no_proxy;
          }
		else
		  {
            t_len = end - no_proxy;
          }

        /* don't worry about case when comparing the requested host to the proxies in the
	   no proxy list, i.e. use XP_STRNCASECMP */
	if ((!templ_port || templ_port == port)  && (t_len > 0  &&  t_len <= h_len  &&
		!XP_STRNCASECMP(host + h_len - t_len, no_proxy, t_len))) 
		  {
            XP_FREE(host);
            return YES;
          }
        if (*end)
            no_proxy = end+1;
        else
            break;
    }

    XP_FREE(host);
    return NO;
}

PUBLIC void
NET_SetProxyServer(NET_ProxyType type, const char * org_host_port)
{
	char *host_port = 0;

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 64
#endif

	if(org_host_port && *org_host_port) {
		host_port = XP_STRDUP(org_host_port);

		if(!host_port)
			return;

		/* limit the size of host_port to within MAXHOSTNAMELEN */
		if(XP_STRLEN(host_port) > MAXHOSTNAMELEN)
			host_port[MAXHOSTNAMELEN] = '\0';
	}

	TRACEMSG(("NET_SetProxyServer called with host: %s",
			  host_port ? host_port : "(null)"));

    switch(type)
	  {
		case PROXY_AUTOCONF_URL:
			if (host_port) {
				if (!MKproxy_ac_url || XP_STRCMP(MKproxy_ac_url, org_host_port)) {
					StrAllocCopy(MKproxy_ac_url, org_host_port);
					NET_ProxyAcLoaded = FALSE;
				}
			}
			else {
				if (MKproxy_ac_url) {
					FREE_AND_CLEAR(MKproxy_ac_url);
					NET_ProxyAcLoaded = FALSE;
				}
			}
			break;

		case FTP_PROXY:
			if(host_port)
		        StrAllocCopy(MKftp_proxy, host_port);
			else
				FREE_AND_CLEAR(MKftp_proxy);
			break;
		case GOPHER_PROXY:
			if(host_port)
		        StrAllocCopy(MKgopher_proxy, host_port);
			else
				FREE_AND_CLEAR(MKgopher_proxy);
			break;
		case HTTP_PROXY:
			if(host_port)
		        StrAllocCopy(MKhttp_proxy, host_port);
			else
				FREE_AND_CLEAR(MKhttp_proxy);
			break;
		case HTTPS_PROXY:
			if(host_port)
		        StrAllocCopy(MKhttps_proxy, host_port);
			else
				FREE_AND_CLEAR(MKhttps_proxy);
			break;
		case NEWS_PROXY:
			if(host_port)
		        StrAllocCopy(MKnews_proxy, host_port);
			else
				FREE_AND_CLEAR(MKnews_proxy);
			break;
		case WAIS_PROXY:
			if(host_port)
		        StrAllocCopy(MKwais_proxy, host_port);
			else
				FREE_AND_CLEAR(MKno_proxy);
			break;

		case NO_PROXY:
			if(host_port)
		        StrAllocCopy(MKno_proxy, org_host_port);
			break;

		default:
			/* ignore */
			break;
	  }

	FREEIF(host_port);
}


/*
 * Select proxy style; one of:
 *		PROXY_STYLE_MANUAL		-- old style proxies
 *		PROXY_STYLE_AUTOMATIC	-- new style proxies
 *		PROXY_STYLE_NONE		-- no proxies
 *
 */
PUBLIC void
NET_SelectProxyStyle(NET_ProxyStyle style)
{
	if (MKproxy_style != style) {
		NET_ProxyAcLoaded = FALSE;
	}

	MKproxy_style = style;

	if (MKproxy_style != PROXY_STYLE_AUTOMATIC) {
		FREE_AND_CLEAR(MKproxy_ac_url);
		NET_ProxyAcLoaded = FALSE;
	}

	if (MKproxy_style != PROXY_STYLE_MANUAL) {
		FREE_AND_CLEAR(MKftp_proxy);
		FREE_AND_CLEAR(MKgopher_proxy);
		FREE_AND_CLEAR(MKhttp_proxy);
		FREE_AND_CLEAR(MKhttps_proxy);
		FREE_AND_CLEAR(MKwais_proxy);
		FREE_AND_CLEAR(MKnews_proxy);
		FREE_AND_CLEAR(MKno_proxy);

		NET_SetSocksHost(NULL);
	}
}


/*
 * Force it to reload automatic proxy config.
 *
 *
 */
#ifdef MOZILLA_CLIENT
PUBLIC void
NET_ReloadProxyConfig(MWContext *window_id)
{
    if (MKproxy_ac_url &&
		(!MKproxy_style || MKproxy_style == PROXY_STYLE_AUTOMATIC))
      {
		  NET_ProxyAcLoaded = FALSE;
      }
}
#endif	/* MOZILLA_CLIENT */


/* Debugging routine prints an URL (and string "header")
 */
#ifdef DEBUG
void TraceURL (URL_Struct *url, char *header)
{
    TRACEMSG(("URL %s", header));
    if (url->address)           TRACEMSG(("    address          %s", url->address));
    if (url->content_length)    TRACEMSG(("    content_length   %i", url->content_length));
    if (url->content_type)      TRACEMSG(("    content_type     %s", url->content_type));
    if (url->content_encoding)  TRACEMSG(("    content_encoding %s", url->content_encoding));
}
#endif

#if 0
/* This function's existance was prompted by the front end desiring the ability to put the
   client in kiosk mode. The function removes any potentially sensitive information from the
   background: passwords, caches, histories, cookies. */
PUBLIC void
NET_DestroyEvidence()
{
	int32 oldSize = 0;

	/* Handle authorizations */
	NET_RemoveAllAuthorizations();
	
	/* Handle global history */
	GH_ClearGlobalHistory();

	/* Handle Session History */

	/* Handle cookies */
	NET_RemoveAllCookies();

	/* Handle disk cache */
	oldSize = NET_GetDiskCacheSize();
	NET_SetDiskCacheSize(0); /* zero it out */
	NET_SetDiskCacheSize(oldSize); /* set it back up */

	/* Handle memory cache */
	oldSize = NET_GetMemoryCacheSize();
	NET_SetMemoryCacheSize(0); /* zero it out */
	NET_SetMemoryCacheSize(oldSize); /* set it back up */
}
#endif

#ifdef PROFILE
#pragma profile off
#endif

