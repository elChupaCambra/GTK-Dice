#include <gtk/gtk.h>
#include "dice.h"
#include "windows.h"

// Main window
GtkWidget *total_display_label;
GtkWidget *list_display_label;
GtkWidget *sides_input_spin;
GtkWidget *amount_input_spin;
GtkWidget *stack;
GtkWidget *stack_switcher;
GtkWidget *text_view_scrollwindow;
GtkWidget *icon_view;
// Stats window
GtkWidget *stats_display_label;

// Dice images
enum {
  COL_DISPLAY_NAME,
  COL_PIXBUF,
  NUM_COLS
};

#define DICE_6_1 "resources/Dice-1.png"
#define DICE_6_2 "resources/Dice-2.png"
#define DICE_6_3 "resources/Dice-3.png"
#define DICE_6_4 "resources/Dice-4.png"
#define DICE_6_5 "resources/Dice-5.png"
#define DICE_6_6 "resources/Dice-6.png"
const char *dice_6_filenames[] = {DICE_6_1, DICE_6_2, DICE_6_3, DICE_6_4, DICE_6_5, DICE_6_6};

static GdkPixbuf* dice_6_pixbufs[6];

// Need to load the pictures into memory first
static void load_pixbufs () {
	for (int i = 0; i < 6; i++) {
		dice_6_pixbufs[i] = gdk_pixbuf_new_from_file (dice_6_filenames[i], NULL);
		g_assert (dice_6_pixbufs[i]); // Must be loaded successfully
		// Resize image
		dice_6_pixbufs[i] = gdk_pixbuf_scale_simple(dice_6_pixbufs[i], 96, 96, GDK_INTERP_BILINEAR);
	}
}

static void fill_store (GtkListStore *store) {
	GtkTreeIter iter;
	
	// Clear the store of old stuff
	gtk_list_store_clear (store);
	
	int i = 0;
	while(dice_rack[i] != 0) { // Go through each dice in rack
		GdkPixbuf *current_dice_pixbuf;
		char dice_name[9];
		
		sprintf(dice_name, "DICE %i", i + 1);
		// Current pixbuf setter
		switch(dice_rack[i]) {
			case 1:
				current_dice_pixbuf = dice_6_pixbufs[0];
				break;
			case 2:
				current_dice_pixbuf = dice_6_pixbufs[1];
				break;
			case 3:
				current_dice_pixbuf = dice_6_pixbufs[2];
				break;
			case 4:
				current_dice_pixbuf = dice_6_pixbufs[3];
				break;
			case 5:
				current_dice_pixbuf = dice_6_pixbufs[4];
				break;
			case 6: 
				current_dice_pixbuf = dice_6_pixbufs[5];
				break;
		}
	
		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter, 
							COL_DISPLAY_NAME, dice_name,
							COL_PIXBUF, current_dice_pixbuf,
							-1);
		i++;
	}			
}

static GtkListStore* create_store () {
	GtkListStore *store;
	
	store = gtk_list_store_new (NUM_COLS, 
								G_TYPE_STRING, 
								GDK_TYPE_PIXBUF,
								G_TYPE_BOOLEAN);
	// NOTE: You might want a sort function to show the dice in custom orders?
	return store;
}

void print_icon_view() {
	GtkListStore *store;
	
	load_pixbufs();
	store = create_store();
	fill_store(store);
	
	gtk_icon_view_set_model (GTK_ICON_VIEW (icon_view), GTK_TREE_MODEL (store));
	g_object_unref (store);
	gtk_icon_view_set_text_column (GTK_ICON_VIEW (icon_view), COL_DISPLAY_NAME);
    gtk_icon_view_set_pixbuf_column (GTK_ICON_VIEW (icon_view), COL_PIXBUF);
}

void print_dice() {
	char output[16384]; // String that gets pushed onto label
	output[0] = '\0';
	int i = 0;
	int total = 0;
	char list_buffer[50]; // Where each line of the list is held
	while(dice_rack[i] != 0) {
		if(i == 0) // Avoid first line of results starting on second line of label
			sprintf(list_buffer, "DICE %i: %i", i + 1, dice_rack[i]);
		else
			sprintf(list_buffer, "\nDICE %i: %i", i + 1, dice_rack[i]);
			
		g_strlcat(output, list_buffer, 16384); 
		total += dice_rack[i];
		i++;
	}
	
	//Printing list of dice
	gtk_label_set_markup(GTK_LABEL(list_display_label),	output);
	memset(list_buffer, 0, sizeof list_buffer);
	
	// Printing total
	char charred_total[50];
	sprintf(charred_total, "<big><b>%i</b></big>", total);
	gtk_label_set_markup(GTK_LABEL(total_display_label), charred_total);
	memset(output, 0, sizeof output);
}

void reset() {
	sides_dice = 6;
	amount_dice = 2;
	memset(dice_rack, 0, sizeof dice_rack);
	memset(roll_history, 0, sizeof roll_history);
	
	// These vars have already been initialised, saving us from calling Builder again.
	print_dice();
	print_icon_view();
	gtk_label_set_markup(GTK_LABEL(total_display_label), "0");
	gtk_spin_button_set_value((GtkSpinButton*)sides_input_spin, sides_dice);
	gtk_spin_button_set_value((GtkSpinButton*)amount_input_spin, amount_dice);
}

void show_stats_window() {
	GtkBuilder *stats_builder;
	GObject *stats_window;
	
	stats_builder = gtk_builder_new_from_file("ui/stats.ui");
	stats_window = gtk_builder_get_object(stats_builder, "stats_window");
	gtk_builder_connect_signals(stats_builder, NULL);
	
	if (roll_history[0] != 0) { // If there has been a roll...
		stats_display_label = GTK_WIDGET(gtk_builder_get_object(stats_builder, "stats_display"));
		
		char output[1048576]; // This will be set as the target label text
		char stat_buffer[50]; // Buffer for each line of the output text
		int min_num = roll_history[0], max_num = roll_history[0], nextnum, i = 0; // Min and max totals to be printed
		
		// Set min_num smallest number in results total array
		while(roll_history[i] != 0) { 
			nextnum = roll_history[i];
			
			if(min_num > nextnum)
				min_num = nextnum;
			
			i++;
		}
		i = 0; // Reset iteration counter for next uses
		
		// Set max_num to biggest number in results total array
		while(roll_history[i] != 0) { 
			nextnum = roll_history[i];
			
			if(max_num < nextnum)
				max_num = nextnum;
			
			i++;
		}
		i = 0;
		
		// Printing bar chart
		// Add header to bar chart, with markup
		sprintf(stat_buffer, "<big><b>FREQUENCY OF TOTALS:</b></big>\n");
		g_strlcat(output, stat_buffer, 1048576);
		memset(stat_buffer, 0, sizeof stat_buffer); // Reset stat line buffer
		
		// current_total holds the total that is going to have its frequency calculated.
		// this_num_count holds the frequency of this total.
		int current_total = 0, this_num_count = 0; 
		// Bar is the actual bar to be printed on the line.
		char bar[45];
		// This for loop starts at the min total result and goes to the max.
		for(int num = min_num; num <= max_num; num++) {
		
			//Count number of occurrences of each total	
			while(roll_history[current_total] != 0) {
				if (roll_history[current_total] == num)
					this_num_count++;
				current_total++;
			}
			
			//Draw bar for bar chart
			int barcount;
			for(barcount = 0; barcount < this_num_count; barcount++) {
				bar[barcount] = '#'; // Stamp char onto bar array
			}
			bar[barcount] = '\0'; // Add string finisher to prevent garbage being printed
			
			// Print bar and details onto the line buffer
			sprintf(stat_buffer, "%i:\t%s\n", num, bar);
			g_strlcat(output, stat_buffer, 1048576);
			memset(stat_buffer, 0, sizeof stat_buffer);
			
			// Clear variables
			memset(bar, 0, sizeof bar);
			current_total = 0;
			this_num_count = 0;
		}
		gtk_label_set_markup(GTK_LABEL(stats_display_label), output);
		memset(output, 0, sizeof output);
	}
	
	g_object_unref(stats_builder);
	
	gtk_widget_show(GTK_WIDGET (stats_window));
}


// Signal handlers
void sides_spin_handler() {
	int quantity = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(sides_input_spin));
	sides_dice = quantity;
}

void amount_spin_handler() {
	int quantity = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(amount_input_spin));
	amount_dice = quantity;
}

void roll_button_handler() {
	roll_dice();
	print_dice();
	
	if(sides_dice == 6) {
		gtk_widget_set_sensitive(stack_switcher, TRUE); // Enable switcher
		print_icon_view();
	} else {
		gtk_widget_set_sensitive(stack_switcher, FALSE); // Disable switcher
		// Return to text view of on other stack page
		gtk_stack_set_visible_child (GTK_STACK(stack), text_view_scrollwindow);
	}
}

void clear_button_handler() {
	reset();
}

void stats_button_handler() {
	show_stats_window();
}

void on_main_window_destroy() {
    gtk_main_quit();
}

void on_stats_window_destroy(GtkWindow *window) {
    gtk_window_close(window);
}
