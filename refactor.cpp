#include "headers/classes.hpp"

int main()
{
    setlocale(LC_ALL, "");
    std::setlocale(LC_ALL, nullptr);

    // Initialize ncurses
    initscr();

    // Give ncurses more control over key presses
    keypad(stdscr, TRUE);

    // Disable CTRL-C to kill
    cbreak();

    // Check if terminal supports colors
    if (has_colors() == FALSE)
    {
        endwin();
        std::cerr << "Terminal does not support color. Exiting" << std::endl;
        return -1;
    }

    // This allows for terminal transparency
    use_default_colors();
    start_color();
    init_pair(1, COLOR_YELLOW, COLOR_BLACK); // Unselected option
    init_pair(2, COLOR_CYAN, COLOR_BLACK);   // UI Color
    init_pair(3, COLOR_BLACK, COLOR_YELLOW); // Selected option
    init_pair(4, COLOR_RED, COLOR_BLACK);    // Bad
    init_pair(5, COLOR_GREEN, COLOR_BLACK);  // Good
    // RGB color pairs
    init_pair(10, COLOR_BLACK, COLOR_RED);     // Red highlighted
    init_pair(11, COLOR_BLACK, COLOR_GREEN);   // Green highlighted
    init_pair(12, COLOR_BLACK, COLOR_BLUE);    // Blue highlighted
    init_pair(13, COLOR_BLACK, COLOR_MAGENTA); // Magenta highlighted
    init_pair(14, COLOR_BLACK, COLOR_WHITE);   // Monochrome highlighted
    init_pair(15, COLOR_BLUE, COLOR_BLACK);    // Blue not highlighted
    init_pair(16, COLOR_MAGENTA, COLOR_BLACK); // Magenta not highlighted
    init_pair(17, COLOR_WHITE, COLOR_BLACK);
    //wissen::quizzer quizzer;
    //quizzer.pickVocab();
    endwin();
    return 0;
}