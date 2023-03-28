// ���������������������������������������������������������������������������
//	CBevelButton.cp						  �1996 Netscape. All rights reserved.	
// ���������������������������������������������������������������������������

#ifdef PowerPlant_PCH
#include PowerPlant_PCH
#endif

#include <UGWorld.h>
#include <UDrawingState.h>
#include <UDrawingUtils.h>
#include <UMemoryMgr.h>
#include <UTextTraits.h>
#include <LStream.h>
#include "UGraphicGizmos.h"
#include "CBevelButton.h"

#ifndef __ICONS__
#include <Icons.h>
#endif

#ifndef __PALETTES__
#include <Palettes.h>
#endif

#ifndef __TOOLUTILS__
#include <ToolUtils.h>
#endif

// ���������������������������������������������������������������������������
//	���
//	�	Class CBevelButton
//	���	
//
//	The bevel button class is the base in which other types of beveled graphic
//	buttons are based.  The base class maintains the offscreen GWorld so that
//	the parts of the button can be draw without flicker.  It also maintains
//	a mask region so that the buttons background view will show through if
// 	the corners are rounded.
// ���������������������������������������������������������������������������

// ���������������������������������������������������������������������������
//	�	CreateBevelButtonStream
// ���������������������������������������������������������������������������

CBevelButton* CBevelButton::CreateBevelButtonStream(LStream *inStream)
{
	return (new CBevelButton(inStream));
}

// ���������������������������������������������������������������������������
//	�	CBevelButton
// ���������������������������������������������������������������������������

CBevelButton::CBevelButton(LStream *inStream)
	:	LControl(inStream)
{
	mButtonMask = NULL;
	mGraphicHandle = NULL;
	
	*inStream >> mBevel;
	*inStream >> mOvalWidth;
	*inStream >> mOvalHeight;
	
	ResIDT theBevelTraitsID;
	*inStream >> theBevelTraitsID;
	UGraphicGizmos::LoadBevelTraits(theBevelTraitsID, mUpColors);

	*inStream >> theBevelTraitsID;
	UGraphicGizmos::LoadBevelTraits(theBevelTraitsID, mDownColors);
	
	inStream->ReadPString(mTitle);
	*inStream >> mTitleTraitsID;	
	*inStream >> mTitlePosition.h;
	*inStream >> mTitlePosition.v;
		
	*inStream >> mGraphicType;
	*inStream >> mGraphicID;
	*inStream >> mGraphicPosition.h;
	*inStream >> mGraphicPosition.v;
	
	InitBevelButton();
}

// ���������������������������������������������������������������������������
//	�	~CBevelButton
// ���������������������������������������������������������������������������

CBevelButton::~CBevelButton()
{
	if (mButtonMask != NULL)
		::DisposeRgn(mButtonMask);
		
	if (mGraphicHandle != NULL)
		::DisposeCIcon(mGraphicHandle);
}

// ���������������������������������������������������������������������������
//	�	InitBevelButton
// ���������������������������������������������������������������������������

void CBevelButton::InitBevelButton(void)
{
	mPushed = false;

	Rect theButtonRect;
	CalcLocalFrameRect(theButtonRect);
	
	mButtonMask = ::NewRgn();
	ThrowIfNULL_(mButtonMask);
	
	::OpenRgn();
	::FrameRoundRect(&theButtonRect, mOvalWidth, mOvalHeight);
	::CloseRgn(mButtonMask);
	
	if ((mGraphicID != 0) && (mGraphicType == 'cicn'))
		{
		mGraphicHandle = ::GetCIcon(mGraphicID);
		ThrowIfNULL_(mGraphicHandle);
		}
}

// ���������������������������������������������������������������������������
//	�	Draw
// ���������������������������������������������������������������������������

void CBevelButton::Draw(RgnHandle inSuperDrawRgnH)
{
	Rect theFrame;
	if ((mVisible == triState_On) && CalcPortFrameRect(theFrame) &&
			((inSuperDrawRgnH == nil) || RectInRgn(&theFrame, inSuperDrawRgnH)) && FocusDraw())
		{
		PortToLocalPoint(topLeft(theFrame));	// Get Frame in Local coords
		PortToLocalPoint(botRight(theFrame));

		if (ExecuteAttachments(msg_DrawOrPrint, &theFrame))
			{
			Boolean bDidDraw = false;

			StColorPenState thePenSaver;
			StColorPenState::Normalize();
			
			// Fail safe offscreen drawing
			StValueChanger<EDebugAction> okayToFail(gDebugThrow, debugAction_Nothing);
			try
				{			
				LGWorld theOffWorld(theFrame, 0, useTempMem);

				if (!theOffWorld.BeginDrawing())
					throw (ExceptionCode)memFullErr;
					
				DrawSelf();
					
				theOffWorld.EndDrawing();
				theOffWorld.CopyImage(GetMacPort(), theFrame, srcCopy, mButtonMask);
				bDidDraw = true;
				}
			catch (ExceptionCode inErr)
				{
				// 	& draw onscreen
				}
				
			if (!bDidDraw)
				{
				::SetClip(mButtonMask);
				DrawSelf();
				}
			}
		}
}

// ���������������������������������������������������������������������������
//	�	
// ���������������������������������������������������������������������������

void CBevelButton::DrawSelf(void)
{
	PrepareDrawButton();

	DrawButtonContent();

	if (mTitle.Length() > 0)
		DrawButtonTitle();

	if (mGraphicID != 0)
		DrawButtonGraphic();
			
	if (!IsEnabled() || !IsActive())
		DrawSelfDisabled();
			
	FinalizeDrawButton();
}

// ���������������������������������������������������������������������������
//	�	
// ���������������������������������������������������������������������������

void CBevelButton::DrawSelfDisabled(void)
{
	UGraphicGizmos::PaintDisabledRect(mCachedButtonFrame);
}

// ���������������������������������������������������������������������������
//	�	DrawButtonContent
//
//	Here we draw the button's frame and beveled area.  Subclasses should not
//	need to override this method, nor call it directly.
// ���������������������������������������������������������������������������

void CBevelButton::DrawButtonContent(void)
{
	const SBevelColorDesc& theColors = (IsPushed() || mValue == Button_On) ? mDownColors : mUpColors;

	Rect tempButtonRect = mCachedButtonFrame;
	::PmForeColor(theColors.frameColor);
	::FrameRoundRect(&tempButtonRect, mOvalWidth, mOvalHeight);
	::InsetRect(&tempButtonRect, 1, 1);
	::PmForeColor(theColors.fillColor);
	::PaintRoundRect(&tempButtonRect, mOvalWidth, mOvalHeight);
	
	UGraphicGizmos::BevelRect(tempButtonRect, mBevel, theColors.topBevelColor, theColors.bottomBevelColor);
}

// ���������������������������������������������������������������������������
//	�	
// ���������������������������������������������������������������������������

void CBevelButton::DrawButtonTitle(void)
{
	StColorPenState::Normalize();
	
	Rect theTitleFrame = mCachedButtonFrame;
	if (mPushed || mValue == Button_On)
		::OffsetRect(&theTitleFrame, 1, 1);
	
	UTextTraits::SetPortTextTraits(mTitleTraitsID);	
	UGraphicGizmos::PlaceStringInRect(mTitle, theTitleFrame, teCenter, teCenter);
}

// ���������������������������������������������������������������������������
//	�	
// ���������������������������������������������������������������������������

void CBevelButton::DrawButtonGraphic(void)
{
	Rect theGraphicFrame = mCachedButtonFrame;
	if (mPushed || mValue == Button_On)
		::OffsetRect(&theGraphicFrame, 1, 1);
		
	if (mGraphicType == 'cicn')
		{
		Assert_(mGraphicHandle != NULL);
		Rect theCicnFrame = (**mGraphicHandle).iconPMap.bounds;
		UGraphicGizmos::CenterRectOnRect(theCicnFrame, theGraphicFrame);
		::PlotCIcon(&theCicnFrame, mGraphicHandle);
		}
	else if (mGraphicType == 'ICN#')
		{
		::PlotIconID(&theGraphicFrame, atNone, ttNone, mGraphicID);
		}
	else
		{
		::FillRect(&mCachedButtonFrame, &UQDGlobals::GetQDGlobals()->ltGray);
		::FrameRect(&mCachedButtonFrame);
		}
}

// ���������������������������������������������������������������������������
//	�	PrepareDrawButton
// ���������������������������������������������������������������������������

void CBevelButton::PrepareDrawButton(void)
{
	CalcLocalFrameRect(mCachedButtonFrame);

	// Calculate the drawing mask region.
	::OpenRgn();
	::FrameRoundRect(&mCachedButtonFrame, mOvalWidth, mOvalHeight);
	::CloseRgn(mButtonMask);

//	::EraseRect(&mCachedButtonFrame);
}

// ���������������������������������������������������������������������������
//	�	FinalizeDrawButton
// ���������������������������������������������������������������������������

void CBevelButton::FinalizeDrawButton(void)
{
}

// ���������������������������������������������������������������������������
//	�
// ���������������������������������������������������������������������������

void CBevelButton::ActivateSelf(void)
{
	if (FocusExposed())
		{
		FocusDraw();
		Draw(NULL);
		
		Rect theFrame;
		CalcLocalFrameRect(theFrame);
		::ValidRgn(mButtonMask);
		}
}

// ���������������������������������������������������������������������������
//	�
// ���������������������������������������������������������������������������

void CBevelButton::DeactivateSelf(void)
{
	if (FocusExposed())
		{
		FocusDraw();
		Draw(NULL);
		
		Rect theFrame;
		CalcLocalFrameRect(theFrame);
		::ValidRgn(mButtonMask);
		}
}

// ���������������������������������������������������������������������������
//	�
// ���������������������������������������������������������������������������

void CBevelButton::EnableSelf(void)
{
	if (FocusExposed())
		{
		FocusDraw();
		Draw(NULL);
		}
}

// ���������������������������������������������������������������������������
//	�
// ���������������������������������������������������������������������������

void CBevelButton::DisableSelf(void)
{
	if (FocusExposed())
		{
		FocusDraw();
		Draw(NULL);
		}
}

// ���������������������������������������������������������������������������
//	�	HotSpotAction
// ���������������������������������������������������������������������������

void CBevelButton::HotSpotAction(short /* inHotSpot */, Boolean inCurrInside, Boolean inPrevInside)
{
	if (inCurrInside != inPrevInside)
		{
		mPushed = inCurrInside;
		Draw(NULL);
		}
}

// ���������������������������������������������������������������������������
//	�	HotSpotResult
// ���������������������������������������������������������������������������

void CBevelButton::HotSpotResult(Int16 inHotSpot)
{
	// Undo Button hilighting
	HotSpotAction(inHotSpot, false, true);

	// Although value doesn't change, send message to inform Listeners
	// that button was clicked
	BroadcastValueMessage();		
}

// ���������������������������������������������������������������������������
//	�	GetDescriptor
// ���������������������������������������������������������������������������

StringPtr CBevelButton::GetDescriptor(Str255 outDescriptor) const
{
	::BlockMoveData(&mTitle[0], &outDescriptor[0], mTitle.Length() + 1);
	return (StringPtr)&mTitle[0];
}

// ���������������������������������������������������������������������������
//	�	SetDescriptor
// ���������������������������������������������������������������������������

void CBevelButton::SetDescriptor(ConstStr255Param inDescriptor)
{
	mTitle = inDescriptor;
}

// ���������������������������������������������������������������������������
//	�	SetDescriptor
// ���������������������������������������������������������������������������

void CBevelButton::SetGraphicID(ResIDT inResID)
{
	if (inResID != mGraphicID)
		{
		if (mGraphicHandle != NULL)
			::DisposeCIcon(mGraphicHandle);
		
		mGraphicHandle = NULL;
		
		if (mGraphicType == 'cicn')
			{		
			mGraphicHandle = ::GetCIcon(inResID);
			ThrowIfNULL_(mGraphicHandle);
			}
		}

	mGraphicID = inResID;
}


// ���������������������������������������������������������������������������
//	���
//	�	Class CDeluxeBevelButton
//	���	
// ���������������������������������������������������������������������������

CDeluxeBevelButton* CDeluxeBevelButton::CreateDeluxeBevelButtonStream(LStream *inStream)
{
	return (new CDeluxeBevelButton(inStream));
}

// ���������������������������������������������������������������������������
//	�	
// ���������������������������������������������������������������������������

CDeluxeBevelButton::CDeluxeBevelButton(LStream *inStream)
	:	CBevelButton(inStream)
{
}

// ���������������������������������������������������������������������������
//	�	
// ���������������������������������������������������������������������������

void CDeluxeBevelButton::DrawButtonContent(void)
{
	const SBevelColorDesc& theColors = (IsPushed() || mValue == Button_On) ? mDownColors : mUpColors;

	Rect tempButtonRect = mCachedButtonFrame;
	::PmForeColor(theColors.frameColor);
	::FrameRoundRect(&tempButtonRect, mOvalWidth, mOvalHeight);
	::InsetRect(&tempButtonRect, 1, 1);
	::PmForeColor(theColors.fillColor);
	::PaintRoundRect(&tempButtonRect, mOvalWidth, mOvalHeight);
	
	Int16 theShiftedTopColor = (theColors.topBevelColor > eStdGray93) ? theColors.topBevelColor - 1 : eStdGrayWhite;

	UGraphicGizmos::BevelRect(tempButtonRect, mBevel, theColors.topBevelColor, theColors.bottomBevelColor);
	::InsetRect(&tempButtonRect, 1, 1);
	UGraphicGizmos::BevelRect(tempButtonRect, mBevel, theShiftedTopColor, theColors.bottomBevelColor - 1);
}


