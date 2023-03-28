/*
 * @@(#)MozillaAppletContext.java	1.4 95/07/31
 * 
 * Copyright (c) 1995 Netscape Communications Corporation. All Rights Reserved.
 * 
 */
package netscape.applet;

import java.applet.*;
import java.io.PrintStream;
import java.util.*;
import java.net.MalformedURLException;
import java.net.URL;
import java.awt.Image;
import java.awt.Toolkit;
import java.io.IOException;
import java.lang.SecurityManager;
import netscape.applet.AppletSecurity;
import netscape.javascript.JSObject;

// XXX we need to get rid of these
import sun.awt.image.URLImageSource;
import sun.audio.AudioData;

/**
 * A MozillaAppletContext provides the implmementation of the applet
 * context for an applet running in mozilla.
 */
class MozillaAppletContext implements AppletContext {
    // Shadow of our frame's MWContext for C code
    int frameMWContext;

    // The embedded applet frames present in this context. There is one
    // embedded applet frame per applet in the context. The key's are the
    // appletID's generated by mozilla. 
    // (We need to use a Mapping here so that iteration doesn't cause
    // memory to be allocated.)
    Mapping appletFrames;

    // Image cache for this page
    Hashtable imageCache;

    // Audio clip cache for this page
    Hashtable audioClipCache;

    // Document URL (the page that "contains" this context)
    URL documentURL;

    // Use Integer instead of int to avoid allocation during trimming:
    Integer contextID;

    MozillaAppletContext(Integer contextID) {
	this.contextID = contextID;
	appletFrames = new Mapping();
	imageCache = new Hashtable();
	audioClipCache = new Hashtable();
    }

    static void indent(PrintStream out, int i) {
	while (--i >= 0) {
	    out.print("  ");
	}
    }

    void dumpState(PrintStream out, int i) {
	indent(out, i);
	out.println("MozillaAppletContext id=" + contextID + " #frames=" + appletFrames.size()
	    + " #images=" + imageCache.size()
	    + " #audioClips=" + audioClipCache.size()
	    + " url=" + documentURL);
	int size = appletFrames.size();
	for (int j = 0; j < size; j++) {
	    EmbeddedAppletFrame eaf = (EmbeddedAppletFrame)
		appletFrames.elementAt(j);
	    eaf.dumpState(out, i+1);
	}
	if (imageCache.size() > 0) {
	    indent(out, i+1);
	    out.println("image cache:");
	    for (Enumeration e = imageCache.keys(); e.hasMoreElements(); ) {
		URL u = (URL) e.nextElement();
		indent(out, i+2);
		out.println(u.toString());
	    }
	}
	if (audioClipCache.size() > 0) {
	    indent(out, i+1);
	    out.println("audio clip cache:");
	    for (Enumeration e = audioClipCache.keys(); e.hasMoreElements(); ) {
		URL u = (URL) e.nextElement();
		indent(out, i+2);
		out.println(u.toString());
	    }
	}
	/*
	indent(out, i+2);
	out.println("Applet frames: ");
	appletFrames.dump(out, i+4);
	indent(out, i+2);
	out.println("Global applet contexts: ");
	appletContexts.dump(out, i+4);
	*/
    }

    // When we are destroyed zap references to the things we
    // contain to make the GC's life easier.
    void destroy() {
        if (debug > 0) {
            System.err.println("#   destroy acx="+this+" frameMWContext="
                               +frameMWContext);
        }
	destroyApplets();
	frameMWContext = 0;
	appletFrames = null;
	imageCache = null;
	audioClipCache = null;
	//	System.out.println("<<<<< removing ctxt:" + contextID + " from " + appletContexts + "");
	appletContexts.remove(contextID);
    }

    public AudioClip getAudioClip(URL url) {
	AudioClip clip = lookupAudioClip(this, url);
	return clip;
    }

    public Image getImage(URL url) {
	Image img = lookupImage(this, url);
	return img;
    }

    public Applet getApplet(String name) {
	int size = appletFrames.size();
	for (int i = 0; i < size; i++) {
	    EmbeddedAppletFrame frame = (EmbeddedAppletFrame)
		appletFrames.elementAt(i);
	    if (name.equals(frame.applet.getParameter("name"))) {
		return frame.applet;
	    }
	}
	return null;
    }

    public Enumeration getApplets() {
	int size = appletFrames.size();
	Vector v = new Vector(size);
	for (int i = 0; i < size; i++) {
	    EmbeddedAppletFrame frame = (EmbeddedAppletFrame)
		appletFrames.elementAt(i);
	    v.addElement(frame.applet);
	}
	return v.elements();
    }

    public void showDocument(URL url) {
        showDocument(url, "_top");
    }

    public void showDocument(URL url, String target) {
	pShowDocument(url.toExternalForm(), (String)null, target); // no current restriction on host names
    }

    public void showStatus(String status) {
//	System.err.println("######### Status: "+status);
	pShowStatus(status);
    }

    // signal to the rest of the world that applet is loaded
    public void mochaOnLoad(int result) {
//	System.err.println("called OnLoad");
	pMochaOnLoad(result);
    }

    private native void pShowDocument(String doc, String hostAddress, String target);
    private native void pShowStatus(String status);
    private native void pMochaOnLoad(int result);

    ////////////////////////////////////////////////////////////////////////////////
    // Cache support methods. These are static methods so that multiple
    // threads loading classes, audio or images will properly synchronize
    // so that the caches do not have duplicate values.

    // Look for an audio clip, given a URL, in each context's image cache
    static synchronized AudioClip lookupAudioClip(MozillaAppletContext incx, URL url) {
	// search all the other active contexts first:
	int size = appletContexts.size();
	for (int i = 0; i < size; i++) {
	    MozillaAppletContext acx = (MozillaAppletContext)
		appletContexts.elementAt(i);
	    Object clip = acx.audioClipCache.get(url);
	    if (clip != null) {
		return (AudioClip) clip;
	    }
	}

	// else create a new audio clip:
	AudioClip clip;
	clip = new AppletAudioClip(url);
	incx.audioClipCache.put(url, clip);
	if (debug > 0) {
	    System.err.println("# New audio clip: " + url);
	}
	return clip;
    }

    // Look for an image, given a URL, in each context's audio clip cache
    static synchronized Image lookupImage(MozillaAppletContext incx, URL url) {
	// search all the other active contexts first:
	int size = appletContexts.size();
	for (int i = 0; i < size; i++) {
	    MozillaAppletContext acx = (MozillaAppletContext)
		appletContexts.elementAt(i);
	    Object image = acx.imageCache.get(url);
	    if (image != null) {
		return (Image) image;
	    }
	}

	// else create a new image:
	Image image;
	try {	
	    URLImageSource source = new URLImageSource(url);
	    image = Toolkit.getDefaultToolkit().createImage(source);
	    incx.imageCache.put(url, image);
	    if (debug > 2) {
		System.err.println("# New image: " + url);
	    }
	} catch (Exception ex) {
	    image = null;
	}
	return image;
    }

    ////////////////////////////////////////////////////////////////////////////////

    synchronized void initApplet(int appletID, String docURL, Hashtable params) {
	// Convert argv into a hashtable. Argv comes from the applet tag
	// and from the param tags.
	if (debug > 0) {
	    System.err.println("#   initApplet: appletID=" + appletID);
	}

	// Remember the document URL
	try {
	    documentURL = new URL(docURL);
	} catch (MalformedURLException e) {
	    // What to do? The applet is going to be hosed because we
	    // can't fetch it's class. Punt. The implication here is that
	    // the browser is broken or that the URL parsing code in java
	    // disagrees with the browsers parsing code.
	    if (debug > 0) {
		System.err.println("#     Malformed documentURL: " + docURL);
	    }
	    // indicate to mocha that there was an error in initializing applet
	    mochaOnLoad(-1);
	    return;
	}

	// Clean up the codebase parameter
	String codebase = (String) params.get("codebase");
	if (codebase == null) {
	    // No codebase defined. Define it using the documentURL
	    codebase = docURL;
	    int tail = codebase.lastIndexOf('/') + 1;
	    codebase = codebase.substring(0, tail);
	} else {
	    // Codebase is defined. Use the URL class to convert the documentURL
	    // and the codebase into a new URL. codebase might be absolute or
	    // relative. URL will figure it out for us and make a URL that is
	    // absolute.
	    if (!codebase.endsWith("/")) {
		codebase = codebase + "/";
	    }
	    try {
		URL u = new URL(documentURL, codebase);
		codebase = u.toString();
		int tail = codebase.lastIndexOf('/') + 1;
		if (tail != codebase.length() - 1) {
		    codebase = codebase.substring(0, tail);
		}
	    } catch (MalformedURLException e) {
	    }
            // For security reasons, prevent folks from giving a file URL in the 
            // codebase explicitly (they might specificy root... and open up the disk!!)
            // Don't just do this test in the above try via gethost(), in case
            // that method throws an exception for whatever reason :-(
            if ((!docURL.startsWith("file:")) && (codebase.startsWith("file:")))
        	throw new AppletSecurityException("AppletContext: Can't use file:// URL in CODEBASE spec", 
                        codebase);
	}
	params.put("codebase", codebase);
	URL codebaseURL = documentURL;
	try {
	    codebaseURL = new URL(codebase);
	} catch (MalformedURLException e) {
	}

	// Clean up the archive parameter
	String archive = (String) params.get("archive");
	if (archive == null) {
	    // No archive defined. Define it using the codebaseURL
	    archive = docURL;
	    int tail = archive.lastIndexOf('/') + 1;
	    archive = archive.substring(0, tail);
	} else {
	    // Archive is defined. Use the URL class to convert the codebaseURL
	    // and the archive into a new URL. archive might be absolute or
	    // relative. URL will figure it out for us and make a URL that is
	    // absolute.
	    try {
		// if archive specifies a list of zip files, use only the first
		// one for now:
		int commaIndex = archive.indexOf(',');
		if (commaIndex != -1)
		    archive = archive.substring(0, commaIndex);
		
		URL u = new URL(codebaseURL, archive);
		archive = u.toString();
	    } catch (MalformedURLException e) {
	    }
	}
	params.put("archive", archive);
	URL archiveURL = codebaseURL;
	try {
	    archiveURL = new URL(archive);
	} catch (MalformedURLException e) {
	}

	// Now create a new EmbeddedAppletFrame. 
	Integer frameKey = new Integer(appletID);
	EmbeddedAppletFrame frame =
	    new EmbeddedAppletFrame(documentURL, codebaseURL, archiveURL,
				    params, this, frameKey);
	//	System.out.println("----- adding frame:"+frameKey+" value:"+frame+" to "+appletFrames);
	appletFrames.put(frameKey, frame);
	totalApplets++;
	if (debug > 0) {
	    System.err.println("#     total applets=" + totalApplets);
	}
	if (debug > 0) {
	    String p = "";
	    for (Enumeration e = params.keys(); e.hasMoreElements();) {
		Object key = e.nextElement();
		p = p + (String)key + "=" + (String)params.get(key) + " ";
	    }
	    System.err.println("#     New applet: " + appletID + " at "
			       + codebaseURL + " " + p);
	}

	// Fire up the applet. First get the "manager" thread created
	// and prepared to respond to sendEvent's. Also, parent the
	// viewer to mozilla's window.
	frame.start();
	frame.pack();
	frame.show();

	// Now tell the listener to load and initialize the applet
	frame.sendEvent(EmbeddedAppletFrame.APPLET_LOAD);
	frame.sendEvent(EmbeddedAppletFrame.APPLET_INIT);
    }

    // This takes an Integer rather than an int because it is called from 
    // destroyApplet which must not allocate memory. This is because destroyApplet
    // is calling during the trimming process.
    EmbeddedAppletFrame getAppletFrame(Integer appletID) {
	EmbeddedAppletFrame frame = (EmbeddedAppletFrame) appletFrames.get(appletID);
	if (frame == null && debug > 0) {
	    System.err.println("#   Warning: AppletFrame not found for appletID " + appletID);
	}
	return frame;
    }

    synchronized void startApplet(int appletID) {
	if (debug > 0) {
	    System.err.println("#   startApplet: appletID=" + appletID);
	}
	EmbeddedAppletFrame frame = getAppletFrame(new Integer(appletID));
	if (frame == null) {
	    // must have gotten pruned -- how to restart?
	    System.err.println("---> applet must have been pruned: "+appletID
			       + " on " + documentURL);
	    return;
	}
	frame.inHistory = false;
	frame.sendEvent(EmbeddedAppletFrame.APPLET_START);
    }

    synchronized void stopApplet(int appletID) {
	if (debug > 0) {
	    System.err.println("#   stopApplet: appletID=" + appletID);
	}
	EmbeddedAppletFrame frame = getAppletFrame(new Integer(appletID));
	if (frame == null) return;
	frame.inHistory = true;
	frame.sendEvent(EmbeddedAppletFrame.APPLET_STOP);
	trimApplets();	// trim in case we're over our threshold
    }

    synchronized void destroyApplet(Integer appletID) {
	if (noisyTrimming && debug > 0) {
	    System.err.println("#   destroyApplet: appletID=" + appletID);
	}
	EmbeddedAppletFrame frame = getAppletFrame(appletID);
	if (frame == null) return;
	totalApplets--;
	if (noisyTrimming && debug > 0) {
	    System.err.println("#     total applets=" + totalApplets);
	}
	if (!noisyTrimming)
	    frame.noisy = false;

	//	System.out.println("----- removing frame:"+appletID+" from "+appletFrames);
	appletFrames.remove(appletID, true);
	//	System.out.println("-----   remaining count = "+appletFrames.size());

	// Enter the monitor associated with the frame's manager thread's
	// thread group. This way we won't lose a notify when the
	// thread group notices it has no more threads.
	ThreadGroup group = frame.handler.getThreadGroup();
	synchronized (group) {
	    // Tell applet to destroy itself, nicely
	    //	    System.out.println("========== stopping " + frame);
	    frame.sendEvent(EmbeddedAppletFrame.APPLET_STOP);
	    //	    System.out.println("========== destroy " + frame);
	    frame.sendEvent(EmbeddedAppletFrame.APPLET_DESTROY);
	    //	    System.out.println("========== dispose " + frame);
	    frame.sendEvent(EmbeddedAppletFrame.APPLET_DISPOSE);
/*	    frame.sendEvent(EmbeddedAppletFrame.shutdownEvents);
 */
	    // Give the applet at most 1 second to exit. If it exits
	    // sooner then we will be notified when the group runs
	    // out of threads.
	    try {
	        //		System.out.println("========== waiting " + frame);
		group.wait(1000);
	    } catch (InterruptedException e) {
	    }

	    // Check and see if thread is really gone
	    if (group.activeCount() > 0) {
		// Whoops. Thread did not cooperate. Gun it down.
	        SecurityManager.setScopePermission();
		//		System.out.println("========== killing " + frame);
		group.stop();
	        SecurityManager.resetScopePermission();
		try {
		    //		    System.out.println("========== waiting again " + frame);
		    group.wait(10000);
		} catch (InterruptedException e) {
		}
	    }

	    // Finally, destroy the thread group
	    try {
	        SecurityManager.setScopePermission();
		//		System.out.println("========== destroying group " + frame);
		group.destroy();
	        SecurityManager.resetScopePermission();
	    } catch (Exception e) {
	    }
	}

	if (appletFrames.isEmpty()) {
	    if (debug > 0) {
		System.err.println("#   destroyApplet: destroying context for contextID " + contextID);
	    }
	    destroy();
	}
    }

    synchronized void iconifyApplets() {
	if (debug > 2) {
	    System.err.println("#   iconifyApplets");
	}
	int size = appletFrames.size();
	for (int i = 0; i < size; i++) {
	    EmbeddedAppletFrame frame = (EmbeddedAppletFrame)
		appletFrames.elementAt(i);
	    if (debug > 2)
		System.err.println("#     iconifyApplets: stopping appletID " + frame.pData);
	    frame.sendEvent(EmbeddedAppletFrame.APPLET_STOP);
	}
    }

    synchronized void uniconifyApplets() {
	if (debug > 2) {
	    System.err.println("#   uniconifyApplets");
	}
	int size = appletFrames.size();
	for (int i = 0; i < size; i++) {
	    EmbeddedAppletFrame frame = (EmbeddedAppletFrame)
		appletFrames.elementAt(i);
	    if (debug > 2)
		System.err.println("#     uniconifyApplets: starting appletID " + frame.pData);
	    frame.sendEvent(EmbeddedAppletFrame.APPLET_START);
	}
    }

    synchronized Object reflectApplet(int appletID) {
	if (debug > 2) {
	    System.err.println("#   reflectApplet: appletID=" + appletID);
	}
	EmbeddedAppletFrame frame = getAppletFrame(new Integer(appletID));
	if (frame == null) return null;
	return frame.applet;
    }

    // Destroy all applets, and the context itself.
    synchronized void destroyApplets() {
	if (debug > 0) {
	    System.err.println("#   destroyApplets");
	}
	// This enumeration works for us during removal because
	// (a) we know how Mapping works, and (b) we're always running on
	// a single thread (the mozilla thread):
	int size = appletFrames.size();
	for (int i = size - 1; i >= 0; i--) {
	    Integer frame = (Integer)appletFrames.keyAt(i);
	    destroyApplet(frame);
	}
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Static methods used by libjava to control java from mozilla
    ////////////////////////////////////////////////////////////////////////////////

    static Console console;

    static void init() {
	// Setup console
	console = new Console();

	// Setup system properties
	System.setProperties(new AppletProperties());

	// XXX Create and install a facist socket factory?

	// And also install a URLStreamHandlerFactory:
	URL.setURLStreamHandlerFactory(new netscape.net.URLStreamHandlerFactory());
	// Create and install the security manager LAST
	System.setSecurityManager(new AppletSecurity());
    }

    static void setConsoleState(int newstate) {
	setConsoleState0(newstate);
    }

    static native void setConsoleState0(int newstate);

    static void showConsole() {
	console.show();
    }

    static void hideConsole() {
	console.hide();
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Applet Trimming

    static int totalApplets;
    static int trimThreshold;		// set in lj_init.c
    static boolean noisyTrimming;	// set in lj_init.c

    // this entry point is called from native code to force trimming:
    // returns the number actually trimmed:
    // Warning: This function should not allocate memory (if possible)
    // because it gets called in low-memory conditions.
    static int trimApplets(int numberToTrim, boolean trimOnlyStoppedApplets) {
	// first try to trim only the stopped applets:
	int numberTrimmed = trimApplets1(numberToTrim, true);
	// if that wasn't enough, and we want to trim live applets, try again:
	if (numberTrimmed < numberToTrim && !trimOnlyStoppedApplets)
	    numberTrimmed += trimApplets1(numberToTrim - numberTrimmed, false);
	return numberTrimmed;
    }

    static int trimApplets1(int numberToTrim, boolean trimOnlyStoppedApplets) {
	int numberTrimmed = 0;
	while (numberToTrim-- > 0) {
	    long oldestTime = -1;
	    EmbeddedAppletFrame oldestFrame = null;
	    MozillaAppletContext contextOfOldest = null;
	    int size = appletContexts.size();
	    for (int i = 0; i < size; i++) {
		MozillaAppletContext acx = (MozillaAppletContext)
		    appletContexts.elementAt(i);
		int size2 = acx.appletFrames.size();
		for (int j = 0; j < size2; j++) {
		    EmbeddedAppletFrame frame = (EmbeddedAppletFrame)
			acx.appletFrames.elementAt(j);
		    if (!trimOnlyStoppedApplets || frame.inHistory) {
			if (oldestTime == -1 || frame.timestamp < oldestTime) {
			    oldestTime = frame.timestamp;
			    oldestFrame = frame;
			    contextOfOldest = acx;
			}
		    }
		}
	    }

	    if (oldestFrame == null) {
		if (noisyTrimming && debug > 0)
		    System.err.println("# No stopped applets to prune.");
		return numberTrimmed;
	    }

	    try {
		if (noisyTrimming) {
		    Applet applet = oldestFrame.applet;
		    String name = null;
		    if (applet != null)
			name = applet.getParameter("name");
		    if (name == null) 
			name = applet.getClass().getName().toString();
		    System.err.println("# Pruning applet " + name
				       + " from " + contextOfOldest.documentURL
				       + " to save memory.");
		    if (debug > 0) {
			System.err.println("#   Pruning appletID=" + oldestFrame.pData
					   + " contextID=" + contextOfOldest.contextID
					   + " applet=" + oldestFrame.applet);
		    }
		}
	    } catch (Throwable e) {
		// can't do anything here (like println or printStackTrace) 
		// because it would cause allocation 
	    }

	    try {
		contextOfOldest.destroyApplet(oldestFrame.appletID);
	    } catch (Throwable e) {
		// can't do anything here (like println or printStackTrace) 
		// because it would cause allocation 
	    }
	    numberTrimmed++;
	}
	return numberTrimmed;
    }

    // this entry point is called whenever an applet is created or stopped:
    static void trimApplets() {
	trimApplets(totalApplets - trimThreshold, true);
    }

    ////////////////////////////////////////////////////////////////////////////////

    // (We need to use a Mapping here so that iteration doesn't cause
    // memory to be allocated.)
    static Mapping appletContexts = new Mapping();

    // Debugging flag
    static int debug = 0;

    static MozillaAppletContext getAppletContext(int contextID) {
	Integer key = new Integer(contextID);
	MozillaAppletContext context = (MozillaAppletContext)appletContexts.get(key);
	if (context == null && debug > 0) {
	    System.err.println("# Warning: applet context not found for contextID " + contextID);
	}
	return context;
    }

    static MozillaAppletContext ensureAppletContext(int contextID) {
	Integer key = new Integer(contextID);
	MozillaAppletContext context = (MozillaAppletContext)appletContexts.get(key);
	if (context == null) {
	    context = new MozillaAppletContext(key);
	    //	    System.out.println(">>>>> adding ctxt:" + key +" value:"+ context + " to " + appletContexts + "");
	    appletContexts.put(key, context);
	}
	return context;
    }

    static void initApplet(int parentContext, int frameContext, int contextID,
    			   int appletID, String argv[]) {
	Hashtable params = new Hashtable();
	for (int i = 1 ; i < argv.length ; i++) {
	    String str = argv[i];
	    int j = str.indexOf('=');
	    if (j < 0) {
		continue;
	    }
	    params.put(str.substring(0, j).toLowerCase(), str.substring(j+1));
	}

	// Turn on debugging if told to
	String dbg = (String) params.get("debug");
	if (dbg != null) {
	    try {
		debug = Integer.parseInt(dbg);
	    } catch (Exception e) {
	    }
	}
	
	if (debug > 0) {
	    System.err.println("# initApplet: contextID="+contextID
			       +" appletID="+appletID
			       +" parentContext="+parentContext
			       +" frameContext="+frameContext);
	}
	trimApplets();
	MozillaAppletContext acx = ensureAppletContext(contextID);

	/*
	** This next line is needed by the C code that may look up the
	** MWContext while the class is loading in order to put up a
	** (security) dialog:
	*/
/*	if (debug > 0) {
	    System.err.println("#   initApplet: setting frameMWContext, acx="
                               +acx+" old frameMWContext="+acx.frameMWContext
                               +" new frameMWContext="+frameContext);
        }
*/	acx.frameMWContext = frameContext;

	acx.initApplet(appletID, argv[0], params);
    }

    static void startApplet(int contextID, int appletID, int newFrameMWContext) {
	if (debug > 0) {
	    System.err.println("# startApplet: contextID=" + contextID
			       +" appletID=" + appletID
			       +" newFrameMWContext="+newFrameMWContext);
	}
	MozillaAppletContext acx = getAppletContext(contextID);
	if (acx == null) return;
	acx.frameMWContext = newFrameMWContext;
	acx.startApplet(appletID);
    }

    static void stopApplet(int contextID, int appletID) {
	if (debug > 0) {
	    System.err.println("# stopApplet: contextID=" + contextID
			       + " appletID=" + appletID);
	}
	MozillaAppletContext acx = getAppletContext(contextID);
	if (acx == null) return;
	acx.frameMWContext = 0;
	acx.stopApplet(appletID);

	// Try to trim applets again. Since we only trim visible ones, we
	// may have more than trimThreshold outstanding. By trimming them
	// when we destroy applets (as well as when we create them) we can
	// save space sooner.
	trimApplets();
    }

    static void destroyApplet(int contextID, int appletID) {
	if (debug > 0) {
	    System.err.println("# destroyApplet: contextID=" + contextID
			       + " appletID=" + appletID);
	}
	MozillaAppletContext acx = getAppletContext(contextID);
	if (acx == null) return;
	acx.destroyApplet(new Integer(appletID));
    }

    /**
     * reflect the applet into the JavaScript world
     */
    static Object reflectApplet(int contextID, int appletID) {
	if (debug > 2) {
	    System.err.println("# reflectApplet: contextID=" + contextID
			       + " appletID=" + appletID);
	}
	MozillaAppletContext acx = getAppletContext(contextID);
	if (acx == null) return null;
	return acx.reflectApplet(appletID);
    }

    static void iconifyApplets(int contextID) {
	if (debug > 2) {
	    System.err.println("# iconifyApplets: contextID=" + contextID);
	}
	MozillaAppletContext acx = getAppletContext(contextID);
	if (acx == null) return;
	acx.iconifyApplets();
    }

    static void uniconifyApplets(int contextID) {
	if (debug > 2) {
	    System.err.println("# uniconifyApplets: contextID=" + contextID);
	}
	MozillaAppletContext acx = getAppletContext(contextID);
	if (acx == null) return;
	acx.uniconifyApplets();
    }

    // should be destroyAppletContext
    static void destroyApplets(int contextID) {
	if (debug > 0) {
	    System.err.println("# destroyApplets: contextID=" + contextID);
	}
	MozillaAppletContext acx = getAppletContext(contextID);
	if (acx == null) return;
	acx.destroy();
    }

    static void destroyAll() {
	if (debug > 0) {
	    System.err.println("# destroyAll");
	}
	// This enumeration works for us during removal because
	// (a) we know how Mapping works, and (b) we're always running on
	// a single thread (the mozilla thread):
	int size = appletContexts.size();
	for (int i = size - 1; i >= 0; i--) {
	    MozillaAppletContext acx = (MozillaAppletContext)
		appletContexts.elementAt(i);
	    acx.destroy();
	}
    }

    static void dumpState(PrintStream out) {
	int size = appletContexts.size();
	for (int i = 0; i < size; i++) {
	    MozillaAppletContext acx = (MozillaAppletContext)
		appletContexts.elementAt(i);
	    acx.dumpState(out, 0);
	}
    }
}

////////////////////////////////////////////////////////////////////////////////
// Mapping: This utility act as both a Hashtable and an Enumeration, but
// does not require allocation to do the iteration. Normal Hashtables require
// you to allocate the Enumerator which means we can't use it during gc.

class Mapping {
    Vector keys = new Vector();
    Vector values = new Vector();

    ////////////////////////////////////////////////////////////////////////////////
    // This interface lets you iterate through the Mapping:

    synchronized int size() {
	return keys.size();
    }

    synchronized Object keyAt(int index) {
	return keys.elementAt(index);
    }

    synchronized Object elementAt(int index) {
	return values.elementAt(index);
    }

    void dump(PrintStream out, int i) {
	MozillaAppletContext.indent(out, i);
	out.println(this);
	MozillaAppletContext.indent(out, i+2);
	out.println("keys: " + keys);
	MozillaAppletContext.indent(out, i+2);
	out.println("vals: " + values);
    }

    ////////////////////////////////////////////////////////////////////////////////
    // This interface lets you deal with the Mapping as a Hashtable:

    synchronized Object get(Object key) {
	int size = keys.size();
	for (int i = 0; i < size; i++) {
	    if (keys.elementAt(i).equals(key)) {
		return values.elementAt(i);
	    }
	}
	return null;
    }

    synchronized Object put(Object key, Object value) {
	Object result = remove(key);
	keys.addElement(key);
	values.addElement(value);
	return result;
    }

    Object remove(Object key) {
	return remove(key, false);
    }

    synchronized Object remove(Object key, boolean trace) {
	int size = keys.size();
	for (int i = 0; i < size; i++) {
	    if (keys.elementAt(i).equals(key)) {
		keys.removeElementAt(i);
		Object result = values.elementAt(i);
		values.removeElementAt(i);
		//		if (trace)
		//		    System.out.println("!!! removed key:"+key+" value:"+result+" from "+this);
		return result;
	    }
	}
	//	if (trace)
	//	    System.out.println("!!! removed nothing from "+this);
	return null;
    }

    synchronized boolean isEmpty() {
	return keys.isEmpty();
    }
}

////////////////////////////////////////////////////////////////////////////////
