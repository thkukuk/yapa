/* Copyright (c) 2006, 2007, 2009 Thorsten Kukuk
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "main.h"

static config_t default_config = {
  subdirformat: 0,
  subdircols: 5,
  imagecols: 5,
  imagerows: 3,
  thumbnail: 128,
  midnail: 640,
  sort_dir: 1,
  sort_img: 1
};

config_t
get_config (dir_l *dir, dir_l *parent)
{
  char *filename;
  config_t ret;

  if (parent == NULL)
    ret = default_config;
  else
    ret = parent->config;

  if (dir->name == NULL)
    {
      if (asprintf (&filename, "%s/yapa/config", dir->path) < 0)
	yapa_oom ();
    }
  else
    {
      if (asprintf (&filename, "%s/%s/yapa/config", dir->path, dir->name) < 0)
	yapa_oom ();
    }

  /* open old file with order and labels */
  FILE *fp = fopen (filename, "r");
  free (filename);
  if (fp != NULL)
    {
      char *buf = NULL;
      size_t buflen = 0;

      while (!feof (fp))
	{
	  char *cp, *value;
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

	  cp = strtok (cp, "\t :=");
	  value = strtok (NULL, "\t :=");
	  if (value)
	    {
	      /* XXX better error checking! */
	      if (strcasecmp (cp, "subdir-format") == 0)
		ret.subdirformat = atoi (value);
	      else if (strcasecmp (cp, "subdir-columns") == 0)
		ret.subdircols = atoi (value);
	      else if (strcasecmp (cp, "image-columns") == 0)
		ret.imagecols = atoi (value);
	      else if (strcasecmp (cp, "image-rows") == 0)
		ret.imagerows = atoi (value);
	      else if (strcasecmp (cp, "thumbnail-size") == 0)
		ret.thumbnail = atoi (value);
	      else if (strcasecmp (cp, "midnail-size") == 0)
		ret.midnail = atoi (value);
	      else if (strcasecmp (cp, "sort-directory") == 0)
		ret.sort_dir = atoi (value);
	      else if (strcasecmp (cp, "sort-images") == 0)
		ret.sort_img = atoi (value);
	      else
		fprintf (stderr, "WARNING: unknown option %s\n", cp);
	    }
	}

      free (buf);
      fclose (fp);
    }

  return ret;
}

void
get_root_config (dir_l *dir)
{
  char *filename;

  if (asprintf (&filename, "%s/yapa/root", dir->path) < 0)
    yapa_oom ();

  /* open old file with order and labels */
  FILE *fp = fopen (filename, "r");
  free (filename);
  if (fp != NULL)
    {
      char *buf = NULL;
      size_t buflen = 0;

      while (!feof (fp))
	{
	  char *cp, *key;
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

	  key = strsep (&cp, "\t :=");
	  while (cp != NULL &&
		 (*cp == '\t' || *cp == ' ' || *cp == ':' || *cp == '='))
	    ++cp;
	  if (key && cp)
	    {
	      /* XXX better error checking! */
	      if (strcasecmp (key, "gallery-name") == 0)
		dir->label = strdup (cp);
	      else
		fprintf (stderr, "WARNING: unknown option %s\n", key);
	    }
	}
      free (buf);
      fclose (fp);
    }

  return;
}

void
create_root_config (const char *rootdir)
{
  FILE *fp;
  char *cp;

  if (asprintf (&cp, "%s/yapa", rootdir) < 0)
    yapa_oom ();
  mkdir (cp, 0755);
  free (cp);

  if (asprintf (&cp, "%s/yapa/root", rootdir) < 0)
    yapa_oom ();

  fp = fopen (cp, "w");
  if (fp == NULL)
    {
      fprintf (stderr, _("ERROR: Cannot create %s: %m\n"), cp);
    }
  else
    {
      fprintf (fp, "gallery-name=Photo Gallery\n");
      fclose (fp);
    }
  free (cp);

  if (asprintf (&cp, "%s/yapa/config", rootdir) < 0)
    yapa_oom ();

  fp = fopen (cp, "w");
  if (fp == NULL)
    {
      fprintf (stderr, _("ERROR: Cannot create %s: %m\n"), cp);
    }
  else
    {
      fprintf (fp, "subdir-format=%d\n", default_config.subdirformat);
      fprintf (fp, "subdir-columns=%d\n", default_config.subdircols);
      fprintf (fp, "image-columns=%d\n", default_config.imagecols);
      fprintf (fp, "image-rows=%d\n", default_config.imagerows);
      fprintf (fp, "thumbnail-size=%d\n", default_config.thumbnail);
      fprintf (fp, "midnail-size=%d\n", default_config.midnail);
      fprintf (fp, "sort-directory=%d\n", default_config.sort_dir);
      fprintf (fp, "sort-images=%d\n", default_config.sort_img);
      fclose (fp);
    }
  free (cp);
}
