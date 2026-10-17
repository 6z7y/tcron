`tcron` is a tiny cron-style job scheduler for Linux.

## Requirements

- Linux (inotify is built into the kernel)
- `gcc`, `make`, `git`

Install:

    # Arch
    sudo pacman -S base-devel git

    # Ubuntu / Debian
    sudo apt install build-essential git

    # Void
    sudo xbps-install -S base-devel git

## Build

Build the project from source

```sh
git clone https://github.com/6z7y/tcron && cd tcron
sudo make install
```

To remove the binary:

```sh
sudo make uninstall
```

## Configuration

Config file (created on first run):

    ~/.local/share/tcron/.tcron

Edit it with `tcron -e`. Changes reload automatically.

### Interval

Run every N seconds, minutes, hours, days or weeks (`s`, `m`, `h`, `d`, `w`).

    <number><unit> <command>

Example:

    10s echo hello
    5m date

### Time

Run every day at a fixed time (24-hour).

    HH:MM <command>

Example:

    08:30 echo Good morning

## Usage

```sh
tcron
```

Available command-line options:

- `-e` — Open the configuration in the editor specified by `$EDITOR`.
- `-h`, `--help` — Show help.
- `-v`, `--version` — Show version information.

Changes are detected while `tcron` is running, and the configuration is reloaded.
