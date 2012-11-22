
#ifndef PARSE_H
#define PARSE_H

void parse_text(
	char* text,
	int byte_offset,
	int byte_length,
	void (*process_word)(char*,int,int,void*),
	void (*process_chinese)(char*,int,int,void*),
	void* data
);

void parse_html(
	char* html,
	int byte_offset,
	int byte_length,
	void (*process_text)(char*,int,int,void*),
	void (*process_word)(char*,int,int,void*),
	void (*process_chinese)(char*,int,int,void*),
	void (*process_tag)(char*,int,int,void*),
	void* data
);

#endif
