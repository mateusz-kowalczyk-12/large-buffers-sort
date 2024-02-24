#pragma once

#include <string>

#include "Buffer.h"
#include "Types.h"


class FileHandler
{
	static const std::string SEQ_FILES_DIR;

	std::string filename;
	int file_cursor;

	Buffer* buffer;

	SortingMetadata* sorting_metadata;


public:
	FileHandler(std::string filename, Buffer* buffer, SortingMetadata* sorting_metadata);


	std::string get_filename();

	/* Resets the given buffer if the parameter set so*/
	void set_buffer(Buffer* buffer, bool reset_buffer);

	Buffer* get_buffer();

	void set_file_cursor(int file_cursor);


	/* Writes the given record to the buffer */
	void write_record_to_buffer(byte record[], bool change_sorting_metadata);

	/* Flushes the buffer to the file */
	void write_buffer_to_file(bool change_sorting_metadata);

	/* Reads next record from the buffer and saves it in the given array.
	Returns whether the read operation was successful */
	bool read_record_from_buffer(byte record[], bool change_sorting_metadata);

	/* Performs a block read from the file handled by the object to the buffer */
	void read_buffer_from_file(bool change_sorting_metadata);
};