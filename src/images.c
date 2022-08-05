/* Copyright (c) 2006, 2007, 2008, 2009 Thorsten Kukuk
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
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#define X_DISPLAY_MISSING
#include <Imlib2.h>

#include "main.h"

void
create_nail (const char *srcdir, const char *dstdir, const char *fname,
	     int size, const char *nailname)
{
  Imlib_Image orig_image;
  Imlib_Load_Error error;
  char *filename;

  if (asprintf (&filename, "%s/%s", srcdir, fname) < 0)
    yapa_oom ();
  if (debug_flag)
    printf ("========>CREATE: %s from %s\n", nailname, filename);
  else
    printf ("Create %s (max. %dx%d) for %s\n", nailname, size, size, fname);

  orig_image = imlib_load_image_with_error_return (filename, &error);
  if (orig_image == NULL)
    {
      fprintf (stderr,
	       _("ERROR: Couldn't load image %s, imlib2 error code %d\n"),
	       filename, error);
      free (filename);
      return;
    }
  else
    {
      Imlib_Image nail_image;

      imlib_context_set_image (orig_image);

      unsigned int new_width = 0;
      unsigned int new_height = 0;
      unsigned int width = imlib_image_get_width ();
      unsigned int height = imlib_image_get_height ();
      double max_size = size;
      double actual_size = height > width ? height : width;
      double scale_factor = max_size / actual_size;

      if (scale_factor < 1.0)
	{
	  new_width  = (unsigned int)(width * scale_factor);
	  new_height = (unsigned int)(height * scale_factor);


	  nail_image =
	    imlib_create_cropped_scaled_image (0, 0,
					       width, height,
					       new_width,
					       new_height);
	  imlib_free_image();
	  imlib_context_set_image(nail_image);
	}
      free (filename);

      if (asprintf (&filename, "%s/yapa/%s/%s",
		    dstdir, nailname, fname) < 0)
	yapa_oom ();

      imlib_save_image_with_error_return (filename, &error);
      if (error != IMLIB_LOAD_ERROR_NONE)
	{
	  if (error == IMLIB_LOAD_ERROR_PATH_COMPONENT_NON_EXISTANT)
	    {
	      char *cp;
	      if (asprintf (&cp, "%s/yapa/%s", dstdir, nailname) < 0)
		yapa_oom ();
	      mkdir (cp, 0755);
	      free (cp);
	      imlib_save_image_with_error_return (filename, &error);
	    }
	  if (error != IMLIB_LOAD_ERROR_NONE)
	    {
	      fprintf (stderr,
		       _("ERROR: Couldn't create nail %s, imlib2 error code %d\n"),
		       filename, error);
	      abort ();
	    }
	}
      imlib_free_image ();
      free (filename);
    }
}

static void
internal_add_image (image_l **image_list, const char *srcdir,
		    const char *dstdir, const char *filename,
		    time_t mtime, const char *dbgmsg)
{
  if (debug_flag)
    printf ("ADD %s: %s/%s\n", dbgmsg, srcdir, filename);

  if (*image_list == NULL)
    {
      *image_list = calloc (1, sizeof (image_l));
      (*image_list)->name = strdup (filename);
      (*image_list)->srcdir = strdup (srcdir);
      (*image_list)->dstdir = strdup (dstdir);
      (*image_list)->mtime = mtime;
      (*image_list)->prev = NULL;
      (*image_list)->next = NULL;
    }
  else
    {
      image_l *ptr, *new = calloc (1, sizeof (image_l));

      ptr = *image_list;
      while (ptr->next != NULL)
	ptr = ptr->next;

      ptr->next = new;
      new->prev = ptr;
      new->next = NULL;
      new->name = strdup (filename);
      new->srcdir = strdup (srcdir);
      new->dstdir = strdup (dstdir);
      new->mtime = mtime;
    }
}

void
add_image (dir_l *dir, const char *srcdir, const char *dstdir,
	   const char *filename, time_t mtime)
{
  internal_add_image (&dir->images, srcdir, dstdir, filename, mtime, "IMAGE");
}

void
free_images (image_l **img)
{

  if (*img == NULL)
    return;

  if (debug_flag)
    printf ("FREE IMAGE: %s\n", (*img)->name);

  if ((*img)->name != NULL)
    free ((*img)->name);
  if ((*img)->srcdir != NULL)
    free ((*img)->srcdir);
  if ((*img)->dstdir != NULL)
    free ((*img)->dstdir);
  if ((*img)->label != NULL)
    free ((*img)->label);
  if ((*img)->exif_google_url != NULL)
    free ((*img)->exif_google_url);
  if ((*img)->exif_osm_url != NULL)
    free ((*img)->exif_osm_url);

  int i;

  for (i = 0; i < MAX_EXIF_LINES; i++)
    {
      if ((*img)->exif_val[i] != NULL)
	free ((*img)->exif_val[i]);
    }

  if ((*img)->next != NULL)
    free_images (&((*img)->next));
  free (*img);
}

void
add_midnail (image_l **nails, const char *path,
	     const char *filename, time_t mtime)
{
  internal_add_image (nails, path, path, filename, mtime, "MIDNAIL");
}

void
add_thumbnail (image_l **nails, const char *path,
	       const char *filename, time_t mtime)
{
  internal_add_image (nails, path, path, filename, mtime, "THUMBNAIL");
}

#if 0
image_l *
get_image_entry (image_l **image, const char *name)
{
  image_l *ptr = *image;

  while (ptr != NULL)
    {
      if (strcmp (ptr->name, name) == 0)
	return ptr;
      ptr = ptr->next;
    }
  return NULL;
}
#endif


image_l *
get_and_delete_image_entry (image_l **image, const char *name)
{
  image_l *ptr = *image;

  while (ptr != NULL)
    {
      if (strcmp (ptr->name, name) == 0)
	{
	  image_l *ret = ptr;

	  if (ptr->prev == NULL) /* first image */
	    {
	      *image = (*image)->next;
	      if (*image != NULL)
		(*image)->prev = NULL;
	      return ret;
	    }
	  else /* previous images points to next image */
	    ptr->prev->next = ptr->next;

	  if (ptr->next == NULL) /* last image */
	    {
	      ptr = ptr->prev;
	      if (ptr != NULL)
		ptr->next = NULL;
	      return ret;
	    }
	  else /* next image points to previous one */
	    ptr->next->prev = ptr->prev;

	  return ret;
	}
      ptr = ptr->next;
    }
  return NULL;
}

static image_l *
sort_img (image_l *img)
{
  image_l *new = img;

  if (debug_flag)
    {
      printf ("=> Sort Images\n");
      image_l *dbg = img;
      while (dbg != NULL)
	{
	  printf ("==> %s\n", dbg->name);
	  dbg = dbg->next;
	}
    }

  img = img->next;
  new->prev = NULL;
  new->next = NULL;

  while (img != NULL)
    {
      image_l *tmp = new;

      while (tmp != NULL)
	{
	  int cmp = strcmp (tmp->name, img->name);

	  /* img->name should come before tmp->name */
	  if (cmp >= 0)
	    {
	      if (tmp->prev == NULL)
		{
		  new = img;
		  img = img->next;
		  new->prev = NULL;
		  new->next = tmp;
		  tmp->prev = new;
		}
	      else
		{
		  image_l *next = img->next;

		  tmp->prev->next = img;
		  img->prev = tmp->prev;
		  tmp->prev = img;
		  img->next = tmp;
		  img = next;
		}
	      break;
	    }
	  /* img->name should come after tmp->name */
	  else /* if (cmp < 0) */
	    {
	      if (tmp->next == NULL)
		{
		  image_l *next = img->next;

		  tmp->next = img;
		  tmp->next->prev = tmp;
		  tmp->next->next = NULL;
		  img = next;
		  break;
		}
	      else
		tmp = tmp->next;
	    }
	}

      /* img = img->next; */
    }

  if (debug_flag)
    {
      printf ("=> Result\n");
      image_l *dbg = new;
      image_l *back = NULL;

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
sort_images (dir_l *dir)
{
  char *filename;
  int need_to_save = 0;
  time_t image_mtime = 0;

  if (dir->name == NULL)
    {
      if (asprintf (&filename, "%s/yapa/images", dir->path) < 0)
	yapa_oom ();
    }
  else
    {
      if (asprintf (&filename, "%s/%s/yapa/images",
		    dir->path, dir->name) < 0)
	yapa_oom ();
    }

  if (debug_flag)
    printf ("SORT_IMAGES(%s)\n", filename);

  if (dir->images == NULL)
    {
      unlink (filename); /* delete old crap */
      free (filename);
      return;
    }

  /* open old file with order and labels */
  FILE *fp = fopen (filename, "r");
  if (fp != NULL)
    {
      image_l *newlist = NULL, *curr_new = NULL;
      char *buf = NULL;
      size_t buflen = 0;
      struct stat st;

      stat (filename, &st);
      image_mtime = st.st_mtime;

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

	  image_l *img = get_and_delete_image_entry (&dir->images, cp);
	  if (img == NULL)
	    {
	      if (debug_flag)
		printf ("===> OBSOLETE IMAGE=%s\n", cp);
	      dir->force_html = 1;
	      need_to_save = 1;
	    }
	  else
	    {
	      if (ptr)
		img->label = strdup (ptr);
	      if (newlist == NULL)
		{
		  newlist = img;
		  curr_new = img;
		  newlist->prev = NULL;
		  newlist->next = NULL;
		}
	      else
		{
		  curr_new->next = img;
		  img->prev = curr_new;
		  img->next = NULL;
		  curr_new = img;
		}
	    }
	}
      if (dir->images != NULL)
	{
	  if (debug_flag)
	    printf ("FOUND NEW IMAGES\n");

	  if (dir->config.sort_img == 1) /* Add sorted images at the
					    end of existing list */
	    dir->images = sort_img (dir->images);

	  if (newlist)
	    {
	      curr_new->next = dir->images;
	      dir->images->prev = curr_new;
	    }
	  else
	    {
	      newlist = dir->images;
	      curr_new = dir->images;
	      curr_new->prev = NULL;
	    }
	  dir->force_html = 1;
	  need_to_save = 1;
	}
      dir->images = newlist;
      free (buf);
      fclose (fp);
    }
  else
    {
      need_to_save = 1;
      if (dir->config.sort_img == 1) /* Add sorted images at the
					end of existing list */
	dir->images = sort_img (dir->images);
    }

  /* Save new file with order and labels */
  if (dir->images != NULL && need_to_save)
    {
      /* sort all images */
      if (dir->config.sort_img == 2)
	dir->images = sort_img (dir->images);

      fp = fopen (filename, "w");
      if (fp == NULL)
	abort ();
      image_l *ptr = dir->images;
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
  image_l *ptr = dir->images;
  while (ptr != NULL)
    {
      txt_l *html = get_and_delete_html_entry (&dir->html, ptr->name);
      if (html == NULL)
	{
	  if (debug_flag)
	    printf ("===> NO html file for %s\n", ptr->name);
	  dir->force_html = 1; /* new image -> recreate everything */
	}
      else
	{
	  ptr->html_mtime = html->mtime;
	  free (html->name);
	  free (html->path);
	  free (html);
	  if (image_mtime > ptr->html_mtime)
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


  /* Go through all txt files and look if we need to recreate html
     files. */
  ptr = dir->images;
  while (ptr != NULL)
    {
      txt_l *txt = get_txt_entry (dir->texts, ptr->name);
      if (txt == NULL)
	{
	  if (debug_flag)
	    printf ("===> NO text file for %s\n", ptr->name);
	}
      else
	ptr->descr_mtime = txt->mtime;

      ptr = ptr->next;
    }
}
