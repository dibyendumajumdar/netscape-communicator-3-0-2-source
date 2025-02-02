/*
	LFlexTableGeometry.cp

	� 1996 Netscape Communications, Inc.
	
	Created 3/25/96 - Tim Craycroft

*/

#include "LFlexTableGeometry.h"


/*
	LFlexTableGeometry::LFlexTableGeometry	
*/
LFlexTableGeometry::LFlexTableGeometry(	LTableView		*inTableView,
										LTableHeader	*inTableHeader)
	: LTableGeometry(inTableView)
{
	mTableHeader = inTableHeader;
	mRowCount 	 = 0;
	
	// set to a reasonable default
	//
	// this class has no other inteligence for determining this value
	//
	SetRowHeight(16, 0, 0);
}



/*
	LFlexTableGeometry::GetImageCellBounds	
	
	Get the column position from the header view and compute
	the row position.
*/
void		
LFlexTableGeometry::GetImageCellBounds(	const STableCell	&inCell,
										Int32				&outLeft,
										Int32				&outTop,
										Int32				&outRight,
										Int32				&outBottom) const
{	
	if (inCell.col <= mTableHeader->CountColumns())
	{
		outLeft  = mTableHeader->GetColumnPosition(inCell.col);
		outRight = outLeft + mTableHeader->GetColumnWidth(inCell.col);
			
		outTop 		= (inCell.row-1) * mRowHeight;
		outBottom	= outTop + mRowHeight;
	}
	else {
		outLeft = outTop = outRight = outBottom = 0;
	}
}



/*
	LFlexTableGeometry::GetRowHitBy	
*/
TableIndexT	
LFlexTableGeometry::GetRowHitBy(const SPoint32	&inImagePt) const
{
	return (inImagePt.v / mRowHeight) + 1;
}
								

/*
	LFlexTableGeometry::GetColHitBy	
*/
TableIndexT	
LFlexTableGeometry::GetColHitBy(const SPoint32	&inImagePt) const
{
	UInt16	i, nColumns;
	
	nColumns = mTableHeader->CountVisibleColumns();
	
	for (i=2; i<=nColumns; i++)
	{
		if (inImagePt.h < mTableHeader->GetColumnPosition(i)) {
			return i-1;
		}
	}
	
	return nColumns;

}



/*
	LFlexTableGeometry::SetRowHeight
	
	Changes row height for ALL rows and sets the scroll
	unit of the table view to the row height.	
*/
void
LFlexTableGeometry::SetRowHeight(	Uint16		inHeight,
									TableIndexT	inFromRow,
									TableIndexT	inToRow)
{
	#pragma unused(inFromRow, inToRow)
	
	mRowHeight = inHeight;
	
	SPoint32	scrollUnit;
	scrollUnit.h = 0;
	scrollUnit.v = mRowHeight;
	mTableView->SetScrollUnit(scrollUnit);
}



/*
	LFlexTableGeometry::GetTableDimensions
*/
void
LFlexTableGeometry::GetTableDimensions(	Uint32		&outWidth,
										Uint32		&outHeight) const
{
	outHeight 	= CountRows() * mRowHeight;
	outWidth	= mTableHeader->GetHeaderWidth();
}										
	


/*
	LFlexTableGeometry::GetRowHeight
*/
Uint16
LFlexTableGeometry::GetRowHeight(TableIndexT	inRow) const
{
	#pragma unused(inRow)
	return mRowHeight;
}
								
								
/*
	LFlexTableGeometry::GetColWidth
*/
Uint16		
LFlexTableGeometry::GetColWidth(TableIndexT	inCol) const
{
	return mTableHeader->GetColumnWidth(inCol);
}
							
							

/*
	LFlexTableGeometry::InsertRows
*/
void		
LFlexTableGeometry::InsertRows(	Uint32		inHowMany,
								TableIndexT	inAfterRow )
{
	#pragma unused(inAfterRow)
	
	mRowCount += inHowMany;
}							

	
/*
	LFlexTableGeometry::RemoveRows
*/
void		
LFlexTableGeometry::RemoveRows(	Uint32		inHowMany,
								TableIndexT	inFromRow)
{
	#pragma unused(inFromRow)

	mRowCount -= inHowMany;
}
								
							

/*
	LFlexTableGeometry::CountRows
*/
UInt32				
LFlexTableGeometry::CountRows() const
{
	return mRowCount;
}
	
	
	
	
/*
	LFlexTableGeometry::SetColWidth
	
	Invalid, since column widths are controlled by the mTableHeader
	widget.
*/
void		
LFlexTableGeometry::SetColWidth(Uint16		inWidth,
								TableIndexT inFromCol,
								TableIndexT	inToCol)
{
	#pragma unused(inWidth, inFromCol, inToCol)
	
	// We get our widths from mTableHeader
	SignalPStr_("\pLFlexTableGeometry does not support SetColWidth");
}								




								
							
		