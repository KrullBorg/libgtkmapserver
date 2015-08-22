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

static gboolean gtk_mapserver_resize_timer (gpointer user_data);
static void gtk_mapserver_draw (GtkMapserver *gtkm);

static void gtk_mapserver_on_size_allocate (GtkWidget *widget,
											GdkRectangle *allocation,
											gpointer user_data);

static gboolean gtk_mapserver_on_key_release_event (GooCanvasItem *item,
													GooCanvasItem *target_item,
													GdkEventKey *event,
													gpointer user_data);

static gboolean gtk_mapserver_on_button_press_event (GtkWidget *widget,
													 GdkEventButton *event,
													 gpointer user_data);

static gboolean gtk_mapserver_on_button_release_event (GtkWidget *widget,
													   GdkEventButton *event,
													   gpointer user_data);

static gboolean gtk_mapserver_on_motion_notify_event (GtkWidget *widget,
													  GdkEventMotion *event,
													  gpointer user_data);

#define GTK_MAPSERVER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GTK_TYPE_MAPSERVER, GtkMapserverPrivate))

typedef struct _GtkMapserverPrivate GtkMapserverPrivate;
struct _GtkMapserverPrivate
	{
		GooCanvasItem *root;
		GooCanvasItem *img;
		SoupSession *soup_session;

		gchar *url;

		gdouble sel_x_start;
		gdouble sel_y_start;

		GSource *sresize;
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
	gchar *localedir;

	GtkMapserverPrivate *priv = GTK_MAPSERVER_GET_PRIVATE (gtk_mapserver);

	priv->root = NULL;
	priv->img = NULL;
	priv->soup_session = NULL;

	priv->url = NULL;

	priv->sel_x_start = 0.0;
	priv->sel_y_start = 0.0;

	priv->sresize = NULL;

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

	gtk_widget_set_can_focus (GTK_WIDGET (gtk_mapserver), TRUE);

	g_signal_connect (G_OBJECT (gtk_mapserver), "size-allocate",
	                  G_CALLBACK (gtk_mapserver_on_size_allocate), (gpointer)gtk_mapserver);

	g_signal_connect (G_OBJECT (gtk_mapserver), "button-press-event",
	                  G_CALLBACK (gtk_mapserver_on_button_press_event), (gpointer)gtk_mapserver);
	g_signal_connect (G_OBJECT (gtk_mapserver), "button-release-event",
	                  G_CALLBACK (gtk_mapserver_on_button_release_event), (gpointer)gtk_mapserver);
	g_signal_connect (G_OBJECT (gtk_mapserver), "motion-notify-event",
	                  G_CALLBACK (gtk_mapserver_on_motion_notify_event), (gpointer)gtk_mapserver);

	g_object_set (G_OBJECT (gtk_mapserver),
				  "background-color", "white",
				  NULL);

	priv->root = goo_canvas_get_root_item (GOO_CANVAS (gtk_mapserver));

	priv->img = goo_canvas_image_new (priv->root,
									  NULL,
									  0, 0,
									  NULL);

	goo_canvas_grab_focus (GOO_CANVAS (gtk_mapserver), priv->img);

	g_signal_connect (G_OBJECT (priv->img), "key-release-event",
					  G_CALLBACK (gtk_mapserver_on_key_release_event), (gpointer)gtk_mapserver);

	/* Soup */
	priv->soup_session = soup_session_sync_new_with_options (SOUP_SESSION_SSL_CA_FILE, NULL,
															 SOUP_SESSION_ADD_FEATURE_BY_TYPE, SOUP_TYPE_CONTENT_DECODER,
															 SOUP_SESSION_ADD_FEATURE_BY_TYPE, SOUP_TYPE_COOKIE_JAR,
															 SOUP_SESSION_USER_AGENT, "get ",
															 SOUP_SESSION_ACCEPT_LANGUAGE_AUTO, TRUE,
															 SOUP_SESSION_USE_NTLM, FALSE,
															 NULL);
}

/**
 * gtk_mapserver_new:
 *
 * Returns: the new created #GtkMapserver object.
 */
GtkWidget
*gtk_mapserver_new ()
{
	GtkWidget *gtk_mapserver = GTK_WIDGET (g_object_new (gtk_mapserver_get_type (), NULL));

	return gtk_mapserver;
}

/**
 * gtk_mapserver_get_soup_message:
 * @gtkm:
 * @url:
 *
 * Returns:
 */
SoupMessage
*gtk_mapserver_get_soup_message (GtkMapserver *gtkm,
								 const gchar *url)
{
	SoupMessage *msg;

	GtkMapserverPrivate *priv = GTK_MAPSERVER_GET_PRIVATE (gtkm);

	msg = NULL;

	msg = soup_message_new (SOUP_METHOD_GET, url);
	if (SOUP_IS_MESSAGE (msg))
		{
			soup_message_set_flags (msg, SOUP_MESSAGE_NO_REDIRECT);
			soup_session_send_message (priv->soup_session, msg);
		}

	if (!SOUP_IS_MESSAGE (msg) || !SOUP_STATUS_IS_SUCCESSFUL (msg->status_code))
		{
			g_warning ("Error on retrieving url: %s.", url);
			msg = NULL;
		}

	return msg;
}

/**
 * gtk_mapserver_get_extent:
 * @gtkm:
 * @url:
 *
 * Returns: a #GdkPibxbuf.
 */
GdkPixbuf
*gtk_mapserver_get_gdk_pixbuf (GtkMapserver *gtkm, const gchar *url)
{
	GdkPixbuf *ret;
	GError *error;
	SoupMessage *msg;
	GdkPixbufLoader *pxb_loader;

	ret = NULL;

	msg = gtk_mapserver_get_soup_message (gtkm, url);

	if (msg == NULL)
		{
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

			g_object_unref (msg);
		}
	if (pxb_loader != NULL)
		{
			ret = g_object_ref (gdk_pixbuf_loader_get_pixbuf (pxb_loader));
			g_object_unref (pxb_loader);
		}

	return ret;
}

/**
 * gtk_mapserver_set_home:
 * @gtkm:
 * @url:
 */
void
gtk_mapserver_set_home (GtkMapserver *gtkm,
						const gchar *url)
{
	GtkMapserverPrivate *priv = GTK_MAPSERVER_GET_PRIVATE (gtkm);

	g_return_if_fail (url != NULL);

	if (priv->url != NULL)
		{
			g_free (priv->url);
		}

	priv->url = g_strdup (url);

	gtk_mapserver_draw (gtkm);
}

/**
 * gtk_mapserver_get_extent:
 * @gtkm:
 * @url:
 *
 * Returns: a #GtkMapserverExtent. Mapserver must returns an html page in the form "minx miny maxx maxy".
 */
GtkMapserverExtent
*gtk_mapserver_get_extent (GtkMapserver *gtkm, const gchar *url)
{
	GtkMapserverExtent *ext;
	SoupMessage *msg;

	ext = NULL;

	msg = gtk_mapserver_get_soup_message (gtkm, url);
	if (msg != NULL)
		{
			gchar **coords;

			ext = (GtkMapserverExtent *)g_new0 (GtkMapserverExtent, 1);

			coords = g_strsplit (msg->response_body->data, " ", -1);
			ext->minx = g_strtod (coords[0], NULL);
			ext->miny = g_strtod (coords[1], NULL);
			ext->maxx = g_strtod (coords[2], NULL);
			ext->maxy = g_strtod (coords[3], NULL);

			g_strfreev (coords);

			g_object_unref (msg);
		}

	return ext;
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

static gboolean
gtk_mapserver_resize_timer (gpointer user_data)
{
	GtkMapserver *gtkm = (GtkMapserver *)user_data;
	GtkMapserverPrivate *priv = GTK_MAPSERVER_GET_PRIVATE (gtkm);

	if (gtk_widget_get_realized (GTK_WIDGET (gtkm)))
		{
			gtk_mapserver_draw (gtkm);
			g_source_destroy (priv->sresize);
			priv->sresize = NULL;
		}

	return TRUE;
}

static void
gtk_mapserver_draw (GtkMapserver *gtkm)
{
	GtkAllocation allocation;
	GdkPixbuf *pixbuf;

	gchar *_url;

	GtkMapserverPrivate *priv = GTK_MAPSERVER_GET_PRIVATE (gtkm);

	gtk_widget_get_allocation (GTK_WIDGET (gtkm), &allocation);

	_url = g_strdup_printf ("%s&MAPSIZE=%d %d",
							priv->url,
							allocation.width - allocation.x,
							allocation.height - allocation.y);

	pixbuf = gtk_mapserver_get_gdk_pixbuf (gtkm, _url);

	g_object_set (G_OBJECT (priv->img),
				  "pixbuf", pixbuf,
				  NULL);

	g_free (_url);
}

/* SIGNALS */
static void
gtk_mapserver_on_size_allocate (GtkWidget *widget,
								GdkRectangle *allocation,
								gpointer user_data)
{
	GtkMapserver *gtkm = (GtkMapserver *)user_data;
	GtkMapserverPrivate *priv = GTK_MAPSERVER_GET_PRIVATE (gtkm);

	if (!gtk_widget_get_realized (GTK_WIDGET (gtkm)))
		{
			return;
		}

	if (priv->sresize != NULL)
		{
			g_source_destroy (priv->sresize);
		}
	priv->sresize = g_timeout_source_new_seconds (1);
	g_source_set_callback (priv->sresize, gtk_mapserver_resize_timer, (gpointer)gtkm, NULL);
	g_source_attach (priv->sresize, NULL);
}

static gboolean
gtk_mapserver_on_key_release_event (GooCanvasItem *item,
									GooCanvasItem *target_item,
									GdkEventKey *event,
									gpointer user_data)
{
	switch (event->keyval)
		{
			case GDK_KEY_0:
			case GDK_KEY_KP_0:
				{
					GtkMapserverPrivate *priv = GTK_MAPSERVER_GET_PRIVATE ((GtkMapserver *)user_data);

					gdouble x;
					gdouble y;
					gdouble scale;
					gdouble rotation;

					goo_canvas_item_get_simple_transform (priv->img,
														  &x,
														  &y,
														  &scale,
														  &rotation);
					goo_canvas_item_set_simple_transform (priv->img,
														  x,
														  y,
														  1,
														  rotation);

					return TRUE;
				}

			case GDK_KEY_plus:
			case GDK_KEY_KP_Add:
				{
					GtkMapserverPrivate *priv = GTK_MAPSERVER_GET_PRIVATE ((GtkMapserver *)user_data);

					gdouble x;
					gdouble y;
					gdouble scale;
					gdouble rotation;

					goo_canvas_item_get_simple_transform (priv->img,
														  &x,
														  &y,
														  &scale,
														  &rotation);
					goo_canvas_item_set_simple_transform (priv->img,
														  x,
														  y,
														  scale + 0.1,
														  rotation);

					return TRUE;
				}

			case GDK_KEY_minus:
			case GDK_KEY_KP_Subtract:
				{
					GtkMapserverPrivate *priv = GTK_MAPSERVER_GET_PRIVATE ((GtkMapserver *)user_data);

					gdouble x;
					gdouble y;
					gdouble scale;
					gdouble rotation;

					goo_canvas_item_get_simple_transform (priv->img,
														  &x,
														  &y,
														  &scale,
														  &rotation);
					goo_canvas_item_set_simple_transform (priv->img,
														  x,
														  y,
														  scale - 0.1,
														  rotation);

					return TRUE;
				}
		}

	return FALSE;
}

static gboolean
gtk_mapserver_on_button_press_event (GtkWidget *widget,
									 GdkEventButton *event,
									 gpointer user_data)
{
	GtkMapserver *gtk_mapserver = GTK_MAPSERVER (user_data);
	GtkMapserverPrivate *priv = GTK_MAPSERVER_GET_PRIVATE (gtk_mapserver);

	goo_canvas_grab_focus (GOO_CANVAS (gtk_mapserver), priv->img);

	if (event->button == 1)
		{
			priv->sel_x_start = event->x;
			priv->sel_y_start = event->y;
		}
}

static gboolean
gtk_mapserver_on_button_release_event (GtkWidget *widget,
									   GdkEventButton *event,
									   gpointer user_data)
{
	GtkMapserver *gtk_mapserver = GTK_MAPSERVER (user_data);
	GtkMapserverPrivate *priv = GTK_MAPSERVER_GET_PRIVATE (gtk_mapserver);
}

static gboolean
gtk_mapserver_on_motion_notify_event (GtkWidget *widget,
									  GdkEventMotion *event,
									  gpointer user_data)
{
	gint x;
	gint y;
	GdkModifierType state;

	GtkMapserver *gtk_mapserver = GTK_MAPSERVER (user_data);
	GtkMapserverPrivate *priv = GTK_MAPSERVER_GET_PRIVATE (gtk_mapserver);

	if (event->is_hint)
		{
			gdk_window_get_device_position (event->window, event->device, &x, &y, &state);
		}
	else
		{
			x = event->x;
			y = event->y;
			state = event->state;
		}

	if (state & GDK_BUTTON1_MASK)
		{
			goo_canvas_item_translate (priv->img,
									   x - priv->sel_x_start,
									   y - priv->sel_y_start);

			priv->sel_x_start = x;
			priv->sel_y_start = y;
		}
}

/* UTILS */
