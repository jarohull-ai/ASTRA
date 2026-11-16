# ASTRA

Native Qt Quick system dashboard with translucent glass tiles, live Omarchy accent, Ice and Amber palettes. CPU, RAM, GPU (sysfs-compatible drivers), CPU temperature, root volume usage, and combined traffic on active network interfaces. No network requests, accounts or services.

Build: `sh bin/build`. Test: `sh bin/test`. Run: `build/astramon`.
Package: `cd pkgbuild && makepkg -f`; install the resulting package with pacman.

On this computer the app is installed per-user at `~/.local/bin/astramon`, with its launcher and icon in `~/.local/share`. The Arch package is also available in `pkgbuild/`. System-wide installation was not completed because authentication was pending.

Use Settings to add/remove tiles and adjust opacity/colour. Edit layout exposes move-left, move-right and remove buttons. Settings persist in `~/.config/astramon/astramon.conf`.

| Key | Action |
| --- | --- |
| E | Edit layout |
| Space | Pause/resume readings |
| ? | Help |
| Q | Quit |

History accumulates over 90 samples; it is intentionally empty on startup. Missing sensors show an unavailable state. GPU monitoring currently requires gpu_busy_percent (typically AMD); NVIDIA NVML and Intel engine counters are not implemented. Storage measures capacity, not I/O. Network aggregates active non-loopback interfaces and may count VPN/bridge traffic twice. Backdrop blur is supplied by the compositor, not simulated in the app.

Diagnostics: `astramon --probe` outputs real sensor readings as JSON without a window. `QT_QPA_PLATFORMTHEME=basic QT_QUICK_BACKEND=software astramon --screenshot /tmp/astra.png` renders the actual UI offscreen.
