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

#ifndef _MAIN_H_
#define _MAIN_H_

typedef struct config_t {
  int subdirformat; /* 0: table, 1: list with <LI> tags */
  int subdircols;   /* number of cols in a subdir table */
  int imagecols;    /* number of images in a col on index page */
  int imagerows;    /* number of images in a row on index page */
  int thumbnail;    /* size of thumbnails */
  int midnail;      /* size of midnails */
  int sort_dir;     /* 0: none, 1: add sorted to end, 2: sort all */
  int sort_img;     /* 0: none, 1: add sorted to end, 2: sort all */
} config_t;

#define MAX_EXIF_LINES 18
typedef struct image_l {
  char *name;         /* name of image file */
  char *srcdir;       /* path to image */
  char *dstdir;       /* where html files should be created */
  char *label;        /* label of image used for html */
  time_t mtime;       /* last modification time of image */
  time_t html_mtime;  /* last modification time of html page */
  time_t descr_mtime; /* last modification time of description */
  int have_exif_data; /* do we have exif data? */
  char *exif_key[MAX_EXIF_LINES]; /* exif key */
  char *exif_val[MAX_EXIF_LINES]; /* exif value */
  char *exif_google_url;
  char *exif_osm_url;
  struct image_l *prev;
  struct image_l *next;
} image_l;

typedef struct txt_l {
  char *name;   /* name of text file */
  char *path;   /* path to text file */
  time_t mtime; /* last modification time of text file */
  struct txt_l *prev;
  struct txt_l *next;
} txt_l;

typedef struct gpx_l {
  char *name;   /* name of gpx file */
  char *path;   /* path to gpx file */
  char *label;  /* label of gpx file */
  time_t mtime; /* last modification time of gpx file */
  struct gpx_l *prev;
  struct gpx_l *next;
} gpx_l;

typedef struct dir_l {
  char *name;              /* name of directory. NULL if top directory */
  char *path;              /* path to directory */
  char *label;             /* label of directory */
  image_l *images;         /* linked list of images in this directory */
  txt_l *texts;            /* linked list of text files with descriptions */
  txt_l *html;             /* linked list of html files */
  gpx_l *gpx;              /* linked list of gpx files */
  config_t config;         /* config options for HTML output */
  int force_html;          /* force recreation of html pages */
  time_t mtime;            /* Creation time of index*.html */
  time_t descr_mtime;      /* Last modification time of directroy.txt */
  time_t directory_mtime;  /* Last modification time of yapa/directory */
  struct dir_l *parentdir; /* pointer to data of parent directory */
  struct dir_l *subdirs;   /* linked list of subdirectories */
  struct dir_l *prev;
  struct dir_l *next;
} dir_l;


extern int debug_flag; /* enable debug messages */
extern int force_html_flag; /* force recreation of all html files */
extern int force_nail_flag; /* force recreation of all thumb files */

extern void yapa_oom (void);

/* config.c */
extern config_t get_config (dir_l *dir, dir_l *parent);
extern void get_root_config (dir_l *dir);
extern void create_root_config (const char *rootdir);


/* directories.c */
extern char *find_root_dir (const char *start_dir);
extern dir_l *add_dir (dir_l **dir, const char *path, const char *dirname);
extern void free_dir (dir_l **dir);
extern dir_l *get_and_delete_dir_entry (dir_l **dirs, const char *name);
extern void update_html (dir_l *dir);


/* txtnotes.c */
extern txt_l *add_txt (txt_l **descr, const char *path,
		       const char *filename, time_t mtime);
extern txt_l *get_txt_entry (txt_l *txt, const char *name);
extern void free_txt (txt_l **ptr);

/* gpx-tracks.c */
extern gpx_l *add_gpx (gpx_l **descr, const char *path,
                       const char *filename, time_t mtime);
extern void free_gpx (gpx_l **ptr);
extern void sort_gpx (dir_l *dir);

/* htmlfiles.c */
extern txt_l *add_html (txt_l **html, const char *path,
			const char *filename, time_t mtime);
extern txt_l *get_html_entry (txt_l *html, const char *name);
extern txt_l *get_and_delete_html_entry (txt_l **html,
					 const char *name);


/* images.c */
extern void create_nail (const char *srcdir, const char *dstdir,
			 const char *fname, int size, const char *nailname);
extern void add_image (dir_l *dir, const char *srcdir, const char *dstdir,
		       const char *filename, time_t mtime);
extern void free_images (image_l **img);
extern void add_midnail (image_l **midnails, const char *path,
			 const char *filename, time_t mtime);
extern void add_thumbnail (image_l **thumbnails, const char *path,
			   const char *filename, time_t mtime);
extern image_l *get_and_delete_image_entry (image_l **image,
					    const char *name);
extern void sort_images (dir_l *dir);


/* exif.c */
extern void load_exif_data (image_l *img);


/* style.c */
extern void create_html_image (image_l *img, dir_l *dir, unsigned long long maxnumber);
extern void create_html_index (dir_l *img);


#endif
