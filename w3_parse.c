static int checkSaveHeader( struct SaveFile *f ) {
    f->pos = 4;
    return strncmp( f->contents, "SAV3", 4 );
}

static void calculateOffsets( struct SaveFile *f, int *variableTable, int *stringTable ) {
    // goto end
    f->pos = f->len - 6;
    *variableTable = readInt32( f );
    *stringTable = *variableTable - 10;

    // TODO: check if next 2 chars are "SE" (magic number)
}

int w3_parseFile( char *filename ) {

    char *path = w3_outputFilename( filename );
    struct SaveFile f = openSaveFile( path );
    free( path );

    if ( f.file == NULL ) {
        free( f.contents );
        return -1;
    }

    if ( checkSaveHeader( &f ) != 0 ) {
        free( f.contents );
        return -1;
    }

    int variableTableOffset, stringTableOffset;
    calculateOffsets( &f, &variableTableOffset, &stringTableOffset );

    printf( "%d, %d\n", variableTableOffset, stringTableOffset );

    f.pos = stringTableOffset;
    int nmSectionOffset = readInt32( &f );
    int rbSectionOffset = readInt32( &f );


    printf( "%d, %d\n", nmSectionOffset, rbSectionOffset );



    free( f.contents );

    return 0;
}
