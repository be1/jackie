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

#include <gtk/gtk.h>
#include "jk.h"
#include "menu.h"
#include "about.h"
#include "version.h"

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
	GSList* pr;
	gboolean ret;
	JkProg* p;

	for (pr = d->progs; pr; pr = g_slist_next(pr)) {
		p = (JkProg*)pr->data;
		if (!g_strcmp0("jackd",p->name))
			break;
	}
	if (!pr) { /* jackd group not found */
		gtk_status_icon_set_tooltip(d->tray_icon, "Missing jackd in configuration");
		return;
	}
	p = (JkProg*)pr->data;
	/* check if jackd is running */
	if (p->pid) { /* stop jackd */
		kill (p->pid, 3); /* SIGQUIT */
		p->pid = (GPid)0;
		gtk_status_icon_set_tooltip(d->tray_icon, "Jackd Stopped");
		gtk_menu_item_set_label(instance, "Start");
		g_source_remove(p->timetag);
		return;
	}

	/* else reload config then start jackd */
	jk_delete_progs(d->progs);
	d->progs = jk_read_config(d->config_path);
	for (pr = d->progs; pr; pr = g_slist_next(pr)) {
		p = (JkProg*)pr->data;
		if (!g_strcmp0("jackd",p->name))
			break;
	}
	if (!pr) { /* jackd group not found */
		gtk_status_icon_set_tooltip(d->tray_icon, "Missing jackd in configuration");
		return;
	}
	p = (JkProg*)pr->data;
	ret = jk_spawn_prog(p); /* start jackd */
	if (ret == FALSE) { /* bad commandline */
		gtk_status_icon_set_tooltip(d->tray_icon, "Bad jackd command line");
		p->pid = (GPid)0;
	} else {
		p = (JkProg*)pr->data;
		gtk_status_icon_set_tooltip(d->tray_icon, "Jackd started");
		gtk_menu_item_set_label(instance, "Stop");
		p->timetag = g_timeout_add_seconds(1, jk_update_tooltip, app_data);
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

/* handler for the "Patch" menu item */
void menu_item_on_patch(GtkMenuItem* instance, gpointer app_data) {
	/* FIXME: launch patchbay */
	instance = app_data = NULL;
}

/* handler for the "Trans" menu item */
void menu_item_on_trans(GtkMenuItem* instance, gpointer app_data) {
	/* FIXME: launch transport control */
	instance = app_data = NULL;
}

/* handler for the "Edit" menu item */
void menu_item_on_edit(GtkMenuItem* instance, gpointer app_data) {
	/* FIXME: open the configuration editor */
	instance = app_data = NULL;
}

/* handler for the "Quit" menu item */
void menu_item_on_quit(GtkMenuItem* instance, gpointer app_data)
{
	JkAppData* d = (JkAppData*) app_data;
	jk_quit(d);
	instance = NULL; /* useless but does not warn at compile time */
}

