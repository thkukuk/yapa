/* Copyright (c) 2006, 2007, 2008, 2009, 2018 Thorsten Kukuk
   Author: Thorsten Kukuk <kukuk@thkukuk.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2 as
   published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <signal.h>
#include <getopt.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include "main.h"

int debug_flag = 0;
int force_html_flag = 0;
int force_nail_flag = 0;

/* Print the version information.  */
static void
print_version (const char *program, const char *years)
{
  fprintf (stdout, "%s (%s) %s\n", program, PACKAGE, VERSION);
  fprintf (stdout, _("\
Copyright (C) %s Thorsten Kukuk.\n\
This is free software; see the source for copying conditions.  There is NO\n\
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\
"), years);
  /* fprintf (stdout, _("Written by %s.\n"), "Thorsten Kukuk"); */
}

static void
print_error (const char *program)
{
  fprintf (stderr,
           _("Try `%s --help' for more information.\n"),
           program);
}

static void
print_help (const char *program)
{
  fprintf (stdout, _("%s - Yet Another Photo Album\n"), program);
  fprintf (stdout, _("Usage: %s [options] directory\n\n"), program);

  fputs (_("  -d, --debug       Print debug messages\n"), stdout);
  fputs (_("  -f, --force       Recreate all html pages and thumb images\n"),
	 stdout);
  fputs (_("      --force-html  Recreate all html pages\n"), stdout);
  fputs (_("      --force-nails Recreate all thumb imabes\n"), stdout);
  fputs (_("  -v, --version     Print program version\n"), stdout);
  fputs (_("      --help        Give this help list\n"), stdout);
}

void
yapa_oom (void)
{
  fprintf (stderr, "Running out of memory, aborting ...\n");
  abort ();
}

/* Go recursive through all directories and create list
   of images in every directroy. */
static int
go_through_dir (const char *directory, dir_l *dirs)
{
  DIR *dir = opendir (directory);
  struct dirent *d;
  struct stat st;
  int found_meta_data = 0;
  char *linksfile;

  if (dir == NULL)
    return 1;

  if (!debug_flag)
    printf (_("Import data from %s\n"), directory);

  if (asprintf (&linksfile, "%s/yapa/links", directory) < 0)
    yapa_oom ();

  if (access (linksfile, R_OK) == 0)
    {
      /* open file with links to other images outside this directory */
      FILE *fp = fopen (linksfile, "r");

      if (!debug_flag)
	printf (_("Import data from %s/yapa/links\n"), directory);

      if (fp != NULL)
	{
	  char *buf = NULL;
	  size_t buflen = 0;

	  while (!feof (fp))
	    {
	      char *cp, *path;
	      ssize_t n = getline (&buf, &buflen, fp);

	      cp = buf;

	      if (n < 1)
		break;

	      while (isspace ((int)*cp))    /* remove spaces and tabs */
		++cp;
	      if (*cp == '\0')        /* ignore empty lines */
		continue;

	      n = strlen (cp) - 1;
	      if (cp[n] == '\n') /* remove trailing newline */
		cp[n--] = '\0';
	      while (n > 0 && isspace ((int)cp[n]))
		cp[n--] = '\0';


	      if (asprintf (&path, "%s/%s", directory, cp) < 0)
		yapa_oom ();

	      if (stat (path, &st) == 0)
		{
		  if ((strcasecmp (&cp[strlen (cp) - 4],
				   ".jpg") == 0) ||
		      (strcasecmp (&cp[strlen (cp) - 4],
				   ".png") == 0))
		    {
		      char *srcdir, *newname;

		      newname = strrchr (cp, '/');
		      if (newname == NULL)
			{
			  newname = cp;
			  srcdir = strdup (directory);
			}
		      else
			{
			  *newname++ = '\0';

			  if (asprintf (&srcdir, "%s/%s",
					directory, cp) < 0)
			    yapa_oom ();
			}

		      if (debug_flag)
			printf ("==> ");

		      add_image (dirs, srcdir, directory, newname, st.st_mtime);
		      free (srcdir);
		    }
		  else
		    if (debug_flag)
		      printf ("==> ignored\n");
		}
	      else
		fprintf (stderr, "WARNING: file %s not found, ignoring\n", cp);

	      free (path);
	    }

	  fclose (fp);

	  if (!debug_flag)
	    printf (_("Finished importing data from links\n"));
	}
    }

  free (linksfile);

  while ((d = readdir (dir)) != NULL)
    {
      char *buf;

      if (debug_flag)
	printf ("FOUND: %s ", d->d_name);
      if (d->d_name[0] == '.')
	{
	  if (debug_flag)
	    printf ("==> ignored\n");
	  continue;
	}

      if (asprintf (&buf, "%s/%s", directory, d->d_name) < 0)
	yapa_oom ();

      stat (buf, &st);
      if (S_ISDIR(st.st_mode))
	{
	  if (strcmp (d->d_name, "yapa") == 0)
	    {
	      if (debug_flag)
		printf ("==> ignored\n");
	      found_meta_data = 1;
	    }
	  else if (strcmp (d->d_name, "picfolio") == 0)
	    {
	      if (debug_flag)
		printf ("==> ignored\n");
	    }
	  else if (strcmp (d->d_name, "GPXViewer") == 0)
	    {
	      if (debug_flag)
		printf ("==> ignored\n");
	    }
	  else
	    {
	      dir_l *subdir;
	      if (debug_flag)
		printf ("==> Go through Subdirectory\n");

	      subdir = add_dir (&dirs->subdirs, directory, d->d_name);
	      subdir->parentdir = dirs;
	      subdir->config = get_config (subdir, dirs);

	      go_through_dir (buf, subdir);
	      /* Directory is empty, so don't add it */
	      if (subdir->images == NULL && subdir->subdirs == NULL)
		{
		  subdir = get_and_delete_dir_entry (&dirs->subdirs,
						     d->d_name);
		  free (subdir->path);
		  free (subdir->name);
		  free (subdir);
		}
	    }
	}
      else if (S_ISREG(st.st_mode))
	{
	  if (strcasecmp (&d->d_name[strlen (d->d_name) - 4], ".jpg") == 0)
	    {
	      if (debug_flag)
		printf ("==> ");
	      add_image (dirs, directory, directory, d->d_name, st.st_mtime);
	    }
	  else if (strcasecmp (&d->d_name[strlen (d->d_name) - 4], ".png") == 0)
	    {
	      if (debug_flag)
		printf ("==> ");
	      add_image (dirs, directory, directory, d->d_name, st.st_mtime);
	    }
	  else if (strcasecmp (&d->d_name[strlen (d->d_name) - 5], ".html") == 0)
	    {
	      /* ignore index-*.html files */
	      /* XXX yes, this means we will not delete index-*.html files */
	      if (strncmp (d->d_name, "index-", 6) != 0)
		{
		  if (debug_flag)
		    printf ("==> ");
		  add_html (&dirs->html, directory, d->d_name, st.st_mtime);
		}
	    }
	  else if (strcasecmp (&d->d_name[strlen (d->d_name) - 4], ".txt") == 0)
	    {
	      if (debug_flag)
		printf ("==> ");
	      add_txt (&dirs->texts, directory, d->d_name, st.st_mtime);
	    }
	  else if (strcasecmp (&d->d_name[strlen (d->d_name) - 4], ".gpx") == 0)
	    {
	      if (debug_flag)
		printf ("==> ");
	      add_gpx (&dirs->gpx, directory, d->d_name, st.st_mtime);
	    }
	  else if (debug_flag)
	    printf ("==> ignored\n");
	}
      else if (debug_flag)
	printf ("==> ignored\n");
      free (buf);
    }

  closedir (dir);

  if (found_meta_data == 0 &&
      (dirs->subdirs != NULL || dirs->images != NULL))
    {
      char *cp;

      if (asprintf (&cp, "%s/yapa", directory) < 0)
	yapa_oom ();
      mkdir (cp, 0755);
      free (cp);
      if (dirs->images)
	{
	  if (asprintf (&cp, "%s/yapa/midnails", directory) < 0)
	    yapa_oom ();
	  mkdir (cp, 0755);
	  free (cp);
	  if (asprintf (&cp, "%s/yapa/thumbnails", directory) < 0)
	    yapa_oom ();
	  mkdir (cp, 0755);
	  free (cp);
	}
    }
  return 0;
}

int
main (int argc, char *argv[])
{
  const char *program = "yapa";

#ifdef ENABLE_NLS
  setlocale(LC_ALL, "");
  bindtextdomain(PACKAGE, LOCALEDIR);
  textdomain(PACKAGE);
#endif

  openlog (program, LOG_ODELAY | LOG_PID, LOG_AUTHPRIV);

  while (1)
    {
      int c;
      int option_index = 0;
      static struct option long_options[] = {
	{"debug",       no_argument,       NULL, 'd' },
	{"force",       no_argument,       NULL, 'f' },
	{"force-html",  no_argument,       NULL, 501 },
	{"force_html",  no_argument,       NULL, 501 },
	{"force-nails", no_argument,       NULL, 502 },
	{"force_nails", no_argument,       NULL, 502 },
	{"help",        no_argument,       NULL, 500 },
        {"version",     no_argument,       NULL, 'v' },
        {NULL,          0,                 NULL, '\0'}
      };

      c = getopt_long (argc, argv, "dfv",
                       long_options, &option_index);

      if (c == (-1))
        break;
      switch (c)
	{
	case 'd':
	  debug_flag = 1;
	  break;
	case 'f':
	  force_html_flag = 1;
	  force_nail_flag = 1;
	  break;
	case 501:
	  force_html_flag = 1;
	  break;
	case 502:
	  force_nail_flag = 1;
	  break;
        case 'v':
          print_version (program, "2007");
          return 0;
	case 500:
          print_help (program);
          return 0;
	default:
	  print_error (program);
	  return 1;
	}
    }

  argc -= optind;
  argv += optind;

  if (argc != 1)
    {
      fprintf (stderr, _("%s: Wrong number of arguments.\n"), program);
      print_error (program);
      return 1;
    }

  char *root_path = find_root_dir (argv[0]);
  if (root_path == NULL)
    {
      root_path = realpath (argv[0], NULL);
      printf (_("Create root configuration in %s\n"), root_path);
      create_root_config (root_path);
    }

  dir_l *rootdir = NULL;

  add_dir (&rootdir, root_path, NULL);
  get_root_config (rootdir);
  rootdir->config = get_config (rootdir, NULL);
  if (go_through_dir (root_path, rootdir) != 0)
    abort ();

  free (root_path);

  update_html (rootdir);

  free_dir (&rootdir);

  return 0;
}
