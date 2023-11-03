# gtkclipblock

A hack to prevent GTK programs from interacting with the primary clipboard (aka "primary selection"). Supports GTK 2/3/4.

This was made to work around [a Firefox bug](https://bugzilla.mozilla.org/show_bug.cgi?id=1791417), but may be useful for other purposes.

## How to use

### Method 1 (global)

Add `/usr/lib/libgtkclipblock.so` to `/etc/ld.so.preload`.

This is the most effective method, but it's also the most invasive as it gets loaded by all processes.

It also has the benefit of working with setcap/setuid/setgid binaries.

### Method 2 (specific programs)

Launch the program like this: `LD_PRELOAD=/usr/lib/libgtkclipblock.so /usr/bin/firefox`.

This is less invasive than the first method, however there's a few caveats:

- environment variables are inherited by child processes, which may unwittingly load the library
- doesn't work with setcap/setuid/setgid binaries

For convenience, you can patch a specific program by modifying its `.desktop` file:

1. copy the `.desktop` file from `/usr/share/applications` to `~/.local/share/applications`
2. modify the `Exec=` line
   - before: `Exec=/usr/bin/firefox %u`
   - after: `Exec=/usr/bin/env LD_PRELOAD=/usr/lib/libgtkclipblock.so /usr/bin/firefox %u`

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
