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

typedef struct {
	gchar* name;		/* prog name (key) */
	gchar* cmdline;		/* commandline (value) */
	GPid pid;		/* pid of process */
	gint in;		/* process stdin */
	gint out;		/* process stdout */
	gint err;		/* process stderr */
	gint ret;		/* process return */
	GError* error;		/* GLib error details */
} JkProg;

/* data to pass to the callbacks */
typedef struct {
	gchar* config_path;		/* configuration path */
	GSList* progs;		/* configuration structure  */
	GtkMenu* left_menu;		/* gtk left menu */
	GtkMenu* right_menu;		/* gtk right menu */
	GtkStatusIcon* tray_icon;	/* gtk tray icon */
} JkAppData;

/* creates a JkConfig given the config file */
GSList* jk_read_config (gchar* config_path);

/* deletes a JkConfig */
void jk_delete_progs(GSList* config);

/* spawn a program */
gboolean jk_spawn_prog(JkProg* prog);

/* quit application */
void jk_quit(JkAppData* d);
#endif
