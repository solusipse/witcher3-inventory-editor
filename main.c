#include "w3_decompress.c"

int main( int argc, char *argv[] ) {

    if ( argc <= 1 ) {
        puts( "Provide input files." );
        return 1;
    }

    for ( int i = 1; i < argc; i++ ) {
        w3_decompressSaveFile( argv[i] );
    }

    return 0;
}
