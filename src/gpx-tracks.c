/* Copyright (c) 2018 Thorsten Kukuk
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

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

#include "main.h"


gpx_l *
add_gpx (gpx_l **descr, const char *path,
	 const char *filename, time_t mtime)
{
  if (debug_flag)
    printf ("ADD GPX Track: %s\n", path);

  if (*descr == NULL)
    {
      *descr = calloc (1, sizeof (gpx_l));
      if (filename != NULL)
	(*descr)->name = strdup (filename);
      (*descr)->path = strdup (path);
      (*descr)->mtime = mtime;
      return *descr;
    }
  else
    {
      gpx_l *ptr, *new = calloc (1, sizeof (gpx_l));

      ptr = *descr;
      while (ptr->next != NULL)
	ptr = ptr->next;

      ptr->next = new;
      new->prev = ptr;
      if (filename != NULL)
	new->name = strdup (filename);
      new->path = strdup (path);
      new->mtime = mtime;
      return new;
    }
}

void
free_gpx (gpx_l **ptr)
{
  if ((*ptr)->name)
    free ((*ptr)->name);
  if ((*ptr)->path)
    free ((*ptr)->path);
  if ((*ptr)->label)
    free ((*ptr)->label);

  if ((*ptr)->next != NULL)
    free_gpx (&((*ptr)->next));
  free (*ptr);
}

#if 0
gpx_l *
get_gpx_entry (gpx_l *txt, const char *name)
{
  gpx_l *ptr = txt;
  char *cp, *fullname, *shortname = strdup (name);

  cp = strrchr (shortname, '.');
  if (cp)
    {
      *cp='\0';
      cp = shortname;
      if (asprintf (&shortname, "%s.gpx", cp) < 0)
	yapa_oom ();
      free (cp);
    }
  if (asprintf (&fullname, "%s.gpx", name) < 0)
    yapa_oom ();

  while (ptr)
    {
      if (strcmp (ptr->name, shortname) == 0 ||
	  strcmp (ptr->name, fullname) == 0)
	{
	  free (shortname);
	  free (fullname);
	  return ptr;
	}
      ptr = ptr->next;
    }
  free (shortname);
  free (fullname);
  return NULL;
}
#endif

#if 0
static dir_l *
get_and_delete_dir_entry (dir_l **dirs, const char *name)
{
  dir_l *ptr = *dirs;

  while (ptr != NULL)
    {
      if (strcmp (ptr->name, name) == 0)
	{
	  dir_l *ret = ptr;

	  if (ptr->prev == NULL) /* first image */
	    {
	      *dirs = (*dirs)->next;
	      if (*dirs != NULL)
		(*dirs)->prev = NULL;
	      return ret;
	    }
	  else /* previous dir points to next dir */
	    ptr->prev->next = ptr->next;

	  if (ptr->next == NULL) /* last dir */
	    {
	      ptr = ptr->prev;
	      if (ptr != NULL)
		ptr->next = NULL;
	      return ret;
	    }
	  else /* next dir points to previous one */
	    ptr->next->prev = ptr->prev;

	  return ret;
	}
      ptr = ptr->next;
    }
  return NULL;
}
#endif

static gpx_l *
get_and_delete_gpx_entry (gpx_l **gpx, const char *name)
{
  gpx_l *ptr = *gpx;

  while (ptr != NULL)
    {
      if (strcmp (ptr->name, name) == 0)
	{
	  gpx_l *ret = ptr;

	  if (ptr->prev == NULL) /* first gpx file */
	    {
	      *gpx = (*gpx)->next;
	      if (*gpx != NULL)
		(*gpx)->prev = NULL;
	      return ret;
	    }
	  else /* previous gpx file points to next gpx */
	    ptr->prev->next = ptr->next;

	  if (ptr->next == NULL) /* last image */
	    {
	      ptr = ptr->prev;
	      if (ptr != NULL)
		ptr->next = NULL;
	      return ret;
	    }
	  else /* next gpx points to previous one */
	    ptr->next->prev = ptr->prev;

	  return ret;
	}
      ptr = ptr->next;
    }
  return NULL;
}

static gpx_l *
sort_gpx_list (gpx_l *gpx)
{
  gpx_l *new = gpx;

  if (debug_flag)
    {
      printf ("=> Sort GPX Files\n");
      gpx_l *dbg = gpx;
      while (dbg != NULL)
	{
	  printf ("==> %s\n", dbg->name);
	  dbg = dbg->next;
	}
    }

  gpx = gpx->next;
  new->prev = NULL;
  new->next = NULL;

  while (gpx != NULL)
    {
      gpx_l *tmp = new;

      while (tmp != NULL)
	{
	  int cmp = strcmp (tmp->name, gpx->name);

	  /* gpx->name should come before tmp->name */
	  if (cmp >= 0)
	    {
	      if (tmp->prev == NULL)
		{
		  new = gpx;
		  gpx = gpx->next;
		  new->prev = NULL;
		  new->next = tmp;
		  tmp->prev = new;
		}
	      else
		{
		  gpx_l *next = gpx->next;

		  tmp->prev->next = gpx;
		  gpx->prev = tmp->prev;
		  tmp->prev = gpx;
		  gpx->next = tmp;
		  gpx = next;
		}
	      break;
	    }
	  /* gpx->name should come after tmp->name */
	  else /* if (cmp < 0) */
	    {
	      if (tmp->next == NULL)
		{
		  gpx_l *next = gpx->next;

		  tmp->next = gpx;
		  tmp->next->prev = tmp;
		  tmp->next->next = NULL;
		  gpx = next;
		  break;
		}
	      else
		tmp = tmp->next;
	    }
	}

      /* gpx = gpx->next; */
    }

  if (debug_flag)
    {
      printf ("=> Result\n");
      gpx_l *dbg = new;
      gpx_l *back = NULL;

      while (dbg != NULL)
	{
	  printf ("==> %s\n", dbg->name);
	  if (dbg->next == NULL)
	    back = dbg;
	  dbg = dbg->next;
	}
      while (back != NULL)
	{
	  printf ("==> %s\n", back->name);
	  back = back->prev;
	}
    }

  return new;
}

void
sort_gpx (dir_l *dir)
{
  char *filename;
  int need_to_save = 0;
  time_t gpx_mtime = 0;

  if (dir->name == NULL)
    {
      if (asprintf (&filename, "%s/yapa/gpx", dir->path) < 0)
	yapa_oom ();
    }
  else
    {
      if (asprintf (&filename, "%s/%s/yapa/gpx",
		    dir->path, dir->name) < 0)
	yapa_oom ();
    }

  if (debug_flag)
    printf ("SORT_GPX(%s)\n", filename);

  if (dir->gpx == NULL)
    {
      unlink (filename); /* delete old crap */
      free (filename);
      return;
    }

  /* open old file with order and labels */
  FILE *fp = fopen (filename, "r");
  if (fp != NULL)
    {
      gpx_l *newlist = NULL, *curr_new = NULL;
      char *buf = NULL;
      size_t buflen = 0;
      struct stat st;

      stat (filename, &st);
      gpx_mtime = st.st_mtime;

      while (!feof (fp))
	{
	  char *cp, *ptr;
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

	  ptr = strchr (cp, '@');
	  if (ptr != NULL)
	    *ptr++='\0';

	  gpx_l *gpx = get_and_delete_gpx_entry (&dir->gpx, cp);
	  if (gpx == NULL)
	    {
	      if (debug_flag)
		printf ("===> OBSOLETE GPX Track=%s\n", cp);
	      dir->force_html = 1;
	      need_to_save = 1;
	    }
	  else
	    {
	      if (ptr)
		gpx->label = strdup (ptr);
	      if (newlist == NULL)
		{
		  newlist = gpx;
		  curr_new = gpx;
		  newlist->prev = NULL;
		  newlist->next = NULL;
		}
	      else
		{
		  curr_new->next = gpx;
		  gpx->prev = curr_new;
		  gpx->next = NULL;
		  curr_new = gpx;
		}
	    }
	}
      if (dir->gpx != NULL)
	{
	  if (debug_flag)
	    printf ("FOUND NEW GPX Tracks\n");

	  if (dir->config.sort_img == 1) /* Add sorted gpx files at the
					    end of existing list */
	    dir->gpx = sort_gpx_list(dir->gpx);

	  if (newlist)
	    {
	      curr_new->next = dir->gpx;
	      dir->gpx->prev = curr_new;
	    }
	  else
	    {
	      newlist = dir->gpx;
	      curr_new = dir->gpx;
	      curr_new->prev = NULL;
	    }
	  dir->force_html = 1;
	  need_to_save = 1;
	}
      dir->gpx = newlist;
      free (buf);
      fclose (fp);
    }
  else
    {
      need_to_save = 1;
      if (dir->config.sort_img == 1) /* Add sorted gpx tracks at the
					end of existing list */
	dir->gpx = sort_gpx_list (dir->gpx);
    }

  /* Save new file with order and labels */
  if (dir->gpx != NULL && need_to_save)
    {
      /* sort all gpx files */
      if (dir->config.sort_img == 2)
	dir->gpx = sort_gpx_list (dir->gpx);

      fp = fopen (filename, "w");
      if (fp == NULL)
	abort ();
      gpx_l *ptr = dir->gpx;
      while (ptr)
	{
	  fprintf (fp, "%s", ptr->name);
	  if (ptr->label)
	    fprintf (fp, "@%s", ptr->label);
	  fputs ("\n", fp);
	  ptr = ptr->next;
	}
      fclose (fp);
    }

  free (filename);

  /* Go through all html files, look if we need to create or delete
     some of them. */
  gpx_l *ptr = dir->gpx;
  while (ptr != NULL)
    {
      txt_l *html = get_and_delete_html_entry (&dir->html, ptr->name);
      if (html == NULL)
	{
	  if (debug_flag)
	    printf ("===> NO html file for %s\n", ptr->name);
	  dir->force_html = 1; /* new gpx file -> recreate everything */
	}
      else
	{
	  ptr->mtime = html->mtime;
	  free (html->name);
	  free (html->path);
	  free (html);
	  if (gpx_mtime > ptr->mtime)
	    dir->force_html = 1; /* image file is changed */
	}
      ptr = ptr->next;
    }

  if (dir->html != NULL)
    {
      if (debug_flag)
	printf ("===> OBSOLETE HTML FILES -> Recreate all html files\n");
      dir->force_html = 1; /* delete images, -> recreate everything */

      while (dir->html != NULL)
	{
	  txt_l *tmp;
	  char *fname;

	  if (asprintf (&fname, "%s/%s", dir->html->path, dir->html->name) < 0)
	    yapa_oom ();
	  if (debug_flag)
	    printf ("===> OBSOLETE HTML FILE %s\n", dir->html->name);
	  else
	    printf ("Delete obsolete html file %s\n", dir->html->name);
	  unlink (fname); /* Delete old html file */
	  free (fname);
	  tmp = dir->html;
	  dir->html = dir->html->next;
	  free (tmp->path);
	  free (tmp->name);
	  free (tmp);
	}
    }
}
