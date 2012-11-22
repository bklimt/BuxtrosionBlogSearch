
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
	map< int, vector<double> > docmap;
};

struct query_result {
	int document_id;
	double score;
	string best_snippet;
	string relevance_mask;
};

bool query_result_less( query_result qr1, query_result qr2 ) {
	if ( qr1.score == qr2.score ) {
		return ( qr1.document_id < qr2.document_id );
	}
	return ( qr1.score > qr2.score );
}

void find_best_snippet( query_result* result, int length, int context, query_data* query ) {
	int doc_id = result->document_id;
	int doc_length = query->docmap[ doc_id ].size();

	result->relevance_mask = string( doc_length, ' ' );
	for ( int i=0; i<doc_length; i++ ) {
		result->relevance_mask[ i ] = ( query->docmap[ doc_id ][ i ] ? '*' : ' ' );
	}

	if ( doc_length < length ) {
		length = doc_length;
	}
	if ( doc_length == length ) {
		result->best_snippet = get_blog_snippet( doc_id, 0, length, result->relevance_mask.c_str() );
		return;
	}

	vector<double> score_map( doc_length-length, 0.0 );
	double current_score = 0;
	int current_index = 0;
	for ( int i=0; i<length; i++ ) {
		current_score += query->docmap[ doc_id ][i];
	}
	score_map[ current_index ] = current_score;

	for ( current_index = 1; current_index < score_map.size(); current_index++ ) {
		current_score -= query->docmap[ doc_id ][ current_index-1 ];
		current_score += query->docmap[ doc_id ][ current_index+(length-1) ];
		score_map[ current_index ] = current_score;
	}

	int best_index = 0;
	double best_score = score_map[ 0 ];
	for ( int i=1; i<score_map.size(); i++ ) {
		if ( score_map[i] > best_score ) {
			best_score = score_map[i];
			best_index = i;
		}
	}

	while ( best_index + context >= score_map.size() ) {
		context--;
	}
	if ( best_score ) {
		if ( score_map[ best_index+context ] == best_score ) {
			best_index += context;
		}
	}

	result->best_snippet = get_blog_snippet( doc_id, best_index, length, result->relevance_mask.c_str() );
}

void process_score( int docid, const char* section, int doclen, int byte_offset, int byte_length, double score, void* pq ) {
	query_data* q = (query_data*)pq;
	if ( q->scores.find( docid ) == q->scores.end() ) {
		q->scores[docid] = score;
	} else {
		q->scores[docid] += score;
	}

	if ( q->docmap.find( docid ) == q->docmap.end() ) {
		q->docmap[ docid ] = vector<double>( doclen, 0 );
	}
	if ( !strcmp( section, "html" ) ) {
		for ( int i=byte_offset; i<byte_offset+byte_length; i++ ) {
			q->docmap[ docid ][ i ]++;
		}
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

vector< query_result > do_query( char* query_text, int snippet_length, int snippet_context ) {
	query_data q;
	parse_text( query_text, 0, strlen(query_text), query_english, query_chinese, &q );
	vector< query_result > ans;
	for ( map<int,double>::iterator it=q.scores.begin(); it!=q.scores.end(); it++ ) {
		query_result result;
		result.score = it->second;
		result.document_id = it->first;
		find_best_snippet( &result, snippet_length, snippet_context, &q );
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

void index_html_document( int id, char* title, char* html, void* data ) {
	index_document_state state;
	state.id = id;
	state.section = "title";
	state.weights.push( 4 );
	parse_text( title, 0, strlen(title), index_english, index_chinese, &state );
	state.weights.pop();
	state.section = "html";
	state.weights.push( 1 );
	parse_html( html, 0, strlen(html), NULL, index_english, index_chinese, NULL, &state );
}

int main( int argc, char** argv ) {
	if ( argc != 4 ) {
		fprintf( stderr, "usage: ./search <string> <snippet length> <snippet context>\n" );
		exit(-1);
	}

	if ( !load_stopwords( "common_words.txt" ) ) {
		exit(-1);
	}
	if ( !get_blog_entries( index_html_document, NULL ) ) {
		exit(-1);
	}
	
	//char buffer[1000];
	//sprintf( buffer, "   China food " );
	vector< query_result > results = do_query( argv[1], atoi(argv[2]), atoi(argv[3]) );
	for ( int i=0; i<results.size(); i++ ) {
		printf( "%.4f\t%d\t%s\n", results[i].score, results[i].document_id, results[i].best_snippet.c_str() );
	}

	return 0;
}

