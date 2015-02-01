#include "scan.h"

scanner::scanner() {
	sd = new struct search_data;
	master_targets = 0;
	dirbufferbytes = 0;
}

scanner::~scanner() {
	if (sd->qtable)
		delete[] (sd->qtable);
	if (sd->qtable_w)
		delete[] (sd->qtable_w);
	if (sd->dprofile)
		delete[] (sd->dprofile);
	if (sd->dprofile_w)
		delete[] (sd->dprofile_w);
	if (sd->hearray)
		delete[] (sd->hearray);
	if (sd->dir_array)
		delete[] (sd->dir_array);
	if (sd)
		delete sd;
}

void scanner::search_init() {
	for (long i = 0; i < query.len; i++) {
		sd->qtable[i] = sd->dprofile + 64 * query.seq[i];
		sd->qtable_w[i] = sd->dprofile_w + 32 * query.seq[i];
	}
}

void scanner::search_chunk() {
	if (sd->target_count == 0)
		return;

	std::vector<queryinfo_t> targets;
	for (unsigned long i = 0; i < sd->target_count; i++) {
		targets.push_back(Property::db_data.get_queryinfo(master_targets[i]));
	}

	if (Property::bits == 16)
		searcher.search16(sd, &targets, &master_result, &query, dirbufferbytes / 8, Property::db_data.longest);
	else
		searcher.search8(sd, &targets, &master_result, &query, dirbufferbytes / 8, Property::db_data.longest);
}

void scanner::master_dump() {
	printf("master_dump\n");
	printf("   i    t    s    d\n");
	for (unsigned long i = 0; i < 1403; i++) {
		printf("%4lu %4lu %4lu %4lu\n", i, master_targets[i], master_result[i].score, master_result[i].diff);
	}
}

void scanner::search_do(unsigned long query_no, unsigned long listlength, unsigned long * targets) {
	query = Property::db_data.get_queryinfo(query_no);

	master_targets = targets;

	sd->target_count = listlength;

	search_init();
	search_chunk();
}

void scanner::search_begin() {
	dirbufferbytes = 8 * Property::longest * ((Property::longest + 3) / 4) * 4;
	sd->qtable = new BYTE*[Property::db_data.longest];
	sd->qtable_w = new WORD*[Property::db_data.longest];
	sd->dprofile = new BYTE[4 * 16 * 32];
	sd->dprofile_w = new WORD[4 * 2 * 8 * 32];
	sd->hearray = new BYTE[Property::db_data.longest * 32];
	sd->dir_array = new unsigned long[dirbufferbytes];

	memset(sd->hearray, 0, Property::db_data.longest * 32);
	memset(sd->dir_array, 0, dirbufferbytes);

	for (unsigned long i = 0; i < Property::db_data.sequences; i++) {
		search_result sr;
		master_result.push_back(sr);
	}
}

