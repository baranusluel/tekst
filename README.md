# tekst
tekst is my *work-in-progress* implementation of a fast, minimal, cross-platform text editor.

It is mostly just for fun and learning, but ideally it should eventually become my default tool for quick edits (i.e. anything not requiring an IDE or formatting).

## Building
tekst aims to be cross-platform, and is actively tested on Windows 10 and Ubuntu Linux (WSL).

For text-based UI functionality in the terminal, the Unix build uses ncurses and the Windows build uses PDCurses (header include path and library linkage must be setup for local environment).

The source can be built with g++ (see .vscode/tasks.json for build arguments). On Windows, the author uses the Mingw-w64 distribution of the compiler.

The project is developed in VSCode with the official "C/C++" and "Remote - WSL" extensions.

---
## Planning

### Goals
- Fast (native)
- Minimal
- Modular & cross-platform (with separate UI and editor engine)

### Desired Features
- Movable text cursor (with persistent position memory)
- Select, insert, delete text
- Efficiently store text in memory
- Undo/redo
- Find and replace
- Copy/cut and paste
- Vertical and horizontal scroll
- Extended character support
- Very large file support

### Data structures for text buffer
#### Want:
- fast operations.
- ability to view & edit very large files (GBs).
  - Note: In preliminary tests, VSCode and Notepad++ were a little slow, Vim very slow in opening a 24MB file.
- reasonable memory performance.
#### Options:
- Array (contiguous char buffer)
- Array / vector (DLL) of lines
- Rope
- Gap buffer
- Piece table
#### Strategy:
Implement various solutions and profile memory & performance to compare.

### References:
- https://web.eecs.utk.edu/~azh/blog/challengingprojects.html
- https://ecc-comp.blogspot.com/2015/05/a-brief-glance-at-how-5-text-editors.html
- http://texteditors.org/cgi-bin/wiki.pl?DesigningTextEditors
- http://www.catch22.net/tuts/neatpad/neatpad-overview