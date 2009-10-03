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
#include "jk.h"

/* writes the config to a file */
void jk_write_config(JkAppData* d) {
	GKeyFile* kf = NULL;
	gchar* data = NULL;
	gsize len;
	FILE* fp = NULL;
	
	kf = g_key_file_new();
	g_key_file_set_value (kf, "jackd", "cmdline", d->jackd_cmdline);
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
		g_free(d->jackd_cmdline);
		d->jackd_cmdline = g_strdup("/usr/bin/jackd -R -d alsa");
		g_free(d->patchbay_cmdline);
		d->patchbay_cmdline = g_strdup("patchage");
		g_free(d->transport_cmdline);
		d->transport_cmdline = g_strdup("");
		
		jk_write_config(d);
		g_error_free(error);
		error = NULL;
		g_key_file_load_from_file(kf, d->config_path, G_KEY_FILE_NONE, &error);
	}
	d->jackd_cmdline = g_strdup(g_key_file_get_value (kf, "jackd", "cmdline", &error));
	if(error) {
		g_warning("read_config: %s\n", error->message);
		g_error_free(error);
		error = NULL;
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

	if (!d->jackd_cmdline || d->jackd_cmdline[0] == '\0') {
		g_free(d->jackd_cmdline);
		d->jackd_cmdline = g_strdup("/usr/bin/jackd -R -d alsa");
	}
	if (!d->patchbay_cmdline || d->patchbay_cmdline[0] == '\0') {
		g_free(d->patchbay_cmdline);
		d->patchbay_cmdline = g_strdup("patchage");
	}
	if (!d->transport_cmdline || d->transport_cmdline[0] == '\0') {
		g_free(d->transport_cmdline);
		d->transport_cmdline = g_strdup("");
	}
}

void jk_quit(JkAppData* d) {
	g_free(d->config_path);
	gtk_main_quit();
}

/* watch return of a program */
void jk_on_jackd_sigchld(GPid pid, gint status, gpointer app_data) {
	JkAppData* d = (JkAppData*)app_data;

	close(d->jackd_in);
	close(d->jackd_out);
	close(d->jackd_err);
	g_source_remove(d->jackd_fd);
	g_source_remove(d->jackd_chld);
	g_spawn_close_pid(pid);

	if (status) {
		d->jackd_pid = (GPid)0;
		gtk_menu_item_set_label(d->startstop, "Start");
		gtk_status_icon_set_tooltip(d->tray_icon, "Jackd stopped");
	}
}

gboolean jk_on_data(GIOChannel* source, GIOCondition condition, gpointer app_data) {
	JkAppData* d = (JkAppData*) app_data;
	gint fd;
	static guint xrun = 0;

	fd = g_io_channel_unix_get_fd(source);
	if (condition == G_IO_IN) {
		ssize_t nread;

		/* read jackd messages */
		nread = read(fd, d->tooltip_buffer, BUFSIZ);
		if (nread < BUFSIZ)
			d->tooltip_buffer[nread] = '\0';
		else d->tooltip_buffer[BUFSIZ-1] = '\0';
#if 0
		puts(d->tooltip_buffer);
#endif
		/* update tooltip_buffer */
		if (strstr(d->tooltip_buffer, "xrun") /* ALSA */
		|| strstr(d->tooltip_buffer, "delay of")) { /* OSS */
			snprintf(d->tooltip_buffer, BUFSIZ, "Jackd xruns: %d", ++xrun);
			gtk_status_icon_set_tooltip (d->tray_icon, d->tooltip_buffer);
		} else if (strstr(d->tooltip_buffer, "jack main caught signal")) /* stopped */
			xrun = 0; /* reset xruns */
	}
	return TRUE;
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

gboolean jk_spawn_jackd(JkAppData* d)
{
	gboolean ret; /* spawn success */
	gchar** argv = NULL;
	GIOChannel* out = NULL;
	
	argv = g_strsplit(d->jackd_cmdline, " ", 0);
	/* FIXME: set any working directory ? ($HOME or /tmp) */
	ret = g_spawn_async_with_pipes
		(NULL, argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD, NULL, NULL, 
		&d->jackd_pid, &d->jackd_in, &d->jackd_out, &d->jackd_err, &d->jackd_error);
	if(ret == FALSE) {
		g_warning("jk_spawn_jackd: %s\n", d->jackd_error->message);
		g_error_free(d->jackd_error);
		d->jackd_error=NULL;
	}
	g_strfreev(argv);
	/* set the child watcher */
	d->jackd_chld = g_child_watch_add (d->jackd_pid, jk_on_jackd_sigchld, d);
	/* set fd watcher if this is jackd */
	out = g_io_channel_unix_new(d->jackd_out);
	d->jackd_fd = g_io_add_watch(out, G_IO_IN, jk_on_data, d);
	return ret;
}
