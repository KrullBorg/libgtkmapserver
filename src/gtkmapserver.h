/*
 *  gtkmapserver.h
 *
 *  Copyright (C) 2015 Andrea Zagli <azagli@libero.it>
 *
 *  This file is part of libgtkmapserver.
 *
 *  libgdaex is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  libgdaex is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with libgdaex; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef __GTK_MAPSERVER_H__
#define __GTK_MAPSERVER_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include <goocanvas.h>
#include <libsoup/soup.h>


G_BEGIN_DECLS


#define GTK_TYPE_MAPSERVER                 (gtk_mapserver_get_type ())
#define GTK_MAPSERVER(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_MAPSERVER, GtkMapserver))
#define GTK_MAPSERVER_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_MAPSERVER, GtkMapserverClass))
#define GTK_IS_MAPSERVER(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_MAPSERVER))
#define GTK_IS_MAPSERVER_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_MAPSERVER))
#define GTK_MAPSERVER_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_MAPSERVER, GtkMapserverClass))


typedef struct _GtkMapserver GtkMapserver;
typedef struct _GtkMapserverClass GtkMapserverClass;

struct _GtkMapserver
	{
		GooCanvas parent;
	};

struct _GtkMapserverClass
	{
		GooCanvasClass parent_class;
	};

GType gtk_mapserver_get_type (void) G_GNUC_CONST;


GtkWidget *gtk_mapserver_new (void);

SoupMessage *gtk_mapserver_get_soup_message (GtkMapserver *gtkm,
											 const gchar *url);

GdkPixbuf *gtk_mapserver_get_gdk_pixbuf (GtkMapserver *gtkm, const gchar *url);

typedef struct
	{
		gdouble minx;
		gdouble miny;
		gdouble maxx;
		gdouble maxy;
	} GtkMapserverExtent;

GtkMapserverExtent *gtk_mapserver_get_extent (GtkMapserver *gtkm, const gchar *url);

void gtk_mapserver_set_home (GtkMapserver *gtkm, const gchar *url, GtkMapserverExtent *ext);


G_END_DECLS

#endif /* __GTK_MAPSERVER_H__ */
