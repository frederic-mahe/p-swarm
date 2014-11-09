/*
 * qgram.h
 *
 *  Created on: Oct 25, 2014
 *      Author: mimitantono
 */

#ifndef QGRAM_H_
#define QGRAM_H_

#include <stdio.h>
#include <emmintrin.h>
#include <string.h>
#include "cpu_info.h"
#include "db.h"

void findqgrams(unsigned char * seq, unsigned long seqlen, unsigned char * qgramvector);
void qgram_work_diff(unsigned long seed, unsigned long listlen, unsigned long * amplist, unsigned long * difflist, class Db_data* db);

#endif /* QGRAM_H_ */
