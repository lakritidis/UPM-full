#include "Entity.cpp"
#include "EntityVendor.cpp"
#include "EntitiesCluster.cpp"
#include "ClustersSimilarity.cpp"
#include "Entities.cpp"

/// Factorial
uint32_t factorial(uint32_t n, uint32_t k) {
	uint32_t ret = 1;
	for (uint32_t i = 0; i < k; i++) {
		ret *= (n - i);
	}
	return ret;
}

/// Comparison callback function for QuickSort
int cmp_matches(const void * a, const void * b) {
	struct match *x = *(struct match **)a;
	struct match *y = *(struct match **)b;

	if (x->e1_id > y->e1_id) return 1;
	if (x->e1_id == y->e1_id) { return x->e2_id - y->e2_id; }
	if (x->e1_id < y->e1_id) return -1;

	return 0;
}


/// Evaluation of the proposed algorithm
double evaluate(const char * ds, char * f, double sim_threshold) {

	printf("\nStarting evaluation...\n"); fflush(NULL);
	char filepath[1024], cfilepath[1024];

	sprintf(filepath, "%s%s_matches", BASE_PATH, ds);
	sprintf(cfilepath, "%s%s_%s_matches", RESULTS_PATH, ds, f);

	uint32_t num_alloc_corr_matches = 65536, num_alloc_algo_matches = 65536;
	uint32_t num_corr_matches = 0, num_algo_matches = 0;
	uint32_t i = 0, j = 0, nread = 0, len = 0, num = 0, eloop = 0;
	char buf[1024];

	struct match ** corr_matches = (struct match **)malloc(num_alloc_corr_matches * sizeof(struct match *));
	struct match ** algo_matches = (struct match **)malloc(num_alloc_algo_matches * sizeof(struct match *));

	/// Read the correct matches
	FILE *correct_matches_file = fopen(filepath, "rb");
	if (correct_matches_file) {
		while (!feof(correct_matches_file)) {
			nread = fread(&len, sizeof(uint32_t), 1, correct_matches_file);
			if (nread == 0) {
				break;
			}
			nread = fread(buf, sizeof(char), len, correct_matches_file);
			buf[len] = 0;
			nread = fread(&num, sizeof(uint32_t), 1, correct_matches_file);

			eloop = factorial(num, 2) / 2;

			for (i = 0; i < eloop; i++) {
				corr_matches[num_corr_matches] = (struct match *)malloc(sizeof(struct match));

				nread = fread(&corr_matches[num_corr_matches]->e1_id, sizeof(uint32_t), 1, correct_matches_file);
				nread = fread(&corr_matches[num_corr_matches]->e2_id, sizeof(uint32_t), 1, correct_matches_file);

				num_corr_matches++;
				if (num_corr_matches >= num_alloc_corr_matches) {
					num_alloc_corr_matches *= 2;
					corr_matches = (struct match **)realloc(corr_matches, num_alloc_corr_matches * sizeof(struct match *));
				}
			}
		}
		fclose(correct_matches_file);
	} else {
		printf("error opening correct matches file %s\n", filepath);
		exit(-1);
	}
	printf("Read %d correct matches (%d allocated)\n", num_corr_matches, num_alloc_corr_matches);
	qsort(corr_matches, (size_t)num_corr_matches, sizeof(match *), cmp_matches);

	/// Read the algorithm matches
	FILE *algorithm_matches_file = fopen(cfilepath, "rb");
	if (algorithm_matches_file) {
		while (!feof(algorithm_matches_file)) {
			nread = fread(&len, sizeof(uint32_t), 1, algorithm_matches_file);
			if (nread == 0) {
				break;
			}
			nread = fread(buf, sizeof(char), len, algorithm_matches_file);
			buf[len] = 0;
			nread = fread(&num, sizeof(uint32_t), 1, algorithm_matches_file);

//			printf("%d. %s (num: %d)\n", len, buf, num);
//			getchar();
			if (ALGORITHM <= 2) {
				eloop = factorial(num, 2) / 2;
			} else {
				eloop = num;
			}

			for (i = 0; i < eloop; i++) {
				algo_matches[num_algo_matches] = (struct match *)malloc(sizeof(struct match));

				nread = fread(&algo_matches[num_algo_matches]->e1_id, sizeof(uint32_t), 1, algorithm_matches_file);
				nread = fread(&algo_matches[num_algo_matches]->e2_id, sizeof(uint32_t), 1, algorithm_matches_file);

				num_algo_matches++;
				if (num_algo_matches >= num_alloc_algo_matches) {
					num_alloc_algo_matches *= 2;
					algo_matches = (struct match **)realloc(algo_matches, num_alloc_algo_matches * sizeof(struct match *));
				}
			}
		}
		fclose(algorithm_matches_file);
	} else {
		printf("error opening algorithm matches file: %s\n", cfilepath);
		exit(-1);
	}
	printf("Read %d algorithm matches (%d allocated)\n", num_algo_matches, num_alloc_algo_matches);
	qsort(algo_matches, (size_t)num_algo_matches, sizeof(struct match *), cmp_matches);

//	for (i = 0; i < num_algo_matches; i++) {
//		printf("%d. (%d,%d)\n", i, algo_matches[i]->p1_id, algo_matches[i]->p2_id);
//	}

	printf("Computing Correct Matches...\n"); fflush(NULL);
	uint32_t correct_algo_matches = 0;

	i = 0;
	j = 0;
	while (i < num_corr_matches) {
//		printf("Checking match %d (%d,%d)...\n", i, corr_matches[i]->p1_id, corr_matches[i]->p2_id);
//		getchar();

		while (algo_matches[j]->e1_id < corr_matches[i]->e1_id || (algo_matches[j]->e1_id == corr_matches[i]->e1_id && algo_matches[j]->e2_id <= corr_matches[i]->e2_id)) {
//			printf("\tPosition: %d (%d,%d)\n", j, algo_matches[j]->p1_id, algo_matches[j]->p2_id);
			if ((corr_matches[i]->e1_id == algo_matches[j]->e1_id && corr_matches[i]->e2_id == algo_matches[j]->e2_id) ||
				(corr_matches[i]->e1_id == algo_matches[j]->e2_id && corr_matches[i]->e2_id == algo_matches[j]->e1_id) ) {
					correct_algo_matches++;
//					printf("\tCorrect Match %d: [%d, %d] - [%d, %d]\n\n", correct_algo_matches,
//						corr_matches[i]->p1_id, corr_matches[i]->p2_id, algo_matches[j]->p1_id, algo_matches[j]->p2_id);

					j++;
					break;
			}
			j++;
			if (j >= num_algo_matches) {
				break;
			}
		}
		i++;
		if (j >= num_algo_matches) {
			break;
		}
	}

	double precision = 0.0, recall = 0.0, f1 = 0.0;
	precision = (double)correct_algo_matches / (double)num_algo_matches;
	recall = (double)correct_algo_matches / (double)num_corr_matches;
	f1 = 2 * precision * recall / (precision + recall);

	printf("===============================================================================\n");
	if (ALGORITHM == 0 || ALGORITHM == 1 || ALGORITHM == 2) {
		printf("==== RESULTS (Algorithm: %s, Dataset: %s, K=%d, a=%3.1f, b=%3.1f)\n",
			ALGORITHMS[ALGORITHM], DATASET, NUM_COMBINATIONS, A_PAR, B_PAR);
	} else {
		printf("==== RESULTS (Algorithm: %s, Dataset: %s, Sim Threshold: %4.2f)\n",
			ALGORITHMS[ALGORITHM], DATASET, sim_threshold);
	}
	printf("===============================================================================\n");
	printf("Algorithm Correct Matches:\t%d\n", correct_algo_matches);
	printf("Algorithm Total Matches:\t%d\n", num_algo_matches);
	printf("Given Correct Matches:\t\t%d\n", num_corr_matches);
	printf("-------------------------------------------------------------------------------\n");
	printf("Precision:\t\t\t%4.3f\n", precision);
	printf("Recall:\t\t\t\t%4.3f\n", recall);
	printf("F1 Metric:\t\t\t%4.3f\n", f1);
	printf("===============================================================================\n\n");

	/// Deallocate the resources
	for (i = 0; i < num_corr_matches; i++) {
		free(corr_matches[i]);
	}
	free(corr_matches);

	for (i = 0; i < num_algo_matches; i++) {
		free(algo_matches[i]);
	}
	free(algo_matches);

	return f1;
}


/// ///////////////////////////////////////////////////////////////////////////////////////////////
/// Evaluation of a string similarity metric
void evaluate_metric() {
	char filepath[1024];
	strcpy(filepath, BASE_PATH);
	strcat(filepath, DATASET);
	filepath[strlen(BASE_PATH) + strlen(DATASET)] = 0;

	class Entities * products = new Entities(1000000);

	printf("Reading Input Dataset '%s'... ", DATASET); fflush(NULL);
	products->read_from_file(filepath);

	printf("%d entities found.\nPreparing output for '%s'...\n",
			products->get_num_entities(), DATASET); fflush(NULL);
	products->prepare_output();

#ifdef __linux__
	struct timespec ts, te;
	clock_gettime(CLOCK_REALTIME, &ts);
#endif

	printf("Preparing data for '%s'...\n", DATASET); fflush(NULL);
	products->prepare();

	printf("Running '%s' on '%s'...\n", ALGORITHMS[ALGORITHM], DATASET); fflush(NULL);

	if (ALGORITHM == 3) {
		products->cosine_sim();
	} else if (ALGORITHM == 4) {
		products->cosine_sim_idf();
	} else if (ALGORITHM == 5) {
		products->cosine_sim_fuzzy();
	} else if (ALGORITHM == 6) {
		products->jaccard_index();
	} else if (ALGORITHM == 7) {
		products->jaccard_index_idf();
	} else if (ALGORITHM == 8) {
		products->jaccard_index_fuzzy();
	} else if (ALGORITHM == 9) {
		products->dice_sim();
	} else if (ALGORITHM == 10) {
		products->dice_sim_idf();
	} else if (ALGORITHM == 11) {
		products->dice_sim_fuzzy();
	} else if (ALGORITHM == 12) {
		products->edit_sim();
	} else if (ALGORITHM == 13) {
		products->leader_clustering(false);
	} else if (ALGORITHM == 14) {
		products->leader_clustering(true);
	} else if (ALGORITHM == 15) {
		products->spectral_clustering(0);
	} else if (ALGORITHM == 16) {
		products->spectral_clustering(1);
	} else if (ALGORITHM == 17) {
		products->dbscan(0);
	} else if (ALGORITHM == 18) {
		products->dbscan(1);
	} else if (ALGORITHM == 19) {
		products->hierarchical_clustering(0);
	} else if (ALGORITHM == 20) {
		products->hierarchical_clustering(1);
	}

	products->finalize();
	delete products;

#ifdef __linux__
	clock_gettime(CLOCK_REALTIME, &te);
	double duration = (te.tv_sec - ts.tv_sec) + (double)(te.tv_nsec - ts.tv_nsec) / (double)(1000000000);
	printf("Finished after %10.3f\n", duration); fflush(NULL);
#elif _WIN32
	printf("Could not compute duration\n"); fflush(NULL);
#endif
}
