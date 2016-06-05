#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lz4.h"

struct SaveFile {
    FILE *file;
    char *contents;

    int len;
    int pos;
};

struct ChunkHeader {
    int compressedSize;
    int decompressedSize;
    int end;
};

static struct SaveFile openSaveFile( char *path ) {
    struct SaveFile f = { NULL, NULL, 0, 0 };
    f.file = fopen( path, "rb" );

    if ( f.file == NULL ) {
        printf( "File %s does not exist.\n", path );
        return f;
    }

    // set length
    fseek( f.file, 0, SEEK_END );
    f.len = ftell( f.file );
    // goto file beginning
    fseek( f.file, 0, SEEK_SET );

    return f;
}

static int cleanup( struct SaveFile *f, FILE *s, struct ChunkHeader *headers ) {
    free( headers );
    free( f->contents );

    fclose( s );
    fclose( f->file );

    return 0;
}

static int checkHeader( struct SaveFile *f ) {
    f->pos = 8;
    return strncmp( f->contents, "SNFHFZLC", 8 );
}

/*
int32_t readInt32( struct SaveFile *f ) {
    int32_t v = *(uint32_t *) &f->contents[ f->pos ];
    f->pos += 4;
    return v;
}
*/

static uint32_t readInt32( struct SaveFile *f ) {
    uint32_t v =
        ((unsigned char) f->contents[ f->pos   ]      ) +
        ((unsigned char) f->contents[ f->pos+1 ] << 8 ) +
        ((unsigned char) f->contents[ f->pos+2 ] << 16) +
        ((unsigned char) f->contents[ f->pos+3 ] << 24);

    f->pos += 4;
    return v;
}

static void read( struct SaveFile *f, char *dest, int size ) {
    memcpy( dest, &f->contents[f->pos], size );
    f->pos += size;
}

char *w3_outputFilename( char *filename ) {
    // remember to free
    char *prefix = "decomp_";
    char *fileout = malloc( strlen(filename) + strlen(prefix) + 1 );
    strcpy( fileout, prefix );
    strcat( fileout, filename );

    return fileout;
}

int w3_decompressSaveFile( char *filename ) {

    struct SaveFile f = openSaveFile( filename );
    if ( f.file == NULL ) {
        return -1;
    }

    printf( "Decompressing file: %s\n", filename );

    f.contents = malloc( (f.len + 1) * sizeof(char) );
    fread( f.contents, f.len, 1, f.file );

    fclose( f.file );

    int ch = checkHeader( &f );
    if ( ch != 0 ) {
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

        if ( headers[i].compressedSize != result || result < 0 ) {
            puts( "Decompression error!" );
            cleanup( &f, s, headers );

            return -1;
        }

        printf( "Decompressed chunk: %d. Compressed size: %d, decompressed size: %d, offset: %d.\n",
                i, headers[i].compressedSize, headers[i].decompressedSize, headers[i].end);

        fwrite( out, headers[i].decompressedSize, 1, s );

        free( input );
        free( out );
    }

    cleanup( &f, s, headers );

    puts( "Success.\n" );

    return 0;

}
