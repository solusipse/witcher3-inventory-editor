

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

    const int maxChunkSize = 1048576;
    const int chunksNumber = f.len / maxChunkSize + 1;
    printf("%d, %d, %d\n", maxChunkSize, f.len, chunksNumber);

    // goto headerOffset
    f.pos += 4;
    const int headerOffset = readInt32( &f );

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

    // copy old header to the new file
    fwrite( f.contents, 1, headerOffset, s );

    int outPos = headerOffset;

    for ( int i = 0; i < chunksNumber; i++ ) {
        int chunkSize = maxChunkSize;

        if ( f.len - f.pos < maxChunkSize ) {
            chunkSize = f.len - f.pos;
        }

        char *input = malloc( chunkSize );
        char *out = malloc( chunkSize );
        
        read( &f, input, headers[i].decompressedSize );
        
        int sizeOut = LZ4_compress_fast( input, out, chunkSize, chunkSize, 1 );
        printf( "Compressed chunk: %d. Compressed size: %d.\n", i, sizeOut);

        outPos += sizeOut;

        headers[i].decompressedSize = chunkSize;
        headers[i].compressedSize = sizeOut;
        headers[i].end = outPos;

        // write contents
        fwrite(out, sizeof(char), sizeOut, s);
        
        free( input );
        free( out );
    }

    // write new header
    fseek( s, 8, 0 );
    fwrite( &chunksNumber, sizeof(chunksNumber), 1, s );
    fwrite( &headerOffset, sizeof(headerOffset), 1, s );

    for ( int i = 0; i < chunksNumber; i++ ) {
        fwrite( &headers[i].compressedSize, sizeof(int), 1, s );
        fwrite( &headers[i].decompressedSize, sizeof(int), 1, s );
        fwrite( &headers[i].end, sizeof(int), 1, s );
    }

    // fill rest of the header with zeros
    const int rest = headerOffset - ftell( s );
    char *b = calloc( rest, 1 );
    fwrite( b, 1, rest, s );

    cleanup( &f, s, headers );

    puts("Compressed succesfully.");


    return 0;
}
