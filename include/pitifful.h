#ifndef _PITIFFUL_H
#define _PITIFFUL_H

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>
#include <algorithm>
// #include "portable_endian.h"

namespace pitifful {

/* Sizes of each TIFF field type in bytes */
const uint16_t TIFF_FIELD_TYPE_SIZES[12] = {
    1,   // type 1 (BYTE): 8-bit unsigned integer
    1,   // type 2 (ASCII): 8-bit byte with a 7-bit ASCII code in the first 7 bits
    2,   // type 3 (SHORT): 16-bit unsigned integer
    4,   // type 4 (LONG): 32-bit unsigned integer
    8,   // type 5 (RATIONAL): rational expressed as two 32-bit unsigned integers
    1,   // type 6 (SBYTE): 8-bit signed (twos-complement) integer
    1,   // type 7 (UNDEFINED): 8-bit undefined
    2,   // type 8 (SSHORT): 16-bit signed (twos-complement) integer
    4,   // type 9 (SLONG): 32-bit signed (twos-complement) integer
    8,   // type 10 (SRATIONAL): rational expressed as two SLONGs
    4,   // type 11 (FLOAT): IEEE single-precision floating point
    8    // type 12 (DOUBLE): IEEE double-precision floating point
};

/*
 *  Function: determine_if_host_is_little_endian
 *  --------------------------------------------
 *  Returns true if this architecture is little-endian, false otherwise.
*/
inline bool determine_if_host_is_little_endian(){
    int n = 1;
    return *(char*)&n == 1;
}

/*
 *  struct: IFD
 *  -----------
 *  Represents a single image file directory from a TIFF6 file.
 *  tiffr only knows to look for specific fields. Generally, integer-valued
 *  fields are represented as the *int* type, and have value -1 if 
 *  not explicitly set.
*/
struct IFD {
    // Byte offset of this IFD relative to BOF
    uint64_t byte_offset = 0;

    // Byte offset of the next IFD relative to BOF, or 0 if this
    // is the last IFD in the file.
    uint64_t next_byte_offset = 0;

    // Total number of fields in this IFD.
    uint16_t count = 0;

    // Location and size of each strip.
    std::vector<uint64_t> strip_offsets, // 273
                          strip_byte_counts; // 279

    // Image metadata (special TIFF fields).
    int width = -1, // 256
        height = -1, // 257
        bits_per_sample = -1, // 258
        compression = -1, // 259
        photometric_interpretation = -1, // 262
        samples_per_pixel = -1, // 277
        rows_per_strip = -1; // 278
};


/*
 *  Function: is_local_value
 *  ------------------------
 *  Return true if a TIFF field has a local value, and false if
 *  its value is located elsewhere in the file. The rule is that
 *  values are local if and only if they fit into 4 bytes
 *  (TIFF6 specification, page 15).
*/
inline bool is_local_value(uint16_t field_type, uint16_t count){
    if((field_type<0) || (field_type>12)){
        return false;
    }
    uint16_t field_size = TIFF_FIELD_TYPE_SIZES[field_type-1] * count;
    return field_size <= 4;
}


/*
 *  Function: is_int_value
 *  ----------------------
 *  Return true if a TIFF field has a type castable to uint.
*/
inline bool is_uint_value(uint16_t field_type){
    return (
        (field_type==1) ||
        (field_type==2) ||
        (field_type==3) ||
        (field_type==4)
    );
}


/*
 *  Function: parse_uint_field
 *  --------------------------
 *  Parse a single uint value from raw bytes.
 *
 *  Parameters
 *  ----------
 *    field_type    :   1, 2, 3, or 4, how to interpret this field
 *    c             :   pointer to raw bytes
 *
 *  Returns
 *  -------
 *    cast value
*/
template <typename T>
inline T parse_uint_field(uint16_t field_type, char* c){
    T x;
    switch(field_type){
        case 1:
            x = static_cast<T>(*reinterpret_cast<uint8_t*>(c));
            break;
        case 2:
            x = static_cast<T>(*reinterpret_cast<char*>(c));
            break;
        case 3:
            x = static_cast<T>(*reinterpret_cast<uint16_t*>(c));
            break;
        case 4:
            x = static_cast<T>(*reinterpret_cast<uint32_t*>(c));
            break;
        default:
            std::string err("cannot interpret TIFF tag type as uint: ");
            err += std::to_string(field_type);
            throw std::runtime_error(err);
    }
    return x;
}
template uint32_t parse_uint_field<uint32_t>(uint16_t field_type, char* c);
template uint64_t parse_uint_field<uint64_t>(uint16_t field_type, char* c);


/*
 *  Function: parse_int_field
 *  -------------------------
 *  Parse a single int-like value from raw bytes.
 *
 *  Parameters
 *  ----------
 *    field_type    :   1, 2, 3, 4, 6, 8, or 9, how to interpret this field
 *    c             :   pointer to raw bytes
 *
 *  Returns
 *  -------
 *    cast value
*/
template <typename T>
inline T parse_int_field(uint16_t field_type, char* c){
    T x;
    switch(field_type){
        case 1:
            x = static_cast<T>(*reinterpret_cast<uint8_t*>(c));
            break;
        case 2:
            x = static_cast<T>(*reinterpret_cast<char*>(c));
            break;
        case 3:
            x = static_cast<T>(*reinterpret_cast<uint16_t*>(c));
            break;
        case 4:
            x = static_cast<T>(*reinterpret_cast<uint32_t*>(c));
            break;
        case 6:
            x = static_cast<T>(*reinterpret_cast<int8_t*>(c));
            break;
        case 8:
            x = static_cast<T>(*reinterpret_cast<int16_t*>(c));
            break;
        case 9:
            x = static_cast<T>(*reinterpret_cast<int32_t*>(c));
            break;
        default:
            std::string err("cannot interpret TIFF tag type as uint: ");
            err += std::to_string(field_type);
            throw std::runtime_error(err);
    }
    return x;
}
template int parse_int_field<int>(uint16_t field_type, char *c);
template int64_t parse_int_field<int64_t>(uint16_t field_type, char* c);


/*
 *  Function: _parse_array
 *  ----------------------
 *  Parse multiple values of the same type from raw bytes into an output
 *  array.
 *
 *  Assumptions
 *  -----------
 *    - the values are dense in the binary filestream (no spaces)
 *    - the values all have the same type
 *    - Tout is static-castable to Tin
 *
 *  Parameters
 *  ----------
 *    count     :   number of elements in the array
 *    host_be   :   true if the host is big-endian, false otherwise
 *    file_be   :   true if the TIFF file is big-endian, false otherwise
 *    in        :   pointer to raw bytes
 *    out       :   pointer to allocated output array (size *count*)
*/

template <typename Tin, typename Tout>
inline void _parse_array(
    uint32_t count,
    bool host_be,
    bool file_be,
    char* in,
    Tout* out
){
    Tin* ptr = reinterpret_cast<Tin*>(in);
    if(host_be==file_be){
        for(uint32_t i=0; i<count; ++i){
            out[i] = static_cast<Tout>(ptr[i]);
        }
    } else if((host_be) && (!file_be)){
        if(std::is_same<Tin, uint16_t>::value){
            for(uint32_t i=0; i<count; ++i){
                out[i] = static_cast<Tout>(le16toh(ptr[i]));
            }
        } else if(std::is_same<Tin, uint32_t>::value){
            for(uint32_t i=0; i<count; ++i){
                out[i] = static_cast<Tout>(le32toh(ptr[i]));
            }
        } else if(std::is_same<Tin, uint64_t>::value){
            for(uint32_t i=0; i<count; ++i){
                out[i] = static_cast<Tout>(le64toh(ptr[i]));
            }
        } else{
            throw std::runtime_error("can only parse uint16, uint32, or uint64 from LE to BE");
        }
    } else if((!host_be) && (file_be)){
        ;
    }
}

/*
 *  Function: parse_array
 *  ---------------------
 *  Same as _parse_array, but also figure out the type based on a TIFF field
 *  type code (see TIFF_FIELD_TYPE_SIZES above).
*/
template <typename T>
inline void parse_array(
    uint16_t field_type,
    uint32_t count,
    bool host_be,
    bool file_be,
    char* in,
    T* out
){
    switch(field_type){
        case 1:
            _parse_array<uint8_t, T>(count, host_be, file_be, in, out);
            break;
        case 3:
            _parse_array<uint16_t, T>(count, host_be, file_be, in, out);
            break;
        case 4:
            _parse_array<uint32_t, T>(count, host_be, file_be, in, out);
            break;
        case 7:
            _parse_array<int8_t, T>(count, host_be, file_be, in, out);
            break;
        case 8:
            _parse_array<int16_t, T>(count, host_be, file_be, in, out);
            break;
        case 9:
            _parse_array<int32_t, T>(count, host_be, file_be, in, out);
            break;
        case 11:
            _parse_array<float, T>(count, host_be, file_be, in, out);
            break;
        case 12:
            _parse_array<double, T>(count, host_be, file_be, in, out);
            break;
        default:
            throw std::runtime_error(
                std::string("unrecognized field_type ") + std::to_string(field_type)
            );
    }
}


/*
 *  Class: TIFFReader
 *  -----------------
 *  Basic TIFF reading class.
*/
class TIFFReader {
private:
    // filestream
    std::ifstream s;

    // endian-ness of host, file
    bool host_is_big_endian, file_is_big_endian;

    // buffer for parsing IFDs
    char ifd_parse_buffer[10000];

    // Image file directories
    std::vector<IFD> ifds;

    // Total number of frames in this TIFF file
    uint64_t n_frames;

    // Size of the largest strip in the file (in bytes)
    uint64_t max_strip_size;

    // Buffers for image file reading
    std::unique_ptr<char[]> ustrip_buffer;
    char* strip_buffer;

public:
    TIFFReader(const char* path):
        host_is_big_endian(false),
        file_is_big_endian(false),
        n_frames(0),
        max_strip_size(0),
        ustrip_buffer(nullptr),
        strip_buffer(nullptr)
    {
        s.open(path, std::ios::in | std::ios::binary);
        if(!s.is_open()){
            throw std::runtime_error(
                std::string("failed to open ") + std::string(path)
            );
        }
        host_is_big_endian = !determine_if_host_is_little_endian();

        // File reading buffer
        char* c = &ifd_parse_buffer[0];

        // TIFF header
        s.read(c, 8);

        // First 2 bytes encode endianness of file
        if((c[0]=='\x49') && (c[1]=='\x49')){
            file_is_big_endian=false;
        } 

        // Current do not support mismatches between endian-ness of
        // file and host
        if(host_is_big_endian!=file_is_big_endian){
            throw std::runtime_error(
                "currently do not support host/file endian-ness mismatches"
            );
        }

        // Next 2 bytes always encode the number 42
        if(file_is_big_endian){
            if((c[2]!='\x00') || (c[3]!='\x2A')){
                throw std::runtime_error("TIFF magic missing; not a TIFF file");
            }
        } else{
            if((c[2]!='\x2A') || (c[3]!='\x00')){
                throw std::runtime_error("TIFF magic missing; not a TIFF file");
            }
        }

        // Bytes 4, 5, 6, and 7 encode the byte offset of the first IFD from BOF
        uint64_t ifd_offset = static_cast<uint64_t>(*reinterpret_cast<uint32_t*>(c+4));
        while(ifd_offset>0){
            IFD ifd = parse_ifd(ifd_offset);
            ifd_offset = ifd.next_byte_offset;
            ifds.push_back(ifd);
        }

        // Total number of IFDs
        n_frames = static_cast<uint64_t>(ifds.size());

        // Size of the largest strip in the entire file (in bytes)
        uint64_t strip_size;
        for(uint64_t frame=0; frame<n_frames; ++frame){
            const IFD& ifd = ifds[frame];
            strip_size = *std::max_element(ifd.strip_byte_counts.begin(), ifd.strip_byte_counts.end());
            if(strip_size>max_strip_size){
                max_strip_size = strip_size;
            }
        }

        // Allocate a buffer for reading strips
        ustrip_buffer = std::make_unique<char[]>(max_strip_size);
        strip_buffer = &ustrip_buffer[0];
    }

    /* Getters */
    const IFD& get_ifd(int frame) const{return ifds[frame];}
    uint64_t get_n_frames() const{return n_frames;}
    uint64_t get_max_strip_size() const{return max_strip_size;}


    /*
     *  Method: get_n_samples
     *  ---------------------
     *  Return the total number of samples in a single frame.
    */
    int get_n_samples(int frame) const{
        const IFD& ifd = ifds[frame];
        return ifd.height * ifd.width * ifd.samples_per_pixel;
    }


    /*
     *  Method: read_frame
     *  ------------------
     *  Read a single frame into memory.
     *
     *  Parameters
     *  ----------
     *    T     :   type of the destination array
     *    frame :   index of the target frame (from 0 to n_frames-1)
     *    out   :   allocated array of size *get_n_samples(frame)*
    */
    template <typename T>
    void read_frame(int frame, T* out){
        const IFD& ifd = ifds[frame];

        // Total number of strips to read
        int n_strips = ifd.strip_offsets.size();

        // Uncompressed
        for(int strip=0; strip<n_strips; ++strip){
            s.seekg(ifd.strip_offsets[strip], s.beg);
            s.read(strip_buffer, ifd.strip_byte_counts[strip]);
            if(ifd.bits_per_sample==8){
                _parse_array<uint8_t, T>(
                    ifd.strip_byte_counts[strip] * 8 / ifd.bits_per_sample,
                    host_is_big_endian,
                    file_is_big_endian,
                    strip_buffer,
                    out
                );
            } else if(ifd.bits_per_sample==16){
                _parse_array<uint16_t, T>(
                    ifd.strip_byte_counts[strip] * 8 / ifd.bits_per_sample,
                    host_is_big_endian,
                    file_is_big_endian,
                    strip_buffer,
                    out
                );
            } else if(ifd.bits_per_sample==32){
                 _parse_array<uint32_t, T>(
                    ifd.strip_byte_counts[strip] * 8 / ifd.bits_per_sample,
                    host_is_big_endian,
                    file_is_big_endian,
                    strip_buffer,
                    out
                );
            } else if(ifd.bits_per_sample==64){
                _parse_array<double, T>(
                    ifd.strip_byte_counts[strip] * 8 / ifd.bits_per_sample,
                    host_is_big_endian,
                    file_is_big_endian,
                    strip_buffer,
                    out                   
                );
            }

        }
    }


    /*
     *  Method: parse_ifd
     *  -----------------
     *  Parse an image file directory. Extracts a subset of known fields (including all fields
     *  required for bilevel, grayscale, palette, and RGB images in the TIFF6 spec) and assigns
     *  as attributes of an IFD struct.
     *
     *  Parameters
     *  ----------
     *    byte_offset   :   location of the start of the IFD relative to BOF in bytes
     *
     *  Returns
     *  -------
     *    IFD, image file directory metadata
    */
    IFD parse_ifd(uint64_t byte_offset){
        IFD ifd;
        ifd.byte_offset = byte_offset;
        char* c = &ifd_parse_buffer[0];

        // Go to the start of the IFD
        s.seekg(byte_offset, s.beg);

        // First 2 bytes encode the count (number of fields)
        s.read(c, 2);
        ifd.count = *reinterpret_cast<uint16_t*>(c);

        // Read the field array
        s.read(c, 12*ifd.count+4);
        uint16_t ftag, ftype;
        uint32_t fcount, fsize;

        // Parse each field in the field array
        for(uint16_t i=0; i<ifd.count; ++i){
            ftag = parse_int_field<uint16_t>(3, c+12*i);
            ftype = parse_int_field<uint16_t>(3, c+12*i+2);
            fcount = parse_int_field<uint32_t>(4, c+12*i+4);

            // Recognized single-value TIFF tags. Currently, these must be 4
            // bytes or less, meaning they are stored in the last 4 bytes of
            // the 12-byte field.
            if((fcount==1) && (is_local_value(ftype, fcount))){
                switch(ftag){
                    case 256:
                        ifd.width = parse_int_field<int>(ftype, c+12*i+8);
                        break;
                    case 257:
                        ifd.height = parse_int_field<int>(ftype, c+12*i+8);
                        break;
                    case 258:
                        ifd.bits_per_sample = parse_int_field<int>(ftype, c+12*i+8);
                        break;
                    case 259:
                        ifd.compression = parse_int_field<int>(ftype, c+12*i+8);
                        break;
                    case 262:
                        ifd.photometric_interpretation = parse_int_field<int>(ftype, c+12*i+8);
                        break;
                    case 277:
                        ifd.samples_per_pixel = parse_int_field<int>(ftype, c+12*i+8);
                        break;
                    case 278:
                        ifd.rows_per_strip = parse_int_field<int>(ftype, c+12*i+8);
                        break;
                    default:
                        break;
                }
            }

            // strip offsets
            if(ftag==273){
                if(ftype>=5){
                    throw std::runtime_error("strip offsets must be 8-, 16-, or 32-bit integers");
                }
                fsize = TIFF_FIELD_TYPE_SIZES[ftype-1] * fcount;
                if(fsize<=4){
                    ifd.strip_offsets.push_back(parse_int_field<uint64_t>(ftype, c+12*i+8));
                } else{
                    ifd.strip_offsets.resize(fcount, 0);
                    uint64_t foffset = parse_int_field<uint64_t>(4, c+12*i+8);
                    std::unique_ptr<char[]> bytes(new char[fsize]);
                    s.seekg(foffset, s.beg);
                    s.read(&bytes[0], fsize);
                    parse_array<uint64_t>(
                        ftype,
                        fcount,
                        host_is_big_endian,
                        file_is_big_endian,
                        &bytes[0],
                        ifd.strip_offsets.data()
                    );
                }

            // strip byte counts
            } else if(ftag==279){
                if(ftype>=5){
                    throw std::runtime_error("strip byte counts must be 8-, 16-, or 32-bit integers...\n");
                }
                fsize = TIFF_FIELD_TYPE_SIZES[ftype-1] * fcount;
                if(fsize<=4){
                    ifd.strip_byte_counts.push_back(parse_int_field<uint64_t>(ftype, c+12*i+8));
                } else{
                    ifd.strip_byte_counts.resize(fcount, 0);
                    uint64_t foffset = parse_int_field<uint64_t>(4, c+12*i+8);
                    std::unique_ptr<char[]> bytes(new char[fsize]);
                    s.seekg(foffset, s.beg);
                    s.read(&bytes[0], fsize);
                    parse_array<uint64_t>(
                        ftype,
                        fcount,
                        host_is_big_endian,
                        file_is_big_endian,
                        &bytes[0],
                        ifd.strip_byte_counts.data()
                    );                   
                }
            }
        }

        // Last 4 bytes of the IFD contain the byte offset of the next IFD,
        // or 0 if this is the last IFD
        ifd.next_byte_offset = parse_uint_field<uint64_t>(4, c+12*ifd.count);

        return ifd;
    }


    /*
     *  Method: print_tiff_info
     *  -----------------------
     *  Print all of the stored metadata in this TIFFReader to stdout,
     *  including all recognized tags for all IFDs. Useful for debugging.
    */
    void print_tiff_info() const{
        std::cout << "host_is_big_endian: " << host_is_big_endian << std::endl;
        std::cout << "file_is_big_endian: " << file_is_big_endian << std::endl;
        std::cout << "n_frames: " << n_frames << std::endl;
        std::cout << "max_strip_size: " << max_strip_size << std::endl;
        for(uint64_t frame=0; frame<n_frames; ++frame){
            const IFD& ifd = ifds[frame];
            std::cout << "frame " << frame << ":\n";
            std::cout << "  height: " << ifd.height << std::endl;
            std::cout << "  width: " << ifd.width << std::endl;
            std::cout << "  bits_per_sample: " << ifd.bits_per_sample << std::endl;
            std::cout << "  samples_per_pixel: " << ifd.samples_per_pixel << std::endl;
            std::cout << "  rows_per_strip: " << ifd.rows_per_strip << std::endl;
            std::cout << "  compression: " << ifd.compression << std::endl;
            std::cout << "  photometric_interpretation: " << ifd.photometric_interpretation << std::endl;
        }
    }


    /* Destructor; close the raw filestream */
    ~TIFFReader(){
        if(s.is_open()){
            s.close();
        }
    }
}; // end TIFFReader

} // end namespace pitifful

#endif
