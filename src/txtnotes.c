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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "main.h"


txt_l *
add_txt (txt_l **descr, const char *path,
	 const char *filename, time_t mtime)
{
  if (debug_flag)
    printf ("ADD TEXT: %s\n", path);

  if (*descr == NULL)
    {
      *descr = calloc (1, sizeof (txt_l));
      if (filename != NULL)
	(*descr)->name = strdup (filename);
      (*descr)->path = strdup (path);
      (*descr)->mtime = mtime;
      return *descr;
    }
  else
    {
      txt_l *ptr, *new = calloc (1, sizeof (txt_l));

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
free_txt (txt_l **ptr)
{
  if ((*ptr)->name)
    free ((*ptr)->name);
  if ((*ptr)->path)
    free ((*ptr)->path);

  if ((*ptr)->next != NULL)
    free_txt (&((*ptr)->next));
  free (*ptr);
}

txt_l *
get_txt_entry (txt_l *txt, const char *name)
{
  txt_l *ptr = txt;
  char *cp, *fullname, *shortname = strdup (name);

  cp = strrchr (shortname, '.');
  if (cp)
    {
      *cp='\0';
      cp = shortname;
      if (asprintf (&shortname, "%s.txt", cp) < 0)
	yapa_oom ();
      free (cp);
    }
  if (asprintf (&fullname, "%s.txt", name) < 0)
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
