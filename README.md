# Synvixa

A terminal-based text editor with team-based code editing.

## Features
 - Collaboration – edit files with other people over the internet
 - Syntax highlighting – colours code to make it easier to read and understand
 - Cross-platform – supports Linux, macOS and Windows

## How to use
Open with `synvixa <file_name>` to run a server, or no arguments to open a file explorer menu, close using Ctrl+C (I know it feels wrong, but it does save). To connect as a client use `synvixa --ip <ip_address>`.

## Building
Clone this git repository and run `mkdir build && cd build && cmake .. && cmake --build .`. This should build the executable and all plugins in the `plugins` folder.

## Supported languages
You can use Synvixa with any text-based language. However, only the following languages currently have support for advanced features:
 - C
 - C++
 - CMake
 - Python
