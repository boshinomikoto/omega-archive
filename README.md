# OmegaArchive

A terminal-based student database viewer built with C++ and FTXUI.

## Features

- Login / registration screen
- Browse students by category (degree, course, year)
- Search by name, surname, or group
- All data stored locally in SQLite

## Requirements

- CMake 3.11+
- C++17 compiler (GCC / MinGW / Clang)
- Internet connection on first build (CMake downloads FTXUI and SQLite automatically)

## Build

```bash
git clone https://github.com/your-username/omega-archive.git
cd omega-archive
cmake -S . -B build
cmake --build build
```

Then run the executable from the `build/` folder:

```bash
./build/POSINAPI       # Linux / macOS
build\POSINAPI.exe     # Windows
```

> On first run the database is created and filled with 20 test students automatically.

## Usage

1. **Register** a new account or **log in** with an existing one
2. Use the **menu** on the left to browse students by category
3. Use the **search fields** (Name / Surname / Group) and click **Search** to filter
4. Press **Esc** to exit

## Project Structure

```
omega-archive/
├── main.cpp          — UI logic (FTXUI)
├── CMakeLists.txt
└── mylib/
    ├── omegadb.h/c   — student database (SQLite)
    ├── auth.h/c      — login & registration
    └── CMakeLists.txt
```

## Stack

| Tool | Purpose |
|------|---------|
| [FTXUI](https://github.com/ArthurSonzogni/ftxui) | Terminal UI framework |
| [SQLite](https://www.sqlite.org) | Embedded database |
| CMake FetchContent | Automatic dependency management |
