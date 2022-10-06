# pitifful
Super simple C++ header-only TIFF reader with limited functionality.

The purpose of this library is to provide an extremely simple, quick-and-dirty TIFF reader for
various personal projects/experiments that do not merit inclusion of larger libraries.

The library is designed for ease of use, _not_ for speed or full compliance with
the TIFF specification. It was also my own way of learning the TIFF specification.
We highly recommend other libraries (e.g. `libtiff`) if you're doing anything other than playing around.

## Functionality
 - Reads individual frames from uncompressed TIFFs.
 - Reads a small subset of all possible TIFF fields.
 - Supports images in various bit depths (8-bit, 16-bit, 32-bit, 64-bit, and so on)

## What this does not do
 - Does not handle tile-oriented layout (only strip-oriented layout)
 - Does not handle BigTIFF (yet)
 - Does not handle compressed TIFFs
 - Does not recognize extra metadata present in specialized TIFFs (e.g. OME-TIFF)

## Example usage

See `example/example.cpp`.
