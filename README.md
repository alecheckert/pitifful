# pitifful
Super simple C++ header-only TIFF reader with limited functionality.

The purpose of this library is to provide an easy-to-use, quick-and-dirty TIFF reader for
projects/experiments that do not merit inclusion of larger libraries.

The library is designed for ease of use, _not_ for speed or full compliance with
the TIFF specification. It was also my own way of learning the TIFF specification.
We highly recommend other libraries (e.g. `libtiff`) if you're doing anything other than playing around.

## Functionality
 - Reads individual frames from uncompressed TIFFs
 - Reads a small subset of all possible TIFF tags
 - Supports images in various bit depths (8-bit, 16-bit, 32-bit, 64-bit, and so on)
 - Supports DEFLATE compression

## Nonfunctionality
 - Does not handle tile-oriented layout (only strip-oriented layout)
 - Does not handle BigTIFF (yet)
 - Does not handle other compression types (e.g. LZW)
 - Does not recognize extra metadata present in specialized TIFFs (e.g. OME-TIFF)
 - Cannot write TIFFs

## Dependencies

The only dependency is `libz`.

## Example usage

`example/example.cpp` provides an example of usage:
```
cd example
make
./example <PATH_TO_TIFF>
```

## Python bindings

`pitifful` also provides Python bindings, mostly to facilitate
comparison with `tifffile` (the gold-standard TIFF reading library
in Python).

To install the bindings, run `pip install -e .` from the
same directory as this README.

Example usage in Python:
```
from pitifful import TIFFReader

reader = TIFFReader(path_to_tif)

# Get the first image file directory
ifd = reader.get_ifd(0)
ifd.summary()

# Read the first frame
im = reader.read_frame_16bit(0)

# Read the entire image stack (if multi-frame)
stack = reader.read_stack_16bit()
```
