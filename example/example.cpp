/* Demo some pitifful functionality */
#include <iostream>
#include <memory>
#include <pitifful.h>


int main(int argc, char* argv[]){
    if(argc!=2){
        std::cerr << "usage: example <TIFF_PATH>\n";
        return 1;
    }
    const char* path = argv[1];

    // Instantiate reader
    pitifful::TIFFReader reader(path);

    // Print a bunch of TIFF info to stdout, including all of
    // the basic tags for each frame
    reader.print_tiff_info();

    // Get the number of image frames
    uint64_t n_frames = reader.get_n_frames();

    // Example 1: Read all of the image frames into memory
    for(uint64_t frame=0; frame<n_frames; ++frame){

        // Number of elements in this frame's image. For grayscale images
        // this is the number of pixels. For RGB images this is the number
        // of pixels multiplied by 3, and so on.
        int n_samples = reader.get_n_samples(frame);

        // Read this frame into a buffer, casting to 16-bit (in this case).
        // Also supports 8-bit, 32-bit, float, double, and other types.
        std::unique_ptr<uint16_t[]> im(new uint16_t[n_samples]);
        reader.read_frame<uint16_t>(frame, &im[0]);
    }

    // Example 2: Get metadata for all of the image frames. In this trival
    // example, we compute the total number of bits per uncompressed frame. 
    for(uint64_t frame=0; frame<n_frames; ++frame){

        // Get the image file directory (IFD) for this frame
        const pitifful::IFD& ifd = reader.get_ifd(frame);

        // Total number of bits in this frame
        uint32_t n_pixels = ifd.height * ifd.width;
        uint32_t n_samples = n_pixels * ifd.samples_per_pixel;
        uint32_t n_bits = n_samples * ifd.bits_per_sample;
    }

    return 0;
}
