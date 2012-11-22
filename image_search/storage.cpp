
#include <stdlib.h>
#include <libpq-fe.h>
#include "storage.h"

int get_blog_photos( void (*process_entry)(int,char*,char*,void*), void* data ) {
	int i;
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

	PGresult *rs = PQexec( conn, "select id,caption,keywords from blog_photo" );
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

int get_blog_photo( int id, void (*process_entry)(int,char*,char*,void*), void* data ) {
	int i;
	char query[1000];
	sprintf( query, "select id,caption,keywords from blog_photo where id=%d", id );

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

