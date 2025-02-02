/* -*- Mode: C; tab-width: 4 -*-
   mimecryp.c --- definition of the MimeEncrypted class (see mimei.h)
   Copyright � 1997 Netscape Communications Corporation, all rights reserved.
   Created: Jamie Zawinski <jwz@netscape.com>, 11-Aug-96.
 */

#include "mimecryp.h"

#define MIME_SUPERCLASS mimeContainerClass
MimeDefClass(MimeEncrypted, MimeEncryptedClass, mimeEncryptedClass,
			 &MIME_SUPERCLASS);

static int MimeEncrypted_initialize (MimeObject *);
static void MimeEncrypted_finalize (MimeObject *);
static int MimeEncrypted_parse_begin (MimeObject *);
static int MimeEncrypted_parse_buffer (char *, int32, MimeObject *);
static int MimeEncrypted_parse_line (char *, int32, MimeObject *);
static int MimeEncrypted_parse_decoded_buffer (char *, int32, MimeObject *);
static int MimeEncrypted_parse_eof (MimeObject *, XP_Bool);
static int MimeEncrypted_parse_end (MimeObject *, XP_Bool);
static int MimeEncrypted_add_child (MimeObject *, MimeObject *);

static int MimeHandleDecryptedOutput (const char *, int32, void *);
static int MimeHandleDecryptedOutputLine (char *, int32, MimeObject *);
static int MimeEncrypted_close_headers (MimeObject *);
static int MimeEncrypted_emit_buffered_child(MimeObject *);

extern MimeObjectClass mimeMultipartClass;

static int
MimeEncryptedClassInitialize(MimeEncryptedClass *class)
{
  MimeObjectClass    *oclass = (MimeObjectClass *)    class;
  MimeContainerClass *cclass = (MimeContainerClass *) class;

  XP_ASSERT(!oclass->class_initialized);
  oclass->initialize   = MimeEncrypted_initialize;
  oclass->finalize     = MimeEncrypted_finalize;
  oclass->parse_begin  = MimeEncrypted_parse_begin;
  oclass->parse_buffer = MimeEncrypted_parse_buffer;
  oclass->parse_line   = MimeEncrypted_parse_line;
  oclass->parse_eof    = MimeEncrypted_parse_eof;
  oclass->parse_end    = MimeEncrypted_parse_end;

  cclass->add_child    = MimeEncrypted_add_child;

  class->parse_decoded_buffer = MimeEncrypted_parse_decoded_buffer;

  return 0;
}


static int
MimeEncrypted_initialize (MimeObject *obj)
{
  /* This is an abstract class; it shouldn't be directly instanciated. */
  XP_ASSERT(obj->class != (MimeObjectClass *) &mimeEncryptedClass);

  return ((MimeObjectClass*)&MIME_SUPERCLASS)->initialize(obj);
}


static int
MimeEncrypted_parse_begin (MimeObject *obj)
{
  MimeEncrypted *enc = (MimeEncrypted *) obj;
  MimeDecoderData *(*fn) (int (*) (const char*, int32, void*), void*) = 0;

  XP_ASSERT(!enc->crypto_closure);
  if (enc->crypto_closure)
	return -1;

  enc->crypto_closure = (((MimeEncryptedClass *) obj->class)
						 ->crypto_init) (obj, MimeHandleDecryptedOutput, obj);
  if (!enc->crypto_closure)
	return -1;


  /* (Mostly duplicated from MimeLeaf, see comments in mimecryp.h.)
     Initialize a decoder if necessary.
   */
  if (!obj->encoding)
	;
  else if (!strcasecomp(obj->encoding, ENCODING_BASE64))
	fn = &MimeB64DecoderInit;
  else if (!strcasecomp(obj->encoding, ENCODING_QUOTED_PRINTABLE))
	fn = &MimeQPDecoderInit;
  else if (!strcasecomp(obj->encoding, ENCODING_UUENCODE) ||
		   !strcasecomp(obj->encoding, ENCODING_UUENCODE2) ||
		   !strcasecomp(obj->encoding, ENCODING_UUENCODE3) ||
		   !strcasecomp(obj->encoding, ENCODING_UUENCODE4))
	fn = &MimeUUDecoderInit;
  if (fn)
	{
	  enc->decoder_data =
		fn (/* The (int (*) ...) cast is to turn the `void' argument
			   into `MimeObject'. */
			((int (*) (const char *, int32, void *))
			 ((MimeEncryptedClass *)obj->class)->parse_decoded_buffer),
			obj);

	  if (!enc->decoder_data)
		return MK_OUT_OF_MEMORY;
	}

  return ((MimeObjectClass*)&MIME_SUPERCLASS)->parse_begin(obj);
}


static int
MimeEncrypted_parse_buffer (char *buffer, int32 size, MimeObject *obj)
{
  /* (Duplicated from MimeLeaf, see comments in mimecryp.h.)
   */

  MimeEncrypted *enc = (MimeEncrypted *) obj;

  XP_ASSERT(!obj->closed_p);
  if (obj->closed_p) return -1;

  /* Don't consult output_p here, since at this point we're behaving as a
	 simple container object -- the output_p decision should be made by
	 the child of this object. */

  if (enc->decoder_data)
	return MimeDecoderWrite (enc->decoder_data, buffer, size);
  else
	return ((MimeEncryptedClass *)obj->class)->parse_decoded_buffer (buffer,
																	 size,
																	 obj);
}


static int
MimeEncrypted_parse_line (char *line, int32 length, MimeObject *obj)
{
  XP_ASSERT(0);
  /* This method shouldn't ever be called. */
  return -1;
}

static int
MimeEncrypted_parse_decoded_buffer (char *buffer, int32 size, MimeObject *obj)
{
  MimeEncrypted *enc = (MimeEncrypted *) obj;
  return
	((MimeEncryptedClass *) obj->class)->crypto_write (buffer, size,
													   enc->crypto_closure);
}


static int
MimeEncrypted_parse_eof (MimeObject *obj, XP_Bool abort_p)
{
  int status = 0;
  MimeEncrypted *enc = (MimeEncrypted *) obj;

  if (obj->closed_p) return 0;
  XP_ASSERT(!obj->parsed_p);

  /* (Duplicated from MimeLeaf, see comments in mimecryp.h.)
     Close off the decoder, to cause it to give up any buffered data that
	 it is still holding.
   */
  if (enc->decoder_data)
	{
	  int status = MimeDecoderDestroy(enc->decoder_data, FALSE);
	  enc->decoder_data = 0;
	  if (status < 0) return status;
	}


  /* If there is still data in the ibuffer, that means that the last
	 *decrypted* line of this part didn't end in a newline; so push it out
	 anyway (this means that the parse_line method will be called with a
	 string with no trailing newline, which isn't the usual case.)  */
  if (!abort_p &&
	  obj->ibuffer_fp > 0)
	{
	  int status = MimeHandleDecryptedOutputLine (obj->ibuffer,
												  obj->ibuffer_fp, obj);
	  obj->ibuffer_fp = 0;
	  if (status < 0)
		{
		  obj->closed_p = TRUE;
		  return status;
		}
	}


  /* Now run the superclass's parse_eof, which (because we've already taken
	 care of ibuffer in a way appropriate for this class, immediately above)
	 will ony set closed_p to true.
   */
  status = ((MimeObjectClass*)&MIME_SUPERCLASS)->parse_eof (obj, abort_p);
  if (status < 0) return status;


  /* Now close off the underlying crypto module.  At this point, the crypto
	 module has all of the input.  (DecoderDestroy called parse_decoded_buffer
	 which called crypto_write, with the last of the data.)
   */
  if (enc->crypto_closure)
	{
	  status =
		((MimeEncryptedClass *) obj->class)->crypto_eof (enc->crypto_closure,
														 abort_p);
	  if (status < 0 && !abort_p)
		return status;
	}

  /* Now we have the entire child part in the part buffer.
	 We are now able to verify its signature, emit a blurb, and then
	 emit the part.
   */
  if (abort_p)
	return 0;
  else
	return MimeEncrypted_emit_buffered_child (obj);
}


static int
MimeEncrypted_parse_end (MimeObject *obj, XP_Bool abort_p)
{
  /* Don't free these yet -- keep them around for the lifetime of the
	 MIME object, so that we can get at the security info of sub-parts
	 of the currently-displayed message. */
#if 0
  MimeEncrypted *enc = (MimeEncrypted *) obj;
  if (enc->crypto_closure)
	{
	  ((MimeEncryptedClass *) obj->class)->crypto_free (enc->crypto_closure);
	  enc->crypto_closure = 0;
	}
#endif /* 0 */

  return ((MimeObjectClass*)&MIME_SUPERCLASS)->parse_end (obj, abort_p);
}


static void
MimeEncrypted_cleanup (MimeObject *obj, XP_Bool finalizing_p)
{
  MimeEncrypted *enc = (MimeEncrypted *) obj;

  if (enc->part_buffer)
	{
	  MimePartBufferDestroy(enc->part_buffer);
	  enc->part_buffer = 0;
	}

  if (finalizing_p && enc->crypto_closure)
	{
	  /* Don't free these until this object is really going away -- keep them
		 around for the lifetime of the MIME object, so that we can get at the
		 security info of sub-parts of the currently-displayed message. */
	  ((MimeEncryptedClass *) obj->class)->crypto_free (enc->crypto_closure);
	  enc->crypto_closure = 0;
	}

  /* (Duplicated from MimeLeaf, see comments in mimecryp.h.)
	 Free the decoder data, if it's still around. */
  if (enc->decoder_data)
	{
	  MimeDecoderDestroy(enc->decoder_data, TRUE);
	  enc->decoder_data = 0;
	}

  if (enc->hdrs)
	{
	  MimeHeaders_free(enc->hdrs);
	  enc->hdrs = 0;
	}
}


static void
MimeEncrypted_finalize (MimeObject *obj)
{
  MimeEncrypted_cleanup (obj, TRUE);
  ((MimeObjectClass*)&MIME_SUPERCLASS)->finalize (obj);
}


static int
MimeHandleDecryptedOutput (const char *buf, int32 buf_size,
						   void *output_closure)
{
  /* This method is invoked by the underlying decryption module.
	 The module is assumed to return a MIME object, and its associated
	 headers.  For example, if a text/plain document was encrypted,
	 the encryption module would return the following data:

	   Content-Type: text/plain

	   Decrypted text goes here.

	 This function will then extract a header block (up to the first
	 blank line, as usual) and will then handle the included data as
	 appropriate.
   */
  MimeObject *obj = (MimeObject *) output_closure;

  /* Is it truly safe to use ibuffer here?  I think so... */
  return msg_LineBuffer (buf, buf_size,
						 &obj->ibuffer, &obj->ibuffer_size, &obj->ibuffer_fp,
						 TRUE,
						 ((int (*) (char *, int32, void *))
						  /* This cast is to turn void into MimeObject */
						  MimeHandleDecryptedOutputLine),
						 obj);
}

static int
MimeHandleDecryptedOutputLine (char *line, int32 length, MimeObject *obj)
{
  /* Largely the same as MimeMessage_parse_line (the other MIME container
	 type which contains exactly one child.)
   */
  MimeEncrypted *enc = (MimeEncrypted *) obj;
  int status = 0;

  XP_ASSERT(line && *line);
  if (!line || !*line) return -1;

  /* If we're supposed to write this object, but aren't supposed to convert
	 it to HTML, simply pass it through unaltered. */
  if (obj->output_p &&
	  obj->options &&
#ifdef MIME_TO_HTML
	  !obj->options->write_html_p &&
#endif /* MIME_TO_HTML */
#ifdef MIME_LEAF_TEXT
	  !obj->options->write_leaf_p &&
#endif /* MIME_LEAF_TEXT */
	  obj->options->output_fn)
	return MimeObject_write(obj, line, length, TRUE);

  /* If we already have a child object in the buffer, then we're done parsing
	 headers, and all subsequent lines get passed to the inferior object
	 without further processing by us.  (Our parent will stop feeding us
	 lines when this MimeMessage part is out of data.)
   */
  if (enc->part_buffer)
	return MimePartBufferWrite (enc->part_buffer, line, length);

  /* Otherwise we don't yet have a child object in the buffer, which means
	 we're not done parsing our headers yet.
   */
  if (!enc->hdrs)
	{
	  enc->hdrs = MimeHeaders_new();
	  if (!enc->hdrs) return MK_OUT_OF_MEMORY;
	}

  status = MimeHeaders_parse_line(line, length, enc->hdrs);
  if (status < 0) return status;

  /* If this line is blank, we're now done parsing headers, and should
	 examine our content-type to create our "body" part.
   */
  if (*line == CR || *line == LF)
	{
	  status = MimeEncrypted_close_headers(obj);
	  if (status < 0) return status;
	}

  return 0;
}

static int
MimeEncrypted_close_headers (MimeObject *obj)
{
  MimeEncrypted *enc = (MimeEncrypted *) obj;

  XP_ASSERT(!enc->part_buffer);
  if (enc->part_buffer) return -1;
  enc->part_buffer = MimePartBufferCreate();
  if (!enc->part_buffer)
	return MK_OUT_OF_MEMORY;

  return 0;
}


static int
MimeEncrypted_add_child (MimeObject *parent, MimeObject *child)
{
  MimeContainer *cont = (MimeContainer *) parent;
  XP_ASSERT(parent && child);
  if (!parent || !child) return -1;

  /* Encryption containers can only have one child. */
  XP_ASSERT(cont->nchildren == 0);
  if (cont->nchildren != 0) return -1;

  return ((MimeContainerClass*)&MIME_SUPERCLASS)->add_child (parent, child);
}


static int
MimeEncrypted_emit_buffered_child(MimeObject *obj)
{
  MimeEncrypted *enc = (MimeEncrypted *) obj;
  int status = 0;
  char *ct = 0;
  MimeObject *body;

  XP_ASSERT(enc->crypto_closure);

#ifdef MIME_TO_HTML
  /* Emit some HTML saying whether the signature was cool.
	 But don't emit anything if in FO_QUOTE_MESSAGE mode.

	 Also, don't emit anything if the enclosed object is itself a signed
	 object -- in the case of an encrypted object which contains a signed
	 object, we only emit the HTML once (since the normal way of encrypting
	 and signing is to nest the signature inside the crypto envelope.)
   */
  if (enc->crypto_closure &&
	  obj->options &&
	  obj->options->headers != MimeHeadersCitation &&
	  obj->options->write_html_p &&
	  obj->options->output_fn &&
	  !mime_crypto_object_p(enc->hdrs, TRUE))
	{
	  char *html = (((MimeEncryptedClass *) obj->class)->crypto_generate_html
					(enc->crypto_closure));
	  if (!html) return -1; /* MK_OUT_OF_MEMORY? */
	  
	  status = MimeObject_write(obj, html, XP_STRLEN(html), FALSE);
	  XP_FREE(html);
	  if (status < 0) return status;


	  /* Now that we have written out the crypto stamp, the outermost header
		 block is well and truly closed.  If this is in fact the outermost
		 message, then run the post_header_html_fn now.
	   */
	  if (obj->options &&
		  obj->options->state &&
		  obj->options->generate_post_header_html_fn &&
		  !obj->options->state->post_header_html_run_p)
		{
		  MimeHeaders *outer_headers;
		  MimeObject *p;
		  for (p = obj; p->parent; p = p->parent)
			outer_headers = p->headers;
		  XP_ASSERT(obj->options->state->first_data_written_p);
		  html = obj->options->generate_post_header_html_fn(NULL,
													obj->options->html_closure,
															obj->headers);
		  obj->options->state->post_header_html_run_p = TRUE;
		  if (html)
			{
			  status = MimeObject_write(obj, html, XP_STRLEN(html), FALSE);
			  XP_FREE(html);
			  if (status < 0) return status;
			}
		}
	}
  else if (enc->crypto_closure &&
		   obj->options &&
		   obj->options->decrypt_p)
	{
	  /* Do this just to cause `mime_set_crypto_stamp' to be called, and to
		 cause the various `decode_error' and `verify_error' slots to be set:
		 we don't actually use the returned HTML, because we're not emitting
		 HTML.  It's maybe not such a good thing that the determination of
		 whether it was encrypted or not is tied up with generating HTML,
		 but oh well. */
	  char *html = (((MimeEncryptedClass *) obj->class)->crypto_generate_html
					(enc->crypto_closure));
	  FREEIF(html);
	}
#endif /* MIME_TO_HTML */

  if (enc->hdrs)
	ct = MimeHeaders_get (enc->hdrs, HEADER_CONTENT_TYPE, TRUE, FALSE);
  body = mime_create((ct ? ct : TEXT_PLAIN), enc->hdrs, obj->options);
  
#ifdef MIME_DRAFTS
  if (obj->options->decompose_file_p) {
	if (mime_typep (body, (MimeObjectClass*) &mimeMultipartClass) )
		obj->options->is_multipart_msg = TRUE;
	else if (obj->options->decompose_file_init_fn)
		obj->options->decompose_file_init_fn(obj->options->stream_closure,
											 enc->hdrs);
  }
#endif /* MIME_DRAFTS */

  FREEIF(ct);

  if (!body) return MK_OUT_OF_MEMORY;
  status = ((MimeContainerClass *) obj->class)->add_child (obj, body);
  if (status < 0)
	{
	  mime_free(body);
	  return status;
	}

  /* Now that we've added this new object to our list of children,
	 start its parser going. */
  status = body->class->parse_begin(body);
  if (status < 0) return status;

  /* If this object (or the parent) is being output, then by definition
	 the child is as well.  (This is only necessary because this is such
	 a funny sort of container...)
   */
  if (!body->output_p &&
	  (obj->output_p ||
	   (obj->parent && obj->parent->output_p)))
	body->output_p = TRUE;

  /* If the body is being written raw (not as HTML) then make sure to
	 write its headers as well. */
  if (body->output_p && obj->output_p
#ifdef MIME_TO_HTML
	  && !obj->options->write_html_p
#endif /* MIME_TO_HTML */
#ifdef MIME_LEAF_TEXT
	  && !obj->options->write_leaf_p
#endif /* MIME_LEAF_TEXT */
	  )
	{
	  status = MimeObject_write(body, "", 0, FALSE);  /* initialize */
	  if (status < 0) return status;
	  status = MimeHeaders_write_raw_headers(body->headers, obj->options,
											 FALSE);
	  if (status < 0) return status;
	}

  if (enc->part_buffer)  /* part_buffer is 0 for 0-length encrypted data. */

#ifdef MIME_DRAFTS
    if (obj->options->decompose_file_p && !obj->options->is_multipart_msg)
		status = MimePartBufferRead(enc->part_buffer,
								/* The (int (*) ...) cast is to turn the `void'
								   argument into `MimeObject'. */
								((int (*) (char *, int32, void *))
								obj->options->decompose_file_output_fn),
								obj->options->stream_closure);
    else
#endif /* MIME_DRAFTS */

	status = MimePartBufferRead(enc->part_buffer,
								/* The (int (*) ...) cast is to turn the `void'
								   argument into `MimeObject'. */
								((int (*) (char *, int32, void *))
								 body->class->parse_buffer),
								body);
  if (status < 0) return status;

  /* The child has been fully processed.  Close it off.
   */
  status = body->class->parse_eof(body, FALSE);
  if (status < 0) return status;

  status = body->class->parse_end(body, FALSE);
  if (status < 0) return status;

#ifdef MIME_DRAFTS
  if (obj->options->decompose_file_p && !obj->options->is_multipart_msg)
	  obj->options->decompose_file_close_fn(obj->options->stream_closure);
#endif /* MIME_DRAFTS */

  /* Put out a separator after every encrypted object. */
  status = MimeObject_write_separator(obj);
  if (status < 0) return status;

  MimeEncrypted_cleanup (obj, FALSE);

  return 0;
}
