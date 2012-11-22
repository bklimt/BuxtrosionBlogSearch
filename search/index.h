
#ifndef INDEX_H
#define INDEX_H

#include <stack>
#include <string>
using namespace std;

void index_word( char* s, int byte_offset, int byte_length, void* pstate );

struct index_document_state {
	int id;
	string section;
	stack<double> weights;
};

// process_score( docid, section, doclen, offset, length, score, data )
void lookup_word(
	char* s,
	void (*process_score)(int,const char*,int,int,int,double,void*),
	void* data
);

#endif
