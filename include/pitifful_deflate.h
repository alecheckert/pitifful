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

        /* allocate inflate state */
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        strm.avail_in = 0;
        strm.next_in = Z_NULL;
        ret = inflateInit(&strm);
        if(ret!=Z_OK){
            return ret;
        }

        /* decompress until deflate streams ends or eof */
        do {
            int chunk_size = CHUNK < to_read ? CHUNK : to_read;
            source.read(inptr, chunk_size);
            strm.avail_in = chunk_size;
            if(!source){
                (void)inflateEnd(&strm);
                return Z_ERRNO;
            }
            if(strm.avail_in==0){
                break;
            }
            strm.next_in = inbuffer;
            /* run inflate() on input until output buffer not full */
            do {
                strm.avail_out = CHUNK;
                strm.next_out = outbuffer;
                ret = inflate(&strm, Z_NO_FLUSH);
                assert(ret!=Z_STREAM_ERROR);
                switch(ret){
                    case Z_NEED_DICT:
                        ret = Z_DATA_ERROR;
                    case Z_DATA_ERROR:
                    case Z_MEM_ERROR:
                        (void)inflateEnd(&strm);
                        return ret;
                }
                have = CHUNK - strm.avail_out;
                if(max_out_buf_size<written+have){
                    throw std::runtime_error(
                        std::string("cannot write additional ")
                        + std::to_string(have)
                        + std::string(" bytes to output buffer with size ")
                        + std::to_string(max_out_buf_size)
                        + std::string(" bytes, only ")
                        + std::to_string(max_out_buf_size-written)
                        + std::string(" bytes remaining")
                    );
                }
                std::memcpy(out, outbuffer, have);
                out += have;
                written += have;
            } while (strm.avail_out == 0);
            to_read -= chunk_size;
        } while (ret != Z_STREAM_END);
        (void)inflateEnd(&strm);
        return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
    }
};

} // end namespace pitifful

#endif
