# Simple Wallpaper Swaper
This GUI tool is designed to be desktop agnostic allowing the user to choose which technology they use to swap wallpapers. Almost all desktops environments and especially window managers should have some form of cli tool for switching wallpapers with the wallpaper paths remaining constant

## Config File
```
swps -c /path/to/config/file.conf
```
| name    | description | type | default |
|---------|-------------|------|---------|
| wallpaper_directory | path to the wallpaper directory | string | ~/Pictures/ |
| window_size | size of the window |int, int | 800, 600 |
| border_size | size of wallpaper thumbnail borders on hover | int | 2 |
| border_color | wallpaper thumbnail border color | rgb(int8, int8, int8) | rgb(255, 255, 0) |
| inner_gaps | gaps inbetween thumbnails (supports css style conventions: top,bottom,left,right) | int | 10 |
| outer_gaps | gaps between thumbnails and the window edge (supports css style conventions: top,bottom,left,right) | int | 25 |
| column_count | number of columns to display | int | 3 |
| exec | an array of commands to execute. Use '$' to represent the wallpaper path. (put each command on a new line) | string[] | echo '<!> swps not configured' |

## Example Configs
------
### Hyprland
hyprland.conf
```
bind = $mainMod CONTROL, S, exec, swps -c ~/.config/hypr/swps.conf
```
swps.conf
```
wallpaper_directory = ~/.config/hypr/bg/

border_size = 2
border_color = rgb(255, 255, 0)
inner_gaps = 15
column_count = 4
exec = [
    hyprctl hyprpaper preload $
    hyprctl hyprpaper wallpaper ,$
]
```
------
### Sway
config
```
bindsym Shift+s exec swps -c ~/.config/sway/swps.conf
```
swps.conf
```
wallpaper_directory = ~/.config/sway/bg/

exec = swaybg -i /path/to/image.png -m fill

window_size = 700, 350
border_size = 4
border_color = rgb(0, 255, 255)
outer_gaps = 15, 50, 50, 50
```
