
extern "C" {
	int stem(char* word);
}

int load_stopwords( char* filename );
int is_stopword( char* word );
