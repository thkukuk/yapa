/* Copyright (c) 2006, 2007 Thorsten Kukuk
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
add_html (txt_l **html, const char *path,
	 const char *filename, time_t mtime)
{
  if (debug_flag)
    printf ("ADD HTML: %s\n", path);

  if (*html == NULL)
    {
      *html = calloc (1, sizeof (txt_l));
      if (filename != NULL)
	(*html)->name = strdup (filename);
      (*html)->path = strdup (path);
      (*html)->mtime = mtime;
      return *html;
    }
  else
    {
      txt_l *ptr, *new = calloc (1, sizeof (txt_l));

      ptr = *html;
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

txt_l *
get_html_entry (txt_l *html, const char *name)
{
  txt_l *ptr = html;
  char *fullname;

  if (asprintf (&fullname, "%s.html", name) < 0)
    yapa_oom ();

  while (ptr)
    {
      if (strcmp (ptr->name, fullname) == 0)
	{
	  free (fullname);
	  return ptr;
	}
      ptr = ptr->next;
    }
  free (fullname);
  return NULL;
}

txt_l *
get_and_delete_html_entry (txt_l **html, const char *name)
{
  txt_l *ptr = *html;
  char *htmlname;

  if (asprintf (&htmlname, "%s.html", name) < 0)
    yapa_oom ();

  while (ptr != NULL)
    {
      if (strcmp (ptr->name, htmlname) == 0)
	{
	  txt_l *ret = ptr;

	  if (ptr->prev == NULL) /* first entry */
	    {
	      *html = (*html)->next;
	      if (*html != NULL)
		(*html)->prev = NULL;
	      free (htmlname);
	      return ret;
	    }
	  else /* previous entry points to next html file */
	    ptr->prev->next = ptr->next;

	  if (ptr->next == NULL) /* last entry */
	    {
	      ptr = ptr->prev;
	      if (ptr != NULL)
		ptr->next = NULL;
	      free (htmlname);
	      return ret;
	    }
	  else /* next dir points to previous one */
	    ptr->next->prev = ptr->prev;

	  free (htmlname);
	  return ret;
	}
      ptr = ptr->next;
    }
  free (htmlname);
  return NULL;
}
