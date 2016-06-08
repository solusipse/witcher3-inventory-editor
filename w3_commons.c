#include <string.h>

const char *outPrefix = "decomp_";

struct SaveFile {
    FILE *file;
    unsigned char *contents;

    uint32_t len;
    uint32_t pos;
};

struct SaveFile openSaveFile( char *path ) {
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

    // load contents
    f.contents = malloc( (f.len + 1) * sizeof(char) );
    fread( f.contents, f.len, 1, f.file );

    // close file
    fclose( f.file );

    return f;
}

uint32_t readInt32( struct SaveFile *f ) {
    uint32_t v =
        ((uint32_t) f->contents[ f->pos   ]      ) +
        ((uint32_t) f->contents[ f->pos+1 ] << 8 ) +
        ((uint32_t) f->contents[ f->pos+2 ] << 16) +
        ((uint32_t) f->contents[ f->pos+3 ] << 24);

    f->pos += 4;
    return v;
}

void read( struct SaveFile *f, char *dest, int size ) {
    memcpy( dest, &f->contents[f->pos], size );
    f->pos += size;
}

char *w3_outputFilename( char *filename ) {
    // remember to free
    char *fileout = malloc( strlen(filename) + strlen(outPrefix) + 1 );
    strcpy( fileout, outPrefix );
    strcat( fileout, filename );

    return fileout;
}

int checkMagicNumber( struct SaveFile *f, char *mnum ) {
    printf("%s\n", mnum);
    if ( f->pos >= f->len ) {
        return -1;
    }
    int s = memcmp( &f->contents[ f->pos ], mnum, strlen(mnum) );
    f->pos += strlen( mnum );
    return s;
}
