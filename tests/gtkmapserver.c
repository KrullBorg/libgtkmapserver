/*
 * Copyright (C) 2015 Andrea Zagli <azagli@libero.it>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <stdlib.h>

#include "gtkmapserver.h"

/* This is our handler for the "delete-event" signal of the window, which
 is emitted when the 'x' close button is clicked. We just exit here. */
static gboolean
on_delete_event (GtkWidget *window,
               GdkEvent  *event,
               gpointer   unused_data)
{
	exit (0);
}

int
main (int argc, char **argv)
{
	GtkWidget *window;
	GtkWidget *gtkmap;

	/* Initialize GTK+. */
	gtk_init (&argc, &argv);

	/* Create the window and widgets. */
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size (GTK_WINDOW (window), 640, 600);
	g_signal_connect (window,
					  "delete_event", G_CALLBACK (on_delete_event),
		              NULL);

	gtkmap = gtk_mapserver_new ();
	gtk_container_add (GTK_CONTAINER (window), gtkmap);

	gtk_widget_show_all (window);

	gtk_mapserver_set_home (GTK_MAPSERVER (gtkmap), "http://atlante/cgi-bin/mapserv?map=/var/www_mapper/www_pm4/config/cdu/RU_cdu.map&mode=map&layers=catasto");

	/* Pass control to the GTK+ main event loop. */
	gtk_main ();

	return 0;
}
