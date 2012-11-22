#ifndef STORAGE_H
#define STORAGE_H

#include <string>
using namespace std;

int get_blog_entries(
	void (*process_entry)(int,char*,char*,void*), // id, title, html, data
	void* data
);

int get_blog_entry(
	int id,
	void (*process_entry)(int,char*,char*,void*), // id, title, html, data
	void* data
);

string get_blog_snippet( int document_id, int byte_offset, int byte_length, const char* relevance_mask );

extern "C" {
	char* pg_encoding_to_char(int);
}

#endif
