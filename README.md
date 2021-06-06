# tekst
tekst is my *work-in-progress* implementation of a fast, minimal, cross-platform text editor.

It is mainly just for fun and learning, but ideally it will also become my default tool for quick edits (i.e. for anything not requiring an IDE or formatting).

---
## Planning

### Goals
- Fast (native)
- Minimal
- Modular/cross-platform (with separate UI and editor engine)

### Desired Features
- Movable text cursor (with persistent position memory)
- Select, insert, delete text
- Efficiently store text in memory
- Undo/redo
- Word wrapping
- Find and replace
- Copy/cut and paste

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
https://web.eecs.utk.edu/~azh/blog/challengingprojects.html
https://ecc-comp.blogspot.com/2015/05/a-brief-glance-at-how-5-text-editors.html
http://texteditors.org/cgi-bin/wiki.pl?DesigningTextEditors
