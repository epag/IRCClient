
#include <stdio.h>
#include <gtk/gtk.h>

GtkListStore * list_rooms;
GtkListStore * users_list;

void update_list_rooms() {
    GtkTreeIter iter;
    int i;

    /* Add some messages to the window */
    for (i = 0; i < 10; i++) {
        gchar *msg = g_strdup_printf ("Room %d", i);
        gtk_list_store_append (GTK_LIST_STORE (list_rooms), &iter);
        gtk_list_store_set (GTK_LIST_STORE (list_rooms), &iter, 0, msg,-1);
	    g_free (msg);
    }
}

void send_clicked (GtkWidget *widget, gpointer data) {

} 

void join_clicked (GtkWidget *widget, gpointer data) {
    g_print("join\n");
}
void create_clicked (GtkWidget *widget, gpointer data) {
    g_print("create\n");
}


/* Create the list of "messages" */
static GtkWidget *create_list( const char * titleColumn, GtkListStore *model )
{
    GtkWidget *scrolled_window;
    GtkWidget *tree_view;
    //GtkListStore *model;
    GtkCellRenderer *cell;
    GtkTreeViewColumn *column;

    int i;
   
    /* Create a new scrolled window, with scrollbars only if needed */
    scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
				    GTK_POLICY_AUTOMATIC, 
				    GTK_POLICY_AUTOMATIC);
   
    //model = gtk_list_store_new (1, G_TYPE_STRING);
    tree_view = gtk_tree_view_new ();
    gtk_container_add (GTK_CONTAINER (scrolled_window), tree_view);
    gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view), GTK_TREE_MODEL (model));
    gtk_widget_show (tree_view);
   
    cell = gtk_cell_renderer_text_new ();

    column = gtk_tree_view_column_new_with_attributes (titleColumn,
                                                       cell,
                                                       "text", 0,
                                                       NULL);
  
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view),
	  		         GTK_TREE_VIEW_COLUMN (column));

    return scrolled_window;
}
   
/* Add some text to our text widget - this is a callback that is invoked
when our window is realized. We could also force our window to be
realized with gtk_widget_realize, but it would have to be part of
a hierarchy first */

static void insert_text( GtkTextBuffer *buffer, const char * initialText )
{
   GtkTextIter iter;
 
   gtk_text_buffer_get_iter_at_offset (buffer, &iter, 0);
   gtk_text_buffer_insert (buffer, &iter, initialText,-1);
}
   
/* Create a scrolled text area that displays a "message" */
static GtkWidget *create_text( const char * initialText )
{
   GtkWidget *scrolled_window;
   GtkWidget *view;
   GtkTextBuffer *buffer;

   view = gtk_text_view_new ();
   buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));

   scrolled_window = gtk_scrolled_window_new (NULL, NULL);
   gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
		   	           GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_AUTOMATIC);

   gtk_container_add (GTK_CONTAINER (scrolled_window), view);
   insert_text (buffer, initialText);

   gtk_widget_show_all (scrolled_window);

   return scrolled_window;
}
void log_clicked (GtkWidget *widget, gpointer data) {
    g_print("logged on\n");
    g_print("send\n");
    GtkWidget *window;
    GtkWidget *list;
    GtkWidget *messages;
    GtkWidget *myMessage;
    GtkWidget *users;

   
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (window), "Log In Window");
    g_signal_connect (window, "destroy",
	              G_CALLBACK (gtk_main_quit), NULL);
    gtk_container_set_border_width (GTK_CONTAINER (window), 1);
    gtk_widget_set_size_request (GTK_WIDGET (window), 250, 100);

    // Create a table to place the widgets. Use a 7x4 Grid (7 rows x 4 columns)
    GtkWidget *table = gtk_table_new (3, 4, TRUE);
    gtk_container_add (GTK_CONTAINER (window), table);
    gtk_table_set_row_spacings(GTK_TABLE (table), 5);
    gtk_table_set_col_spacings(GTK_TABLE (table), 5);
    gtk_widget_show (table);

    // Add messages text. Use columns 0 to 4 (exclusive) and rows 4 to 7 (exclusive)
    messages = create_text ("Name");
    gtk_table_attach_defaults (GTK_TABLE (table), messages, 0, 2, 0, 1);
    gtk_widget_show (messages);

    // Add messages text. Use columns 0 to 4 (exclusive) and rows 4 to 7 (exclusive) 
    myMessage = create_text ("Password");
    gtk_table_attach_defaults (GTK_TABLE (table), myMessage, 2, 4, 0, 1);
    gtk_widget_show (myMessage);

    // Create room button
    GtkWidget *create_button = gtk_button_new_with_label ("Add User");
    gtk_table_attach_defaults(GTK_TABLE (table), create_button, 0, 2, 1, 2); 
    gtk_widget_show (create_button);

    // Send Button
    GtkWidget *send_button = gtk_button_new_with_label ("Log In");
    gtk_table_attach_defaults(GTK_TABLE (table), send_button, 2, 4, 1, 2);
    gtk_widget_show (send_button);
 
    gtk_widget_show (table);
    gtk_widget_show (window);

    gtk_main ();

    return;
}
int main( int   argc,
          char *argv[] )
{
    GtkWidget *window;
    GtkWidget *list;
    GtkWidget *messages;
    GtkWidget *myMessage;
    GtkWidget *users;

    gtk_init (&argc, &argv);
   
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (window), "Paned Windows");
    g_signal_connect (window, "destroy",
	              G_CALLBACK (gtk_main_quit), NULL);
    gtk_container_set_border_width (GTK_CONTAINER (window), 10);
    gtk_widget_set_size_request (GTK_WIDGET (window), 450, 400);

    // Create a table to place the widgets. Use a 7x4 Grid (7 rows x 4 columns)
    GtkWidget *table = gtk_table_new (7, 4, TRUE);
    gtk_container_add (GTK_CONTAINER (window), table);
    gtk_table_set_row_spacings(GTK_TABLE (table), 5);
    gtk_table_set_col_spacings(GTK_TABLE (table), 5);
    gtk_widget_show (table);

    // Add list of rooms. Use columns 0 to 4 (exclusive) and rows 0 to 4 (exclusive)
    list_rooms = gtk_list_store_new (1, G_TYPE_STRING);
    update_list_rooms();
    list = create_list ("Rooms", list_rooms);
    gtk_table_attach_defaults (GTK_TABLE (table), list, 2, 4, 0, 2);
    gtk_widget_show (list);
   
    // Add messages text. Use columns 0 to 4 (exclusive) and rows 4 to 7 (exclusive)
    messages = create_text ("");
    gtk_table_attach_defaults (GTK_TABLE (table), messages, 0, 4, 2, 5);
    gtk_widget_show (messages);

    // Add list of Users.
    users_list = gtk_list_store_new (1, G_TYPE_STRING);
    users = create_list ("Users", users_list);
    gtk_table_attach_defaults (GTK_TABLE (table), users, 0, 2, 0, 2);
    gtk_widget_show (users);

    // Add messages text. Use columns 0 to 4 (exclusive) and rows 4 to 7 (exclusive) 
    myMessage = create_text ("");
    gtk_table_attach_defaults (GTK_TABLE (table), myMessage, 0, 4, 5, 7);
    gtk_widget_show (myMessage);

    // Create room button
    GtkWidget *create_button = gtk_button_new_with_label ("Create Room");
    gtk_table_attach_defaults(GTK_TABLE (table), create_button, 0, 1, 7, 8); 
    gtk_widget_show (create_button);

    // login
    GtkWidget *join_button = gtk_button_new_with_label ("New User/\n   Log In");
    gtk_table_attach_defaults(GTK_TABLE (table), join_button, 1, 2, 7, 8);
    gtk_widget_show (join_button);

    // Leave Room button
    GtkWidget *leave_button = gtk_button_new_with_label ("Enter or Leave\n      Room");
    gtk_table_attach_defaults(GTK_TABLE (table), leave_button, 2, 3, 7, 8);
    gtk_widget_show (leave_button);

    // Send Button
    GtkWidget *send_button = gtk_button_new_with_label ("Send");
    gtk_table_attach_defaults(GTK_TABLE (table), send_button, 3, 4, 7, 8);
    gtk_widget_show (send_button);
 
    g_signal_connect (G_OBJECT(send_button), "clicked", G_CALLBACK(send_clicked), NULL);
    g_signal_connect (G_OBJECT(leave_button), "clicked", G_CALLBACK(join_clicked), NULL);
    g_signal_connect (G_OBJECT(create_button), "clicked", G_CALLBACK(create_clicked), NULL);
    g_signal_connect (G_OBJECT(join_button), "clicked", G_CALLBACK(log_clicked), NULL);
    gtk_widget_show (table);
    gtk_widget_show (window);

    gtk_main ();

    return 0;
}

