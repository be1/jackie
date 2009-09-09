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

/* creates a default JkConfig structure */
GSList* jk_create_default_config(void) {
	GSList* progs = NULL;
	JkProg* pr = NULL;
	int i;
	char* names [3] = {"jackd", "patchbay", "transport"};
	char* cmdlines [3] = {"/usr/bin/jackd -R -d alsa", "/usr/bin/patchage", ""};

	for (i = 0; i < 3; i++) {
		pr = (JkProg*) malloc (sizeof(JkProg));
		pr->name = g_strdup(names[i]);
		pr->cmdline = g_strdup(cmdlines[i]);
		progs = g_slist_append(progs, (gpointer)pr);
	}
	return progs;
}

/* update jackd command line to list of progs */
void jk_update_cmdline (GSList* progs, const gchar* name, const gchar* cmdline) {
	GSList* pr;

	for (pr = progs; pr; pr = g_slist_next(pr)) {
		JkProg* p = (JkProg*)pr->data;

		if (!strcmp(p->name, name)) {
			g_free(p->cmdline);
			p->cmdline = g_strdup(cmdline);
			break;
		}
	}
}

/* writes the config to a file */
void jk_write_config(const char* config_path, const GSList* progs) {
	GKeyFile* kf = NULL;
	const GSList* pr;
	gchar* data = NULL;
	gsize len;
	FILE* fp = NULL;

	kf = g_key_file_new();
	for (pr = progs; pr; pr = g_slist_next(pr)) {
		JkProg* p = (JkProg*)pr->data;
		g_key_file_set_value (kf, p->name, "cmdline", p->cmdline);
		/* append other params here */
	}
	data = g_key_file_to_data(kf, &len, NULL);
	fp = fopen(config_path, "w");
	/* FIXME: check error (disk full, e.g.)*/
	fputs(data, fp);
	fclose(fp);
	g_free(data);
}

gchar* jk_read_cmdline(const gchar* name, gchar* config_path) {
	GKeyFile* kf = NULL;
	GError* error = NULL;
	gchar* cmdline = NULL;
	gchar** keys = NULL;
	gchar** groups = NULL;
	gchar* val;
	gsize glen, klen, i, j;

	kf = g_key_file_new();
	g_key_file_load_from_file(kf, config_path, G_KEY_FILE_NONE, &error);
	if(error) {
		GSList* progs = NULL;
		progs = jk_create_default_config();
		jk_write_config(config_path, progs);
		g_error_free(error);
		jk_delete_progs(progs);
		error = NULL;
		g_key_file_load_from_file(kf, config_path, G_KEY_FILE_NONE, &error);
	}
	groups = g_key_file_get_groups(kf, &glen);
	for (i = 0; i < glen; ++i) {
		if (strcmp(groups[i], name))
			continue; /* pass if this is not about 'name' */
		keys = g_key_file_get_keys(kf, groups[i], &klen, &error);
		if(error) {
			g_warning("jk_read_cmdline: %s\n", error->message);
			g_error_free(error);
			error = NULL;
		} else for (j = 0; j < klen; ++j) {
			val = g_key_file_get_value (kf, groups[i], keys[j], &error);
			if(error) {
				g_warning("jk_read_cmdline: %s\n", error->message);
				g_error_free(error);
				error = NULL;
			}
			/* return cmdline */
			if (!strcmp(keys [j], "cmdline")) {
				cmdline = g_strdup(val);
			}
			/* append other params here */
		}
	}
	g_strfreev(groups);
	g_strfreev(keys);
	g_key_file_free(kf);
	/* defaults */
	if (!cmdline) {
		if (!strcmp(name, "jackd"))
			cmdline = g_strdup("/usr/bin/jackd -R -d alsa");
		else if (!strcmp(name, "patchbay"))
			cmdline = g_strdup("/usr/bin/patchage");
		/* other defaults here */
	}
	return cmdline;
}

/* creates a list of JkProgs given the config file */
GSList* jk_read_config (gchar* config_path) {
	GSList* progs = NULL;
	GKeyFile* kf = NULL;
	GError* error = NULL;
	gchar** keys = NULL;
	gchar** groups = NULL;
	gchar* val;
	gsize glen, klen, i, j;

	kf = g_key_file_new();
	g_key_file_load_from_file(kf, config_path, G_KEY_FILE_NONE, &error);
	if(error) {
		progs = jk_create_default_config();
		jk_write_config(config_path, progs);
		g_error_free(error);
		error = NULL;
		g_key_file_load_from_file(kf, config_path, G_KEY_FILE_NONE, &error);
	}
	groups = g_key_file_get_groups(kf, &glen);
	for (i = 0; i < glen; ++i) {
		JkProg* p;

		keys = g_key_file_get_keys(kf, groups[i], &klen, &error);
		if(error) {
			g_warning("jk_read_config: %s\n", error->message);
			g_error_free(error);
			error = NULL;
		} else for (j = 0; j < klen; ++j) {
			val = g_key_file_get_value (kf, groups[i], keys[j], &error);
			if(error) {
				g_warning("jk_read_config: %s\n", error->message);
				g_error_free(error);
				error = NULL;
			}
			/* store config */
			p = (JkProg*) malloc(sizeof(JkProg));
			p->name = g_strdup(groups[i]);
			if (!strcmp(keys [j], "cmdline")) {
				p->cmdline = val;
			}
			p->error = NULL;
			/* append other params here */
		}
		progs = g_slist_append(progs, (gpointer)p);
	}
	g_strfreev(groups);
	g_strfreev(keys);
	g_key_file_free(kf);
	return progs;
}

/* deletes a JkProg list */
void jk_delete_progs(GSList* progs) {
	GSList* pr = NULL;
	JkProg* p = NULL;

	for (pr = progs; pr; pr = g_slist_next(pr)) {
		p = (JkProg*)pr->data;
		g_free(p->cmdline);
		free(p);
		p = NULL;
	}
	g_slist_free(progs);
}

void jk_quit(JkAppData* d) {
	g_free(d->config_path);
	jk_delete_progs(d->progs);
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
		if (g_regex_match_simple(".*xrun.*", d->tooltip_buffer, 0, 0) /* ALSA */
		|| g_regex_match_simple(".*delay of.*restart.*", d->tooltip_buffer, 0, 0)) { /* OSS */
			snprintf(d->tooltip_buffer, BUFSIZ, "Jackd xruns: %d", ++xrun);
			gtk_status_icon_set_tooltip (d->tray_icon, d->tooltip_buffer);
		} else if (g_regex_match_simple("^jack main caught signal.*$", d->tooltip_buffer, 0, 0)) /* stopped */
			xrun = 0; /* reset xruns */
	}
	return TRUE;
}

/* spawn a external program */
gboolean jk_spawn_application(JkAppData* d, const gchar* name) {
	GSList* progs = d->progs;
	GSList* pr;
	JkProg* p;
	gchar** argv = NULL;
	gboolean ret;
	GPid pid = (GPid)0;
	GError* error = NULL;

	for (pr = progs; pr; pr = g_slist_next(pr)) {
		p = (JkProg*) pr->data;

		if (!strcmp(p->name, name))
			break;
	}
	argv = g_strsplit(p->cmdline, " ", 0);
	ret = g_spawn_async (NULL, argv, NULL, 0, NULL, NULL, &pid, &error);
	if(ret == FALSE) {
		g_warning("jk_spawn_application: %s\n", error->message);
		g_error_free(error);
		d->jackd_error=NULL;
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
