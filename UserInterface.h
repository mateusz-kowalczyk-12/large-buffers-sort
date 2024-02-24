#pragma once

#include <cstdio>
#include <iostream>
#include <string>

#include "Controller.h"
#include "Types.h"


class UserInterface
{
	Controller *controller;

public:
	UserInterface();

	UserInterface(Controller* controller);


	/* Shows the main menu */
	void show_main_menu();


	/* Shows the file creating menu */
	void show_file_creating_menu();

	/* Shows the existing file adding menu */
	void show_existing_file_adding_menu();

	/* Shows the file filling menu */
	void show_file_filling_menu();

	/* Shows the file printing menu */
	void show_file_printing_menu();

	/* Shows the file sorting menu */
	void show_file_sorting_menu();


	/* Shows the custom records filling menu and enables the user to fill in custom records */
	void show_custom_records_filling_menu(int records_n);

	/* Asks the user for filename and returns it as a std::string */
	std::string ask_for_filename();

	/* Prints a record given by the Controller */
	void print_record(short key, byte record[], int record_idx, bool intermediate);
};