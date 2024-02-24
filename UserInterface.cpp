#include "UserInterface.h"


UserInterface::UserInterface()
	: controller(nullptr)
{}

UserInterface::UserInterface(Controller* controller)
	: controller(controller)
{}


void UserInterface::show_main_menu()
{
	bool exit = false;

	while (!exit)
	{
		printf("\nMain menu:\n");
		printf(
			"1: Create new sequential file\n"
			"2: Use existing sequential file\n"
			"3: Print a sequential file content\n"
			"4: Fill a sequential file\n"
			"5: Sort a sequential file\n"
			"7: Exit\n");

		std::string choice;
		std::cin >> choice;

		switch (choice[0])
		{
		case '1':
			this->show_file_creating_menu();
			break;
		case '2':
			this->show_existing_file_adding_menu();
			break;
		case '3':
			this->show_file_printing_menu();
			break;
		case '4':
			this->show_file_filling_menu();
			break;
		case '5':
			this->show_file_sorting_menu();
			break;
		case '7':
			this->controller->close_application();
			exit = true;
			break;
		default:
			break;
		}
	}
}


void UserInterface::show_file_creating_menu()
{
	std::string filename = this->ask_for_filename();

	this->controller->create_file(filename);
}

void UserInterface::show_existing_file_adding_menu()
{
	std::string filename = this->ask_for_filename();

	this->controller->add_existing_file(filename);
}

void UserInterface::show_file_filling_menu()
{
	printf("\nFile filling menu:\n");
	printf(
		"1: Use random records\n"
		"2: Use custom records\n");

	std::string choice;
	std::cin >> choice;

	int records_n;
	printf("- records number: ");
	std::cin >> records_n;

	switch (choice[0])
	{
	case '1':
		this->controller->generate_random_records(records_n);
		break;
	case '2':
		this->show_custom_records_filling_menu(records_n);
		break;
	}
}

void UserInterface::show_file_printing_menu()
{
	printf("\nRecords:\n");
	this->controller->get_all_records();
}

void UserInterface::show_file_sorting_menu()
{
	printf("Show file after each phase? (y/n): ");
	
	std::string choice;
	std::cin >> choice;

	printf("\nRecords before sorting:\n");
	this->controller->get_all_records();

	this->controller->sort_file(choice[0] == 'y');

	printf("\nRecords after sorting:\n");
	this->controller->get_all_records();

	SortingMetadata sorting_metadata = this->controller->get_sorting_metadata();
	printf("\nphases number: %d\ndisk writes number: %d\ndisk reads number: %d\n",
		sorting_metadata.phases_n, sorting_metadata.disk_writes, sorting_metadata.disk_reads);
}


void UserInterface::show_custom_records_filling_menu(int records_n)
{
	byte record_data[BYTES_PER_RECORD - BYTES_PER_KEY]; // without the key

	for (int r_idx = 0; r_idx < records_n; r_idx++)
	{
		printf("record %d: ", r_idx);

		for (int j = 0; j < PENTAGON_SIDES_N; j++)
		{
			int pentagon_side_len;
			scanf_s("%d", &pentagon_side_len);
			record_data[j] = (byte)pentagon_side_len;
		}
		this->controller->save_record(record_data, r_idx, records_n);
	}
}

std::string UserInterface::ask_for_filename()
{
	std::string filename;

	printf("- filename: ");
	std::cin >> filename;

	return filename;
}


void UserInterface::print_record(short key, byte record_data[], int record_idx, bool intermediate)
{
	if (intermediate && record_idx == 0)
		printf("\nRecords after a phase:\n");
#ifndef NO_RECORDS_PRINTING
	printf("%d: [perimeter: %hi] sides lengths: ", record_idx, key);

	for (int i = 0; i < PENTAGON_SIDES_N; i++)
	{
		printf("%d", (byte)record_data[i]);
		if (i == PENTAGON_SIDES_N - 1)
			putchar('\n');
		else
			putchar(' ');
	}
#endif
}