/* -*- Mode:C; tab-width: 4 -*- */

#include "xp.h"
#include "mime.h"
#include "mkutils.h"


/* for XP_GetString() */
#include "xpgetstr.h"
extern int MK_MSG_NO_RETURN_ADDRESS;
extern int MK_MSG_NO_RETURN_ADDRESS_AT;
extern int MK_MSG_NO_RETURN_ADDRESS_DOT;
extern int MK_OUT_OF_MEMORY;
extern int MK_SIGNATURE_TOO_LONG;
extern int MK_SIGNATURE_TOO_WIDE;


#define mime_enc_7bit 0
#define mime_enc_8bit 1
#define mime_enc_qp   2
#define mime_enc_b64  3

/* build a mailto: url address given a to field
 *
 * returns a malloc'd string
 */
PUBLIC char *
MIME_BuildMailtoURLAddress(const char * to)
{
	char * rv=0;

	StrAllocCopy(rv,"mailto:");
	StrAllocCat(rv, to);

	return(rv);
}


/* build a news: url address given a partial news post
 * URL and the newsgroups line
 *
 * returns a malloc'd string
 */
PUBLIC char *
MIME_BuildNewspostURLAddress(const char *partial_newspost_url,
							 const char *newsgroups)
{
	char * rv=0;

	if (!partial_newspost_url)
	  partial_newspost_url = "news:";
	StrAllocCopy(rv, partial_newspost_url);
	StrAllocCat(rv, newsgroups);

	return(rv);
}


/*
	Returns the appropriate contents of a From: field of a mail message
	originating from the current user.  This calls FE_UsersFullName()
	and FE_UsersMailAddress() and correctly munges the values.
	
	A new string is returned, which you must free when you're done with it.

    An email address must be provided but a user name is optional.  If no
    address has been entered or other errors are encountered return NULL.
*/
PUBLIC char *
MIME_MakeFromField (void)
{
    return MSG_MakeFullAddress(FE_UsersFullName(), FE_UsersMailAddress());
}


/* Ok, this doesn't really belong here... */

void
MISC_ValidateSignature (MWContext *context, const char *signature)
{
  int max_columns = 0;
  int column = 0;
  int newlines = 0;
  const char *sig = signature;

  /* ensure that sig is valid */
  if (!sig) return;

  for (; *sig; sig++)
    {
      if (*sig == '\n' || *sig == '\r')
		{
		  /* Treat CR, LF, CRLF, and LFCR as a single line-break. */
		  if ((sig[0] == '\n' && sig[1] == '\r') ||
			  (sig[0] == '\r' && sig[1] == '\n'))
			sig++;

		  if (column > max_columns)
			max_columns = column;
		  newlines++;
		  column = 0;
		}
      else
		{
		  column++;
		}
    }
  /* If the last line doesn't end in a newline, pretend it does. */
  if (column != 0)
    newlines++;
  if (column > max_columns)
    max_columns = column;

  /* If the signature begins with "--" followed by whitespace or a newline,
	 that means that the pseudo-standard sig delimiter "-- \n" is actually
	 in the file, so don't count that as a "line". */
  if (signature [0] == '-' &&
	  signature [1] == '-' &&
	  (signature [2] == ' ' ||
	   (signature [2] == '\012' ||
		signature [2] == '\015')))
	newlines--;

  if (newlines > 4)
	{
	  FE_Alert (context, XP_GetString (MK_SIGNATURE_TOO_LONG));
	}
  else if (max_columns > 79)
	{
	  FE_Alert (context, XP_GetString (MK_SIGNATURE_TOO_WIDE));
	}
}

int
MISC_ValidateReturnAddress (MWContext *context, const char *addr)
{
  char *at;
  char *dot;
  char *fmt = 0;

  if (addr)
	while (XP_IS_SPACE (*addr))
	  addr++;

  if (!addr || !*addr)
	{
	  FE_Alert (context, XP_GetString (MK_MSG_NO_RETURN_ADDRESS));
#ifdef XP_MAC
      /*FE_EditPreference(PREF_EmailAddress); someone broke it */
#endif
	  return -1;
	}

  at = XP_STRRCHR (addr, '@');
  if (!at)
	{
	  fmt = XP_GetString (MK_MSG_NO_RETURN_ADDRESS_AT);
	  goto FAIL;
	}
  dot = XP_STRCHR (at, '.');
  if (!dot)
	{
	  fmt = XP_GetString (MK_MSG_NO_RETURN_ADDRESS_DOT);
	  goto FAIL;
	}

  return 0;

FAIL:
  {
	char *buf = (char *)XP_ALLOC (XP_STRLEN (addr) + XP_STRLEN (fmt) + 20);
	if (!buf) return MK_OUT_OF_MEMORY;
	XP_SPRINTF (buf, fmt, addr);
	FE_Alert (context, buf);
	return -1;
  }
}
