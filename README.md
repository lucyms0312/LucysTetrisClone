# Tetris Clone C++

A classic Tetris game implemented in C++ using the SFML (Simple and Fast Multimedia Library) for graphics, audio, and window management.

# THIS IS AN OPEN SOURCE CODE, FEEL FREE TO CLONE MY REPO AND EDIT IT YOUR OWN WAY, IF YOU WANT ME TO FEATURE IT ON MY GITHUB/A FUTURE DISCORD SERVER, PLEASE CONTACT ME ON DISCORD AT:
     <span style="color:red;">@luxifer_ms03.</span>

Although this script was prompted by me and slightly edited by me, this code heavily uses AI so some features might be unstable, please bear with me as I take the time to fix these issues

## Features

- **Classic Tetris Gameplay**: Standard Tetris mechanics with falling tetrominoes, line clearing, scoring, and levels.
- **Multiple Game States**: Main menu, options, mod menu, shop, and game over screens.
- **Customization Options**:
  - Adjustable brightness, volume, and rainbow speed.
  - Window wobble effect for a fun, jiggly window interaction.
- **Mods**:
  - Rainbow mode: Tetrominoes change colors dynamically.
- **Shop System**:
  - Purchase wallpapers (blue, green, red) and toggle space background.
  - Earn coins by placing blocks (every 5 blocks).
- **Audio**: Built-in Tetris theme music generated programmatically.
- **Save System**: Game data (coins, purchased items) is saved to `gamedata.dat`.
- **Responsive UI**: Menus and buttons for easy navigation.

## Dependencies

- **SFML 3.0**: Graphics, window, audio, and system libraries.
  - Install on Linux: `sudo apt-get install libsfml-dev` (or equivalent for your package manager).
- **C++ Compiler**: GCC or Clang with C++17 support.

## Building and Running

### Prerequisites
Ensure SFML is installed and your compiler supports C++17.

### Compilation
Compile the project using g++:

```bash
g++ main.cpp -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -o tetris
```

**Note**: There may be compilation errors in the current `main.cpp` file (see `compilingerror.txt`). These need to be resolved before successful compilation. The errors appear to be related to missing closing braces and function declarations.

### Running
After successful compilation:

```bash
./tetris
```

## Controls

- **Left Arrow**: Move piece left
- **Right Arrow**: Move piece right
- **Up Arrow**: Rotate piece
- **Down Arrow**: Soft drop (move down faster)
- **S**: Hard drop (instant drop)
- **R + Ctrl**: Reset game
- **Escape**: Return to main menu
- **Mouse**: Interact with menus, buttons, and sliders

## Game Mechanics

- **Scoring**: Earn points for clearing lines and soft dropping.
- **Levels**: Increase level every 10 lines cleared, speeding up piece fall.
- **Coins**: Earn 4 coins every 5 blocks placed. Use in shop for wallpapers.
- **Game Over**: When pieces reach the top of the board.

## File Structure

- `main.cpp`: Main game source code.
- `main_backup.cpp`: Backup of previous version.
- `compilingerror.txt`: Compilation errors (needs fixing).
- `gamedata.dat`: Saved game data (coins, wallpapers).
- `coins.dat`: Legacy coin data file.
- `tetris`: Compiled executable (if compilation succeeds).
- `TetrisThemeArduino.ino`: Arduino file for Tetris theme (not used in C++ version).

## Known Issues

- Compilation errors present in `main.cpp` (refer to `compilingerror.txt`).
- Font loading may fail if system fonts are not found; defaults to system font.
- Audio may not work if SFML audio module is not properly linked.

## Contributing

Feel free to fork the repository, fix issues, and submit pull requests. Ensure any changes maintain compatibility with SFML 3.0.

## License

This project is open-source. No specific license is defined; consider it under MIT License if redistributing.

## Credits

- Developed by Lucy (based on code comments).
- Tetris theme generated from Arduino notes.
- Uses SFML for multimedia functionality.
