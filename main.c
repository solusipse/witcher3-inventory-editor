#include "w3_decompress.c"
#include "w3_compress.c"
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
        char *o = w3_outputFilename( argv[i] );
        w3_compressSaveFile( o );
        free( o );
        //w3_parseFile( argv[i] );
    }

    return 0;
}
