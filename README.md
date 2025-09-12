# Tetris Clone C++

A classic Tetris game implemented in C++ using the SFML (Simple and Fast Multimedia Library) for graphics, audio, and window management.

# THIS IS AN OPEN SOURCE CODE, FEEL FREE TO CLONE MY REPO AND EDIT IT YOUR OWN WAY, IF YOU WANT ME TO FEATURE IT ON MY GITHUB/A FUTURE DISCORD SERVER, PLEASE CONTACT ME ON DISCORD: @luxifer_ms03

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
  - Install on Linux: `sudo apt-get install libsfml-dev`, for arch based distros: `sudo pacman -S libsfml-dev` 
   - (or equivalent for your package manager).
- **C++ Compiler**: GCC or Clang with C++17 support.

## Building and Running

### Prerequisites
Ensure SFML is installed and your compiler supports C++17.

### Compilation
Compile the project using g++:

```bash
g++ main.cpp -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -o tetris
```

Alternatively, use the provided `compile.sh` script to compile and capture any errors:

```bash
./compile.sh
```

This will save compilation output to `compilererror.txt`.

If you want to compile the Hello World example instead of the full game:

```bash
g++ -DHELLO_WORLD main.cpp -o hello_world
./hello_world
```

### Error Analysis
If compilation fails, run the error parser to analyze errors and get suggestions:

```bash
python3 error_parser.py
```

This will append a description of errors and fix steps to `../TODO_tetris_fixes.txt` with checkboxes for tracking.

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

- `main.cpp`: Main game source code (includes Hello World example with -DHELLO_WORLD).
- `compile.sh`: Shell script to compile the game and save output to `compilererror.txt`.
- `error_parser.py`: Python script to analyze `compilererror.txt` and suggest fixes in `../TODO_tetris_fixes.txt`.
- `compilererror.txt`: Compilation output/errors.
- `gamedata.dat`: Saved game data (coins, wallpapers).
- `coins.dat`: Legacy coin data file.
- `tetris`: Compiled executable (if compilation succeeds).
- `hello_world`: Compiled Hello World executable (if compiled with -DHELLO_WORLD).
- `main_backup.cpp`: Backup of previous version.
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



<img width="600" height="625" alt="image" src="https://github.com/user-attachments/assets/663ab119-0c93-43b5-8c5a-869b71f7c6f0" />
<img width="608" height="629" alt="image" src="https://github.com/user-attachments/assets/6d04d0be-a220-4a9f-91bc-0e29efebd02a" />
<img width="593" height="617" alt="image" src="https://github.com/user-attachments/assets/796ffa37-a7bb-4b23-8282-a8291aa9a79f" />
<img width="591" height="628" alt="image" src="https://github.com/user-attachments/assets/d7655b03-6a65-401d-99a1-0453b5a2d59b" />
<img width="600" height="630" alt="image" src="https://github.com/user-attachments/assets/16a76a17-2b10-401c-86b2-2d7530b0f50d" />

