#ifndef GRAPH_H
#define GRAPH_H

// Function to add a new WPM value to the history
void update_wpm_history(uint8_t current_wpm);

// Function to draw the WPM history graph
void draw_wpm_graph(bool scrollRight);

// Main function to be called from oled_task_kb to update and draw the WPM graph
void render_wpm_graph(void);

#endif
