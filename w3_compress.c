

int w3_compressSaveFile( char *filename ) {
    // TODO: factor

    struct SaveFile f = openSaveFile( filename );
    if ( f.file == NULL ) {
        return -1;
    }

    printf( "Compressing file: %s\n", filename );

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
    char *fileout = "compressed.sav";

    FILE *s = fopen( fileout, "wb" );
    if ( s == NULL ) {
        puts( "Could not save output!" );
        return -1;
    }

    // copy header to the new file (to maintain offset addresses)
    fwrite( f.contents, 1, headerOffset, s );

    for ( int i = 0; i < chunksNumber; i++ ) {
        char *input = malloc( headers[i].decompressedSize );
        char *out = malloc( headers[i].compressedSize );

        read( &f, input, headers[i].decompressedSize );
        int result = LZ4_compress_default( input, out, headers[i].decompressedSize, headers[i].compressedSize );

        if ( result != headers[i].compressedSize ) {
            puts( "Compression error!" );
            cleanup( &f, s, headers );

            return -1;
        }

        printf( "Decompressed chunk: %d. Compressed size: %d, decompressed size: %d, offset: %d.\n",
                i, headers[i].compressedSize, headers[i].decompressedSize, headers[i].end);

        // write contents
        fwrite(out, sizeof(char), headers[i].compressedSize, s);

        free( input );
        free( out );
    }

    cleanup( &f, s, headers );

    puts("Compressed succesfully.");


    return 0;
}
