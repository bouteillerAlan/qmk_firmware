#include "quantum.h"

#define WPM_HISTORY_LENGTH 10 // Adjustable history length
#define MAX_WPM 100 // Maximum WPM value for scaling
#define GRAPH_HEIGHT 50 // Height of the graph in pixels
#define OLED_HEIGHT 128
#define OLED_WIDTH 32
static uint8_t wpm_history[WPM_HISTORY_LENGTH] = {0};
static uint8_t wpm_index = 0;

// Function to add a new WPM value to the history
void update_wpm_history(uint8_t current_wpm) {
    wpm_history[wpm_index] = current_wpm > MAX_WPM ? MAX_WPM : current_wpm;
    wpm_index = (wpm_index + 1) % WPM_HISTORY_LENGTH;
}

// Bresenham's line algorithm to draw lines on the OLED
void oled_write_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, bool color) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;
    for (;;) {
        oled_write_pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

// Function to draw the WPM history graph
void draw_wpm_graph(void) {
    uint8_t graph_bottom = OLED_HEIGHT - 1; // Bottom Y coordinate of the graph
    uint8_t point_spacing = OLED_WIDTH / (WPM_HISTORY_LENGTH - 1); // Spacing between points

    for (uint8_t i = 0; i < WPM_HISTORY_LENGTH - 1; i++) {
        // Scale the WPM value to the graph height
        uint8_t y0 = graph_bottom - (wpm_history[i] * GRAPH_HEIGHT / MAX_WPM);
        uint8_t y1 = graph_bottom - (wpm_history[i + 1] * GRAPH_HEIGHT / MAX_WPM);

        // Calculate the X coordinates
        uint8_t x0 = i * point_spacing;
        uint8_t x1 = (i + 1) * point_spacing;

        // Draw the line between the current and next point
        oled_write_line(x0, y0, x1, y1, true); // true for color (on), assuming a monochrome display
    }
}

// Main function to be called from oled_task_kb to update and draw the WPM graph
void render_wpm_graph(void) {
    update_wpm_history(get_current_wpm());
    draw_wpm_graph();
}
