/* -*- Mode: C; tab-width: 4 -*-
   mimei.c --- MIME parser, version 2 (see mimei.h).
   Copyright � 1997 Netscape Communications Corporation, all rights reserved.
   Created: Jamie Zawinski <jwz@netscape.com>, 15-May-96.
 */

#ifdef MIME_SECURITY
#include "secpkcs7.h"
#endif /* MIME_SECURITY */

#include "mimeobj.h"	/*  MimeObject (abstract)							*/
#include "mimecont.h"	/*   |--- MimeContainer (abstract)					*/
#include "mimemult.h"	/*   |     |--- MimeMultipart (abstract)			*/
#include "mimemmix.h"	/*   |     |     |--- MimeMultipartMixed			*/
#include "mimemdig.h"	/*   |     |     |--- MimeMultipartDigest			*/
#ifdef MIME_TO_HTML		/*   |     |     |                                  */
#include "mimempar.h"	/*   |     |     |--- MimeMultipartParallel			*/
#include "mimemalt.h"	/*   |     |     |--- MimeMultipartAlternative		*/
#include "mimemrel.h"	/*   |     |     |--- MimeMultipartRelated			*/
#include "mimemapl.h"	/*   |     |     |--- MimeMultipartAppleDouble		*/
#endif /* MIME_TO_HTML       |     |     |                                  */
#include "mimesun.h"	/*   |     |     |--- MimeSunAttachment				*/
#ifdef MIME_SECURITY	/*   |     |     |                                  */
#include "mimemsig.h"	/*   |     |     |--- MimeMultipartSigned (abstract)*/
#include "mimempkc.h"	/*   |     |          |--- MimeMultipartSignedPKCS7	*/
#include "mimecryp.h"	/*   |     |--- MimeEncrypted (abstract)			*/
#include "mimepkcs.h"	/*   |     |     |--- MimeEncryptedPKCS7			*/
#endif /* MIME_SECURITY      |     |                                        */
#include "mimemsg.h"	/*   |     |--- MimeMessage							*/
#include "mimeunty.h"	/*   |     |--- MimeUntypedText						*/
#include "mimeleaf.h"	/*   |--- MimeLeaf (abstract)						*/
#include "mimetext.h"	/*   |     |--- MimeInlineText (abstract)			*/
#include "mimetpla.h"	/*   |     |     |--- MimeInlineTextPlain			*/
#ifdef MIME_TO_HTML     /*   |     |     |                                  */
#include "mimethtm.h"	/*   |     |     |--- MimeInlineTextHTML			*/
#include "mimetric.h"	/*   |     |     |--- MimeInlineTextRichtext		*/
#include "mimetenr.h"	/*   |     |     |     |--- MimeInlineTextEnriched	*/
#ifndef AKBAR			/*   |     |     |                                  */
#include "mimevcrd.h"   /*   |     |     |--------- MimeInlineTextVCard		*/
#endif /* !AKBAR             |     |                                        */
#endif /* MIME_TO_HTML       |     |                                        */
#ifdef MIME_TO_HTML     /*   |     |                                        */
#include "mimeiimg.h"	/*   |     |--- MimeInlineImage						*/
#endif /* MIME_TO_HTML       |     |                                        */
#include "mimeeobj.h"	/*   |     |--- MimeExternalObject					*/
#ifdef MIME_TO_HTML     /*   |                                              */
#include "mimeebod.h"	/*   |--- MimeExternalBody							*/
#endif /* MIME_TO_HTML                                                      */


/* ==========================================================================
   Allocation and destruction
   ==========================================================================
 */

static int mime_classinit(MimeObjectClass *class);

MimeObject *
mime_new (MimeObjectClass *class, MimeHeaders *hdrs,
		  const char *override_content_type)
{
  int size = class->instance_size;
  MimeObject *object;
  int status;

  /* Some assertions to verify that this isn't random junk memory... */
  XP_ASSERT(class->class_name && XP_STRLEN(class->class_name) > 0);
  XP_ASSERT(size > 0 && size < 1000);

  if (!class->class_initialized)
	{
	  status = mime_classinit(class);
	  if (status < 0) return 0;
	}

  XP_ASSERT(class->initialize && class->finalize);

  if (hdrs)
	{
	  hdrs = MimeHeaders_copy (hdrs);
	  if (!hdrs) return 0;
	}

  object = (MimeObject *) XP_ALLOC(size);
  if (!object) return 0;

  XP_MEMSET(object, 0, size);
  object->class = class;
  object->headers = hdrs;

  if (override_content_type && *override_content_type)
	object->content_type = XP_STRDUP(override_content_type);

  status = class->initialize(object);
  if (status < 0)
	{
	  class->finalize(object);
	  XP_FREE(object);
	  return 0;
	}

  return object;
}

void
mime_free (MimeObject *object)
{
# ifdef DEBUG__
  int i, size = object->class->instance_size;
  uint32 *array = (uint32*) object;
# endif /* DEBUG */

  object->class->finalize(object);

# ifdef DEBUG__
  for (i = 0; i < (size / sizeof(*array)); i++)
	array[i] = (uint32) 0xDEADBEEF;
# endif /* DEBUG */

  XP_FREE(object);
}


MimeObjectClass *
mime_find_class (const char *content_type, MimeHeaders *hdrs,
				 MimeDisplayOptions *opts, XP_Bool exact_match_p)
{
  MimeObjectClass *class = 0;

#ifdef MIME_LEAF_TEXT
  XP_Bool leaf_p = (opts && opts->write_leaf_p);
#endif /* MIME_LEAF_TEXT */

  if (!content_type || !*content_type ||
	  !strcasecomp(content_type, "text"))  /* with no / in the type */
	class = (MimeObjectClass *)&mimeUntypedTextClass;

  /* Subtypes of text...
   */
  else if (!strncasecomp(content_type,			"text/", 5))
	{
#ifdef MIME_LEAF_TEXT
	  if (leaf_p)
		class = (MimeObjectClass *)&mimeInlineTextPlainClass;
	  else
#endif /* MIME_LEAF_TEXT */

#ifdef MIME_TO_HTML
	  if      (!strcasecomp(content_type+5,		"html"))
		class = (MimeObjectClass *)&mimeInlineTextHTMLClass;
	  else if (!strcasecomp(content_type+5,		"enriched"))
		class = (MimeObjectClass *)&mimeInlineTextEnrichedClass;
	  else if (!strcasecomp(content_type+5,		"richtext"))
		class = (MimeObjectClass *)&mimeInlineTextRichtextClass;
	  else if (!strcasecomp(content_type+5,		"plain"))
		class = (MimeObjectClass *)&mimeInlineTextPlainClass;
# ifndef AKBAR
	  else if (!strcasecomp(content_type+5,		"x-vcard"))
		class = (MimeObjectClass *)&mimeInlineTextVCardClass;
# endif /* !AKBAR */
	  else
#endif /* MIME_TO_HTML */
		if (!exact_match_p)
		class = (MimeObjectClass *)&mimeInlineTextPlainClass;
	}

  /* Subtypes of multipart...
   */
  else if (!strncasecomp(content_type,			"multipart/", 10))
	{
	  if (!strcasecomp(content_type+10,	"mixed"))
		class = (MimeObjectClass *)&mimeMultipartMixedClass;
	  else if (!strcasecomp(content_type+10,	"digest"))
		class = (MimeObjectClass *)&mimeMultipartDigestClass;

#ifdef MIME_LEAF_TEXT
	  else if (leaf_p)
		class = (MimeObjectClass *)&mimeMultipartMixedClass;
#endif /* MIME_LEAF_TEXT */

#ifdef MIME_TO_HTML
	  else if (!strcasecomp(content_type+10,	"alternative"))
		class = (MimeObjectClass *)&mimeMultipartAlternativeClass;
	  else if (!strcasecomp(content_type+10,	"related"))
		class = (MimeObjectClass *)&mimeMultipartRelatedClass;
	  else if (!strcasecomp(content_type+10,	"appledouble") ||
			   !strcasecomp(content_type+10,	"header-set"))
		class = (MimeObjectClass *)&mimeMultipartAppleDoubleClass;
	  else if (!strcasecomp(content_type+10,	"parallel"))
		class = (MimeObjectClass *)&mimeMultipartParallelClass;
#endif /* MIME_TO_HTML */

#ifdef MIME_SECURITY
	  else if (!strcasecomp(content_type+10,	"signed"))
		{
		  /* Check that the "protocol" and "micalg" parameters are ones we
			 know about. */
		  char *ct = (hdrs
					  ? MimeHeaders_get(hdrs, HEADER_CONTENT_TYPE,
										FALSE, FALSE)
					  : 0);
		  char *proto = (ct
						 ? MimeHeaders_get_parameter(ct, PARAM_PROTOCOL)
						 : 0);
		  char *micalg = (ct
						  ? MimeHeaders_get_parameter(ct, PARAM_MICALG)
						 : 0);
		  if (proto && !strcasecomp(proto, APPLICATION_XPKCS7_SIGNATURE) &&
			  micalg && (!strcasecomp(micalg, PARAM_MICALG_MD5) ||
						 !strcasecomp(micalg, PARAM_MICALG_SHA1) ||
						 !strcasecomp(micalg, PARAM_MICALG_SHA1_2) ||
						 !strcasecomp(micalg, PARAM_MICALG_SHA1_3) ||
						 !strcasecomp(micalg, PARAM_MICALG_SHA1_4) ||
						 !strcasecomp(micalg, PARAM_MICALG_SHA1_5) ||
						 !strcasecomp(micalg, PARAM_MICALG_MD2)))
			class = (MimeObjectClass *)&mimeMultipartSignedPKCS7Class;
/*		  else if (proto && !strcasecomp(proto, "application/pgp-encrypted"))
			class = (MimeObjectClass *)&...; */
		  else
			class = 0;
		  FREEIF(proto);
		  FREEIF(micalg);
		  FREEIF(ct);
		}
#endif /* MIME_SECURITY */

	  if (!class && !exact_match_p)
		/* Treat all unknown multipart subtypes as "multipart/mixed" */
		class = (MimeObjectClass *)&mimeMultipartMixedClass;
	}

  /* Subtypes of message...
   */
  else if (!strncasecomp(content_type,			"message/", 8))
	{
	  if      (!strcasecomp(content_type+8,		"rfc822") ||
			   !strcasecomp(content_type+8,		"news"))
		class = (MimeObjectClass *)&mimeMessageClass;
#ifdef MIME_TO_HTML
	  else if (!strcasecomp(content_type+8,		"external-body")
# ifdef MIME_LEAF_TEXT
			   && !leaf_p
# endif /* MIME_LEAF_TEXT */
			   )
		class = (MimeObjectClass *)&mimeExternalBodyClass;
#endif /* MIME_TO_HTML */
	  else if (!strcasecomp(content_type+8,		"partial"))
		/* I guess these are most useful as externals, for now... */
		class = (MimeObjectClass *)&mimeExternalObjectClass;
	  else if (!exact_match_p)
		/* Treat all unknown message subtypes as "text/plain" */
		class = (MimeObjectClass *)&mimeInlineTextPlainClass;
	}

  /* The magic image types which we are able to display internally...
   */
#ifdef MIME_TO_HTML
  else if ((!strcasecomp(content_type,			IMAGE_GIF)  ||
			!strcasecomp(content_type,			IMAGE_JPG) ||
			!strcasecomp(content_type,			IMAGE_PJPG) ||
			!strcasecomp(content_type,			IMAGE_XBM)  ||
			!strcasecomp(content_type,			IMAGE_XBM2) ||
			!strcasecomp(content_type,			IMAGE_XBM3))
# ifdef MIME_LEAF_TEXT
		   && !leaf_p
# endif /* MIME_LEAF_TEXT */
		   )
	class = (MimeObjectClass *)&mimeInlineImageClass;
#endif /* MIME_TO_HTML */

  /* The encrypted types we know about...
   */
#ifdef MIME_SECURITY
  else if (!strcasecomp(content_type,			APPLICATION_XPKCS7_MIME))
	class = (MimeObjectClass *)&mimeEncryptedPKCS7Class;
#endif /* MIME_SECURITY */

  /* A few types which occur in the real world and which we would otherwise
	 treat as non-text types (which would be bad) without this special-case...
   */
  else if (!strcasecomp(content_type,			APPLICATION_PGP) ||
		   !strcasecomp(content_type,			APPLICATION_PGP2))
	class = (MimeObjectClass *)&mimeInlineTextPlainClass;

  else if (!strcasecomp(content_type,			SUN_ATTACHMENT))
	class = (MimeObjectClass *)&mimeSunAttachmentClass;

  /* Everything else gets represented as a clickable link.
   */
  else if (!exact_match_p)
	class = (MimeObjectClass *)&mimeExternalObjectClass;

 if (!exact_match_p)
   XP_ASSERT(class);
  if (!class) return 0;

  /* If the `Show Attachments as Links' kludge is on, now would be the time
	 to change our mind... */
  if (opts && opts->no_inline_p
# ifdef MIME_LEAF_TEXT
	  && !opts->write_leaf_p
# endif /* MIME_LEAF_TEXT */
	  )
	{
	  if (mime_subclass_p(class, (MimeObjectClass *)&mimeInlineTextClass))
		{
		  /* It's a text type.  Write it only if it's the *first* part
			 that we're writing, and then only if it has no "filename"
			 specified (the assumption here being, if it has a filename,
			 it wasn't simply typed into the text field -- it was actually
			 an attached document.) */
		  if (opts->state && opts->state->first_part_written_p)
			class = (MimeObjectClass *)&mimeExternalObjectClass;
		  else
			{
			  /* If there's a name, then write this as an attachment. */
			  char *name = (hdrs ? MimeHeaders_get_name(hdrs) : 0);
			  if (name)
				class = (MimeObjectClass *)&mimeExternalObjectClass;
			  FREEIF(name);
			}

		  if (opts->state)
			opts->state->first_part_written_p = TRUE;
		}
	  else if (mime_subclass_p(class,(MimeObjectClass *)&mimeContainerClass) &&
			   !mime_subclass_p(class,(MimeObjectClass *)&mimeMessageClass))
		/* Multipart subtypes are ok, except for messages; descend into
		   multiparts, and defer judgement.

		   Encrypted blobs are just like other containers (make the crypto
		   layer invisible, and treat them as simple containers.  So there's
		   no easy way to save encrypted data directly to disk; it will tend
		   to always be wrapped inside a message/rfc822.  That's ok.)
		 */
		;
	  else if (opts &&
			   opts->part_to_load &&
			   mime_subclass_p(class,(MimeObjectClass *)&mimeMessageClass))
		/* Descend into messages only if we're looking for a specific sub-part.
		 */
		;
	  else
		{
		  /* Anything else, and display it as a link (and cause subsequent
			 text parts to also be displayed as links.) */
		  class = (MimeObjectClass *)&mimeExternalObjectClass;
		  if (opts->state)
			opts->state->first_part_written_p = TRUE;
		}
	}

  XP_ASSERT(class);

  if (class && !class->class_initialized)
	{
	  int status = mime_classinit(class);
	  if (status < 0) return 0;
	}

  return class;
}


MimeObject *
mime_create (const char *content_type, MimeHeaders *hdrs,
			 MimeDisplayOptions *opts)
{
  /* If there is no Content-Disposition header, or if the Content-Disposition
	 is ``inline'', then we display the part inline (and let mime_find_class()
	 decide how.)

	 If there is any other Content-Disposition (either ``attachment'' or some
	 disposition that we don't recognise) then we always display the part as
	 an external link, by using MimeExternalObject to display it.

	 But Content-Disposition is ignored for all containers except `message'.
	 (including multipart/mixed, and multipart/digest.)  It's not clear if
	 this is to spec, but from a usability standpoint, I think it's necessary.
   */

  MimeObjectClass *class = 0;
  char *content_disposition = 0;
  MimeObject *obj = 0;
  char *override_content_type = 0;


  /* Those pinheads at Microsoft send out all attachments with a content-type
	 of application/octet-stream.  So, if we have an octet-stream attachment,
	 try to guess what type it really is based on the file extension.  I HATE
	 that we have to do this...
   */
  if (hdrs && opts && opts->file_type_fn &&
	  (!content_type ||
	   !strcasecomp(content_type, APPLICATION_OCTET_STREAM) ||
	   !strcasecomp(content_type, UNKNOWN_CONTENT_TYPE)))
	{
	  char *name = MimeHeaders_get_name(hdrs);
	  if (name)
		{
		  override_content_type = opts->file_type_fn (name,
													  opts->stream_closure);
		  FREEIF(name);

		  if (override_content_type &&
			  !strcasecomp(override_content_type, UNKNOWN_CONTENT_TYPE))
			FREEIF(override_content_type);

		  if (override_content_type)
			content_type = override_content_type;
		}
	}


  class = mime_find_class(content_type, hdrs, opts, FALSE);

  XP_ASSERT(class);
  if (!class) goto FAIL;

  if (opts && opts->part_to_load)
	/* Always ignore Content-Disposition when we're loading some specific
	   sub-part (which may be within some container that we wouldn't otherwise
	   descend into, if the container itself had a Content-Disposition of
	   `attachment'. */
	content_disposition = 0;

  else if (mime_subclass_p(class,(MimeObjectClass *)&mimeContainerClass) &&
		   !mime_subclass_p(class,(MimeObjectClass *)&mimeMessageClass))
	/* Ignore Content-Disposition on all containers except `message'.
	   That is, Content-Disposition is ignored for multipart/mixed objects,
	   but is obeyed for message/rfc822 objects. */
	content_disposition = 0;

  else
  {
	/* change content-Disposition for vcards to be inline so */
	/* we can see a nice html display */
#ifndef AKBAR
	if (mime_subclass_p(class,(MimeObjectClass *)&mimeInlineTextVCardClass))
		StrAllocCopy(content_disposition, "inline");
	else
#endif /* !AKBAR */
		content_disposition = (hdrs
							   ? MimeHeaders_get(hdrs, HEADER_CONTENT_DISPOSITION,
												 TRUE, FALSE)
							   : 0);
  }

  if (!content_disposition ||
	  !strcasecomp(content_disposition, "inline"))
	;	/* Use the class we've got. */
  else
	class = (MimeObjectClass *)&mimeExternalObjectClass;

  FREEIF(content_disposition);
  obj = mime_new (class, hdrs, content_type);

 FAIL:

  /* If we decided to ignore the content-type in the headers of this object
	 (see above) then make sure that our new content-type is stored in the
	 object itself.  (Or free it, if we're in an out-of-memory situation.)
   */
  if (override_content_type)
	{
	  if (obj)
		{
		  FREEIF(obj->content_type);
		  obj->content_type = override_content_type;
		}
	  else
		{
		  XP_FREE(override_content_type);
		}
	}

  return obj;
}



static int mime_classinit_1(MimeObjectClass *class, MimeObjectClass *target);

static int
mime_classinit(MimeObjectClass *class)
{
  int status;
  if (class->class_initialized)
	return 0;

  XP_ASSERT(class->class_initialize);
  if (!class->class_initialize)
	return -1;

  /* First initialize the superclass.
   */
  if (class->superclass && !class->superclass->class_initialized)
	{
	  status = mime_classinit(class->superclass);
	  if (status < 0) return status;
	}

  /* Now run each of the superclass-init procedures in turn,
	 parentmost-first. */
  status = mime_classinit_1(class, class);
  if (status < 0) return status;

  /* Now we're done. */
  class->class_initialized = TRUE;
  return 0;
}

static int
mime_classinit_1(MimeObjectClass *class, MimeObjectClass *target)
{
  int status;
  if (class->superclass)
	{
	  status = mime_classinit_1(class->superclass, target);
	  if (status < 0) return status;
	}
  return class->class_initialize(target);
}


XP_Bool
mime_subclass_p(MimeObjectClass *child, MimeObjectClass *parent)
{
  if (child == parent)
	return TRUE;
  else if (!child->superclass)
	return FALSE;
  else
	return mime_subclass_p(child->superclass, parent);
}

XP_Bool
mime_typep(MimeObject *obj, MimeObjectClass *class)
{
  return mime_subclass_p(obj->class, class);
}



/* URL munging
 */


/* Returns a string describing the location of the part (like "2.5.3").
   This is not a full URL, just a part-number.
 */
char *
mime_part_address(MimeObject *obj)
{
  if (!obj->parent)
	return XP_STRDUP("0");
  else
	{
	  /* Find this object in its parent. */
	  int32 i, j = -1;
	  char buf [20];
	  char *higher = 0;
	  MimeContainer *cont = (MimeContainer *) obj->parent;
	  XP_ASSERT(mime_typep(obj->parent,
						   (MimeObjectClass *)&mimeContainerClass));
	  for (i = 0; i < cont->nchildren; i++)
		if (cont->children[i] == obj)
		  {
			j = i+1;
			break;
		  }
	  if (j == -1)
		{
		  XP_ASSERT(0);
		  return 0;
		}

	  XP_SPRINTF(buf, "%ld", j);
	  if (obj->parent->parent)
		{
		  higher = mime_part_address(obj->parent);
		  if (!higher) return 0;  /* MK_OUT_OF_MEMORY */
		}

	  if (!higher)
		return XP_STRDUP(buf);
	  else
		{
		  char *s = XP_ALLOC(XP_STRLEN(higher) + XP_STRLEN(buf) + 3);
		  if (!s)
			{
			  XP_FREE(higher);
			  return 0;  /* MK_OUT_OF_MEMORY */
			}
		  XP_STRCPY(s, higher);
		  XP_STRCAT(s, ".");
		  XP_STRCAT(s, buf);
		  XP_FREE(higher);
		  return s;
		}
	}
}


#ifdef MIME_SECURITY
/* Asks whether the given object is one of the cryptographically signed
   or encrypted objects that we know about.  (MimeMessageClass uses this
   to decide if the headers need to be presented differently.)
 */
XP_Bool
mime_crypto_object_p(MimeHeaders *hdrs, XP_Bool clearsigned_counts)
{
  char *ct;
  MimeObjectClass *class;

  if (!hdrs) return FALSE;

  ct = MimeHeaders_get (hdrs, HEADER_CONTENT_TYPE, TRUE, FALSE);
  if (!ct) return FALSE;

  /* Rough cut -- look at the string before doing a more complex comparison. */
  if (strcasecomp(ct, MULTIPART_SIGNED) &&
	  strncasecomp(ct, "application/", 12))
	{
	  XP_FREE(ct);
	  return FALSE;
	}

  /* It's a candidate for being a crypto object.  Let's find out for sure... */
  class = mime_find_class (ct, hdrs, 0, TRUE);
  XP_FREE(ct);

  if (class == ((MimeObjectClass *)&mimeEncryptedPKCS7Class))
	return TRUE;
  else if (clearsigned_counts &&
		   class == ((MimeObjectClass *)&mimeMultipartSignedPKCS7Class))
	return TRUE;
  else
	return FALSE;
}


/* Whether the given object has written out the HTML version of its headers
   in such a way that it will have a "crypto stamp" next to the headers.  If
   this is true, then the child must write out its HTML slightly differently
   to take this into account...
 */
XP_Bool
mime_crypto_stamped_p(MimeObject *obj)
{
  XP_ASSERT(obj);
  if (!obj) return FALSE;
  if (mime_typep (obj, (MimeObjectClass *) &mimeMessageClass))
	return ((MimeMessage *) obj)->crypto_stamped_p;
  else
	return FALSE;
}


/* Tells whether the given MimeObject is a message which has been encrypted
   or signed.  (Helper for MIME_GetMessageCryptoState()). 
 */
void
mime_get_crypto_state (MimeObject *obj,
					   XP_Bool *signed_ret,
					   XP_Bool *encrypted_ret,
					   XP_Bool *signed_ok_ret,
					   XP_Bool *encrypted_ok_ret)
{
  XP_Bool signed_p, encrypted_p;

  XP_ASSERT(signed_ret || encrypted_ret || signed_ok_ret || encrypted_ok_ret);
  if (signed_ret) *signed_ret = FALSE;
  if (encrypted_ret) *encrypted_ret = FALSE;
  if (signed_ok_ret) *signed_ok_ret = FALSE;
  if (encrypted_ok_ret) *encrypted_ok_ret = FALSE;

  XP_ASSERT(obj);
  if (!obj) return;

  if (!mime_typep (obj, (MimeObjectClass *) &mimeMessageClass))
	return;

  signed_p = ((MimeMessage *) obj)->crypto_msg_signed_p;
  encrypted_p = ((MimeMessage *) obj)->crypto_msg_encrypted_p;

  if (signed_ret)
	*signed_ret = signed_p;
  if (encrypted_ret)
	*encrypted_ret = encrypted_p;

  if ((signed_p || encrypted_p) &&
	  (signed_ok_ret || encrypted_ok_ret))
	{
	  SEC_PKCS7ContentInfo *encrypted_ci = 0;
	  SEC_PKCS7ContentInfo *signed_ci = 0;
	  int32 decode_error = 0, verify_error = 0;
	  char *addr = mime_part_address(obj);

	  mime_find_security_info_of_part(addr, obj,
									  (void *) &encrypted_ci,
									  (void *) &signed_ci,
									  0,  /* email_addr */
									  &decode_error, &verify_error);

	  if (encrypted_p && encrypted_ok_ret)
		*encrypted_ok_ret = (encrypted_ci && decode_error >= 0);

	  if (signed_p && signed_ok_ret)
		*signed_ok_ret = (verify_error >= 0 && decode_error >= 0);

	  if (encrypted_ci)
		SEC_PKCS7DestroyContentInfo(encrypted_ci);
	  if (signed_ci)
		SEC_PKCS7DestroyContentInfo(signed_ci);
	  FREEIF(addr);
	}
}


/* How the crypto code tells the MimeMessage object what the crypto stamp
   on it says. */
void
mime_set_crypto_stamp(MimeObject *obj, XP_Bool signed_p, XP_Bool encrypted_p)
{
  if (!obj) return;
  if (mime_typep (obj, (MimeObjectClass *) &mimeMessageClass))
	{
	  MimeMessage *msg = (MimeMessage *) obj;
	  if (!msg->crypto_msg_signed_p)
		msg->crypto_msg_signed_p = signed_p;
	  if (!msg->crypto_msg_encrypted_p)
		msg->crypto_msg_encrypted_p = encrypted_p;

	  /* If the `decrypt_p' option is on, record whether any decryption has
		 actually occurred. */
	  if (encrypted_p &&
		  obj->options &&
		  obj->options->decrypt_p &&
		  obj->options->state)
		{
#ifdef MIME_TO_HTML
		  /* decrypt_p and write_html_p are incompatible. */
		  XP_ASSERT(!obj->options->write_html_p);
#endif /* MIME_TO_HTML */
		  obj->options->state->decrypted_p = TRUE;
		}

	  return;  /* continue up the tree?  I think that's not a good idea. */
	}

  if (obj->parent)
	mime_set_crypto_stamp(obj->parent, signed_p, encrypted_p);
}
#endif /* MIME_SECURITY */



/* Puts a part-number into a URL.  If append_p is true, then the part number
   is appended to any existing part-number already in that URL; otherwise,
   it replaces it.
 */
char *
mime_set_url_part(const char *url, char *part, XP_Bool append_p)
{
  const char *part_begin = 0;
  const char *part_end = 0;
  XP_Bool got_q = FALSE;
  const char *s;
  char *result;

  for (s = url; *s; s++)
	{
	  if (*s == '?')
		{
		  got_q = TRUE;
		  if (!strncasecomp(s, "?part=", 6))
			part_begin = (s += 6);
		}
	  else if (got_q && *s == '&' && !strncasecomp(s, "&part=", 6))
		part_begin = (s += 6);

	  if (part_begin)
		{
		  for (; (*s && *s != '?' && *s != '&'); s++)
			;
		  part_end = s;
		  break;
		}
	}

  result = (char *) XP_ALLOC(XP_STRLEN(url) + XP_STRLEN(part) + 10);
  if (!result) return 0;

  if (part_begin)
	{
	  if (append_p)
		{
		  XP_MEMCPY(result, url, part_end - url);
		  result [part_end - url]     = '.';
		  result [part_end - url + 1] = 0;
		}
	  else
		{
		  XP_MEMCPY(result, url, part_begin - url);
		  result [part_begin - url] = 0;
		}
	}
  else
	{
	  XP_STRCPY(result, url);
	  if (got_q)
		XP_STRCAT(result, "&part=");
	  else
		XP_STRCAT(result, "?part=");
	}

  XP_STRCAT(result, part);

  if (part_end && *part_end)
	XP_STRCAT(result, part_end);

  /* Semi-broken kludge to omit a trailing "?part=0". */
  {
	int L = XP_STRLEN(result);
	if (L > 6 &&
		(result[L-7] == '?' || result[L-7] == '&') &&
		!XP_STRCMP("part=0", result + L - 6))
	  result[L-7] = 0;
  }

  return result;
}


/* Given a part ID, looks through the MimeObject tree for a sub-part whose ID
   number matches, and returns the MimeObject (else NULL.)
   (part is not a URL -- it's of the form "1.3.5".)
 */
MimeObject *
mime_address_to_part(const char *part, MimeObject *obj)
{
  /* Note: this is an N^2 operation, but the number of parts in a message
	 shouldn't ever be large enough that this really matters... */

  XP_Bool match;

  if (!part || !*part)
	{
	  match = !obj->parent;
	}
  else
	{
	  char *part2 = mime_part_address(obj);
	  if (!part2) return 0;  /* MK_OUT_OF_MEMORY */
	  match = !XP_STRCMP(part, part2);
	  XP_FREE(part2);
	}

  if (match)
	{
	  /* These are the droids we're looking for. */
	  return obj;
	}
  else if (!mime_typep(obj, (MimeObjectClass *) &mimeContainerClass))
	{
	  /* Not a container, pull up, pull up! */
	  return 0;
	}
  else
	{
	  int32 i;
	  MimeContainer *cont = (MimeContainer *) obj;
	  for (i = 0; i < cont->nchildren; i++)
		{
		  MimeObject *o2 = mime_address_to_part(part, cont->children[i]);
		  if (o2) return o2;
		}
	  return 0;
	}
}


/* Given a part ID, looks through the MimeObject tree for a sub-part whose ID
   number matches; if one is found, returns the Content-Name of that part.
   Else returns NULL.  (part is not a URL -- it's of the form "1.3.5".)
 */
char *
mime_find_suggested_name_of_part(const char *part, MimeObject *obj)
{
  char *result = 0;

  obj = mime_address_to_part(part, obj);
  if (!obj) return 0;

  result = (obj->headers ? MimeHeaders_get_name(obj->headers) : 0);

#ifdef MIME_TO_HTML
  /* If this part doesn't have a name, but this part is one fork of an
	 AppleDouble, and the AppleDouble itself has a name, then use that. */
  if (!result &&
	  obj->parent &&
	  obj->parent->headers &&
	  mime_typep(obj->parent,
				 (MimeObjectClass *) &mimeMultipartAppleDoubleClass))
	result = MimeHeaders_get_name(obj->parent->headers);

  /* Else, if this part is itself an AppleDouble, and one of its children
	 has a name, then use that (check data fork first, then resource.) */
  if (!result &&
	  mime_typep(obj, (MimeObjectClass *) &mimeMultipartAppleDoubleClass))
	{
	  MimeContainer *cont = (MimeContainer *) obj;
	  if (cont->nchildren > 1 &&
		  cont->children[1] &&
		  cont->children[1]->headers)
		result = MimeHeaders_get_name(cont->children[1]->headers);

	  if (!result &&
		  cont->nchildren > 0 &&
		  cont->children[0] &&
		  cont->children[0]->headers)
		result = MimeHeaders_get_name(cont->children[0]->headers);
	}
#endif /* MIME_TO_HTML */

  /* Ok, now we have the suggested name, if any.
	 Now we remove any extensions that correspond to the
	 Content-Transfer-Encoding.  For example, if we see the headers

		Content-Type: text/plain
		Content-Disposition: inline; filename=foo.text.uue
		Content-Transfer-Encoding: x-uuencode

	 then we would look up (in mime.types) the file extensions which are
	 associated with the x-uuencode encoding, find that "uue" is one of
	 them, and remove that from the end of the file name, thus returning
	 "foo.text" as the name.  This is because, by the time this file ends
	 up on disk, its content-transfer-encoding will have been removed;
	 therefore, we should suggest a file name that indicates that.
   */
  if (result && obj->encoding && *obj->encoding)
	{
	  int32 L = XP_STRLEN(result);
	  const char **exts = 0;

	  /* #### TOTAL FUCKING KLUDGE.
		 I'd like to ask the mime.types file, "what extensions correspond
		 to obj->encoding (which happens to be "x-uuencode") but doing that
		 in a non-sphagetti way would require brain surgery.  So, since
		 currently uuencode is the only content-transfer-encoding which we
		 understand which traditionally has an extension, we just special-
		 case it here!  Icepicks in my forehead!

		 Note that it's special-cased in a similar way in libmsg/compose.c.
	   */
	  if (!strcasecomp(obj->encoding, ENCODING_UUENCODE))
		{
		  static const char *uue_exts[] = { "uu", "uue", 0 };
		  exts = uue_exts;
		}

	  while (exts && *exts)
		{
		  const char *ext = *exts;
		  int32 L2 = XP_STRLEN(ext);
		  if (L > L2 + 1 &&							/* long enough */
			  result[L - L2 - 1] == '.' &&			/* '.' in right place*/
			  !strcasecomp(ext, result + (L - L2)))	/* ext matches */
			{
			  result[L - L2 - 1] = 0;		/* truncate at '.' and stop. */
			  break;
			}
		  exts++;
		}
	}

  return result;
}


#ifdef MIME_SECURITY
/* Given a part ID, looks through the MimeObject tree for a sub-part whose ID
   number matches; if one is found, and if it represents a PKCS7-encrypted
   object, returns information about the security status of that object.

   `part' is not a URL -- it's of the form "1.3.5" and is interpreted relative
   to the `obj' argument.

   Returned values:

     void **pkcs7_content_info_return
          this is the SEC_PKCS7ContentInfo* of the object.

     int32 *decode_error_return
          this is the error code, if any, that the security library returned
	      while trying to parse the PKCS7 data (if this is negative, then it
		  probably means the message was corrupt in some way.)

     int32 *verify_error_return
          this is the error code, if any, that the security library returned
  	      while trying to decrypt or verify or otherwise validate the data
		  (if this is negative, it might mean the message was corrupt, or might
		  mean the signature didn't match, or the cert was expired, or...)
  */
void
mime_find_security_info_of_part(const char *part, MimeObject *obj,
								void **pkcs7_encrypted_content_info_return,
								void **pkcs7_signed_content_info_return,
								char **sender_email_addr_return,
								int32 *decode_error_return,
								int32 *verify_error_return)
{
  obj = mime_address_to_part(part, obj);

  *pkcs7_encrypted_content_info_return = 0;
  *pkcs7_signed_content_info_return = 0;
  *decode_error_return = 0;
  *verify_error_return = 0;
  if (sender_email_addr_return)
	*sender_email_addr_return = 0;

  if (!obj)
	return;

  /* If someone asks for the security info of a message/rfc822 object,
	 instead give them the security info of its child (the body of the
	 message.)
   */
  if (mime_typep (obj, (MimeObjectClass *) &mimeMessageClass))
	{
	  MimeContainer *cont = (MimeContainer *) obj;
	  if (cont->nchildren >= 1)
		{
		  XP_ASSERT(cont->nchildren == 1);
		  obj = cont->children[0];
		}
	}

  /* "Object oriented?  Fuck that shit!  Pabst Blue Ribbon!" --Dennis Hopper */

  while (obj &&
		 (mime_typep(obj, (MimeObjectClass *) &mimeEncryptedPKCS7Class) ||
		  mime_typep(obj, (MimeObjectClass *) &mimeMultipartSignedPKCS7Class)))
	{
	  SEC_PKCS7ContentInfo *ci = 0;
	  int32 decode_error = 0, verify_error = 0;
	  char *sender = 0;

	  if (mime_typep(obj, (MimeObjectClass *) &mimeEncryptedPKCS7Class))
		(((MimeEncryptedPKCS7Class *) (obj->class))
		 ->get_content_info) (obj, &ci, &sender, &decode_error, &verify_error);

	  else if (mime_typep(obj,
						  (MimeObjectClass *) &mimeMultipartSignedPKCS7Class))
		(((MimeMultipartSignedPKCS7Class *) (obj->class))
		 ->get_content_info) (obj, &ci, &sender, &decode_error, &verify_error);

	  if (ci)
		{
		  if (SEC_PKCS7ContentIsEncrypted(ci))
			*pkcs7_encrypted_content_info_return = ci;
		  else
			*pkcs7_signed_content_info_return = ci;
		}

	  if (sender_email_addr_return)
		*sender_email_addr_return = sender;
	  else
		FREEIF(sender);

	  if (*decode_error_return >= 0)
		*decode_error_return = decode_error;

	  if (*verify_error_return >= 0)
		*verify_error_return = verify_error;


	  XP_ASSERT(mime_typep(obj, (MimeObjectClass *) &mimeContainerClass) &&
				((MimeContainer *) obj)->nchildren <= 1);

	  obj = ((((MimeContainer *) obj)->nchildren > 0)
			 ? ((MimeContainer *) obj)->children[0]
			 : 0);
	}
}
#endif /* MIME_SECURITY */


/* Parse the various "?" options off the URL and into the options struct.
 */
int
mime_parse_url_options(const char *url, MimeDisplayOptions *options)
{
  const char *q;
  MimeHeadersState default_headers = options->headers;

  if (!url || !*url) return 0;
  if (!options) return 0;

  q = XP_STRRCHR (url, '?');
  if (! q) return 0;
  q++;
  while (*q)
	{
	  const char *end, *value, *name_end;
	  for (end = q; *end && *end != '&'; end++)
		;
	  for (value = q; *value != '=' && value < end; value++)
		;
	  name_end = value;
	  if (value < end) value++;
	  if (name_end <= q)
		;
	  else if (!strncasecomp ("headers", q, name_end - q))
		{
		  if (end > value && !strncasecomp ("all", value, end - value))
			options->headers = MimeHeadersAll;
		  else if (end > value && !strncasecomp ("some", value, end - value))
			options->headers = MimeHeadersSome;
		  else if (end > value && !strncasecomp ("micro", value, end - value))
			options->headers = MimeHeadersMicro;
		  else if (end > value && !strncasecomp ("cite", value, end - value))
			options->headers = MimeHeadersCitation;
		  else if (end > value && !strncasecomp ("citation", value, end-value))
			options->headers = MimeHeadersCitation;
		  else
			options->headers = default_headers;
		}
	  else if (!strncasecomp ("part", q, name_end - q))
		{
		  FREEIF (options->part_to_load);
		  if (end > value)
			{
			  options->part_to_load = (char *) XP_ALLOC(end - value + 1);
			  if (!options->part_to_load)
				return MK_OUT_OF_MEMORY;
			  XP_MEMCPY(options->part_to_load, value, end-value);
			  options->part_to_load[end-value] = 0;
			}
		}
	  else if (!strncasecomp ("rot13", q, name_end - q))
		{
		  if (end <= value || !strncasecomp ("true", value, end - value))
			options->rot13_p = TRUE;
		  else
			options->rot13_p = FALSE;
		}
	  else if (!strncasecomp ("inline", q, name_end - q))
		{
		  if (end <= value || !strncasecomp ("true", value, end - value))
			options->no_inline_p = FALSE;
		  else
			options->no_inline_p = TRUE;
		}

	  q = end;
	  if (*q)
		q++;
	}


  /* Compatibility with the "?part=" syntax used in the old (Mozilla 2.0)
	 MIME parser.

	 Basically, the problem is that the old part-numbering code was totally
	 busted: here's a comparison of the old and new numberings with a pair
	 of hypothetical messages (one with a single part, and one with nested
	 containers.)
                               NEW:      OLD:  OR:
         message/rfc822
           image/jpeg           1         0     0

         message/rfc822
           multipart/mixed      1         0     0
             text/plain         1.1       1     1
             image/jpeg         1.2       2     2
             message/rfc822     1.3       -     3
               text/plain       1.3.1     3     -
             message/rfc822     1.4       -     4
               multipart/mixed  1.4.1     4     -
                 text/plain     1.4.1.1   4.1   -
                 image/jpeg     1.4.1.2   4.2   -
             text/plain         1.5       5     5

	 The "NEW" column is how the current code counts.  The "OLD" column is
	 what "?part=" references would do in 3.0b4 and earlier; you'll see that
	 you couldn't directly refer to the child message/rfc822 objects at all!
	 But that's when it got really weird, because if you turned on
	 "Attachments As Links" (or used a URL like "?inline=false&part=...")
	 then you got a totally different numbering system (seen in the "OR"
	 column.)  Gag!

	 So, the problem is, ClariNet had been using these part numbers in their
	 HTML news feeds, as a sleazy way of transmitting both complex HTML layouts
	 and images using NNTP as transport, without invoking HTTP.

	 The following clause is to provide some small amount of backward
	 compatibility.  By looking at that table, one can see that in the new
	 model, "part=0" has no meaning, and neither does "part=2" or "part=3"
	 and so on.

     "part=1" is ambiguous between the old and new way, as is any part
	 specification that has a "." in it.

	 So, the compatibility hack we do here is: if the part is "0", then map
	 that to "1".  And if the part is >= "2", then prepend "1." to it (so that
	 we map "2" to "1.2", and "3" to "1.3".)

	 This leaves the URLs compatible in the cases of:

	   = single part messages
	   = references to elements of a top-level multipart except the first

     and leaves them incompatible for:

	   = the first part of a top-level multipart
	   = all elements deeper than the outermost part

	 Life sucks when you don't properly think out things that end up turning
	 into de-facto standards...
   */
  if (options->part_to_load &&
	  !XP_STRCHR(options->part_to_load, '.'))		/* doesn't contain a dot */
	{
	  if (!XP_STRCMP(options->part_to_load, "0"))		/* 0 */
		{
		  XP_FREE(options->part_to_load);
		  options->part_to_load = XP_STRDUP("1");
		  if (!options->part_to_load)
			return MK_OUT_OF_MEMORY;
		}
	  else if (XP_STRCMP(options->part_to_load, "1"))	/* not 1 */
		{
		  const char *prefix = "1.";
		  char *s = (char *) XP_ALLOC(XP_STRLEN(options->part_to_load) +
									  XP_STRLEN(prefix) + 1);
		  if (!s) return MK_OUT_OF_MEMORY;
		  XP_STRCPY(s, prefix);
		  XP_STRCAT(s, options->part_to_load);
		  XP_FREE(options->part_to_load);
		  options->part_to_load = s;
		}
	}


  return 0;
}


/* Some output-generation utility functions...
 */

int
MimeOptions_write(MimeDisplayOptions *opt, char *data, int32 length,
				  XP_Bool user_visible_p)
{
  int status = 0;
  void* closure = 0;
  if (!opt || !opt->output_fn || !opt->state)
	return 0;

  closure = opt->output_closure;
  if (!closure) closure = opt->stream_closure;

  XP_ASSERT(opt->state->first_data_written_p);

  if (opt->state->separator_queued_p && user_visible_p)
	{
	  opt->state->separator_queued_p = FALSE;
	  if (opt->state->separator_suppressed_p)
		opt->state->separator_suppressed_p = FALSE;
	  else
		{
		  char sep[] = "<HR WIDTH=\"90%\" SIZE=4>";
		  int status = opt->output_fn(sep, XP_STRLEN(sep), closure);
		  opt->state->separator_suppressed_p = FALSE;
		  if (status < 0) return status;
		}
	}
  if (user_visible_p)
	opt->state->separator_suppressed_p = FALSE;

  if (length > 0)
	{
	  status = opt->output_fn(data, length, closure);
	  if (status < 0) return status;
	}

  return 0;
}

int
MimeObject_write(MimeObject *obj, char *output, int32 length,
				 XP_Bool user_visible_p)
{
  if (!obj->output_p) return 0;

  if (!obj->options->state->first_data_written_p)
	{
	  int status = MimeObject_output_init(obj, 0);
	  if (status < 0) return status;
	  XP_ASSERT(obj->options->state->first_data_written_p);
	}

  return MimeOptions_write(obj->options, output, length, user_visible_p);
}

int
MimeObject_write_separator(MimeObject *obj)
{
  if (obj->options && obj->options->state)
	obj->options->state->separator_queued_p = TRUE;
  return 0;
}

int
MimeObject_output_init(MimeObject *obj, const char *content_type)
{
  if (obj &&
	  obj->options &&
	  obj->options->state &&
	  !obj->options->state->first_data_written_p)
	{
	  int status;
	  const char *charset = 0;
	  char *name = 0, *x_mac_type = 0, *x_mac_creator = 0;

	  if (!obj->options->output_init_fn)
		{
		  obj->options->state->first_data_written_p = TRUE;
		  return 0;
		}

	  if (obj->headers)
		{
		  char *ct;
		  name = MimeHeaders_get_name(obj->headers);

		  ct = MimeHeaders_get(obj->headers, HEADER_CONTENT_TYPE,
							   FALSE, FALSE);
		  if (ct)
			{
			  x_mac_type   = MimeHeaders_get_parameter(ct,PARAM_X_MAC_TYPE);
			  x_mac_creator= MimeHeaders_get_parameter(ct,PARAM_X_MAC_CREATOR);
			  XP_FREE(ct);
			}
		}

	  if (mime_typep(obj, (MimeObjectClass *) &mimeInlineTextClass))
		charset = ((MimeInlineText *)obj)->charset;

	  if (!content_type)
		content_type = obj->content_type;
	  if (!content_type)
		content_type = TEXT_PLAIN;

	  status = obj->options->output_init_fn (content_type, charset, name,
											 x_mac_type, x_mac_creator,
											 obj->options->stream_closure);
	  FREEIF(name);
	  FREEIF(x_mac_type);
	  FREEIF(x_mac_creator);
	  obj->options->state->first_data_written_p = TRUE;
	  return status;
	}
  return 0;
}
