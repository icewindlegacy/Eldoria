# Eldoria MUD

A fantasy Multi-User Dungeon (MUD) server built in C++.

## Features

- Clean, modern C++ codebase
- Organized build system with automatic file detection
- Proper signal handling for graceful shutdowns
- Fixed pager system for long text output
- Comprehensive game mechanics including:
  - Character creation and progression
  - Combat system
  - Magic and skills
  - Guilds and religions
  - Object and room management
  - Online building tools (OLC)

## Building

```bash
cd src
make clean
make
```

The build system automatically detects all `.c` files and compiles them into the `o/` directory, keeping the source directory clean.

## Running

```bash
./shadow
```

## Project Structure

- `src/` - Source code directory
  - `o/` - Object files (generated during build)
  - `*.c` - C source files
  - `*.h` - Header files
  - `Makefile` - Build configuration
- `area/` - Game area files
- `data/` - Game data files
- `doc/` - Documentation
- `log/` - Server logs
- `note/` - Game notes and help files

## License

This project is based on the Merc/DikuMUD codebase.

## Contributing

Feel free to submit issues and enhancement requests!
