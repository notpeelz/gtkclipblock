# gtkclipblock

A hack to prevent GTK programs from interacting with the primary clipboard (aka "primary selection"). Supports GTK 2/3/4.

This was made to work around [a Firefox bug](https://bugzilla.mozilla.org/show_bug.cgi?id=1791417), but may be useful for other purposes.

## How to use

### Method 1

1. add `/usr/lib/libgtkclipblock.so` to `/etc/ld.so.preload`
2. add `GTKCLIPBLOCK_HOOK=1` to `/etc/environment` or `~/.config/environment.d/gtkclipblock.conf`

This is the most effective method, but it's also the most invasive as it gets loaded by all processes.

It also has the benefit of working with setcap/setuid/setgid binaries.

### Method 2

Launch the program like this: `GTKCLIPBLOCK_HOOK=1 LD_PRELOAD=/usr/lib/libgtkclipblock.so /usr/bin/firefox`.

This is less invasive than the first method, however there's a few caveats:

- environment variables are inherited by child processes, which may unwittingly load the library
- doesn't work with setcap/setuid/setgid binaries

For convenience, you can patch a specific program by modifying its `.desktop` file:

1. copy the `.desktop` file from `/usr/share/applications` to `~/.local/share/applications`
2. modify the `Exec=` line
   - before: `Exec=/usr/bin/firefox %u`
   - after: `Exec=/usr/bin/env GTKCLIPBLOCK_HOOK=1 LD_PRELOAD=/usr/lib/libgtkclipblock.so /usr/bin/firefox %u`

For CLI use, you can create a bootstrap script:

Save this as `~/.local/bin/firefox` (make sure to `chmod +x` it!):

```sh
#!/usr/bin/env sh

export LD_PRELOAD="/usr/lib/libgtkclipblock.so${LD_PRELOAD:+:$LD_PRELOAD}"
exec /usr/bin/firefox "$@"
```

## Install from package

Available on the [AUR](https://aur.archlinux.org/packages/gtkclipblock).

## Install from source

```sh
meson setup --prefix=/usr/local build
meson install -C build
```

## Environment variables

| env var                   | description                                                   | value                                                                                               |
| ------------------------- | ------------------------------------------------------------- | --------------------------------------------------------------------------------------------------- |
| `GTKCLIPBLOCK_HOOK`       | determines which GTK libraries should be hooked               | `0` (disables all; **default**), `1` (enables all); or a comma-separated list, e.g `gtk2,gtk3,gtk4` |
| `GTKCLIPBLOCK_HOOK_DLFCN` | if disabled, libraries loaded via `dlopen()` won't get hooked | `0` (disabled), `1` (enabled; **default**)                                                          |
