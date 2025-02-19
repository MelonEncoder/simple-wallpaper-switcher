# Simple Wallpaper Swaper
Almost all desktops environments and especially window managers should have some form of cli tool for switching wallpapers.
This GUI tool is designed to be desktop agnostic, allowing the user to choose how they switch their wallpaper.
SWPS uses a configuration file to define its look and the commands for execution.

## Code Dependencies
- [SFML](https://www.sfml-dev.org/) >= v3.0.0

# Usage
1. Create a config file using the info below.
2. Run the program and you should see your wallpapers loaded.
3. Move with VIM or Arrows Keys.
4. Press enter to execute wallpaper command.

## Config File
```
swps -c /path/to/config/file.conf
```
| VARIABLE    | DESCRIPTION | TYPE | DEFAULT |
|---------|-------------|------|---------|
| wallpaper_directory | The path to the wallpaper directory. | string | |
| window_size | Size of the window. | int, int | 800, 600 |
| exec | An array of commands to execute. Use {wp} to represent the wallpaper path. (put each command on new line if multiple) | string[] | [] |
| column_count | Number of columns to display. | int | 3 |
| inner_gaps | Gaps inbetween thumbnails. (vertical, horizontal) | int, int | 10 |
| outer_gaps | Gaps between thumbnails and the window edge. (vertical, horizontal) | int, int | 25 |
| outline_thickness | Size of wallpaper thumbnail borders on hover. | float | 2.0 |
| outline_color | Wallpaper thumbnail border color. | [int8, int8, int8] | [255, 255, 0] |
| background_color | Background color of window. | [int8, int8, int8] | [0, 0, 0] |

# Example Configs
### [Hyprland](https://hyprland.org/)
Launch via Hyprland configuration file: hyprland.conf
```
bind = $mainMod CONTROL, S, exec, swps -c ~/.config/hypr/swps.conf
```
swps.conf
```
wallpaper_directory = /home/usr/.config/hypr/bg/

outline_thickness = 2.5
outline_color = [255, 255, 0]
inner_gaps = 15
column_count = 4
exec = [
    hyprctl hyprpaper preload {wp}
    hyprctl hyprpaper wallpaper ,{wp}
]
background_color = [60, 90, 0]
```
------
### [Sway](https://swaywm.org/)
Launch via the sway configuration file: config
```
bindsym Shift+s exec swps -c ~/.config/sway/swps.conf
```
swps.conf
```
wallpaper_directory = /home/usr/.config/sway/bg/

exec = swaybg -i {wp} -m fill

window_size = 700, 350
outline_thickness = 4
outline_color = [0, 255, 255]
outer_gaps = 15, 50
```
