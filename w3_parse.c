int calculateOffsets( struct SaveFile *f, int *variableTable, int *stringTable ) {
    // goto end
    f->pos = f->len - 6;
    *variableTable = readInt32( f );
    *stringTable = *variableTable - 10;

    if ( checkMagicNumber(f, "SE") != 0 ) {
        puts( "Not valid Witcher 3 save file!" );
        return -1;
    }

    return 0;
}

int w3_parseFile( char *filename ) {

    char *path = w3_outputFilename( filename );
    struct SaveFile f = openSaveFile( path );
    free( path );

    if ( f.file == NULL ) {
        free( f.contents );
        return -1;
    }

    // calculate header's end
    f.pos = readInt32( &f );

    if ( checkMagicNumber(&f, "SAV3") != 0 ) {
        puts( "Not valid Witcher 3 save file!");
        free( f.contents );
        return -1;
    }

    int variableTableOffset, stringTableOffset;
    if ( calculateOffsets( &f, &variableTableOffset, &stringTableOffset ) != 0 ) {
        puts( "Not valid Witcher 3 save file!" );
        free( f.contents );
        return -2;
    }

    f.pos = stringTableOffset;
    int nmSectionOffset = readInt32( &f );
    int rbSectionOffset = readInt32( &f );

    f.pos = nmSectionOffset;
    if ( checkMagicNumber( &f, "NM" ) != 0 ) {
        puts( "Not valid Witcher 3 save file!" );
        free( f.contents );
        return -2;
    }

    free( f.contents );

    return 0;
}
