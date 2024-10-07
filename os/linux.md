# Linux

## Force to run in X instead of Wayland
`env -u WAYLAND_DISPLAY ./soil`

## Sync clocks on dual boot between Windows and Linux
`timedatectl set-local-rtc 1 --adjust-system-clock`
