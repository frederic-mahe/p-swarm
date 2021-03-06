/*
 * clusterdata.cc
 *
 *  Created on: Mar 1, 2015
 *      Author: mimitantono
 */

#include "clusterdata.h"
#include "property.h"
#include <string.h>

cluster_data::cluster_data() {
	thread_id = -1;
	next_comparison = new std::vector<unsigned long int>[Property::depth + 1];
	matches_found = 0;
	qgram_performed = 0;
	scan_performed = 0;
	row_stat = 0;
	scanner.search_begin();
	iteration_stat = new unsigned long int[Property::depth + 1];
	for (unsigned int i = 0; i <= Property::depth; i++) {
		iteration_stat[i] = 0;
	}
}

cluster_data::~cluster_data() {
	if (next_comparison)
		delete[] next_comparison;
	if (iteration_stat)
		delete[] iteration_stat;
}

void cluster_data::reset() {
	for (unsigned int i = 0; i <= Property::depth; i++) {
		std::vector<unsigned long int>().swap(next_comparison[i]);
	}
}

void cluster_data::write_next_comparison(unsigned long int col_id, unsigned int distance) {
	if (distance <= Property::max_next)
		next_comparison[Property::max_next_map[distance]].push_back(col_id);
}

