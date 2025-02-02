/* -*- Mode: C; tab-width: 4 -*-
   mimehdrs.h --- external interface to the MIME header parser, version 2.
   Copyright � 1997 Netscape Communications Corporation, all rights reserved.
   Created: Jamie Zawinski <jwz@netscape.com>, 15-May-96.
 */

#ifndef _MIMEHDRS_H_
#define _MIMEHDRS_H_

#include "libmime.h"

/* This file defines the interface to message-header parsing and formatting
   code, including conversion to HTML.
 */



/* Other structs defined later in this file.
 */

/* Creation and destruction.
 */
extern MimeHeaders *MimeHeaders_new (void);
extern void MimeHeaders_free (MimeHeaders *);
extern MimeHeaders *MimeHeaders_copy (MimeHeaders *);


/* Feed this method the raw data from which you would like a header
   block to be parsed, one line at a time.  Feed it a blank line when
   you're done.  Returns negative on allocation-related failure.
 */
extern int MimeHeaders_parse_line (const char *buffer, int32 size,
								   MimeHeaders *hdrs);


/* Converts a MimeHeaders object into HTML, by writing to the provided
   output function.
 */
extern int MimeHeaders_write_headers_html (MimeHeaders *hdrs,
										   MimeDisplayOptions *opt);

/* Writes the headers as text/plain.
   This writes out a blank line after the headers, unless
   dont_write_content_type is true, in which case the header-block
   is not closed off, and none of the Content- headers are written.
 */
extern int MimeHeaders_write_raw_headers (MimeHeaders *hdrs,
										  MimeDisplayOptions *opt,
										  XP_Bool dont_write_content_type);


/* For drawing the tables that represent objects that can't be displayed
   inline. */
extern int MimeHeaders_write_attachment_box(MimeHeaders *hdrs,
											MimeDisplayOptions *opt,
											const char *content_type,
											const char *encoding,
											const char *name,
											const char *name_url,
											const char *body);

/* Some crypto-related HTML-generated utility routines.
 */
extern char *MimeHeaders_open_crypto_stamp(void);
extern char *MimeHeaders_finish_open_crypto_stamp(void);
extern char *MimeHeaders_close_crypto_stamp(void);
extern char *MimeHeaders_make_crypto_stamp(XP_Bool encrypted_p,
										   XP_Bool signed_p,
										   XP_Bool good_p,
										   XP_Bool close_parent_stamp_p,
										   const char *stamp_url);


/* Does all the heuristic silliness to find the filename in the given headers.
 */
extern char *MimeHeaders_get_name(MimeHeaders *hdrs);


#endif /* _MIMEHDRS_H_ */
