#include "Buffer.h"


Buffer::Buffer()
{
	this->reset();
}


byte* Buffer::get_content()
{
	return this->content;
}

int Buffer::get_cursor_pos()
{
	return this->cursor_pos;
}

void Buffer::set_cursor_pos(int cursor_pos)
{
	this->cursor_pos = cursor_pos;
}

int Buffer::get_records_n()
{
	return records_n;
}

void Buffer::set_records_n(int records_n)
{
	this->records_n = records_n;
}


void Buffer::reset()
{
	memset(this->content, 0, BYTES_PER_BUFFER);

	this->cursor_pos = 0;
	this->records_n = 0;
}