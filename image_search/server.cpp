
#include <ctype.h>
#include <stdlib.h>

#include "index.h"
#include "parse.h"
#include "storage.h"
#include "language.h"

#include <map>
#include <vector>
#include <algorithm>
using namespace std;

#pragma warning(disable:4018)

struct query_data {
	map<int,double> scores;
};

struct query_result {
	int document_id;
	double score;
};

bool query_result_less( query_result qr1, query_result qr2 ) {
	if ( qr1.score == qr2.score ) {
		return ( qr1.document_id < qr2.document_id );
	}
	return ( qr1.score > qr2.score );
}

void process_score( int docid, const char* section, int doclen, int byte_offset, int byte_length, double score, void* pq ) {
	query_data* q = (query_data*)pq;
	if ( q->scores.find( docid ) == q->scores.end() ) {
		q->scores[docid] = score;
	} else {
		q->scores[docid] += score;
	}
}

void query_english( char* word, int byte_offset, int byte_length, void* pq ) {
	if ( !is_stopword( word ) ) {
		stem( word );
		char* c = word;
		while ( *c ) {
			if ( isupper( *c ) ) {
				*c = tolower( *c );
			}
			c++;
		}
		if ( *word ) {
			lookup_word( word, process_score, pq );
		}
	}
}

void query_chinese( char* word, int byte_offset, int byte_length, void* pq ) {
	lookup_word( word, process_score, pq );
}

vector< query_result > do_query( char* query_text ) {
	query_data q;
	parse_text( query_text, 0, strlen(query_text), query_english, query_chinese, &q );
	vector< query_result > ans;
	for ( map<int,double>::iterator it=q.scores.begin(); it!=q.scores.end(); it++ ) {
		query_result result;
		result.score = it->second;
		result.document_id = it->first;
		ans.push_back( result );
	}
	sort( ans.begin(), ans.end(), query_result_less );
	return ans;
}

void index_english( char* word, int byte_offset, int byte_length, void* pstate ) {
	if ( !is_stopword( word ) ) {
		stem( word );
		char* c = word;
		while ( *c ) {
			if ( isupper( *c ) ) {
				*c = tolower( *c );
			}
			c++;
		}
		if ( *word ) {
			index_word( word, byte_offset, byte_length, pstate );
		}
	}
}

void index_chinese( char* word, int byte_offset, int byte_length, void* pstate ) {
	index_word( word, byte_offset, byte_length, pstate );
}

void index_html_document( int id, char* caption, char* keywords, void* data ) {
	index_document_state state;
	state.id = id;
	state.section = "keywords";
	state.weights.push( 1 );
	parse_text( keywords, 0, strlen(keywords), index_english, index_chinese, &state );
	state.weights.pop();
	state.section = "caption";
	state.weights.push( 1 );
	parse_html( caption, 0, strlen(caption), NULL, index_english, index_chinese, NULL, &state );
}

int main( int argc, char** argv ) {
	if ( argc != 2 ) {
		fprintf( stderr, "usage: ./image_search <string>\n" );
		exit(-1);
	}

	if ( !load_stopwords( "common_words.txt" ) ) {
		exit(-1);
	}
	if ( !get_blog_photos( index_html_document, NULL ) ) {
		exit(-1);
	}
	
	//char buffer[1000];
	//sprintf( buffer, "   China food " );
	vector< query_result > results = do_query( argv[1] );
	for ( int i=0; i<results.size(); i++ ) {
		printf( "%.4f\t%d\n", results[i].score, results[i].document_id );
	}

	return 0;
}

