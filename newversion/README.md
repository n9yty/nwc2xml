# nwc2xml (wxWidgets-free version)

nwc2xml reads a NoteWorthy Composer file (`.nwc`) and writes a MusicXML 1.1
file (`.xml`). It supports NWC file versions 1.30 through 2.1, including
`[NWZ]` compressed files.

This directory is a self-contained port of nwc2xml v1.5 by James Lee
(juria90@yahoo.com) with the wxWidgets dependency removed. The converter
logic is unchanged C++; the wxWidgets classes it used (wxString, wxFile,
wxCSConv, zlib streams, command line parsing) are replaced by a small shim
(`wxshim.h` / `wxshim.cpp`) built on the C standard library, plus the system
zlib and iconv libraries.

nwc2xml is distributed under the terms of the GNU GPL v2 (see LICENSE.TXT).

## Building

Requirements: a C++11 compiler, zlib, and iconv.

- macOS: nothing to install; zlib and libiconv ship with the system.
- Linux: install the zlib development package (e.g. `zlib1g-dev` on
  Debian/Ubuntu, `zlib-devel` on Fedora); iconv is part of glibc.

```
make
```

This produces the `nwc2xml` binary in this directory. There is no
configure step and no other dependency.

## Usage

```
nwc2xml [-h] [-l] [--charset <charset>] <file.nwc>...
  -h, --help            show help
  -l, --log             also write a <file>.txt parse log next to the input
  --charset <charset>   charset used to decode strings in the .nwc file
```

For each `file.nwc` argument, the converter writes `file.xml` in the same
directory. Files whose names do not end in `.nwc` are skipped.

Strings inside `.nwc` files are decoded as follows: each string is tried
with the selected charset first (UTF-8 unless `--charset` is given), and if
it is not valid in that charset the converter falls back to CP1252, which
is the charset NWC itself uses. CP1252 is built in; any other `--charset`
name is passed to iconv, so anything iconv knows (`CP932`, `CP949`,
`ISO-8859-2`, ...) is accepted.

## Files

```
Makefile          build rules (make / make clean)
README.md         this file
LICENSE.TXT       GNU GPL v2
nwc2xml.cpp       main(): command line handling, drives the conversion
nwcfile.h/.cpp    CNWCFile: parses the .nwc container, inflates [NWZ] files
nwcobj.h/.cpp     CStaff/CObj hierarchy: clefs, notes, rests, bar lines, ...
XMLSaver.h,
xmlsaver.cpp      XMLSaver: walks the parsed staffs and emits MusicXML
XMLWriter.h,
xmlwriter.cpp     CXMLWriter: low level XML element/attribute writer
wxshim.h/.cpp     the wxWidgets replacement layer (see below)
Precompile.h      common includes, small Win32-style typedefs and macros
TVector.h         thin vector wrapper used throughout (std::vector based)
nwcendian.h       little endian read helpers
nwc2xml-version.h version and about text
```

## About the shim layer

`wxshim.h` / `wxshim.cpp` implement only the wxWidgets surface this program
actually uses, so the rest of the source could stay close to the original:

- `wxString`: UTF-8 text in a `std::string`; `wxChar` is `char` and `_T()`
  is a no-op, so `wxFprintf` is plain `fprintf`.
- `wxFile`: binary file I/O on stdio (`Open/Create/Read/Write/Seek/Tell`).
- `wxMBConv` / `wxCSConv` / `wxConvLocal`: charset decoding to UTF-8 with a
  built-in CP1252 table; other charsets go through iconv.
- `wxFileName` / `wxDateTime`: file modification date (for the MusicXML
  encoding-date) and temporary file creation (for inflating [NWZ] files).
- `wxZlibInputStream` has no shim; `UncompressNSave()` in nwcfile.cpp calls
  zlib's `inflate()` directly.

## Changes from the original source (beyond removing wxWidgets)

- Byte order detection in `nwcendian.h` now uses the compiler's
  `__BYTE_ORDER__` macro. The old header needed `-DMACOS` or `-DLINUX` and
  silently fell back to big endian otherwise, which made a plain
  `g++ *.cpp` build on macOS misread every file ("unknown version number").
- Command line parsing is hand-rolled (same options as before) instead of
  wxCmdLineParser.
- The unused `SaveStringNULTerminated()` was dropped.
- Small fixes to keep a clean, warning-free build: a missing `return` in
  `CRestCMObj::Dump`, an `=` vs `==` typo in the segno handling of
  `XMLSaver::SaveFlowDir`, a non-advancing loop in
  `CXMLWriter::WriteComment`, and const-correct string tables.
- Undefined CP1252 bytes (0x81, 0x8D, 0x8F, 0x90, 0x9D) decode to U+FFFD
  instead of failing the whole string.

The port was verified against the original wxWidgets build (compiled with
wxWidgets 3.3 and the endian define corrected): the `.xml` output and `-l`
parse logs are byte-identical for the three sample files shipped with the
project (NWC 1.75, [NWZ] compressed, with lyrics) and for a synthetic NWC
2.0 file exercising the uncompressed path and the CP1252 fallback.
