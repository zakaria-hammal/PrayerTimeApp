# Prayer Timer

A lightweight C program that fetches Islamic prayer times from the **Aladhan API** and sends **desktop notifications** when it’s time to pray.  
It is designed to run automatically in the background as a **systemd user service**, and works seamlessly on **KDE Plasma** via `libnotify`.

---

## 📂 Project Structure

```
.
├── main.c                # Main program (fetches prayer times, schedules notifications)
├── queue.c               # Queue implementation (used internally)
├── queue.h               # Header for queue.c
├── prayertimer.service   # systemd unit file for background execution
```

---

## ⚙️ Dependencies

You need the following packages:

- **OpenSSL** → HTTPS requests  
- **cJSON** → JSON parsing  
- **libnotify** → desktop notifications (integrates with KDE Plasma notifications)  
- **systemd** → service manager  

### On Arch Linux / Manjaro

```bash
sudo pacman -S base-devel openssl cjson libnotify systemd
```

### On Debian / Ubuntu

```bash
sudo apt install build-essential libssl-dev libcjson-dev libnotify-dev systemd
```

---

## 🔨 Building

Clone the repo and compile:

```bash
git clone https://github.com/yourusername/prayertimer.git
cd prayertimer

gcc main.c queue.c -o prayertimer \
  -lssl -lcrypto -lcjson -lnotify -Wall
```

This creates the executable `prayertimer`.

---

## ▶️ Running Manually

To test without installing as a service:

```bash
./prayertimer
```

You should see notifications appear in **KDE Plasma’s notification panel** when prayer times arrive.

---

## 🚀 Running as a Background Service

To have the program run automatically in the background:

1. Copy the binary to your local bin:

   ```bash
   mkdir -p ~/.local/bin
   cp prayertimer ~/.local/bin/
   ```

2. Copy the service file:

   ```bash
   mkdir -p ~/.config/systemd/user
   cp prayertimer.service ~/.config/systemd/user/
   ```

3. Reload `systemd` and enable the service:

   ```bash
   systemctl --user daemon-reload
   systemctl --user enable --now prayertimer.service
   ```

4. Check if it’s running:

   ```bash
   systemctl --user status prayertimer.service
   ```

If enabled, the service will automatically start at login and send notifications at prayer times.

---

## 🛠️ Configuration

- Currently, the **city and country** are hardcoded inside `main.c` (look at the API request).  
- To change location, edit the request URL in `main.c`, recompile, and restart the service.  
- Future improvements could add:
  - Config file support  
  - CLI arguments for location  
  - Logging  

---

## 🖥️ KDE Plasma Notes

- Notifications appear in **Plasma’s native notification tray**.  
- You can control how they behave in:  
  `System Settings → Notifications → Prayer Timer` (after first run).  
- Make sure `plasmashell` is running (default in KDE sessions).  

---

## 🙏 Credits

- [Aladhan API](https://aladhan.com/prayer-times-api) for prayer time data  
- GNOME [`libnotify`](https://gitlab.gnome.org/GNOME/libnotify) for notifications  
