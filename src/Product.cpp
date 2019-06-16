#include "Product.h"

/// Default Constructor
Product::Product() {
	this->id = 0;
	this->num_tokens = 0;
	this->tokens = NULL;
	this->vendor_id = 0;
	this->num_combinations = 0;
	this->combinations = NULL;
	this->real_cluster = NULL;
	this->predicted_cluster = NULL;
	this->idf = 0.0;
}

/// Constructor 2
Product::Product(uint32_t p_id, uint32_t v_id, uint32_t nt, class Token **t, uint32_t cl, char *c) {

	this->id = p_id;
	this->vendor_id = v_id;

	this->num_tokens = nt;
	this->tokens = t;

	if (cl > 0) {
		this->real_cluster = new char[cl + 1];
		strcpy(this->real_cluster, c);
	}

	this->num_combinations = 0;

#if NUM_COMBINATIONS == 2
	uint32_t nc = factorial_frac(nt, 2) / 2;
#elif NUM_COMBINATIONS == 3
	uint32_t nc = factorial_frac(nt, 2) / 2 + factorial_frac(nt, 3) / 6;
#elif NUM_COMBINATIONS == 4
	uint32_t nc = factorial_frac(nt, 2) / 2 +
		factorial_frac(nt, 3) / 6 +
		factorial_frac(nt, 4) / 24;
#elif NUM_COMBINATIONS == 5
	uint32_t nc = factorial_frac(nt, 2) / 2 +
		factorial_frac(nt, 3) / 6 +
		factorial_frac(nt, 4) / 24 +
		factorial_frac(nt, 5) / 120;
#elif NUM_COMBINATIONS == 6
	uint32_t nc = factorial_frac(nt, 2) / 2 +
		factorial_frac(nt, 3) / 6 +
		factorial_frac(nt, 4) / 24 +
		factorial_frac(nt, 5) / 120 +
		factorial_frac(nt, 6) / 720;
#elif NUM_COMBINATIONS == 7
	uint32_t nc = factorial_frac(nt, 2) / 2 +
		factorial_frac(nt, 3) / 6 +
		factorial_frac(nt, 4) / 24 +
		factorial_frac(nt, 5) / 120 +
		factorial_frac(nt, 6) / 720 +
		factorial_frac(nt, 7) / 5040;
#endif

	if (nc > 0) {
		this->combinations = new Combination * [nc];
	}
	this->predicted_cluster = NULL;
	this->idf = 0.0;
}

/// Destructor
Product::~Product() {
	if (this->num_tokens > 0) {
		delete [] this->tokens;
	}

	if (this->combinations) {
		delete [] this->combinations;
	}

	if (this->real_cluster) {
		delete [] this->real_cluster;
	}
}

/// Standard factorial calculation
uint32_t Product::factorial(int32_t x) {
	uint32_t factorial = 1;
	for (int32_t i = 2; i <= x; i++) {
		factorial *= i;
	}
	return factorial;
}

/// n! / (n-k)! calculation
uint32_t Product::factorial_frac(uint32_t n, uint32_t k) {
	uint32_t frac = 1;
	for (uint32_t i = 0; i < k; i++) {
		frac *= (n - i);
	}
	return frac;
}

/// Sort the clusters by decreasing score value
void Product::sort_clusters(class statistics * stats, class Token ** tindex) {
	double max_score = 0.0, score = 0.0;
	for (uint32_t i = 0; i < this->num_combinations; i++) {
		score = this->combinations[i]->compute_score(stats, tindex);
		if (score >= max_score) {
			max_score = score;
			this->predicted_cluster = this->combinations[i];
		}
	}
//	printf("Max Score: %5.3f\n", max_score);
}

/// Prepare a product for efficient verification stage: Sort the tokens in increasing token_id
/// order and pre-compute the accumulated IDFs
void Product::prepare() {
	qsort(this->tokens, this->num_tokens, sizeof(class Token *), cmp_tokens);

	this->idf = 0.0;
	for (uint32_t i = 0; i < this->num_tokens; i++) {
		this->idf += this->tokens[i]->get_idf() * this->tokens[i]->get_idf();
	}
}

/// Insert a cluster into the combinations array
void Product::insert_cluster(class Combination * c) {
	this->combinations[this->num_combinations++] = c;
}

/// Display function
void Product::display(class TokensLexicon *h) {
	uint32_t display_combinations = 1;

	if (this->num_combinations > 100) {
		display_combinations = 100;
	} else {
		display_combinations = this->num_combinations;
	}

	char title[1024];
	title[0] = 0;
	for (uint32_t i = 0; i < this->num_tokens; i++) {
		sprintf(title + strlen(title), "%s ", this->tokens[i]->get_str());
	}

	printf("ID: %d - Title: %s - Real Cluster: %s - Candidate Clusters: %d\n",
		this->id, title, this->real_cluster, this->num_combinations);

	for (uint32_t i = 0; i < display_combinations; i++) {
		this->combinations[i]->display(h, title);
		if (this->combinations[i] == this->predicted_cluster) {
			printf("\t(*)");
		}
		printf("\n");
	}
	printf("\n");
	getchar();
}


/// Accessors
inline uint32_t Product::get_id() { return this->id; }
inline uint32_t Product::get_vendor_id() { return this->vendor_id; }
inline uint32_t Product::get_num_tokens() { return this->num_tokens; }
inline uint32_t Product::get_num_combinations() { return this->num_combinations; }
inline class Combination * Product::get_predicted_cluster() { return this->predicted_cluster; }
inline class Combination * Product::get_combination(uint32_t v) { return this->combinations[v]; }
inline double Product::get_idf() { return this->idf; }

inline void Product::get_title(char *title) {
	title[0] = 0;
	for (uint32_t i = 0; i < this->num_tokens; i++) {
		sprintf(title + strlen(title), "%s ", this->tokens[i]->get_str());
	}
}

inline class Token * Product::get_token(uint32_t v) { return this->tokens[v]; }

/// Mutators
inline void Product::set_predicted_cluster(uint32_t v) {
	this->predicted_cluster = this->combinations[v];
}
