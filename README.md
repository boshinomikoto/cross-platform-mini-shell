sv is a lightweight, cross-platform mini shell written in C.
It implements basic filesystem and console commands without using ```system()```, relying directly on OS APIs **(POSIX / WinAPI)**.

The project is designed for educational purposes and focuses on understanding how shell-like utilities work internally.

## Features
* Cross-platform: Windows / Unix-like systems
* No system() calls
* Direct work with the filesystem
* Simple and clear command architecture
* Minimal dynamic memory allocation


## Supported Commands
| Command | Description                      |
| ------- | -------------------------------- |
| `ls`    | Display directory contents       |
| `cl`    | Clear the console                |
| `dir`   | Create a directory               |
| `cwd`   | Show current working directory   |
| `cd`    | Change directory                 |
| `cd ..` | Go up one directory              |
| `cat`   | Display file contents            |
| `cf`    | Create file                      |
| `rm`    | Delete file                      |
| `bin`   | View binary file contents        |
| `s`     | Show file size                   |
| `tree`  | Recursively traverse directories |
| `exit`  | Exit the shell                   |
## GCC
```
gcc main.c -o main
./main
```
