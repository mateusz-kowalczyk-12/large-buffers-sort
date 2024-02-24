#pragma once

#include <ctime>
#include <string>

#include "Dbms.h"
#include "Types.h"
#include "UserInterface.h"


class Controller
{
	Dbms* p_dbms;
	UserInterface* p_ui;


public:
	Controller();

	Controller(Dbms* p_dbms);


	void set_p_dbms(Dbms* p_dbms);

	void set_p_ui(UserInterface* p_ui);


	/* Creates a file */
	void create_file(std::string filename);

	/* Add an existing file */
	void add_existing_file(std::string filename);

	/* Loads the record, that is to be saved in the file, to the
	database and computes the key value */
	void save_record(byte record[], int record_idx, int records_n);

	/* Generates random records and passes it to the Controller::save_record function */
	void generate_random_records(int records_n);

	/* Asks the DBMS for all the records from the file */
	void get_all_records();

	/* Asks the DBMS to sort the file and, if parameter set so, asks to show intermediate results */
	void sort_file(bool show_intermediate);


	/* Accepts a record read by the Dbms */
	void accept_record(byte record[], int record_idx, bool intermediate);

	/* Asks the DBMS for the sorting metadata */
	SortingMetadata get_sorting_metadata();

	/* Asks the DBMS to prepare everything to close the application */
	void close_application();
};