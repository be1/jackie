/* 
 * callback.h Copyright © 2009 by Benoît Rouits <brouits@free.fr>
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

#ifndef _CB_H
#define _CB_H
#include <gtk/gtk.h>

/* handler for left-button click */
void tray_icon_on_left_click(GtkStatusIcon* instance, gpointer app_data);

/* handler for right-button click */
void tray_icon_on_right_click(GtkStatusIcon* instance, guint button, guint activate_time, gpointer app_data);

/* handler for the "Start/Stop" menu item */
void menu_item_on_start_stop(GtkMenuItem* instance, gpointer app_data);

/* handler for the "Edit" menu item */
void menu_item_on_pref(GtkMenuItem* instance, gpointer app_data);

/* handler for the "Patch" menu item */
void menu_item_on_patch(GtkMenuItem* instance, gpointer app_data);

/* handler for the "Trans" menu item */
void menu_item_on_trans(GtkMenuItem* instance, gpointer app_data);

/* handler for the "About" menu item */
void menu_item_on_about(GtkMenuItem* instance, gpointer app_data);

/* handler for the "Quit" menu item */
void menu_item_on_quit(GtkMenuItem* instance, gpointer app_data);
#endif /* _CB_H */
