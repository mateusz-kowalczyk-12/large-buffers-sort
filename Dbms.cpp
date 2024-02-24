#include "Dbms.h"


/*** Private ***/

const std::string Dbms::SEQ_FILES_DIR = "seq_files/";
const std::string Dbms::TAPES_SUBDIR = "tapes/";


int Dbms::create_long_runs()
{
	int long_run_idx = 0;
	this->file_handler->set_file_cursor(0);

	while (true)
	{
		int records_in_buffers_n = this->read_all_buffers_from_file();
		if (records_in_buffers_n == 0)
			break;

		this->build_heap_in_buffers(records_in_buffers_n);
		this->heap_sort_in_buffers(records_in_buffers_n);
#ifdef DEBUG
		this->debug__print_all_buffers(records_in_buffers_n);
#endif
		this->write_all_buffers_to_tape(records_in_buffers_n, long_run_idx++);

		if (records_in_buffers_n < this->BUFFERS_NUMBER * RECORDS_PER_BUFFER) // no more records in the file
			break;
	}

	return std::min(long_run_idx, this->BUFFERS_NUMBER - 1);
}

void Dbms::do_merging_cycles(int distr_tapes_n, bool show_intermediate)
{
	for (int i = 0; i < distr_tapes_n; i++)
	{
		this->tape_handlers.at(i)->set_buffer(&this->buffers[i], true);
	}

	do
	{
		this->sorting_metadata.phases_n++;

		this->merge_tapes(distr_tapes_n);
		distr_tapes_n = this->distribute_to_tapes(distr_tapes_n);

		if (show_intermediate && distr_tapes_n > 1)
		{
			this->write_result_to_file();
			this->get_all_records(true);
		}

	} while (distr_tapes_n > 1);

	this->write_result_to_file();
	this->delete_tapes();
}


int Dbms::read_all_buffers_from_file()
{
	int records_in_buffers_n = 0;

	for (int i = 0; i < this->BUFFERS_NUMBER; i++)
	{
		this->file_handler->set_buffer(&this->buffers[i], true);
		this->file_handler->read_buffer_from_file(true);

		int records_read = this->buffers[i].get_records_n();

		if (records_read == 0)
			break;
		else
			records_in_buffers_n += records_read;
	}

	return records_in_buffers_n;
}

void Dbms::write_all_buffers_to_tape(int records_in_buffers_n, int long_run_idx)
{
	int tape_idx = long_run_idx % (this->BUFFERS_NUMBER - 1);
	std::string filename = this->TAPES_SUBDIR + "tape" + std::to_string(tape_idx);
	FileHandler* tape_handler = this->get_tape_handler(filename);

	if (tape_handler == nullptr)
		tape_handler = this->create_tape(filename);

	for (int i = 0; i < this->BUFFERS_NUMBER; i++)
	{
		tape_handler->set_buffer(&this->buffers[i], false);
		tape_handler->write_buffer_to_file(true);
	}
}

void Dbms::build_heap_in_buffers(int records_in_buffers_n)
{
	int last_parent = ((records_in_buffers_n - 1) - 1) / 2;

	for (int i = last_parent; i >= 0; i--)
	{
		this->heapify_in_buffers(records_in_buffers_n, i);
	}
}

void Dbms::heap_sort_in_buffers(int records_in_buffers_n)
{
	int unsorted_records_n = records_in_buffers_n;

	while (unsorted_records_n > 0)
	{
		byte* max_record = this->get_record_from_buffers(0);
		byte* last_record = this->get_record_from_buffers(unsorted_records_n - 1);

		this->swap_records(max_record, last_record);
		unsorted_records_n--;

		this->heapify_in_buffers(unsorted_records_n, 0);
	}
}

void Dbms::heapify_in_buffers(int records_in_buffers_n, int record_idx)
{
	byte* record = this->get_record_from_buffers(record_idx);
	short key = this->get_key_from_record(record);

	int l_child_idx = 2 * record_idx + 1;
	if (l_child_idx < records_in_buffers_n)
	{
		byte* l_child = this->get_record_from_buffers(l_child_idx);
		short l_child_key = this->get_key_from_record(l_child);

		int max_record_idx = record_idx;
		byte* max_record = record;
		short max_key = key;
		if (l_child_key > max_key)
		{
			max_record_idx = l_child_idx;
			max_record = l_child;
			max_key = l_child_key;
		}

		int r_child_idx = l_child_idx + 1;
		if (r_child_idx < records_in_buffers_n)
		{
			byte* r_child = this->get_record_from_buffers(r_child_idx);
			short r_child_key = this->get_key_from_record(r_child);

			if (r_child_key > max_key)
			{
				max_record_idx = r_child_idx;
				max_record = r_child;
				max_key = r_child_key;
			}
		}

		if (max_record_idx != record_idx)
		{
			this->swap_records(record, max_record);
			this->heapify_in_buffers(records_in_buffers_n, max_record_idx);
		}
	}
}

void Dbms::merge_tapes(int distr_tapes_n)
{
	std::string merge_tape_filename = this->TAPES_SUBDIR + "tape" + std::to_string(this->BUFFERS_NUMBER - 1);
	FileHandler* merge_tape = this->get_tape_handler(merge_tape_filename);
	
	if (merge_tape == nullptr)
		merge_tape = this->create_tape(merge_tape_filename);

	this->clear_tape(merge_tape_filename);
	merge_tape->set_buffer(&this->buffers[this->BUFFERS_NUMBER - 1], true);

	for (int i = 0; i < distr_tapes_n; i++)
	{
		this->tape_handlers.at(i)->set_file_cursor(0);
		this->tape_handlers.at(i)->get_buffer()->reset();
	}

	short* last_key_from_tapes = new short[distr_tapes_n];
	for (int i = 0; i < distr_tapes_n; i++)
	{
		last_key_from_tapes[i] = -INF;
	} // unnece

	while(this->merge_next_long_runs(distr_tapes_n, last_key_from_tapes));

	delete[] last_key_from_tapes;
}

int Dbms::distribute_to_tapes(int distr_tapes_n)
{
	for (int i = 0; i < distr_tapes_n; i++)
	{
		FileHandler* tape_handler = this->tape_handlers.at(i);
		
		this->clear_tape(tape_handler->get_filename());
		tape_handler->set_buffer(&this->buffers[i], false);
	}

	FileHandler* src_tape_handler = this->tape_handlers.at(this->tape_handlers.size() - 1);
	src_tape_handler->set_file_cursor(0);

	short last_key = -INF;
	int dst_tape_idx = 0;
	int new_distr_tapes_n = 1;

	byte record[BYTES_PER_RECORD];
	while (src_tape_handler->read_record_from_buffer(record, true))
	{
		short key = this->get_key_from_record(record);
		if (key < last_key)
		{
			dst_tape_idx = (dst_tape_idx + 1) % distr_tapes_n;
			new_distr_tapes_n++;
		}

		this->tape_handlers.at(dst_tape_idx)->write_record_to_buffer(record, true);
		last_key = key;
	}
	new_distr_tapes_n = std::min(new_distr_tapes_n, distr_tapes_n);

	for (int i = 0; i < new_distr_tapes_n; i++)
	{
		this->tape_handlers.at(i)->write_buffer_to_file(true);
	}

	return new_distr_tapes_n;
}


bool Dbms::merge_next_long_runs(int distr_tapes_n, short* last_key_from_tapes)
{
	this->add_record_from_every_distr_tape_to_merging_heap(distr_tapes_n, last_key_from_tapes);

	if (this->merging_heap_size == 0)
		return false;

	while (this->merging_heap_size > 0)
	{
		MergingHeapNode removed_node;
		this->remove_root_from_merging_heap(&removed_node);

		this->tape_handlers.at(this->tape_handlers.size() - 1) // merging tape handler
			->write_record_to_buffer(removed_node.record, true);

		byte record[BYTES_PER_RECORD];
		if (this->tape_handlers.at(removed_node.distr_tape_idx)->read_record_from_buffer(record, true))
		{
			short key = this->get_key_from_record(record);
			if (key >= last_key_from_tapes[removed_node.distr_tape_idx]) // records from the same run
			{
				this->add_record_to_merging_heap(record, removed_node.distr_tape_idx);
				last_key_from_tapes[removed_node.distr_tape_idx] = key;
			}
			else // record from a new run - needs to be saved for future read by moving the buffer cursor back
			{
				Buffer* buffer = this->tape_handlers.at(removed_node.distr_tape_idx)->get_buffer();
				buffer->set_cursor_pos(buffer->get_cursor_pos() - BYTES_PER_RECORD);
			}
		}
	}

	this->tape_handlers.at(this->tape_handlers.size() - 1) // merging tape handler
		->write_buffer_to_file(true);

	return true;
}

void Dbms::add_record_from_every_distr_tape_to_merging_heap(int distr_tapes_n, short* last_key_from_tapes)
{
	for (int i = 0; i < distr_tapes_n; i++)
	{
		byte record[BYTES_PER_RECORD];

		if (this->tape_handlers.at(i)->read_record_from_buffer(record, true))
		{
			short key = this->get_key_from_record(record);
			this->add_record_to_merging_heap(record, i);
			last_key_from_tapes[i] = key;
		}
	}
}

void Dbms::add_record_to_merging_heap(byte record[BYTES_PER_RECORD], int distr_tape_idx)
{
	memcpy(this->merging_heap[this->merging_heap_size].record, record, BYTES_PER_RECORD);
	this->merging_heap[this->merging_heap_size].distr_tape_idx = distr_tape_idx;

	this->heapify_upwards_in_merging_heap(this->merging_heap_size);
	this->merging_heap_size++;
}

void Dbms::remove_root_from_merging_heap(MergingHeapNode* root)
{
	memcpy(root->record, this->merging_heap[0].record, BYTES_PER_RECORD);
	root->distr_tape_idx = this->merging_heap[0].distr_tape_idx;

	std::swap(this->merging_heap[0], this->merging_heap[this->merging_heap_size - 1]);
	this->merging_heap_size--;
	this->heapify_downwards_in_merging_heap(0);
}

void Dbms::heapify_upwards_in_merging_heap(int start_idx)
{
	int parent_idx = this->get_parent_idx_in_heap(start_idx);
	if (parent_idx < 0)
		return;

	short parent_key = this->get_key_from_record(this->merging_heap[parent_idx].record);
	short key = this->get_key_from_record(this->merging_heap[start_idx].record);

	if (key < parent_key)
	{
		std::swap(this->merging_heap[start_idx], this->merging_heap[parent_idx]);
		heapify_upwards_in_merging_heap(parent_idx);
	}
}

void Dbms::heapify_downwards_in_merging_heap(int start_idx)
{
	short key = this->get_key_from_record(this->merging_heap[start_idx].record);

	int l_child_idx = 2 * start_idx + 1;
	if (l_child_idx < this->merging_heap_size)
	{
		int min_node_idx = start_idx;
		short min_key = key;

		short l_child_key = this->get_key_from_record(this->merging_heap[l_child_idx].record);
		if (l_child_key < min_key)
		{
			min_node_idx = l_child_idx;
			min_key = l_child_key;
		}

		int r_child_idx = l_child_idx + 1;
		if (r_child_idx < this->merging_heap_size)
		{
			short r_child_key = this->get_key_from_record(this->merging_heap[r_child_idx].record);
			if (r_child_key < min_key)
			{
				min_node_idx = r_child_idx;
				min_key = r_child_key;
			}
		}

		if (min_node_idx != start_idx)
		{
			std::swap(this->merging_heap[min_node_idx], this->merging_heap[start_idx]);
			this->heapify_downwards_in_merging_heap(min_node_idx);
		}
	}
}


void Dbms::write_result_to_file()
{
	this->create_file(this->file_handler->get_filename());

	FileHandler* src_tape_handler = this->tape_handlers.at(this->tape_handlers.size() - 1);
	src_tape_handler->set_file_cursor(0);

	byte record[BYTES_PER_RECORD];

	while (src_tape_handler->read_record_from_buffer(record, false))
	{
		this->file_handler->write_record_to_buffer(record, false);
	}
	this->file_handler->write_buffer_to_file(false);
}

void Dbms::delete_tapes()
{
	for (FileHandler* th : this->tape_handlers)
	{
		std::string filepath = this->SEQ_FILES_DIR + th->get_filename();
		remove(filepath.c_str());
		delete th;
	}

	this->tape_handlers.erase(this->tape_handlers.begin(), this->tape_handlers.end());
}

void Dbms::reset_buffers()
{
	for (int i = 0; i < this->BUFFERS_NUMBER; i++)
	{
		this->buffers[i].reset();
	}
}

byte* Dbms::get_record_from_buffers(int record_idx)
{
	Buffer* buffer = &this->buffers[record_idx / RECORDS_PER_BUFFER];
	byte* record = &buffer->get_content()[(record_idx % RECORDS_PER_BUFFER) * BYTES_PER_RECORD];

	return record;
}

void Dbms::clear_tape(std::string filename)
{
	FILE* file;
	std::string filepath = this->SEQ_FILES_DIR + filename;

	fopen_s(&file, filepath.c_str(), "wb");
	fclose(file);

	FileHandler* tape_handler = this->get_tape_handler(filename);
	tape_handler->set_file_cursor(0);
	if (tape_handler->get_buffer() != nullptr)
		tape_handler->get_buffer()->reset();
}
#ifdef DEBUG
void Dbms::debug__print_all_buffers(int records_in_buffers_n)
{
	for (int i = 0; i < records_in_buffers_n; i++)
	{
		Buffer* buffer = &this->buffers[i / RECORDS_PER_BUFFER];
		byte* record = &buffer->get_content()[(i % RECORDS_PER_BUFFER) * BYTES_PER_RECORD];
		printf("%d ", this->get_key_from_record(record));
	}
	putchar('\n');
}
#endif
short Dbms::get_key_from_record(byte* record)
{
	short key;
	memcpy(&key, record, BYTES_PER_KEY);

	return key;
}

int Dbms::get_parent_idx_in_heap(int child_idx)
{
	return (child_idx - 1) / 2;
}

void Dbms::swap_records(byte* record1, byte* record2)
{
	byte record_temp[BYTES_PER_RECORD];

	memcpy(record_temp, record1, BYTES_PER_RECORD);
	memcpy(record1, record2, BYTES_PER_RECORD);
	memcpy(record2, record_temp, BYTES_PER_RECORD);
}


/*** Public ***/

Dbms::Dbms()
{
	this->p_controller = nullptr;

	for (int i = 0; i < this->BUFFERS_NUMBER - 1; i++)
	{
		this->merging_heap[i].distr_tape_idx = NAN;
		memset(this->merging_heap[i].record, 0, BYTES_PER_RECORD);
	}

	this->file_handler = nullptr;
	this->merging_heap_size = 0;

	this->sorting_metadata.phases_n = 0;
	this->sorting_metadata.disk_writes = 0;
	this->sorting_metadata.disk_reads = 0;
}


void Dbms::set_p_controller(Controller* p_controller)
{
	this->p_controller = p_controller;
}

FileHandler* Dbms::get_tape_handler(std::string filename)
{
	for (FileHandler* fh : this->tape_handlers)
	{
		if (fh->get_filename() == filename)
		{
			fh->set_file_cursor(0);
			return fh;
		}
	}

	return nullptr;
}

SortingMetadata Dbms::get_sorting_metadata()
{
	return this->sorting_metadata;
}


void Dbms::create_file(std::string filename)
{
	FILE* file;
	std::string filepath = this->SEQ_FILES_DIR + filename;

	fopen_s(&file, filepath.c_str(), "wb");
	fclose(file);
	
	if (this->file_handler != nullptr)
		delete this->file_handler;
	this->file_handler = new FileHandler(filename, &this->buffers[0], &this->sorting_metadata);
}

FileHandler* Dbms::create_tape(std::string filename)
{
	FILE* file;
	std::string filepath = this->SEQ_FILES_DIR + filename;

	fopen_s(&file, filepath.c_str(), "wb");
	fclose(file);

	FileHandler* tape_handler = new FileHandler(filename, nullptr, &this->sorting_metadata);
	this->tape_handlers.push_back(tape_handler);

	return tape_handler;
}

void Dbms::add_existing_file(std::string filename)
{
	if (this->file_handler != nullptr)
		delete this->file_handler;
	this->file_handler = new FileHandler(filename, &this->buffers[0], &this->sorting_metadata);
	this->file_handler->get_buffer()->reset();
}

void Dbms::save_record(byte record[], bool last_rec)
{
	if (this->file_handler->get_buffer() == nullptr)
		this->file_handler->set_buffer(&this->buffers[0], true);

	this->file_handler->write_record_to_buffer(record, false);
	if (last_rec)
		this->file_handler->write_buffer_to_file(false);
}

void Dbms::get_all_records(bool intermediate)
{
	byte record[BYTES_PER_RECORD];
	int record_idx = 0;

	this->file_handler->set_file_cursor(0);

	while (this->file_handler->read_record_from_buffer(record, false))
	{
		this->p_controller->accept_record(record, record_idx, intermediate);
		record_idx++;
	}
}

void Dbms::sort_file(bool show_intermediate)
{
	this->sorting_metadata.phases_n = 0;
	this->sorting_metadata.disk_writes = 0;
	this->sorting_metadata.disk_reads = 0;

	int distr_tapes_n = this->create_long_runs();
	this->do_merging_cycles(distr_tapes_n, show_intermediate);
}


void Dbms::free_memory()
{
	if (this->file_handler != nullptr)
		delete this->file_handler;

	this->delete_tapes();
}