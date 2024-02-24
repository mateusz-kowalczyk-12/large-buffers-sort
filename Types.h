#pragma once

#define PENTAGON_SIDES_N 5
#define PENTAGON_SIDE_LEN_MAX 255

#define NAN -1
#define INF PENTAGON_SIDE_LEN_MAX + 1

#define BYTES_PER_KEY 2
#define RECORDS_PER_BUFFER 5 // 16 in experiment
#define BYTES_PER_RECORD (BYTES_PER_KEY + PENTAGON_SIDES_N) * sizeof(byte)
#define BYTES_PER_BUFFER BYTES_PER_RECORD * RECORDS_PER_BUFFER

// #define DEBUG
// #define NO_RECORDS_PRINTING


typedef unsigned char byte;

struct MergingHeapNode
{
	int distr_tape_idx;
	byte record[BYTES_PER_RECORD];
};

struct SortingMetadata
{
	int phases_n;
	int disk_writes;
	int disk_reads;
};


class Controller;
class Dbms;
class UserInterface;
struct MergingHeapNode;