
#include "index.h"

#include <stdio.h>
#include <map>
#include <string>
#include <vector>
using namespace std;

#pragma warning(disable:4018)

struct document {
	int id;
	int word_length;
	int byte_length;
};

struct position {
	string section;
	int byte_offset;
	int byte_length;
	double weight;
};

struct occurrence {
	int count;
	vector<position> positions;
};

struct word {
	string text;
	int df;
	map<int,occurrence> occurrences;
};

vector< document > documents;
vector< word > words;

map< int, int > document_map;
map< string, int > word_map;

int total_document_length = 0;

void index_word( char* s, int byte_offset, int byte_length, void* pstate ) {
	index_document_state* state = (index_document_state*)pstate;
	int word_index = 0;
	int document_index = 0;

	map<string,int>::iterator word_ptr = word_map.find( s );
	if ( word_ptr == word_map.end() ) {
		word new_word;
		new_word.df = 0;
		new_word.text = s;
		word_index = words.size();
		word_map[ s ] = words.size();
		words.push_back( new_word );
	} else {
		word_index = word_ptr->second;
	}
	
	map<int,int>::iterator document_ptr = document_map.find( state->id );
	if ( document_ptr == document_map.end() ) {
		document new_document;
		new_document.id = state->id;
		new_document.word_length = 0;
		new_document.byte_length = 0;
		document_index = documents.size();
		document_map[ state->id ] = documents.size();
		documents.push_back( new_document );
	} else {
		document_index = document_ptr->second;
	}
	
	map<int,occurrence>::iterator occurrence_ptr;
	occurrence_ptr = words[ word_index ].occurrences.find( document_index );
	if ( occurrence_ptr == words[ word_index ].occurrences.end() ) {
		occurrence new_occurrence;
		new_occurrence.count = 0;
		words[ word_index ].occurrences[ document_index ] = new_occurrence;
	}

	position new_position;
	new_position.section = state->section;
	new_position.byte_offset = byte_offset;
	new_position.byte_length = byte_length;
	new_position.weight = state->weights.top();

	words[ word_index ].occurrences[ document_index ].count++;
	words[ word_index ].occurrences[ document_index ].positions.push_back( new_position );
	words[ word_index ].df++;
	documents[ document_index ].word_length++;
	if ( byte_offset + byte_length > documents[ document_index ].byte_length ) {
		documents[ document_index ].byte_length = byte_offset + byte_length;
	}
	total_document_length++;
}

void lookup_word( char* s, void (*process_score)(int,const char*,int,int,int,double,void*), void* data ) {
	map<string,int>::iterator word_ptr = word_map.find( s );
	if ( word_ptr != word_map.end() ) {
		int word_id = word_ptr->second;
		map<int,occurrence>::iterator doc_it;
		for ( doc_it=words[word_id].occurrences.begin(); doc_it!=words[word_id].occurrences.end(); doc_it++ ) {
			int did = documents[ doc_it->first ].id;
			int dl = documents[ doc_it->first ].word_length;
			int byte_dl = documents[ doc_it->first ].byte_length;
			double adl = (double)total_document_length / documents.size();
			int df = words[ word_id ].df;
			double tf = 0;
			for ( int i=0; i<doc_it->second.positions.size(); i++ ) {
				tf = doc_it->second.positions[i].weight;
				double tfw = tf / ( tf + 1.0 + (dl/adl) );
				double idf =  1.0 / df;
				double score = ( tfw * idf );
				int byte_offset = doc_it->second.positions[i].byte_offset;
				int byte_length = doc_it->second.positions[i].byte_length;
				const char* section = doc_it->second.positions[i].section.c_str();
				process_score( did, section, byte_dl, byte_offset, byte_length, score, data );
			}
		}
	}
}

