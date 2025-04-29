#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/ContextSettings.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/Window.hpp>
#include <SFML/Window/WindowEnums.hpp>
#include <SFML/Main.hpp>
#include <SFML/Graphics.hpp>
#include <string>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <system_error>

class WP {
    std::string _path;
    std::string _filename;
    sf::Texture _texture;

    public:
        WP(std::string path) : _path(path) {
            _filename = _path.substr(_path.find_last_of("/") + 1);
            _texture = sf::Texture(path);
        }
        sf::Texture& getTexture() {
            return _texture;
        }
        std::string getFilename() {
            return _filename;
        }
        std::string getPath() {
            return _path;
        }
};

class WPButton {
    bool _isSelected = false;
    float _outlineThickness = 0;
    std::string _path;
    sf::RectangleShape _shape;
    sf::Vector2i _position;
    public:
        static inline sf::Vector2i selected = {0, 0};
        static inline std::vector<std::string> commands;
        WPButton(sf::RectangleShape& shape, std::string path, sf::Vector2i position) :
            _shape(shape) {
                _path = path;
                _outlineThickness = shape.getOutlineThickness();
                _position = position;
        }
        void draw(sf::RenderTarget& target, sf::RenderStates states) {
            target.draw(_shape, states);
        }
        void onSelected() {
            if (WPButton::selected == _position) {
                _isSelected = true;
                _shape.setOutlineThickness(_outlineThickness);
            } else {
                _isSelected = false;
                _shape.setOutlineThickness(0);
            }
        }
        void onEnter() {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter) && _isSelected) {
                execute();
            }
        }
        void execute() {
            for (std::string &cmd : commands) {
                int wp_index = cmd.find("{wp}");
                cmd.erase(wp_index, 4);
                cmd.insert(wp_index, _path);
                int result = std::system(cmd.c_str());
                if (result != 0) {
                    std::cerr << "<!> Error running commands." << std::endl;
                    _shape.setOutlineColor(sf::Color::Red);
                }
            }
            exit(0);
        }
        void setPosition(sf::Vector2f pos) {
            _shape.setPosition(pos);
        }
        bool getIsSelected() {
            return _isSelected;
        }
        sf::Vector2f getPosition() {
            return _shape.getPosition();
        }
        sf::Vector2i getGridPosition() {
            return _position;
        }
};

class KeyPlus {
    sf::Keyboard::Key key;
    bool key_pressed;
    bool key_released;
    bool key_down;
    public:
        KeyPlus(sf::Keyboard::Key key) {
            this->key = key;
            key_pressed = false;
            key_released = false;
            key_down = false;
        }
        bool isKeyReleased() {
            key_pressed = sf::Keyboard::isKeyPressed(key);
            if (key_released) {
                key_pressed = false;
                key_released = false;
                key_down = false;
            } else if (key_pressed && !key_released) {
                key_down = true;
            } else if (key_down && !key_pressed) {
                key_released = true;
            }
            return key_released;
        }
};

// Variables
int scroll_offset = 0;
uint column_count = 3;
float outline_thickness = 2.0f;
float scroll_speed = 0.1f;
sf::Color outline_color(255, 255, 0);
sf::Color background_color(0, 0, 0);
std::string config_path = "";
std::string wallpaper_directory = "";
sf::Vector2u window_size = {900, 600};
sf::Vector2u inner_gaps = {10, 10};
sf::Vector2u outer_gaps = {25, 25};
sf::Vector2f thumb_size;
std::vector<WP> wallpapers;
std::vector<std::string> exec_commands;
std::vector<std::vector<WPButton>> buttons;

bool isKeyReleased(sf::Keyboard::Key key);
std::string trim (const std::string& str);
void parseConfig(std::string config_path);

std::string help =
"Simple Wallpaper Switcher (SWPS)\n"
"---------- Arguments -----------\n"
"    -h     : help\n"
"    -c     : specify config file"
"";

int main(int argc, char* argv[]) {
    // CLI
    if (argc <= 1) {
        std::cout << "<!> Not enough arguments." << std::endl;
        return 1;
    }
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            std::cout << help << std::endl;
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
    parseConfig(config_path);

    WPButton::commands = exec_commands;

    thumb_size.x = ((float)window_size.x - (((column_count - 1) * inner_gaps.x) + (outer_gaps.x * 2))) / column_count;
    thumb_size.y = thumb_size.x * (9.0/16.0);

    // Load Wallpapers
    for (const auto &entry : std::filesystem::directory_iterator(wallpaper_directory)) {
        std::string wp_path = entry.path().string();
        WP wallpaper(wp_path);
        wallpapers.push_back(wallpaper);
    }

    // GUI
    sf::ContextSettings settings;
    settings.antiAliasingLevel = 3;
    sf::RenderWindow window(sf::VideoMode(window_size), "SFML Window", sf::Style::None, sf::State::Windowed, settings);
    window.setPosition(((sf::Vector2i)sf::VideoMode::getDesktopMode().size / 2) - ((sf::Vector2i)window_size / 2));

    // Create Wallpaper Buttons
    size_t wp_index = 0;
    size_t rows = (int)(wallpapers.size() / column_count) + (wallpapers.size() % column_count > 0 ? 1 : 0);
    size_t x = outer_gaps.x;
    size_t y = outer_gaps.y;
    for (size_t row = 0; row < rows; row++) {
        std::vector<WPButton> button_row = {};
        for (size_t col = 0; col < column_count; col++) {
            if (wp_index == wallpapers.size()) {
                break;
            }
            sf::RectangleShape shape((sf::Vector2f)thumb_size);
            shape.setOutlineColor(outline_color);
            shape.setOutlineThickness(outline_thickness);
            shape.setPosition({(float)x, (float)y});
            shape.setTexture(&wallpapers[wp_index].getTexture());
            WPButton button(shape, wallpapers[wp_index].getPath(), {(int)col, (int)row});
            button_row.push_back(button);
            x += thumb_size.x + inner_gaps.x;
            wp_index++;
        }
        y += thumb_size.y + inner_gaps.y;
        x = outer_gaps.x + scroll_offset;
        buttons.push_back(button_row);
    }

    // Background
    sf::RectangleShape background((sf::Vector2f)window_size);
    background.setFillColor(background_color);

    // Move keys
    KeyPlus k_key(sf::Keyboard::Key::K);
    KeyPlus j_key(sf::Keyboard::Key::J);
    KeyPlus h_key(sf::Keyboard::Key::H);
    KeyPlus l_key(sf::Keyboard::Key::L);

    KeyPlus up_key(sf::Keyboard::Key::Up);
    KeyPlus down_key(sf::Keyboard::Key::Down);
    KeyPlus left_key(sf::Keyboard::Key::Left);
    KeyPlus right_key(sf::Keyboard::Key::Right);

    // App Loop
    while (window.isOpen()) {
        window.clear();
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        // Move via VIM or Arrow keys
        if (k_key.isKeyReleased() || up_key.isKeyReleased()) {
            if (WPButton::selected.y > 0) {
                WPButton::selected.y -= 1;
            }
        } else if (j_key.isKeyReleased() || down_key.isKeyReleased()) {
            if (WPButton::selected.y < (int)buttons.size() - 1) {
                if (WPButton::selected.x < (int)buttons[WPButton::selected.y + 1].size()) {
                    WPButton::selected.y += 1;
                }
            }
        } else if (h_key.isKeyReleased() || left_key.isKeyReleased()) {
            if (WPButton::selected.x > 0) {
                WPButton::selected.x -= 1;
            }
        } else if (l_key.isKeyReleased() || right_key.isKeyReleased()) {
            if (WPButton::selected.x < (int)buttons[0].size() - 1) {
                if (WPButton::selected.x < (int)buttons[WPButton::selected.y].size() - 1) {
                    WPButton::selected.x += 1;
                }
            }
        }

        // Scroll Functionality
        WPButton tmpBtn = buttons[WPButton::selected.y][WPButton::selected.x];
        if (tmpBtn.getPosition().y + thumb_size.y > window_size.y) {
            for (size_t row = 0; row < buttons.size(); row++) {
                for (size_t col = 0; col < buttons[0].size(); col++) {
                    if (col == buttons[row].size()) {
                        break;
                    }
                    sf::Vector2f pos = buttons[row][col].getPosition();
                    buttons[row][col].setPosition({pos.x, pos.y - thumb_size.y});
                }
            }
        } else if (tmpBtn.getPosition().y < 0) {
            for (size_t row = 0; row < buttons.size(); row++) {
                for (size_t col = 0; col < buttons[0].size(); col++) {
                    if (col == buttons[row].size()) {
                        break;
                    }
                    sf::Vector2f pos = buttons[row][col].getPosition();
                    buttons[row][col].setPosition({pos.x, pos.y + thumb_size.y});
                }
            }
        }

        window.draw(background);

        // Draw wallpapers to screen
        for (auto rows : buttons) {
            for (auto btn : rows) {
                btn.onSelected();
                btn.draw(window, sf::RenderStates::Default);
                btn.onEnter();
            }
        }

        window.display();
    }
    return 1;
}

bool isKeyReleased(sf::Keyboard::Key key) {
    if (sf::Keyboard::isKeyPressed(key)) {
        if (!sf::Keyboard::isKeyPressed(key)) {
            return true;
        }
    }
    return false;
}

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r\f\v");
    size_t last = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(first, last);
}

std::string getHomeDir() {
    char * home = std::getenv("HOME");
    if (!home) {
        throw std::runtime_error("Failed to get home directory");
    }
    return std::string(home);
}

void parseConfig(std::string config_path) {
    std::fstream config_file(config_path, std::ios::in);
    if (!config_file.is_open()) {
        std::cerr << "<!> Error: Unable to open config file." << std::endl;
        exit(1);
    }
    std::vector<std::string> lines;
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
            if (wallpaper_directory.find('~') == 0) {
                try {
                    wallpaper_directory = getHomeDir() + wallpaper_directory.substr(1);
                } catch (std::error_code e) {
                    std::cerr << "<!> Error: " << e << std::endl;
                    exit(1);
                }
            }
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
        } else if (var == "scroll_speed") {
            scroll_speed = std::stof(value) / 100.0f;
        }
    }
    config_file.close();
}
