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

#include <libsoup/soup.h>

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
		GooCanvasItem *root;
		GooCanvasItem *img;
		SoupSession *soup_session;
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

	priv->img = NULL;
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

	g_object_set (G_OBJECT (gtk_mapserver),
				  "background-color", "white",
				  NULL);

	priv->root = goo_canvas_get_root_item (GOO_CANVAS (gtk_mapserver));

	priv->img = goo_canvas_image_new (priv->root,
									  NULL,
									  0, 0,
									  NULL);

	/* Soup */
	priv->soup_session = soup_session_sync_new_with_options (SOUP_SESSION_SSL_CA_FILE, NULL,
															 SOUP_SESSION_ADD_FEATURE_BY_TYPE, SOUP_TYPE_CONTENT_DECODER,
															 SOUP_SESSION_ADD_FEATURE_BY_TYPE, SOUP_TYPE_COOKIE_JAR,
															 SOUP_SESSION_USER_AGENT, "get ",
															 SOUP_SESSION_ACCEPT_LANGUAGE_AUTO, TRUE,
															 SOUP_SESSION_USE_NTLM, FALSE,
															 NULL);

	return gtk_mapserver;
}

void
gtk_mapserver_set_home (GtkMapserver *gtkm,
						const gchar *url)
{
	GError *error;
	SoupMessage *msg;
	GdkPixbufLoader *pxb_loader;

	GtkMapserverPrivate *priv = GTK_MAPSERVER_GET_PRIVATE (gtkm);

	msg = soup_message_new (SOUP_METHOD_GET, url);
	if (SOUP_IS_MESSAGE (msg))
		{
			soup_message_set_flags (msg, SOUP_MESSAGE_NO_REDIRECT);
			soup_session_send_message (priv->soup_session, msg);
		}

	if (!SOUP_IS_MESSAGE (msg) || !SOUP_STATUS_IS_SUCCESSFUL (msg->status_code))
		{
			g_warning ("Error on retrieving url.");
			pxb_loader = NULL;
		}
	else
		{
			error = NULL;
			pxb_loader = gdk_pixbuf_loader_new ();
			if (!gdk_pixbuf_loader_write (pxb_loader,
										  msg->response_body->data,
										  msg->response_body->length,
										  &error)
				|| error != NULL)
				{
					g_warning ("Error on retrieving map image: %s.",
							   error != NULL && error->message != NULL ? error->message : "no details");
					g_object_unref (pxb_loader);
					pxb_loader = NULL;
				}
			else
				{
					gdk_pixbuf_loader_close (pxb_loader, NULL);
				}
		}
	g_object_unref (msg);
	if (pxb_loader != NULL)
		{
			g_object_set (G_OBJECT (priv->img),
						  "pixbuf", gdk_pixbuf_loader_get_pixbuf (pxb_loader),
						  NULL);

			g_object_unref (pxb_loader);
			pxb_loader = NULL;
		}
	else
		{
			g_object_set (G_OBJECT (priv->img),
						  "pixbuf", NULL,
						  NULL);
		}
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
