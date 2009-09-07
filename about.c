/* 
 * about.c Copyright © 2009 by Benoît Rouits <brouits@free.fr>
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

/* destroy the about window on response GTK_RESPONSE_CANCEL */
void about_on_response (GtkDialog* dialog, gint response_id, gpointer unused)
{
	unused = NULL;
	if (response_id == GTK_RESPONSE_CANCEL) {
		gtk_widget_hide(GTK_WIDGET(dialog));
		gtk_widget_destroy(GTK_WIDGET(dialog));
	}
	else
		g_message ("about_on_response: %d was given", response_id);
	return;
}

GtkAboutDialog*
about_create (const gchar* name, const gchar* version, const gchar* copyright,
	const gchar* comment, const gchar* license, const gchar* website,
	const gchar** authors)
{
	GtkWidget* aboutdialog;

	aboutdialog = gtk_about_dialog_new ();
	/* gtk_container_set_border_width (GTK_CONTAINER (aboutdialog), 5); */
	gtk_about_dialog_set_version (GTK_ABOUT_DIALOG (aboutdialog), version);
	gtk_about_dialog_set_program_name (GTK_ABOUT_DIALOG (aboutdialog), name);
	gtk_about_dialog_set_copyright (GTK_ABOUT_DIALOG (aboutdialog), copyright);
	gtk_about_dialog_set_comments (GTK_ABOUT_DIALOG (aboutdialog), comment);
	gtk_about_dialog_set_license (GTK_ABOUT_DIALOG (aboutdialog), license);
	gtk_about_dialog_set_website (GTK_ABOUT_DIALOG (aboutdialog), website);
#if 0
	gtk_about_dialog_set_website_label (GTK_ABOUT_DIALOG (aboutdialog), website_label);
#endif
	gtk_about_dialog_set_authors (GTK_ABOUT_DIALOG (aboutdialog), authors);
	gtk_about_dialog_set_logo (GTK_ABOUT_DIALOG (aboutdialog), NULL);
	/* call about_on_response on [Close] button-press */
	g_signal_connect(aboutdialog, "response", G_CALLBACK(about_on_response), NULL);
	return GTK_ABOUT_DIALOG(aboutdialog);
}

void about_show (GtkAboutDialog* dialog)
{
	gtk_widget_show(GTK_WIDGET(dialog));
}

void about_hide (GtkAboutDialog* dialog)
{
	gtk_widget_hide(GTK_WIDGET(dialog));
}

void about_destroy (GtkAboutDialog* dialog)
{
	gtk_widget_destroy(GTK_WIDGET(dialog));
}

