#define RESOURCE_STR
#include "xfe_err.h"

#include "mozilla.h"
#include "xfe.h"


char *XP_GetBuiltinString(int16 i);


static XrmDatabase	database;


char *
XP_GetString(int16 i)
{
	static char	buf[128];
	static char	class[128];
	char		*ret;
	char		*type;
	XrmValue	value;

#ifdef notdef
	char		*file;
	if (!database)
	{
		char	*path;
		Boolean	deallocate = False;

		database = XrmGetStringDatabase("");

		if (!(path = getenv("XUSERFILESEARCHPATH")))
		{
			char	*old_path;
			char	*homedir;

			homedir = getenv("HOME");
			if (!homedir)
			{
				homedir = "/";
			}
			if (!(old_path = getenv("XAPPLRESDIR")))
			{
				char *path_default =
					"%s/%%L/%%N%%C%%S"
					":%s/%%l/%%N%%C%%S"
					":%s/%%N%%C%%S"
					":%s/%%L/%%N%%S"
					":%s/%%l/%%N%%S"
					":%s/%%N%%S";

				if (!(path = malloc(6*strlen(homedir) +
					strlen(path_default))))
				{
					_XtAllocError(NULL);
				}
				(void) PR_snprintf(path, sizeof (path),
						   path_default, homedir,
						   homedir, homedir, homedir,
						   homedir, homedir);
			}
			else
			{
				char *path_default =
					"%s/%%L/%%N%%C%%S"
					":%s/%%l/%%N%%C%%S"
					":%s/%%N%%C%%S"
					":%s/%%N%%C%%S"
					":%s/%%L/%%N%%S"
					":%s/%%l/%%N%%S"
					":%s/%%N%%S"
					":%s/%%N%%S";

				if (!(path = malloc( 6*strlen(old_path) +
					2*strlen(homedir) + strlen(path_default))))
				{
					_XtAllocError(NULL);
				}
				(void) PR_snprintf(path, sizeof (path),
						   path_default, old_path,
						   old_path, old_path,
						   homedir, old_path, old_path,
						   old_path, homedir );
			}
			deallocate = True;
		}

		file = (char *) XtResolvePathname
		(
			fe_display,
			NULL,
			NULL,
			".dat",
			path,
			NULL,
			0,
			NULL
		);
		if (file)
		{
			(void) XrmCombineFileDatabase(file, &database, False);
			XtFree(file);
		}

		if (deallocate)
		{
			free(path);
		}


		file = (char *) XtResolvePathname
		(
			fe_display,
			"app-defaults",
			NULL,
			".dat",
			NULL,
			NULL,
			0,
			NULL
		);
		if (file)
		{
			(void) XrmCombineFileDatabase(file, &database, False);
			XtFree(file);
		}
	}
#endif /* notdef */

	(void) PR_snprintf(buf, sizeof (buf),
			"%s.strings.%d", fe_progclass, i + RES_OFFSET);
	(void) PR_snprintf(class, sizeof (class), 
			"%s.Strings.Number", fe_progclass);
	if (fe_display && ((database = XtDatabase(fe_display))) &&
		XrmGetResource(database, buf, class, &type, &value))
	{
		return value.addr;
	}

	if ((ret = mcom_cmd_xfe_xfe_err_h_strings (i + RES_OFFSET)))
	{
		return ret;
	}

	return XP_GetBuiltinString(i);
}
