#include "FileHandler.h"


/*** Private ***/

const std::string FileHandler::SEQ_FILES_DIR = "seq_files/";


/*** Public ***/

FileHandler::FileHandler(std::string filename, Buffer* buffer, SortingMetadata* sorting_metadata)
{
	this->filename = filename;
	this->file_cursor = 0;

	this->buffer = buffer;

	this->sorting_metadata = sorting_metadata;
}


std::string FileHandler::get_filename()
{
	return this->filename;
}

void FileHandler::set_buffer(Buffer* buffer, bool reset_buffer)
{
	this->buffer = buffer;

	if (reset_buffer && buffer != nullptr)
		this->buffer->reset();
}

Buffer* FileHandler::get_buffer()
{
	return this->buffer;
}

void FileHandler::set_file_cursor(int file_cursor)
{
	this->file_cursor = file_cursor;
}


void FileHandler::write_record_to_buffer(byte record[], bool change_sorting_metadata)
{
	int buffer_records_n = this->buffer->get_records_n();
	int buffer_cursor_pos = this->buffer->get_cursor_pos();

	byte* buffer_write_addr = &this->buffer->get_content()[buffer_cursor_pos];
	memcpy(buffer_write_addr, record, BYTES_PER_RECORD);

	this->buffer->set_records_n(buffer_records_n + 1);
	this->buffer->set_cursor_pos(buffer_cursor_pos + BYTES_PER_RECORD);

	if (this->buffer->get_cursor_pos() >= BYTES_PER_BUFFER)
		this->write_buffer_to_file(change_sorting_metadata);
}

void FileHandler::write_buffer_to_file(bool change_sorting_metadata)
{
	FILE* file;
	std::string filepath = this->SEQ_FILES_DIR + this->filename;

	fopen_s(&file, filepath.c_str(), "ab");
	if (errno != 0)
	{
		char buffer[1024];
		strerror_s(buffer, 1024, errno);
		printf("%s\n", buffer);
		exit(0);
	}

	fseek(file, this->file_cursor, SEEK_SET);
	size_t written_bytes_n = fwrite(this->buffer->get_content(), sizeof(byte),
		this->buffer->get_records_n() * BYTES_PER_RECORD, file);
	fclose(file);

	this->file_cursor += written_bytes_n;

	memset(this->buffer->get_content(), 0, BYTES_PER_BUFFER);
	this->buffer->set_cursor_pos(0);
	this->buffer->set_records_n(0);

	if (change_sorting_metadata && written_bytes_n > 0)
		this->sorting_metadata->disk_writes++;
}

bool FileHandler::read_record_from_buffer(byte record[], bool change_sorting_metadata)
{
	int buffer_records_n = this->buffer->get_records_n();

	if ((buffer_records_n == 0) || //no records in the buffer
		(this->buffer->get_cursor_pos() >= buffer_records_n * BYTES_PER_RECORD)) // all records in the buffer already read
	{
		this->read_buffer_from_file(change_sorting_metadata);
	}

	buffer_records_n = this->buffer->get_records_n();

	if (buffer_records_n > 0)
	{
		int buffer_cursor_pos = this->buffer->get_cursor_pos();
		byte* buffer_read_addr = &this->buffer->get_content()[buffer_cursor_pos];

		memcpy(record, buffer_read_addr, BYTES_PER_RECORD);

		this->buffer->set_cursor_pos(buffer_cursor_pos + BYTES_PER_RECORD);

		return true;
	}

	return false;
}

void FileHandler::read_buffer_from_file(bool change_sorting_metadata)
{
	FILE* file;
	std::string filepath = this->SEQ_FILES_DIR + this->filename;

	fopen_s(&file, filepath.c_str(), "rb");
	fseek(file, this->file_cursor, SEEK_SET);
	size_t read_bytes_n = fread(this->buffer->get_content(), 1, BYTES_PER_BUFFER, file);
	fclose(file);

	this->file_cursor += read_bytes_n;

	this->buffer->set_cursor_pos(0);
	this->buffer->set_records_n(read_bytes_n / BYTES_PER_RECORD);

	if (change_sorting_metadata && read_bytes_n > 0)
		this->sorting_metadata->disk_reads++;
}