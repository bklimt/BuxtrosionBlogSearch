#ifndef STORAGE_H
#define STORAGE_H

#include <string>
using namespace std;

int get_blog_photos(
	void (*process_entry)(int,char*,char*,void*), // id, caption, keywords, data
	void* data
);

int get_blog_photo(
	int id,
	void (*process_entry)(int,char*,char*,void*), // id, caption, keywords, data
	void* data
);

extern "C" {
	char* pg_encoding_to_char(int);
}

#endif
