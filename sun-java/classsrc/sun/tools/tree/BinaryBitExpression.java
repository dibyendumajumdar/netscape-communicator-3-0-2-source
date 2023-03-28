/*
 * @(#)BinaryBitExpression.java	1.9 95/09/14 Arthur van Hoff
 *
 * Copyright (c) 1994 Sun Microsystems, Inc. All Rights Reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * and its documentation for NON-COMMERCIAL purposes and without
 * fee is hereby granted provided that this copyright notice
 * appears in all copies. Please refer to the file "copyright.html"
 * for further important copyright and licensing information.
 *
 * SUN MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE SUITABILITY OF
 * THE SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, OR NON-INFRINGEMENT. SUN SHALL NOT BE LIABLE FOR
 * ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR
 * DISTRIBUTING THIS SOFTWARE OR ITS DERIVATIVES.
 */

package sun.tools.tree;

import sun.tools.java.*;
import sun.tools.asm.Assembler;

public
class BinaryBitExpression extends BinaryExpression {
    /**
     * constructor
     */
    public BinaryBitExpression(int op, int where, Expression left, Expression right) {
	super(op, where, left.type, left, right);
    }

    /**
     * Select the type
     */
    void selectType(Environment env, Context ctx, int tm) {
	if ((tm & TM_BOOLEAN) != 0) {
	    type = Type.tBoolean;
	} else if ((tm & TM_LONG) != 0) {
	    type = Type.tLong;
	} else {
	    type = Type.tInt;
	} 
	left = convert(env, ctx, type, left);
	right = convert(env, ctx, type, right);
    }

    /**
     * Code
     */
    public void codeValue(Environment env, Context ctx, Assembler asm) {
	left.codeValue(env, ctx, asm);
	right.codeValue(env, ctx, asm);
	codeOperation(env, ctx, asm);
    }
}
