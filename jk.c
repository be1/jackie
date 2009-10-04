/* 
 * jk.c Copyright © 2009 by Benoît Rouits <brouits@free.fr>
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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <gtk/gtk.h>
#include <jack/jack.h>
#include <jack/statistics.h>
#include "jk.h"

/* writes the config to a file */
void jk_write_config(JkAppData* d) {
	GKeyFile* kf = NULL;
	gchar* data = NULL;
	gsize len;
	FILE* fp = NULL;
	
	kf = g_key_file_new();
	g_key_file_set_value (kf, "patchbay", "cmdline", d->patchbay_cmdline);
	g_key_file_set_value (kf, "transport", "cmdline", d->transport_cmdline);
	data = g_key_file_to_data(kf, &len, NULL);
	fp = fopen(d->config_path, "w");
	/* FIXME: check error (disk full, e.g.) */
	fputs(data, fp);
	fclose(fp);
	g_free(data);
}

void jk_read_config (JkAppData* d) {
	GKeyFile* kf = NULL;
	GError* error = NULL;

	kf = g_key_file_new();
	g_key_file_load_from_file(kf, d->config_path, G_KEY_FILE_NONE, &error);
	if(error) {
		g_free(d->patchbay_cmdline);
		d->patchbay_cmdline = g_strdup("/usr/bin/patchage");
		g_free(d->transport_cmdline);
		d->transport_cmdline = g_strdup("");
		
		jk_write_config(d);
		g_error_free(error);
		error = NULL;
		g_key_file_load_from_file(kf, d->config_path, G_KEY_FILE_NONE, &error);
	}
	d->patchbay_cmdline = g_key_file_get_value(kf, "patchbay", "cmdline", &error);
	if(error) {
		g_warning("read_config: %s\n", error->message);
		g_error_free(error);
		error = NULL;
	}
	d->transport_cmdline = g_key_file_get_value(kf, "transport", "cmdline", &error);
	if(error) {
		g_warning("read_config: %s\n", error->message);
		g_error_free(error);
		error = NULL;
	}
	g_key_file_free(kf);

	if (!d->patchbay_cmdline || d->patchbay_cmdline[0] == '\0') {
		g_free(d->patchbay_cmdline);
		d->patchbay_cmdline = g_strdup("/usr/bin/patchage");
	}
	if (!d->transport_cmdline || d->transport_cmdline[0] == '\0') {
		g_free(d->transport_cmdline);
		d->transport_cmdline = g_strdup("");
	}
	jk_write_config(d);
}

void jk_quit(JkAppData* d) {
	g_free(d->config_path);
	gtk_main_quit();
}

/* spawn a external program */
gboolean jk_spawn_application(const gchar* cmdline) {
	gchar** argv = NULL;
	gboolean ret;
	GPid pid = (GPid)0;
	GError* error = NULL;

	argv = g_strsplit(cmdline, " ", 0);

	ret = g_spawn_async (NULL, argv, NULL, 0, NULL, NULL, &pid, &error);
	if(ret == FALSE) {
		g_warning("jk_spawn_application: %s\n", error->message);
		g_error_free(error);
		error=NULL;
	}
	g_strfreev(argv);
	return ret;
}

