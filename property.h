/*
 * property.h
 *
 *  Created on: Oct 26, 2014
 *      Author: mimitantono
 */

#ifndef PROPERTY_H_
#define PROPERTY_H_

#include <stdio.h>
#include <string>
#include "util.h"

#define DEFAULT_GAPOPEN 12
#define DEFAULT_GAPEXTEND 4
#define DEFAULT_MATCHSCORE 5
#define DEFAULT_MISMATCHSCORE (-4)
#define DEFAULT_THREADS 2
#define DEFAULT_RESOLUTION 1
#define MAX_THREADS 100

class Property {
public:
	static void init();
	static void print();
	static std::string databasename;
	static std::string outfilename;
	static long gapopen;
	static long gapextend;
	static long penalty_gapopen;
	static long penalty_gapextend;
	static long penalty_mismatch;
	static long penalty_factor;
	static long gapopenextend;
	static long matchscore;
	static long mismatchscore;
	static long threads;
	static long resolution;
	static unsigned long bits;
	static unsigned long diff_saturation;
	static FILE* outfile;
	static FILE* debugfile;
	static FILE* dbdebug;
	static void set_resolution(long value);
	static void set_outfile(std::string value);
	static void set_threads(long value);
	static void set_matchscore(long value);
	static void set_mismatchscore(long value);
	static void set_gapopen(long value);
	static void set_gapextend(long value);
	static void calculate_penalty();
};

#endif /* PROPERTY_H_ */
