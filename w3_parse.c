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

    // read NM section
    f.pos = nmSectionOffset;
    if ( checkMagicNumber( &f, "NM" ) != 0 ) {
        puts( "Not valid Witcher 3 save file!" );
        free( f.contents );
        return -2;
    }
    stringTableOffset = f.pos;

    // read RB section
    f.pos = rbSectionOffset;
    if ( checkMagicNumber( &f, "RB" ) != 0 ) {
        puts( "Not valid Witcher 3 save file!" );
        free( f.contents );
        return -2;
    }
    int rbCount = readInt32( &f );

    struct rbEntry {
        uint16_t size;
        uint32_t offset;
    };

    struct rbEntry *rbEntries = malloc( rbCount * sizeof(struct rbEntry) );

    for ( int i = 0; i < rbCount; i++ ) {
        rbEntries[i].size   = readInt16( &f );
        rbEntries[i].offset = readInt32( &f );
        /*
        printf("%d\n", rbEntries[i].size);
        printf("%d\n", rbEntries[i].offset);
        */
    }

    free( rbEntries  );
    free( f.contents );

    // read variable name section
    f.pos = stringTableOffset;

    return 0;
}
