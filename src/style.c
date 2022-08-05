/* Copyright (c) 2006, 2007, 2008, 2009, 2011, 2018 Thorsten Kukuk
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
#include <unistd.h>
#include <sys/stat.h>

#include "main.h"

static char *
get_dir_label (dir_l *dir)
{
  char *buf = NULL;
  size_t i;

  if (buf != NULL)
    {
      free (buf);
      buf = NULL;
    }

  if (dir->label != NULL)
    buf = strdup (dir->label);
  else if (dir->name != NULL)
    buf = strdup (dir->name);
  else
    buf = strdup ("Photo Gallery");

  for (i = 0; i < strlen (buf); i++)
    if (buf[i] == '_')
      buf[i] = ' ';

  return buf;
}

static char *
get_gpx_label (gpx_l *gpx)
{
  char *buf = NULL;
  size_t i;

  if (buf != NULL)
    {
      free (buf);
      buf = NULL;
    }

  if (gpx->label != NULL)
    buf = strdup (gpx->label);
  else if (gpx->name != NULL)
    buf = strdup (gpx->name);
  else
    buf = strdup ("GPX Track");

  for (i = 0; i < strlen (buf); i++)
    if (buf[i] == '_')
      buf[i] = ' ';

  return buf;
}

static void
create_html_frame_line (FILE *fp)
{
  /* Trennlinie */
  fprintf (fp, "	<tr>\n");
  fprintf (fp, "	  <td>\n");
  fprintf (fp, "	    <div align=\"center\">\n");
  fprintf (fp, "	      <hr width=\"90%%\"></hr>\n");
  fprintf (fp, "	    </div>\n");
  fprintf (fp, "	  </td>\n");
  fprintf (fp, "	</tr>\n");
}

static void
print_html_path (FILE *fp, dir_l *dir, int level)
{
  if (dir->parentdir)
    print_html_path (fp, dir->parentdir, level + 1);

  if (dir->name == NULL && level == 0)
    {
      char *cp = get_dir_label (dir);
      fprintf (fp, "%s\n", cp);
      free (cp);
    }
  else
    {
      int i;
      char *cp;

      fprintf (fp, "<a href=\"");
      for (i = 0; i < level; i++)
	fprintf (fp, "../");
      if (level == 0)
	{
	  cp = get_dir_label (dir);
	  fprintf (fp, "index.html\"><i>%s</i></a>", cp);
	  free (cp);
	}
      else
	{
	  cp = get_dir_label (dir);
	  fprintf (fp, "index.html\">%s</a>", cp);
	  free (cp);
	  fprintf (fp, "  &gt;");
	}
      fprintf (fp, "\n");
    }
}

static void
create_html_frame_start (FILE *fp, const char *label,
			 dir_l *dir, image_l *img, int is_index)
{
  /* Write the HTML header */

  fprintf (fp, "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n");
  fprintf (fp, "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n");
  fprintf (fp, "<head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n");
  fprintf (fp, "  <title>%s</title>\n", label);

  if (!is_index && img && img->have_exif_data)
    {
      int i, count = 0;

      for (i = 0; i < MAX_EXIF_LINES; i++)
	if (img->exif_key[i] != NULL && img->exif_val[i] != NULL &&
	    strlen (img->exif_val[i]) > 0)
	  count++;

      fprintf (fp, "<script type=\"text/javascript\">\n");
      fprintf (fp, "<!--\n");
      fprintf (fp, "window.captureEvents(Event.UNLOAD);\n");
      fprintf (fp, "window.onunload = close_popup;\n");
      fprintf (fp, "function close_popup(value) {exif_window.close();}\n");
      fprintf (fp, "\n");
      fprintf (fp, "function popup() {\n");
      fprintf (fp, "  exif_window = window.open(\"\",\"EXIF_tags\",\"height=%d,width=640,left=80,top=80\");\n", count * 34 + 55);
      fprintf (fp, "  exif_window.document.open();\n");
      fprintf (fp, "  exif_window.document.write('<title>Extra Image Information (%s)</title>');\n", label);
      fprintf (fp, "  exif_window.document.write('<body>');\n");
      fprintf (fp, "  exif_window.document.write('<table border=\"0\" cellpadding=\"4\" cellspacing=\"0\">');\n");


      for (i = 0; i < MAX_EXIF_LINES; i++)
	if (img->exif_key[i] != NULL && img->exif_val[i] != NULL &&
	    strlen (img->exif_val[i]) > 0)
	  fprintf (fp, "  exif_window.document.write('<TR><TD align=\"right\"><B>%s:</B></TD><TD align=\"left\">%s</TD></TR>');\n",
		   img->exif_key[i], img->exif_val[i]);


      fprintf (fp, "  exif_window.document.write('</table>');\n");
      fprintf (fp, "  exif_window.document.write('<br><div align=\"right\" valign=\"botton\" > <a href=\"javascript:self.close()\">close</a>   </div>');\n");
      fprintf (fp, "  exif_window.document.write('</body></html>');\n");
      fprintf (fp, "  exif_window.document.close();\n");
      fprintf (fp, "  exif_window.focus();\n");
      fprintf (fp, "}\n");
      fprintf (fp, "    //-->\n");
      fprintf (fp, "  </script>\n");
    }

  fprintf (fp, "</head>\n");

  fprintf (fp, "  <body bgcolor=\"#3a3c69\" text=\"#dcdef2\" link=\"#dcdef2\" alink=\"#dcdef2\" vlink=\"#dcdef2\">\n");
  fprintf (fp, "    <table border=\"0\" width=\"100%%\" cellpadding=\"0\" cellspacing=\"0\">\n");
  /* Generate the directory tree */
  fprintf (fp, "	<tr>\n");
  fprintf (fp, "	  <td align=\"center\" valign=\"middle\">\n");
  fprintf (fp, "	    <table border=\"0\" cellpadding=\"15\" cellspacing=\"0\">\n");
  fprintf (fp, "		<tr>\n");
  fprintf (fp, "		  <td align=\"center\" valign=\"middle\">\n");
  fprintf (fp, "		    <font size=\"+1\">\n");

  print_html_path (fp, dir, 0);

  fprintf (fp, "</font>\n");
  fprintf (fp, "		  </td>\n");
  fprintf (fp, "		</tr>\n");

  /* Insert directory description */
  if (is_index)
    {
      txt_l *descr= get_txt_entry (dir->texts, "directory");

      if (descr != NULL)
	{
	  char *fname;
	  FILE *tp;
	  char *buf = NULL;
	  size_t buflen = 0;


	  if (asprintf (&fname, "%s/%s", descr->path, descr->name) < 0)
	    yapa_oom ();
	  tp = fopen (fname, "r");
	  free (fname);
	  fprintf (fp, "<tr><td align=\"center\" valign=\"middle\"><p>\n");
	  while (!feof (tp))
	    {
	      ssize_t n = getline (&buf, &buflen, tp);

	      if (n < 1)
		break;

	      n = strlen (buf) - 1;
	      if (buf[n] == '\n') /* remove trailing newline */
		buf[n] = '\0';

	      fprintf (fp, "%s\n", buf);
	    }
	  fprintf (fp, "</p></td></td>\n");
	  fclose (tp);
	  free (buf);
	}
    }

  fprintf (fp, "	    </table>\n");
  fprintf (fp, "	  </td>\n");
  fprintf (fp, "	</tr>\n");

  create_html_frame_line (fp);
}

static void
create_html_frame_end (FILE *fp)
{
  create_html_frame_line (fp);

  fprintf (fp, "	<tr>\n");
  fprintf (fp, "	  <td align=\"center\" valign=\"bottom\">\n");
  fprintf (fp, "	    <i>Photo gallery generated by yapa.</i>\n");
  fprintf (fp, "	  </td>\n");
  fprintf (fp, "	</tr>\n");
  fprintf (fp, "    </table>\n");
  fprintf (fp, "  </body>\n");
  fprintf (fp, "</html>\n");
}

static char *
get_label (image_l *img)
{
  char *buf = NULL;
  size_t i;

  if (buf != NULL)
    {
      free (buf);
      buf = NULL;
    }

  if (img->label != NULL)
    buf = strdup (img->label);
  else
    {
      char *cp = strrchr (img->name, '.');
      if (cp != NULL)
	{
	  buf = strdup (img->name);
	  cp = strrchr (buf, '.');
	  *cp = '\0';
	}
      else
	return img->name;
    }

  for (i = 0; i < strlen (buf); i++)
    if (buf[i] == '_')
      buf[i] = ' ';

  return buf;
}

void
create_html_image (image_l *img, dir_l *dir, unsigned long long imgnumber)
{
  FILE *fp;
  char *cp, *filename;
  txt_l *descr = get_txt_entry (dir->texts, img->name);

  if (asprintf (&filename, "%s/%s.html", img->dstdir, img->name) < 0)
    yapa_oom ();
  if (debug_flag)
    printf ("========>CREATE HTML: %s\n",filename);
  else
    printf ("Create html file for %s\n", img->name);

  load_exif_data (img);

  fp = fopen (filename, "w");
  free (filename);

  if (fp == NULL)
    {
      fprintf (stderr, "ERROR: Cannot create %s: %m", filename);
      exit (1);
    }

  cp = get_label (img);
  create_html_frame_start (fp, cp, dir, img, 0);
  free (cp);

  /* <prev> <title> <next> */
  fprintf (fp, "	<tr>\n");
  fprintf (fp, "	  <td>\n");
  fprintf (fp, "	    <div align=\"center\">\n");
  fprintf (fp, "	      <table border=\"0\" cellpadding=\"6\" cellspacing=\"0\" width=\"80%%\">\n");
  fprintf (fp, "		  <tr>\n");
  if (img->prev != NULL)
    {
      fprintf (fp, "		    <td align=\"left\" valign=\"middle\" width=\"30%%\">\n");
      cp = get_label (img->prev);
      fprintf (fp, "		      <a href=\"%s.html\" title=\"Preview Picture: %s\">&lt;&lt; Previous</a>\n",
	       img->prev->name, cp);
      free (cp);
      fprintf (fp, "		    </td>\n");
    }
  else
    fprintf (fp, "		      <td align=\"left\" valign=\"middle\" width=\"30%%\">&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;</td>\n");

  fprintf (fp, "		    <td align=\"center\" valign=\"middle\" width=\"40%%\">\n");
  cp = get_label (img);
  fprintf (fp, "		      <b>%s</b><br>\n", cp);
  free (cp);
  fprintf (fp, "		    </td>\n");
  if (img->next != NULL)
    {
      fprintf (fp, "		    <td align=\"right\" valign=\"middle\" width=\"30%%\">\n");
      cp = get_label (img->next);
      fprintf (fp, "		      <a href=\"%s.html\" title=\"Next Picture: %s\">Next  &gt;&gt;</a>\n",
	       img->next->name, cp);
      free (cp);
      fprintf (fp, "		    </td>\n");
    }
  else
    fprintf (fp, "		      <td align=\"right\" valign=\"middle\" width=\"30%%\">&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;</td>\n");

  fprintf (fp, "		  </tr>\n");

  /* Show image */
  fprintf (fp, "		  <tr>\n");
  fprintf (fp, "		    <td colspan=\"3\" align=\"center\" valign=\"middle\">\n");
  fprintf (fp, "		      <table border=\"0\" cellpadding=\"10\" cellspacing=\"0\" bgcolor=\"#ffffff\">\n");
  fprintf (fp, "			  <tr>\n");
  fprintf (fp, "			    <td>\n");
  //  fprintf (fp, "			      <a href=\"%s\"><img src=\"yapa/midnails/%s\" width=\"640\" height=\"480\" border=\"0\" title=\"Click on image for full view\"></a>\n", img->name, img->name);
  if (strcmp (img->srcdir, img->dstdir) != 0)
    {
      /* Directory where the image is stored is not the directory we
	 create the html page */
      char *relpath;

      /* srcdir is always dstdir + relative path to image from links file */
      relpath = img->srcdir;
      relpath+=(strlen (img->dstdir) + 1);

      fprintf (fp, "			      <a href=\"%s/%s\"><img src=\"yapa/midnails/%s\" border=\"0\" title=\"Click on image for full view\"></a>\n", relpath, img->name, img->name);
    }
  else
    fprintf (fp, "			      <a href=\"%s\"><img src=\"yapa/midnails/%s\" border=\"0\" title=\"Click on image for full view\"></a>\n", img->name, img->name);
  fprintf (fp, "			    </td>\n");
  fprintf (fp, "			  </tr>\n");
  fprintf (fp, "		      </table>\n");
  fprintf (fp, "		    </td>\n");
  fprintf (fp, "		  </tr>\n");

  if (descr)
    {
      char *fname;
      FILE *tp;
      char *buf = NULL;
      size_t buflen = 0;

      fprintf (fp, "<tr>\n");
      fprintf (fp, "  <td colspan=\"3\" align=\"center\">\n");
      fprintf (fp, "  <table border=\"0\" cellpadding=\"0\" cellspacing=\"0\" width=\"660\"><tr>\n");
      fprintf (fp, "    <td>\n");
      fprintf (fp, "      <p>\n");

      if (asprintf (&fname, "%s/%s", descr->path, descr->name) < 0)
	yapa_oom ();
      tp = fopen (fname, "r");
      free (fname);
      while (!feof (tp))
        {
          ssize_t n = getline (&buf, &buflen, tp);

	  if (n < 1)
	    break;

          n = strlen (buf) - 1;
          if (buf[n] == '\n') /* remove trailing newline */
            buf[n] = '\0';

	  fprintf (fp, "%s<br/>\n", buf);
	}
      fclose (tp);
      free (buf);

      fprintf (fp, "    </p></td></tr></table>\n");
      fprintf (fp, "  </td>\n");
      fprintf (fp, "</tr>\n");
    }

  if (img->have_exif_data)
    {
      fprintf (fp, "		  <tr>\n");
      fprintf (fp, "		    <td align=\"center\" valign=\"middle\" colspan=\"3\">\n");
      fprintf (fp, "		      <a href=\"javascript:popup()\"><font size=\"-1\">Show Extra Image Information (EXIF tags)</font></a>\n");

      if (img->exif_google_url)
	{
	  fprintf (fp, "<font size=\"-1\"> / </font>\n");
	  fprintf (fp, "                  <a href=\"%s\" target=\"_blank\"><font size=\"-1\">Google Maps</font></a>\n",
		   img->exif_google_url);
	}

      if (img->exif_osm_url)
        {
          fprintf (fp, "<font size=\"-1\"> / </font>\n");
          fprintf (fp, "                  <a href=\"%s\" target=\"_blank\"><font size=\"-1\">OpenStreetMap</font></a>\n",
                   img->exif_osm_url);
        }


      fprintf (fp, "		    </td>\n");
      fprintf (fp, "		  </tr>\n");
    }

  fprintf (fp, "		  <tr>\n");
  if (img->prev != NULL)
    {
      fprintf (fp, "		    <td align=\"left\" valign=\"bottom\" width=\"30%%\">\n");
      cp = get_label (img->prev);
      fprintf (fp, "		      <a href=\"%s.html\" title=\"Preview Picture: %s\">&lt;&lt; Previous</a>\n",
	       img->prev->name, cp);
      free (cp);
      fprintf (fp, "		    </td>\n");
    }
  else
    fprintf (fp, "		      <td align=\"left\" valign=\"bottom\" width=\"30%%\">&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;</td>\n");

  fprintf (fp, "		    <td align=\"center\" valign=\"bottom\" width=\"40%%\">\n");

  unsigned long pagenumber =
    1.0 + (imgnumber / ((1.0 * dir->config.imagerows * dir->config.imagecols)));

  if (pagenumber == 1)
    fprintf (fp, "		      <a href=\"index.html\">Return to Index</a>\n");
  else
    fprintf (fp, "		      <a href=\"index-%li.html\">Return to Index</a>\n", pagenumber);

  fprintf (fp, "		    </td>\n");
  if (img->next != NULL)
    {
      fprintf (fp, "		    <td align=\"right\" valign=\"bottom\" width=\"30%%\">\n");
      cp = get_label (img->next);
      fprintf (fp, "		      <a href=\"%s.html\" title=\"Next Picture: %s\">Next  &gt;&gt;</a>\n",
	       img->next->name, cp);
      free (cp);
      fprintf (fp, "		    </td>\n");
    }
  else
    fprintf (fp, "		      <td align=\"right\" valign=\"bottom\" width=\"30%%\">&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;&#160;</td>\n");
  fprintf (fp, "		  </tr>\n");
  fprintf (fp, "	      </table>\n");
  fprintf (fp, "	    </div>\n");
  fprintf (fp, "	  </td>\n");
  fprintf (fp, "	</tr>\n");

  create_html_frame_end (fp);

  fclose (fp);
}

static void
create_html_index_nr (dir_l *dir, int pagenr, int maxpages, int maximages)
{
  FILE *fp;
  char *filename;
  dir_l *subdir = dir->subdirs;
  image_l *image = dir->images;
  gpx_l *gpx = dir->gpx;

  if (pagenr == 1)
    {
      if (dir->name == NULL)
	{
	  if (asprintf (&filename, "%s/index.html", dir->path) < 0)
	    yapa_oom ();
	}
      else
	{
	  if (asprintf (&filename, "%s/%s/index.html",
			dir->path, dir->name) < 0)
	    yapa_oom ();
	}
    }
  else
    {
      if (dir->name == NULL)
	{
	  if (asprintf (&filename, "%s/index-%d.html",
			dir->path, pagenr) < 0)
	    yapa_oom ();
	}
      else
	{
	  if (asprintf (&filename, "%s/%s/index-%d.html",
			dir->path, dir->name, pagenr) < 0)
	    yapa_oom ();
	}
    }

  if (!dir->force_html && !force_html_flag && dir->mtime > dir->descr_mtime)
    {
      struct stat st;

      /* File exists and we don't need to recreate them,
	 return */
      if (stat (filename, &st) == 0)
	{
	  /* ../yapa/directories and yapa/directories should be
	     older */
	  if (st.st_mtime > dir->directory_mtime &&
	      (dir->parentdir == NULL ||
	       st.st_mtime > dir->parentdir->directory_mtime))
	    {
	      free (filename);
	      return;
	    }
	}
    }

  if (debug_flag)
    printf ("========>CREATE HTML: %s\n",filename);
  else
    printf ("Create index file %s\n", basename (filename));

  fp = fopen (filename, "w");
  free (filename);

  if (fp == NULL)
    {
      fprintf (stderr, "ERROR: Cannot create %s: %m", filename);
      exit (1);
    }

  char *cp = get_dir_label (dir);
  create_html_frame_start (fp, cp, dir, NULL, 1);
  free (cp);

  if (subdir != NULL)
    {
      fprintf (fp, "<tr>\n");
      fprintf (fp, "  <td>\n");
      fprintf (fp, "    <div align=\"center\">\n");
      fprintf (fp, "    <table border=\"0\" cellpadding=\"1\" cellspacing=\"0\" width=\"80%%\">\n");
      fprintf (fp, "      <tr>\n");
      fprintf (fp, "        <th align=\"left\" valign=\"top\" colspan=\"%d\">Sub-Galleries:</th>\n",
	       dir->config.subdircols);
      fprintf (fp, "      </tr><tr>\n");

      if (dir->config.subdirformat == 1)
	{ /* use <ul><li></ul> */
	  fprintf (fp, "    <td colspan=\"%d\"><ul>\n", dir->config.subdircols);
	  while (subdir != NULL)
	    {
	      cp = get_dir_label (subdir);
	      fprintf (fp, "<li><a href=\"%s/index.html\">%s</a></li>\n",
		       subdir->name, cp);
	      free (cp);
	      subdir = subdir->next;
	    }
	  fprintf (fp, "    </ul></td>\n");
	}
      else /* do it in a table */
	{
	  int count = 0;
	  while (subdir != NULL)
	    {
	      cp = get_dir_label (subdir);
	      fprintf (fp, "<td><a href=\"%s/index.html\">%s</a></td>\n",
		       subdir->name, cp);
	      free (cp);
	      subdir = subdir->next;
	      if (count % dir->config.subdircols == (dir->config.subdircols - 1))
		fprintf (fp, "      </tr><tr>\n");
	      count++;
	    }
	}
      fprintf (fp, "      </tr>\n");
      fprintf (fp, "    </table>\n");
      fprintf (fp, "    </div>\n");
      fprintf (fp, "  </td>\n");
      fprintf (fp, "</tr>\n");
    }

  if (dir->subdirs && dir->gpx)
    create_html_frame_line (fp);

  if (gpx != NULL)
    {
      fprintf (fp, "<tr>\n");
      fprintf (fp, "  <td>\n");
      fprintf (fp, "    <div align=\"center\">\n");
      fprintf (fp, "    <table border=\"0\" cellpadding=\"1\" cellspacing=\"0\" width=\"80%%\">\n");
      fprintf (fp, "      <tr>\n");
      fprintf (fp, "        <th align=\"left\" valign=\"top\" colspan=\"%d\">GPS-Track Visualisierung:</th>\n",
	       dir->config.subdircols);
      fprintf (fp, "      </tr><tr>\n");

      if (dir->config.subdirformat == 1)
	{ /* use <ul><li></ul> */
	  fprintf (fp, "    <td colspan=\"%d\"><ul>\n", dir->config.subdircols);
	  while (gpx != NULL)
	    {
	      cp = get_gpx_label (gpx);
	      fprintf (fp, "<li><a href=\"%s.html\">%s</a></li>\n",
		       gpx->name, cp);
	      free (cp);
	      gpx =gpx->next;
	    }
	  fprintf (fp, "    </ul></td>\n");
	}
      else /* do it in a table */
	{
	  int count = 0;
	  while (gpx != NULL)
	    {
	      cp = get_gpx_label (gpx);
	      fprintf (fp, "<td><a href=\"%s.html\">%s</a></td>\n",
		       gpx->name, cp);
	      free (cp);
	      gpx = gpx->next;
	      if (count % dir->config.subdircols == (dir->config.subdircols - 1))
		fprintf (fp, "      </tr><tr>\n");
	      count++;
	    }
	}
      fprintf (fp, "      </tr>\n");
      fprintf (fp, "    </table>\n");
      fprintf (fp, "    </div>\n");
      fprintf (fp, "  </td>\n");
      fprintf (fp, "</tr>\n");
    }

  if (dir->gpx && dir->images)
    create_html_frame_line (fp);

  if (dir->subdirs && !dir->gpx && dir->images)
    create_html_frame_line (fp);

  if (image != NULL)
    {
      int count = 0, i;

      fprintf (fp, "<tr>\n");
      fprintf (fp, "  <td>\n");
      fprintf (fp, "    <div align=\"center\">\n");
      fprintf (fp, "    <table border=\"0\" cellpadding=\"16\" cellspacing=\"0\" width=\"80%%\">\n");
      fprintf (fp, "      <tr>\n");
      fprintf (fp, "        <td align=\"center\" valign=\"top\">\n");

      if (maxpages > 1)
	{
	  fprintf (fp, "<table border=\"0\" cellpadding=\"4\" cellspacing=\"0\" width=\"100%%\">\n");
	  fprintf (fp, "  <tr>\n");
	  fprintf (fp, "    <td align=\"left\" valign=\"top\" nowrap width=\"10%%\">\n");
	  if (pagenr == 1)
	    fprintf (fp, "      <b>&#160;</b>\n");
	  else if (pagenr == 2)
	    fprintf (fp, "<a href=\"index.html\"><b>&lt;</b></a>\n");
	  else
	    fprintf (fp, "<a href=\"index-%d.html\"><b>&lt;</b></a>\n", pagenr - 1);
	  fprintf (fp, "    </td>\n");
	  fprintf (fp, "    <td align=\"center\" valign=\"top\" width=\"80%%\">\n");
	  fprintf (fp, "Page: ");
	  for (i = 1; i <= maxpages; i++)
	    if (i == pagenr)
	      fprintf (fp, " <b>%d</b>", pagenr);
	    else
	      if (i == 1)
		fprintf (fp, " <a href=\"index.html\">1</a>");
	      else
		fprintf (fp, " <a href=\"index-%d.html\">%d</a>", i, i);
	  fprintf (fp, "\n");
	  fprintf (fp, "     </td>\n");
	  fprintf (fp, "     <td align=\"right\" valign=\"top\" nowrap width=\"10%%\">\n");
	  if (pagenr == maxpages)
	    fprintf (fp, "      <b>&#160;</b>\n");
	  else
	    fprintf (fp, "      <a href=\"index-%d.html\"><b>&gt;</b></a>\n", pagenr+1);
	  fprintf (fp, "    </td>\n");
	  fprintf (fp, "  </tr>\n");
	  fprintf (fp, "</table>\n");
	}

      fprintf (fp, "      <table border=\"0\" cellpadding=\"14\" cellspacing=\"0\">\n");
      fprintf (fp, "      <tr>\n");


      int start = 1;
      while (start < (pagenr - 1) * dir->config.imagerows * dir->config.imagecols + 1)
	{
	  ++start;
	  image = image->next;
	}

      while (image != NULL)
	{
	  fprintf (fp, "<td align=\"center\" valign=\"middle\">\n");
	  fprintf (fp, "<table border=\"0\" cellpadding=\"5\" cellspacing=\"0\" bgcolor=\"#ffffff\">\n");
	  fprintf (fp, "  <tr>\n");
	  fprintf (fp, "    <td><a href=\"%s.html\"><img src=\"yapa/thumbnails/%s\" border=\"0\" ALT=\"%s\"></a></td>\n",
		   image->name, image->name, image->name);
	  cp = get_label (image);
	  fprintf (fp, "</tr></table><br>%s</td>\n", cp);
	  free (cp);
	  image = image->next;
	  if (count % dir->config.imagecols == (dir->config.imagecols - 1))
	    fprintf (fp, "      </tr><tr>\n");
	  count++;
	  if (count % (dir->config.imagecols * dir->config.imagerows) == 0)
	    break;
	}

      fprintf (fp, "      </tr>\n");
      fprintf (fp, "      </table>\n");

      if (maxpages > 1)
	{
	  fprintf (fp, "<table border=\"0\" cellpadding=\"4\" cellspacing=\"0\" width=\"100%%\">\n");
	  fprintf (fp, "  <tr>\n");
	  fprintf (fp, "    <td align=\"left\" valign=\"top\" nowrap width=\"10%%\">\n");
	  if (pagenr == 1)
	    fprintf (fp, "      <b>&#160;</b>\n");
	  else if (pagenr == 2)
	    fprintf (fp, "<a href=\"index.html\"><b>&lt;</b></a>\n");
	  else
	    fprintf (fp, "<a href=\"index-%d.html\"><b>&lt;</b></a>\n", pagenr - 1);
	  fprintf (fp, "    </td>\n");
	  fprintf (fp, "    <td align=\"center\" valign=\"top\" width=\"80%%\">\n");
	  fprintf (fp, "Page: ");
	  for (i = 1; i <= maxpages; i++)
	    if (i == pagenr)
	      fprintf (fp, " <b>%d</b>", pagenr);
	    else
	      if (i == 1)
		fprintf (fp, " <a href=\"index.html\">1</a>");
	      else
		fprintf (fp, " <a href=\"index-%d.html\">%d</a>", i, i);
	  fprintf (fp, "\n");
	  fprintf (fp, "     </td>\n");
	  fprintf (fp, "     <td align=\"right\" valign=\"top\" nowrap width=\"10%%\">\n");
	  if (pagenr == maxpages)
	    fprintf (fp, "      <b>&#160;</b>\n");
	  else
	    fprintf (fp, "      <a href=\"index-%d.html\"><b>&gt;</b></a>\n", pagenr+1);
	  fprintf (fp, "    </td>\n");
	  fprintf (fp, "  </tr>\n");
	  fprintf (fp, "</table>\n");
	}

      fprintf (fp, "          <br>\n");
      if (maximages == 1)
	fprintf (fp, "          1 Picture on ");
      else
	fprintf (fp, "          %d Pictures on ", maximages);
      if (maxpages == 1)
	fprintf (fp, "1 Page\n");
      else
	fprintf (fp, "%d Pages\n", maxpages);
      fprintf (fp, "        </td>\n");
      fprintf (fp, "      </tr>\n");
      fprintf (fp, "    </table>\n");
      fprintf (fp, "    </div>\n");
      fprintf (fp, "  </td>\n");
      fprintf (fp, "</tr>\n");
    }

  create_html_frame_end (fp);

  fclose (fp);
}

void
create_html_index (dir_l *dir)
{
  int maximages, maxpages, i;
  image_l *image = dir->images;

  maximages = 0;
  while (image != NULL)
    {
      ++maximages;
      image = image->next;
    }

  maxpages = 0;
  i = maximages;
  while (i > (dir->config.imagerows * dir->config.imagecols))
    {
      ++maxpages;
      i-=(dir->config.imagerows * dir->config.imagecols);
    }
  if (i > 0 || maxpages == 0)
    ++maxpages;

  for (i = 1; i <= maxpages; i++)
    create_html_index_nr (dir, i, maxpages, maximages);
}
