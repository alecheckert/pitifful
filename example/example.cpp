#include <iostream>
#include <pitifful.h>


int main(int argc, char* argv[]){
    if(argc!=2){
        std::cerr << "usage: example <TIFF_PATH>\n";
        return 1;
    }
    const char* path = argv[1];

    // Instantiate reader
    pitifful::TIFFReader reader(path);

    // Print a bunch of TIFF info to stdout
    reader.print_tiff_info();

    return 0;
}
