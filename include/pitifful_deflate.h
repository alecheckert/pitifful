/* zlib routines for DEFLATE-compressed TIFFs */
#ifndef _PITIFFUL_DEFLATE_H
#define _PITIFFUL_DEFLATE_H

#include <iostream>
#include <fstream>
#include <cassert>
#include <cstring>
#include "zlib.h"

// MSDOS compatibility
#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) _setmode(_fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

// #define CHUNK 16384
// #define CHUNK 32768
// #define CHUNK 65536
// #define CHUNK 131072
#define CHUNK 524288
// #define CHUNK 1048576

namespace pitifful {

/* interpret and report exit values of zlib ops */
void zerr(int ret)
{
    switch(ret){
        case Z_ERRNO:
            std::cerr << "zlib error: stream handling error\n";
            break;
        case Z_STREAM_ERROR:
            std::cerr << "zlib error: invalid compression level\n";
            break;
        case Z_DATA_ERROR:
            std::cerr << "zlib error: invalid or incomplete deflate data\n";
            break;
        case Z_MEM_ERROR:
            std::cerr << "zlib error: out of memory\n";
            break;
        case Z_VERSION_ERROR:
            std::cerr << "zlib error: zlib version mismatch!\n";
    }
}

class DEFLATEDecompressor{
    unsigned char inbuffer[CHUNK];
    unsigned char outbuffer[CHUNK];
public:
    DEFLATEDecompressor(){}
    ~DEFLATEDecompressor(){}
    int decompress(
        std::ifstream& source,
        char* out,
        int to_read,
        unsigned& written,
        const unsigned max_out_buf_size
    ){
        int ret;
        unsigned have;
        written = 0;
        z_stream strm;
        if(!source.is_open()){
            throw std::runtime_error("input stream not open");
        }
        char* inptr = reinterpret_cast<char*>(&inbuffer[0]);
        source.read(inptr, to_read);

        /* allocate inflate state */
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        strm.avail_in = 0;
        strm.avail_in = static_cast<unsigned>(to_read);
        strm.next_in = inbuffer;
        strm.avail_out = max_out_buf_size;
        strm.next_out = reinterpret_cast<unsigned char*>(out);

        // Initialize stream
        ret = inflateInit(&strm);
        if(ret!=Z_OK){
            std::cerr << "error with inflateInit: " << ret << "\n";
            zerr(ret);
            return ret;
        }

        // Decompress
        ret = inflate(&strm, Z_NO_FLUSH);
        if(ret!=Z_STREAM_END){
            std::cerr << "error with inflate: " << ret << "\n";
            if(ret==Z_OK){
                std::cerr << "zlib: stream okay, but incomplete decompression\n";
            } else{
                zerr(ret);
            }
            return ret;
        }

        // Clean up
        ret = inflateEnd(&strm);
        if(ret!=Z_OK){
            std::cerr << "error with inflateEnd: " << ret << "\n";
            zerr(ret);
            return ret;
        }

        // Get number of bytes written
        written = static_cast<unsigned>(strm.total_out);
        return ret;
    }
};

} // end namespace pitifful

#endif
