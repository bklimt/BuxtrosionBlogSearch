
#include <stdlib.h>
#include <libpq-fe.h>
#include "storage.h"

#pragma warning(disable:4018)

int get_blog_entries( void (*process_entry)(int,char*,char*,void*), void* data ) {
	int i;
	PGconn *conn = PQconnectdb("dbname=blog user=nobody password=nobody");
	//PGconn *conn = PQsetdbLogin("localhost", "5432", "", "", "blog", "nobody", "nobody");
	if ( !conn ) {
		fprintf( stderr, "unable to connect to database:  out of memory!\n" );
		return 0;
	}
	if ( PQstatus(conn) == CONNECTION_BAD ) {
		fprintf( stderr, "unable to connect to database: %s\n", PQerrorMessage(conn) );
		return 0;
	}
	PQsetClientEncoding(conn,"UNICODE");
	//fprintf( stderr, "PostgreSQL client encoding: %s\n", pg_encoding_to_char(PQclientEncoding(conn)) );

	PGresult *rs = PQexec( conn, "select id,title,html from blog_entry where deleted is null" );
	if ( !rs ) {
		fprintf( stderr, "unable to query database:  out of memory!\n" );
		return 0;
	}
	if ( PQresultStatus(rs) != PGRES_TUPLES_OK ) {
		fprintf( stderr, "unable to query database:  %s\n", PQresultErrorMessage(rs) );
		return 0;
	}
	int rs_size = PQntuples(rs);

	for ( i=0; i<rs_size; i++ ) {
		int id = atoi( PQgetvalue(rs,i,0) );
		if ( process_entry ) {
			process_entry( id, PQgetvalue(rs,i,1), PQgetvalue(rs,i,2), data );
		}
	}

	PQfinish( conn );
	return rs_size;
}

int get_blog_entry( int id, void (*process_entry)(int,char*,char*,void*), void* data ) {
	int i;
	char query[1000];
	sprintf( query, "select id,title,html from blog_entry where id=%d", id );

	PGconn *conn = PQconnectdb("dbname=blog user=nobody password=nobody");
	if ( !conn ) {
		fprintf( stderr, "unable to connect to database:  out of memory!\n" );
		return 0;
	}
	if ( PQstatus(conn) == CONNECTION_BAD ) {
		fprintf( stderr, "unable to connect to database: %s\n", PQerrorMessage(conn) );
		return 0;
	}
	PQsetClientEncoding(conn,"UNICODE");
	//fprintf( stderr, "PostgreSQL client encoding: %s\n", pg_encoding_to_char(PQclientEncoding(conn)) );

	PGresult *rs = PQexec( conn, query );
	if ( !rs ) {
		fprintf( stderr, "unable to query database:  out of memory!\n" );
		return 0;
	}
	if ( PQresultStatus(rs) != PGRES_TUPLES_OK ) {
		fprintf( stderr, "unable to query database:  %s\n", PQresultErrorMessage(rs) );
		return 0;
	}
	int rs_size = PQntuples(rs);

	for ( i=0; i<rs_size; i++ ) {
		int id = atoi( PQgetvalue(rs,i,0) );
		if ( process_entry ) {
			process_entry( id, PQgetvalue(rs,i,1), PQgetvalue(rs,i,2), data );
		}
	}

	PQfinish( conn );
	return rs_size;
}

string get_blog_snippet( int document_id, int byte_offset, int length, const char* relevance_mask ) {
	int i;
	char query[1000];
	const char* snippet_mask = relevance_mask;
	sprintf( query, "select id,title,html from blog_entry where id=%d", document_id );

	PGconn *conn = PQconnectdb("dbname=blog user=nobody password=nobody");
	if ( !conn ) {
		fprintf( stderr, "unable to connect to database:  out of memory!\n" );
		return 0;
	}
	if ( PQstatus(conn) == CONNECTION_BAD ) {
		fprintf( stderr, "unable to connect to database: %s\n", PQerrorMessage(conn) );
		return 0;
	}
	PQsetClientEncoding(conn,"UNICODE");
	//fprintf( stderr, "PostgreSQL client encoding: %s\n", pg_encoding_to_char(PQclientEncoding(conn)) );

	PGresult *rs = PQexec( conn, query );
	if ( !rs ) {
		fprintf( stderr, "unable to query database:  out of memory!\n" );
		return 0;
	}
	if ( PQresultStatus(rs) != PGRES_TUPLES_OK ) {
		fprintf( stderr, "unable to query database:  %s\n", PQresultErrorMessage(rs) );
		return 0;
	}
	int rs_size = PQntuples(rs);

	string snippet;
	string snippet_text;
	for ( i=0; i<rs_size; i++ ) {
		//int id = atoi( PQgetvalue(rs,i,0) );
		if ( strlen( PQgetvalue(rs,i,2) ) < length ) {
			snippet_text = PQgetvalue(rs,i,2);
		} else {
			snippet_mask += byte_offset;
			char* temp = (char*)malloc( length+1 );
			memcpy( temp, PQgetvalue(rs,i,2)+byte_offset, length );
			temp[ length ] = 0;
			char* end = temp+length-1;
			while ( *end & 128 ) {
				*end-- = 0;
			}
			char* start = temp;
			while ( *start ) {
				if ( *start == '\n' || *start == '\t' || *start == '\r' ) {
					*start = ' ';
				}
				start++;
			}
			start = temp;
			while ( ( *start & 0xC0 ) == 0x80 ) {
				snippet_mask++;
				*start++;
			}
			char* tag_end = NULL;
			char* tag_start = strchr( start, '<' );
			while ( tag_start ) {
				tag_end = strchr( tag_start, '>' );
				if ( tag_end ) {
					while ( tag_start != tag_end ) {
						*tag_start++ = ' ';
					}
					*tag_end = ' ';
				} else {
					*tag_start = 0;
				}
				tag_start = strchr( start, '<' );
			}
			tag_end = strchr( start, '>' );
			while ( tag_end ) {
				snippet_mask += ( ( tag_end+1 ) - start );
				start = tag_end + 1;
				tag_end = strchr( start, '>' );
			}
			snippet_text = start;
			free( temp );
		}
	}
	snippet = "";
	bool in_bold = false;
	for ( int i=0; i<snippet_text.length(); i++ ) {
		if ( (!in_bold) && (snippet_mask[i] != ' ') ) {
			snippet += "<b>";
			in_bold = true;
		}
		if ( (in_bold) && (snippet_mask[i] == ' ') ) {
			snippet += "</b>";
			in_bold = false;
		}
		snippet += snippet_text[i];
	}
	if ( in_bold ) {
		snippet += "</b>";
		in_bold = false;
	}
	if ( snippet.length() && byte_offset ) {
		snippet = (string)"..." + snippet;
	}
	snippet += "...";

	PQfinish( conn );
	return snippet;
}

