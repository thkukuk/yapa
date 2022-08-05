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
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "main.h"

char *
find_root_dir (const char *start_dir)
{
  struct stat st;
  char *dirname, *directory;

  if (start_dir[0] != '/')
    directory = realpath (start_dir, NULL);
  else
    directory = strdup (start_dir);

  if (directory[strlen (directory) - 1] == '/')
    {
      if (asprintf (&dirname, "%syapa/root", directory) < 0)
	yapa_oom ();
    }
  else
    {
      if (asprintf (&dirname, "%s/yapa/root", directory) < 0)
	yapa_oom ();
    }

  if (stat (dirname, &st) < 0)
    {
      if (errno == ENOENT || errno == ENOTDIR)
	{
	  char *cp;

	  if (strlen (directory) == 1)
	    {
	      free (directory);
	      free (dirname);
	      return NULL;
	    }

	  cp = strrchr (directory, '/');
	  if (cp == NULL)
	    {
	      fprintf (stderr, "ERROR: cannot parse path '%s'\n", directory);
	      abort ();
	    }
	  else
	    *cp = '\0';

	  if (directory[0] == '\0')
	    strcpy (directory, "/");
	  cp = find_root_dir (directory);
	  free (directory);
	  free (dirname);
	  return cp;
	}
      else
	{
	  fprintf (stderr, "ERROR: stat(%s): %m\n", dirname);
	  abort ();
	}
    }

  if (!S_ISREG(st.st_mode))
    {
      fprintf (stderr, "ERROR: %s is no regular file\n", dirname);
      abort ();
    }

  free (dirname);

  return directory;
}


dir_l *
add_dir (dir_l **dir, const char *path, const char *dirname)
{
  if (debug_flag)
    printf ("ADD DIRECTORY: %s\n", path);

  if (*dir == NULL)
    {
      *dir = calloc (1, sizeof (dir_l));
      if (dirname != NULL)
	(*dir)->name = strdup (dirname);
      (*dir)->path = strdup (path);
      return *dir;
    }
  else
    {
      dir_l *ptr, *new = calloc (1, sizeof (dir_l));

      ptr = *dir;
      while (ptr->next != NULL)
	ptr = ptr->next;

      ptr->next = new;
      new->prev = ptr;
      if (dirname != NULL)
	new->name = strdup (dirname);
      new->path = strdup (path);
      return new;
    }
}

void
free_dir (dir_l **dir)
{
  if (debug_flag)
    printf ("FREE DIRECTORY: %s\n", (*dir)->path);

  if ((*dir)->name != NULL)
    free ((*dir)->name);
  if ((*dir)->path != NULL)
    free ((*dir)->path);
  if ((*dir)->label != NULL)
    free ((*dir)->label);

  if ((*dir)->texts)
    free_txt (&((*dir)->texts));
  if ((*dir)->html)
    free_txt (&((*dir)->html));
  free_images (&((*dir)->images));

  if ((*dir)->next != NULL)
    free_dir (&((*dir)->next));
  if ((*dir)->subdirs != NULL)
    free_dir (&((*dir)->subdirs));
  free (*dir);
}

dir_l *
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


/* Go through the midnail and thumbnail directories and create
   list of available nails. */
static void
go_through_nails (image_l **images, const char *directory,
		  void(*add_nail)(image_l **images, const char *,
				  const char *, time_t))
{
  DIR *dir = opendir (directory);
  struct dirent *d;

  if (dir == NULL)
    return;

  if (debug_flag)
    printf ("TODO: Searching for existing nails in %s\n", directory);

  while ((d = readdir (dir)) != NULL)
    {
      char *buf;
      struct stat st;

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
	printf ("==> Please remove wrong subdirectory\n");
      else if (S_ISREG(st.st_mode))
	{
	  if (strcasecmp (&d->d_name[strlen (d->d_name) - 4], ".jpg") == 0)
	    {
	      if (debug_flag)
		printf ("==> ");
	      add_nail (images, directory, d->d_name, st.st_mtime);

	    }
	  else if (strcasecmp (&d->d_name[strlen (d->d_name) - 4], ".png") == 0)
	    {
	      if (debug_flag)
		printf ("==> Add ");
	      add_nail (images, directory, d->d_name, st.st_mtime);
	    }
	  else
	    {
	      unlink (buf);
	      if (debug_flag)
		printf ("==> DELETED\n");
	      else
		printf (_("Delete old nail %s/%s\n"), basename (directory),
			d->d_name);
	    }
	}
      else
	{
	  unlink (buf);
	  printf ("==> DELETED\n");
	}
      free (buf);
    }

  closedir (dir);
}

static dir_l *
sort_dir (dir_l *dir)
{
  dir_l *new = dir;

  if (debug_flag)
    {
      printf ("=> Sort Directories\n");
      dir_l *dbg = dir;
      while (dbg != NULL)
	{
	  printf ("==> %s\n", dbg->name);
	  dbg = dbg->next;
	}
    }

  dir = dir->next;
  new->prev = NULL;
  new->next = NULL;

  while (dir != NULL)
    {
      dir_l *tmp = new;

      while (tmp != NULL)
	{
	  int cmp = strcmp (tmp->name, dir->name);

	  /* dir->name should come before tmp->name */
	  if (cmp >= 0)
	    {
	      if (tmp->prev == NULL)
		{
		  new = dir;
		  dir = dir->next;
		  new->prev = NULL;
		  new->next = tmp;
		  tmp->prev = new;
		}
	      else
		{
		  dir_l *next = dir->next;

		  tmp->prev->next = dir;
		  dir->prev = tmp->prev;
		  tmp->prev = dir;
		  dir->next = tmp;
		  dir = next;
		}
	      break;
	    }
	  /* dir->name should come after tmp->name */
	  else /* if (cmp < 0) */
	    {
	      if (tmp->next == NULL)
		{
		  dir_l *next = dir->next;

		  tmp->next = dir;
		  tmp->next->prev = tmp;
		  tmp->next->next = NULL;
		  dir = next;
		  break;
		}
	      else
		tmp = tmp->next;
	    }
	}

      /* dir = dir->next; */
    }

  if (debug_flag)
    {
      printf ("=> Result\n");
      dir_l *dbg = new;
      dir_l *back = NULL;

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

static void
sort_directories (dir_l *dir)
{
  char *filename;
  int need_to_save = 0;

  if (dir->name == NULL)
    {
      if (asprintf (&filename, "%s/yapa/directories", dir->path) < 0)
	yapa_oom ();
    }
  else
    {
      if (asprintf (&filename, "%s/%s/yapa/directories",
		    dir->path, dir->name) < 0)
	yapa_oom ();
    }

  if (dir->subdirs == NULL)
    {
      unlink (filename); /* Delete old crap */
      free (filename);
      return;
    }

  /* open old file with order and labels */
  FILE *fp = fopen (filename, "r");
  if (fp != NULL)
    {
      dir_l *newlist = NULL, *curr_new = NULL;
      char *buf = NULL;
      size_t buflen = 0;
      struct stat st;

      stat (filename, &st);
      dir->directory_mtime = st.st_mtime;

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

	  dir_l *entry = get_and_delete_dir_entry (&dir->subdirs, cp);
	  if (entry == NULL ||
	      (entry->subdirs == NULL && entry->images == NULL))
	    {
	      if (debug_flag)
		printf ("===> OBSOLETE DIR=%s, recreate all html pages\n", cp);
	      dir->force_html = 1;
	      need_to_save = 1;
	    }
	  else
	    {
	      if (ptr)
		entry->label = strdup (ptr);
	      if (newlist == NULL)
		{
		  newlist = entry;
		  curr_new = entry;
		  newlist->prev = NULL;
		  newlist->next = NULL;
		}
	      else
		{
		  curr_new->next = entry;
		  entry->prev = curr_new;
		  entry->next = NULL;
		  curr_new = entry;
		}
	    }
	}
      if (dir->subdirs != NULL)
	{
	  int first = 0;

	  if (dir->config.sort_dir == 1) /* Add sorted directories at the
					    end of existing list */
	    dir->subdirs = sort_dir (dir->subdirs);

	  while (dir->subdirs != NULL)
	    {
	      if (dir->subdirs->subdirs != NULL ||
		  dir->subdirs->images != NULL)
		{
		  if (!first)
		    {
		      if (debug_flag)
			printf ("FOUND NEW SUBDIRS\n");
		      dir->force_html = 1;
		      need_to_save = 1;
		      first = 1;
		    }

		  if (debug_flag)
		    printf ("=> FOUND %s\n", dir->subdirs->name);
		  curr_new->next = dir->subdirs;
		  dir->subdirs->prev = curr_new;
		  dir->subdirs = dir->subdirs->next;
		  curr_new->next->next = NULL;
		  curr_new = curr_new->next;
		}
	      else
		{
		  if (debug_flag)
		    printf ("FOUND EMPTY DIR: %s\n", dir->subdirs->name);
		  dir->subdirs = dir->subdirs->next;
		}
	    }
	}
      dir->subdirs = newlist;

      free (buf);
      fclose (fp);
    }
  else
    {
      dir->force_html = 1;
      need_to_save = 1;

      if (dir->config.sort_dir == 1) /* Add sorted directories at the
					end of existing list */
	dir->subdirs = sort_dir (dir->subdirs);
    }

  /* Save new file with order and labels */
  if (dir->subdirs != NULL && need_to_save)
    {
      /* sort all directories */
      if (dir->config.sort_dir == 2)
	dir->subdirs = sort_dir (dir->subdirs);

      fp = fopen (filename, "w");
      if (fp == NULL)
	abort ();
      dir_l *ptr = dir->subdirs;
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
}

static void
update_nails (dir_l *dir)
{
  char *cp;
  image_l *midnails = NULL, *thumbnails = NULL;
  image_l *images = dir->images;

  if (dir->name == NULL) /* root directory */
    {
      if (debug_flag)
	printf ("DIR=ROOT [%s]\n", dir->path);

      if (asprintf (&cp, "%s/yapa/midnails", dir->path) < 0)
	yapa_oom ();
      go_through_nails (&midnails, cp, add_midnail);
      free (cp);

      if (asprintf (&cp, "%s/yapa/thumbnails", dir->path) < 0)
	yapa_oom ();

      go_through_nails (&thumbnails, cp, add_thumbnail);
      free (cp);
    }
  else
    {
      if (debug_flag)
	printf ("DIR=%s [%s]\n", dir->name, dir->path);

      if (asprintf (&cp, "%s/%s/yapa/midnails", dir->path, dir->name) < 0)
	yapa_oom ();
      go_through_nails (&midnails, cp, add_midnail);
      free (cp);

      if (asprintf (&cp, "%s/%s/yapa/thumbnails", dir->path, dir->name) < 0)
	yapa_oom ();
      go_through_nails (&thumbnails, cp, add_thumbnail);
      free (cp);
    }

  /* make sure we have a midnail and a thumbnail for
     every image */
  while (images != NULL)
    {
      if (debug_flag)
	printf ("===>IMAGE=%s\n", images->name);

      image_l *nail = get_and_delete_image_entry (&midnails, images->name);
      if (nail == NULL || images->mtime > nail->mtime || force_nail_flag)
	create_nail (images->srcdir, images->dstdir,  images->name,
		     dir->config.midnail, "midnails");
      if (nail)
	{
	  free (nail->name);
	  free (nail->srcdir);
	  free (nail->dstdir);
	  free (nail);
	}
      nail = get_and_delete_image_entry (&thumbnails, images->name);
      if (nail == NULL || images->mtime > nail->mtime || force_nail_flag)
	create_nail (images->srcdir, images->dstdir, images->name,
		     dir->config.thumbnail, "thumbnails");
      if (nail)
	{
	  free (nail->name);
	  free (nail->srcdir);
	  free (nail->dstdir);
	  free (nail);
	}
      images = images->next;
    }

  /* if midnails are left, delete them. */
  while (midnails != NULL)
    {
      image_l *tmp;

      if (debug_flag)
	printf ("===>MIDNAIL=%s => DELETE\n", midnails->name);
      else
	printf ("Delete obsolete midnail %s\n", midnails->name);
      if (asprintf (&cp, "%s/%s", midnails->srcdir, midnails->name) < 0)
	yapa_oom ();
      unlink (cp);
      free (cp);
      tmp = midnails;
      midnails = midnails->next;
      free (tmp->name);
      free (tmp->srcdir);
      free (tmp->dstdir);
      free (tmp);
    }

  /* if thumbnails are left, delete them. */
  while (thumbnails != NULL)
    {
      image_l *tmp;

      if (debug_flag)
	printf ("===>THUMBNAIL=%s => DELETE\n", thumbnails->name);
      else
	printf ("Delete obsolete thumbnail %s\n", thumbnails->name);
      if (asprintf (&cp, "%s/%s", thumbnails->dstdir, thumbnails->name) < 0)
	yapa_oom ();
      unlink (cp);
      free (cp);
      tmp = thumbnails;
      thumbnails = thumbnails->next;
      free (tmp->name);
      free (tmp->srcdir);
      free (tmp->dstdir);
      free (tmp);
    }
}

void
update_html (dir_l *dir)
{
  unsigned long long imgnumber;
  image_l *images;
  dir_l *subdirs;
  txt_l *tptr;

  if (!debug_flag)
    printf ("Entering directory %s\n", dir->name ? dir->name : "root");

  tptr = get_and_delete_html_entry (&dir->html, "index");
  if (tptr != NULL)
    {
      dir->mtime = tptr->mtime;
      free (tptr->name);
      free (tptr->path);
      free (tptr);
    }
  tptr = get_txt_entry (dir->texts, "directory");
  if (tptr)
    dir->descr_mtime = tptr->mtime;

  sort_images (dir);
  sort_gpx (dir);
  sort_directories (dir);

  update_nails (dir);

  /* Create html for every image */
  images = dir->images;
  imgnumber = 0;
  while (images != NULL)
    {
      if (debug_flag)
	printf ("===>HTML page for %s, html=%lu, img=%lu\n",
		images->name, (unsigned long)images->mtime,
		(unsigned long)images->html_mtime);

      if (images->mtime > images->html_mtime ||
	  images->descr_mtime > images->html_mtime ||
	  dir->force_html || force_html_flag)
	create_html_image (images, dir, imgnumber);

      images = images->next;

      ++imgnumber;
    }

  create_html_index (dir);

  subdirs = dir->subdirs;
  while (subdirs != NULL)
    {
      update_html (subdirs);
      subdirs = subdirs->next;
    }
}
