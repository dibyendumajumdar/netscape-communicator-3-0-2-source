//	===========================================================================
//	LDynamicBuffer.cp				�1994 Metrowerks Inc. All rights reserved.
//	===========================================================================

#ifdef PowerPlant_PCH
#include PowerPlant_PCH
#endif

#include	"LDynamicBuffer.h"
#include	<URange32.h>

#ifndef		__ERRORS__
#include	<Errors.h>
#endif

#ifndef		__MEMORY__
#include	<Memory.h>
#endif

LDynamicBuffer::LDynamicBuffer()
{
	mData = NULL;
}

LDynamicBuffer::LDynamicBuffer(const LDynamicBuffer &inBuffer)
{
	OSErr	err;
	
	mData = inBuffer.mData;
	err = HandToHand(&mData);
	
	if (err) {
		mData = NULL;
		ThrowIfOSErr_(err);
	}
}

LDynamicBuffer::~LDynamicBuffer()
{
	if (mData)
		DisposeHandle(mData);
	mData = NULL;
}

void *	LDynamicBuffer::GetOffsetPtr(
	Int32	inOffset) const
/*
	Returns NULL if offset is non-existent.
*/
{
	if (mData == NULL)
		return NULL;
	
	if (inOffset >= GetSize())
		return NULL;
		
	if (inOffset < 0) {
		Throw_(paramErr);
		return NULL;
	}
	
	return *mData + inOffset;
}

void	LDynamicBuffer::Shift(
	Int32	inOffset,
	Int32	inDelta)
{
	void	*destination,
			*source;
	Size	amount;
	
	if (mData == NULL) {
		Throw_(paramErr);
		return;
	}
	
	if (inOffset < 0) {
		Throw_(paramErr);
		return;
	}
	
	if (inDelta == 0)
		return;

	source = GetOffsetPtr(inOffset);
	if (source == NULL) {
		Throw_(paramErr);
		return;
	}
	
	amount = GetSize() - inOffset;
	if (inDelta > 0)
		amount -= inDelta;
	if (amount <= 0)
		return;
	
	destination = GetOffsetPtr(inOffset + inDelta);
	if (destination == NULL) {
		Throw_(paramErr);
		return;
	}

	BlockMoveData(source, destination, amount);
}

void	LDynamicBuffer::Clear(void)
{
	if (mData)
		DisposeHandle(mData);
	mData = NULL;
}

Int32	LDynamicBuffer::GetSize(void) const
{
	if (mData)
		return GetHandleSize(mData);
	else
		return 0;
}

void	LDynamicBuffer::Delete(
	Int32	inOffset,
	Int32	inAmount)
/*
	If inOffset is past end of buffer, inAmount is deleted from the end
	of the buffer.
	
	If inOffset is before start of buffer (< 0), inAmount is deleted from
	start of buffer (data is moved so this is computationally unattractive).
	
	Delete will not attempt to delete more than the current size of the buffer.
*/
{
	Int32	size;
	
	if (mData == NULL)
		return;
	
	if (inOffset < 0)
		inOffset = 0;
	
	if (inAmount < 0) {
		Throw_(paramErr);
		return;
	}
	
	if (inAmount == 0)
		return;

	if (inOffset < GetSize()) {
		//	likely a hole deletion...
		if (GetOffsetPtr(inOffset) == NULL) {
			Throw_(paramErr);
			return;
		}
	
		inAmount = URange32::Min(inAmount, GetSize() - inOffset);

		Shift(inOffset + inAmount, -inAmount);
	} else {
		//	tale deletion
		inAmount = URange32::Min(GetSize(), inAmount);
	}
	size = GetSize() - inAmount;
	
	SetHandleSize(mData, size);
	ThrowIfMemError_();
}

void *	LDynamicBuffer::Insert(
	Int32		inOffset,
	const void	*inData,
	Int32		inDataSize)
{
	void	*destination;
	
	if (inDataSize < 0) {
		Throw_(paramErr);
		return NULL;
	}
	
	if (inDataSize == 0)
		return NULL;
		
	if (mData == NULL) {
		mData = NewHandle(inDataSize + inOffset);
		ThrowIfNULL_(mData);
		
		destination = GetOffsetPtr(inOffset);
	} else {
		Size	reqdSize = inOffset + inDataSize;
		
		reqdSize = URange32::Max(reqdSize, GetHandleSize(mData) + inDataSize);

		SetHandleSize(mData, reqdSize);
		ThrowIfMemError_();

		Shift(inOffset, inDataSize);
		
		destination = GetOffsetPtr(inOffset);
		
		if (destination == NULL) {
			Throw_(paramErr);
			return NULL;
		}
	}
	
	if (inData == NULL)
		return destination;
		
	BlockMoveData(inData, destination, inDataSize);
	return NULL;
}

/*
void *	LDynamicBuffer::Append(
	const void	*inData,
	Int32		inDataSize)
{
	return Insert(GetSize(), inData, inDataSize);
}

void *	LDynamicBuffer::Prepend(
	const void	*inData,
	Int32		inDataSize)
{
	return Insert(0, inData, inDataSize);
}
*/

void	LDynamicBuffer::GetData(
	Int32	inOffset,
	Int32	inAmount,
	void	*outData) const
{
	void	*source;
	Size	amount;
	
	source = GetOffsetPtr(inOffset);
	if (source == NULL) {
		Throw_(paramErr);
		return;
	}
	
	amount = URange32::Min(inAmount, GetSize() - inOffset);
	if (amount < 0) {
		Throw_(paramErr);
		return;
	}
	
	if (amount == 0)
		return;
		
	if (outData) {
		BlockMoveData(source, outData, amount);
	} else {
		Throw_(paramErr);
	}
}

