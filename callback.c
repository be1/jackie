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
#include <libintl.h>
#include <locale.h>
#include <gtk/gtk.h>
#include <jack/jack.h>
#include <jack/statistics.h>
#include "jk.h"
#include "menu.h"
#include "about.h"
#include "version.h"
#include "window.h"
#define _(string) gettext (string)

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

void jk_on_jackd_error(const char* msg) {
	puts(msg);
}

void jk_on_jackd_info(const char* msg) {
	puts(msg);
}

int jk_on_jackd_xrun(void* app_data) {
	JkAppData* d = (JkAppData*) app_data;
	gchar* buf = NULL;

	buf = g_strdup_printf(_("xrun: %d. last: %.0f usecs"), ++d->xrun, jack_get_xrun_delayed_usecs(d->jackd_client)); 
	gtk_status_icon_set_tooltip (d->tray_icon, buf);
	g_free(buf);
	return 0;
}

/* handler for the "Start" menu item */
void menu_item_on_start_stop(GtkMenuItem* instance, gpointer app_data)
{
	JkAppData* d = (JkAppData*) app_data;
	static GtkWidget* img = NULL;

	/* check if jackd is running */
	if (d->jackd_client) { /* stop jackd */
		jack_client_close(d->jackd_client);
		d->jackd_client = NULL;
		gtk_status_icon_set_tooltip(d->tray_icon, _("Disconnected"));
		if (img) {
			gtk_widget_destroy(img);
			img = NULL;
		}
		img = gtk_image_new_from_stock(GTK_STOCK_CONNECT, GTK_ICON_SIZE_MENU);
		gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(instance), img);
		gtk_menu_item_set_label(instance, "gtk-connect");
		gtk_image_menu_item_set_use_stock(GTK_IMAGE_MENU_ITEM(instance), TRUE);
		return;
	}

	/* else reload config then start jackd */
	d->jackd_client = jack_client_open("jackie", JackNullOption, &d->jackd_status);

	if (!d->jackd_client) {
		gtk_status_icon_set_tooltip(d->tray_icon, _("Could not connect or start Jackd"));
	} else {
		/* set info/error callbacks */
		jack_set_error_function(jk_on_jackd_error);
		jack_set_info_function(jk_on_jackd_info);
		jack_set_xrun_callback(d->jackd_client, jk_on_jackd_xrun, app_data);
		jack_activate(d->jackd_client);

		gtk_status_icon_set_tooltip(d->tray_icon, _("Connected"));
		if (img) {
			gtk_widget_destroy(img);
			img = NULL;
		}
		img = gtk_image_new_from_stock(GTK_STOCK_DISCONNECT, GTK_ICON_SIZE_MENU);
		gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(instance), img);
		gtk_menu_item_set_label(instance, "gtk-disconnect");
		gtk_image_menu_item_set_use_stock(GTK_IMAGE_MENU_ITEM(instance), TRUE);
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
		gtk_status_icon_set_tooltip(d->tray_icon, _("Failed to launch application"));
		
	instance = NULL;
}

/* handler for the "Transport" menu item */
void menu_item_on_trans(GtkMenuItem* instance, gpointer app_data) {
	JkAppData* d = (JkAppData*)app_data;
	gboolean ret;

	jk_read_config(d);
	ret = jk_spawn_application(d->transport_cmdline);
	if (ret == FALSE)
		gtk_status_icon_set_tooltip(d->tray_icon, _("Failed to launch application"));
		
	instance = NULL;
}

/* callback on Preferences window closed */
void on_pref_close (gpointer app_data) {
	JkAppData* d = (JkAppData*)app_data;
	const gchar* text;

	text = gtk_entry_get_text(d->patchbay_entry);
	g_free(d->patchbay_cmdline);
	d->patchbay_cmdline = g_strdup(text); /* set patchbay command line */
	
	text = gtk_entry_get_text(d->transport_entry);
	g_free(d->transport_cmdline);
	d->transport_cmdline = g_strdup(text); /* set transport command line */
	
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
	GtkLabel* transport_label;
	GtkLabel* patchbay_label;

	d->pref_window = window_create(_("Preferences"));
	vbox = GTK_VBOX(gtk_vbox_new(FALSE, 0));
	gtk_container_add(GTK_CONTAINER(d->pref_window), GTK_WIDGET(vbox));

	hbox1 = GTK_HBOX(gtk_hbox_new(TRUE, 0));
	gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(hbox1), TRUE, FALSE, 0);

	hbox2 = GTK_HBOX(gtk_hbox_new(TRUE, 0));
	gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(hbox2), TRUE, FALSE, 0);

	patchbay_label = GTK_LABEL(gtk_label_new(_("Patchbay command line")));
	gtk_box_pack_start(GTK_BOX(hbox1), GTK_WIDGET(patchbay_label), FALSE, FALSE, 0);
	transport_label = GTK_LABEL(gtk_label_new(_("Transport command line")));
	gtk_box_pack_start(GTK_BOX(hbox2), GTK_WIDGET(transport_label), FALSE, FALSE, 0);

	jk_read_config(d);

	d->patchbay_entry = GTK_ENTRY(gtk_entry_new());
	gtk_entry_set_text(d->patchbay_entry, d->patchbay_cmdline); /* patchbay command line */
	gtk_box_pack_start(GTK_BOX(hbox1), GTK_WIDGET(d->patchbay_entry), FALSE, FALSE, 0);
	d->transport_entry = GTK_ENTRY(gtk_entry_new());
	gtk_entry_set_text(d->transport_entry, d->transport_cmdline); /* transport command line */
	gtk_box_pack_start(GTK_BOX(hbox2), GTK_WIDGET(d->transport_entry), FALSE, FALSE, 0);
	
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

