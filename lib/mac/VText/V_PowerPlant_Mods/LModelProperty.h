// ===========================================================================
//	LModelProperty.h		   �1993-1996 Metrowerks Inc. All rights reserved.
// ===========================================================================

#ifndef _H_LModelProperty
#define _H_LModelProperty
#pragma once

#include <LModelObject.h>


class	LModelProperty : public LModelObject {
public:
					LModelProperty(
							DescType			inPropertyID,
							LModelObject		*inSuperModel,
							Boolean				inBeLazy = true);
	
	virtual void	HandleAppleEvent(
							const AppleEvent	&inAppleEvent,
							AppleEvent			&outAEReply,
							AEDesc				&outResult,
							Int32				inAENumber);
									 
	void			SendSetDataAE(
							DescType			inDataType,
							Ptr					inDataPtr,
							Size				inDataSize,
							Boolean				inExecute = true);
									 
	void			SendSetDataAEDesc(
							const AEDesc		&inDesc,
							Boolean				inExecute = true);
									
	virtual Boolean	CompareToModel(
							DescType		inComparisonOperator,
							LModelObject	*inCompareModel) const;
							
	virtual Boolean	CompareToDescriptor(
							DescType			inComparisonOperator,
							const AEDesc		&inCompareDesc) const;
											
protected:
	DescType		mPropertyID;

	virtual void	MakeSelfSpecifier(
							AEDesc				&inSuperSpecifier,
							AEDesc				&outSelfSpecifier) const;
									  
	virtual void	HandleGetData(
							const AppleEvent	&inAppleEvent,
							AEDesc				&outResult,
							Int32				inAENumber);

	virtual void	HandleSetData(
							const AppleEvent	&inAppleEvent,
							AppleEvent			&outAEReply);
};	

#endif