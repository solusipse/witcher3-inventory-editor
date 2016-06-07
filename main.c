#include "w3_decompress.c"
#include "w3_parse.c"

int main( int argc, char *argv[] ) {

    if ( argc <= 1 ) {
        puts( "Provide input files." );
        return 1;
    }

    for ( int i = 1; i < argc; i++ ) {
        int s;
        s = w3_decompressSaveFile( argv[i] );
        if ( s != 0 ) {
            continue;
        }
        w3_parseFile( argv[i] );
    }

    return 0;
}
