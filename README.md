# Rainbow Six Siege Server Switcher

A utility for quickly changing your Rainbow Six Siege matchmaking server. Includes both a Windows GUI (C) and a command-line Python script.

## Features
- **List and select available matchmaking servers**
- **Automatically detects all Rainbow Six Siege config files**
- **Easily switch servers via GUI or CLI**
- **Supports all official server regions**

## Components
- `siegexSwitcher.exe` / `siegexSwitcher.c`: Modern Windows GUI for server switching (C, Win32 API)
- `siegexSwitcher.py`: Command-line tool for server switching (Python 3)
- `Makefile`: Build instructions for the C GUI
- `appicon.ico`, `appicon.rc`, `appicon.res`: Application icon resources

## Download
You can download the latest release binaries from the [Releases](https://github.com/0xsweat/r6x-switcher/releases) page.
Alternatively, you can clone the repository using Git:

```bash
git clone https://github.com/0xsweat/r6x-switcher
```

## Usage

### Python Script
1. **Requirements:** Python 3.x
2. **Run:**
   ```powershell
   python siegexSwitcher.py [options]
   ```
3. **Options:**
   - `--servers` : List available servers
   - `--configs` : List found config files and their current server
   - `--server <name>` : Set the matchmaking server (e.g. `--server westeurope`)
   - `--data-dir <path>` : Specify custom config directory

### Windows GUI
1. **Build:**
   - Requires GCC (MinGW) and windres
   - Run:
     ```powershell
     mingw32-make
     ```
2. **Run:**
   - Double-click `siegexSwitcher.exe`

## Notes
- The tool automatically finds all user config files for Rainbow Six Siege X
- Always restart the game after switching servers for your change to apply.
- You **DON'T** have to switch servers using this if you are just doing a custom game; this is only needed if you want to switch your matchmaking server.

## License
MIT License

---
by 0xsweat, 2025
