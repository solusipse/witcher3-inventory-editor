#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "lz4.h"
#include "w3_commons.c"


struct ChunkHeader {
    int compressedSize;
    int decompressedSize;
    int end;
};

static int cleanup( struct SaveFile *f, FILE *s, struct ChunkHeader *headers ) {
    free( headers );
    free( f->contents );

    fclose( s );
    fclose( f->file );

    return 0;
}

int w3_decompressSaveFile( char *filename ) {

    struct SaveFile f = openSaveFile( filename );
    if ( f.file == NULL ) {
        return -1;
    }

    printf( "Decompressing file: %s\n", filename );

    if ( checkMagicNumber( &f, "SNFHFZLC" ) != 0 ) {
        puts( "Invalid save file!" );
        return 1;
    }

    int chunksNumber = readInt32( &f );
    int headerOffset = readInt32( &f );

    // alloc memory for the header
    struct ChunkHeader *headers = malloc( chunksNumber * sizeof(struct ChunkHeader) );
    if ( headers == NULL ) {
        puts( "Could not reserve memory! " );
        return 2;
    }

    // insert values into header
    for ( int i = 0; i < chunksNumber; i++ ) {
        headers[i].compressedSize = readInt32( &f );
        headers[i].decompressedSize = readInt32( &f );
        headers[i].end = readInt32( &f );
    }

    // set position to the header's start
    f.pos = headerOffset;

    // add decomp_ prefix for the output file
    char *fileout = w3_outputFilename( filename );

    FILE *s = fopen( fileout, "w" );
    if ( s == NULL ) {
        puts( "Could not save output!" );
        free( fileout );
        return -1;
    }

    free( fileout );

    // set header to zeros (except first 4 bytes which is
    // header's offet) to maintain offsets addresses
    char *emptyHeader = calloc( headerOffset - 4, 1 );
    fwrite( &headerOffset, sizeof(headerOffset), 1, s );
    fwrite( emptyHeader, headerOffset - 4, 1, s );
    free( emptyHeader );

    for ( int i = 0; i < chunksNumber; i++ ) {
        char *input = malloc( headers[i].compressedSize );
        read( &f, input, headers[i].compressedSize );

        if ( f.pos != headers[i].end && headers[i].end != 0 ) {
            puts( "File reading error!" );
            cleanup( &f, s, headers );

            return -1;
        }

        char *out = malloc( headers[i].decompressedSize );
        int result = LZ4_decompress_fast( input, out, headers[i].decompressedSize );

        if ( headers[i].compressedSize != result || (( i == chunksNumber - 1 ) && result < 0) ) {
            puts( "Decompression error!" );
            cleanup( &f, s, headers );

            return -1;
        }

        printf( "Decompressed chunk: %d. Compressed size: %d, decompressed size: %d, offset: %d.\n",
                i, headers[i].compressedSize, headers[i].decompressedSize, headers[i].end);

        // write contents
        size_t sizeOut = fwrite( out, headers[i].decompressedSize, 1, s );
        if ( sizeOut != 1 ) {
            puts( "Write error!" );
            cleanup( &f, s, headers );
            return -1;
        }

        free( input );
        free( out );
    }

    cleanup( &f, s, headers );

    puts("Decompressed succesfully.");

    return 0;

}
