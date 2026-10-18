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

    /etc/tcron/tcron

Edit it with `tcron -e`. Changes reload automatically.

#### Interval Type

Run commands after a specified interval using seconds, minutes, hours, days, or weeks (s, m, h, d, w).

    <number><unit>[,<number><unit>...] <command>

Examples:

    10s echo hello
    5m date
    1h,33m echo "hello after 1h 33m"
    2h,13m,33s echo "hello after 2h 13m 33s"
    33s,13m,2h echo "same duration, different order"

The order of the time units does not matter.

#### Time Type

Run every day at a fixed time (24-hour).

    HH:MM <command>

Example:

    08:30 echo Good morning


## Logs

tcron writes job executions and errors:

    /var/log/tcron/access.log   (jobs that were started)
    /var/log/tcron/errors.log   (errors)

The logs are useful for checking executed jobs and errors.

## Recovery Files [logs|config]

If the config file or a log file is deleted or moved, tcron recreates it automatically.


## Usage

tcron must be run as root:

```bash
sudo tcron
```

Available command-line options:

- `-e` — Open the configuration in the editor specified by `$EDITOR`.
- `-h`, `--help` — Show help.
- `-v`, `--version` — Show version information.

## Services

`make install` detects your init system (systemd or runit) and installs the matching service file. It does not enable it, so run the command for your init:

### runit

    sudo ln -s /etc/sv/tcron /var/service/

### systemd

    sudo systemctl enable --now tcron
