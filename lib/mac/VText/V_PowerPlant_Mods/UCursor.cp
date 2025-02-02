//	===========================================================================
//	UCursor.cp						�1994 Metrowerks Inc. All rights reserved.
//	===========================================================================

#ifdef PowerPlant_PCH
#include PowerPlant_PCH
#endif

#include	"UCursor.h"
#include	<UDebugging.h>
#include	<UEnvironment.h>
#include	<UDrawingState.h>

#ifndef		__TOOLUTILS__
#include	<ToolUtils.h>
#endif

//	===========================================================================
//	Misc:

/*
	Thanks to Patrick Doane for providing the color cursor support.
*/

/*
	In ToolUtils.h remember:
	
		    iBeamCursor = 1,
		    crossCursor = 2,
		    plusCursor = 3,
		    watchCursor = 4
*/

#define	arrowCursor		0
#define	defaultCursor	arrowCursor

Int16		UCursor::sCurrentCursorID = 0;
CursHandle	UCursor::sBWCursor = NULL;
CCrsrHandle	UCursor::sColorCursor = NULL;

//	===========================================================================
//	Routines:

void	UCursor::Reset(void)
{
	sCurrentCursorID = 0;
	UCursor::Set(cu_Default);
}

void	UCursor::Tick(Int16 inID)
/*
	inID indicates cursor family.  O or of same family implies no family
	change but just a "tick."
*/
{
	Int16	oldID = UCursor::Get(),
			newID;
	
	if (inID == 0)
		inID = oldID;

	if ((inID & cu_Mask) == (oldID & cu_Mask)) {
		//	Same family -- tick cursor
		newID = (oldID + 1) & ~cu_Mask;
		newID += inID & cu_Mask;
		UCursor::Set(newID);
	} else {
		//	Different family -- change cursor
		UCursor::Set(inID);
	}
}

void	UCursor::Set(Int16 inID)
{
	CursHandle	crsr = NULL;
	CCrsrHandle	colorCrsr = NULL;
	Int16		newID,
				oldID = UCursor::Get();

	if (inID == 0)
		inID = UCursor::Get();

	newID = inID;
	
	if (newID != oldID) {
		//	First check resources
		if (UEnvironment::HasFeature(env_SupportsColor))
			colorCrsr = GetCCursor(newID);
		if (colorCrsr == NULL) {
			crsr = GetCursor(newID);
		}
		
		if (colorCrsr == NULL && crsr == NULL) {
			//	Try first in family...
			newID = newID & cu_Mask;
			if (UEnvironment::HasFeature(env_SupportsColor))
				colorCrsr = GetCCursor(newID);
			if (colorCrsr == NULL) {
				crsr = GetCursor(newID);
			}
		}
			
		if (colorCrsr == NULL && crsr == NULL) {
			//	Try remapping to a known or system cursor...
			
			switch(inID & cu_Mask) {
				case cu_IBeam:
					newID = iBeamCursor;
					break;
				case cu_Cross:
					newID = crossCursor;
					break;
				case cu_Hand:
				case cu_Plus:
					newID = plusCursor;
					break;
				case cu_Watch:
					newID = watchCursor;
					break;
				case cu_EmptyArrow:
				case cu_Arrow:
				default:
					newID = arrowCursor;
					break;
			}
			
			if (newID == oldID)
				return;
			
			if (UEnvironment::HasFeature(env_SupportsColor))
				colorCrsr = GetCCursor(newID);
			if (colorCrsr == NULL) {
				crsr = GetCursor(newID);
			}
		}
		
		// Dispose old cursor if its a color cursor
		if (sColorCursor != NULL) {
			DisposeCCursor(sColorCursor);
			sColorCursor = NULL;
		} else {
			sBWCursor = NULL;
		}
		
		if (colorCrsr != NULL) {
			Assert_(*colorCrsr != NULL);
			SetCCursor(colorCrsr);
			
			sColorCursor = colorCrsr;
		} else if (crsr != NULL) {
			Assert_(*crsr != NULL);
			SetCursor(*crsr);
			sBWCursor = crsr;
		} else {
			SetCursor( &UQDGlobals::GetQDGlobals()->arrow );
			newID = cu_Arrow;
		}
		sCurrentCursorID = newID;
	}
}

Int16	UCursor::Get(void)
{
	return sCurrentCursorID;
}

void	UCursor::Restore()
{
	if (sColorCursor != NULL) {
		SetCCursor(sColorCursor);
	} else if (sBWCursor != NULL) {
		SetCursor(*sBWCursor);
	} else {
		SetCursor( &UQDGlobals::GetQDGlobals()->arrow );
	}
}

//	===========================================================================
//	StCursor:

StCursor::StCursor(Int16 inID)
{
	mFamilyID = inID;
	mOldCursorID = UCursor::Get();
	UCursor::Set(mFamilyID);
}

StCursor::~StCursor()
{
	UCursor::Set(mOldCursorID);
}
					
void	StCursor::Tick(Int16 inID)
{
	UCursor::Tick(inID);
}
