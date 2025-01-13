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

std::string preload_cmd = "hyprctl hyprpaper preload ";
std::string load_cmd = "hyprctl hyprpaper wallpaper ,";

struct Vector4u {
    uint top;
    uint bottom;
    uint left;
    uint right;
};

class WallpaperButton {
    public:
        WallpaperButton(sf::Text text, sf::Vector2f size, sf::Vector2f position, sf::Texture& texture, std::string path) :
            shape(size), text(text), position(position) {
                this->shape.setPosition(position);
                this->shape.setTexture(&texture);
                this->path = path;
                this->text.setOrigin(text.getLocalBounds().size);
                this->text.setPosition(position + size - (sf::Vector2f){10.0f, 10.0f});
        }

        void draw(sf::RenderTarget& target, sf::RenderStates states) {
            target.draw(shape);
            target.draw(text);
        }
        void onHover(sf::Vector2f mouse_pos, sf::Color outline_color) {
            shape.setOutlineColor(outline_color);
            if (shape.getGlobalBounds().contains(mouse_pos)) {
                isHovered = true;
                shape.setOutlineThickness(2.0f);
            } else {
                isHovered = false;
                shape.setOutlineThickness(0);
            }
        }
        void onClick() {
            if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && isHovered) {
                std::string cmd1 = preload_cmd + path;
                std::string cmd2 = load_cmd + path;

                std::cout << cmd1 << "\n" << cmd2 << std::endl;
                std::system(cmd1.c_str());
                std::system(cmd2.c_str());
                // exit(0);
            }
        }

    private:
        // Button properties like position, size, texture, etc.
        sf::RectangleShape shape;
        sf::Text text;
        sf::Vector2f position;
        std::string path;

        // State properties
        bool isHovered = false;
};

int main(int argc, char* argv[]) {
    // CLI
    uint cols = 3;
    sf::Color border_color(255, 255, 0);
    sf::Vector2u window_size = {900, 600};
    sf::Vector2u inner_gaps = {10, 10};
    sf::Vector2u outer_gaps = {25, 25};
    sf::Vector2f thumb_size;
    sf::Font font = sf::Font("/usr/share/fonts/TTF/Vera.ttf");
    std::string wp_dir = "/home/ian/.config/hypr/bg/";
    std::vector<std::string> wp_paths;
    std::vector<sf::Texture> wp_textures;
    std::vector<WallpaperButton> buttons;

    thumb_size.x = ((float)window_size.x - (((cols - 1) * inner_gaps.x) + (outer_gaps.x * 2))) / cols;
    thumb_size.y = thumb_size.x * (9.0/16.0);

    for (const auto &entry : std::filesystem::directory_iterator(wp_dir)) {
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

    for (size_t i = 0, x = outer_gaps.x, y = outer_gaps.y; i < wp_paths.size(); i++, x += thumb_size.x + inner_gaps.x) {
        if (i % cols == 0 && i != 0) {
            y += thumb_size.y + inner_gaps.y;
            x = outer_gaps.x;
        }
        sf::Text text(font, std::to_string(i + 1), 16);
        text.setOutlineColor(sf::Color::Black);
        text.setOutlineThickness(2.0f);
        sf::Vector2f pos = {(float)x, (float)y};
        WallpaperButton button(text, (sf::Vector2f)thumb_size, pos, wp_textures[i], wp_paths[i]);
        buttons.push_back(button);
    }

    sf::Vector2i mouse_pos;

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }
        mouse_pos = sf::Mouse::getPosition(window);
        window.clear();

        // Draw wallpapers to screen
        for (WallpaperButton &btn : buttons) {
            btn.onHover((sf::Vector2f)mouse_pos, border_color);
            btn.onClick();
            btn.draw(window, sf::RenderStates::Default);
        }

        window.display();
    }


    return 1;
}
