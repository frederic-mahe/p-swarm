#include "cluster.h"

cluster_job::cluster_job(Db_data * db) {
	this->db = db;
	scanner.set_db(db);
	scanner.search_begin();
}

cluster_job::~cluster_job() {
}

cluster_result * cluster_job::algo_run(int threadid) {
	fprintf(stderr, "\nRunning thread #%d for %lu sequences", threadid, db->sequences);

	unsigned long swarmed;
	unsigned long seeded;
	unsigned long count_comparisons_8 = 0;
	unsigned long count_comparisons_16 = 0;

	unsigned long targetcount;
	unsigned long * targetindices = new unsigned long[db->sequences];
	unsigned long * targetampliconids = new unsigned long[db->sequences];
	unsigned long * scores = new unsigned long[db->sequences];
	unsigned long * diffs = new unsigned long[db->sequences];
	unsigned long * alignlengths = new unsigned long[db->sequences];
	unsigned long * qgramamps = new unsigned long[db->sequences];
	unsigned long * qgramdiffs = new unsigned long[db->sequences];
	unsigned long * qgramindices = new unsigned long[db->sequences];
	unsigned long * hits = new unsigned long[db->sequences];

	unsigned long searches = 0;
	unsigned long estimates = 0;

	unsigned long largestswarm = 0;
	unsigned long swarmsize = 0;

	unsigned long maxgenerations = 0;

	ampliconinfo_s * amps = new ampliconinfo_s[db->sequences];

	/* set ampliconid for all */
	for (unsigned long i = 0; i < db->sequences; i++) {
		amps[i].ampliconid = i;
	}

	/* always search in 8 bit mode unless resolution is very high */

	unsigned long bits;

	if ((unsigned long) Property::resolution <= Property::diff_saturation())
		bits = 8;
	else
		bits = 16;

	seeded = 0;
	swarmed = 0;

	unsigned long swarmid = 0;

	progress_init("Clustering:       ", db->sequences);
	while (seeded < db->sequences) {

		/* process each initial seed */

		swarmid++;

		unsigned long amplicons_copies = 0;
		unsigned long singletons = 0;
		unsigned long hitcount = 0;
		unsigned long diffsum = 0;
		unsigned long maxradius = 0;
		unsigned long maxgen = 0;
		unsigned long seedindex;

		seedindex = seeded;
		seeded++;

		amps[seedindex].swarmid = swarmid;
		amps[seedindex].generation = 0;
		amps[seedindex].radius = 0;

		unsigned long seedampliconid = amps[seedindex].ampliconid;
		hits[hitcount++] = seedampliconid;

		unsigned long abundance = db->get_seqinfo(seedampliconid)->abundance;
		amplicons_copies += abundance;
		if (abundance == 1)
			singletons++;

		swarmsize = 1;
		swarmed++;

		/* find diff estimates between seed and each amplicon in pool */

		targetcount = 0;

		unsigned long listlen = db->sequences - swarmed;

		for (unsigned long i = 0; i < listlen; i++)
			qgramamps[i] = amps[swarmed + i].ampliconid;

		qgram_work_diff(seedampliconid, listlen, qgramamps, qgramdiffs, db);

		estimates += listlen;

		for (unsigned long i = 0; i < listlen; i++) {
			unsigned poolampliconid = qgramamps[i];
			long diff = qgramdiffs[i];
			amps[swarmed + i].diffestimate = diff;
			if (diff <= Property::resolution) {
				targetindices[targetcount] = swarmed + i;
				targetampliconids[targetcount] = poolampliconid;
				targetcount++;
			}
		}

		if (targetcount > 0) {
			scanner.search_do(seedampliconid, targetcount, targetampliconids, scores, diffs, alignlengths, bits);
			searches++;

			if (bits == 8)
				count_comparisons_8 += targetcount;
			else
				count_comparisons_16 += targetcount;

			for (unsigned long t = 0; t < targetcount; t++) {
				unsigned diff = diffs[t];

				if (diff <= (unsigned long) Property::resolution) {
					unsigned i = targetindices[t];

					/* move the target (i) to the position (swarmed)
					 of the first unswarmed amplicon in the pool */

					if (swarmed < i) {
						struct ampliconinfo_s temp = amps[i];
						for (unsigned j = i; j > swarmed; j--) {
							amps[j] = amps[j - 1];
						}
						amps[swarmed] = temp;
					}

					amps[swarmed].swarmid = swarmid;
					amps[swarmed].generation = 1;
					if (maxgen < 1)
						maxgen = 1;
					amps[swarmed].radius = diff;
					if (diff > maxradius)
						maxradius = diff;

					unsigned poolampliconid = amps[swarmed].ampliconid;
					hits[hitcount++] = poolampliconid;

					diffsum += diff;

					abundance = db->get_seqinfo(poolampliconid)->abundance;
					amplicons_copies += abundance;
					if (abundance == 1)
						singletons++;

					swarmsize++;

					swarmed++;
				}
			}

			while (seeded < swarmed) {

				/* process each subseed */

				unsigned subseedampliconid;
				unsigned subseedradius;

				unsigned long subseedindex;
				unsigned long subseedgeneration;

				subseedindex = seeded;
				subseedampliconid = amps[subseedindex].ampliconid;
				subseedradius = amps[subseedindex].radius;
				subseedgeneration = amps[subseedindex].generation;

				seeded++;

				targetcount = 0;

				unsigned long listlen = 0;
				for (unsigned long i = swarmed; i < db->sequences; i++) {
					unsigned long targetampliconid = amps[i].ampliconid;
					if (amps[i].diffestimate <= subseedradius + Property::resolution) {
						qgramamps[listlen] = targetampliconid;
						qgramindices[listlen] = i;
						listlen++;
					}
				}

				qgram_work_diff(subseedampliconid, listlen, qgramamps, qgramdiffs, db);

				estimates += listlen;

				for (unsigned long i = 0; i < listlen; i++)
					if ((long) qgramdiffs[i] <= Property::resolution) {
						targetindices[targetcount] = qgramindices[i];
						targetampliconids[targetcount] = qgramamps[i];
						targetcount++;
					}

				if (targetcount > 0) {
					scanner.search_do(subseedampliconid, targetcount, targetampliconids, scores, diffs, alignlengths, bits);
					searches++;

					if (bits == 8)
						count_comparisons_8 += targetcount;
					else
						count_comparisons_16 += targetcount;

					for (unsigned long t = 0; t < targetcount; t++) {
						unsigned diff = diffs[t];

						if (diff <= (unsigned long) Property::resolution) {
							unsigned i = targetindices[t];

							/* find correct position in list */

							/* move the target (i) to the position (swarmed)
							 of the first unswarmed amplicon in the pool
							 then move the target further into the swarmed
							 but unseeded part of the list, so that the
							 swarmed amplicons are ordered by id */

							unsigned long targetampliconid = amps[i].ampliconid;
							unsigned pos = swarmed;

							while ((pos > seeded) && (amps[pos - 1].ampliconid > targetampliconid)
									&& (amps[pos - 1].generation > subseedgeneration))
								pos--;

							if (pos < i) {
								struct ampliconinfo_s temp = amps[i];
								for (unsigned j = i; j > pos; j--) {
									amps[j] = amps[j - 1];
								}
								amps[pos] = temp;
							}

							amps[pos].swarmid = swarmid;
							amps[pos].generation = subseedgeneration + 1;
							if (maxgen < amps[pos].generation)
								maxgen = amps[pos].generation;
							amps[pos].radius = subseedradius + diff;
							if (amps[pos].radius > maxradius)
								maxradius = amps[pos].radius;

							unsigned poolampliconid = amps[pos].ampliconid;
							hits[hitcount++] = poolampliconid;
							diffsum += diff;

							abundance = db->get_seqinfo(poolampliconid)->abundance;
							amplicons_copies += abundance;
							if (abundance == 1)
								singletons++;

							swarmsize++;

							swarmed++;
						}
					}
				}
			}
		}

		if (swarmsize > largestswarm)
			largestswarm = swarmsize;

		if (maxgen > maxgenerations)
			maxgenerations = maxgen;

		progress_update(seeded);
	}
	progress_done();

	/* output results */

	cluster_result * result = new cluster_result;
	result->partition_id = threadid + 1;

	char sep_amplicons = '\n'; /* usually a space */
	char sep_swarms[] = "\n\n";

	long previd = -1;
	for (unsigned long i = 0; i < db->sequences; i++) {
		member_info * member = new member_info;
		member->sequence = db->get_seqinfo(amps[i].ampliconid);
		member->generation = amps[i].generation;
		member->radius = amps[i].radius;
		member->qgram_diff = amps[i].diffestimate;
		if (amps[i].swarmid != previd) {
			result->new_cluster(amps[i].swarmid);
			fputs(sep_swarms, Property::debugfile);
		} else {
			fputc(sep_amplicons, Property::debugfile);
		}
		result->clusters.back()->cluster_members.push_back(member);
		fprintf(Property::debugfile, "%.*s", db->get_seqinfo(amps[i].ampliconid)->headeridlen, db->get_seqinfo(amps[i].ampliconid)->header);
		previd = amps[i].swarmid;
	}
	fputc('\n', Property::debugfile);

	fprintf(stderr, "\n");
	fprintf(stderr, "Number of swarms:  %lu\n", swarmid);
	fprintf(stderr, "Largest swarm:     %lu\n", largestswarm);
	fprintf(stderr, "Max generations:   %lu\n", maxgenerations);
	fprintf(stderr, "\n");
	fprintf(stderr, "Estimates:         %lu\n", estimates);
	fprintf(stderr, "Searches:          %lu\n", searches);
	fprintf(stderr, "\n");
	fprintf(stderr, "Comparisons (8b):  %lu (%.2lf%%)\n", count_comparisons_8,
			(200.0 * count_comparisons_8 / db->sequences / (db->sequences + 1)));
	fprintf(stderr, "Comparisons (16b): %lu (%.2lf%%)\n", count_comparisons_16,
			(200.0 * count_comparisons_16 / db->sequences / (db->sequences + 1)));
	fprintf(stderr, "Comparisons (tot): %lu (%.2lf%%)\n", count_comparisons_8 + count_comparisons_16,
			(200.0 * (count_comparisons_8 + count_comparisons_16) / db->sequences / (db->sequences + 1)));

	delete (qgramdiffs);
	delete (qgramamps);
	delete (qgramindices);
	delete (hits);
	delete (alignlengths);
	delete (diffs);
	delete (scores);
	delete (targetindices);
	delete (targetampliconids);
	delete (amps);

	return result;
}

