#include <gtk/gtk.h>
/* create preference window */
GtkWindow* window_create(const gchar* title) {
	GtkWidget* window;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), title);
	gtk_window_set_resizable(GTK_WINDOW(window), TRUE);
	
	return GTK_WINDOW(window);
}


