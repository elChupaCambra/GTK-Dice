#include <gtk/gtk.h>

// Defining interactive elements
// Main window
GtkWidget *total_display_label;
GtkWidget *list_display_label;
GtkWidget *sides_input_spin;
GtkWidget *amount_input_spin;
// Stats window
GtkWidget *stats_display_label;

// Dice defaults
int sides_dice = 6;
int amount_dice = 2;

int dice_rack[1000]; // Holds all roll values
int roll_history[10000]; // History of roll totals

// Dice button handlers
void print_dice() {
	char output[1024]; // String that gets pushed onto label
	output[0] = '\0';
	int i = 0;
	int total = 0;
	char list_buffer[50]; // Where each line of the list is held
	while(dice_rack[i] != 0) {
		if(i == 0) // Avoid first line of results starting on second line of label
			sprintf(list_buffer, "DICE %i: %i", i + 1, dice_rack[i]);
		else
			sprintf(list_buffer, "\nDICE %i: %i", i + 1, dice_rack[i]);
			
		g_strlcat(output, list_buffer, 1024); 
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

void roll_dice() { // Called by roll button
	int roll_total = 0;
	
	memset(dice_rack, 0, sizeof dice_rack);
	for(int i = 0; i < amount_dice; i++) {
		dice_rack[i] = rand() % sides_dice + 1;
		roll_total += dice_rack[i];
	}
	
	int i = 0;
	while(roll_history[i] != 0) {
		i++;
	}
	roll_history[i] = roll_total;
	
	print_dice();
}

void sides_spin_handler() {
	int quantity = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(sides_input_spin));
	sides_dice = quantity;
}

void amount_spin_handler() {
	int quantity = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(amount_input_spin));
	amount_dice = quantity;
}

// Window stuff handlers
void show_stats_window() {
	GtkBuilder *stats_builder;
	GObject *stats_window;
	
	stats_builder = gtk_builder_new_from_file("ui/stats.ui");
	stats_window = gtk_builder_get_object(stats_builder, "stats_window");
	gtk_builder_connect_signals(stats_builder, NULL);
	
	if (roll_history[0] != 0) { // If there has been a roll...
		stats_display_label = GTK_WIDGET(gtk_builder_get_object(stats_builder, "stats_display"));
		
		char output[4096]; // This will be set as the target label text
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
		g_strlcat(output, stat_buffer, 4096);
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
			g_strlcat(output, stat_buffer, 4096);
			memset(stat_buffer, 0, sizeof stat_buffer);
			
			// Clear variables
			memset(bar, 0, sizeof bar);
			current_total = 0;
			this_num_count = 0;
		}
		gtk_label_set_markup(GTK_LABEL(stats_display_label), output);
	}
	
	g_object_unref(stats_builder);
	
	gtk_widget_show(GTK_WIDGET (stats_window));
}

void reset() {
	sides_dice = 6;
	amount_dice = 2;
	memset(dice_rack, 0, sizeof dice_rack);
	memset(roll_history, 0, sizeof roll_history);
	
	// These vars have already been initialised, saving us from calling Builder again.
	gtk_label_set_markup(GTK_LABEL(total_display_label), "0");
	gtk_label_set_markup(GTK_LABEL(list_display_label), "");
	gtk_spin_button_set_value((GtkSpinButton*)sides_input_spin, sides_dice);
	gtk_spin_button_set_value((GtkSpinButton*)amount_input_spin, amount_dice);
}

// Called when window is closed
void on_main_window_destroy() {
    gtk_main_quit();
}

void on_stats_window_destroy(GtkWindow *window) {
    gtk_window_close(window);
}


int main (int argc, char **argv) { //Main function should be as small as possible.
	GtkBuilder *builder;
	GObject *window;
	// GObject *button;
	GError *error = NULL;
	  
	gtk_init(&argc, &argv);
	  
	// Construct GtkBuilder instance and load UI description
	builder = gtk_builder_new(); 
	if (gtk_builder_add_from_file (builder, "ui/dice.ui", &error) == 0) {
		g_printerr ("Error loading dice.ui file: %s\n", error->message);
		g_clear_error (&error);
		return 1;
	} 
	  
	// Signal connections brought in from XML
	gtk_builder_connect_signals(builder, NULL);
	
	window = gtk_builder_get_object(builder, "main_window");
	
	// Otherwise, we can use these:
	// Connect signal handlers to constructed widgets
    // g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
	// button = gtk_builder_get_object (builder, "roll_button");
	// g_signal_connect (button, "clicked", G_CALLBACK(roll_dice), NULL);
	
	// Connecting builder widgets to previously defined interactive elements
	total_display_label = GTK_WIDGET(gtk_builder_get_object(builder, "total_display"));
	list_display_label = GTK_WIDGET(gtk_builder_get_object(builder, "list_display"));
	sides_input_spin = GTK_WIDGET(gtk_builder_get_object(builder, "sides_input"));
	amount_input_spin = GTK_WIDGET(gtk_builder_get_object(builder, "amount_input"));
	
	// Set spin input numbers to default values
	gtk_spin_button_set_value((GtkSpinButton*)sides_input_spin, sides_dice);
	gtk_spin_button_set_value((GtkSpinButton*)amount_input_spin, amount_dice);
	
	//Random num generator for dice, seed set as current time
	srand(time(NULL));
	
	g_object_unref(builder); // Free builder from memory
	gtk_widget_show(GTK_WIDGET (window)); //Show the window 
	gtk_main();
	
	return 0;
}
