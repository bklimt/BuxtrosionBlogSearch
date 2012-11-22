
#include "language.h"
#include <stdio.h>
#include <set>
#include <string>
using namespace std;

set<string> stopwords;

int load_stopwords( char* filename ) {
	FILE* f = fopen( filename, "r" );
	if ( f ) {
		char buffer[1001];
		while ( !feof(f) ) {
			fgets( buffer, 1000, f );
			if ( *buffer ) {
				if ( buffer[ strlen(buffer)-1 ] == '\n' ) {
					buffer[ strlen(buffer) -1 ] = 0;
				}
				stopwords.insert( buffer );
			}
		}
		fclose(f);
	}
	return 1;
}

int is_stopword( char* word ) {
	return stopwords.find( word ) != stopwords.end();
}
