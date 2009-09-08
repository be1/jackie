/* 
 * main.c Copyright © 2009 by Benoît Rouits <brouits@free.fr>
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <libgen.h>
#include <glib.h>
#include <gtk/gtk.h>
#include "jk.h"
#include "menu.h"
#include "callback.h"
#include "version.h"

/* cli usage message */
void usage(char* prog, int exitcode) {
	fprintf(stderr, "%s version %s\n", PROG_NAME, PROG_VERSION);
	fprintf(stderr, "Usage: %s\n", basename(prog));
	exit(exitcode);
}

/* here we are */
int main(int argc, char **argv)
{
	GtkStatusIcon* tray_icon = NULL;	/* system tray icon object */
	GtkMenu* left_menu = NULL;		/* left-click menu */
	GtkMenu* right_menu = NULL;		/* right-click menu */
	JkAppData* app_data = NULL;		/* data to pass to callbacks */
	const gchar* const * data_dirs;		/* dirs list for finding icons, etc... */
	gchar* icon_path = NULL;
	int i;

	/* set $HOME/.jackie.ini s config file */

	app_data = (JkAppData*) malloc (sizeof(JkAppData));
	app_data->config_path = g_build_path ("/", g_get_home_dir(), ".jackie", NULL);
	app_data->progs = jk_read_config(app_data->config_path); /* must be called first */
	app_data->jackd_cmdline = jk_read_jackd_cmdline(app_data->config_path);
	app_data->jackd_pid = (GPid)0;
	app_data->jackd_error = NULL;
	app_data->jackd_entry = NULL;

	/* parse cli argument */ 
	gtk_init(&argc, &argv);
#if 0
	switch (argc) {
		case 1:
			break;
		default:
			usage(argv[0], EXIT_FAILURE);
	}
#endif
	/* create left-click menu */
	left_menu = menu_new();
	app_data->left_menu = left_menu;

	/* create right-click menu */
	right_menu = menu_new();
	app_data->right_menu = right_menu;

	/* create the tray icon */
        tray_icon = gtk_status_icon_new();
	app_data->tray_icon = tray_icon;

	/* systray visible icon */
	data_dirs = g_get_system_data_dirs();
	for (i = 0; data_dirs[i]; ++i) {
		icon_path = g_strconcat(data_dirs[i],"icons/jackie.png", NULL);
		if (!access(icon_path,F_OK)) {
			break;
		}
		g_free(icon_path);
		icon_path = NULL;
	}
	if (icon_path) {
	        gtk_status_icon_set_from_file(tray_icon, icon_path);
		/* default wm icon (for subsequent windows) */
	        gtk_window_set_default_icon_from_file(icon_path, NULL);
		g_free(icon_path);
	}
	/* default tooltip */
        gtk_status_icon_set_tooltip(tray_icon, "jackie");
	/* set the icon visible */
        gtk_status_icon_set_visible(tray_icon, TRUE);

	/* connect its callbacks */
        g_signal_connect(G_OBJECT(tray_icon), "activate", 
                         G_CALLBACK(tray_icon_on_left_click), app_data);
	g_signal_connect(G_OBJECT(tray_icon), "popup-menu",
                         G_CALLBACK(tray_icon_on_right_click), app_data);

	/* left menu item callbacks */
	app_data->startstop = menu_append_item(left_menu, "Start", G_CALLBACK(menu_item_on_start_stop), app_data);

	/* right menu items callbacks */
#if 0
	menu_append_item(right_menu, "Patchbay", G_CALLBACK(menu_item_on_trans), app_data);
	menu_append_item(right_menu, "Transport", G_CALLBACK(menu_item_on_trans), app_data);
#endif
	menu_append_item(right_menu, "Preferences", G_CALLBACK(menu_item_on_pref), app_data);
	menu_append_item(right_menu, "About", G_CALLBACK(menu_item_on_about), app_data);
	menu_append_item(right_menu, "Quit", G_CALLBACK(menu_item_on_quit), app_data);

	/* run */
        gtk_main();

        exit(EXIT_SUCCESS);
}

