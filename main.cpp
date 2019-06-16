/**
UPM
--------------------------------------------------------------------------------------------------
Unsupervised Products Matching via Clustering, Combinatorics and Post-Processing Verification. This
project implements the UPM and UPM+  unsupervised  algorithms  for matching products by considering
their titles only.

The code also contains the implementations of 2+2 string similarity metrics,  which can be used for
the evaluation of the performance of the two aformentioned algorithms. These metrics include cosine
similarity and Jaccard index; both in their standard form,  and in  an enhanced form which replaces
the plain token counts by IDF token weights.

This code is completely freeware and it should be used for  scientific and non-commercial purposes.

Leonidas Akritidis, 09/12/2018
**/

#include "stdio.h"
#include "stdlib.h"
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <algorithm>

/**
Set the location (i.e. absolute paths in the local filesystem) of the input dataset and the results
folder.
**/
#ifdef __linux__

//	const char BASE_PATH[] = "path-to-upm/datasets/skroutz/";
	const char BASE_PATH[] = "path-to-upm/datasets/pricerunner/";
	const char RESULTS_PATH[] = "path-to-upm/results/";

#elif _WIN32
//	const char BASE_PATH[] = "path-to-upm/datasets/skroutz/";
	const char BASE_PATH[] = "path-to-upm/datasets/pricerunner/";
	const char RESULTS_PATH[] = "path-to-upm/results/";
#endif // _WIN32


/**
DATASET parameter
--------------------------------------------------------------------------------------------------
For the PriceRunner repository the acceptable values are:
      1. cpus for CPUs,
      2. cameras for Digital Cameras,
      3. dishwashers for Dishwashers,
      4. microwaves for Microwave Ovens,
      5. mobile for Mobile Phones,
      6. refrigerators for Fridges, Freezers, and Refrigerators,
      7. tvs for Televisions,
      8. washers for Washing Machines
      9. aggregate for the aggregate dataset which includes all the above products
For the Skroutz repository  the acceptable values are:
      1. acs for Air Conditioners,
      2. batteries for Car Batteries,
      3. cookers for Cookers & Ovens,
      4. cpus for CPUs,
      5. cameras for Digital Cameras,
      6. refrigerators for Fridges, Freezers, and Refrigerators,
      7. tvs for Televisions,
      8. watches for Mens and Womens Watches
      9. aggregate for the aggregate dataset which includes all the above products
**/
#define DATASET "aggregate"

/** ALGORITHM parameter **/
char ALGORITHMS[21][50] = {
/* 0 */		"N/A",
/* 1 */		"UPM_WithVerif",
/* 2 */		"N/A",
/* 3 */		"Cosine_Similarity",
/* 4 */		"Cosine_Similarity_IDF",
/* 5 */		"N/A",
/* 6 */		"Jaccard_Index",
/* 7 */		"Jaccard_Index_IDF",
/* 8 */		"N/A",
/* 9 */		"Dice_Similarity",
/* 10 */	"Dice_Similarity_IDF",
/* 11 */	"N/A",
/* 12 */	"Edit_Similarity",
/* 13 */	"Leader_Clustering",
/* 14 */	"Leader_Clustering_WithVerif",
/* 15 */	"N/A",
/* 16 */	"N/A",
/* 17 */	"DBSCAN",
/* 18 */	"DBSCAN_WithVerif",
/* 19 */	"Hierarchical_Clustering",
/* 20 */	"Hierarchical_Clustering_WithVerif"
};

const uint32_t ALGORITHM = 1;

#define NUM_COMBINATIONS 3
#define A_PAR 1.0
#define B_PAR 1.0
#define SIMILARITY_THRESHOLD 0.5
#define MAX_TOKENS 1000
#define NUM_ZONES 7
#define DBSCAN_MINPOINTS 2

#define EFFICIENCY_TESTS
#define LOW_THRESHOLD 0
#define HIGH_THRESHOLD 9

#include "src/Statistics.cpp"

#include "src/Token.cpp"
#include "src/TokensLexicon.cpp"
#include "src/Combination.cpp"
#include "src/CombinationsLexicon.cpp"

#include "src/Product.cpp"
#include "src/Vendor.cpp"
#include "src/Cluster.cpp"
#include "src/ProductsClusters.cpp"

#include "src/ForwardIndex.cpp"
#include "src/eval/Evaluation.cpp"

int main() {
	/// UMP MATCHING ALGORITHMS
	if (ALGORITHM <= 2) {
		uint32_t product_id = 0, vendor_id = 0, nprods = 0, nread = 0;
		uint32_t title_len, cluster_len;
		char buf_1[1024], buf_2[1024], filepath[1024];

		sprintf(filepath, "%s%s", BASE_PATH, DATASET);

		FILE *fp = fopen(filepath, "rb");

		if (fp) {
			class forwardIndex * fwd_index = new forwardIndex(1048576);

			printf("Processing Titles...\n"); fflush(NULL);

			while (!feof(fp)) {

				nread = fread(&product_id, sizeof(uint32_t), 1, fp);
				if (nread == 0) {
					break;
				}

				nread = fread(&title_len, sizeof(uint32_t), 1, fp);
				nread = fread(buf_1, sizeof(char), title_len, fp);
				buf_1[title_len] = 0;
				nread = fread(&vendor_id, sizeof(uint32_t), 1, fp);
				nread = fread(&cluster_len, sizeof(uint32_t), 1, fp);
				nread = fread(buf_2, sizeof(char), cluster_len, fp);
				buf_2[cluster_len] = 0;

//				printf("== Product Data:\n\tID: %d\n\tTitle: %s\n\tVendor ID: %d\n\tReal Cluster: %s\n\n",
//					product_id, buf_1, vendor_id, buf_2); getchar();

				fwd_index->process_title(product_id, vendor_id, title_len, buf_1, cluster_len, buf_2);

				nprods++;
			}

			fclose(fp);

			fwd_index->reform_tokens_lexicon(nprods);

			printf("Predicting Clusters...\n"); fflush(NULL);

			fwd_index->sort_clusters();

//			fwd_index->display_products();
//			fwd_index->display_clusters();
			fwd_index->display_statistics();

			delete fwd_index;

			evaluate(DATASET, ALGORITHMS[ALGORITHM], 0);

		} else {
			printf("Dataset Input File %s not found...\n", filepath);
			return -1;
		}

	/// STRING SIMILARITY METRICS
	} else {

		evaluate_metric();
#ifndef EFFICIENCY_TESTS
		char res_file[100];
		struct plot {
			double th;
			double f1;
		} plots [10];

		for (uint32_t i = LOW_THRESHOLD + 1; i < HIGH_THRESHOLD + 1; i++) {
			plots[i].th = 0;
			plots[i].f1 = 0;
		}

		for (uint32_t i = LOW_THRESHOLD + 1; i < HIGH_THRESHOLD + 1; i++) {
			sprintf(res_file, "%s%d", ALGORITHMS[ALGORITHM], i);
			plots[i - 1].th = (double)i / 10;
			plots[i - 1].f1 = evaluate(DATASET, res_file, (double)i/10);
		}

		printf("\n\nPlot:\n");
		for (uint32_t i = LOW_THRESHOLD; i < HIGH_THRESHOLD; i++) {
			printf("(%2.1f,%4.3f)", plots[i].th, plots[i].f1);
		}
#endif
	}
	printf("\n\n");
	return 0;
}
