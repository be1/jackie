/* 
 * about.h Copyright © 2009 by Benoît Rouits <brouits@free.fr>
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

/* create an about widget:
 *
 * authors: a pointer to strings, NULL terminated
 * copyright: a copyright line
 * comment: what does that program
 * license: disclaimer + where to find the full license
 * website: url to the project homepage
 */
GtkAboutDialog* about_create (const gchar* name,
			const gchar* version,
			const gchar* copyright,
			const gchar* comment,
			const gchar* license,
			const gchar* website,
			const gchar** authors);

void about_show (GtkAboutDialog* dialog);

void about_hide (GtkAboutDialog* dialog);

void about_destroy (GtkAboutDialog* dialog);

