
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "parse.h"

int ischinese( char c ) {
	if ( c & 128 ) {
		return 1;
	} else {
		return 0;
	}
}

void parse_chinese( char* text, int byte_offset, int byte_length, void (*process_chinese)(char*,int,int,void*), void* data ) {
	if ( process_chinese ) {
		char* previous = text;
		char* current = text;
		char* next = text;
		while ( *current ) {
			if ( ( *next & 240 ) == 240 ) {
				next += 4;
			} else {
				if ( ( *next & 224 ) == 224 ) {
					next += 3;
				} else {
					if ( ( *next & 192 ) == 192 ) {
						next += 2;
					} else {
						next += 1;
					}
				}
			}
			char temp = *next;
			*next = 0;
			if ( previous != current ) {
				process_chinese( previous, byte_offset, next-previous, data );
			}
			byte_offset += ( current - previous );
			process_chinese( current, byte_offset, next-current, data );
			*next = temp;
			previous = current;
			current = next;
		}
	}
}

void parse_text(
	char* text,
	int byte_offset,
	int byte_length,
	void (*process_english)(char*,int,int,void*),
	void (*process_chinese)(char*,int,int,void*),
	void* data )
{
	char* start = text;
	char* end = text;
	while ( *start ) {
		if ( ischinese( *start ) ) {
			while ( *end && ischinese( *end ) ) {
				if ( ( *end & 240 ) == 240 ) {
					end += 4;
				} else {
					if ( ( *end & 224 ) == 224 ) {
						end += 3;
					} else {
						if ( ( *end & 192 ) == 192 ) {
							end += 2;
						} else {
							end += 1;
						}
					}
				}
			}
			char temp = *end;
			*end = 0;
			parse_chinese( start, byte_offset, end-start, process_chinese, data );
			*end = temp;
			byte_offset += ( end - start );
			start = end;
		} else {
			if ( isalnum( *start ) ) {
				while ( *end && isalnum(*end) ) {
					end++;
				}
				char temp = *end;
				*end = 0;
				if ( process_english ) {
					process_english( start, byte_offset, end-start, data );
				}
				*end = temp;
				byte_offset += ( end-start );
				start = end;
			} else {
				start++;
				end++;
				byte_offset++;
			}
		}
	}
}

void parse_html(
	char* html,
	int byte_offset,
	int byte_length,
	void (*process_text)(char*,int,int,void*),
	void (*process_word)(char*,int,int,void*),
	void (*process_chinese)(char*,int,int,void*),
	void (*process_tag)(char*,int,int,void*),
	void* data )
{
	char* start = html;
	char* end = html;
	while ( *start ) {
		if ( *start == '<' ) {
			while ( *end && *end != '>' ) {
				end++;
			}
			if ( *end ) {
				end++;
			}
			if ( start != end ) {
				char temp = *end;
				*end = 0;
				if ( process_tag ) {
					process_tag( start, byte_offset, end-start, data );
				}
				*end = temp;
				byte_offset += ( end-start );
				start = end;
			}
		} else {
			while ( *end && *end != '<' ) {
				end++;
			}
			if ( start != end ) {
				char temp = *end;
				*end = 0;
				if ( process_text ) {
					process_text( start, byte_offset, end-start, data );
				}
				parse_text( start, byte_offset, end-start, process_word, process_chinese, data );
				*end = temp;
				byte_offset += ( end-start );
				start = end;
			}
		}
	}
}

