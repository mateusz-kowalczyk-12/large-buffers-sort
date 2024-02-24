#pragma once

#include <cstring>

#include "Types.h"


class Buffer
{
	byte content[BYTES_PER_BUFFER];

	int cursor_pos;
	int records_n;


public:
	Buffer();


	byte* get_content();

	int get_cursor_pos();

	void set_cursor_pos(int cursor_pos);

	int get_records_n();

	void set_records_n(int records_n);


	/* Resets to the initial state */
	void reset();
};