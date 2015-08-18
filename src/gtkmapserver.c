/*
 *  gtkmapserver.c
 *
 *  Copyright (C) 2015 Andrea Zagli <azagli@libero.it>
 *
 *  This file is part of libgtkmapserver.
 *
 *  libgtk_mapserver is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  libgtk_mapserver is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with libgdaex; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
	#include <config.h>
#endif

#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>

#ifdef G_OS_WIN32
	#include <windows.h>
#endif

#include "gtkmapserver.h"

static void gtk_mapserver_class_init (GtkMapserverClass *klass);
static void gtk_mapserver_init (GtkMapserver *gtk_mapserver);

static void gtk_mapserver_set_property (GObject *object,
                               guint property_id,
                               const GValue *value,
                               GParamSpec *pspec);
static void gtk_mapserver_get_property (GObject *object,
                               guint property_id,
                               GValue *value,
                               GParamSpec *pspec);

#define GTK_MAPSERVER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GTK_TYPE_MAPSERVER, GtkMapserverPrivate))

typedef struct _GtkMapserverPrivate GtkMapserverPrivate;
struct _GtkMapserverPrivate
	{
	};

G_DEFINE_TYPE (GtkMapserver, gtk_mapserver, GOO_TYPE_CANVAS)

#ifdef G_OS_WIN32
static HMODULE hmodule;

BOOL WINAPI
DllMain (HINSTANCE hinstDLL,
         DWORD     fdwReason,
         LPVOID    lpvReserved)
{
	switch (fdwReason)
		{
			case DLL_PROCESS_ATTACH:
				hmodule = hinstDLL;
				break;
		}

	return TRUE;
}
#endif

static void
gtk_mapserver_class_init (GtkMapserverClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (object_class, sizeof (GtkMapserverPrivate));

	object_class->set_property = gtk_mapserver_set_property;
	object_class->get_property = gtk_mapserver_get_property;
}

static void
gtk_mapserver_init (GtkMapserver *gtk_mapserver)
{
	GtkMapserverPrivate *priv = GTK_MAPSERVER_GET_PRIVATE (gtk_mapserver);
}

GtkWidget
*gtk_mapserver_new ()
{
	gchar *localedir;

	GtkWidget *gtk_mapserver = GTK_WIDGET (g_object_new (gtk_mapserver_get_type (), NULL));

	GtkMapserverPrivate *priv = GTK_MAPSERVER_GET_PRIVATE (GTK_MAPSERVER (gtk_mapserver));

#ifdef G_OS_WIN32

	gchar *moddir;
	gchar *p;

	moddir = g_win32_get_package_installation_directory_of_module (hmodule);

	p = g_strrstr (moddir, g_strdup_printf ("%c", G_DIR_SEPARATOR));
	if (p != NULL
	    && (g_ascii_strcasecmp (p + 1, "src") == 0
	        || g_ascii_strcasecmp (p + 1, ".libs") == 0))
		{
			localedir = g_strdup (LOCALEDIR);
		}
	else
		{
			localedir = g_build_filename (moddir, "share", "locale", NULL);
		}

	g_free (moddir);

#else

	localedir = g_strdup (LOCALEDIR);

#endif

	bindtextdomain (GETTEXT_PACKAGE, localedir);
	textdomain (GETTEXT_PACKAGE);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");

	g_free (localedir);

	return gtk_mapserver;
}

/* PRIVATE */
static void
gtk_mapserver_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	GtkMapserver *gtk_mapserver = GTK_MAPSERVER (object);
	GtkMapserverPrivate *priv = GTK_MAPSERVER_GET_PRIVATE (gtk_mapserver);

	switch (property_id)
		{
			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}

static void
gtk_mapserver_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	GtkMapserver *gtk_mapserver = GTK_MAPSERVER (object);
	GtkMapserverPrivate *priv = GTK_MAPSERVER_GET_PRIVATE (gtk_mapserver);

	switch (property_id)
		{
			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}
