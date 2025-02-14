#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/ContextSettings.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/Window.hpp>
#include <SFML/Window/WindowEnums.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <cstddef>
#include <cstdlib>
#include <SFML/Main.hpp>
#include <SFML/Graphics.hpp>
#include <cwchar>
#include <optional>
#include <string>
#include <iostream>
#include <filesystem>
#include <fstream>

class WallpaperButton {
    public:
        WallpaperButton(sf::RectangleShape& shape, std::string path) :
            shape(shape) {
                this->path = path;
                this->outlineThickness = shape.getOutlineThickness();
        }
        void draw(sf::RenderTarget& target, sf::RenderStates states) {
            target.draw(shape, states);
            // target.draw(text, states);
        }
        void onHover(sf::Vector2f mouse_pos) {
            if (shape.getGlobalBounds().contains(mouse_pos)) {
                isHovered = true;
                shape.setOutlineThickness(outlineThickness);
            } else {
                isHovered = false;
                shape.setOutlineThickness(0);
            }
        }
        void onClick(std::vector<std::string> &commands) {
            if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && isHovered) {
                for (std::string &cmd : commands) {
                    int wp_index = cmd.find("{wp}");
                    cmd.erase(wp_index, 4);
                    cmd.insert(wp_index, path);
                    int result = std::system(cmd.c_str());
                    if (result != 0) {
                        std::cerr << "<!> Error running commands." << std::endl;
                        shape.setOutlineColor(sf::Color::Red);
                    }
                }
                exit(0);
            }
        }
        private:
            // Button properties like position, size, texture, etc.
            sf::RectangleShape shape;
            sf::Vector2f position;
            std::string path;

            // State properties
            bool isHovered = false;
            float outlineThickness = 0;
};

// Variables
uint column_count = 3;
float outline_thickness = 2;
sf::Color outline_color(255, 255, 0);
sf::Color background_color(0, 0, 0);
std::string config_path = "";
std::string wallpaper_directory = "";
sf::Vector2u window_size = {900, 600};
sf::Vector2u inner_gaps = {10, 10};
sf::Vector2u outer_gaps = {25, 25};
sf::Vector2f thumb_size;
std::vector<std::string> wp_paths;
std::vector<sf::Texture> wp_textures;
std::vector<std::string> exec_commands;
std::vector<WallpaperButton> buttons;

std::string trim (const std::string& str);
void parse_config(std::string config_path);

int main(int argc, char* argv[]) {
    if (argc <= 1) {
        std::cout << "<!> Not enough arguments." << std::endl;
        return 1;
    }
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            std::cout << "Help Comming Soon." << std::endl;
            return 1;
        } else if (arg == "-c") {
            if (i + 1 == argc) {
                std::cerr << "<!> No path specified after -c" << std::endl;
                return 1;
            }
            arg = argv[++i];
            if (!std::filesystem::exists(arg)) {
                std::cerr << "<!> Config file dosen't exist." << std::endl;
                return 1;
            }
            config_path = arg;
            i++;
        } else {
            std::cerr << "<!> Not a valid argument: " << arg << std::endl;
            return 1;
        }
    }
    parse_config(config_path);

    thumb_size.x = ((float)window_size.x - (((column_count - 1) * inner_gaps.x) + (outer_gaps.x * 2))) / column_count;
    thumb_size.y = thumb_size.x * (9.0/16.0);

    // Load Wallpapers
    for (const auto &entry : std::filesystem::directory_iterator(wallpaper_directory)) {
        std::string wp_path = entry.path().string();
        sf::Texture texture(wp_path);
        wp_paths.push_back(wp_path);
        wp_textures.push_back(texture);
    }

    // GUI
    sf::ContextSettings settings;
    settings.antiAliasingLevel = 3;
    sf::RenderWindow window(sf::VideoMode(window_size), "SFML Window", sf::Style::None, sf::State::Windowed, settings);
    window.setPosition(((sf::Vector2i)sf::VideoMode::getDesktopMode().size / 2) - ((sf::Vector2i)window_size / 2));

    // Create Wallpaper Buttons
    for (size_t i = 0, x = outer_gaps.x, y = outer_gaps.y; i < wp_paths.size(); i++, x += thumb_size.x + inner_gaps.x) {
        if (i % column_count == 0 && i != 0) {
            y += thumb_size.y + inner_gaps.y;
            x = outer_gaps.x;
        }
        sf::RectangleShape shape((sf::Vector2f) thumb_size);
        shape.setOutlineColor(outline_color);
        shape.setOutlineThickness(outline_thickness);
        shape.setTexture(&wp_textures[i]);
        shape.setPosition({(float)x, (float)y});
        WallpaperButton button(shape, wp_paths[i]);
        buttons.push_back(button);
    }

    sf::RectangleShape background((sf::Vector2f)window_size);
    background.setFillColor(background_color);

    // App Loop
    sf::Vector2i mouse_pos;
    while (window.isOpen()) {
        window.clear();
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        window.draw(background);

        mouse_pos = sf::Mouse::getPosition(window);

        // Draw wallpapers to screen
        for (WallpaperButton &btn : buttons) {
            btn.onHover((sf::Vector2f)mouse_pos);
            btn.onClick(exec_commands);
            btn.draw(window, sf::RenderStates::Default);
        }

        window.display();
    }
    return 1;
}

std::string trim (const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r\f\v");
    size_t last = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(first, last);
}

void parse_config(std::string config_path) {
    std::fstream config_file(config_path, std::ios::in);
    if (!config_file.is_open()) {
        std::cerr << "<!> Error: Unable to open config file." << std::endl;
        exit(1);
    }
    std::vector<std::string> lines;
    // logic
    std::string line;
    while (std::getline(config_file, line)) {
        lines.push_back(line);
    }
    for (size_t i = 0; i < lines.size(); i++) {
        std::string line = lines[i];
        std::string var = line.substr(0, line.find_first_of(' '));
        std::string value = line.substr(line.find_first_of('=') + 1);
        value = trim(value);
        if (var == "wallpaper_directory") {
            wallpaper_directory = value;
            if (!std::filesystem::is_directory(wallpaper_directory)) {
                std::cerr << "<!> wallpaper_directory variable is not valid." << std::endl;
                exit(1);
            }
        } else if (var == "window_size") {
            uint x = std::stoi(value.substr(0, value.find(',')));
            uint y = std::stoi(value.substr(value.find(',') + 1));
            window_size.x = x;
            window_size.y = y;
        } else if (var == "outline_thickness") {
            outline_thickness = std::stof(value);
        } else if (var == "outline_color") {
            char r = std::stoi(value.substr(value.find('[') + 1, value.find_first_of(',')));
            char g = std::stoi(value.substr(value.find_first_of(',') + 1, value.find_last_of(',')));
            char b = std::stoi(value.substr(value.find_last_of(',') + 1, value.find(']')));
            outline_color = sf::Color(r, g, b);
        } else if (var == "inner_gaps") {
            uint x;
            uint y;
            if (var.find(',')) {
                x = std::stoi(value.substr(0, value.find(',')));
                y = std::stoi(value.substr(value.find(',') + 1));
            } else {
                x = std::stoi(value);
                y = std::stoi(value);
            }
            inner_gaps.x = x;
            inner_gaps.y = y;
        } else if (var == "outer_gaps") {
            uint x;
            uint y;
            if (var.find(',')) {
                x = std::stoi(value.substr(0, value.find(',')));
                y = std::stoi(value.substr(value.find(',') + 1));
            } else {
                x = std::stoi(value);
                y = std::stoi(value);
            }
            outer_gaps.x = x;
            outer_gaps.y = y;
        } else if (var == "column_count") {
            column_count = std::stoi(value);
        } else if (var == "exec") {
            if (value[0] != '[') {
                exec_commands.push_back(value);
            } else {
                while (lines[++i][0] != ']') {
                    exec_commands.push_back(trim(lines[i]));
                }
            }
        } else if (var == "background_color") {
            char r = std::stoi(value.substr(value.find('[') + 1, value.find_first_of(',')));
            char g = std::stoi(value.substr(value.find_first_of(',') + 1, value.find_last_of(',')));
            char b = std::stoi(value.substr(value.find_last_of(',') + 1, value.find(']')));
            background_color = sf::Color(r, g, b);
        }
    }
    config_file.close();
}
