/*
 * @(#)CharacterDecoder.java	1.9 95/10/08 Chuck McManis
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

package sun.misc;

import java.io.OutputStream;
import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.io.ByteArrayInputStream;
import java.io.IOException;

/**
 * This class defines the decoding half of character encoders.
 * A character decoder is an algorithim for transforming 8 bit
 * binary data that has been encoded into text by a character
 * encoder, back into original binary form.
 * 
 * The character encoders, in general, have been structured 
 * around a central theme that binary data can be encoded into
 * text that has the form:
 *
 * <pre>
 *	[Buffer Prefix]
 *	[Line Prefix][encoded data atoms][Line Suffix]
 *	[Buffer Suffix]
 * </pre>
 *
 * Of course in the simplest encoding schemes, the buffer has no
 * distinct prefix of suffix, however all have some fixed relationship
 * between the text in an 'atom' and the binary data itself.
 *
 * In the CharacterEncoder and CharacterDecoder classes, one complete
 * chunk of data is referred to as a <i>buffer</i>. Encoded buffers 
 * are all text, and decoded buffers (sometimes just referred to as 
 * buffers) are binary octets.
 *
 * To create a custom decoder, you must, at a minimum,  overide three
 * abstract methods in this class.
 * <DL>
 * <DD>bytesPerAtom which tells the decoder how many bytes to 
 * expect from decodeAtom
 * <DD>decodeAtom which decodes the bytes sent to it as text.
 * <DD>bytesPerLine which tells the encoder the maximum number of
 * bytes per line.
 * </DL>
 *
 * In general, the character decoders return error in the form of a
 * CEFormatException. The syntax of the detail string is
 * <pre>
 *	DecoderClassName: Error message.
 * </pre>
 *
 * Several useful decoders have already been written and are 
 * referenced in the See Also list below.
 *
 * @version	08 Oct 1995, 1.9
 * @author	Chuck McManis
 * @see		CEFormatException
 * @see		CharacterEncoder
 * @see		UCDecoder
 * @see		UUDecoder
 * @see		BASE64Decoder
 */

public abstract class CharacterDecoder {

    /** Return the number of bytes per atom of decoding */
    abstract int bytesPerAtom();

    /** Return the maximum number of bytes that can be encoded per line */
    abstract int bytesPerLine();

    /** decode the beginning of the buffer, by default this is a NOP. */
    void decodeBufferPrefix(InputStream aStream, OutputStream bStream) throws IOException { }

    /** decode the buffer suffix, again by default it is a NOP. */
    void decodeBufferSuffix(InputStream aStream, OutputStream bStream) throws IOException { }

    /**
     * This method should return, if it knows, the number of bytes
     * that will be decoded. Many formats such as uuencoding provide
     * this information. By default we return the maximum bytes that
     * could have been encoded on the line.
     */
    int decodeLinePrefix(InputStream aStream, OutputStream bStream) throws IOException {
	return (bytesPerLine());
    }

    /**
     * This method post processes the line, if there are error detection
     * or correction codes in a line, they are generally processed by
     * this method. The simplest version of this method looks for the
     * (newline) character. 
     */
    void decodeLineSuffix(InputStream aStream, OutputStream bStream) throws IOException { }

    /**
     * This method does an actual decode. It takes the decoded bytes and
     * writes them to the OuputStream. The integer <i>l</i> tells the
     * method how many bytes are required. This is always <= bytesPerAtom().
     */
    void decodeAtom(InputStream aStream, OutputStream bStream, int l) throws IOException { 
	throw new CEStreamExhausted();
    }

    /**
     * This method works around the bizarre semantics of BufferedInputStream's
     * read method.
     */
    protected int readFully(InputStream in, byte buffer[], int offset, int len) 
	throws java.io.IOException {
	for (int i = 0; i < len; i++) {
	    int q = in.read();
	    if (q == -1)
		return ((i == 0) ? -1 : i);
	    buffer[i+offset] = (byte)q;
	}
	return len;
    }

    /**
     * Decode the text from the InputStream and write the decoded
     * octets to the OutputStream. This method runs until the stream
     * is exhausted.
     * @exception CEFormatException An error has occured while decoding
     * @exception CEStreamExhausted The input stream is unexpectedly out of data
     */
    public void decodeBuffer(InputStream aStream, OutputStream bStream) throws IOException {
	int	i;
	int	totalBytes = 0;

	decodeBufferPrefix(aStream, bStream);
	while (true) {
	    int length;

	    try {
	        length = decodeLinePrefix(aStream, bStream);
		for (i = 0; (i+bytesPerAtom()) < length; i += bytesPerAtom()) {
		    decodeAtom(aStream, bStream, bytesPerAtom());
		    totalBytes += bytesPerAtom();
	        }
		if ((i + bytesPerAtom()) == length) {
		    decodeAtom(aStream, bStream, bytesPerAtom());
		    totalBytes += bytesPerAtom();
		} else {
		    decodeAtom(aStream, bStream, length - i);
		    totalBytes += (length - i);
	        }
	        decodeLineSuffix(aStream, bStream);
	    } catch (CEStreamExhausted e) {
		break;
	    }
	}
	decodeBufferSuffix(aStream, bStream);
    }

    /**
     * Alternate decode interface that takes a String containing the encoded
     * buffer and returns a byte array containing the data.
     * @exception CEFormatException An error has occured while decoding
     */
    public byte decodeBuffer(String inputString)[] throws IOException {
	byte	inputBuffer[] = new byte[inputString.length()];
	ByteArrayInputStream inStream;
	ByteArrayOutputStream outStream;

	inputString.getBytes(0, inputString.length(), inputBuffer, 0);
	inStream = new ByteArrayInputStream(inputBuffer);
	outStream = new ByteArrayOutputStream();
	decodeBuffer(inStream, outStream);
	return (outStream.toByteArray());
    }

    /**
     * Decode the contents of the inputstream into a buffer.
     */
    public byte decodeBuffer(InputStream in)[] throws IOException {
	ByteArrayOutputStream outStream = new ByteArrayOutputStream();
	decodeBuffer(in, outStream);
	return (outStream.toByteArray());
    }
}
