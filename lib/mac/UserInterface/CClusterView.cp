// ���������������������������������������������������������������������������
//	CClusterView.cp						  �1996 Netscape. All rights reserved.
// ���������������������������������������������������������������������������

#ifdef PowerPlant_PCH
#include PowerPlant_PCH
#endif

#include <LStream.h>
#include <UTextTraits.h>
#include <UDrawingState.h>
#include <UDrawingUtils.h>

#include "CClusterView.h"
#include "UStdBevels.h"

// some private constants
const Int16 kClusterIndentSize = 13;
const Int16 kClusterTitleFudgeSize = 4;

// ���������������������������������������������������������������������������
//	�	
// ���������������������������������������������������������������������������

CClusterView* CClusterView::CreateClusterViewStream(LStream* inStream)
{
	return (new CClusterView(inStream));
}

// ���������������������������������������������������������������������������
//	�	
// ���������������������������������������������������������������������������

CClusterView::CClusterView(LStream *inStream)
	:	LView(inStream)
{
	inStream->ReadPString(mTitle);
	inStream->ReadData(&mTraitsID, sizeof(ResIDT));
}

// ���������������������������������������������������������������������������
//	�	
// ���������������������������������������������������������������������������

CClusterView::~CClusterView()
{
}

// ���������������������������������������������������������������������������
//	�	
// ���������������������������������������������������������������������������

void CClusterView::FinishCreateSelf(void)
{
	LView::FinishCreateSelf();

	CalcTitleFrame();	
}

// ���������������������������������������������������������������������������
//	�	
// ���������������������������������������������������������������������������

StringPtr CClusterView::GetDescriptor(Str255 outDescriptor) const
{
	return LString::CopyPStr(mTitle, outDescriptor);
}

// ���������������������������������������������������������������������������
//	�	
// ���������������������������������������������������������������������������

void CClusterView::SetDescriptor(ConstStr255Param inDescriptor)
{
	mTitle = inDescriptor;
	CalcTitleFrame();
}

// ���������������������������������������������������������������������������
//	�	
// ���������������������������������������������������������������������������

void CClusterView::DrawSelf(void)
{
	StColorPenState theStateSaver;
	theStateSaver.Normalize();
	
	Rect theFrame;
	CalcLocalFrameRect(theFrame);

	Rect theOutlineFrame = theFrame;
	theOutlineFrame.top += (mTitleFrame.bottom - mTitleFrame.top) / 2;

	UTextTraits::SetPortTextTraits(mTraitsID);

	Int16 theDepth;
	StDeviceLoop theLoop(theFrame);
	while (theLoop.NextDepth(theDepth))
		{
		if (theDepth > 4)
			{
			// Draw the frame in color
			::PmForeColor(eStdGray40);
			::PmBackColor(eStdGray53);
			}

		::PenPat(&qd.gray);			
		::FrameRect(&theOutlineFrame);
		
		if (mTitle.Length() > 0)
			{
			StColorState::Normalize();
			ApplyForeAndBackColors();
			::EraseRect(&mTitleFrame);
			::MoveTo(mTitleFrame.left + kClusterTitleFudgeSize, mTitleBaseline);
			::DrawText(mTitle, 1, mTitle.Length());
			}
		}
}

// ���������������������������������������������������������������������������
//	�	
// ���������������������������������������������������������������������������

void CClusterView::CalcTitleFrame(void)
{
	if (mTitle.Length() == 0)
		return;
		
	Rect theFrame;
	CalcLocalFrameRect(theFrame);
	mTitleFrame = theFrame;

	Int16 theJust = UTextTraits::SetPortTextTraits(mTraitsID);
	Int16 theTextWidth = ::StringWidth(mTitle) + (2 * kClusterTitleFudgeSize);
	
	FontInfo theInfo;
	::GetFontInfo(&theInfo);
	Int16 theTextHeight = theInfo.ascent + theInfo.descent + theInfo.leading;
	
	mTitleBaseline = theFrame.top + theInfo.ascent;
	mTitleFrame.bottom = mTitleFrame.top + theTextHeight;
	
	switch (theJust)
		{
		case teCenter:
			mTitleFrame.left += ((theFrame.right - theFrame.left - theTextWidth) / 2);
			mTitleFrame.right = mTitleFrame.left + theTextWidth;
			break;
				
		case teFlushRight:
			mTitleFrame.right = theFrame.right - kClusterIndentSize;
			mTitleFrame.left = mTitleFrame.right - theTextWidth;
			break;

		case teFlushDefault:
		case teFlushLeft:
		default:
			mTitleFrame.left += kClusterIndentSize;
			mTitleFrame.right = mTitleFrame.left + theTextWidth;
			break;
		}
}
