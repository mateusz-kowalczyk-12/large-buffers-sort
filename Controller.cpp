#include "Controller.h"


Controller::Controller()
	: p_dbms(nullptr)
{}

Controller::Controller(Dbms* p_dbms)
	: p_dbms(p_dbms)
{}


void Controller::set_p_dbms(Dbms* p_dbms)
{
	this->p_dbms = p_dbms;
}

void Controller::set_p_ui(UserInterface* p_ui)
{
	this->p_ui = p_ui;
}


void Controller::create_file(std::string filename)
{
	this->p_dbms->create_file(filename);
}

void Controller::add_existing_file(std::string filename)
{
	this->p_dbms->add_existing_file(filename);
}

void Controller::save_record(byte record_data[], int record_idx, int records_n)
{
	byte record[BYTES_PER_RECORD];
	short key = 0;

	for (int i = 0; i < PENTAGON_SIDES_N; i++)
	{
		key += (byte)record_data[i];
	}

	memcpy(record, &key, BYTES_PER_KEY);
	memcpy(&record[BYTES_PER_KEY], record_data, BYTES_PER_RECORD);

	this->p_dbms->save_record(record, record_idx == records_n - 1);
}

void Controller::generate_random_records(int records_n)
{
	srand(time(NULL));
	byte record_data[BYTES_PER_RECORD];

	for (int r_idx = 0; r_idx < records_n; r_idx++)
	{
		while (true)
		{
			for (int j = 0; j < PENTAGON_SIDES_N; j++) // generate
			{
				byte pentagon_side_len = rand() % (PENTAGON_SIDE_LEN_MAX + 1);
				record_data[j] = pentagon_side_len;
			}

			bool record_valid = true;

			for (int j = 0; j < PENTAGON_SIDES_N; j++) // validate
			{
				short sum = 0;
				for (int k = 0; k < PENTAGON_SIDES_N; k++)
				{
					if (j == k)
						continue;
					sum += record_data[k];
				}
				if (record_data[j] >= sum)
				{
					record_valid = false;
					break;
				}
			}

			if (record_valid)
				break;
		}
		this->save_record(record_data, r_idx, records_n);
	}
}

void Controller::get_all_records()
{
	this->p_dbms->get_all_records(false);
}

void Controller::sort_file(bool show_intermediate)
{
	this->p_dbms->sort_file(show_intermediate);
}


void Controller::accept_record(byte record[], int record_idx, bool intermediate)
{
	short key;
	byte record_data[BYTES_PER_RECORD - BYTES_PER_KEY];

	memcpy(&key, record, BYTES_PER_KEY);
	memcpy(record_data, &record[BYTES_PER_KEY], BYTES_PER_RECORD - BYTES_PER_KEY);

	this->p_ui->print_record(key, record_data, record_idx, intermediate);
}

SortingMetadata Controller::get_sorting_metadata()
{
	return this->p_dbms->get_sorting_metadata();
}

void Controller::close_application()
{
	this->p_dbms->free_memory();
}