/* -*- Mode: C; tab-width: 8 -*-
   icons.c --- icons and stuff
   Copyright � 1997 Netscape Communications Corporation, all rights reserved.
   Created: Jamie Zawinski <jwz@netscape.com>, 19-Oct-94.
 */

#include "mozilla.h"
#include "xfe.h"
#include "fonts.h"
#include "e_kit.h"

#include <X11/IntrinsicP.h>	/* just for core.width/height!! */
#include <Xm/PushBP.h>		/* For fast updating of the button pixmap... */

#include "../../lib/xp/biglogo.h"
#include "../../lib/xp/photo.h"
#include "../../lib/xp/hype.h"
#ifdef HAVE_SECURITY
# include "../../lib/xp/rsalogo.h"
# include "xp_sec.h"
#endif
#ifdef JAVA
# include "../../lib/xp/javalogo.h"
#endif
#ifdef HAVE_QUICKTIME
# include "../../lib/xp/qt_logo.h"
#endif
#include "icons/deutsch.h"
#include "icons/francais.h"
#include "icons/nihongo.h"
#ifdef EDITOR
#include "icons/editor.h"
#endif

#include "icondata.h"

#include "icons.h"

#include "nspr/prcpucfg.h"  /* for IS_LITTLE_ENDIAN / IS_BIG_ENDIAN */

/* for XP_GetString() */
#include <xpgetstr.h>
extern int XFE_SECURITY_WITH;
extern int XFE_SECURITY_DISABLED;


extern char fe_LicenseData[];

#define DELAYED_ICON_BORDER	1
#define DELAYED_ICON_PAD	2

static struct fe_icon fe_icons[512 + MAX_ANIM_FRAMES * MAX_ANIMS] = { { 0, } };

Pixel *fe_icon_pixels = 0;

void
fe_InitIconColors (MWContext *context)
{
  int i;
  static Boolean done = False;
  Pixel pixel;

  /* If anyone adds any colors - we want to know about it. */
  XP_ASSERT(fe_n_icon_colors <= 22);

  if (CONTEXT_DATA (context)->icon_colors_initialized) return;
  CONTEXT_DATA (context)->icon_colors_initialized = True;

  if (!fe_icon_pixels)
    fe_icon_pixels = (Pixel *) malloc (sizeof (Pixel) * 256);
    /*fe_icon_pixels = (Pixel *) malloc (sizeof (Pixel) * fe_n_icon_colors);*/
  
  for (i = 0; i < fe_n_icon_colors; i++)
    {
      XColor color;
      color.red   = fe_icon_colors [i][0];
      color.green = fe_icon_colors [i][1],
      color.blue  = fe_icon_colors [i][2];
      pixel = fe_GetPermanentPixel (context, color.red, color.green, color.blue);
      
/*      if (done)
        assert(fe_icon_pixels [i] == pixel);
 */
      fe_icon_pixels [i] = pixel;
    }

  done = True;
}


static char *
fe_get_app_dir(Display *dpy)
{
	char		clas[64];
	XrmDatabase	db;
	char		instance[64];
	char		*type;
	XrmValue	value;

	db = XtDatabase(dpy);
	PR_snprintf(instance, sizeof (instance), "%s.appDir", fe_progclass);
	PR_snprintf(clas, sizeof (clas), "%s.AppDir", fe_progclass);
	if (XrmGetResource(db, instance, clas, &type, &value))
	{
		return value.addr;
	}

#ifdef __sun
	return "/usr/openwin/lib/netscape";
#else
	return "/usr/lib/X11/netscape";
#endif
}


static char *
fe_get_xpm_string(FILE *f, int size)
{
	static int	alloc;
	static char	*buf = NULL;
	int		c;
	int		i;

	if (buf)
	{
		if (size > alloc)
		{
			alloc = size;
			buf = realloc(buf, alloc);
			if (!buf)
			{
				return NULL;
			}
		}
	}
	else
	{
		if (size > 128)
		{
			alloc = size;
		}
		else
		{
			alloc = 128;
		}
		buf = malloc(alloc);
		if (!buf)
		{
			return NULL;
		}
	}

	do
	{
		c = getc(f);
	}
	while ((c != '"') && (c != EOF));

	for (i = 0; i < alloc; i++)
	{
		c = getc(f);
		buf[i] = c;
		if ((c == EOF) || (buf[i] == '"'))
		{
			break;
		}
	}

	if (buf[i] != '"')
	{
		do
		{
			c = getc(f);
		}
		while ((c != '"') && (c != EOF));
	}

	buf[i] = 0;

	return buf;
}


typedef struct {
	char	type[16];
	char	spec[16];
} xpm_color_entry;


typedef struct {
	unsigned int	mask : 1;
	unsigned int	mono : 1;
	unsigned char	color;
} fe_color_entry;

static fe_color_entry fe_color_entries[128];

void
fe_process_color(int index, xpm_color_entry *entry)
{
	int	b;
	int	dist;
	int	g;
	int	i;
	int	min;
	int	min_i;
	int	r;

	switch (entry->type[0])
	{
	case 'm':
		if (entry->spec[0] == '#')
		{
			if (entry->spec[1] == '0')
			{
				fe_color_entries[index].mono = 1;
			}
		}
		break;
	case 'c':
		if (entry->spec[0] == '#')
		{
			sscanf(entry->spec + 1, "%2x%2x%2x", &r, &g, &b);
			r |= (r << 8);
			g |= (g << 8);
			b |= (b << 8);
			min = 0xffff * 3;
			min_i = 0;
			for (i = 0; i < fe_n_icon_colors; i++)
			{
				dist =
					ABS(r - (int) fe_icon_colors[i][0]) +
					ABS(g - (int) fe_icon_colors[i][1]) +
					ABS(b - (int) fe_icon_colors[i][2]);
				if (dist < min)
				{
					min = dist;
					min_i = i;
				}
			}
			fe_color_entries[index].color = min_i;
		}
		break;
	case 's':
		if (!strcmp(entry->spec, "mask"))
		{
			fe_color_entries[index].mask = 1;
		}
		break;
	}
}


void
fe_get_external_icon(Display *dpy, char **name, int *width, int *height,
	unsigned char **mono_data, unsigned char **color_data,
	unsigned char **mask_data)
{
	int		chars_per_pixel;
	unsigned char	*color;
	xpm_color_entry	entry[3];
	FILE		*f;
	char		file[512];
	int		h;
	int		i;
	int		j;
	int		k;
	unsigned char	*mask;
	int		mlen;
	unsigned char	*mono;
	int		ncolors;
	unsigned char	*p;
	char		*s;
	int		w;

	f = NULL;

	mono = NULL;
	color = NULL;
	mask = NULL;

        if ( **name != '/' ) PR_snprintf(file, sizeof (file),
		    "%s/%s.xpm", fe_get_app_dir(dpy), *name);
	f = fopen(file, "r");
	if (!f)
	{
		goto BAD_BAD_ICON;
	}

	s = fe_get_xpm_string(f, -1);
	if (!s)
	{
		goto BAD_BAD_ICON;
	}

	w = h = ncolors = chars_per_pixel = -1;
	sscanf(s, "%d %d %d %d", &w, &h, &ncolors, &chars_per_pixel);
	if ((w < 0) || (h < 0) || (ncolors < 0) || (chars_per_pixel < 0))
	{
		goto BAD_BAD_ICON;
	}

	mlen = ((w + 7) / 8) * h;

	mono = malloc(mlen);
	color = malloc(w * h);
	mask = malloc(mlen);
	if ((!mono) || (!color) || (!mask))
	{
		goto BAD_BAD_ICON;
	}

	for (i = 0; i < 128; i++)
	{
		fe_color_entries[i].mask = 0;
		fe_color_entries[i].mono = 0;
		fe_color_entries[i].color = 0;
	}

	for (i = 0; i < ncolors; i++)
	{
		s = fe_get_xpm_string(f, -1);
		if ((!s) || (!(*s)) || (s[1] != ' '))
		{
			goto BAD_BAD_ICON;
		}
		entry[0].type[0] = 0;
		entry[0].spec[0] = 0;
		entry[1].type[0] = 0;
		entry[1].spec[0] = 0;
		entry[2].type[0] = 0;
		entry[2].spec[0] = 0;
		sscanf(s + 2, "%s %s %s %s %s %s",
			entry[0].type, entry[0].spec,
			entry[1].type, entry[1].spec,
			entry[2].type, entry[2].spec);
		fe_process_color(s[0], &entry[0]);
		fe_process_color(s[0], &entry[1]);
		fe_process_color(s[0], &entry[2]);
	}

	for (i = 0; i < mlen; i++)
	{
		mask[i] = 0;
		mono[i] = 0;
	}

	j = 0;
	k = 0;
	for (i = 0; i < h; i++)
	{
		s = fe_get_xpm_string(f, w + 1);
		if (!s)
		{
			goto BAD_BAD_ICON;
		}
		p = (unsigned char *) s;
		while (*p)
		{
			color[j] = fe_color_entries[*p].color;
			if (!fe_color_entries[*p].mask)
			{
				mask[k / 8] |= (1 << (k % 8));
			}
			if (fe_color_entries[*p].mono)
			{
				mono[k / 8] |= (1 << (k % 8));
			}
			p++;
			j++;
			k++;
		}
		k = ((k + 7) / 8) * 8;
	}

	*width = w;
	*height = h;
	*mono_data = mono;
	*color_data = color;
	*mask_data = mask;

	fclose(f);

	return;

BAD_BAD_ICON:
	if (f)
	{
		fclose(f);
	}

	if (mono)
	{
		free(mono);
	}
	if (color)
	{
		free(color);
	}
	if (mask)
	{
		free(mask);
	}

	*name = NULL;
}


void
fe_MakeIcon(MWContext *context, Pixel transparent_color, fe_icon* result,
	    char *name,
	    int width, int height,
	    unsigned char *mono_data,
	    unsigned char *color_data,
	    unsigned char *mask_data,
	    Boolean hack_mask_and_cmap_p)
{
  Widget widget = CONTEXT_WIDGET (context);
  Display *dpy = XtDisplay (widget);
  Screen *screen;
  Window window;
  Visual *v = 0;
  Colormap cmap = 0;
  Cardinal visual_depth = 0;
  Cardinal pixmap_depth = 0;
  unsigned char *data;
  Boolean free_data = False;
  XImage *ximage;
  Pixmap pixmap = 0;
  Pixmap mask_pixmap = 0;
  XGCValues gcv;
  GC gc;
  int i;

  if (result->pixmap) return;	/* Already done. */

  if (name) fe_get_external_icon(dpy, &name, &width, &height, &mono_data,
	&color_data, &mask_data);

  XtVaGetValues (widget, XtNvisual, &v, XtNcolormap, &cmap,
		 XtNscreen, &screen, XtNdepth, &visual_depth, 0);

  if (hack_mask_and_cmap_p)
    {
      v = DefaultVisualOfScreen (screen);
      cmap = DefaultColormapOfScreen (screen);
      visual_depth = fe_VisualDepth (dpy, v);
    }

  window = RootWindowOfScreen (screen);
  pixmap_depth = fe_VisualPixmapDepth (dpy, v);

  if (pixmap_depth == 1 || fe_globalData.force_mono_p)
    {
  MONO:
      data = mono_data;
    }
  else
    {
      /* Remap the numbers in the data to match the colors we allocated.
	 We need to copy it since the string might not be writable.
	 Also, the data is 8 deep - we might need to deepen it if we're
	 on a deeper visual.
       */
      unsigned char  *data8  = 0;
      unsigned short *data16 = 0;
      unsigned int   *data32 = 0;
      unsigned long  *data64 = 0;

      if (pixmap_depth == 8)
	{
	  data8 = (unsigned char *) malloc (width * height);
	  data  = (unsigned char *) data8;
	}
      else if (pixmap_depth == 16)
	{
	  data16 = (unsigned short *) malloc (width * height * 2);
	  data   = (unsigned char *) data16;
	}
      else if (pixmap_depth == 32)
	{
	  data32 = (unsigned int *) malloc (width * height * 4);
	  data   = (unsigned char *) data32;
	}
      else if (pixmap_depth == 64)
	{
	  data64 = (unsigned long *) malloc (width * height * 8);
	  data   = (unsigned char *) data64;
	}
      else
	{
	  /* Oh great, a goofy depth.  What the hell. */
	  goto MONO;
	}

      free_data = True;

      if (!hack_mask_and_cmap_p)
	{
	  if (pixmap_depth == 8)
	    for (i = 0; i < (width * height); i++)
	      data8  [i] = fe_icon_pixels [color_data [i]];
	  else if (pixmap_depth == 16)
	    for (i = 0; i < (width * height); i++)
	      data16 [i] = fe_icon_pixels [color_data [i]];
	  else if (pixmap_depth == 32)
	    for (i = 0; i < (width * height); i++)
	      data32 [i] = fe_icon_pixels [color_data [i]];
	  else if (pixmap_depth == 64)
	    for (i = 0; i < (width * height); i++)
	      data64 [i] = fe_icon_pixels [color_data [i]];
	  else
	    abort ();
	}
      else
	{
	  /* The hack_mask_and_cmap_p flag means that these colors need to come
	     out of the default colormap, not the window's colormap, since this
	     is an icon for the desktop.  So, go through the image, find the
	     colors that are in it, and duplicate them.
	   */
	  char color_duped [255];
	  Pixel new_pixels [255];
	  memset (color_duped, 0, sizeof (color_duped));
	  memset (new_pixels, ~0, sizeof (new_pixels));
	  for (i = 0; i < (width * height); i++)
	    {
	      if (!color_duped [color_data [i]])
		{
		  XColor color;
                  fe_colormap *colormap = fe_globalData.default_colormap;
		  color.red   = fe_icon_colors [color_data [i]][0];
		  color.green = fe_icon_colors [color_data [i]][1];
		  color.blue  = fe_icon_colors [color_data [i]][2];
                  if (!fe_AllocColor (colormap, &color))
                      fe_AllocClosestColor (colormap, &color);
		  new_pixels [color_data [i]] = color.pixel;
		  color_duped [color_data [i]] = 1;
		}
	      if (pixmap_depth == 8)
		data8  [i] = new_pixels [color_data [i]];
	      else if (pixmap_depth == 16)
		data16 [i] = new_pixels [color_data [i]];
	      else if (pixmap_depth == 32)
		data32 [i] = new_pixels [color_data [i]];
	      else if (pixmap_depth == 64)
		data64 [i] = new_pixels [color_data [i]];
	      else
		abort ();
	    }
	}
    }

  ximage = XCreateImage (dpy, v,
			 (data == mono_data ? 1 : visual_depth),
			 (data == mono_data ? XYPixmap : ZPixmap),
			 0,				   /* offset */
			 (char *) data, width, height,
			 8,				   /* bitmap_pad */
			 0);
  if (data == mono_data)
    {
      /* This ordering is implicit in the data in icondata.h, which is
	 the same implicit ordering as in all XBM files.  I think. */
      ximage->bitmap_bit_order = LSBFirst;
      ximage->byte_order = LSBFirst;
    }
  else
    {
#if defined(IS_LITTLE_ENDIAN)
      ximage->byte_order = LSBFirst;
#elif defined (IS_BIG_ENDIAN)
      ximage->byte_order = MSBFirst;
#else
  ERROR! Endianness is unknown.
#endif
    }

  if (data == mono_data && visual_depth != 1 && !hack_mask_and_cmap_p)
    {
      /* If we're in mono-mode, and the screen is not of depth 1,
	 deepen the pixmap. */
      Pixmap shallow;
      shallow = XCreatePixmap (dpy, window, width, height, 1);
      pixmap  = XCreatePixmap (dpy, window, width, height, pixmap_depth);
      gcv.function = GXcopy;
      gcv.background = 0;
      gcv.foreground = 1;
      gc = XCreateGC (dpy, shallow, GCFunction|GCForeground|GCBackground,
		      &gcv);
      XPutImage (dpy, shallow, gc, ximage, 0, 0, 0, 0, width, height);
      XFreeGC (dpy, gc);

      gcv.function = GXcopy;
      gcv.background = transparent_color;
      gcv.foreground = CONTEXT_DATA (context)->fg_pixel;
      gc = XCreateGC (dpy, pixmap, GCFunction|GCForeground|GCBackground,
		      &gcv);
      XCopyPlane (dpy, shallow, pixmap, gc, 0, 0, width, height, 0, 0, 1L);
      XFreePixmap (dpy, shallow);
      XFreeGC (dpy, gc);
      /* No need for a mask in this case - the coloring is done. */
      mask_data = 0;
    }
  else
    {
      /* Both the screen and pixmap are of the same depth.
       */
      pixmap = XCreatePixmap (dpy, window, width, height,
			      (data == mono_data ? 1 : visual_depth));

      if (visual_depth == 1 && WhitePixelOfScreen (screen) == 1)
	/* A server with backwards WhitePixel, like NCD... */
	gcv.function = GXcopyInverted;
      else
	gcv.function = GXcopy;

      gcv.background = transparent_color;
      gcv.foreground = CONTEXT_DATA (context)->fg_pixel;
      gc = XCreateGC (dpy, pixmap, GCFunction|GCForeground|GCBackground, &gcv);
      XPutImage (dpy, pixmap, gc, ximage, 0, 0, 0, 0, width, height);
      XFreeGC (dpy, gc);
    }

  ximage->data = 0;
  XDestroyImage (ximage);
  if (free_data)
    free (data);

  /* Optimization: if the mask is all 1's, don't bother sending it. */
  if (mask_data)
    {
      int max = width * height / 8;
      for (i = 0; i < max; i++)
	if (mask_data [i] != 0xFF)
	  break;
      if (i == max)
	mask_data = 0;
    }

  /* Fill the "transparent" areas with the background color. */
  if (mask_data)
    {
      ximage = XCreateImage (dpy, v, 1, XYPixmap,
			     0,				   /* offset */
			     (char *) mask_data, width, height,
			     8,				   /* bitmap_pad */
			     0);
      /* This ordering is implicit in the data in icondata.h, which is
	 the same implicit ordering as in all XBM files.  I think. */
      ximage->byte_order = LSBFirst;
      ximage->bitmap_bit_order = LSBFirst;

      mask_pixmap = XCreatePixmap (dpy, window, width, height, 1);

      gcv.function = GXcopy;
      gc = XCreateGC (dpy, mask_pixmap, GCFunction, &gcv);
      XPutImage (dpy, mask_pixmap, gc, ximage, 0, 0, 0, 0, width, height);
      XFreeGC (dpy, gc);
      ximage->data = 0;
      XDestroyImage (ximage);

      if (! hack_mask_and_cmap_p)
	{
	  /* Create a pixmap of the mask, inverted. */
	  Pixmap inverted_mask_pixmap =
	    XCreatePixmap (dpy, window, width, height, 1);
	  gcv.function = GXcopyInverted;
	  gc = XCreateGC (dpy, inverted_mask_pixmap, GCFunction, &gcv);
	  XCopyArea (dpy, mask_pixmap, inverted_mask_pixmap, gc,
		     0, 0, width, height, 0, 0);
	  XFreeGC (dpy, gc);

	  /* Fill the background color through that inverted mask. */
	  gcv.function = GXcopy;
	  gcv.foreground = transparent_color;
	  gcv.clip_mask = inverted_mask_pixmap;
	  gc = XCreateGC (dpy, pixmap, GCFunction|GCForeground|GCClipMask,
			  &gcv);
	  XFillRectangle (dpy, pixmap, gc, 0, 0, width, height);
	  XFreeGC (dpy, gc);

	  XFreePixmap (dpy, inverted_mask_pixmap);
	}
    }

  result->pixmap = pixmap;
  result->mask = mask_pixmap;
  result->width = width;
  result->height = height;

  if (name)
    {
      free(mono_data);
      free(color_data);
      free(mask_data);
    }
}


static void
fe_make_icon_1 (MWContext *context, Pixel transparent_color, int id,
		char *name,
		int width, int height,
		unsigned char *mono_data,
		unsigned char *color_data,
		unsigned char *mask_data,
		Boolean hack_mask_and_cmap_p)
{
  fe_MakeIcon(context, transparent_color, fe_icons + id, name, width, height,
	      mono_data, color_data, mask_data, hack_mask_and_cmap_p);
}


static void
fe_make_icon (MWContext *context, Pixel transparent_color, int id,
	      char *name,
	      int width, int height,
	      unsigned char *mono_data,
	      unsigned char *color_data,
	      unsigned char *mask_data)
{
  fe_make_icon_1 (context, transparent_color, id, name, width, height,
		  mono_data, color_data, mask_data, False);
}


static void
fe_init_document_icons (MWContext *c)
{
  Pixel bg, bg2, white;
  static Bool done = False;
  Boolean save;

  if (done) return;
  done = True;

  bg = CONTEXT_DATA (c)->default_bg_pixel;
  white = WhitePixelOfScreen (XtScreen (CONTEXT_WIDGET (c)));

  fe_make_icon (c, bg, IL_IMAGE_DELAYED,
		NULL,
		IReplace.width, IReplace.height,
		IReplace.mono_bits, IReplace.color_bits, IReplace.mask_bits);
  fe_make_icon (c, bg, IL_IMAGE_NOT_FOUND,
		NULL,
		IconUnknown.width, IconUnknown.height,
		IconUnknown.mono_bits, IconUnknown.color_bits, IconUnknown.mask_bits);
  fe_make_icon (c, bg, IL_IMAGE_BAD_DATA,
		NULL,
		IBad.width, IBad.height,
		IBad.mono_bits, IBad.color_bits, IBad.mask_bits);
  fe_make_icon (c, bg, IL_IMAGE_EMBED,    /* #### Need an XPM for this one */
		NULL,
		IconUnknown.width, IconUnknown.height,
		IconUnknown.mono_bits, IconUnknown.color_bits, IconUnknown.mask_bits);
#ifdef HAVE_SECURITY
  fe_make_icon (c, bg, IL_IMAGE_INSECURE,
		NULL,
		Sreplace.width, Sreplace.height,
		Sreplace.mono_bits, Sreplace.color_bits, Sreplace.mask_bits);

  bg2 = bg;
  if (CONTEXT_DATA(c)->dashboard) {
   /* XXX need to find why we do this - dp */
    XtVaGetValues (CONTEXT_DATA (c)->dashboard, XmNbackground, &bg2, 0);
  }
  fe_make_icon (c, bg2, IL_ICON_SECURITY_ON,
		NULL,
		Ssecure.width, Ssecure.height,
		Ssecure.mono_bits, Ssecure.color_bits, Ssecure.mask_bits);
  fe_make_icon (c, bg2, IL_ICON_SECURITY_OFF,
		NULL,
		Sinsecure.width, Sinsecure.height,
		Sinsecure.mono_bits, Sinsecure.color_bits,
		Sinsecure.mask_bits);
  fe_make_icon (c, bg2, IL_ICON_SECURITY_HIGH,
		NULL,
		Shsecure.width, Shsecure.height,
		Shsecure.mono_bits, Shsecure.color_bits, Shsecure.mask_bits);
  fe_make_icon (c, bg2, IL_ICON_SECURITY_MIXED,
		NULL,
		Smix.width, Smix.height,
		Smix.mono_bits, Smix.color_bits, Smix.mask_bits);
#endif
  /* Load all the desktop icons */
  save = fe_globalData.force_mono_p; /* hack. hack, hack */
  if (strcasecmp(fe_globalData.wm_icon_policy, "mono") == 0)
      fe_globalData.force_mono_p = TRUE;

#ifdef GOLD
  fe_make_icon_1 (c, white, IL_ICON_DESKTOP_NAVIGATOR, /* Navigator Gold */
		  NULL,
		  gold_app.width, gold_app.height,
		  gold_app.mono_bits, gold_app.color_bits, gold_app.mask_bits, True);
#else
  fe_make_icon_1 (c, white, IL_ICON_DESKTOP_NAVIGATOR,	/* Navigator */
		  NULL,
		  app.width, app.height,
		  app.mono_bits, app.color_bits, app.mask_bits, True);
#endif
  fe_make_icon_1 (c, white, IL_ICON_DESKTOP_ABOOK,	/* AddressBook */
		  NULL,
		  DABook.width, DABook.height,
		  DABook.mono_bits, DABook.color_bits, DABook.mask_bits, True);
  fe_make_icon_1 (c, white, IL_ICON_DESKTOP_BOOKMARK,	/* Bookmark */
		  NULL,
		  DBookmark.width, DBookmark.height,
		  DBookmark.mono_bits, DBookmark.color_bits,
		  DBookmark.mask_bits, True);
  fe_make_icon_1 (c, white, IL_ICON_DESKTOP_NOMAIL,	/* No new mail */
		  NULL,
		  DNoNewMail.width, DNoNewMail.height,
		  DNoNewMail.mono_bits, DNoNewMail.color_bits,
		  DNoNewMail.mask_bits, True);
  fe_make_icon_1 (c, white, IL_ICON_DESKTOP_YESMAIL,	/* New mail */
		  NULL,
		  DNewMail.width, DNewMail.height,
		  DNewMail.mono_bits, DNewMail.color_bits,
		  DNewMail.mask_bits, True);
  fe_make_icon_1 (c, white, IL_ICON_DESKTOP_NEWS,	/* News */
		  NULL,
		  DNews.width, DNews.height,
		  DNews.mono_bits, DNews.color_bits, DNews.mask_bits, True);
  fe_make_icon_1 (c, white, IL_ICON_DESKTOP_COMPOSE,	/* Compose */
		  NULL,
		  DCompose.width, DCompose.height,
		  DCompose.mono_bits, DCompose.color_bits,
		  DCompose.mask_bits, True);
#ifdef EDITOR
  fe_make_icon_1 (c, white, IL_ICON_DESKTOP_EDITOR,	/* Editor */
		  NULL,
		  DEditor.width, DEditor.height,
		  DEditor.mono_bits, DEditor.color_bits,
		  DEditor.mask_bits, True);
#endif /*EDITOR*/
  fe_globalData.force_mono_p = save;
}

static void
fe_init_gopher_icons (MWContext *c)
{
  Pixel bg;
  static Bool done = False;

  if (done) return;
  done = True;

  bg = CONTEXT_DATA (c)->default_bg_pixel;

  fe_make_icon (c, bg, IL_GOPHER_TEXT,
		NULL,
		GText.width, GText.height,
		GText.mono_bits, GText.color_bits, GText.mask_bits);
  fe_make_icon (c, bg, IL_GOPHER_IMAGE,
		NULL,
		GImage.width, GImage.height,
		GImage.mono_bits, GImage.color_bits, GImage.mask_bits);
  fe_make_icon (c, bg, IL_GOPHER_BINARY,
		NULL,
		GBinary.width, GBinary.height,
		GBinary.mono_bits, GBinary.color_bits, GBinary.mask_bits);
  fe_make_icon (c, bg, IL_GOPHER_SOUND,
		NULL,
		GAudio.width, GAudio.height,
		GAudio.mono_bits, GAudio.color_bits, GAudio.mask_bits);
  fe_make_icon (c, bg, IL_GOPHER_MOVIE,
		NULL,
		GMovie.width, GMovie.height,
		GMovie.mono_bits, GMovie.color_bits, GMovie.mask_bits);
  fe_make_icon (c, bg, IL_GOPHER_FOLDER,
		NULL,
		GFolder.width, GFolder.height,
		GFolder.mono_bits, GFolder.color_bits, GFolder.mask_bits);
  fe_make_icon (c, bg, IL_GOPHER_SEARCHABLE,
		NULL,
		GFind.width, GFind.height,
		GFind.mono_bits, GFind.color_bits, GFind.mask_bits);
  fe_make_icon (c, bg, IL_GOPHER_TELNET,
		NULL,
		GTelnet.width, GTelnet.height,
		GTelnet.mono_bits, GTelnet.color_bits, GTelnet.mask_bits);
  fe_make_icon (c, bg, IL_GOPHER_UNKNOWN,
		NULL,
		GUnknown.width, GUnknown.height,
		GUnknown.mono_bits, GUnknown.color_bits, GUnknown.mask_bits);
}


static void
fe_init_toolbar_icons_1 (MWContext *c)
{
  Pixel  bg2 = 0;
  static Bool done = False;

  if (done)
    return;

  XtVaGetValues (CONTEXT_DATA (c)->top_area, XmNbackground, &bg2, 0);

  done = True;
  fe_make_icon (c, bg2, IL_ICON_BACK,
		NULL,
		TBack.width, TBack.height,
		TBack.mono_bits, TBack.color_bits, TBack.mask_bits);
  fe_make_icon (c, bg2, IL_ICON_FWD,
		NULL,
		TNext.width, TNext.height,
		TNext.mono_bits, TNext.color_bits, TNext.mask_bits);
  fe_make_icon (c, bg2, IL_ICON_HOME,
		NULL,
		THome.width, THome.height,
		THome.mono_bits, THome.color_bits, THome.mask_bits);
#ifdef EDITOR
  fe_make_icon (c, bg2, IL_EDITOR_EDIT,
		"ed_edit",
		ed_edit.width, ed_edit.height,
		ed_edit.mono_bits, ed_edit.color_bits, ed_edit.mask_bits);
  fe_make_icon (c, bg2, IL_EDITOR_EDIT_GREY,
		"ed_edit.i", ed_edit_i.width, ed_edit_i.height,
		ed_edit_i.mono_bits, ed_edit_i.color_bits, ed_edit_i.mask_bits);
#endif /*EDITOR*/
  fe_make_icon (c, bg2, IL_ICON_RELOAD,
		NULL,
		TReload.width, TReload.height,
		TReload.mono_bits, TReload.color_bits, TReload.mask_bits);
  fe_make_icon (c, bg2, IL_ICON_LOAD,
		NULL,
		TLoadImages.width, TLoadImages.height,
		TLoadImages.mono_bits, TLoadImages.color_bits,
		TLoadImages.mask_bits);
  fe_make_icon (c, bg2, IL_ICON_OPEN,
		NULL,
		TOpenUrl.width, TOpenUrl.height,
		TOpenUrl.mono_bits, TOpenUrl.color_bits, TOpenUrl.mask_bits);
  fe_make_icon (c, bg2, IL_ICON_PRINT,
		NULL,
		TPrint.width, TPrint.height,
		TPrint.mono_bits, TPrint.color_bits, TPrint.mask_bits);
  fe_make_icon (c, bg2, IL_ICON_FIND,
		NULL,
		TFind.width, TFind.height,
		TFind.mono_bits, TFind.color_bits, TFind.mask_bits);
  fe_make_icon (c, bg2, IL_ICON_STOP,
		NULL,
		TStop.width, TStop.height,
		TStop.mono_bits, TStop.color_bits, TStop.mask_bits);
  fe_make_icon (c, bg2, IL_ICON_NETSCAPE,
		NULL,
		TLogo.width, TLogo.height,
		TLogo.mono_bits, TLogo.color_bits, TLogo.mask_bits);

  fe_make_icon (c, bg2, IL_ICON_BACK_GREY,
		NULL,
		TBack_i.width, TBack_i.height,
		TBack_i.mono_bits, TBack_i.color_bits, TBack_i.mask_bits);
  fe_make_icon (c, bg2, IL_ICON_HOME_GREY,
		NULL,
		THome_i.width, THome_i.height,
		THome_i.mono_bits, THome_i.color_bits, THome_i.mask_bits);
  fe_make_icon (c, bg2, IL_ICON_LOAD_GREY,
		NULL,
		TLoadImages_i.width, TLoadImages_i.height,
		TLoadImages_i.mono_bits, TLoadImages_i.color_bits,
		TLoadImages_i.mask_bits);
  fe_make_icon (c, bg2, IL_ICON_FWD_GREY,
		NULL,
		TNext_i.width, TNext_i.height,
		TNext_i.mono_bits, TNext_i.color_bits, TNext_i.mask_bits);
  fe_make_icon (c, bg2, IL_ICON_STOP_GREY,
		NULL,
		TStop_i.width, TStop_i.height,
		TStop_i.mono_bits, TStop_i.color_bits, TStop_i.mask_bits);
}


static void
fe_init_toolbar_icons_2 (MWContext *c)
{
  Pixel bg2 = 0;
  static Bool done = False;

  if (done)
    return;

  XtVaGetValues (CONTEXT_DATA (c)->top_area, XmNbackground, &bg2, 0);

  done = True;

  fe_make_icon (c, bg2, IL_ICON_BACK_PT,
		"TBack.pt",
		TBack_pt.width, TBack_pt.height,
		TBack_pt.mono_bits, TBack_pt.color_bits, TBack_pt.mask_bits);
  fe_make_icon (c, bg2, IL_ICON_FWD_PT,
		"TNext.pt",
		TNext_pt.width, TNext_pt.height,
		TNext_pt.mono_bits, TNext_pt.color_bits, TNext_pt.mask_bits);
  fe_make_icon (c, bg2, IL_ICON_HOME_PT,
		"THome.pt",
		THome_pt.width, THome_pt.height,
		THome_pt.mono_bits, THome_pt.color_bits, THome_pt.mask_bits);
#ifdef EDITOR
  fe_make_icon (c, bg2, IL_EDITOR_EDIT_PT,
		"ed_edit.pt",
		ed_edit_pt.width, ed_edit_pt.height,
		ed_edit_pt.mono_bits, ed_edit_pt.color_bits, ed_edit_pt.mask_bits);
  fe_make_icon (c, bg2, IL_EDITOR_EDIT_PT_GREY,
		"ed_edit.i.pt",
		ed_edit_pt_i.width, ed_edit_pt_i.height,
		ed_edit_pt_i.mono_bits, ed_edit_pt_i.color_bits, ed_edit_pt_i.mask_bits);
#endif /*EDITOR*/
  fe_make_icon (c, bg2, IL_ICON_RELOAD_PT,
		"TReload.pt",
		TReload_pt.width, TReload_pt.height,
		TReload_pt.mono_bits, TReload_pt.color_bits,
		TReload_pt.mask_bits);
  fe_make_icon (c, bg2, IL_ICON_LOAD_PT,
		"TLoadImages.pt",
		TLoadImages_pt.width, TLoadImages_pt.height,
		TLoadImages_pt.mono_bits, TLoadImages_pt.color_bits,
		TLoadImages_pt.mask_bits);
  fe_make_icon (c, bg2, IL_ICON_OPEN_PT,
		"TOpenUrl.pt",
		TOpenUrl_pt.width, TOpenUrl_pt.height,
		TOpenUrl_pt.mono_bits, TOpenUrl_pt.color_bits,
		TOpenUrl_pt.mask_bits);
  fe_make_icon (c, bg2, IL_ICON_PRINT_PT,
		"TPrint.pt",
		TPrint_pt.width, TPrint_pt.height,
		TPrint_pt.mono_bits, TPrint_pt.color_bits,
		TPrint_pt.mask_bits);
#if 0 /* no exists at the moment but maybe later 5MAR96RCJ */
  fe_make_icon (c, bg2, IL_ICON_PRINT_PT_GREY,
		"TPrint.pt.i",
		TPrint_pt_i.width, TPrint_p_i.height,
		TPrint_pt_i.mono_bits, TPrint_pt_i.color_bits,
		TPrint_pt_i.mask_bits);
#endif
  fe_make_icon (c, bg2, IL_ICON_FIND_PT,
		"TFind.pt",
		TFind_pt.width, TFind_pt.height,
		TFind_pt.mono_bits, TFind_pt.color_bits, TFind_pt.mask_bits);
  fe_make_icon (c, bg2, IL_ICON_STOP_PT,
		"TStop.pt",
		TStop_pt.width, TStop_pt.height,
		TStop_pt.mono_bits, TStop_pt.color_bits, TStop_pt.mask_bits);
  fe_make_icon (c, bg2, IL_ICON_NETSCAPE_PT,
		"TLogo.pt",
		TLogo_pt.width, TLogo_pt.height,
		TLogo_pt.mono_bits, TLogo_pt.color_bits, TLogo_pt.mask_bits);

  fe_make_icon (c, bg2, IL_ICON_BACK_PT_GREY,
		"TBack.pt.i",
		TBack_pt_i.width, TBack_pt_i.height,
		TBack_pt_i.mono_bits, TBack_pt_i.color_bits,
		TBack_pt_i.mask_bits);
  fe_make_icon (c, bg2, IL_ICON_HOME_PT_GREY,
		"THome.pt.i",
		THome_pt_i.width, THome_pt_i.height,
		THome_pt_i.mono_bits, THome_pt_i.color_bits,
		THome_pt_i.mask_bits);
  fe_make_icon (c, bg2, IL_ICON_LOAD_PT_GREY,	
		"TLoadImages.pt.i",
		TLoadImages_pt_i.width, TLoadImages_pt_i.height,
		TLoadImages_pt_i.mono_bits, TLoadImages_pt_i.color_bits,
		TLoadImages_pt_i.mask_bits);
  fe_make_icon (c, bg2, IL_ICON_FWD_PT_GREY,
		"TNext.pt.i",
		TNext_pt_i.width, TNext_pt_i.height,
		TNext_pt_i.mono_bits, TNext_pt_i.color_bits,
		TNext_pt_i.mask_bits);
  fe_make_icon (c, bg2, IL_ICON_STOP_PT_GREY,
		"TStop.pt.i",
		TStop_pt_i.width, TStop_pt_i.height,
		TStop_pt_i.mono_bits, TStop_pt_i.color_bits,
		TStop_pt_i.mask_bits);
}

#ifdef EDITOR

static void
fe_init_editor_icons_notext(MWContext* c)
{
  Pixel  bg2 = 0;
  static Bool done = False;

  if (done)
    return;

  XtVaGetValues (CONTEXT_DATA (c)->top_area, XmNbackground, &bg2, 0);

  fe_make_icon (c, bg2, IL_EDITOR_NEW,		/* 23FEB96RCJ */
		"ed_new",
		ed_new.width, ed_new.height,
		ed_new.mono_bits, ed_new.color_bits, ed_new.mask_bits);
  fe_make_icon (c, bg2, IL_EDITOR_NEW_GREY,		/* 23FEB96RCJ */
		"ed_new.i",
		ed_new_i.width, ed_new_i.height,
		ed_new_i.mono_bits, ed_new_i.color_bits, ed_new_i.mask_bits);
  fe_make_icon (c, bg2, IL_EDITOR_OPEN,
		"ed_open",
		ed_open.width, ed_open.height,
		ed_open.mono_bits, ed_open.color_bits, ed_open.mask_bits);
  fe_make_icon (c, bg2, IL_EDITOR_OPEN_GREY,
		"ed_open.i",
		ed_open_i.width, ed_open_i.height,
		ed_open_i.mono_bits, ed_open_i.color_bits, ed_open_i.mask_bits);
  fe_make_icon (c, bg2, IL_EDITOR_SAVE,	
		"ed_save",
		ed_save.width, ed_save.height,
		ed_save.mono_bits, ed_save.color_bits, ed_save.mask_bits);
  fe_make_icon (c, bg2, IL_EDITOR_SAVE_GREY,
		"ed_save.i",
		ed_save_i.width, ed_save_i.height,
		ed_save_i.mono_bits, ed_save_i.color_bits, ed_save_i.mask_bits);
  fe_make_icon (c, bg2, IL_EDITOR_BROWSE,	
		"ed_browse",
		ed_browse.width, ed_browse.height,
		ed_browse.mono_bits, ed_browse.color_bits, ed_browse.mask_bits);
  fe_make_icon (c, bg2, IL_EDITOR_BROWSE_GREY,
		"ed_browse.i",
		ed_browse_i.width, ed_browse_i.height,
		ed_browse_i.mono_bits, ed_browse_i.color_bits, ed_browse_i.mask_bits);
  fe_make_icon (c, bg2, IL_EDITOR_CUT,
		"ed_cut",
		ed_cut.width, ed_cut.height,
		ed_cut.mono_bits, ed_cut.color_bits, ed_cut.mask_bits);
  fe_make_icon (c, bg2, IL_EDITOR_CUT_GREY,
		"ed_cut.i",
		ed_cut_i.width, ed_cut_i.height,
		ed_cut_i.mono_bits, ed_cut_i.color_bits, ed_cut_i.mask_bits);
  fe_make_icon (c, bg2, IL_EDITOR_COPY,
		"ed_copy",
		ed_copy.width, ed_copy.height,
		ed_copy.mono_bits, ed_copy.color_bits, ed_copy.mask_bits);
  fe_make_icon (c, bg2, IL_EDITOR_COPY_GREY,
		"ed_copy.i",
		ed_copy_i.width, ed_copy_i.height,
		ed_copy_i.mono_bits, ed_copy_i.color_bits, ed_copy_i.mask_bits);
  fe_make_icon (c, bg2, IL_EDITOR_PASTE,
		"ed_paste",
		ed_paste.width, ed_paste.height,
		ed_paste.mono_bits, ed_paste.color_bits, ed_paste.mask_bits);
  fe_make_icon (c, bg2, IL_EDITOR_PASTE_GREY,
		"ed_paste.i",
		ed_paste_i.width, ed_paste_i.height,
		ed_paste_i.mono_bits, ed_paste_i.color_bits, ed_paste_i.mask_bits);
  fe_make_icon (c, bg2, IL_EDITOR_PRINT,
		"ed_print",
		ed_print.width, ed_print.height,
		ed_print.mono_bits, ed_print.color_bits, ed_print_i.mask_bits);
  fe_make_icon (c, bg2, IL_EDITOR_PRINT_GREY,
		"ed_print.i",
		ed_print_i.width, ed_print_i.height,
		ed_print_i.mono_bits, ed_print_i.color_bits, ed_print_i.mask_bits);
  fe_make_icon (c, bg2, IL_EDITOR_FIND,
		"ed_find",
		ed_find.width, ed_find.height,
		ed_find.mono_bits, ed_find.color_bits, ed_find.mask_bits);
  fe_make_icon (c, bg2, IL_EDITOR_FIND_GREY,
		"ed_find.i",
		ed_find_i.width, ed_find_i.height,
		ed_find_i.mono_bits, ed_find_i.color_bits, ed_find_i.mask_bits);
  fe_make_icon (c, bg2, IL_EDITOR_PUBLISH,
		"ed_publish",
		ed_publish.width, ed_publish.height, 
		ed_publish.mono_bits, ed_publish.color_bits, ed_publish.mask_bits); 
  fe_make_icon (c, bg2, IL_EDITOR_PUBLISH_GREY,
		"ed_publish.i",
		ed_publish_i.width, ed_publish_i.height, 
		ed_publish_i.mono_bits, ed_publish_i.color_bits, ed_publish_i.mask_bits); 
  
  fe_make_icon (c, bg2, IL_ICON_BACK_GREY,
		NULL,
		TBack_i.width, TBack_i.height,
		TBack_i.mono_bits, TBack_i.color_bits, TBack_i.mask_bits);
  fe_make_icon (c, bg2, IL_ICON_LOAD_GREY,
		NULL,
		TLoadImages_i.width, TLoadImages_i.height,
		TLoadImages_i.mono_bits, TLoadImages_i.color_bits,
		TLoadImages_i.mask_bits);
  fe_make_icon (c, bg2, IL_ICON_FWD_GREY,
		NULL,
		TNext_i.width, TNext_i.height,
		TNext_i.mono_bits, TNext_i.color_bits, TNext_i.mask_bits);
  fe_make_icon (c, bg2, IL_ICON_STOP_GREY,
		NULL,
		TStop_i.width, TStop_i.height,
		TStop_i.mono_bits, TStop_i.color_bits, TStop_i.mask_bits);
  done = True;
}

static void fe_init_align_icons(MWContext* c) /* added 14MAR96RCJ */
{
  Pixel  bg2 = 0;

  XtVaGetValues (CONTEXT_DATA (c)->top_area, XmNbackground, &bg2, 0);

  fe_make_icon (c, bg2, IL_ALIGN4_RAISED,
		"ImgB2B_r",
		ImgB2B_r.width, ImgB2B_r.height,
		ImgB2B_r.mono_bits, ImgB2B_r.color_bits, ImgB2B_r.mask_bits);

  fe_make_icon (c, bg2, IL_ALIGN5_RAISED,
		"ImgB2D_r",
		ImgB2D_r.width, ImgB2D_r.height,
		ImgB2D_r.mono_bits, ImgB2D_r.color_bits, ImgB2D_r.mask_bits);

  fe_make_icon (c, bg2, IL_ALIGN3_RAISED,
		"ImgC2B_r",
		ImgC2B_r.width, ImgC2B_r.height,
		ImgC2B_r.mono_bits, ImgC2B_r.color_bits, ImgC2B_r.mask_bits);

  fe_make_icon (c, bg2, IL_ALIGN2_RAISED,
		"ImgC2C_r",
		ImgC2C_r.width, ImgC2C_r.height,
		ImgC2C_r.mono_bits, ImgC2C_r.color_bits, ImgC2C_r.mask_bits);

  fe_make_icon (c, bg2, IL_ALIGN7_RAISED,
		"ImgWL_r",
		ImgWL_r.width, ImgWL_r.height,
		ImgWL_r.mono_bits, ImgWL_r.color_bits, ImgWL_r.mask_bits);

  fe_make_icon (c, bg2, IL_ALIGN6_RAISED,
		"ImgWR_r",
		ImgWR_r.width, ImgWR_r.height,
		ImgWR_r.mono_bits, ImgWR_r.color_bits, ImgWR_r.mask_bits);

  fe_make_icon (c, bg2, IL_ALIGN1_RAISED,
		"ImgT2T_r",
		ImgT2T_r.width, ImgT2T_r.height,
		ImgT2T_r.mono_bits, ImgT2T_r.color_bits, ImgT2T_r.mask_bits);

} /* end fe_init_align_icons 14MAR96RCJ */

static void
fe_init_editor_icons_withtext(MWContext* c)
{
  Pixel  bg2 = 0;
  static Bool done = False;

  if (done)
    return;

  XtVaGetValues (CONTEXT_DATA (c)->top_area, XmNbackground, &bg2, 0);

  fe_make_icon (c, bg2, IL_EDITOR_NEW_PT,
		"ed_new.pt",
		ed_new_pt.width, ed_new_pt.height,
		ed_new_pt.mono_bits, ed_new_pt.color_bits, ed_new_pt.mask_bits);
  fe_make_icon (c, bg2, IL_EDITOR_OPEN_PT,
		"ed_open.pt",
		ed_open_pt.width, ed_open_pt.height,
		ed_open_pt.mono_bits, ed_open_pt.color_bits, ed_open_pt.mask_bits);
  fe_make_icon (c, bg2, IL_EDITOR_SAVE_PT,
		"ed_save.pt",
		ed_save_pt.width, ed_save_pt.height,
		ed_save_pt.mono_bits, ed_save_pt.color_bits, ed_save_pt.mask_bits);
  fe_make_icon (c, bg2, IL_EDITOR_BROWSE_PT,
		"ed_browse.pt",
		ed_browse_pt.width, ed_browse_pt.height,
		ed_browse_pt.mono_bits, ed_browse_pt.color_bits, ed_browse_pt.mask_bits);
  fe_make_icon (c, bg2, IL_EDITOR_CUT_PT,
		"ed_cut.pt",
		ed_cut_pt.width, ed_cut_pt.height,
		ed_cut_pt.mono_bits, ed_cut_pt.color_bits, ed_cut_pt.mask_bits);
  fe_make_icon (c, bg2, IL_EDITOR_COPY_PT,
		"ed_copy.pt",
		ed_copy_pt.width, ed_copy_pt.height,
		ed_copy_pt.mono_bits, ed_copy_pt.color_bits, ed_copy_pt.mask_bits);
  fe_make_icon (c, bg2, IL_EDITOR_PASTE_PT,
		"ed_paste.pt",
		ed_paste_pt.width, ed_paste_pt.height,
		ed_paste_pt.mono_bits, ed_paste_pt.color_bits, ed_paste_pt.mask_bits);
  fe_make_icon (c, bg2, IL_EDITOR_PRINT_PT,
		"ed_print.pt",
		ed_print_pt.width, ed_print_pt.height,
		ed_print_pt.mono_bits, ed_print_pt.color_bits, ed_print_pt.mask_bits);
  fe_make_icon (c, bg2, IL_EDITOR_FIND_PT,
		"ed_find.pt",
		ed_find_pt.width, ed_find_pt.height,
		ed_find_pt.mono_bits, ed_find_pt.color_bits, ed_find_pt.mask_bits);
  fe_make_icon (c, bg2, IL_EDITOR_PUBLISH_PT,
		"ed_publish.pt",
		ed_publish_pt.width, ed_publish_pt.height, 
		ed_publish_pt.mono_bits, ed_publish_pt.color_bits, ed_publish_pt.mask_bits); 

  fe_make_icon (c, bg2, IL_EDITOR_NEW_PT_GREY,
		"ed_new.pt",
		ed_new_pt_i.width, ed_new_pt_i.height,
		ed_new_pt_i.mono_bits, ed_new_pt_i.color_bits, ed_new_pt_i.mask_bits);
  fe_make_icon (c, bg2, IL_EDITOR_OPEN_PT_GREY,
		"ed_open.pt",
		ed_open_pt_i.width, ed_open_pt_i.height,
		ed_open_pt_i.mono_bits, ed_open_pt_i.color_bits, ed_open_pt_i.mask_bits);
  fe_make_icon (c, bg2, IL_EDITOR_SAVE_PT_GREY,
		"ed_save.pt",
		ed_save_pt_i.width, ed_save_pt_i.height,
		ed_save_pt_i.mono_bits, ed_save_pt_i.color_bits, ed_save_pt_i.mask_bits);
  fe_make_icon (c, bg2, IL_EDITOR_BROWSE_PT_GREY,
		"ed_browse.pt",
		ed_browse_pt_i.width, ed_browse_pt_i.height,
		ed_browse_pt_i.mono_bits, ed_browse_pt_i.color_bits, ed_browse_pt_i.mask_bits);
  fe_make_icon (c, bg2, IL_EDITOR_CUT_PT_GREY,
		"ed_cut.pt",
		ed_cut_pt_i.width, ed_cut_pt_i.height,
		ed_cut_pt_i.mono_bits, ed_cut_pt_i.color_bits, ed_cut_pt_i.mask_bits);
  fe_make_icon (c, bg2, IL_EDITOR_COPY_PT_GREY,
		"ed_copy.pt",
		ed_copy_pt_i.width, ed_copy_pt_i.height,
		ed_copy_pt_i.mono_bits, ed_copy_pt_i.color_bits, ed_copy_pt_i.mask_bits);
  fe_make_icon (c, bg2, IL_EDITOR_PASTE_PT_GREY,
		"ed_paste.pt",
		ed_paste_pt_i.width, ed_paste_pt_i.height,
		ed_paste_pt_i.mono_bits, ed_paste_pt_i.color_bits, ed_paste_pt_i.mask_bits);
  fe_make_icon (c, bg2, IL_EDITOR_PRINT_PT_GREY,
		"ed_print.pt",
		ed_print_pt_i.width, ed_print_pt_i.height,
		ed_print_pt_i.mono_bits, ed_print_pt_i.color_bits, ed_print_pt_i.mask_bits);
  fe_make_icon (c, bg2, IL_EDITOR_FIND_PT_GREY,
		"ed_find.pt",
		ed_find_pt_i.width, ed_find_pt_i.height,
		ed_find_pt_i.mono_bits, ed_find_pt_i.color_bits, ed_find_pt_i.mask_bits);
  fe_make_icon (c, bg2, IL_EDITOR_PUBLISH_PT_GREY,
		"ed_publish.pt",
		ed_publish_pt_i.width, ed_publish_pt_i.height, 
		ed_publish_pt_i.mono_bits, ed_publish_pt_i.color_bits, ed_publish_pt_i.mask_bits); 
  fe_make_icon (c, bg2, IL_ICON_BACK_GREY, 
		NULL, 
		TBack_i.width, TBack_i.height, 
		TBack_i.mono_bits, TBack_i.color_bits, TBack_i.mask_bits); 
  fe_make_icon (c, bg2, IL_ICON_LOAD_GREY, 
		NULL, 
		TLoadImages_i.width, TLoadImages_i.height,
		TLoadImages_i.mono_bits, TLoadImages_i.color_bits,
		TLoadImages_i.mask_bits);
  fe_make_icon (c, bg2, IL_ICON_FWD_GREY,
		NULL,
		TNext_i.width, TNext_i.height,
		TNext_i.mono_bits, TNext_i.color_bits, TNext_i.mask_bits);
  fe_make_icon (c, bg2, IL_ICON_STOP_GREY,
		NULL,
		TStop_i.width, TStop_i.height,
		TStop_i.mono_bits, TStop_i.color_bits, TStop_i.mask_bits);
  done = True;
}

static void
fe_init_editor_icons_other(MWContext* c)
{
  Pixel  bg2 = 0;
  static Bool done = False;

  if (done)
    return;

  XtVaGetValues (CONTEXT_DATA (c)->top_area, XmNbackground, &bg2, 0);

  done = True;
  
  fe_make_icon(c, bg2, IL_EDITOR_BOLD,
	       "ed_bold",
	       ed_bold.width, ed_bold.height,
	       ed_bold.mono_bits, ed_bold.color_bits, ed_bold.mask_bits);
  fe_make_icon(c, bg2, IL_EDITOR_BULLET,
	       "ed_bullet",
	       ed_bullet.width, ed_bullet.height,
	       ed_bullet.mono_bits, ed_bullet.color_bits, ed_bullet.mask_bits);
  fe_make_icon(c, bg2, IL_EDITOR_CENTER,
	       "ed_center",
	       ed_center.width, ed_center.height,
	       ed_center.mono_bits, ed_center.color_bits, ed_center.mask_bits);
  fe_make_icon(c, bg2, IL_EDITOR_CLEAR,
	       "ed_clear",
	       ed_clear.width, ed_clear.height,
	       ed_clear.mono_bits, ed_clear.color_bits, ed_clear.mask_bits);
  fe_make_icon(c, bg2, IL_EDITOR_COLOR,
	       "ed_color",
	       ed_color.width, ed_color.height,
	       ed_color.mono_bits, ed_color.color_bits, ed_color.mask_bits);
  fe_make_icon(c, bg2, IL_EDITOR_FIXED,
	       "ed_fixed",
	       ed_fixed.width, ed_fixed.height,
	       ed_fixed.mono_bits, ed_fixed.color_bits, ed_fixed.mask_bits);
  fe_make_icon(c, bg2, IL_EDITOR_GROW,
	       "ed_grow",
	       ed_grow.width, ed_grow.height,
	       ed_grow.mono_bits, ed_grow.color_bits, ed_grow.mask_bits);
  fe_make_icon(c, bg2, IL_EDITOR_GROW_GREY,
	       "ed_grow.i",
	       ed_grow_i.width, ed_grow_i.height,
	       ed_grow_i.mono_bits, ed_grow_i.color_bits, ed_grow_i.mask_bits);
  fe_make_icon(c, bg2, IL_EDITOR_HRULE,
	       "ed_hrule",
	       ed_hrule.width, ed_hrule.height,
	       ed_hrule.mono_bits, ed_hrule.color_bits, ed_hrule.mask_bits);
  fe_make_icon(c, bg2, IL_EDITOR_IMAGE,
	       "ed_image",
	       ed_image.width, ed_image.height,
	       ed_image.mono_bits, ed_image.color_bits, ed_image.mask_bits);
  fe_make_icon(c, bg2, IL_EDITOR_INDENT,
	       "ed_indent",
	       ed_indent.width, ed_indent.height,
	       ed_indent.mono_bits, ed_indent.color_bits, ed_indent.mask_bits);
  fe_make_icon(c, bg2, IL_EDITOR_ITALIC,
	       "ed_italic",
	       ed_italic.width, ed_italic.height,
	       ed_italic.mono_bits, ed_italic.color_bits, ed_italic.mask_bits);
  fe_make_icon(c, bg2, IL_EDITOR_LEFT,
	       "ed_left",
	       ed_left.width, ed_left.height,
	       ed_left.mono_bits, ed_left.color_bits, ed_left.mask_bits);
  fe_make_icon(c, bg2, IL_EDITOR_LINK,
	       "ed_link",
	       ed_link.width, ed_link.height,
	       ed_link.mono_bits, ed_link.color_bits, ed_link.mask_bits);
  fe_make_icon(c, bg2, IL_EDITOR_NUMBER,
	       "ed_number",
	       ed_number.width, ed_number.height,
	       ed_number.mono_bits, ed_number.color_bits, ed_number.mask_bits);
  fe_make_icon(c, bg2, IL_EDITOR_OUTDENT,
	       "ed_outdent",
	       ed_outdent.width, ed_outdent.height,
	       ed_outdent.mono_bits, ed_outdent.color_bits, ed_outdent.mask_bits);
  fe_make_icon(c, bg2, IL_EDITOR_PROPS,
	       "ed_props",
	       ed_props.width, ed_props.height,
	       ed_props.mono_bits, ed_props.color_bits, ed_props.mask_bits);
  fe_make_icon(c, bg2, IL_EDITOR_RIGHT,
	       "ed_right",
	       ed_right.width, ed_right.height,
	       ed_right.mono_bits, ed_right.color_bits, ed_right.mask_bits);
  fe_make_icon(c, bg2, IL_EDITOR_SHRINK,
	       "ed_shrink",
	       ed_shrink.width, ed_shrink.height,
	       ed_shrink.mono_bits, ed_shrink.color_bits, ed_shrink.mask_bits);
  fe_make_icon(c, bg2, IL_EDITOR_SHRINK_GREY,
	       "ed_shrink.i",
	       ed_shrink_i.width, ed_shrink_i.height,
	       ed_shrink_i.mono_bits, ed_shrink_i.color_bits, ed_shrink_i.mask_bits);
  fe_make_icon(c, bg2, IL_EDITOR_TARGET,
	       "ed_target",
	       ed_target.width, ed_target.height,
	       ed_target.mono_bits, ed_target.color_bits, ed_target.mask_bits);
  fe_make_icon(c, bg2, IL_EDITOR_TABLE,
	       "ed_table",
	       ed_table.width, ed_table.height,
	       ed_table.mono_bits, ed_table.color_bits, ed_table.mask_bits);
}

/*
 *    Icons used on the page.
 */
static void
fe_init_editor_icons(MWContext* c)
{
  Pixel  bg_pixel = 0;
  static Bool done = False;

  if (done)
    return;

  XtVaGetValues (CONTEXT_DATA (c)->top_area, XmNbackground, &bg_pixel, 0);

  done = True;
  
  fe_make_icon(c, bg_pixel, IL_EDIT_UNSUPPORTED_TAG,
	       "ed_tag",
	       ed_tag.width, ed_tag.height,
	       ed_tag.mono_bits, ed_tag.color_bits, ed_tag.mask_bits);
  fe_make_icon(c, bg_pixel, IL_EDIT_UNSUPPORTED_END_TAG,
	       "ed_tage",
	       ed_tage.width, ed_tage.height,
	       ed_tage.mono_bits, ed_tage.color_bits, ed_tage.mask_bits);
  fe_make_icon(c, bg_pixel, IL_EDIT_FORM_ELEMENT,
	       "ed_form",
	       ed_form.width, ed_form.height,
	       ed_form.mono_bits, ed_form.color_bits, ed_form.mask_bits);
  fe_make_icon(c, bg_pixel, IL_EDIT_NAMED_ANCHOR,
	       "ed_target",
	       ed_target.width, ed_target.height,
	       ed_target.mono_bits, ed_target.color_bits, ed_target.mask_bits);
}

/*
 *    Map the toolbar location onto an ICON id.
 */
static int gold_browser_map[] = {
  IL_ICON_BACK, IL_ICON_BACK_GREY, IL_ICON_BACK_PT, IL_ICON_BACK_PT_GREY,
  IL_ICON_FWD,  IL_ICON_FWD_GREY,  IL_ICON_FWD_PT,  IL_ICON_FWD_PT_GREY,
  IL_ICON_HOME, IL_ICON_HOME_GREY, IL_ICON_HOME_PT, IL_ICON_HOME_PT_GREY,
  IL_EDITOR_EDIT,IL_EDITOR_EDIT_GREY,IL_EDITOR_EDIT_PT,IL_EDITOR_EDIT_PT_GREY,
  IL_ICON_RELOAD,IL_ICON_RELOAD_GREY,IL_ICON_RELOAD_PT,IL_ICON_RELOAD_PT_GREY,
  IL_ICON_LOAD, IL_ICON_LOAD_GREY, IL_ICON_LOAD_PT, IL_ICON_LOAD_PT_GREY,
  IL_ICON_OPEN, IL_ICON_OPEN_GREY, IL_ICON_OPEN_PT, IL_ICON_OPEN_PT_GREY,
  IL_ICON_PRINT,IL_ICON_PRINT_GREY,IL_ICON_PRINT_PT,IL_ICON_PRINT_PT_GREY,
  IL_ICON_FIND, IL_ICON_FIND_GREY, IL_ICON_FIND_PT, IL_ICON_FIND_PT_GREY,
  IL_ICON_STOP, IL_ICON_STOP_GREY, IL_ICON_STOP_PT, IL_ICON_STOP_PT_GREY,
  IL_ICON_NETSCAPE,IL_ICON_NETSCAPE,IL_ICON_NETSCAPE_PT,IL_ICON_NETSCAPE_PT
};

static int gold_editor_map[] = {
  /* toolbar */
  IL_EDITOR_NEW, IL_EDITOR_NEW_GREY, IL_EDITOR_NEW_PT, IL_EDITOR_NEW_PT_GREY,
  IL_EDITOR_OPEN,IL_EDITOR_OPEN_GREY,IL_EDITOR_OPEN_PT,IL_EDITOR_OPEN_PT_GREY,
  IL_EDITOR_SAVE,IL_EDITOR_SAVE_GREY,IL_EDITOR_SAVE_PT,IL_EDITOR_SAVE_PT_GREY,
  IL_EDITOR_BROWSE,IL_EDITOR_BROWSE_GREY,IL_EDITOR_BROWSE_PT,
    IL_EDITOR_BROWSE_PT_GREY,
  IL_EDITOR_CUT,IL_EDITOR_CUT_GREY, IL_EDITOR_CUT_PT,  IL_EDITOR_CUT_PT_GREY,
  IL_EDITOR_COPY, IL_EDITOR_COPY_GREY, IL_EDITOR_COPY_PT,
    IL_EDITOR_COPY_PT_GREY,
  IL_EDITOR_PASTE, IL_EDITOR_PASTE_GREY, IL_EDITOR_PASTE_PT,
    IL_EDITOR_PASTE_PT_GREY,
  IL_ICON_PRINT, IL_ICON_PRINT_GREY, IL_ICON_PRINT_PT, IL_ICON_PRINT_PT_GREY,
  IL_ICON_FIND,  IL_ICON_FIND_GREY,  IL_ICON_FIND_PT,  IL_ICON_FIND_PT_GREY,
  IL_EDITOR_PUBLISH, IL_EDITOR_PUBLISH_GREY, IL_EDITOR_PUBLISH_PT,
    IL_EDITOR_PUBLISH_PT_GREY
};

#endif /* EDITOR */

static void
fe_init_news_icons (MWContext *c)
{
  Pixel bg;
  static Bool done = False;

  fe_init_toolbar_icons_1 (c);

  if (done) return;
  done = True;

  XtVaGetValues (CONTEXT_DATA (c)->top_area, XmNbackground, &bg, 0);

#if 0
  fe_make_icon (c, bg, IL_MSG_HIER_ARTICLE,
		"HArt",
		HArt.width, HArt.height,
		HArt.mono_bits, HArt.color_bits, HArt.mask_bits);
  fe_make_icon (c, bg, IL_MSG_HIER_FOLDER_CLOSED,
		"HFolder",
		HFolder.width, HFolder.height,
		HFolder.mono_bits, HFolder.color_bits, HFolder.mask_bits);
  fe_make_icon (c, bg, IL_MSG_HIER_FOLDER_OPEN,
		"HFolderO",
		HFolderO.width, HFolderO.height,
		HFolderO.mono_bits, HFolderO.color_bits, HFolderO.mask_bits);
  fe_make_icon (c, bg, IL_MSG_HIER_NEWSGROUP,
		"HGroup",
		HGroup.width, HGroup.height,
		HGroup.mono_bits, HGroup.color_bits, HGroup.mask_bits);
  fe_make_icon (c, bg, IL_MSG_HIER_MARKED,
		"HMarked",
		HMarked.width, HMarked.height,
		HMarked.mono_bits, HMarked.color_bits, HMarked.mask_bits);
  fe_make_icon (c, bg, IL_MSG_HIER_MESSAGE,
		"HMsg",
		HMsg.width, HMsg.height,
		HMsg.mono_bits, HMsg.color_bits, HMsg.mask_bits);
  fe_make_icon (c, bg, IL_MSG_HIER_READ,
		"HRead",
		HRead.width, HRead.height,
		HRead.mono_bits, HRead.color_bits, HRead.mask_bits);
  fe_make_icon (c, bg, IL_MSG_HIER_SUBSCRIBED,
		"HSub",
		HSub.width, HSub.height,
		HSub.mono_bits, HSub.color_bits, HSub.mask_bits);
  fe_make_icon (c, bg, IL_MSG_HIER_UNMARKED,
		"HUMarked",
		HUMarked.width, HUMarked.height,
		HUMarked.mono_bits, HUMarked.color_bits, HUMarked.mask_bits);
  fe_make_icon (c, bg, IL_MSG_HIER_UNREAD,
		"HURead",
		HURead.width, HURead.height,
		HURead.mono_bits, HURead.color_bits, HURead.mask_bits);
  fe_make_icon (c, bg, IL_MSG_HIER_UNSUBSCRIBED,
		"HUSub",
		HUSub.width, HUSub.height,
		HUSub.mono_bits, HUSub.color_bits, HUSub.mask_bits);
#endif /* 0 */

  fe_make_icon (c, bg, IL_MSG_DELETE,
		"MDel",
		MDel.width, MDel.height,
		MDel.mono_bits, MDel.color_bits, MDel.mask_bits);
  fe_make_icon (c, bg, IL_MSG_DELETE_GREY,
		"MDel.i",
		MDel_i.width, MDel_i.height,
		MDel_i.mono_bits, MDel_i.color_bits, MDel_i.mask_bits);
  fe_make_icon (c, bg, IL_MSG_DELETE_PT,
		"MDel.pt",
		MDel_pt.width, MDel_pt.height,
		MDel_pt.mono_bits, MDel_pt.color_bits, MDel_pt.mask_bits);
  fe_make_icon (c, bg, IL_MSG_DELETE_PT_GREY,
		"MDel.pt.i",
		MDel_pt_i.width, MDel_pt_i.height,
		MDel_pt_i.mono_bits, MDel_pt_i.color_bits,
		MDel_pt_i.mask_bits);
  fe_make_icon (c, bg, IL_MSG_GET_MAIL,
		"MGetM",
		MGetM.width, MGetM.height,
		MGetM.mono_bits, MGetM.color_bits, MGetM.mask_bits);
  fe_make_icon (c, bg, IL_MSG_GET_MAIL_GREY,
		"MGetM.i",
		MGetM_i.width, MGetM_i.height,
		MGetM_i.mono_bits, MGetM_i.color_bits, MGetM_i.mask_bits);
  fe_make_icon (c, bg, IL_MSG_GET_MAIL_PT,
		"MGetM.pt",
		MGetM_pt.width, MGetM_pt.height,
		MGetM_pt.mono_bits, MGetM_pt.color_bits, MGetM_pt.mask_bits);
  fe_make_icon (c, bg, IL_MSG_MARK_ALL_READ,
		"MMarkA",
		MMarkA.width, MMarkA.height,
		MMarkA.mono_bits, MMarkA.color_bits, MMarkA.mask_bits);
  fe_make_icon (c, bg, IL_MSG_MARK_ALL_READ_GREY,
		"MMarkA.i",
		MMarkA_i.width, MMarkA_i.height,
		MMarkA_i.mono_bits, MMarkA_i.color_bits, MMarkA_i.mask_bits);
  fe_make_icon (c, bg, IL_MSG_MARK_ALL_READ_PT,
		"MMarkA.pt",
		MMarkA_pt.width, MMarkA_pt.height,
		MMarkA_pt.mono_bits, MMarkA_pt.color_bits,
		MMarkA_pt.mask_bits);
  fe_make_icon (c, bg, IL_MSG_MARK_ALL_READ_PT_GREY,
		"MMarkA.pt.i",
		MMarkA_pt_i.width, MMarkA_pt_i.height,
		MMarkA_pt_i.mono_bits, MMarkA_pt_i.color_bits,
		MMarkA_pt_i.mask_bits);
  fe_make_icon (c, bg, IL_MSG_MARK_THREAD_READ,
		"MMarkT",
		MMarkT.width, MMarkT.height,
		MMarkT.mono_bits, MMarkT.color_bits, MMarkT.mask_bits);
  fe_make_icon (c, bg, IL_MSG_MARK_THREAD_READ_GREY,
		"MMarkT.i",
		MMarkT_i.width, MMarkT_i.height,
		MMarkT_i.mono_bits, MMarkT_i.color_bits, MMarkT_i.mask_bits);
  fe_make_icon (c, bg, IL_MSG_MARK_THREAD_READ_PT,
		"MMarkT.pt",
		MMarkT_pt.width, MMarkT_pt.height,
		MMarkT_pt.mono_bits, MMarkT_pt.color_bits,
		MMarkT_pt.mask_bits);
  fe_make_icon (c, bg, IL_MSG_MARK_THREAD_READ_PT_GREY,
		"MMarkT.pt.i",
		MMarkT_pt_i.width, MMarkT_pt_i.height,
		MMarkT_pt_i.mono_bits, MMarkT_pt_i.color_bits,
		MMarkT_pt_i.mask_bits);
  fe_make_icon (c, bg, IL_MSG_FORWARD_MSG,
		"MMsgFwd",
		MMsgFwd.width, MMsgFwd.height,
		MMsgFwd.mono_bits, MMsgFwd.color_bits, MMsgFwd.mask_bits);
  fe_make_icon (c, bg, IL_MSG_FORWARD_MSG_GREY,
		"MMsgFwd.i",
		MMsgFwd_i.width, MMsgFwd_i.height,
		MMsgFwd_i.mono_bits, MMsgFwd_i.color_bits,
		MMsgFwd_i.mask_bits);
  fe_make_icon (c, bg, IL_MSG_FORWARD_MSG_PT,
		"MMsgFwd.pt",
		MMsgFwd_pt.width, MMsgFwd_pt.height,
		MMsgFwd_pt.mono_bits, MMsgFwd_pt.color_bits,
		MMsgFwd_pt.mask_bits);
  fe_make_icon (c, bg, IL_MSG_FORWARD_MSG_PT_GREY,
		"MMsgFwd.pt.i",
		MMsgFwd_pt_i.width, MMsgFwd_pt_i.height,
		MMsgFwd_pt_i.mono_bits, MMsgFwd_pt_i.color_bits,
		MMsgFwd_pt_i.mask_bits);
  fe_make_icon (c, bg, IL_MSG_NEW_MSG,
		"MMsgNew",
		MMsgNew.width, MMsgNew.height,
		MMsgNew.mono_bits, MMsgNew.color_bits, MMsgNew.mask_bits);
  fe_make_icon (c, bg, IL_MSG_NEW_MSG_GREY,
		"MMsgNew.i",
		MMsgNew_i.width, MMsgNew_i.height,
		MMsgNew_i.mono_bits, MMsgNew_i.color_bits,
		MMsgNew_i.mask_bits);
  fe_make_icon (c, bg, IL_MSG_NEW_MSG_PT,
		"MMsgNew.pt",
		MMsgNew_pt.width, MMsgNew_pt.height,
		MMsgNew_pt.mono_bits, MMsgNew_pt.color_bits,
		MMsgNew_pt.mask_bits);
  fe_make_icon (c, bg, IL_MSG_NEW_MSG_PT_GREY,
		"MMsgNew.pt.i",
		MMsgNew_pt_i.width, MMsgNew_pt_i.height,
		MMsgNew_pt_i.mono_bits, MMsgNew_pt_i.color_bits,
		MMsgNew_pt_i.mask_bits);
  fe_make_icon (c, bg, IL_MSG_NEW_POST,
		"MMsgPst",
		MMsgPst.width, MMsgPst.height,
		MMsgPst.mono_bits, MMsgPst.color_bits, MMsgPst.mask_bits);
  fe_make_icon (c, bg, IL_MSG_NEW_POST_GREY,
		"MMsgPst.i",
		MMsgPst_i.width, MMsgPst_i.height,
		MMsgPst_i.mono_bits, MMsgPst_i.color_bits,
		MMsgPst_i.mask_bits);
  fe_make_icon (c, bg, IL_MSG_NEW_POST_PT,
		"MMsgPst.pt",
		MMsgPst_pt.width, MMsgPst_pt.height,
		MMsgPst_pt.mono_bits, MMsgPst_pt.color_bits,
		MMsgPst_pt.mask_bits);
  fe_make_icon (c, bg, IL_MSG_NEW_POST_PT_GREY,
		"MMsgPst.pt.i",
		MMsgPst_pt_i.width, MMsgPst_pt_i.height,
		MMsgPst_pt_i.mono_bits, MMsgPst_pt_i.color_bits,
		MMsgPst_pt_i.mask_bits);
  fe_make_icon (c, bg, IL_MSG_FOLLOWUP,
		"MMsgPstF",
		MMsgPstF.width, MMsgPstF.height,
		MMsgPstF.mono_bits, MMsgPstF.color_bits, MMsgPstF.mask_bits);
  fe_make_icon (c, bg, IL_MSG_FOLLOWUP_GREY,
		"MMsgPstF.i",
		MMsgPstF_i.width, MMsgPstF_i.height,
		MMsgPstF_i.mono_bits, MMsgPstF_i.color_bits,
		MMsgPstF_i.mask_bits);
  fe_make_icon (c, bg, IL_MSG_FOLLOWUP_PT,
		"MMsgPstF.pt",
		MMsgPstF_pt.width, MMsgPstF_pt.height,
		MMsgPstF_pt.mono_bits, MMsgPstF_pt.color_bits,
		MMsgPstF_pt.mask_bits);
  fe_make_icon (c, bg, IL_MSG_FOLLOWUP_PT_GREY,
		"MMsgPstF.pt.i",
		MMsgPstF_pt_i.width, MMsgPstF_pt_i.height,
		MMsgPstF_pt_i.mono_bits, MMsgPstF_pt_i.color_bits,
		MMsgPstF_pt_i.mask_bits);
  fe_make_icon (c, bg, IL_MSG_FOLLOWUP_AND_REPLY,
		"MMsgPstR",
		MMsgPstR.width, MMsgPstR.height,
		MMsgPstR.mono_bits, MMsgPstR.color_bits, MMsgPstR.mask_bits);
  fe_make_icon (c, bg, IL_MSG_FOLLOWUP_AND_REPLY_GREY,
		"MMsgPstR.i",
		MMsgPstR_i.width, MMsgPstR_i.height,
		MMsgPstR_i.mono_bits, MMsgPstR_i.color_bits,
		MMsgPstR_i.mask_bits);
  fe_make_icon (c, bg, IL_MSG_FOLLOWUP_AND_REPLY_PT,
		"MMsgPstR.pt",
		MMsgPstR_pt.width, MMsgPstR_pt.height,
		MMsgPstR_pt.mono_bits, MMsgPstR_pt.color_bits,
		MMsgPstR_pt.mask_bits);
  fe_make_icon (c, bg, IL_MSG_FOLLOWUP_AND_REPLY_PT_GREY,
		"MMsgPstR.pt.i",
		MMsgPstR_pt_i.width, MMsgPstR_pt_i.height,
		MMsgPstR_pt_i.mono_bits, MMsgPstR_pt_i.color_bits,
		MMsgPstR_pt_i.mask_bits);
  fe_make_icon (c, bg, IL_MSG_REPLY_TO_SENDER,
		"MMsgRep",
		MMsgRep.width, MMsgRep.height,
		MMsgRep.mono_bits, MMsgRep.color_bits, MMsgRep.mask_bits);
  fe_make_icon (c, bg, IL_MSG_REPLY_TO_SENDER_GREY,
		"MMsgRep.i",
		MMsgRep_i.width, MMsgRep_i.height,
		MMsgRep_i.mono_bits, MMsgRep_i.color_bits,
		MMsgRep_i.mask_bits);
  fe_make_icon (c, bg, IL_MSG_REPLY_TO_SENDER_PT,
		"MMsgRep.pt",
		MMsgRep_pt.width, MMsgRep_pt.height,
		MMsgRep_pt.mono_bits, MMsgRep_pt.color_bits,
		MMsgRep_pt.mask_bits);
  fe_make_icon (c, bg, IL_MSG_REPLY_TO_SENDER_PT_GREY,
		"MMsgRep.pt.i",
		MMsgRep_pt_i.width, MMsgRep_pt_i.height,
		MMsgRep_pt_i.mono_bits, MMsgRep_pt_i.color_bits,
		MMsgRep_pt_i.mask_bits);
  fe_make_icon (c, bg, IL_MSG_REPLY_TO_ALL,
		"MMsgRepA",
		MMsgRepA.width, MMsgRepA.height,
		MMsgRepA.mono_bits, MMsgRepA.color_bits, MMsgRepA.mask_bits);
  fe_make_icon (c, bg, IL_MSG_REPLY_TO_ALL_GREY,
		"MMsgRepA.i",
		MMsgRepA_i.width, MMsgRepA_i.height,
		MMsgRepA_i.mono_bits, MMsgRepA_i.color_bits,
		MMsgRepA_i.mask_bits);
  fe_make_icon (c, bg, IL_MSG_REPLY_TO_ALL_PT,
		"MMsgRepA.pt",
		MMsgRepA_pt.width, MMsgRepA_pt.height,
		MMsgRepA_pt.mono_bits, MMsgRepA_pt.color_bits,
		MMsgRepA_pt.mask_bits);
  fe_make_icon (c, bg, IL_MSG_REPLY_TO_ALL_PT_GREY,
		"MMsgRepA.pt.i",
		MMsgRepA_pt_i.width, MMsgRepA_pt_i.height,
		MMsgRepA_pt_i.mono_bits, MMsgRepA_pt_i.color_bits,
		MMsgRepA_pt_i.mask_bits);
  fe_make_icon (c, bg, IL_MSG_NEXT_UNREAD,
		"MNextU",
		MNextU.width, MNextU.height,
		MNextU.mono_bits, MNextU.color_bits, MNextU.mask_bits);
  fe_make_icon (c, bg, IL_MSG_NEXT_UNREAD_GREY,
		"MNextU.i",
		MNextU_i.width, MNextU_i.height,
		MNextU_i.mono_bits, MNextU_i.color_bits, MNextU_i.mask_bits);
  fe_make_icon (c, bg, IL_MSG_NEXT_UNREAD_PT,
		"MNextU.pt",
		MNextU_pt.width, MNextU_pt.height,
		MNextU_pt.mono_bits, MNextU_pt.color_bits,
		MNextU_pt.mask_bits);
  fe_make_icon (c, bg, IL_MSG_NEXT_UNREAD_PT_GREY,
		"MNextU.pt.i",
		MNextU_pt_i.width, MNextU_pt_i.height,
		MNextU_pt_i.mono_bits, MNextU_pt_i.color_bits,
		MNextU_pt_i.mask_bits);
  fe_make_icon (c, bg, IL_MSG_PREV_UNREAD,
		"MPrevU",
		MPrevU.width, MPrevU.height,
		MPrevU.mono_bits, MPrevU.color_bits, MPrevU.mask_bits);
  fe_make_icon (c, bg, IL_MSG_PREV_UNREAD_GREY,
		"MPrevU.i",
		MPrevU_i.width, MPrevU_i.height,
		MPrevU_i.mono_bits, MPrevU_i.color_bits, MPrevU_i.mask_bits);
  fe_make_icon (c, bg, IL_MSG_PREV_UNREAD_PT,
		"MPrevU.pt",
		MPrevU_pt.width, MPrevU_pt.height,
		MPrevU_pt.mono_bits, MPrevU_pt.color_bits,
		MPrevU_pt.mask_bits);
  fe_make_icon (c, bg, IL_MSG_PREV_UNREAD_PT_GREY,
		"MPrevU.pt.i",
		MPrevU_pt_i.width, MPrevU_pt_i.height,
		MPrevU_pt_i.mono_bits, MPrevU_pt_i.color_bits,
		MPrevU_pt_i.mask_bits);
}

static void
fe_init_compose_icons (MWContext *c)
{
  Pixel bg;
  static Bool done = False;
  if (done) return;
  done = True;

  fe_init_toolbar_icons_1 (c);

  XtVaGetValues (CONTEXT_DATA (c)->top_area, XmNbackground, &bg, 0);

  fe_make_icon (c, bg, IL_COMPOSE_SEND,
		"CSendM",
		CSendM.width, CSendM.height,
		CSendM.mono_bits, CSendM.color_bits, CSendM.mask_bits);
  fe_make_icon (c, bg, IL_COMPOSE_SEND_PT,
		"CSendM.pt",
		CSendM_pt.width, CSendM_pt.height,
		CSendM_pt.mono_bits, CSendM_pt.color_bits, CSendM_pt.mask_bits);
  fe_make_icon (c, bg, IL_COMPOSE_SEND_GREY,
		"CSendM.i",
		CSendM_i.width, CSendM_i.height,
		CSendM_i.mono_bits, CSendM_i.color_bits, CSendM_i.mask_bits);
  fe_make_icon (c, bg, IL_COMPOSE_SEND_PT_GREY,
		"CSendM.pt.i",
		CSendM_pt_i.width, CSendM_pt_i.height,
		CSendM_pt_i.mono_bits, CSendM_pt_i.color_bits,
		CSendM_pt_i.mask_bits);
  fe_make_icon (c, bg, IL_COMPOSE_ATTACH,
		"CAttachM",
		CAttachM.width, CAttachM.height,
		CAttachM.mono_bits, CAttachM.color_bits, CAttachM.mask_bits);
  fe_make_icon (c, bg, IL_COMPOSE_ATTACH_PT,
		"CAttachM.pt",
		CAttachM_pt.width, CAttachM_pt.height,
		CAttachM_pt.mono_bits, CAttachM_pt.color_bits,
		CAttachM_pt.mask_bits);
  fe_make_icon (c, bg, IL_COMPOSE_ATTACH_GREY,
		"CAttachM.i",
		CAttachM_i.width, CAttachM_i.height,
		CAttachM_i.mono_bits, CAttachM_i.color_bits, CAttachM_i.mask_bits);
  fe_make_icon (c, bg, IL_COMPOSE_ATTACH_PT_GREY,
		"CAttachM.pt.i",
		CAttachM_pt_i.width, CAttachM_pt_i.height,
		CAttachM_pt_i.mono_bits, CAttachM_pt_i.color_bits,
		CAttachM_pt_i.mask_bits);
  fe_make_icon (c, bg, IL_COMPOSE_ADDRESSBOOK,
		"CAddress",
		CAddress.width, CAddress.height,
		CAddress.mono_bits, CAddress.color_bits, CAddress.mask_bits);
  fe_make_icon (c, bg, IL_COMPOSE_ADDRESSBOOK_PT,
		"CAddress.pt",
		CAddress_pt.width, CAddress_pt.height,
		CAddress_pt.mono_bits, CAddress_pt.color_bits,
		CAddress_pt.mask_bits);
  fe_make_icon (c, bg, IL_COMPOSE_ADDRESSBOOK_GREY,
		"CAddress.i",
		CAddress_i.width, CAddress_i.height,
		CAddress_i.mono_bits, CAddress_i.color_bits,
		CAddress_i.mask_bits);
  fe_make_icon (c, bg, IL_COMPOSE_ADDRESSBOOK_PT_GREY,
		"CAddress.pt.i",
		CAddress_pt_i.width, CAddress_pt_i.height,
		CAddress_pt_i.mono_bits, CAddress_pt_i.color_bits,
		CAddress_pt_i.mask_bits);
  fe_make_icon (c, bg, IL_COMPOSE_QUOTE,
		"CQuote",
		CQuote.width, CQuote.height,
		CQuote.mono_bits, CQuote.color_bits, CQuote.mask_bits);
  fe_make_icon (c, bg, IL_COMPOSE_QUOTE_PT,
		"CQuote.pt",
		CQuote_pt.width, CQuote_pt.height,
		CQuote_pt.mono_bits, CQuote_pt.color_bits,
		CQuote_pt.mask_bits);
  fe_make_icon (c, bg, IL_COMPOSE_QUOTE_GREY,
		"CQuote.i",
		CQuote_i.width, CQuote_i.height,
		CQuote_i.mono_bits, CQuote_i.color_bits,
		CQuote_i.mask_bits);
  fe_make_icon (c, bg, IL_COMPOSE_QUOTE_PT_GREY,
		"CQuote.pt.i",
		CQuote_pt_i.width, CQuote_pt_i.height,
		CQuote_pt_i.mono_bits, CQuote_pt_i.color_bits,
		CQuote_pt_i.mask_bits);
  fe_make_icon (c, bg, IL_COMPOSE_SENDLATER,
		"CSendLater",
		CSendLater.width, CSendLater.height,
		CSendLater.mono_bits, CSendLater.color_bits, CSendLater.mask_bits);
  fe_make_icon (c, bg, IL_COMPOSE_SENDLATER_PT,
		"CSendLater.pt",
		CSendLater_pt.width, CSendLater_pt.height,
		CSendLater_pt.mono_bits, CSendLater_pt.color_bits,
		CSendLater_pt.mask_bits);
  fe_make_icon (c, bg, IL_COMPOSE_SENDLATER_GREY,
		"CSendLater.i",
		CSendLater_i.width, CSendLater_i.height,
		CSendLater_i.mono_bits, CSendLater_i.color_bits,
		CSendLater_i.mask_bits);
  fe_make_icon (c, bg, IL_COMPOSE_SENDLATER_PT_GREY,
		"CSendLater.pt.i",
		CSendLater_pt_i.width, CSendLater_pt_i.height,
		CSendLater_pt_i.mono_bits, CSendLater_pt_i.color_bits,
		CSendLater_pt_i.mask_bits);
}

static void
fe_init_sa_icons (MWContext *c)
{
  Pixel bg = 0;
  static Bool done = False;
  if (done) return;
  done = True;

  fe_init_toolbar_icons_1 (c);

  /* If this is a grid cell, find the topmost window, since that's the one
     on which `top_area' is filled in. */
  while (c && c->is_grid_cell && c->grid_parent)
    c = c->grid_parent;

  XP_ASSERT(c && CONTEXT_DATA (c)->top_area);
  if (CONTEXT_DATA (c)->top_area)
    XtVaGetValues (CONTEXT_DATA (c)->top_area, XmNbackground, &bg, 0);
  else
    bg = WhitePixelOfScreen (XtScreen (CONTEXT_WIDGET (c)));

  fe_make_icon (c, bg, IL_SA_SIGNED,
		"A_Signed",
		A_Signed.width, A_Signed.height,
		A_Signed.mono_bits, A_Signed.color_bits, A_Signed.mask_bits);
  fe_make_icon (c, bg, IL_SA_ENCRYPTED,
		"A_Encrypt",
		A_Encrypt.width, A_Encrypt.height,
		A_Encrypt.mono_bits, A_Encrypt.color_bits,
		A_Encrypt.mask_bits);
  fe_make_icon (c, bg, IL_SA_NONENCRYPTED,
		"A_NoEncrypt",
		A_NoEncrypt.width, A_NoEncrypt.height,
		A_NoEncrypt.mono_bits, A_NoEncrypt.color_bits,
		A_NoEncrypt.mask_bits);
  fe_make_icon (c, bg, IL_SA_SIGNED_BAD,
		"A_SignBad",
		A_SignBad.width, A_SignBad.height,
		A_SignBad.mono_bits, A_SignBad.color_bits,
		A_SignBad.mask_bits);
  fe_make_icon (c, bg, IL_SA_ENCRYPTED_BAD,
		"A_EncrypBad",
		A_EncrypBad.width, A_EncrypBad.height,
		A_EncrypBad.mono_bits, A_EncrypBad.color_bits,
		A_EncrypBad.mask_bits);
  fe_make_icon (c, bg, IL_SMIME_ATTACHED,
		"M_Attach",
		M_Attach.width, M_Attach.height,
		M_Attach.mono_bits, M_Attach.color_bits, M_Attach.mask_bits);
  fe_make_icon (c, bg, IL_SMIME_SIGNED,
		"M_Signed",
		M_Signed.width, M_Signed.height,
		M_Signed.mono_bits, M_Signed.color_bits, M_Signed.mask_bits);
  fe_make_icon (c, bg, IL_SMIME_ENCRYPTED,
		"M_Encrypt",
		M_Encrypt.width, M_Encrypt.height,
		M_Encrypt.mono_bits, M_Encrypt.color_bits,
		M_Encrypt.mask_bits);
  fe_make_icon (c, bg, IL_SMIME_ENC_SIGNED,
		"M_SignEncyp",
		M_SignEncyp.width, M_SignEncyp.height,
		M_SignEncyp.mono_bits, M_SignEncyp.color_bits,
		M_SignEncyp.mask_bits);
  fe_make_icon (c, bg, IL_SMIME_SIGNED_BAD,
		"M_SignBad",
		M_SignBad.width, M_SignBad.height,
		M_SignBad.mono_bits, M_SignBad.color_bits,
		M_SignBad.mask_bits);
  fe_make_icon (c, bg, IL_SMIME_ENCRYPTED_BAD,
		"M_EncrypBad",
		M_EncrypBad.width, M_EncrypBad.height,
		M_EncrypBad.mono_bits, M_EncrypBad.color_bits,
		M_EncrypBad.mask_bits);
  fe_make_icon (c, bg, IL_SMIME_ENC_SIGNED_BAD,
		"M_SgnEncypBad",
		M_SgnEncypBad.width, M_SgnEncypBad.height,
		M_SgnEncypBad.mono_bits, M_SgnEncypBad.color_bits,
		M_SgnEncypBad.mask_bits);
}


extern MSG_BIFF_STATE biffstate;

void
fe_InitIcons (MWContext *c)
{
  int icon_index;

  fe_init_document_icons (c);
  switch (c->type) {
      case MWContextMail:
	icon_index = (biffstate == MSG_BIFF_NewMail) ?
			IL_ICON_DESKTOP_YESMAIL : IL_ICON_DESKTOP_NOMAIL;
	break;
      case MWContextMessageComposition:
	icon_index = IL_ICON_DESKTOP_COMPOSE;
	break;
      case MWContextNews:
	icon_index = IL_ICON_DESKTOP_NEWS;
	break;
      case MWContextBookmarks:
	icon_index = IL_ICON_DESKTOP_BOOKMARK;
	break;
      case MWContextAddressBook:
	icon_index = IL_ICON_DESKTOP_ABOOK;
	break;
#ifdef EDITOR
      case MWContextEditor:
        icon_index = IL_ICON_DESKTOP_EDITOR;
	break;
#endif /*EDITOR*/
      case MWContextBrowser:	/* FALL THROUGH */
      case MWContextDialog:	/* FALL THROUGH */
      default:
	icon_index = IL_ICON_DESKTOP_NAVIGATOR;
	break;
  }
  if (fe_icons [icon_index].pixmap) {
      Arg av [5];
      int ac = 0;
      XtSetArg (av[ac], XtNiconPixmap, fe_icons[icon_index].pixmap); ac++;
      if (!fe_icons [icon_index].mask) {
	  /*
	   *    Must make a mask, because olwm is so stupid, it doesn't clip
	   *    mask the image to the image size, just blasts away at 60x60.
	   */
	  Pixmap mask_pixmap;
	  Dimension height = fe_icons[icon_index].height;
	  Dimension width = fe_icons[icon_index].width;
	  Display*  dpy = XtDisplay(CONTEXT_WIDGET(c));
	  Screen*   screen = XtScreen(CONTEXT_WIDGET(c));
	  XGCValues gcv;
	  GC        gc;
	  
	  mask_pixmap = XCreatePixmap(dpy, RootWindowOfScreen(screen),
				      width, height, 1);
	  
	  gcv.function = GXset;
	  /* gcv.foreground = BlackPixelOfScreen(screen); */
	  gc = XCreateGC(dpy, mask_pixmap, GCFunction, &gcv);
	  XFillRectangle(dpy, mask_pixmap, gc, 0, 0, width, height);
	  XFreeGC(dpy, gc);

	  fe_icons[icon_index].mask = mask_pixmap;
      }
      XtSetArg (av[ac], XtNiconMask, fe_icons[icon_index].mask); ac++;
      XtSetValues (CONTEXT_WIDGET (c), av, ac);
  }
}

int logomatic = 0;

struct fe_icon_data *
fe_which_anim (Boolean large_p, int *offset, int *frames, Boolean *rest_p)
{
  struct fe_icon_data *anim = 0;
  int anim_number = 9999;

  if ( anim_custom_large && large_p ) {
      anim = anim_custom_large;
      anim_number = 7;
      if (rest_p) *rest_p = True;
  } else if ( anim_custom_small && !large_p ) {
      anim = anim_custom_small;
      anim_number = 8;
      if (rest_p) *rest_p = True;
  } else if (logomatic == 1 && large_p)
    {
      anim = anim_mozilla_large;
      anim_number = 2;
      if (rest_p) *rest_p = True;
    }
  else if (logomatic == 1 && !large_p)
    {
      anim = anim_mozilla_small;
      anim_number = 3;
      if (rest_p) *rest_p = True;
    }
  else if (logomatic == 2 && large_p)
    {
      anim = anim_compass_large;
      anim_number = 4;
      if (rest_p) *rest_p = False;
    }
  else if (logomatic == 2 && !large_p)
    {
      anim = anim_compass_small;
      anim_number = 5;
      if (rest_p) *rest_p = False;
    }
#ifdef __sgi
  else if (large_p && fe_VendorAnim)
    {
      anim = anim_sgi_large;
      if (rest_p) *rest_p = True;
      anim_number = 6;
    }
  else if (!large_p && fe_VendorAnim)
    {
      anim = anim_sgi_small;
      if (rest_p) *rest_p = True;
      anim_number = 7;
    }
#endif /* __sgi */
  else if (large_p)
    {
      anim = anim_meteors_large;
      if (rest_p) *rest_p = True;
      anim_number = 0;
    }
  else
    {
      anim = anim_meteors_small;
      if (rest_p) *rest_p = True;
      anim_number = 1;
    }

  if (! anim) abort ();

  if (anim_number > MAX_ANIMS) abort();
  *offset = IL_ICON_LOGO + (anim_number * MAX_ANIM_FRAMES);

  *frames = fe_anim_frames [anim_number];

  if (*frames <= 0 || *frames > MAX_ANIM_FRAMES)
    abort ();

  return anim;
}

Pixmap
fe_LogoPixmap (MWContext *context, Dimension *w, Dimension *h, Boolean large_p)
{
  struct fe_icon_data *anim;
  int id0;
  int frames;
  anim = fe_which_anim (large_p, &id0, &frames, 0);

  if (!fe_icons [id0].pixmap)  /* pixmaps not yet created for this one. */
    {
      Pixel bg = 0;
      int i = id0;
      int j;

      XtVaGetValues (CONTEXT_DATA (context)->top_area, XmNbackground, &bg, 0);
      for (j = 0; j < frames; j++)
	fe_make_icon (context, bg, i++,
		      NULL,
		      anim[j].width, anim[j].height,
		      anim[j].mono_bits, anim[j].color_bits,
		      anim[j].mask_bits);
    }

  if (w) *w = fe_icons [id0].width;
  if (h) *h = fe_icons [id0].height;
  return fe_icons [id0].pixmap;
}

#ifdef HAVE_SECURITY
Pixmap
fe_SecurityPixmap (MWContext *context, Dimension *w, Dimension *h,
		   int type)
{
  int index = 0;
  switch (type)
    {
    case SSL_SECURITY_STATUS_ON_LOW:  index = IL_ICON_SECURITY_ON; break;
    case SSL_SECURITY_STATUS_ON_HIGH: index = IL_ICON_SECURITY_HIGH; break;

    case SSL_SECURITY_STATUS_OFF:	/* Fall Through */
    default:				index = IL_ICON_SECURITY_OFF;
    }
  if (context)	/* for awt, we know we've already done this */
      fe_init_document_icons (context);
  if (w) *w = fe_icons [index].width;
  if (h) *h = fe_icons [index].height;
  return fe_icons [index].pixmap;
}
#endif /* HAVE_SECURITY */



Pixmap
fe_SecurityAdvisorButtonPixmap(MWContext *context,
			       XP_Bool encrypting, XP_Bool signing)
{
  int icon;
  if (encrypting && signing)	icon = IL_SMIME_ENC_SIGNED;
  else if (encrypting)		icon = IL_SMIME_ENCRYPTED;
  else if (signing)		icon = IL_SMIME_SIGNED;
  else				icon = IL_SA_NONENCRYPTED;

  if (!fe_icons[icon].pixmap)
    fe_init_sa_icons (context);
  XP_ASSERT(fe_icons[icon].pixmap);
  return fe_icons[icon].pixmap;
}


Pixmap
fe_ToolbarPixmap (MWContext *context, int i, Boolean disabled_p,
		  Boolean urls_p)
{
  Boolean both_p = CONTEXT_DATA (context)->show_toolbar_text_p;
  int offset;
  int grey_offset;
  int pt_offset;
#ifdef EDITOR
  static align_icons_done=0; 	/* added 14MAR96RCJ */
#endif

  if (both_p)
    fe_init_toolbar_icons_2 (context);
  else
    fe_init_toolbar_icons_1 (context);

  if (urls_p)
    return (fe_icons [IL_ICON_TOUR + i].pixmap);

  offset = (both_p ? IL_ICON_BACK_PT : IL_ICON_BACK);
  grey_offset = (disabled_p ? 1 : 0);
  pt_offset = (both_p ? 1: 0);

  switch (context->type)
    {
#ifdef EDITOR
    case MWContextEditor:
      if (both_p)
	fe_init_editor_icons_withtext(context);
      else
	fe_init_editor_icons_notext(context);

      fe_init_editor_icons_other(context);

      if (i < 10)
	i = gold_editor_map[(4*i) + grey_offset + (2*pt_offset)];
      else if (i < 23) 
	i = IL_EDITOR_OTHER_GROUP + (2*(i - 10)) + grey_offset;
      else if (i>=IL_ALIGN1_RAISED && i <= IL_ALIGN7_DEPRESSED) {
           if (!align_icons_done) {
               fe_init_align_icons(context);
               align_icons_done=1;
           }
      }
      else 
	i = IL_EDITOR_BULLET + (2*(i - 23)) + grey_offset;

      return fe_icons[i].pixmap;
      /*NOTREACHED*/
      break;
    case MWContextBrowser:
      return fe_icons[gold_browser_map[(4*i) + grey_offset + (2*pt_offset)]].pixmap;
      /*NOTREACHED*/
      break;
#else
    case MWContextBrowser:
      return fe_icons [offset + (i*2) + (disabled_p ? 1 : 0)].pixmap;
      break;
#endif
    case MWContextMail:
      fe_init_news_icons (context);
      switch (i)
	{
	case 0: return fe_icons[disabled_p && both_p ? IL_MSG_GET_MAIL_PT_GREY:
			        disabled_p ? IL_MSG_GET_MAIL_GREY :
			        both_p ? IL_MSG_GET_MAIL_PT :
				IL_MSG_GET_MAIL].pixmap;
	case 1: return fe_icons[disabled_p && both_p ? IL_MSG_DELETE_PT_GREY :
				disabled_p ? IL_MSG_DELETE_GREY :
				both_p ? IL_MSG_DELETE_PT :
				IL_MSG_DELETE].pixmap;
	case 2: return fe_icons[disabled_p && both_p ? IL_MSG_NEW_MSG_PT_GREY :
				disabled_p ? IL_MSG_NEW_MSG_GREY :
				both_p ? IL_MSG_NEW_MSG_PT :
				IL_MSG_NEW_MSG].pixmap;
	case 3: return fe_icons[disabled_p && both_p ?
			         IL_MSG_REPLY_TO_SENDER_PT_GREY :
				disabled_p ? IL_MSG_REPLY_TO_SENDER_GREY :
				both_p ? IL_MSG_REPLY_TO_SENDER_PT :
				IL_MSG_REPLY_TO_SENDER].pixmap;
	case 4: return fe_icons[disabled_p && both_p ?
			          IL_MSG_REPLY_TO_ALL_PT_GREY :
				disabled_p ? IL_MSG_REPLY_TO_ALL_GREY :
				both_p ? IL_MSG_REPLY_TO_ALL_PT :
				IL_MSG_REPLY_TO_ALL].pixmap;
	case 5: return fe_icons[disabled_p && both_p ?
			         IL_MSG_FORWARD_MSG_PT_GREY :
				disabled_p ? IL_MSG_FORWARD_MSG_GREY :
				both_p ? IL_MSG_FORWARD_MSG_PT :
				IL_MSG_FORWARD_MSG].pixmap;
	case 6: return fe_icons[disabled_p && both_p ?
				  IL_MSG_PREV_UNREAD_PT_GREY :
				disabled_p ? IL_MSG_PREV_UNREAD_GREY :
				both_p ? IL_MSG_PREV_UNREAD_PT :
				IL_MSG_PREV_UNREAD].pixmap;
	case 7: return fe_icons[disabled_p && both_p ?
			         IL_MSG_NEXT_UNREAD_PT_GREY :
				disabled_p ? IL_MSG_NEXT_UNREAD_GREY :
				both_p ? IL_MSG_NEXT_UNREAD_PT :
				IL_MSG_NEXT_UNREAD].pixmap;
#ifdef DEBUG_jwz
	case 8: return fe_icons[disabled_p && both_p ?
			         IL_MSG_MARK_THREAD_READ_PT_GREY :
				disabled_p ? IL_MSG_MARK_THREAD_READ_GREY :
				both_p ? IL_MSG_MARK_THREAD_READ_PT :
				IL_MSG_MARK_THREAD_READ].pixmap;
	case 9: return fe_icons[disabled_p && both_p ? IL_ICON_PRINT_PT_GREY :
				disabled_p ? IL_ICON_PRINT_GREY :
				both_p ? IL_ICON_PRINT_PT :
				IL_ICON_PRINT].pixmap;
	case 10: return fe_icons[disabled_p && both_p ? IL_ICON_STOP_PT_GREY :
				disabled_p ? IL_ICON_STOP_GREY :
				both_p ? IL_ICON_STOP_PT :
				IL_ICON_STOP].pixmap;
#else /* !DEBUG_jwz */
	case 8: return fe_icons[disabled_p && both_p ? IL_ICON_PRINT_PT_GREY :
				disabled_p ? IL_ICON_PRINT_GREY :
				both_p ? IL_ICON_PRINT_PT :
				IL_ICON_PRINT].pixmap;
	case 9: return fe_icons[disabled_p && both_p ? IL_ICON_STOP_PT_GREY :
				disabled_p ? IL_ICON_STOP_GREY :
				both_p ? IL_ICON_STOP_PT :
				IL_ICON_STOP].pixmap;
#endif /* !DEBUG_jwz */
/*	default: abort (); */
	default: fe_perror (context, "internal error: bogus mail icon accessed"); break;
	}
      break;
    case MWContextNews:
      fe_init_news_icons (context);
      switch (i)
	{
	case 0: return fe_icons[disabled_p && both_p ? IL_MSG_NEW_POST_PT_GREY:
				disabled_p ? IL_MSG_NEW_POST_GREY :
				both_p ? IL_MSG_NEW_POST_PT :
				IL_MSG_NEW_POST].pixmap;
	case 1: return fe_icons[disabled_p && both_p ? IL_MSG_NEW_MSG_PT_GREY :
				disabled_p ? IL_MSG_NEW_MSG_GREY :
				both_p ? IL_MSG_NEW_MSG_PT :
				IL_MSG_NEW_MSG].pixmap;
	case 2: return fe_icons[disabled_p && both_p ?
			         IL_MSG_REPLY_TO_SENDER_PT_GREY :
				disabled_p ? IL_MSG_REPLY_TO_SENDER_GREY :
				both_p ? IL_MSG_REPLY_TO_SENDER_PT :
				IL_MSG_REPLY_TO_SENDER].pixmap;
	case 3: return fe_icons[disabled_p && both_p ? IL_MSG_FOLLOWUP_PT_GREY:
				disabled_p ? IL_MSG_FOLLOWUP_GREY :
				both_p ? IL_MSG_FOLLOWUP_PT :
				IL_MSG_FOLLOWUP].pixmap;
	case 4: return fe_icons[disabled_p && both_p ?
			         IL_MSG_FOLLOWUP_AND_REPLY_PT_GREY :
				disabled_p ? IL_MSG_FOLLOWUP_AND_REPLY_GREY :
				both_p ? IL_MSG_FOLLOWUP_AND_REPLY_PT :
				IL_MSG_FOLLOWUP_AND_REPLY].pixmap;
	case 5: return fe_icons[disabled_p && both_p ?
			         IL_MSG_FORWARD_MSG_PT_GREY :
				disabled_p ? IL_MSG_FORWARD_MSG_GREY :
				both_p ? IL_MSG_FORWARD_MSG_PT :
				IL_MSG_FORWARD_MSG].pixmap;
	case 6: return fe_icons[disabled_p && both_p ?
				  IL_MSG_PREV_UNREAD_PT_GREY :
				disabled_p ? IL_MSG_PREV_UNREAD_GREY :
				both_p ? IL_MSG_PREV_UNREAD_PT :
				IL_MSG_PREV_UNREAD].pixmap;
	case 7: return fe_icons[disabled_p && both_p ?
			         IL_MSG_NEXT_UNREAD_PT_GREY :
				disabled_p ? IL_MSG_NEXT_UNREAD_GREY :
				both_p ? IL_MSG_NEXT_UNREAD_PT :
				IL_MSG_NEXT_UNREAD].pixmap;
	case 8: return fe_icons[disabled_p && both_p ?
			         IL_MSG_MARK_THREAD_READ_PT_GREY :
				disabled_p ? IL_MSG_MARK_THREAD_READ_GREY :
				both_p ? IL_MSG_MARK_THREAD_READ_PT :
				IL_MSG_MARK_THREAD_READ].pixmap;
	case 9: return fe_icons[disabled_p && both_p ?
			         IL_MSG_MARK_ALL_READ_PT_GREY :
				disabled_p ? IL_MSG_MARK_ALL_READ_GREY :
				both_p ? IL_MSG_MARK_ALL_READ_PT :
				IL_MSG_MARK_ALL_READ].pixmap;
	case 10:return fe_icons[disabled_p && both_p ? IL_ICON_PRINT_PT_GREY :
				disabled_p ? IL_ICON_PRINT_GREY :
				both_p ? IL_ICON_PRINT_PT :
				IL_ICON_PRINT].pixmap;
	case 11:return fe_icons[disabled_p && both_p ? IL_ICON_STOP_PT_GREY :
				disabled_p ? IL_ICON_STOP_GREY :
				both_p ? IL_ICON_STOP_PT :
				IL_ICON_STOP].pixmap;
/*	default: abort (); */
	default: fe_perror (context, "internal error: bogus news icon accessed"); break;
	}
      break;

    case MWContextMessageComposition:
      fe_init_compose_icons (context);
      switch (i)
	{
	/* sendOrSendLater */
	case 0:
	    if (fe_globalPrefs.queue_for_later_p)
		/* SendLater */
		return fe_icons[disabled_p && both_p ? IL_COMPOSE_SENDLATER_PT_GREY :
				disabled_p ? IL_COMPOSE_SENDLATER_GREY :
				both_p ? IL_COMPOSE_SENDLATER_PT :
				IL_COMPOSE_SENDLATER].pixmap;
	    else
		/* Send */
		return fe_icons[disabled_p && both_p ? IL_COMPOSE_SEND_PT_GREY :
				disabled_p ? IL_COMPOSE_SEND_GREY :
				both_p ? IL_COMPOSE_SEND_PT :
				IL_COMPOSE_SEND].pixmap;
	/* Quote */
	case 1:return fe_icons[disabled_p && both_p ? IL_COMPOSE_QUOTE_PT_GREY :
				disabled_p ? IL_COMPOSE_QUOTE_GREY :
				both_p ? IL_COMPOSE_QUOTE_PT :
				IL_COMPOSE_QUOTE].pixmap;
	/* Attach */
	case 2:return fe_icons[disabled_p &&
				both_p ? IL_COMPOSE_ATTACH_PT_GREY :
				disabled_p ? IL_COMPOSE_ATTACH_GREY :
				both_p ? IL_COMPOSE_ATTACH_PT :
				IL_COMPOSE_ATTACH].pixmap;
	/* AddressBook */
	case 3:return fe_icons[disabled_p &&
				both_p ? IL_COMPOSE_ADDRESSBOOK_PT_GREY :
				disabled_p ? IL_COMPOSE_ADDRESSBOOK_GREY :
				both_p ? IL_COMPOSE_ADDRESSBOOK_PT :
				IL_COMPOSE_ADDRESSBOOK].pixmap;

	case 4:
#ifndef NO_XFE_SA_ICON
	  return fe_SecurityAdvisorButtonPixmap(context, FALSE, FALSE);

	/* Stop */
	case 5:
#endif /* !NO_XFE_SA_ICON */
	  return fe_icons[disabled_p && both_p ? IL_ICON_STOP_PT_GREY :
				disabled_p ? IL_ICON_STOP_GREY :
				both_p ? IL_ICON_STOP_PT :
				IL_ICON_STOP].pixmap;
	default: fe_perror (context, "internal error: bogus compose icon accessed"); break;
	}
      break;



    default:
      abort ();
      break;
    }
  return 0;
}



void
XFE_ImageIcon (MWContext *context, int icon_number, void* client_data)
{
  int icon_width, icon_height;
  LO_ImageStruct *lo_image = (LO_ImageStruct *) client_data;
  struct fe_Pixmap *fep = lo_image->FE_Data;

  if (! fep)
      fep = fe_new_fe_Pixmap();

  /* If we're getting an icon for a backdrop image, something went wrong.
     So, clear the backdrop block to allow layout to continue. */
  if (lo_image && lo_image->image_attr &&
      (lo_image->image_attr->attrmask & LO_ATTR_BACKDROP))
    LO_ClearBackdropBlock (context, lo_image, False);

  if (icon_number >= IL_GOPHER_FIRST && icon_number <= IL_GOPHER_LAST)
    fe_init_gopher_icons (context);
  else if (icon_number >= IL_NEWS_FIRST && icon_number <= IL_NEWS_LAST)
    fe_init_news_icons (context);
  else if (icon_number >= IL_SA_FIRST && icon_number <= IL_SA_LAST)
    fe_init_sa_icons(context);
#ifdef EDITOR
  else if (icon_number >= IL_EDIT_FIRST && icon_number <= IL_EDIT_LAST)
    fe_init_editor_icons(context);
#endif /*EDITOR*/

  fep->type = fep_icon;
  fep->data.icon_number = icon_number;
  fep->bits_creator_p = False; /* unused in this case */

  if (icon_number == IL_IMAGE_DELAYED &&
      !CONTEXT_DATA (context)->delayed_images_p &&
      !fe_ImagesCantWork)
    {
      CONTEXT_DATA (context)->delayed_images_p = True;
      if (CONTEXT_DATA (context)->delayed_button)
	XtVaSetValues (CONTEXT_DATA (context)->delayed_button,
		       XmNsensitive, True, 0);
      if (CONTEXT_DATA (context)->delayed_menuitem)
	XtVaSetValues (CONTEXT_DATA (context)->delayed_menuitem,
		       XmNsensitive, True, 0);
    }

  if (fe_icons [icon_number].pixmap)
    {
      icon_width  = fe_icons [icon_number].width;
      icon_height = fe_icons [icon_number].height;
    }
  else if (fe_icons [IL_IMAGE_NOT_FOUND].pixmap)
    {
      icon_width  = fe_icons [IL_IMAGE_NOT_FOUND].width;
      icon_height = fe_icons [IL_IMAGE_NOT_FOUND].height;
    }
  else /* aaaaagh! */
    {
      icon_width  = 50;
      icon_height = 50;
    }

  /*
   * delayed icons for images with ALT text are wider by the
   * size of the text they contain.
   */
  if (icon_number == IL_IMAGE_DELAYED &&
      lo_image->alt &&
      lo_image->alt_len)
    {
      fe_Font font = fe_LoadFontFromFace (context, lo_image->text_attr,
					  &lo_image->text_attr->charset,
					  lo_image->text_attr->font_face,
					  lo_image->text_attr->size,
					  lo_image->text_attr->fontmask);
      char *str = (char *) lo_image->alt;
      int length = lo_image->alt_len;
      int remaining = length;
      int width = 0;
      int height = 0;
#define SUCKY_X_MAX_LENGTH 600

      if (!font)
      {
		return;
      }

      do
        {
          int ascent, descent;
          XCharStruct overall;
          int L = (remaining > SUCKY_X_MAX_LENGTH ? SUCKY_X_MAX_LENGTH :
                   remaining);

          FE_TEXT_EXTENTS (lo_image->text_attr->charset, font, str, L,
		&ascent, &descent, &overall);

          /* ascent and descent are per the font, not per this text. */
          if (ascent + descent > height)
                height = ascent + descent;
          if (ascent + overall.descent > height)
                height = ascent + overall.descent;
          width += overall.width;

          str += L;
          remaining -= L;
        }
      while (remaining > 0);

      icon_width += width;
      if (height > icon_height)
        icon_height = height;
      icon_width += (2 * DELAYED_ICON_BORDER) + (3 * DELAYED_ICON_PAD);
      icon_height += (2 * DELAYED_ICON_BORDER) + (2 * DELAYED_ICON_PAD);
    }

  /* Tell layout the size of the icon on the screen, but if an image
     has already been displayed in this LO_ImageStruct, don't reset
     its size. */
  if (!lo_image->width || !lo_image->height) {
      lo_image->width  = icon_width;
      lo_image->height = icon_height;
      LO_SetImageInfo (context, lo_image->ele_id, icon_width, icon_height);
  }

  /*
   * if this icon resulted in any way from an IMG tag that had
   * its own set of WIDTH and/or HEIGHT attributes, then it is possible
   * for layout to think it has already been displayed, while the
   * front end has actually never displayed it.  This catches that
   * case (image_display_begun = False means front end never displayed).
   */
  if ((fep->image_display_begun == False)&&
      (lo_image->image_attr->attrmask & LO_ATTR_DISPLAYED))
    {
       XFE_DisplayImage (context, FE_VIEW, lo_image);
    }
}

#ifdef EDITOR
GC fe_GetGCfromDW(Display*, Window, unsigned long, XGCValues*);

static void
fe_DrawPixmapAndMask(MWContext* context, Drawable window,
		     Pixmap pixmap, Pixmap mask,
		     Position x, Position y,
		     Dimension w, Dimension h)
{
    Widget    widget = CONTEXT_WIDGET(context);
    Display*  display = XtDisplay(widget);
    Pixmap    tmp_pixmap;
    XGCValues gcv;
    GC        gc;
    unsigned long flags;
    Visual*   visual;
    int       visual_depth;

    /*
     *    If there is a mask, merge in the background.
     */
    if (mask) {

        XtVaGetValues(widget, XmNvisual, &visual, 0);
	if (!visual)
	    visual = fe_globalData.default_visual;
#if 1
        visual_depth = fe_VisualDepth(display, visual);
#else
        visual_depth = fe_VisualPixmapDepth(display, visual);
#endif
	tmp_pixmap = XCreatePixmap(display, window, w, h, visual_depth);

	memset(&gcv, ~0, sizeof(gcv));

	if (CONTEXT_DATA(context)->backdrop_pixmap
	    &&
	    CONTEXT_DATA(context)->backdrop_pixmap != (Pixmap)~0) {

	    gcv.fill_style = FillTiled;
	    gcv.tile = CONTEXT_DATA(context)->backdrop_pixmap;
	    gcv.ts_x_origin = -(CONTEXT_DATA(context)->document_x + x);
	    gcv.ts_y_origin = -(CONTEXT_DATA(context)->document_y + y);
	    flags = GCTile|GCFillStyle|GCTileStipXOrigin|GCTileStipYOrigin;

	} else {
	     gcv.foreground = CONTEXT_DATA(context)->bg_pixel;;
	     flags = GCForeground;
	}
	gc = fe_GetGCfromDW(display, window, flags, &gcv);

	XFillRectangle(display, tmp_pixmap, gc, 0, 0, w, h);

	/*
	 *    Merge pixmap.
	 */
	memset(&gcv, ~0, sizeof (gcv));
	gcv.function = GXcopy;
	gcv.clip_mask = mask;
	gcv.clip_x_origin = 0;
	gcv.clip_y_origin = 0;
	flags = GCFunction|GCClipMask|GCClipXOrigin|GCClipYOrigin;
	gc = fe_GetGCfromDW(display, window, flags, &gcv);

	XCopyArea(display, pixmap, tmp_pixmap, gc, 0, 0, w, h, 0, 0);

	/*
	 *    Finally copy onto window.
	 */
	memset(&gcv, ~0, sizeof (gcv));
	gcv.function = GXcopy;
	flags = GCFunction;
	gc = fe_GetGCfromDW(display, window, flags, &gcv);

	XCopyArea(display, tmp_pixmap, window, gc, 0, 0, w, h, x, y);

	XFreePixmap(display, tmp_pixmap);
    } else {
        /*
	 *    Easy, just copy onto window.
	 */
        memset(&gcv, ~0, sizeof (gcv));
	gcv.function = GXcopy;
	flags = GCFunction;
	gc = fe_GetGCfromDW(display, window, flags, &gcv);

	XCopyArea(display, pixmap, window, gc, 0, 0, w, h, x, y);
    }
}
#endif /*EDITOR*/

void
fe_DrawIcon (MWContext *context, LO_ImageStruct *lo_image, int icon_number)
{
  Pixmap p = fe_icons [icon_number].pixmap;
  Pixmap m = fe_icons [icon_number].mask;
  Widget d = CONTEXT_DATA (context)->drawing_area;
  int x = lo_image->x + lo_image->x_offset
    + lo_image->border_width
    - CONTEXT_DATA (context)->document_x;
  int y = lo_image->y + lo_image->y_offset
    + lo_image->border_width
    - CONTEXT_DATA (context)->document_y;
  int w, h;
  unsigned long flags;

  if (!p)
    {
      icon_number = IL_IMAGE_NOT_FOUND;
      p = fe_icons [icon_number].pixmap;
    }

  w = fe_icons [icon_number].width;
  h = fe_icons [icon_number].height;

  /*
   * A delayed image with ALT text displays the icon, text, and a box instead
   * of just the icon.  I steal the code here from display text, there should
   * probably be some way to share code here.
   */
  if (icon_number == IL_IMAGE_DELAYED &&
      lo_image->alt &&
      lo_image->alt_len)
    {
      fe_Font font = fe_LoadFontFromFace (context, lo_image->text_attr,
					  &lo_image->text_attr->charset,
					  lo_image->text_attr->font_face,
					  lo_image->text_attr->size,
					  lo_image->text_attr->fontmask);
      char *str = (char *) lo_image->alt;
      int ascent, descent;
      int tx, ty;
      Boolean selected_p = False;
      XGCValues gcv;
      GC gc;
      memset (&gcv, ~0, sizeof (gcv));
      gcv.foreground = fe_GetPixel (context,
				    lo_image->text_attr->fg.red,
				    lo_image->text_attr->fg.green,
				    lo_image->text_attr->fg.blue);
      gcv.line_width = 1;
      gc = fe_GetGC (CONTEXT_DATA (context)->widget, GCForeground|GCLineWidth,
		     &gcv);
      /* beware: XDrawRectangle centers the line-thickness on the coords. */
      XDrawRectangle (XtDisplay (CONTEXT_DATA (context)->drawing_area),
		      XtWindow (CONTEXT_DATA (context)->drawing_area),
		      gc, x, y,
		      lo_image->width - DELAYED_ICON_BORDER,
		      lo_image->height - DELAYED_ICON_BORDER);
      x += (DELAYED_ICON_BORDER + DELAYED_ICON_PAD);
      y += (DELAYED_ICON_BORDER + DELAYED_ICON_PAD);

      if (!font)
      {
		return;
      }

        {
          XCharStruct overall;
          FE_TEXT_EXTENTS (lo_image->text_attr->charset, font, str, 1,
			&ascent, &descent, &overall);
        }
      tx = x + w + DELAYED_ICON_PAD;
      ty = y + (h / 2) - ((ascent + descent) / 2) + ascent;
      gcv.background = fe_GetPixel (context,
				    lo_image->text_attr->bg.red,
				    lo_image->text_attr->bg.green,
				    lo_image->text_attr->bg.blue);

      flags = (GCForeground | GCBackground);
      FE_SET_GC_FONT(lo_image->text_attr->charset, &gcv, font, &flags);

      gc = fe_GetGC (CONTEXT_DATA (context)->widget, flags, &gcv);

      if (CONTEXT_DATA (context)->backdrop_pixmap &&
	  /* This can only happen if something went wrong while loading
	     the background pixmap, I think. */
	  CONTEXT_DATA (context)->backdrop_pixmap != (Pixmap) ~0)
        {
	  GC gc2;
	  XGCValues gcv;
	  memset (&gcv, ~0, sizeof (gcv));
	  gcv.fill_style = FillTiled;
	  gcv.tile = CONTEXT_DATA (context)->backdrop_pixmap;
	  gcv.ts_x_origin = -CONTEXT_DATA (context)->document_x;
	  gcv.ts_y_origin = -CONTEXT_DATA (context)->document_y;
	  gc2 = fe_GetGC (d,
			  (GCTile | GCFillStyle |
			   GCTileStipXOrigin | GCTileStipYOrigin),
			  &gcv);
          XFillRectangle (XtDisplay (d), XtWindow (d), gc2,
			  x, y,
			  lo_image->width, lo_image->height);
        }

      if (CONTEXT_DATA (context)->backdrop_pixmap &&
	  /* This can only happen if something went wrong while loading
	     the background pixmap, I think. */
	  CONTEXT_DATA (context)->backdrop_pixmap != (Pixmap) ~0 &&
	  !selected_p)
        FE_DRAW_STRING (lo_image->text_attr->charset, XtDisplay (d),
		XtWindow (d), font, gc, tx, ty, (char *) lo_image->alt,
		lo_image->alt_len);
      else
        FE_DRAW_IMAGE_STRING (lo_image->text_attr->charset, XtDisplay (d),
		XtWindow (d), font, gc, tx, ty, (char *) lo_image->alt,
		lo_image->alt_len);
    }

  if (p) {
#ifdef EDITOR
     fe_DrawPixmapAndMask(context,
			  XtWindow(CONTEXT_DATA(context)->drawing_area),
			  p, m,
			  x, y, w, h);
#else
      XGCValues gcv;
      unsigned long flags;
      GC gc;
      memset (&gcv, ~0, sizeof (gcv));
      gcv.function = GXcopy;
      flags = GCFunction;

      if (m) /* #### no need for this if using default solid bg color */
	{
	  gcv.clip_mask = m;
	  gcv.clip_x_origin = x;
	  gcv.clip_y_origin = y;
	  flags |= (GCClipMask | GCClipXOrigin | GCClipYOrigin);
	}

      gc = fe_GetGC (d, flags, &gcv);
      XCopyArea (XtDisplay (d), p, XtWindow (d), gc, 0, 0, w, h, x, y);
#endif /*EDITOR*/
  } else { /* aaaaagh! */
      fe_DrawShadows(context,
		     XtWindow(CONTEXT_DATA(context)->drawing_area),
		     x, y, 50, 50, 2, XmSHADOW_OUT);
  }
}

/*
 * the fe_icons array is only local to this module.  We need
 * a way to get the size of an icon form another module.
 */
void
fe_IconSize (int icon_number, long *width, long *height)
{
  *width = (long)fe_icons [icon_number].width;
  *height = (long)fe_icons [icon_number].height;
}


void
fe_DrawLogo (MWContext *context, Boolean tick_p, Boolean large_p)
{
  Widget widget = CONTEXT_DATA (context)->logo;
  XGCValues gcv;
  GC gc;
  int j;
  int x, y;
  struct fe_icon_data *anim;
  int id0;
  int frames;
  int frame;
  Boolean rest_p;
  if (! widget) return;
  anim = fe_which_anim (large_p, &id0, &frames, &rest_p);
  if (rest_p)
    frames--;
  if (tick_p)
    CONTEXT_DATA(context)->logo_frame =
      ((CONTEXT_DATA(context)->logo_frame+1) % frames);
  frame = CONTEXT_DATA(context)->logo_frame;
  if (tick_p && rest_p)
    frame++;
  j = frame + id0;
  if (! fe_icons [j].pixmap)
    fe_LogoPixmap (context, 0, 0, large_p);
  x = (widget->core.width - fe_icons [j].width) / 2;
  y = (widget->core.height - fe_icons [j].height) / 2;
  memset (&gcv, ~0, sizeof (gcv));
  gcv.function = GXcopy;
  gc = fe_GetGC (CONTEXT_DATA (context)->widget, GCFunction, &gcv);
  XCopyArea (XtDisplay (widget), fe_icons [j].pixmap, XtWindow (widget), gc,
	     0, 0, fe_icons [j].width, fe_icons [j].height, x, y);
#if 1
  /* Now store this pixmap in the button, so that when it is pressed or
     exposed, the right one gets drawn by the Motif PushB expose handler. */
  if (tick_p)
    {
      XmPushButtonWidget b = (XmPushButtonWidget) widget;
      b->label.pixmap = fe_icons [j].pixmap;
      b->label.pixmap_insen = fe_icons [j].pixmap;
      b->pushbutton.arm_pixmap = fe_icons [j].pixmap;
      b->pushbutton.unarm_pixmap = fe_icons [j].pixmap;
    }
#endif
}

void
fe_ResetLogo (MWContext *context, Boolean large_p)
{
  Dimension w, h;
  Widget widget = CONTEXT_DATA (context)->logo;
  Pixmap pixmap;
  int id0;
  int frames;
  Boolean rest_p;
  if (!widget)
    return;
  fe_which_anim (large_p, &id0, &frames, &rest_p);
  if (rest_p)
    CONTEXT_DATA(context)->logo_frame = 0;
  pixmap = fe_LogoPixmap (context, &w, &h, large_p);
  XtVaSetValues (widget,
		 XmNlabelPixmap, pixmap,
		 XmNlabelInsensitivePixmap, pixmap,
		 0);
  fe_DrawLogo (context, False, large_p);
}


#define XFE_ABOUT_FILE		0
#define XFE_SPLASH_FILE		1
#define XFE_LICENSE_FILE	2
#define XFE_MAILINTRO_FILE	3
#define XFE_PLUGIN_FILE		4

struct { char *name; char *str; } fe_localized_files[] = {
	{ "about",	NULL },
	{ "splash",	NULL },
	{ "license",	NULL },
	{ "mail.msg",	NULL },
	{ "plugins",	NULL }
};


static char *
fe_get_localized_file(int which)
{
	FILE		*f;
	char		file[512];
	char		*p;
	char		*ret;
	int		size;

	ret = fe_localized_files[which].str;
	if (ret)
	{
		return ret;
	}

	PR_snprintf(file, sizeof (file), "%s/%s", fe_get_app_dir(fe_display),
		fe_localized_files[which].name);
	f = fopen(file, "r");
	if (!f)
	{
		return NULL;
	}

	size = 20000;
	ret = malloc(size + 1);
	if (!ret)
	{
		fclose(f);
		return NULL;
	}
	size = fread(ret, 1, size, f);
	fclose(f);
	ret[size] = 0;

	p = strdup(ret);
	free(ret);
	if (!p)
	{
		return NULL;
	}
	ret = p;

	fe_localized_files[which].str = ret;

	return ret;
}


void *
FE_AboutData (const char *which,
	      char **data_ret, int32 *length_ret, char **content_type_ret)
{
  char *which2;
  unsigned char *tmp;
  static XP_Bool ever_loaded_map = FALSE;
  which2 = strdup (which);
  for (tmp = (unsigned char *) which2; *tmp; tmp++) *tmp += 69;

  if (!strcmp (which2, "\261\264\254\264"))		/* "logo" */
    {
      /* Note, this one returns a read-only string. */
      *data_ret = (char *) biglogo_gif;
      *length_ret = sizeof (biglogo_gif) - 1;
      *content_type_ret = IMAGE_GIF;
    }
  else if (!strcmp (which2, "\265\255\264\271\264"))	/* "photo" */
    {
      if (!ever_loaded_map)
	{
	  *data_ret = 0;
	  *length_ret = 0;
	  *content_type_ret = 0;
	  return 0;
	}
      /* Note, this one returns a read-only string. */
      *data_ret = (char *) photo_jpg;
      *length_ret = sizeof (photo_jpg) - 1;
      *content_type_ret = IMAGE_JPG;
    }
  else if (!strcmp (which2, "\255\276\265\252"))	/* "hype" */
    {
      /* Note, this one returns a read-only string. */
      *data_ret = hype_au;
      *length_ret = sizeof (hype_au) - 1;
      *content_type_ret = AUDIO_BASIC;
    }
#ifdef HAVE_SECURITY
  else if (!strcmp (which2, "\267\270\246\261\264\254\264"))	/* "rsalogo" */
    {
      /* Note, this one returns a read-only string. */
      *data_ret = (char *) rsalogo_gif;
      *length_ret = sizeof (rsalogo_gif) - 1;
      *content_type_ret = IMAGE_GIF;
    }
#endif
#ifdef JAVA
  else if (!strcmp (which2, "\257\246\273\246\261\264\254\264")) /* "javalogo" */
    {
      /* Note, this one returns a read-only string. */
      *data_ret = (char *) javalogo_gif;
      *length_ret = sizeof (javalogo_gif) - 1;
      *content_type_ret = IMAGE_GIF;
    }
#endif
#ifdef HAVE_QUICKTIME
  else if (!strcmp (which2, "\266\271\261\264\254\264")) /* "qtlogo" */
    {
      /* Note, this one returns a read-only string. */
      *data_ret = (char *) qt_logo_gif;
      *length_ret = sizeof (qt_logo_gif) - 1;
      *content_type_ret = IMAGE_GIF;
    }
#endif
  else if (!strcmp (which2, "\251\252\272\271\270\250\255"))	/* "deutsch" */
    {
      /* Note, this one returns a read-only string. */
      *data_ret = (char *) deutsch_gif;
      *length_ret = sizeof (deutsch_gif) - 1;
      *content_type_ret = IMAGE_GIF;
    }
  else if ((!strcmp(which2, "\253\267\246\263\250\246\256\270"))/* "francais" */
    || (!strcmp (which2, "\253\267\246\263\054\246\256\270")))/* "franc,ais" */
    {
      /* Note, this one returns a read-only string. */
      *data_ret = (char *) francais_gif;
      *length_ret = sizeof (francais_gif) - 1;
      *content_type_ret = IMAGE_GIF;
    }
  else if ((!strcmp (which2, "\263\256\255\264\263\254\264"))	/* "nihongo" */
    || (!strcmp (which2, "\013\101\020\041\375\061"))	/* EUC "nihongo" */
    || (!strcmp (which2, "\330\077\333\300\321\057")))	/* "nihongo" */
    {
      /* Note, this one returns a read-only string. */
      *data_ret = (char *) nihongo_gif;
      *length_ret = sizeof (nihongo_gif) - 1;
      *content_type_ret = IMAGE_GIF;
    }
#ifdef EDITOR
  else if ((!strcmp (which2, "\252\251\256\271\264\267")) /* "editor" */
    || (!strcmp (which2, "\254\264\261\251"))) /* "gold" remove for 4.X */
    {
      /* Note, this one returns a read-only string. */
      *data_ret = (char *)editor_gif;
      *length_ret = sizeof(editor_gif) - 1;
      *content_type_ret = IMAGE_GIF;
    }
  else if (!strcmp(which, "editfilenew"))
    {
      /* Magic about: for Editor new (blank) window */
      *data_ret = strdup("\240");
      *length_ret = strlen (*data_ret);
      *content_type_ret = TEXT_MDL; 
    }
#endif
  else
    {
      static char *a = 0;
      char *type = TEXT_HTML;
      Boolean do_PR_snprintf = False;
      Boolean do_rot = True;
      logomatic = 0;
      if (a) free (a);
      a = 0;
      if (!strcmp (which2, ""))
	{
	  do_PR_snprintf = True;
	  a = fe_get_localized_file(XFE_ABOUT_FILE);
	  if (a)
	    {
	      a = strdup(a);
	      do_rot = False;
	    }
	  else
	    {
              /* jwz: stdup() is a macro on Red Hat 6.1 */

	      a =
#ifdef JAVA
#	if defined(EDITOR) && defined(EDITOR_UI)
#		          include "../../l10n/us/xp/about-gold-java.h"
#	else
#		          include "../../l10n/us/xp/about-java.h"
#	endif
#else
#	if defined(EDITOR) && defined(EDITOR_UI)
#		          include "../../l10n/us/xp/about-gold.h"
#	else
#		          include "../../l10n/us/xp/about.h"
#	endif
#endif
                ;
              a = strdup (a);
	    }
	}
      else if (!strcmp (which2, "\270\265\261\246\270\255"))	/* "splash" */
	{
	  do_PR_snprintf = True;
	  a = fe_get_localized_file(XFE_SPLASH_FILE);
	  if (a)
	    {
	      a = strdup(a);
	      do_rot = False;
	    }
	  else
	    {
              /* jwz: stdup() is a macro on Red Hat 6.1 */

	      a =
#ifdef JAVA
#	if defined(EDITOR) && defined(EDITOR_UI)
#		          include "../../l10n/us/xp/splash-gold-java.h"
#	else
#		          include "../../l10n/us/xp/splash-java.h"
#	endif
#else
#	if defined(EDITOR) && defined(EDITOR_UI)
#		          include "../../l10n/us/xp/splash-gold.h"
#	else
#		          include "../../l10n/us/xp/splash.h"
#	endif
#endif
                ;
              a = strdup (a);
	    }
	}
      else if (!strcmp (which2, "\166\176\176\171")) /* "1994" */
	{
	  logomatic = 1;
	  ever_loaded_map = TRUE;

          /* jwz: stdup() is a macro on Red Hat 6.1 */
          a =
#		      include "../../l10n/us/xp/authors2.h"
            ;
          a = strdup (a);
	}
#if 0		/* Other FEs dont want to implement this :-( -dp */
      else if (!strcmp (which2, "\166\176\176\172")) /* "1995" */
	{
          /* jwz: stdup() is a macro on Red Hat 6.1 */
          a =
#		      include "../../l10n/us/xp/authors.h"
            ;
          a = strdup (a);
	}
#endif /* 0 */
      else if (!strcmp (which2,"\261\256\250\252\263\270\252"))	/* "license" */
	{
	  type = TEXT_PLAIN;
	  a = fe_get_localized_file(XFE_LICENSE_FILE);
	  if (a)
	    {
	      a = strdup(a);
	      do_rot = False;
	    }
	  else
	    {
	      a = strdup (fe_LicenseData);
	    }
	}

      else if (!strcmp (which2,"\262\264\277\256\261\261\246")) /* "mozilla" */
	{
	  logomatic = 1;
          /* jwz: stdup() is a macro on Red Hat 6.1 */
          a =
#		      include "../../l10n/us/xp/mozilla.h"
            ;
          a = strdup (a);
	}
      else if (!strcmp (which2,
			"\262\246\256\261\256\263\271\267\264"))/*"mailintro"*/
	{
	  type = MESSAGE_RFC822;
     a = fe_get_localized_file(XFE_MAILINTRO_FILE );
	  if (a)
	  {
        a = strdup( a );
        do_rot = False;
     }
     else
     {
          /* jwz: stdup() is a macro on Red Hat 6.1 */
          a =
#		      include "../../l10n/us/xp/mail.h"
            ;
          a = strdup (a);
     }
	}

      else if (!strcmp (which2, "\247\261\246\263\260"))	/* "blank" */
	a = strdup ("");
      else if (!strcmp (which2, "\250\272\270\271\264\262"))	/* "custom" */
        {
          do_rot = False;
	  a = ekit_AboutData();
        }


      else if (!strcmp (which2, "\265\261\272\254\256\263\270"))	/* "plugins" */
	{
	  a = fe_get_localized_file(XFE_PLUGIN_FILE);
	  if (a)
	    {
	      a = strdup(a);
	      do_rot = False;
	    }
	  else
	    {
          /* jwz: stdup() is a macro on Red Hat 6.1 */
          a =
#		          include "../../l10n/us/xp/aboutplg.h"
            ;
          a = strdup (a);
	    }
	}



      else
	a = strdup ("\234\255\246\271\250\255\252\274\145\271\246"
		    "\261\260\256\263\154\145\154\247\264\272\271"
		    "\161\145\234\256\261\261\256\270\204");
      if (a)
	{
	  if (do_rot)
	    {
	      for (tmp = (unsigned char *) a; *tmp; tmp++) *tmp -= 69;
	    }

	  if (do_PR_snprintf)
	    {
	      char *a2;
	      int len;
	      char *val = (char*)malloc(strlen(fe_version_and_locale) + 20);
#ifdef HAVE_SECURITY /* jwz */
	      char *s0 = XP_GetString(XFE_SECURITY_WITH);
	      char *s1 = XP_SecurityVersion(1);
	      char *s2 = XP_SecurityCapabilities();
	      char *ss;
	      len = strlen(s0)+strlen(s1)+strlen(s2);
	      ss = (char*) malloc(len);
	      PR_snprintf(ss, len, s0, s1, s2);
#else
	      char *ss = XP_GetString(XFE_SECURITY_DISABLED);
#endif

	      strcpy(val, fe_version_and_locale);
	      strcat(val, "+S/MIME");

	      len = strlen(a) + strlen(val) +
                        strlen(val) + strlen(ss);
	      a2 = (char *) malloc(len);
	      PR_snprintf (a2, len, a,
		       val,
		       val,
		       ss
		       );
#ifdef HAVE_SECURITY  /* jwz */
	      free (s2);
	      free (ss);
#endif
	      free(val);
	      free (a);
	      a = a2;
	    }

	  *data_ret = a;
	  *length_ret = strlen (*data_ret);
	  *content_type_ret = type;
	}
      else
	{
	  *data_ret = 0;
	  *length_ret = 0;
	  *content_type_ret = 0;
	}
    }
  free (which2);
  return NULL; /* nothing to free later ??? */
}

