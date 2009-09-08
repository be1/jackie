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
	char* names [3] = {"jackd", "patchage", "transport"};
	char* cmdlines [3] = {"/usr/bin/jackd -R -d alsa", "/usr/bin/patchage", ""};

	for (i = 0; i < 3; i++) {
		pr = (JkProg*) malloc (sizeof(JkProg));
		pr->name = g_strdup(names[i]);
		pr->cmdline = g_strdup(cmdlines[i]);
		progs = g_slist_append(progs, (gpointer)pr);
	}
	return progs;
}

/* writes the config to a file */
void jk_write_config(char* config_path, GSList* progs) {
	GKeyFile* kf = NULL;
	GSList* pr;
	gchar* data = NULL;
	gsize len;
	FILE* fp = NULL;

	pr = progs;
	kf = g_key_file_new();
	for (pr = progs; pr; pr = g_slist_next(pr)) {
		JkProg* p;
		p = pr->data;
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
			p = (JkProg*) malloc(sizeof(JkProg));
			p->name = g_strdup(groups[i]);
			if (!strcmp(keys [j], "cmdline")) {
				p->cmdline = val;
			}
			p->error = NULL;
			p->pid = (GPid)0;
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

/* update jackie tooltip with jackd messages */
gboolean jk_update_tooltip(gpointer app_data) {
	JkAppData* d = (JkAppData*) app_data;
	GSList* prog = NULL;
	JkProg* pr = NULL;

	for (prog = d->progs; prog; prog = g_slist_next(prog)) {
		pr = (JkProg*)prog->data;
		if (!strcmp(pr->name, "jackd") && strlen(pr->buf)) {
			gtk_status_icon_set_tooltip (d->tray_icon, g_strstrip(pr->buf));
			break;
		}
	}
	return TRUE;
}
/* watch return of a program */
void jk_on_sigchld(GPid pid, gint status, gpointer prog) {
	JkProg* pr = (JkProg*)prog;

	close(pr->in);
	close(pr->out);
	close(pr->err);
	g_source_remove(pr->fdtag);
	g_source_remove(pr->chldtag);
	g_spawn_close_pid(pid);

	/* FIXME: could handle status code */
	status = 0;
}

gboolean jk_on_data(GIOChannel* source, GIOCondition condition, gpointer prog) {
	JkProg* pr = (JkProg*)prog;
	gint fd;

	fd = g_io_channel_unix_get_fd(source);
	if (condition == G_IO_IN) {
		ssize_t nread;

		nread = read(pr->out, pr->buf, BUFSIZ);
		if (nread < BUFSIZ)
			pr->buf[nread] = '\0';
		else pr->buf[BUFSIZ-1] = '\0';
	}
	return TRUE;
}

/* spawn a external program */
gboolean jk_spawn_prog(JkProg* prog)
{
	gboolean ret; /* spawn success */
	gchar** argv = NULL;
	GIOChannel* out = NULL;

	argv = g_strsplit(prog->cmdline, " ", 0);
	/* FIXME: set any working directory ? ($HOME or /tmp) */
	ret = g_spawn_async_with_pipes
		(NULL, argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD, NULL, NULL, 
		&prog->pid, &prog->in, &prog->out, &prog->err, &prog->error);
	if(ret == FALSE) {
		g_warning("jk_spawn_prog: %s\n", prog->error->message);
		g_error_free(prog->error);
		prog->error=NULL;
	}
	g_strfreev(argv);
	/* set the child watcher */
	prog->chldtag = g_child_watch_add (prog->pid, jk_on_sigchld, prog);
	/* set fd watcher if this is jackd */
	if (!strcmp(prog->name, "jackd")) {
		out = g_io_channel_unix_new(prog->out);
		prog->fdtag = g_io_add_watch(out, G_IO_IN, jk_on_data, prog);
	}
	return ret;
}
