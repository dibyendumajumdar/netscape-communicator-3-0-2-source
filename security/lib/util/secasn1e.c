/* -*- Mode: C; tab-width: 8 -*-
 *
 * Support for ENcoding ASN.1 data based on BER/DER (Basic/Distinguished
 * Encoding Rules).
 *
 * Copyright � 1996 Netscape Communications Corporation, all rights reserved.
 *
 * $Id: secasn1e.c,v 1.7.2.2 1997/04/26 00:02:05 jwz Exp $
 */

#include "secasn1.h"

typedef enum {
    beforeHeader,
    duringContents,
    duringGroup,
    duringSequence,
    afterContents,
    afterImplicit,
    afterInline,
    afterPointer,
    notInUse
} sec_asn1e_parse_place;

typedef enum {
    allDone,
    encodeError,
    keepGoing,
    needBytes
} sec_asn1e_parse_status;

typedef struct sec_asn1e_state_struct {
    SEC_ASN1EncoderContext *top;
    const SEC_ASN1Template *template;
    void *src;

    struct sec_asn1e_state_struct *parent;	/* aka prev */
    struct sec_asn1e_state_struct *child;	/* aka next */

    sec_asn1e_parse_place place;	/* where we are in encoding process */

    /*
     * XXX explain the next fields as clearly as possible...
     */
    unsigned char tag_modifiers;
    unsigned char tag_number;
    unsigned long underlying_kind;

    int depth;

    PRBool explicit,		/* we are handling an explicit header */
	   indefinite,		/* need end-of-contents */
	   is_string,		/* encoding a simple string or an ANY */
	   may_stream,		/* when streaming, do indefinite encoding */
	   optional;		/* omit field if it has no contents */
} sec_asn1e_state;

/*
 * An "outsider" will have an opaque pointer to this, created by calling
 * SEC_ASN1EncoderStart().  It will be passed back in to all subsequent
 * calls to SEC_ASN1EncoderUpdate() and related routines, and when done
 * it is passed to SEC_ASN1EncoderFinish().
 */
struct sec_EncoderContext_struct {
    PRArenaPool *our_pool;		/* for our internal allocs */

    sec_asn1e_state *current;
    sec_asn1e_parse_status status;

    PRBool streaming;
    PRBool from_buf;

    SEC_ASN1NotifyProc notify_proc;	/* call before/after handling field */
    void *notify_arg;			/* argument to notify_proc */
    PRBool during_notify;		/* true during call to notify_proc */

    SEC_ASN1WriteProc output_proc;	/* pass encoded bytes to this  */
    void *output_arg;			/* argument to that function */
};


static sec_asn1e_state *
sec_asn1e_push_state (SEC_ASN1EncoderContext *cx,
		      const SEC_ASN1Template *template,
		      void *src, PRBool new_depth)
{
    sec_asn1e_state *state, *new_state;

    state = cx->current;

    new_state = PORT_ArenaZAlloc (cx->our_pool, sizeof(*new_state));
    if (new_state == NULL) {
	cx->status = encodeError;
	return NULL;
    }

    new_state->top = cx;
    new_state->parent = state;
    new_state->template = template;
    new_state->place = notInUse;
    if (src != NULL)
	new_state->src = (char *)src + template->offset;

    if (state != NULL) {
	new_state->depth = state->depth;
	if (new_depth)
	    new_state->depth++;
	state->child = new_state;
    }

    cx->current = new_state;
    return new_state;
}


static void
sec_asn1e_scrub_state (sec_asn1e_state *state)
{
    /*
     * Some default "scrubbing".
     * XXX right set of initializations?
     */
    state->place = beforeHeader;
    state->indefinite = PR_FALSE;
}


static void
sec_asn1e_notify_before (SEC_ASN1EncoderContext *cx, void *src, int depth)
{
    if (cx->notify_proc == NULL)
	return;

    cx->during_notify = PR_TRUE;
    (* cx->notify_proc) (cx->notify_arg, PR_TRUE, src, depth);
    cx->during_notify = PR_FALSE;
}


static void
sec_asn1e_notify_after (SEC_ASN1EncoderContext *cx, void *src, int depth)
{
    if (cx->notify_proc == NULL)
	return;

    cx->during_notify = PR_TRUE;
    (* cx->notify_proc) (cx->notify_arg, PR_FALSE, src, depth);
    cx->during_notify = PR_FALSE;
}


static sec_asn1e_state *
sec_asn1e_init_state_based_on_template (sec_asn1e_state *state)
{
    PRBool explicit, is_string, may_stream, optional, universal;
    unsigned char tag_modifiers;
    unsigned long encode_kind, under_kind;
    unsigned long tag_number;


    encode_kind = state->template->kind;

    universal = ((encode_kind & SEC_ASN1_CLASS_MASK) == SEC_ASN1_UNIVERSAL)
		? PR_TRUE : PR_FALSE;

    explicit = (encode_kind & SEC_ASN1_EXPLICIT) ? PR_TRUE : PR_FALSE;
    encode_kind &= ~SEC_ASN1_EXPLICIT;

    optional = (encode_kind & SEC_ASN1_OPTIONAL) ? PR_TRUE : PR_FALSE;
    encode_kind &= ~SEC_ASN1_OPTIONAL;

    PORT_Assert (!(explicit && universal));	/* bad templates */

    may_stream = (encode_kind & SEC_ASN1_MAY_STREAM) ? PR_TRUE : PR_FALSE;
    encode_kind &= ~SEC_ASN1_MAY_STREAM;

    /* Just clear this to get it out of the way; we do not need it here */
    encode_kind &= ~SEC_ASN1_DYNAMIC;

    if ((encode_kind & (SEC_ASN1_POINTER | SEC_ASN1_INLINE)) || (!universal
							      && !explicit)) {
	const SEC_ASN1Template *subt;
	void *src;

	PORT_Assert ((encode_kind & (SEC_ASN1_ANY | SEC_ASN1_SKIP)) == 0);

	sec_asn1e_scrub_state (state);
	subt = SEC_ASN1GetSubtemplate (state->template, state->src, PR_TRUE);

	if (encode_kind & SEC_ASN1_POINTER) {
	    /* check that there are no extraneous bits */
	    PORT_Assert (encode_kind == SEC_ASN1_POINTER && !optional);
	    src = *(void **)state->src;
	    state->place = afterPointer;
	} else {
	    src = state->src;
	    if (encode_kind & SEC_ASN1_INLINE) {
		/* check that there are no extraneous bits */
		PORT_Assert (encode_kind == SEC_ASN1_INLINE && !optional);
		state->place = afterInline;
	    } else {
		/* Save the tag modifiers and tag number here before move
		   on to the next state in case this is a member of a SEQUENCE OF
		   hl
		 */
		state->tag_modifiers = encode_kind & SEC_ASN1_TAG_MASK & ~SEC_ASN1_TAGNUM_MASK;
		state->tag_number = encode_kind & SEC_ASN1_TAGNUM_MASK;
		
		state->place = afterImplicit;
		state->optional = optional;
	    }
	}

	state = sec_asn1e_push_state (state->top, subt, src, PR_FALSE);
	if (state == NULL)
	    return NULL;

	if (universal) {
	    /*
	     * This is a POINTER or INLINE; just init based on that
	     * and we are done.
	     */
	    return sec_asn1e_init_state_based_on_template (state);
	}

	/*
	 * This is an implicit, non-universal (meaning, application-private
	 * or context-specific) field.  This results in a "magic" tag but
	 * encoding based on the underlying type.  We pushed a new state
	 * that is based on the subtemplate (the underlying type), but
	 * now we will sort of alias it to give it some of our properties
	 * (tag, optional status, etc.).
	 */

	under_kind = state->template->kind;
	if (under_kind & SEC_ASN1_MAY_STREAM) {
	    may_stream = PR_TRUE;
	    under_kind &= ~SEC_ASN1_MAY_STREAM;
	}
    } else {
	under_kind = encode_kind;
    }

    /*
     * Sanity check that there are no unwanted bits marked in under_kind.
     * These bits were either removed above (after we recorded them) or
     * they simply should not be found (signalling a bad/broken template).
     * XXX is this the right set of bits to test here? (i.e. need to add
     * or remove any?)
     */
    PORT_Assert ((under_kind & (SEC_ASN1_EXPLICIT | SEC_ASN1_OPTIONAL
				| SEC_ASN1_SKIP | SEC_ASN1_INNER
				| SEC_ASN1_DYNAMIC | SEC_ASN1_MAY_STREAM
				| SEC_ASN1_INLINE | SEC_ASN1_POINTER)) == 0);

    if (encode_kind & SEC_ASN1_ANY) {
	PORT_Assert (encode_kind == under_kind);
	tag_modifiers = 0;
	tag_number = 0;
	is_string = PR_TRUE;
    } else {
	tag_modifiers = encode_kind & SEC_ASN1_TAG_MASK & ~SEC_ASN1_TAGNUM_MASK;
	/*
	 * XXX This assumes only single-octet identifiers.  To handle
	 * the HIGH TAG form we would need to do some more work, especially
	 * in how to specify them in the template, because right now we
	 * do not provide a way to specify more *tag* bits in encode_kind.
	 */
	tag_number = encode_kind & SEC_ASN1_TAGNUM_MASK;

	is_string = PR_FALSE;
	switch (under_kind & SEC_ASN1_TAGNUM_MASK) {
	  case SEC_ASN1_SET:
	    /*
	     * XXX A plain old SET (as opposed to a SET OF) is not implemented.
	     * If it ever is, remove this assert...
	     */
	    PORT_Assert ((under_kind & SEC_ASN1_GROUP) != 0);
	    /* fallthru */
	  case SEC_ASN1_SEQUENCE:
	    tag_modifiers |= SEC_ASN1_CONSTRUCTED;
	    break;
	  case SEC_ASN1_BIT_STRING:
	  case SEC_ASN1_OCTET_STRING:
	  case SEC_ASN1_PRINTABLE_STRING:
	  case SEC_ASN1_T61_STRING:
	  case SEC_ASN1_IA5_STRING:
	  case SEC_ASN1_UTC_TIME:
	  case SEC_ASN1_BMP_STRING: 
	    /*
	     * We do not yet know if we will be constructing the string,
	     * so we have to wait to do this final tag modification.
	     */
	    is_string = PR_TRUE;
	    break;
	}
    }

    state->tag_modifiers = tag_modifiers;
    state->tag_number = tag_number;
    state->underlying_kind = under_kind;
    state->explicit = explicit;
    state->may_stream = may_stream;
    state->is_string = is_string;
    state->optional = optional;

    sec_asn1e_scrub_state (state);

    return state;
}


static void
sec_asn1e_write_part (sec_asn1e_state *state,
		      const char *buf, unsigned long len,
		      SEC_ASN1EncodingPart part)
{
    SEC_ASN1EncoderContext *cx;

    cx = state->top;
    (* cx->output_proc) (cx->output_arg, buf, len, state->depth, part);
}


/*
 * XXX This assumes only single-octet identifiers.  To handle
 * the HIGH TAG form we would need to modify this interface and
 * teach it to properly encode the special form.
 */
static void
sec_asn1e_write_identifier_bytes (sec_asn1e_state *state, unsigned char value)
{
    char byte;

    byte = (char) value;
    sec_asn1e_write_part (state, &byte, 1, SEC_ASN1_Identifier);
}

static void
sec_asn1e_write_length_bytes (sec_asn1e_state *state, unsigned long value,
			      PRBool indefinite)
{
    int lenlen;
    unsigned char buf[sizeof(unsigned long) + 1];

    if (indefinite) {
	PORT_Assert (value == 0);
	buf[0] = 0x80;
	lenlen = 1;
    } else {
	lenlen = SEC_ASN1LengthLength (value);
	if (lenlen == 1) {
	    buf[0] = value;
	} else {
	    int i;

	    i = lenlen - 1;
	    buf[0] = 0x80 | i;
	    while (i) {
		buf[i--] = value;
		value >>= 8;
	    }
	    PORT_Assert (value == 0);
	}
    }

    sec_asn1e_write_part (state, (char *) buf, lenlen, SEC_ASN1_Length);
}


static void
sec_asn1e_write_contents_bytes (sec_asn1e_state *state,
				const char *buf, unsigned long len)
{
    sec_asn1e_write_part (state, buf, len, SEC_ASN1_Contents);
}


static void
sec_asn1e_write_end_of_contents_bytes (sec_asn1e_state *state)
{
    const char eoc[2] = {0, 0};

    sec_asn1e_write_part (state, eoc, 2, SEC_ASN1_EndOfContents);
}


static unsigned long
sec_asn1e_contents_length (const SEC_ASN1Template *template, void *src,
			   PRBool *noheaderp)
{
    unsigned long encode_kind, underlying_kind;
    PRBool explicit, optional, universal;
    unsigned long len;

    encode_kind = template->kind;

    universal = ((encode_kind & SEC_ASN1_CLASS_MASK) == SEC_ASN1_UNIVERSAL)
		? PR_TRUE : PR_FALSE;

    explicit = (encode_kind & SEC_ASN1_EXPLICIT) ? PR_TRUE : PR_FALSE;
    encode_kind &= ~SEC_ASN1_EXPLICIT;

    optional = (encode_kind & SEC_ASN1_OPTIONAL) ? PR_TRUE : PR_FALSE;
    encode_kind &= ~SEC_ASN1_OPTIONAL;

    PORT_Assert (!(explicit && universal));	/* bad templates */

    /* We have already decided we are not streaming. */
    encode_kind &= ~SEC_ASN1_MAY_STREAM;

    /* Just clear this to get it out of the way; we do not need it here */
    encode_kind &= ~SEC_ASN1_DYNAMIC;

    if ((encode_kind & (SEC_ASN1_POINTER | SEC_ASN1_INLINE)) || !universal) {

	/* XXX any bits we want to disallow (PORT_Assert against) here? */

	template = SEC_ASN1GetSubtemplate (template, src, PR_TRUE);

	if (encode_kind & SEC_ASN1_POINTER) {
	    /* check that there are no extraneous bits */
	    PORT_Assert (encode_kind == SEC_ASN1_POINTER && !optional);
	    src = *(void **)src;
	    if (src == NULL) {
		if (optional)
		    *noheaderp = PR_TRUE;
		else 
		    *noheaderp = PR_FALSE;
		return 0;
	    }
	} else if (encode_kind & SEC_ASN1_INLINE) {
	    /* check that there are no extraneous bits */
	    PORT_Assert (encode_kind == SEC_ASN1_INLINE && !optional);
	}

	src = (char *)src + template->offset;

	if (explicit) {
	    len = sec_asn1e_contents_length (template, src, noheaderp);
	    if (len == 0 && optional) {
		*noheaderp = PR_TRUE;
	    } else if (*noheaderp) {
		/*
		 * Okay, *we* do not want to add in a header, but our
		 * caller still does.
		 */
		*noheaderp = PR_FALSE;
	    } else {
		/*
		 * XXX The 1 below is the presumed length of the identifier;
		 * to support a high-tag-number this would need to be smarter.
		 */
		len += 1 + SEC_ASN1LengthLength (len);
	    }
	    return len;
	}

	underlying_kind = template->kind;
	underlying_kind &= ~SEC_ASN1_MAY_STREAM;

	/* XXX Should we recurse here? */
    } else {
	underlying_kind = encode_kind;
    }

    /* This is only used in decoding; it plays no part in encoding.  */
    if (underlying_kind & SEC_ASN1_SAVE) {
	/* check that there are no extraneous bits */
	PORT_Assert (underlying_kind == SEC_ASN1_SAVE);
	*noheaderp = PR_TRUE;
	return 0;
    }

    /* Having any of these bits is not expected here...  */
    PORT_Assert ((underlying_kind & (SEC_ASN1_EXPLICIT | SEC_ASN1_OPTIONAL
				     | SEC_ASN1_INLINE | SEC_ASN1_POINTER
				     | SEC_ASN1_DYNAMIC | SEC_ASN1_MAY_STREAM
				     | SEC_ASN1_SAVE | SEC_ASN1_SKIP)) == 0);

    switch (underlying_kind) {
      case SEC_ASN1_SEQUENCE_OF:
      case SEC_ASN1_SET_OF:
	{
	    const SEC_ASN1Template *tmpt;
	    void *sub_src;
	    unsigned long sub_len;
	    void **group;

	    len = 0;

	    group = *(void ***)src;
	    if (group == NULL)
		break;

	    tmpt = SEC_ASN1GetSubtemplate (template, src, PR_TRUE);

	    for (; *group != NULL; group++) {
		sub_src = (char *)(*group) + tmpt->offset;
		sub_len = sec_asn1e_contents_length (tmpt, sub_src, noheaderp);
		len += sub_len;
		/*
		 * XXX The 1 below is the presumed length of the identifier;
		 * to support a high-tag-number this would need to be smarter.
		 */
		if (!*noheaderp)
		    len += 1 + SEC_ASN1LengthLength (sub_len);
	    }
	}
	break;

      case SEC_ASN1_SEQUENCE:
      case SEC_ASN1_SET:
	{
	    const SEC_ASN1Template *tmpt;
	    void *sub_src;
	    unsigned long sub_len;

	    len = 0;
	    for (tmpt = template + 1; tmpt->kind; tmpt++) {
		sub_src = (char *)src + tmpt->offset;
		sub_len = sec_asn1e_contents_length (tmpt, sub_src, noheaderp);
		len += sub_len;
		/*
		 * XXX The 1 below is the presumed length of the identifier;
		 * to support a high-tag-number this would need to be smarter.
		 */
		if (!*noheaderp)
		    len += 1 + SEC_ASN1LengthLength (sub_len);
	    }
	}
	break;

      case SEC_ASN1_BIT_STRING:
	/* convert bit length to byte */
	len = (((SECItem *)src)->len + 7) >> 3;
	/* bit string contents involve an extra octet */
	if (len)
	    len++;
	break;

      default:
	len = ((SECItem *)src)->len;
	break;
    }

    if ((len == 0 && optional) || underlying_kind == SEC_ASN1_ANY)
	*noheaderp = PR_TRUE;
    else 
	*noheaderp = PR_FALSE;

    return len;
}


static void
sec_asn1e_write_header (sec_asn1e_state *state)
{
    unsigned long contents_length;
    unsigned char tag_number, tag_modifiers;

    PORT_Assert (state->place == beforeHeader);

    tag_number = state->tag_number;
    tag_modifiers = state->tag_modifiers;

    if (state->underlying_kind == SEC_ASN1_ANY) {
	state->place = duringContents;
	return;
    }

    if (state->top->streaming && state->may_stream
			      && (state->top->from_buf || !state->is_string)) {
	/*
	 * We need to put out an indefinite-length encoding.
	 */
	state->indefinite = PR_TRUE;
	/*
	 * The only universal types that can be constructed are SETs,
	 * SEQUENCEs, and strings; so check that it is one of those,
	 * or that it is not universal (e.g. context-specific).
	 */
	PORT_Assert ((tag_number == SEC_ASN1_SET)
		     || (tag_number == SEC_ASN1_SEQUENCE)
		     || ((tag_modifiers & SEC_ASN1_CLASS_MASK) != 0)
		     || state->is_string);
	tag_modifiers |= SEC_ASN1_CONSTRUCTED;
	contents_length = 0;
    } else {
	PRBool noheader;

	/*
	 * We are doing a definite-length encoding.  First we have to
	 * walk the data structure to calculate the entire contents length.
	 */
	contents_length = sec_asn1e_contents_length (state->template,
						     state->src, &noheader);
	/*
	 * We might be told explicitly not to put out a header.
	 * But it can also be the case, via a pushed subtemplate, that
	 * sec_asn1e_contents_length could not know that this field is
	 * really optional.  So check for that explicitly, too.
	 */
	if (noheader || (contents_length == 0 && state->optional)) {
	    state->place = afterContents;
	    return;
	}
    }	

    sec_asn1e_write_identifier_bytes (state, tag_number | tag_modifiers);
    sec_asn1e_write_length_bytes (state, contents_length, state->indefinite);

    if (contents_length == 0 && !state->indefinite) {
	/*
	 * If no real contents to encode, then we are done with this field.
	 */
	state->place = afterContents;
	return;
    }

    /*
     * An EXPLICIT is nothing but an outer header, which we have already
     * written.  Now we need to do the inner header and contents.
     */
    if (state->explicit) {
	state->place = afterContents;
	state = sec_asn1e_push_state (state->top,
				      SEC_ASN1GetSubtemplate(state->template,
							     state->src,
							     PR_TRUE),
				      state->src, PR_TRUE);
	if (state != NULL)
	    state = sec_asn1e_init_state_based_on_template (state);
	return;
    }

    switch (state->underlying_kind) {
      case SEC_ASN1_SET_OF:
      case SEC_ASN1_SEQUENCE_OF:
	/*
	 * We need to push a child to handle each member.
	 */
	{
	    void **group;
	    const SEC_ASN1Template *subt;

	    group = *(void ***)state->src;
	    if (group == NULL || *group == NULL) {
		/*
		 * Group is empty; we are done.
		 */
		state->place = afterContents;
		return;
	    }
	    state->place = duringGroup;
	    subt = SEC_ASN1GetSubtemplate (state->template, state->src,
					   PR_TRUE);
	    state = sec_asn1e_push_state (state->top, subt, *group, PR_TRUE);
	    if (state != NULL)
		state = sec_asn1e_init_state_based_on_template (state);
	}
	break;

      case SEC_ASN1_SEQUENCE:
      case SEC_ASN1_SET:
	/*
	 * We need to push a child to handle the individual fields.
	 */
	state->place = duringSequence;
	state = sec_asn1e_push_state (state->top, state->template + 1,
				      state->src, PR_TRUE);
	if (state != NULL) {
	    /*
	     * Do the "before" field notification.
	     */
	    sec_asn1e_notify_before (state->top, state->src, state->depth);
	    state = sec_asn1e_init_state_based_on_template (state);
	}
	break;

      default:
	/*
	 * I think we do not need to do anything else.
	 * XXX Correct?
	 */
	state->place = duringContents;
	break;
    }
}


static void
sec_asn1e_write_contents (sec_asn1e_state *state,
			  const char *buf, unsigned long len)
{
    PORT_Assert (state->place == duringContents);

    if (state->top->from_buf) {
	/*
	 * Probably they just turned on "take from buf", but have not
	 * yet given us any bytes.  If there is nothing in the buffer
	 * then we have nothing to do but return and wait.
	 */
	if (buf == NULL || len == 0) {
	    state->top->status = needBytes;
	    return;
	}
	/*
	 * We are streaming, reading from a passed-in buffer.
	 * This means we are encoding a simple string or an ANY.
	 * For the former, we need to put out a substring, with its
	 * own identifier and length.  For an ANY, we just write it
	 * out as is (our caller is required to ensure that it
	 * is a properly encoded entity).
	 */
	PORT_Assert (state->is_string);		/* includes ANY */
	if (state->underlying_kind != SEC_ASN1_ANY) {
	    unsigned char identifier;

	    /*
	     * Create the identifier based on underlying_kind.  We cannot
	     * use tag_number and tag_modifiers because this can be an
	     * implicitly encoded field.  In that case, the underlying
	     * substrings *are* encoded with their real tag.
	     */
	    identifier = state->underlying_kind & SEC_ASN1_TAG_MASK;
	    /*
	     * The underlying kind should just be a simple string; there
	     * should be no bits like CONTEXT_SPECIFIC or CONSTRUCTED set.
	     */
	    PORT_Assert ((identifier & SEC_ASN1_TAGNUM_MASK) == identifier);
	    /*
	     * Write out the tag and length for the substring.
	     */
	    sec_asn1e_write_identifier_bytes (state, identifier);
	    if (state->underlying_kind == SEC_ASN1_BIT_STRING) {
		char byte;
		/*
		 * Assume we have a length in bytes but we need to output
		 * a proper bit string.  This interface only works for bit
		 * strings that are full multiples of 8.  If support for
		 * real, variable length bit strings is needed then the
		 * caller will have to know to pass in a bit length instead
		 * of a byte length and then this code will have to
		 * perform the encoding necessary (length written is length
		 * in bytes plus 1, and the first octet of string is the
		 * number of bits remaining between the end of the bit
		 * string and the next byte boundary).
		 */
		sec_asn1e_write_length_bytes (state, len + 1, PR_FALSE);
		byte = 0;
		sec_asn1e_write_contents_bytes (state, &byte, 1);
	    } else {
		sec_asn1e_write_length_bytes (state, len, PR_FALSE);
	    }
	}
	sec_asn1e_write_contents_bytes (state, buf, len);
	state->top->status = needBytes;
    } else {
	switch (state->underlying_kind) {
	  case SEC_ASN1_SET:
	  case SEC_ASN1_SEQUENCE:
	    PORT_Assert (0);
	    break;

	  case SEC_ASN1_BIT_STRING:
	    {
		SECItem *item;
		char rem;

		item = (SECItem *)state->src;
		len = (item->len + 7) >> 3;
		rem = (len << 3) - item->len;	/* remaining bits */
		sec_asn1e_write_contents_bytes (state, &rem, 1);
		sec_asn1e_write_contents_bytes (state, (char *) item->data,
						len);
	    }
	    break;

	  case SEC_ASN1_BMP_STRING:
	    /* The number of bytes must be divisable by 2 */
	    if ((((SECItem *)state->src)->len) % 2) {
		SEC_ASN1EncoderContext *cx;

		cx = state->top;
		cx->status = encodeError;
		break;
	    }
	    /* otherwise, fall through to write the content */
			
	  default:
	    {
		SECItem *item;

		item = (SECItem *)state->src;
		sec_asn1e_write_contents_bytes (state, (char *) item->data,
						item->len);
	    }
	    break;
	}
	state->place = afterContents;
    }
}


/*
 * We are doing a SET OF or SEQUENCE OF, and have just finished an item.
 */
static void
sec_asn1e_next_in_group (sec_asn1e_state *state)
{
    sec_asn1e_state *child;
    void **group;
    void *member;

    PORT_Assert (state->place == duringGroup);
    PORT_Assert (state->child != NULL);

    child = state->child;

    group = *(void ***)state->src;

    /*
     * Find placement of current item.
     */
    member = (char *)(state->child->src) - child->template->offset;
    while (*group != member)
	group++;

    /*
     * Move forward to next item.
     */
    group++;
    if (*group == NULL) {
	/*
	 * That was our last one; we are done now.
	 */
	child->place = notInUse;
	state->place = afterContents;
	return;
    }
    child->src = (char *)(*group) + child->template->offset;

    /*
     * Re-"push" child.
     */
    sec_asn1e_scrub_state (child);
    state->top->current = child;
}


/*
 * We are moving along through a sequence; move forward by one,
 * (detecting end-of-sequence when it happens).
 */
static void
sec_asn1e_next_in_sequence (sec_asn1e_state *state)
{
    sec_asn1e_state *child;

    PORT_Assert (state->place == duringSequence);
    PORT_Assert (state->child != NULL);

    child = state->child;

    /*
     * Do the "after" field notification.
     */
    sec_asn1e_notify_after (state->top, child->src, child->depth);

    /*
     * Move forward.
     */
    child->template++;
    if (child->template->kind == 0) {
	/*
	 * We are done with this sequence.
	 */
	child->place = notInUse;
	state->place = afterContents;
	return;
    }

    /*
     * Reset state and push.
     */

    child->src = (char *)state->src + child->template->offset;

    /*
     * Do the "before" field notification.
     */
    sec_asn1e_notify_before (state->top, child->src, child->depth);

    state->top->current = child;
    (void) sec_asn1e_init_state_based_on_template (child);
}


static void
sec_asn1e_after_contents (sec_asn1e_state *state)
{
    PORT_Assert (state->place == afterContents);

    if (state->indefinite)
	sec_asn1e_write_end_of_contents_bytes (state);

    /*
     * Just make my parent be the current state.  It will then clean
     * up after me and free me (or reuse me).
     */
    state->top->current = state->parent;
}


/*
 * This function is called whether or not we are streaming; if we
 * *are* streaming, our caller can also instruct us to take bytes
 * from the passed-in buffer (at buf, for length len, which is likely
 * bytes but could even mean bits if the current field is a bit string).
 * If we have been so instructed, we will gobble up bytes from there
 * (rather than from our src structure) and output them, and then
 * we will just return, expecting to be called again -- either with
 * more bytes or after our caller has instructed us that we are done
 * (for now) with the buffer.
 */
SECStatus
SEC_ASN1EncoderUpdate (SEC_ASN1EncoderContext *cx,
		       const char *buf, unsigned long len)
{
    sec_asn1e_state *state;

    if (cx->status == needBytes) {
	PORT_Assert (buf != NULL && len != 0);
	cx->status = keepGoing;
    }

    while (cx->status == keepGoing) {
	state = cx->current;
	switch (state->place) {
	  case beforeHeader:
	    sec_asn1e_write_header (state);
	    break;
	  case duringContents:
	    sec_asn1e_write_contents (state, buf, len);
	    break;
	  case duringGroup:
	    sec_asn1e_next_in_group (state);
	    break;
	  case duringSequence:
	    sec_asn1e_next_in_sequence (state);
	    break;
	  case afterContents:
	    sec_asn1e_after_contents (state);
	    break;
	  case afterImplicit:
	  case afterInline:
	  case afterPointer:
	    /*
	     * These states are more documentation than anything.
	     * They just need to force a pop.
	     */
	    PORT_Assert (!state->indefinite);
	    state->place = afterContents;
	    break;
	  case notInUse:
	  default:
	    /* This is not an error, but rather a plain old BUG! */
	    PORT_Assert (0);
	    cx->status = encodeError;
	    break;
	}

	if (cx->status == encodeError)
	    break;

	/* It might have changed, so we have to update our local copy.  */
	state = cx->current;

	/* If it is NULL, we have popped all the way to the top.  */
	if (state == NULL) {
	    cx->status = allDone;
	    break;
	}
    }

    if (cx->status == encodeError) {
	return SECFailure;
    }

    return SECSuccess;
}


void
SEC_ASN1EncoderFinish (SEC_ASN1EncoderContext *cx)
{
    /*
     * XXX anything else that needs to be finished?
     */

    PORT_FreeArena (cx->our_pool, PR_FALSE);
}


SEC_ASN1EncoderContext *
SEC_ASN1EncoderStart (void *src, const SEC_ASN1Template *template,
		      SEC_ASN1WriteProc output_proc, void *output_arg)
{
    PRArenaPool *our_pool;
    SEC_ASN1EncoderContext *cx;

    our_pool = PORT_NewArena (SEC_ASN1_DEFAULT_ARENA_SIZE);
    if (our_pool == NULL)
	return NULL;

    cx = PORT_ArenaZAlloc (our_pool, sizeof(*cx));
    if (cx == NULL) {
	PORT_FreeArena (our_pool, PR_FALSE);
	return NULL;
    }

    cx->our_pool = our_pool;
    cx->output_proc = output_proc;
    cx->output_arg = output_arg;

    cx->status = keepGoing;

    if (sec_asn1e_push_state(cx, template, src, PR_FALSE) == NULL
	|| sec_asn1e_init_state_based_on_template (cx->current) == NULL) {
	/*
	 * Trouble initializing (probably due to failed allocations)
	 * requires that we just give up.
	 */
	PORT_FreeArena (our_pool, PR_FALSE);
	return NULL;
    }

    return cx;
}


/*
 * XXX Do we need a FilterProc, too?
 */


void
SEC_ASN1EncoderSetNotifyProc (SEC_ASN1EncoderContext *cx,
			      SEC_ASN1NotifyProc fn, void *arg)
{
    cx->notify_proc = fn;
    cx->notify_arg = arg;
}


void
SEC_ASN1EncoderClearNotifyProc (SEC_ASN1EncoderContext *cx)
{
    cx->notify_proc = NULL;
    cx->notify_arg = NULL;	/* not necessary; just being clean */
}


void
SEC_ASN1EncoderSetStreaming (SEC_ASN1EncoderContext *cx)
{
    /* XXX is there a way to check that we are "between" fields here? */

    cx->streaming = PR_TRUE;
}


void
SEC_ASN1EncoderClearStreaming (SEC_ASN1EncoderContext *cx)
{
    /* XXX is there a way to check that we are "between" fields here? */

    cx->streaming = PR_FALSE;
}


void
SEC_ASN1EncoderSetTakeFromBuf (SEC_ASN1EncoderContext *cx)
{
    /* check that we are "between" fields here, and streaming */
    PORT_Assert (cx->during_notify && cx->streaming);

    cx->from_buf = PR_TRUE;
}


void
SEC_ASN1EncoderClearTakeFromBuf (SEC_ASN1EncoderContext *cx)
{
    /* we should actually be taking from buf *now* */
    PORT_Assert (cx->from_buf);
    if (! cx->from_buf)		/* if not, just do nothing */
	return;

    cx->from_buf = PR_FALSE;

    if (cx->status == needBytes) {
	cx->status = keepGoing;
	cx->current->place = afterContents;
    }
}


SECStatus
SEC_ASN1Encode (void *src, const SEC_ASN1Template *template,
		SEC_ASN1WriteProc output_proc, void *output_arg)
{
    SEC_ASN1EncoderContext *ecx;
    SECStatus rv;

    ecx = SEC_ASN1EncoderStart (src, template, output_proc, output_arg);
    if (ecx == NULL)
	return SECFailure;

    rv = SEC_ASN1EncoderUpdate (ecx, NULL, 0);

    SEC_ASN1EncoderFinish (ecx);
    return rv;
}


/*
 * XXX depth and data_kind are unused; is there a PC way to silence warnings?
 * (I mean "politically correct", not anything to do with intel/win platform) 
 */
static void
sec_asn1e_encode_item_count (void *arg, const char *buf, unsigned long len,
			     int depth, SEC_ASN1EncodingPart data_kind)
{
    unsigned long *count;

    count = arg;
    PORT_Assert (count != NULL);

    *count += len;
}


/* XXX depth and data_kind are unused; is there a PC way to silence warnings? */
static void
sec_asn1e_encode_item_store (void *arg, const char *buf, unsigned long len,
			     int depth, SEC_ASN1EncodingPart data_kind)
{
    SECItem *dest;

    dest = arg;
    PORT_Assert (dest != NULL);

    PORT_Memcpy (dest->data + dest->len, buf, len);
    dest->len += len;
}


/*
 * Allocate an entire SECItem, or just the data part of it, to hold
 * "len" bytes of stuff.  Allocate from the given pool, if specified,
 * otherwise just do a vanilla PORT_Alloc.
 *
 * XXX This seems like a reasonable general-purpose function (for SECITEM_)?
 */
static SECItem *
sec_asn1e_allocate_item (PRArenaPool *poolp, SECItem *dest, unsigned long len)
{
    if (poolp != NULL) {
	void *release;

	release = PORT_ArenaMark (poolp);
	if (dest == NULL)
	    dest = PORT_ArenaAlloc (poolp, sizeof(SECItem));
	if (dest != NULL) {
	    dest->data = PORT_ArenaAlloc (poolp, len);
	    if (dest->data == NULL) {
		PORT_ArenaRelease (poolp, release);
		dest = NULL;
	    }
	}
    } else {
	SECItem *indest;

	indest = dest;
	if (dest == NULL)
	    dest = PORT_Alloc (sizeof(SECItem));
	if (dest != NULL) {
	    dest->data = PORT_Alloc (len);
	    if (dest->data == NULL) {
		if (indest == NULL)
		    PORT_Free (dest);
		dest = NULL;
	    }
	}
    }

    return dest;
}


SECItem *
SEC_ASN1EncodeItem (PRArenaPool *poolp, SECItem *dest, void *src,
		    const SEC_ASN1Template *template)
{
    unsigned long encoding_length;
    SECStatus rv;

    PORT_Assert (dest == NULL || dest->data == NULL);

    encoding_length = 0;
    rv = SEC_ASN1Encode (src, template,
			 sec_asn1e_encode_item_count, &encoding_length);
    if (rv != SECSuccess)
	return NULL;

    dest = sec_asn1e_allocate_item (poolp, dest, encoding_length);
    if (dest == NULL)
	return NULL;

    /* XXX necessary?  This really just checks for a bug in the allocate fn */
    PORT_Assert (dest->data != NULL);
    if (dest->data == NULL)
	return NULL;

    dest->len = 0;
    (void) SEC_ASN1Encode (src, template, sec_asn1e_encode_item_store, dest);

    PORT_Assert (encoding_length == dest->len);
    return dest;
}


static SECItem *
sec_asn1e_integer(PRArenaPool *poolp, SECItem *dest, unsigned long value,
		  PRBool make_unsigned)
{
    unsigned long copy;
    unsigned char sign;
    int len = 0;

    /*
     * Determine the length of the encoded value (minimum of 1).
     */
    copy = value;
    do {
	len++;
	sign = copy & 0x80;
	copy >>= 8;
    } while (copy);

    /*
     * If this is an unsigned encoding, and the high bit of the last
     * byte we counted was set, we need to add one to the length so
     * we put a high-order zero byte in the encoding.
     */
    if (sign && make_unsigned)
	len++;

    /*
     * Allocate the item (if necessary) and the data pointer within.
     */
    dest = sec_asn1e_allocate_item (poolp, dest, len);
    if (dest == NULL)
	return NULL;

    /*
     * Store the value, byte by byte, in the item.
     */
    dest->len = len;
    while (len) {
	dest->data[--len] = value;
	value >>= 8;
    }
    PORT_Assert (value == 0);

    return dest;
}


SECItem *
SEC_ASN1EncodeInteger(PRArenaPool *poolp, SECItem *dest, long value)
{
    return sec_asn1e_integer (poolp, dest, (unsigned long) value, PR_FALSE);
}

SECItem *
SEC_ASN1EncodeEnumerated(PRArenaPool *poolp, SECItem *dest, long value)
{
    return sec_asn1e_integer (poolp, dest, (unsigned long) value, PR_FALSE);
}


extern SECItem *
SEC_ASN1EncodeUnsignedInteger(PRArenaPool *poolp,
			      SECItem *dest, unsigned long value)
{
    return sec_asn1e_integer (poolp, dest, value, PR_TRUE);
}
