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
#include <jack/jack.h>

/* data to pass to the callbacks */
typedef struct {
	gchar* config_path;		/* configuration path */
	GtkMenu* left_menu;		/* gtk left menu */
	GtkMenu* right_menu;		/* gtk right menu */
	GtkMenuItem* startstop;		/* start/stop menu item */
	GtkStatusIcon* tray_icon;	/* gtk tray icon */
	GtkWindow* pref_window;		/* gtk preference window */
	GtkEntry* patchbay_entry;	/* command line entry for patchbay */
	GtkEntry* transport_entry;	/* command line entry for transport */
	gchar* patchbay_cmdline;		/* command_line text for patchbay */
	gchar* transport_cmdline;		/* command_line text for transport */
	jack_client_t* jackd_client;	/* the jackd handle */
	jack_status_t jackd_status;	/* the jackd status */
	int xrun;
} JkAppData;

/* creates a JkConfig given the config file */
void jk_read_config (JkAppData* d);

/* spawn application */
gboolean jk_spawn_application(const char* cmdline);

/* write config into file */
void jk_write_config(JkAppData* d);

/* quit application */
void jk_quit(JkAppData* d);
#endif
