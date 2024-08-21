#include <iostream>
class UI {
public:
    const std::string color_reset = "\033[0m";   // Zurücksetzen der Farbe
    const std::string color_black = "\033[30m";  // Schwarz
    const std::string color_red = "\033[31m";    // Rot
    const std::string color_green = "\033[32m";  // Grün
    const std::string color_yellow = "\033[33m"; // Gelb
    const std::string color_blue = "\033[34m";   // Blau
    const std::string color_magenta = "\033[35m";// Magenta
    const std::string color_cyan = "\033[36m";   // Cyan
    const std::string color_white = "\033[37m";  // Weiß

    void printLogo() {
        // Print Logo to screen
        std::cout << "\033[33m"; // 33 code for gold color

        std::cout << " $$$$$$\\                                \n";
        std::cout << "$$  __$$\\                               \n";
        std::cout << "$$ /  $$ | $$$$$$\\   $$$$$$\\   $$$$$$\\  \n";
        std::cout << "$$$$$$$$ |$$  __$$\\ $$  __$$\\ $$  __$$\\ \n";
        std::cout << "$$  __$$ |$$$$$$$$ |$$ |  \\__|$$ /  $$ |\n";
        std::cout << "$$ |  $$ |$$   ____|$$ |      $$ |  $$ |\n";
        std::cout << "$$ |  $$ |\\$$$$$$$\\ $$ |      \\$$$$$$  |\n";
        std::cout << "\\__|  \\__| \\_______|\\__|       \\______/ \n";

        std::cout << "\n\nMade By NGon001 & SkyCat537\n\n";
        // Reset color to white
        std::cout << "\033[0m";
    }
};