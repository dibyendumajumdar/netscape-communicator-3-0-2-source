/*
 * @(#)MCanvasPeer.java	1.10 95/12/08 Sami Shaio
 *
 * Copyright (c) 1995 Sun Microsystems, Inc. All Rights Reserved.
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
package sun.awt.win32;

import java.awt.*;
import java.awt.peer.*;

class MCanvasPeer extends MComponentPeer implements CanvasPeer {
    native void create(MComponentPeer parent);
    native void scroll(int dx, int dy);

    MCanvasPeer(Component target) {
	super(target);
    }

    MCanvasPeer(Component target, Object arg) {
	super(target, arg);
    }
}
