//	===========================================================================
//	LTubeableItem.cp				�1994 Metrowerks Inc. All rights reserved.
//	===========================================================================

#ifdef PowerPlant_PCH
#include PowerPlant_PCH
#endif

#include	<LDataTube.h>
#include	"LTubeableItem.h"

#pragma	warn_unusedarg off

//	===========================================================================
//	Maintenance

LTubeableItem::LTubeableItem()
{
}

LTubeableItem::~LTubeableItem()
{
}

//	===========================================================================
//	Read:

FlavorType	LTubeableItem::PickFlavorFrom(const LDataTube *inTube) const
/*
	Warning:	Do not call inTube->AddSubTube from this method or
				infinite recursion may result.  Calling inTube->
				AddSubTube from this method would be incredibly 
				strange anyway.
*/
{
	return typeNull;
}

void	LTubeableItem::ReceiveDataFrom(LDataTube *inTube)
{
	Assert_(false);		//	Subclass must specifiy how to receive data.
}

//	===========================================================================
//	Write:

void	LTubeableItem::AddFlavorTypesTo(LDataTube *inTube) const
/*
	When overriding, remember to "inherit" any superclass methods as they might
	do useful things -- like adding the object specifier flavor for an
	LSelectableItem.
	
	Also, if this method is being overridden by something that is a collection
	of other tubeable items (like LSelection), this is the method where sub-
	tubes should be added.
	
	Do not have a tube which merely has a single sub-tube.
*/
{
//	inTube->AddFlavor(typeObjectSpecifier);

/*	Subclass template:

	inTube->AddFlavor(<your data type 1>);
	inTube->AddFlavor(<your data type 2>);
	...
	
	inherited::AddFlavorTypesTo(inTube);
*/
}

Boolean	LTubeableItem::SendFlavorTo(FlavorType inFlavor, LDataTube *inTube) const
/*
	When overriding, remember to "inherit" this method.
	
	This method also serves as a template for subclass methods.
	
	Return's true if the tube was filled when the flavor.  This is useful
	only for inheritence delegation purposes.
*/
{
/*	Int32	size = 0;
	
	switch(inFlavor) {

		case typeObjectSpecifier:
		{
			StAEDescriptor	desc;

			MakeSpecifier(desc.mDesc);
			inTube->SetFlavorHandle(inFlavor, desc.mDesc.dataHandle);
			return true;
			break;
		}

		case <your data type>:
			inTube->SetFlavorData(inFlavor, ptr, size);
			break;
		
		default:
			return inherited::SendFlavorTo(inTube);
			break;
	}
*/
	return false;
}

