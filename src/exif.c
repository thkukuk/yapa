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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

// libexif includes
#include <libexif/exif-data.h>
#include <libexif/exif-content.h>
#include <libexif/exif-entry.h>
#include <libexif/exif-tag.h>

#include "main.h"

static int have_gps_data = 0;
static char *gps_latitude = NULL, *gps_latitude_ref = NULL;
static char *gps_longitude = NULL, *gps_longitude_ref = NULL;

static void
callback_exif_entry (ExifEntry *ee, void *user_data)
{
  image_l *img = (image_l *)user_data;
  char buf[1024];
  const char *name = exif_tag_get_name (ee->tag);

  exif_entry_get_value (ee, buf, sizeof (buf));

  // printf ("name=[%s], value=[%s]\n", name, buf);

  if (strcasecmp (name, "Make") == 0)
    {
      img->exif_key[0] = _("Camera Maker");
      if (img->exif_val[0])
	free (img->exif_val[0]);
      img->exif_val[0] = strdup (buf);
    }
  else if (strcasecmp (name, "Model") == 0)
    {
      img->exif_key[1] = _("Camera Model");
      if (img->exif_val[1])
	free (img->exif_val[1]);
      img->exif_val[1] = strdup (buf);
    }
  else if (strcasecmp (name, "DateTime") == 0)
    {
      img->exif_key[2] = _("Date and Time Taken");
      img->exif_val[2] = strdup (buf);
    }
  else if (strcasecmp (name, "ExposureTime") == 0)
    {
      img->exif_key[3] = _("Exposure Time");
      img->exif_val[3] = strdup (buf);
    }
  else if (strcasecmp (name, "ApertureValue") == 0 ||
	   strcasecmp (name, "FNumber") == 0)
    {
      if (img->exif_key[4] == NULL)
	{
	  img->exif_key[4] = _("Aperture");
	  img->exif_val[4] = strdup (buf);
	}
    }
  else if (strcasecmp (name, "ExposureProgram") == 0)
    {
      img->exif_key[5] = _("Exposure Program");
      img->exif_val[5] = strdup (buf);
    }
  else if (strcasecmp (name, "ISOSpeedRatings") == 0)
    {
      img->exif_key[6] = _("ISO");
      img->exif_val[6] = strdup (buf);
    }
  else if (strcasecmp (name, "ExposureBiasValue") == 0)
    {
      img->exif_key[7] = _("Exposure Bias");
      img->exif_val[7] = strdup (buf);
    }
  else if (strcasecmp (name, "MeteringMode") == 0)
    {
      img->exif_key[8] = _("Metering Mode");
      img->exif_val[8] = strdup (buf);
    }
  else if (strcasecmp (name, "Flash") == 0)
    {
      img->exif_key[9] = _("Flash Status");
      img->exif_val[9] = strdup (buf);
    }
  else if (strcasecmp (name, "FocalLength") == 0)
    {
      img->exif_key[10] = _("Focal Length");
      img->exif_val[10] = strdup (buf);
    }
  else if (strcasecmp (name, "WhiteBalance") == 0)
    {
      img->exif_key[11] = _("White Balance");
      img->exif_val[11] = strdup (buf);
    }
  else if (strcasecmp (name, "ExposureMode") == 0)
    {
      img->exif_key[12] = _("Exposure Mode");
      img->exif_val[12] = strdup (buf);
    }
  else if (strcasecmp (name, "GPSVersionID") == 0)
    have_gps_data = 5;
  else if (strcasecmp (name, "InteroperabilityIndex") == 0)
    {
      if (have_gps_data)
	{
	  --have_gps_data;
	  if (gps_latitude_ref)
	    free (gps_latitude_ref);
	  gps_latitude_ref = strdup (buf);
	}
    }
  else if (strcasecmp (name, "InteroperabilityVersion") == 0)
    {
      if (have_gps_data)
	{
	  --have_gps_data;
	  if (gps_latitude)
	    free (gps_latitude);
	  gps_latitude = strdup (buf);
	}
    }
  else if (strcasecmp (name, "GPSLongitudeRef") == 0)
    {
      --have_gps_data;
      if (gps_longitude_ref)
	free (gps_longitude_ref);
      gps_longitude_ref = strdup (buf);
    }
  else if (strcasecmp (name, "GPSLongitude") == 0)
    {
      --have_gps_data;
      if (gps_longitude)
	free (gps_longitude);
      gps_longitude = strdup (buf);
    }
  //  else printf ("ignored=%s\n", name);

  if (have_gps_data == 1 && gps_latitude && gps_latitude_ref &&
      gps_longitude && gps_longitude_ref)
    {
      int lat_deg, lat_min;
      float lat_sec;
      int  long_deg, long_min;
      float long_sec;
      double lat_n, long_n;
      have_gps_data = 0;

      sscanf (gps_latitude, "%i, %i, %f", &lat_deg, &lat_min, &lat_sec);
      sscanf (gps_longitude, "%i, %i, %f", &long_deg, &long_min, &long_sec);

      lat_n = lat_deg + lat_min/60.0 + lat_sec/(60.0*60.0);
      if (gps_latitude_ref[0] == 'S')
	lat_n = -1 * lat_n;
      long_n = long_deg + long_min/60.0 + long_sec/(60.0*60.0);
      if (gps_longitude_ref[0] == 'W')
        long_n = -1 * long_n;

      setlocale (LC_NUMERIC, "POSIX");

      img->exif_key[13] = _("GPS Position");

      if (img->exif_val[13])
	free (img->exif_val[13]);
      char *cp = NULL;

#if 0
      /* old code for google maps */
      if (asprintf (&cp,
		    "<a href=\\\"http://maps.google.de/maps?f=q&hl=de&q=+%i%%C2%%B0%i%%27%g%%22%s+++%i%%C2%%B0%i%%27%g%%22%s&ie=UTF8&z=12&om=1&z=15&iwloc=addr\\\" target=\\\"_blank\\\">%i° %i\\' %g\\'\\' %s, %i° %i\\' %g\\\" %s</a>",
		    lat_deg, lat_min, lat_sec, gps_latitude_ref,
		    long_deg, long_min, long_sec, gps_longitude_ref,
		    lat_deg, lat_min, lat_sec, gps_latitude_ref,
		    long_deg, long_min, long_sec, gps_longitude_ref) < 0)
	yapa_oom ();
#else
      /* new code for OpenStreetMap */
      if (asprintf (&cp,
		    "<a href=\\\"http://www.openstreetmap.org/?mlat=%g&mlon=%g&zoom=15\\\" target=\\\"_blank\\\">%i° %i\\' %g\\'\\' %s, %i° %i\\' %g\\\" %s</a>",
		    lat_n, long_n,
		    lat_deg, lat_min, lat_sec, gps_latitude_ref,
		    long_deg, long_min, long_sec, gps_longitude_ref) < 0)
	yapa_oom ();
#endif
      img->exif_val[13] = cp;

      if (asprintf (&(img->exif_google_url),
		    "http://maps.google.de/maps?f=q&hl=de&q=+%i%%C2%%B0%i%%27%g%%22%s+++%i%%C2%%B0%i%%27%g%%22%s&ie=UTF8&z=12&om=1&z=15&iwloc=addr",
		    lat_deg, lat_min, lat_sec, gps_latitude_ref,
		    long_deg, long_min, long_sec, gps_longitude_ref) < 0)
	yapa_oom ();

      if (asprintf (&(img->exif_osm_url),
		    "http://www.openstreetmap.org/?mlat=%g&mlon=%g&zoom=15",
		    lat_n, long_n) < 0)
	yapa_oom ();

      free (gps_latitude);
      gps_latitude = NULL;
      free (gps_latitude_ref);
      gps_latitude_ref = NULL;
      free (gps_longitude);
      gps_longitude = NULL;
      free (gps_longitude_ref);
      gps_longitude_ref = NULL;

      setlocale (LC_NUMERIC, "");
    }
}

static void
callback_exif_content (ExifContent *ec, void *user_data)
{
  exif_content_foreach_entry (ec, callback_exif_entry, user_data);
}

void
load_exif_data (image_l *img)
{
  char *filename;
  ExifData *ed;
  int i;

  if (asprintf (&filename, "%s/%s", img->srcdir, img->name) < 0)
    yapa_oom ();
  ed = exif_data_new_from_file (filename);
  free (filename);

  if (ed == NULL)
    return;

  exif_data_foreach_content (ed, callback_exif_content, img);
  /* clean static data for next image */
  have_gps_data = 0;
  if (gps_latitude)
    {
      free (gps_latitude);
      gps_latitude = NULL;
    }
  if (gps_latitude_ref)
    {
      free (gps_latitude_ref);
      gps_latitude_ref = NULL;
    }
  if (gps_longitude)
    {
      free (gps_longitude);
      gps_longitude = NULL;
    }
  if (gps_longitude_ref)
    {
      free (gps_longitude_ref);
      gps_longitude_ref = NULL;
    }

  ExifMnoteData *mnd = exif_data_get_mnote_data (ed);
  if (mnd)
    {
      int c;

      c = exif_mnote_data_count (mnd);

      if (c > 0)
	{
	  char *shortfocal = NULL, *longfocal = NULL;
	  int have_lens = 0;

	  for (i = 0; i < c; i++)
	    {
	      const char *name = exif_mnote_data_get_name (mnd, i);

	      if (name)
		{
		  const char *value;
		  char buf[1024];

		  value = exif_mnote_data_get_value (mnd, i, buf,
						     sizeof (buf));

		  if (value == NULL)
		    fprintf (stderr, "ERROR: %s: value=NULL!\n", name);
		  if (strcasecmp (name, "Picture style") == 0)
		    {
		      img->exif_key[14] = _("Picture Style");
		      img->exif_val[14] = strdup (value);
                    }
                  else if (strcasecmp (name, "Lens type") == 0)
		    {
		      if (value[0] != '0' && value[1] != 'x') /* unknown lens */
			{
			  img->exif_key[15] = _("Lens Type");
			  img->exif_val[15] = strdup (value);
			  have_lens = 1;
			}
		    }
		  else if (strcasecmp (name, "FirmwareVersion") == 0)
		    {
		      img->exif_key[16] = _("Firmware Version");
		      img->exif_val[16] = strdup (value);
		    }
		  else if (strcasecmp (name, "OwnerName") == 0)
		    {
		      img->exif_key[17] = _("Owner Name");
		      img->exif_val[17] = strdup (value);
		    }
		  else if (strcasecmp (name, "Long focal length of lens") == 0)
		    {
		      if (longfocal)
			free (longfocal);
		      longfocal = strdup (value);
		    }
		  else if (strcasecmp (name, "Short focal length of lens")
			   == 0)
		    {
		      if (shortfocal)
			free (shortfocal);
		      shortfocal = strdup (value);
		    }
		}
	    }
	  if ((shortfocal || longfocal) && have_lens == 0)
	    {
	      img->exif_key[14] = _("Lens");
	      if (shortfocal && longfocal && strcmp (shortfocal, longfocal) != 0)
		{
		  if (asprintf (&img->exif_val[15], "%s-%s",
				shortfocal, longfocal) < 0)
		    yapa_oom ();
		}
	      else if (shortfocal)
		{
		  if (asprintf (&img->exif_val[15], "%s",
				shortfocal) < 0)
		    yapa_oom ();
		}
	    }

	  if (shortfocal)
	    free (shortfocal);
	  if (longfocal)
	    free (longfocal);
	}
    }
  exif_data_free (ed);

  for (i = 0; i < MAX_EXIF_LINES; i++)
    if (img->exif_key[i] != NULL)
      {
	img->have_exif_data = 1;
	break;
      }
}
