/*
 * @(#)CharExpression.java	1.9 95/09/14 Arthur van Hoff
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
import java.io.PrintStream;

public
class CharExpression extends IntegerExpression {
    /**
     * Constructor
     */
    public CharExpression(int where, char value) {
	super(CHARVAL, where, Type.tChar, value);
    }

    /**
     * Print
     */
    public void print(PrintStream out) {
	out.print(value + "c");
    }
}
