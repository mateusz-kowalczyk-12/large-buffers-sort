#pragma once

#include <string>
#include <vector>

#include "Controller.h"
#include "FileHandler.h"
#include "Types.h"


class Dbms
{
	static const std::string SEQ_FILES_DIR;
	static const std::string TAPES_SUBDIR;
	static const int BUFFERS_NUMBER = 8; //1001

	Controller* p_controller;

	FileHandler* file_handler; // for the user's file
	std::vector<FileHandler*> tape_handlers; // for the intermediate tapes

	Buffer buffers[BUFFERS_NUMBER];

	MergingHeapNode merging_heap[BUFFERS_NUMBER - 1];
	int merging_heap_size;

	SortingMetadata sorting_metadata;


	/* Creates long runs on the disk from the file. Returns the number of the
	distribution tapes created */
	int create_long_runs();

	/* Iteratively merges and distributes long runs from the tapes untill one logn run remains */
	void do_merging_cycles(int distr_tapes_n, bool show_intermediate);


	/* Reads records from the file to fill all the buffers.
	Returns the number of records read */
	int read_all_buffers_from_file();

	/* Writes all the buffers to a tape, according to the long_run_idx. Creates a new tape if necessary */
	void write_all_buffers_to_tape(int records_in_buffers_n, int long_run_idx);

	/* Builds a max heap from the buffers with all of the records present in them */
	void build_heap_in_buffers(int records_in_buffers_n);

	/* Sorts the content of buffers as a one array. Uses the heap sort algorithm */
	void heap_sort_in_buffers(int records_in_buffers_n);

	/* Performs a heapify operation on the given records in the buffers */
	void heapify_in_buffers(int records_in_buffers_n, int record_idx);

	/* Merges the given number of first tapes into the last tape */
	void merge_tapes(int distr_tapes_n);

	/* Distributes the runs from the last tape to the rest. Returns the number
	of tapes the runs were distributed to*/
	int distribute_to_tapes(int distr_tapes_n);


	/* Merges first long runs from every distribution tape. Returns if there was anything to merge */
	bool merge_next_long_runs(int distr_tapes_n, short* last_key_from_tapes);

	/* Reads one record from every currently used distribution tapes and adds it to the merging heap*/
	void add_record_from_every_distr_tape_to_merging_heap(int distr_tapes_n, short* last_key_from_tapes);

	/* Adds the record to the merging_heap along with the index of the distribution tape that this
	record was read from */
	void add_record_to_merging_heap(byte record[BYTES_PER_RECORD], int distr_tape_idx);
	
	/* Copies the merging_heap root to the given address and removes it from the merging_heap */
	void remove_root_from_merging_heap(MergingHeapNode* root);

	/* Heapifies bottom-up the merging heap starting with the node of the given index */
	void heapify_upwards_in_merging_heap(int start_idx);

	/* Heapifies top-down the merging heap starting with the node of the given index */
	void heapify_downwards_in_merging_heap(int start_idx);


	/* Writes the sorting result into the original file */
	void write_result_to_file();

	/* Deletes the tapes and frees memory allocated for the tape handlers */
	void delete_tapes();

	/* Resets the buffers to a initial state. Sets al the content bytes to 0 */
	void reset_buffers();

	/* Returns the address of the record with the given index in the buffers */
	byte* get_record_from_buffers(int record_idx);

	/* Clears the content of a tape and resets the file handler */
	void clear_tape(std::string filename);
#ifdef DEBUG
	/* Debug function. Prints all the records present in the buffer according to
	the value of records_in_buffers_n */
	void debug__print_all_buffers(int records_in_buffers_n);
#endif
	/* Returns the value of the key present in the record */
	short get_key_from_record(byte* record);

	/* Returns the index of the parent of the given child in a heap */
	int get_parent_idx_in_heap(int child_idx);

	/* Swaps the given records that are located in the buffers */
	void swap_records(byte* record1, byte* record2);


public:
	Dbms();


	/* Setter */
	void set_p_controller(Controller* p_controller);

	/* Returns the tape handler to the given tape. Sets file_read_cursor to 0 */
	FileHandler* get_tape_handler(std::string filename);

	/* Getter */
	SortingMetadata get_sorting_metadata();


	/* Creates a new empty database sequential file and setts the handler to it */
	void create_file(std::string filename);

	/* Creates a new empty intermediate tape and setts adds a handler to it. Returns that handler */
	FileHandler* create_tape(std::string filename);

	/* Sets the handler to an existing sequential file */
	void add_existing_file(std::string filename);

	/* Loads the record, that is to be saved in the file, to the database.
	If last_rec is true, the records in the DBMS's File Handler's buffer are flushed to the file */
	void save_record(byte record[], bool last_rec);

	/* Passes all the records from the file to the Controller. Tells if this is an intermediate result */
	void get_all_records(bool intermediate);

	/* Sorts the file with large buffers */
	void sort_file(bool show_intermediate);


	/* Frees the dynamically allocated memory */
	void free_memory();
};