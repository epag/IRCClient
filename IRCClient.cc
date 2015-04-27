#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <time.h>
#include <curses.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>

int logon = 0;
pthread_t thread;
int inRoom = 0;
char * room = NULL;
GtkWidget *table;
GtkWidget *messages;
GtkListStore * list_rooms;
GtkListStore * users_list;
GtkTextBuffer * messageBuffer;
GtkTextBuffer * buffer;
GtkTextBuffer * passwordBuffer;
GtkTextBuffer * chatLog;
GtkWidget * LogOnwindow;
GtkWidget * CreateRoomwindow;
char * user_name = NULL;
char * user_password = NULL;
char * sentMessage = NULL;
char * RoomName[100];
char * people[100];
int peopleNumber = 0;
int RoomNumber = 0;
GtkWidget *tree_view;
GtkWidget *tree_view2;
gchar * sMsg = NULL;

// CLIENT VARIABLES
int msgNum = 0;
int inLine = 0;
char * host = NULL;
char * user = NULL;
char * password = NULL;
char * sport = NULL;
int port;
char * RName;
gchar * selectedRoom;

#define MAX_MESSAGES 100
#define MAX_MESSAGE_LEN 300
#define MAX_RESPONCE (20 * 1024)

int lastMessage = 0;

// CLIENT CODE



static void insert_text( GtkTextBuffer *buffer, const char * initialText )
{
    GtkTextIter iter;

    gtk_text_buffer_get_iter_at_offset (buffer, &iter, 0);
    gtk_text_buffer_insert (buffer, &iter, initialText,-1);
}



static GtkWidget *create_text( const char * initialText )
{
    GtkWidget *scrolled_window;
    GtkWidget *view;

    view = gtk_text_view_new ();
    chatLog = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));

    scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
            GTK_POLICY_AUTOMATIC,
            GTK_POLICY_ALWAYS);

    gtk_container_add (GTK_CONTAINER (scrolled_window), view);
    insert_text (chatLog, initialText);

    gtk_widget_show_all (scrolled_window);
    gtk_text_view_set_editable(GTK_TEXT_VIEW (view), false);

    return scrolled_window;
}



int open_client_socket (char * host, int port) {

    struct sockaddr_in socketAddress;

    memset ((char *) &socketAddress, 0, sizeof(socketAddress));

    socketAddress.sin_family = AF_INET;

    socketAddress.sin_port = htons ((u_short) port);

    struct hostent * ptrh = gethostbyname(host);

    if (ptrh == NULL) {
        perror ("gethostbyname");
        exit(1);
    }

    memcpy (&socketAddress.sin_addr, ptrh->h_addr, ptrh->h_length);

    struct protoent * ptrp = getprotobyname("tcp");

    if (ptrp == NULL) {
        perror ("getprotobyname");
        exit(1);
    }

    int sock = socket (PF_INET, SOCK_STREAM, ptrp->p_proto);
    if (sock < 0) {
        perror ("socket");
        exit(1);
    }

    if (connect(sock, (struct sockaddr *) &socketAddress, sizeof(socketAddress)) < 0) {
        perror ("connect");
        exit(1);
    }

    return sock;

}



int sendCommand (char * host, int port, char * command, char * user, char * password, char * args, char * responce) {

    int sock = open_client_socket (host, port);

    write (sock, command, strlen(command));
    write (sock, " ", 1);
    write (sock, user, strlen(user));
    write (sock, " ", 1);
    write (sock, password, strlen(password));
    write (sock, " ", 1);
    write (sock, args, strlen(args));
    write (sock, "\r\n", 2);

    int n = 0;
    int len = 0;

    while ((n=read(sock, responce+len, MAX_RESPONCE - len)) > 0) {
        len += n;
    }

    close (sock);

}
int sendCommand2 (char * host, int port, char * command, char * user, char * password, char * args, char * message, char * responce) {

    int sock = open_client_socket (host, port);

    write (sock, command, strlen(command));
    write (sock, " ", 1);
    write (sock, user, strlen(user));
    write (sock, " ", 1);
    write (sock, password, strlen(password));
    write (sock, " ", 1);
    write (sock, args, strlen(args));
    write (sock, " ", 1);
    write (sock, message, strlen(message));
    write (sock, "\r\n", 2);

    int n = 0;
    int len = 0;

    while ((n=read(sock, responce+len, MAX_RESPONCE - len)) > 0) {
        len += n;
    }

    close (sock);

}

void send_message() {
    char responce [MAX_RESPONCE];
    sendCommand2 (host, port, "SEND-MESSAGE", user, password, selectedRoom, sMsg, responce);
    
}

void leave_room () {
    char responce [MAX_RESPONCE];
    inRoom = 0;
    sMsg = g_strdup_printf ("%s left %s", user, selectedRoom);
    send_message();
    sendCommand (host, port, "LEAVE-ROOM", user, password, selectedRoom, responce);

}

void add_user() {
    char responce [MAX_RESPONCE];
    sendCommand (host, port, "ADD-USER", user, password, "", responce);

    if (!strcmp (responce, "OK\r\n")) {
        printf ("User %s added\n", user);
    }
}



void get_messages(char * room) {
    char answ [MAX_RESPONCE];
    char responce [MAX_RESPONCE];
    for (int i = 0; i < MAX_RESPONCE; i++) {
        responce[i] = '\0';
        answ[i] = '\0';
    }

    char * num = (char *) malloc(sizeof(char) * 100);;
    sprintf(num, "%d", msgNum);
    sendCommand2(host, port, "GET-MESSAGES2", user, password, num, room, responce);
    if (strcmp(responce, "NO-NEW-MESSAGES\r\n")) {
        sendCommand2 (host, port, "GET-MESSAGES2", user, password, num, room, responce);
        insert_text (chatLog, responce);
        while (strcmp (answ, "NO-NEW-MESSAGES\r\n")) {
            for (int i = 0; i < MAX_RESPONCE; i++) {
                responce[i] = '\0';
                answ[i] = '\0';
            }
            char * num = (char *) malloc(sizeof(char) * 100);;
            sprintf(num, "%d", msgNum);
            sendCommand2 (host, port, "GET-MESSAGES2", user, password, num, room, answ);
            msgNum++;
            free(num);
        }
        msgNum--;
    if (msgNum == 100) {
        msgNum--;
    }
    return;
    }
    free(num);
    return;
}
void get_rooms() {
    char responce [MAX_RESPONCE];

    sendCommand (host, port, "GET-ROOMS", user, password, "", responce);
    RoomNumber = 0; 
    int i = 0;

    char * token;
    token = strtok(responce, "*");

    while (token != NULL) { 
        RoomName[inLine] = strdup(token);
        token = strtok(NULL, "*");
        RoomNumber++;
    }
}
void update_list_rooms() {
    GtkTreeIter iter;

    if (!strcmp(RoomName[0], "empty")) {
        gtk_list_store_clear (GTK_LIST_STORE (list_rooms));
        return;
    }
    /* Add some messages to the window */
    for (; inLine < RoomNumber; inLine++) {
        gchar * msg = g_strdup_printf ("%s", RoomName[inLine]);
        gtk_list_store_append (GTK_LIST_STORE (list_rooms), &iter);
        gtk_list_store_set (GTK_LIST_STORE (list_rooms), &iter, 0, msg,-1);
        free(msg);
    }
}

void * getMessagesThread (void * args) {
        usleep(2*1000*1000);
    while (1) {
        if (logon == 1) {
            get_rooms();
        }
            update_list_rooms();
        if (inRoom == 1) {

            get_messages(room);

        }
        usleep(2*1000*1000);
    }
}



void * startGetMessageThread() {

    pthread_create (&thread, NULL, getMessagesThread,NULL);
}



void enter_room () {
    char responce [MAX_RESPONCE];

    inRoom = 1;
    room = selectedRoom;
    sendCommand (host, port, "ENTER-ROOM", user, password, selectedRoom, responce);



    if (!strcmp(responce, "OK\r\n")) {
        sMsg = g_strdup_printf ("%s joined %s!", user, selectedRoom);
        send_message();
            return;
        }
        

}




void add_room() {
    char responce [MAX_RESPONCE];

    sendCommand (host, port, "CREATE-ROOM", user, password, RName, responce);

    if (!strcmp(responce, "OK\r\n")) {
        printf ("Room %s created\n", RName);
    }
}



void get_all_users() {
    char responce [MAX_RESPONCE];
    for (int i = 0; i < MAX_RESPONCE; i++) {
        responce[i] = '\0';
    }
    sendCommand (host, port, "GET-ALL-USERS2", user, password, "", responce);
    peopleNumber = 0; 
    int i = 0;

    char * token;
    token = strtok(responce, "*");

    for (int j = 0; j < 100; j++) {
        people[j] = NULL;
    }
    while (token != NULL) { 

        people[i] = strdup(token);
        token = strtok(NULL, "*");
        i++;
        peopleNumber++;
    }
}



// GUI FUNCTIONS



void update_users() {
    GtkTreeIter iter;
    gtk_list_store_clear (GTK_LIST_STORE (users_list));

    if (!strcmp(people[0], "empty")) {
        gtk_list_store_clear (GTK_LIST_STORE (users_list));
        return;
    }
    /* Add some messages to the window */
    for (int i = 0; i < peopleNumber; i++) {
        gchar * msg = g_strdup_printf ("%s", people[i]);
        gtk_list_store_append (GTK_LIST_STORE (users_list), &iter);
        gtk_list_store_set (GTK_LIST_STORE (users_list), &iter, 0, msg,-1);
        free(msg);
    }
}








static GtkWidget *create_text2( const char * initialText )
{
    GtkWidget *scrolled_window;
    GtkWidget *view;

    view = gtk_text_view_new ();
    messageBuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));

    scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
            GTK_POLICY_AUTOMATIC,
            GTK_POLICY_ALWAYS);

    gtk_container_add (GTK_CONTAINER (scrolled_window), view);
    insert_text (messageBuffer, initialText);

    gtk_widget_show_all (scrolled_window);

    return scrolled_window;
}
void newUsr_clicked (GtkWidget *widget, gpointer data) {

    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter(buffer, &start);
    gtk_text_buffer_get_end_iter(buffer, &end);
    gchar*  name = (char *) gtk_text_buffer_get_text(buffer, &start, &end, false);
    user = name;

    GtkTextIter start2, end2;
    gtk_text_buffer_get_start_iter(passwordBuffer, &start2);
    gtk_text_buffer_get_end_iter(passwordBuffer, &end2);
    gchar* passwords = (char *) gtk_text_buffer_get_text(passwordBuffer, &start2, &end2, false);
    password = passwords;

    logon = 1;


    add_user();
    get_rooms();
    update_list_rooms();
    get_all_users(); 
    update_users();


    GtkTextIter start3, end3;
    gtk_text_buffer_get_start_iter(messageBuffer, &start3);
    gtk_text_buffer_get_end_iter(messageBuffer, &end3);

    messages = create_text("");
    gtk_table_attach_defaults (GTK_TABLE (table), messages, 0, 4, 2, 5);
    gtk_widget_show (messages);
    sentMessage = NULL;

    gtk_widget_destroy(GTK_WIDGET(LogOnwindow));
}

void logOn_clicked (GtkWidget *widget, gpointer data) {

    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter(buffer, &start);
    gtk_text_buffer_get_end_iter(buffer, &end);
    gchar*  names = (char *) gtk_text_buffer_get_text(buffer, &start, &end, false);
    user = names;

    GtkTextIter start2, end2;
    gtk_text_buffer_get_start_iter(passwordBuffer, &start2);
    gtk_text_buffer_get_end_iter(passwordBuffer, &end2);
    gchar* passwords = (char *) gtk_text_buffer_get_text(passwordBuffer, &start2, &end2, false);
    password = passwords;

    logon = 1;

    get_rooms();
    update_list_rooms();
    get_all_users(); 
    update_users();

    GtkTextIter start3, end3;
    gtk_text_buffer_get_start_iter(messageBuffer, &start3);
    gtk_text_buffer_get_end_iter(messageBuffer, &end3);

    messages = create_text("");
    gtk_table_attach_defaults (GTK_TABLE (table), messages, 0, 4, 2, 5);
    gtk_widget_show (messages);
    sentMessage = NULL;

    gtk_widget_destroy(GTK_WIDGET(LogOnwindow));
}

void send_clicked (GtkWidget *widget, gpointer data) {
    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter(messageBuffer, &start);
    gtk_text_buffer_get_end_iter(messageBuffer, &end);
    
    sMsg = " ";
    sMsg = gtk_text_buffer_get_text(messageBuffer, &start, &end, false);
    send_message();

    //get_messages();
    gtk_text_buffer_set_text (messageBuffer, "", -1);
    return;
}


void join_clicked (GtkWidget *widget, gpointer data) {
    char responce [MAX_RESPONCE];
    for (int i = 0; i < peopleNumber; i++) {
        if (!strcmp(user, people[i])) {
            leave_room();
            sendCommand (host, port, "GET-USERS-IN-ROOM2", user, password, selectedRoom, responce);
            peopleNumber = 0; 
            int i = 0;

            char * token;
            token = strtok(responce, "*");

            for (int j = 0; j < 100; j++) {
                people[j] = NULL;
            }

            while (token != NULL) { 

                people[i] = strdup(token);
                token = strtok(NULL, "*");
                i++;
                peopleNumber++;
            }
            update_users();
            return;
        }
    }
    enter_room();
    sendCommand (host, port, "GET-USERS-IN-ROOM2", user, password, selectedRoom, responce);
    peopleNumber = 0; 
    int i = 0;

    char * token;
    token = strtok(responce, "*");

    for (int j = 0; j < 100; j++) {
        people[j] = NULL;
    }

    while (token != NULL) { 

        people[i] = strdup(token);
        token = strtok(NULL, "*");
        i++;
        peopleNumber++;
    }
    update_users();
    return;
}


void newRoom_clicked (GtkWidget *widget, gpointer data) {
    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter(buffer, &start);
    gtk_text_buffer_get_end_iter(buffer, &end);
    gchar*  name = (char *) gtk_text_buffer_get_text(buffer, &start, &end, false);

    RName = name;

    add_room();
    get_rooms();
    update_list_rooms();

    gtk_widget_destroy(GTK_WIDGET(CreateRoomwindow));
}
void create_clicked (GtkWidget *widget, gpointer data) {
    GtkWidget *list;
    GtkWidget *name;
    GtkWidget *password;
    GtkWidget *users;
    buffer = NULL;

    CreateRoomwindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (CreateRoomwindow), "Create New Room");
    g_signal_connect (CreateRoomwindow, "destroy",
            G_CALLBACK (gtk_main_quit), NULL);
    gtk_container_set_border_width (GTK_CONTAINER (CreateRoomwindow), 10);
    gtk_widget_set_size_request (GTK_WIDGET (CreateRoomwindow), 250, 100);

    // Create a table to place the widgets. Use a 7x4 Grid (7 rows x 4 columns)
    GtkWidget *table = gtk_table_new (2, 6, TRUE);
    gtk_container_add (GTK_CONTAINER (CreateRoomwindow), table);
    gtk_table_set_row_spacings(GTK_TABLE (table), 5);
    gtk_table_set_col_spacings(GTK_TABLE (table), 5);
    gtk_widget_show (table);

    //Collects the Users name
    name = gtk_text_view_new ();
    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (name));
    gtk_text_buffer_set_text (buffer, "RoomName", -1);
    gtk_table_attach_defaults (GTK_TABLE (table), name, 1, 5, 0, 1);
    gtk_widget_show(name);

    // Create room button
    GtkWidget *newRoom_button = gtk_button_new_with_label ("Create Room");
    gtk_table_attach_defaults(GTK_TABLE (table), newRoom_button, 1, 5, 1, 2); 
    gtk_widget_show (newRoom_button);

    gtk_widget_show (table);
    gtk_widget_show (CreateRoomwindow);

    g_signal_connect (G_OBJECT(newRoom_button), "clicked", G_CALLBACK(newRoom_clicked), NULL);
    gtk_main ();

    return;
}


/* Create the list of "messages" */
static GtkWidget *create_list( const char * titleColumn, GtkListStore *model )
{
    GtkWidget *scrolled_window;

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

static GtkWidget *create_list2( const char * titleColumn, GtkListStore *model )
{
    GtkWidget *scrolled_window;

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
    tree_view2 = gtk_tree_view_new ();
    gtk_container_add (GTK_CONTAINER (scrolled_window), tree_view2);
    gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view2), GTK_TREE_MODEL (model));
    gtk_widget_show (tree_view2);

    cell = gtk_cell_renderer_text_new ();

    column = gtk_tree_view_column_new_with_attributes (titleColumn,
            cell,
            "text", 0,
            NULL);

    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view2),
            GTK_TREE_VIEW_COLUMN (column));

    return scrolled_window;
}
/* Add some text to our text widget - this is a callback that is invoked
   when our window is realized. We could also force our window to be
   realized with gtk_widget_realize, but it would have to be part of
   a hierarchy first */



/* Create a scrolled text area that displays a "message" */

void log_clicked (GtkWidget *widget, gpointer data) {

    GtkWidget *list;
    GtkWidget *name;
    GtkWidget *password;
    GtkWidget *users;

    LogOnwindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (LogOnwindow), "Log In Window");
    g_signal_connect (LogOnwindow, "destroy",
            G_CALLBACK (gtk_main_quit), NULL);
    gtk_container_set_border_width (GTK_CONTAINER (LogOnwindow), 10);
    gtk_widget_set_size_request (GTK_WIDGET (LogOnwindow), 250, 100);

    // Create a table to place the widgets. Use a 7x4 Grid (7 rows x 4 columns)
    GtkWidget *table = gtk_table_new (2, 4, TRUE);
    gtk_container_add (GTK_CONTAINER (LogOnwindow), table);
    gtk_table_set_row_spacings(GTK_TABLE (table), 5);
    gtk_table_set_col_spacings(GTK_TABLE (table), 5);
    gtk_widget_show (table);

    //Collects the Users name
    name = gtk_text_view_new ();
    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (name));
    gtk_text_buffer_set_text (buffer, "Name", -1);
    gtk_table_attach_defaults (GTK_TABLE (table), name, 0, 2, 0, 1);
    gtk_widget_show(name);

    //Collects password
    password = gtk_text_view_new ();
    passwordBuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (password));
    gtk_text_buffer_set_text (passwordBuffer, "Password", -1);
    gtk_table_attach_defaults (GTK_TABLE (table), password, 2, 4, 0, 1);
    gtk_widget_show(password);

    // Create room button
    GtkWidget *newUsr_button = gtk_button_new_with_label ("Add User");
    gtk_table_attach_defaults(GTK_TABLE (table), newUsr_button, 0, 2, 1, 2); 
    gtk_widget_show (newUsr_button);

    // Send Button
    GtkWidget *logOn_button = gtk_button_new_with_label ("Log In");
    gtk_table_attach_defaults(GTK_TABLE (table), logOn_button, 2, 4, 1, 2);
    gtk_widget_show (logOn_button);

    gtk_widget_show (table);
    gtk_widget_show (LogOnwindow);

    g_signal_connect (G_OBJECT(logOn_button), "clicked", G_CALLBACK(logOn_clicked), NULL);
    g_signal_connect (G_OBJECT(newUsr_button), "clicked", G_CALLBACK(newUsr_clicked), NULL);
    gtk_main ();

    return;
}



void update_list_users (GtkWidget *widget, gpointer data) {
    GtkWidget *users;


    GValue tempVal = G_VALUE_INIT;
    GtkWidget * treeCpy = tree_view;
    GtkTreeSelection * roomSelected = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
    GtkListStore * model = list_rooms;
    GtkTreeModel * treeModel = GTK_TREE_MODEL (model);
    GList * info = gtk_tree_selection_get_selected_rows (roomSelected, &treeModel);
    GtkTreePath * treePath = (GtkTreePath*) info->data;
    GtkTreeIter * iter = (GtkTreeIter *) malloc (sizeof(GtkTreeIter));
    gtk_tree_model_get_iter (treeModel, iter, treePath);    
    gtk_tree_model_get_value (treeModel, iter, 0, &tempVal);
    const GValue * GStore = (const GValue *) &tempVal;
    selectedRoom = g_strdup (g_value_get_string (GStore));



    char responce [MAX_RESPONCE];

    sendCommand (host, port, "GET-USERS-IN-ROOM2", user, password, selectedRoom, responce);
    peopleNumber = 0; 
    int i = 0;

    char * token;
    token = strtok(responce, "*");

    for (int j = 0; j < 100; j++) {
        people[j] = NULL;
    }

    while (token != NULL) { 

        people[i] = strdup(token);
        token = strtok(NULL, "*");
        i++;
        peopleNumber++;
    }

    update_users();
}








int main( int   argc,
        char *argv[] )
{

    //SERVER CODE

    char line[MAX_MESSAGE_LEN+1];

    host = argv[1];
    sport = argv[2];

    sscanf(sport, "%d", &port);

    printf ("\nStrarting talk-client %s %s \n", host, sport);




    // GUI CODE
    sentMessage = (char *) malloc(sizeof(char) * 100000000);
    GtkWidget *window;


    GtkWidget *myMessage;
    GtkWidget *users;

    GtkWidget *list;
    gtk_init (&argc, &argv);

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (window), "Paned Windows");
    g_signal_connect (window, "destroy",
            G_CALLBACK (gtk_main_quit), NULL);
    gtk_container_set_border_width (GTK_CONTAINER (window), 10);
    gtk_widget_set_size_request (GTK_WIDGET (window), 450, 400);

    // Create a table to place the widgets. Use a 7x4 Grid (7 rows x 4 columns)
    table = gtk_table_new (7, 4, TRUE);
    gtk_container_add (GTK_CONTAINER (window), table);
    gtk_table_set_row_spacings(GTK_TABLE (table), 5);
    gtk_table_set_col_spacings(GTK_TABLE (table), 5);
    gtk_widget_show (table);

    // Add list of rooms. Use columns 0 to 4 (exclusive) and rows 0 to 4 (exclusive)
    list_rooms = gtk_list_store_new (1, G_TYPE_STRING);

    list = create_list ("Rooms", list_rooms);
    gtk_table_attach_defaults (GTK_TABLE (table), list, 2, 4, 0, 2);
    gtk_widget_show (list);

    // Add messages text. Use columns 0 to 4 (exclusive) and rows 4 to 7 (exclusive)
    messages = create_text ("");
    gtk_table_attach_defaults (GTK_TABLE (table), messages, 0, 4, 2, 5);
    gtk_widget_show (messages);

    // Add list of Users.
    users_list = gtk_list_store_new (1, G_TYPE_STRING);

    users = create_list2 ("Users", users_list);
    gtk_table_attach_defaults (GTK_TABLE (table), users, 0, 2, 0, 2);
    gtk_widget_show (users);

    // Add messages text. Use columns 0 to 4 (exclusive) and rows 4 to 7 (exclusive) 

    myMessage = create_text2 ("");
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



    g_signal_connect (tree_view, "cursor-changed", G_CALLBACK(update_list_users), NULL);
    g_signal_connect (G_OBJECT(send_button), "clicked", G_CALLBACK(send_clicked), NULL);
    g_signal_connect (G_OBJECT(leave_button), "clicked", G_CALLBACK(join_clicked), NULL);
    g_signal_connect (G_OBJECT(create_button), "clicked", G_CALLBACK(create_clicked), NULL);
    g_signal_connect (G_OBJECT(join_button), "clicked", G_CALLBACK(log_clicked), NULL);
    gtk_widget_show (table);
    gtk_widget_show (window);
    
    startGetMessageThread();

    gtk_main ();

    return 0;
}

