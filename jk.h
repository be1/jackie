/* 
 * jk.h Copyright © 2009 by Benoît Rouits <brouits@free.fr>
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

#ifndef _JK_H
#define _JK_H
#include <gtk/gtk.h>
#include <stdio.h>

typedef struct {
	gchar* name;		/* prog name (key) */
	gchar* cmdline;		/* commandline (value) */
	GError* error;		/* GLib error details */
} JkProg;

/* data to pass to the callbacks */
typedef struct {
	gchar* config_path;		/* configuration path */
	GSList* progs;			/* configuration list  */
	GtkMenu* left_menu;		/* gtk left menu */
	GtkMenu* right_menu;		/* gtk right menu */
	GtkStatusIcon* tray_icon;	/* gtk tray icon */
	gchar tooltip_buffer[BUFSIZ];	/* working copy of tooltip */
	gchar* jackd_cmdline;		/* pid of jackd process */
	GPid jackd_pid;			/* pid of jackd process */
	gint jackd_in;			/* process stdin */
	gint jackd_out;			/* process stdout */
	gint jackd_err;			/* process stderr */
	gint jackd_ret;			/* process return */
	guint jackd_chld;		/* GLib source id for child watch */
	guint jackd_fd;			/* GLib source id for file watch */
	GError* jackd_error;		/* GLib error details */
} JkAppData;

/* creates jackd command line given the config file */
gchar* jk_read_jackd_cmdline(gchar* config_path);

/* creates a JkConfig given the config file */
GSList* jk_read_config (gchar* config_path);

/* deletes a JkConfig */
void jk_delete_progs(GSList* config);

/* spawn jackd */
gboolean jk_spawn_jackd(JkAppData* d);

/* update tooltip */
gboolean jk_update_tooltip (gpointer app_data);

/* quit application */
void jk_quit(JkAppData* d);
#endif
