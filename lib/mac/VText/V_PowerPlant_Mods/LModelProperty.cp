// ===========================================================================
//	LModelProperty.cp		   �1993-1996 Metrowerks Inc. All rights reserved.
// ===========================================================================
//
//	Class for a Property defined by the Apple Event Object Model. A Property,
//	identified by a 4-character ID, is associated with an LModelObject. You
//	change the value of a Property by sending SetData and GetData Apple
//	Events.
//
//	This class handles SetData, GetData, and GetDataSize Apple Events by
//	calling the SetAEProperty and GetAEProperty functions of the LModelObject
//	which owns the LModelProperty.
//
//	Normally, you will not need to create subclasses of LModelProperty. You
//	support Properties by overriding SetAEProperty and GetAEProperty for
//	a subclass of LModelObject.

#ifdef PowerPlant_PCH
#include PowerPlant_PCH
#endif

#include <LModelProperty.h>
#include <LModelDirector.h>
#include <UAppleEventsMgr.h>

#ifndef __AEREGISTRY__
#include <AERegistry.h>
#endif

#ifndef __AEOBJECTS__
#include <AEObjects.h>
#endif

#ifndef __AEPACKOBJECT__
#include <AEPackObject.h>
#endif


// ---------------------------------------------------------------------------
//		� LModelProperty
// ---------------------------------------------------------------------------
//	Constructor

LModelProperty::LModelProperty(
	DescType		inPropertyID,
	LModelObject	*inSuperModel,
	Boolean			inBeLazy)
		: LModelObject(inSuperModel, cProperty)
{
	mPropertyID = inPropertyID;
	SetLaziness(inBeLazy);
}


// ---------------------------------------------------------------------------
//		� HandleAppleEvent
// ---------------------------------------------------------------------------
//	Respond to an AppleEvent
//
//	ModelProperties can respond to "GetData", "GetDataSize", and "SetData"
//	AppleEvents

void
LModelProperty::HandleAppleEvent(
	const AppleEvent	&inAppleEvent,
	AppleEvent			&outAEReply,
	AEDesc				&outResult,
	Int32				inAENumber)
{
	switch (inAENumber) {
	
		case ae_GetData:
		case ae_GetDataSize:
			HandleGetData(inAppleEvent, outResult, inAENumber);
			break;
			
		case ae_SetData:
			HandleSetData(inAppleEvent, outAEReply);
			break;
		
		default:
			LModelObject::HandleAppleEvent(inAppleEvent, outAEReply,
									outResult, inAENumber);
			break;
	}
	
}


// ---------------------------------------------------------------------------
//		� SendSetDataAE
// ---------------------------------------------------------------------------
//	Send this program a "SetData" AppleEvent for a ModelProperty, with
//	the Property value specified by a data type, data ptr, and data length

void
LModelProperty::SendSetDataAE(
	DescType	inDataType,
	Ptr			inDataPtr,
	Size		inDataSize,
	Boolean		inExecute)
{
	OSErr		err;
	
	AppleEvent	theAppleEvent	= {typeNull, nil};
	UAppleEventsMgr::MakeAppleEvent(kAECoreSuite, kAESetData, theAppleEvent);

	StAEDescriptor	propertySpec;
	MakeSpecifier(propertySpec.mDesc);
	err = ::AEPutParamDesc(&theAppleEvent, keyDirectObject, propertySpec);
	ThrowIfOSErr_(err);
	
	err = ::AEPutParamPtr(&theAppleEvent, keyAEData, inDataType, inDataPtr,
									inDataSize);
	ThrowIfOSErr_(err);
	
	UAppleEventsMgr::SendAppleEvent(theAppleEvent, inExecute);
}


// ---------------------------------------------------------------------------
//		� SendSetDataAEDesc
// ---------------------------------------------------------------------------
//	Send this program a "SetData" AppleEvent for a ModelProperty, with the
//	Property value specified by an AE descriptor record

void
LModelProperty::SendSetDataAEDesc(
	const AEDesc	&inDesc,
	Boolean			inExecute)
{
	OSErr		err;
	
	AppleEvent	theAppleEvent	= {typeNull, nil};
	UAppleEventsMgr::MakeAppleEvent(kAECoreSuite, kAESetData, theAppleEvent);

	StAEDescriptor	propertySpec;
	MakeSpecifier(propertySpec.mDesc);

	err = ::AEPutParamDesc(&theAppleEvent, keyDirectObject, propertySpec);
	ThrowIfOSErr_(err);
	
	err = ::AEPutParamDesc(&theAppleEvent, keyAEData, &inDesc);
	ThrowIfOSErr_(err);
	
	UAppleEventsMgr::SendAppleEvent(theAppleEvent, inExecute);
}


// ---------------------------------------------------------------------------
//		� CompareToDescriptor
// ---------------------------------------------------------------------------
//	Return the result of comparing a ModelProperty to the value in an
//	AppleEvent descriptor

Boolean
LModelProperty::CompareToDescriptor(
	DescType		inComparisonOperator,
	const AEDesc	&inCompareDesc) const
{
	StAEDescriptor	propDesc;
	AEDesc			requestedType = {typeNull, nil};
	mSuperModel->GetAEProperty(mPropertyID, requestedType, propDesc.mDesc);
	Boolean result = UAppleEventsMgr::CompareDescriptors(propDesc,
								inComparisonOperator, inCompareDesc);
	return result;
}


// ---------------------------------------------------------------------------
//		� CompareToModel
// ---------------------------------------------------------------------------
//	Return the result of comparing a ModelProperty to a model object.

Boolean
LModelProperty::CompareToModel(
	DescType		inComparisonOperator,
	LModelObject*	inCompareModel) const
{
	Boolean	result = false;
	
	StAEDescriptor	propDesc;
	AEDesc			requestedType = {typeNull, nil};

	mSuperModel->GetAEProperty(mPropertyID, requestedType, propDesc.mDesc);
	if (propDesc.mDesc.descriptorType == typeObjectSpecifier) {
		StAEDescriptor	tokenD;
		LModelObject	*model = NULL;
		OSErr	err = LModelDirector::Resolve(propDesc, tokenD.mDesc);
		if (err == noErr)
			model = LModelObject::GetModelFromToken(tokenD);
		if (model)
			result = model->CompareToModel(inComparisonOperator, inCompareModel);
	} else {
		result = LModelObject::CompareToModel(inComparisonOperator, inCompareModel);
	}
	
	return result;
}	


// ---------------------------------------------------------------------------
//		� MakeSelfSpecifier
// ---------------------------------------------------------------------------
//	Pass back an AppleEvent object specifier for a ModelProperty

void
LModelProperty::MakeSelfSpecifier(
	AEDesc&		inSuperSpecifier,
	AEDesc&		outSelfSpecifier) const
{
		// Make descriptor for the property
		
	StAEDescriptor	keyData;
	OSErr	err = ::AECreateDesc(typeType, (Ptr) &mPropertyID,
									sizeof(mPropertyID), keyData);
	ThrowIfOSErr_(err);
	
		// Make ospec for the property
		
	err = ::CreateObjSpecifier(cProperty, &inSuperSpecifier, formPropertyID,
									keyData, false, &outSelfSpecifier);
	ThrowIfOSErr_(err);
}


// ---------------------------------------------------------------------------
//		� HandleGetData
// ---------------------------------------------------------------------------
//	Respond to a "GetData" or "GetDataSize" AppleEvent for a ModelProperty
//	by loading the value or size of the property's data into a
//	descriptor record

void
LModelProperty::HandleGetData(
	const AppleEvent	&inAppleEvent,
	AEDesc				&outResult,
	long				inAENumber)
{
									// Find requested type for the data
									// This parameter is optional, so it's OK
									//   if it's not found
	StAEDescriptor	requestedType;
	requestedType.GetOptionalParamDesc(inAppleEvent, keyAERequestedType,
									typeAEList);
	
									// Error if there are more parameters
	UAppleEventsMgr::CheckForMissedParams(inAppleEvent);
	
									// Ask SuperModel for the property value
	GetSuperModel()->GetAEProperty(mPropertyID, requestedType, outResult);
		
	if (inAENumber == ae_GetDataSize) {
									// For GetDataSize, size of Property
									//   is the result
		Int32	theSize = GetHandleSize(outResult.dataHandle);
		::AEDisposeDesc(&outResult);
		outResult.dataHandle = nil;
		OSErr err = ::AECreateDesc(typeLongInteger, &theSize, sizeof(Int32),
								&outResult);
		ThrowIfOSErr_(err);
	}
}


// ---------------------------------------------------------------------------
//		� HandleSetData
// ---------------------------------------------------------------------------
//	Respond to a "SetData" AppleEvent for a ModelProperty

void
LModelProperty::HandleSetData(
	const AppleEvent	&inAppleEvent,
	AppleEvent			&outAEReply)
{
	StAEDescriptor	value;			// Extract value to which to set
	value.GetParamDesc(inAppleEvent, keyAEData, typeWildCard);
	
	UAppleEventsMgr::CheckForMissedParams(inAppleEvent);

	GetSuperModel()->SetAEProperty(mPropertyID, value, outAEReply);
}
