#include "File_operations.h"


void create_new_file()
{
	FILE* file = get_open_file("wb");
	
	int records_n;
	printf("- records number: ");
	std::cin >> records_n;


	printf("\nRecords generating menu:\n");
	printf("1: Generate random records\n");
	printf("2: Fill in the records\n");

	std::string choice;
	std::cin >> choice;

	switch (choice[0])
	{
	case '1':
		generate_random_records(file, records_n);
		break;
	case '2':
		fill_in_records(file, records_n);
	}

	fclose(file);
}


void generate_random_records(FILE* file, int records_n)
{
	srand(time(NULL));

	for (int i = 0; i < records_n; i++)
	{
		for (int j = 0; j < PENTAGON_SIDES_N; j++)
		{
			int pentagon_side_len = rand() % (PENTAGON_SIDE_LEN_MAX + 1);
			fwrite(&pentagon_side_len, sizeof(byte), 1, file);
		}
	}
}

void fill_in_records(FILE* file, int records_n)
{
	for (int i = 0; i < records_n; i++)
	{
		printf("record %d: ", i);

		for (int j = 0; j < PENTAGON_SIDES_N; j++)
		{
			byte pentagon_side;
			scanf_s("%d", &pentagon_side);
			fwrite(&pentagon_side, sizeof(byte), 1, file);
		}
	}
}


void print_file()
{
	FILE* file = get_open_file("r");

	int rec_n = 0;
	while (true)
	{
		for (int i = 0; i < PENTAGON_SIDES_N; i++)
		{

		}
	}
}


FILE* get_open_file(std::string mode)
{
	std::string filename;

	printf("- filename: ");
	std::cin >> filename;
	filename = SEQ_FILES_DIR + ("/" + filename);

	FILE* file;
	fopen_s(&file, filename.c_str(), mode.c_str());

	return file;
}