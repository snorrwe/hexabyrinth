# Hexabyrinth

## Description

Hex tile based labyrinth generator.

![exmaple0](docs/example0.png)
![exmaple1](docs/example1.png)

## The algorithm

```
Map generation:
    Set each cell of the world to on or off
    Update the world 10 times

World update:
    For each cell:
        Count the neighbours of the cell that are on
        If the cell is on and the number of neighbours is less than 1 or more than 2:
            Set the cell to off
        Else if number of neighbours is 2:
            Set the cell to on
```

## Requirements

- C++17 compatible compiler
- CMake
- Windows (`Minomaly` which I used for rendering only supports the Windows platform at the moment (sorry))

## Setting up

```
git clone https://github.com/snorrwe/hexabyrinth
cd hexabyrinth
git submodule init
git submodule update -r
cmake -H. -Bbuild
```

## Building

```
cmake --build build
```

## Running

Run the `hexabyrinth` executable in the `build` directory. Press `space` to generate a new labyrinth or `q` to quit.

## TODO

- Parametrize map generation
- Save/Load map files
- GUI for parameter changing on the fly

