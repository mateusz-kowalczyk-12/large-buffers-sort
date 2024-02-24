#pragma once

#include "Types.h"


void create_new_file();


void generate_random_records(FILE* file, int records_n);

void fill_in_records(FILE* file, int records_n);


void print_file();


FILE* get_open_file(std::string mode);