#include <iostream>

int main() {
    // Colors
    std::cout << "\033[31mRed text\033[0m" << std::endl;     // Red
    std::cout << "\033[32mGreen text\033[0m" << std::endl;   // Green
    std::cout << "\033[33mYellow text\033[0m" << std::endl;  // Yellow
    std::cout << "\033[34mBlue text\033[0m" << std::endl;    // Blue

    // Background colors
    std::cout << "\033[41mRed background\033[0m" << std::endl;
    std::cout << "\033[42mGreen background\033[0m" << std::endl;

    // Bold / Bright
    std::cout << "\033[1mBold text\033[0m" << std::endl;

    // Emoji example (works if terminal supports UTF-8)
    std::cout << "\033[35m\U0001F600 Emoji in Magenta\033[0m" << std::endl;

    return 0;
}