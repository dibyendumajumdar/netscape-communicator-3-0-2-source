#ifndef _SECDER_H_
#define _SECDER_H_
/*
 * secder.h - public data structures and prototypes for the DER encoding and
 *	      decoding utilities library
 *
 * $Id: secder.h,v 1.3 1997/02/21 01:20:39 hoa Exp $
 */

#include <time.h>
#include "prarena.h"
#include "prlong.h"

#include "seccomon.h"
#include "secdert.h"

SEC_BEGIN_PROTOS

/*
** Decode a piece of der encoded data.
** 	"dest" points to a structure that will be filled in with the
**	   decoding results.  (NOTE: it should be zeroed before calling;
**	   optional/missing fields are not zero-filled by DER_Decode.)
**	"t" is a template structure which defines the shape of the
**	   expected data.
**	"src" is the der encoded data.
** NOTE: substructures of "dest" will be allocated as needed from
** "arena", but data subfields will point directly into the buffer
** passed in as src->data.  That is, the resulting "dest" structure
** will contain pointers back into src->data, which must remain
** active (allocated) and unmodified for as long as "dest" is active.
** If this is a potential problem, you may want to just dup the buffer
** (allocated from "arena", probably) and pass *that* in instead.
*/
extern SECStatus DER_Decode(PRArenaPool *arena, void *dest, DERTemplate *t,
			   SECItem *src);

/*
** Encode a data structure into DER.
**	"dest" will be filled in (and memory allocated) to hold the der
**	   encoded structure in "src"
**	"t" is a template structure which defines the shape of the
**	   stored data
**	"src" is a pointer to the structure that will be encoded
*/
extern SECStatus DER_Encode(PRArenaPool *arena, SECItem *dest, DERTemplate *t,
			   void *src);

extern SECStatus DER_Lengths(SECItem *item, int *header_len_p, uint32 *contents_len_p);

/*
** Lower level der subroutine that stores the standard header into "to".
** The header is of variable length, based on encodingLen.
** The return value is the new value of "to" after skipping over the header.
**	"to" is where the header will be stored
**	"code" is the der code to write
**	"encodingLen" is the number of bytes of data that will follow
**	   the header
*/
extern unsigned char *DER_StoreHeader(unsigned char *to, unsigned int code,
				      uint32 encodingLen);

/*
** Return the number of bytes it will take to hold a der encoded length.
*/
extern int DER_LengthLength(uint32 len);

/*
** Store a der encoded *signed* integer (whose value is "src") into "dst".
** XXX This should really be enhanced to take a long.
*/
extern SECStatus DER_SetInteger(PRArenaPool *arena, SECItem *dst, int32 src);

/*
** Store a der encoded *unsigned* integer (whose value is "src") into "dst".
** XXX This should really be enhanced to take an unsigned long.
*/
extern SECStatus DER_SetUInteger(PRArenaPool *arena, SECItem *dst, uint32 src);

/*
** Decode a der encoded *signed* integer that is stored in "src".
** If "-1" is returned, then the caller should check the error in
** XP_GetError() to see if an overflow occurred (SEC_ERROR_BAD_DER).
*/
extern long DER_GetInteger(SECItem *src);

/*
** Decode a der encoded *unsigned* integer that is stored in "src".
** If the ULONG_MAX is returned, then the caller should check the error
** in XP_GetError() to see if an overflow occurred (SEC_ERROR_BAD_DER).
*/
extern unsigned long DER_GetUInteger(SECItem *src);

/*
** Convert a "UNIX" time value to a der encoded time value.
**	"result" is the der encoded time (memory is allocated)
**	"time" is the "UNIX" time value (Since Jan 1st, 1970).
** The caller is responsible for freeing up the buffer which
** result->data points to upon a successfull operation.
*/
extern SECStatus DER_TimeToUTCTime(SECItem *result, int64 time);

/*
** Convert an ascii encoded time value (according to DER rules) into
** a UNIX time value.
**	"result" the resulting "UNIX" time
**	"string" the der notation ascii value to decode
*/
extern SECStatus DER_AsciiToTime(int64 *result, char *string);

/*
** Same as DER_AsciiToTime except takes an SECItem instead of a string
*/
extern SECStatus DER_UTCTimeToTime(int64 *result, SECItem *time);

/*
** Convert a DER encoded UTC time to an ascii time representation
** "utctime" is the DER encoded UTC time to be converted. The
** caller is responsible for deallocating the returned buffer.
*/
extern char *DER_UTCTimeToAscii(SECItem *utcTime);

/*
** Convert a DER encoded UTC time to an ascii time representation, but only
** include the day, not the time.
**	"utctime" is the DER encoded UTC time to be converted.
** The caller is responsible for deallocating the returned buffer.
*/
extern char *DER_UTCDayToAscii(SECItem *utctime);

/*
** Convert a int64 time to a DER encoded Generalized time
*/
extern SECStatus DER_TimeToGeneralizedTime(SECItem *dst, int64 gmttime);

/*
** Convert a DER encoded Generalized time value into a UNIX time value.
**	"dst" the resulting "UNIX" time
**	"string" the der notation ascii value to decode
*/
extern SECStatus DER_GeneralizedTimeToTime(int64 *dst, SECItem *time);

/*
** Convert from a int64 UTC time value to a formatted ascii value. The
** caller is responsible for deallocating the returned buffer.
*/
extern char *CERT_UTCTime2FormattedAscii (int64 utcTime, char *format);


/*
** Convert from a int64 Generalized time value to a formatted ascii value. The
** caller is responsible for deallocating the returned buffer.
*/
extern char *CERT_GenTime2FormattedAscii (int64 genTime, char *format);

SEC_END_PROTOS

#endif /* _SECDER_H_ */

