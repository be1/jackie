/* 
 * callback.c Copyright © 2009 by Benoît Rouits <brouits@free.fr>
 * Published under the terms of the GNU General Public License v2 (GPLv2).
 * 
 ************************************************* 
 * jackie: a small jack daemon startup interface *
 *************************************************
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, 
 * Boston, MA  02110-1301, USA.
 * 
 * see the COPYING file included in the jackie package or
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt for the full licence
 * 
 */

#include <unistd.h>
#include <gtk/gtk.h>
#include "jk.h"
#include "menu.h"
#include "about.h"
#include "version.h"
#include "window.h"

/* handler for left-button click */
void tray_icon_on_left_click(GtkStatusIcon* instance, gpointer app_data)
{
	JkAppData* d = (JkAppData*) app_data;
	guint button;

	button = 1;
	gtk_status_icon_set_blinking(instance, FALSE);
	if (app_data)
		menu_show(GTK_MENU(d->left_menu), button, gtk_get_current_event_time());
}

/* handler for right-button click */
void tray_icon_on_right_click(GtkStatusIcon* instance, guint button, guint activate_time, gpointer app_data)
{
	JkAppData* d = (JkAppData*) app_data;

	gtk_status_icon_set_blinking(instance, FALSE);
	if (app_data)
		menu_show(GTK_MENU(d->right_menu), button, activate_time);
}

/* handler for the "Start" menu item */
void menu_item_on_start_stop(GtkMenuItem* instance, gpointer app_data)
{
	JkAppData* d = (JkAppData*) app_data;
	gboolean ret;

	/* check if jackd is running */
	if (d->jackd_client) { /* stop jackd */
		jack_client_close(d->jackd_client);
		d->jackd_client = NULL;
		if (d->jackd_pid) {
			kill (d->jackd_pid, 2); /* SIGINT */
			d->jackd_pid = (GPid)0;
		}
		gtk_status_icon_set_tooltip(d->tray_icon, "Jackd Stopped");
		gtk_menu_item_set_label(instance, "Start");
		return;
	}

	/* else reload config then start jackd */
	jk_read_config(d);
	if (!d->jackd_cmdline) { /* jackd group not found */
		gtk_status_icon_set_tooltip(d->tray_icon, "Missing jackd in configuration");
		return;
	}
	ret = jk_spawn_jackd(d); /* start jackd */
	/* try to connect an existing jackd */
	d->jackd_client = jack_client_open("jackie", JackNoStartServer, &d->jackd_status);

	if (!d->jackd_client) { /* assume bad commandline */
		gtk_status_icon_set_tooltip(d->tray_icon, "Bad jackd command line");
		d->jackd_pid = (GPid)0;
	} else {
		gtk_status_icon_set_tooltip(d->tray_icon, "Jackd started");
		gtk_menu_item_set_label(instance, "Stop");
	}
}

/* handler for the "About" menu item (see version.h) */
void menu_item_on_about(GtkMenuItem* instance, gpointer unused)
{
	GtkAboutDialog* about;
	const gchar* authors [] = {
		PROG_AUTHOR0,
		NULL
	};

	about = about_create (PROG_NAME, PROG_VERSION, PROG_COPYRIGHT,
				PROG_COMMENT, PROG_LICENSE, PROG_WEBSITE,
				authors);
	about_show(about);
	unused = NULL; /* avoid compiler warnings */
	instance = NULL; /* _ */
	return;
}

/* handler for the "Patchbay" menu item */
void menu_item_on_patch(GtkMenuItem* instance, gpointer app_data) {
	JkAppData* d = (JkAppData*)app_data;
	gboolean ret;

	jk_read_config(d);
	ret = jk_spawn_application(d->patchbay_cmdline);
	if (ret == FALSE)
		gtk_status_icon_set_tooltip(d->tray_icon, "Failed to launch application");
		
	instance = NULL;
}

/* handler for the "Transport" menu item */
void menu_item_on_trans(GtkMenuItem* instance, gpointer app_data) {
	JkAppData* d = (JkAppData*)app_data;
	gboolean ret;

	jk_read_config(d);
	ret = jk_spawn_application(d->transport_cmdline);
	if (ret == FALSE)
		gtk_status_icon_set_tooltip(d->tray_icon, "Failed to launch application");
		
	instance = NULL;
}

/* callback on Preferences window closed */
void on_pref_close (gpointer app_data) {
	JkAppData* d = (JkAppData*)app_data;
	const gchar* text;

	text = gtk_entry_get_text(d->jackd_entry);
	g_free(d->jackd_cmdline);
	d->jackd_cmdline = g_strdup(text); /* set jackd command line */
	text = gtk_entry_get_text(d->patchbay_entry);
	g_free(d->patchbay_cmdline);
	d->patchbay_cmdline = g_strdup(text); /* set patchbay command line */
	/* write it to $HOME/.jackie */
	jk_write_config(d);
	gtk_widget_hide(GTK_WIDGET(d->pref_window));
	gtk_widget_destroy(GTK_WIDGET(d->pref_window));
	d->pref_window = NULL;
}
/* handler for the "Preference" menu item */
void menu_item_on_pref(GtkMenuItem* instance, gpointer app_data) {
	JkAppData* d = (JkAppData*)app_data;
	GtkVBox* vbox;
	GtkHBox* hbox1;
	GtkHBox* hbox2;
	GtkLabel* jackd_label;
	GtkLabel* patchbay_label;

	d->pref_window = window_create("Preferences");
	vbox = GTK_VBOX(gtk_vbox_new(FALSE, 0));
	gtk_container_add(GTK_CONTAINER(d->pref_window), GTK_WIDGET(vbox));

	hbox1 = GTK_HBOX(gtk_hbox_new(TRUE, 0));
	gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(hbox1), TRUE, FALSE, 0);

	hbox2 = GTK_HBOX(gtk_hbox_new(TRUE, 0));
	gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(hbox2), TRUE, FALSE, 0);

	jackd_label = GTK_LABEL(gtk_label_new("Jackd command line"));
	gtk_box_pack_start(GTK_BOX(hbox1), GTK_WIDGET(jackd_label), FALSE, FALSE, 0);
	patchbay_label = GTK_LABEL(gtk_label_new("patchbay command line"));
	gtk_box_pack_start(GTK_BOX(hbox2), GTK_WIDGET(patchbay_label), FALSE, FALSE, 0);

	d->jackd_entry = GTK_ENTRY(gtk_entry_new());
	gtk_entry_set_text(d->jackd_entry, d->jackd_cmdline); /* jackd command line */
	gtk_box_pack_start(GTK_BOX(hbox1), GTK_WIDGET(d->jackd_entry), FALSE, FALSE, 0);
	d->patchbay_entry = GTK_ENTRY(gtk_entry_new());
	gtk_entry_set_text(d->patchbay_entry, d->patchbay_cmdline); /* patchbay command line */
	gtk_box_pack_start(GTK_BOX(hbox2), GTK_WIDGET(d->patchbay_entry), FALSE, FALSE, 0);
	
	g_signal_connect_swapped(G_OBJECT(d->pref_window), "delete-event", G_CALLBACK(on_pref_close), (gpointer)d);

	gtk_widget_show_all(GTK_WIDGET(d->pref_window));
	instance = NULL; /* avoid warnings */
}

/* handler for the "Quit" menu item */
void menu_item_on_quit(GtkMenuItem* instance, gpointer app_data)
{
	JkAppData* d = (JkAppData*) app_data;
	jk_quit(d);
	instance = NULL; /* useless but does not warn at compile time */
}

