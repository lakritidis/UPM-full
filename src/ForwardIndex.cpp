#include "ForwardIndex.h"

/// Default Constructor
forwardIndex::forwardIndex() {
	this->tokens_lexicon = NULL;
	this->tokens_index = NULL;
	this->units_lexicon = NULL;
	this->combinations_lexicon = NULL;
	this->num_alloc_products = 0;
	this->num_products = 0;
	this->products = NULL;
	this->stats = NULL;
	this->num_alloc_tokens = 0;
	this->tokens = NULL;
	this->num_vendors = 0;
	this->num_alloc_vendors = 0;
	this->vendor_ids = NULL;
}

/// Constructor 2
forwardIndex::forwardIndex(uint32_t size) {
	this->tokens_lexicon = new TokensLexicon(1048576);
	this->tokens_index = NULL;

	this->units_lexicon = new TokensLexicon(256);
	this->units_lexicon->load_units();

	this->combinations_lexicon = new CombinationsLexicon(16 * 1048576);

	this->stats = new statistics();

	this->num_alloc_products = size;
	this->num_products = 0;
	this->products = new Product * [size];

	this->num_alloc_tokens = 12;
	this->tokens = (struct token **)malloc(this->num_alloc_tokens * sizeof(struct token *));
	for (uint32_t i = 0; i < this->num_alloc_tokens; i++) {
		this->tokens[i] = (struct token *)malloc(sizeof(struct token));
		this->tokens[i]->len = 10;
		this->tokens[i]->t = (char *)malloc(this->tokens[i]->len * sizeof(char));
	}

	this->num_alloc_vendors = 12;
	this->num_vendors = 0;
	this->vendor_ids = (uint32_t *)malloc(this->num_alloc_vendors * sizeof(uint32_t));
}

/// Destructor: Deallocate all used memory resources
forwardIndex::~forwardIndex() {
	/// Delete the products array
	if (this->num_products > 0) {
		for (uint32_t i = 0; i < this->num_products; i++) {
			delete this->products[i];
		}
		delete [] this->products;
	}

	/// Delete the lexicon data structure
	if (this->combinations_lexicon) {
		delete this->combinations_lexicon;
	}

	/// Delete the tokens lexicon data structure
	if (this->tokens_lexicon) {
		delete this->tokens_lexicon;
	}

	/// Delete the tokens index data structure
	if (this->tokens_index) {
		delete [] this->tokens_index;
	}

	/// Delete the units lexicon data structure
	if (this->units_lexicon) {
		delete this->units_lexicon;
	}

	/// Delete the statistics
	if (this->stats) {
		delete this->stats;
	}

	/// Delete the temporary tokens storage
	if (this->tokens) {
		for (uint32_t i = 0; i < this->num_alloc_tokens; i++) {
			if (this->tokens[i]) {
				free(this->tokens[i]->t);
				free(this->tokens[i]);
			}
		}
		free(this->tokens);
	}

	/// Delete the vendor IDs array
	if (this->vendor_ids) {
		free(this->vendor_ids);
	}
}

/// Insert a token into the member tokens structure
/// Sem = 1 (No information), Sem = 2 (Technical Spec/Attribute), Sem = 3 (Possible Model)
void forwardIndex::insert_token(uint32_t *i, uint32_t l, char *buf, uint32_t *num_mixed) {

	/// In case the token exists, return (benefits both speed and precision)
	for (uint32_t y = 0; y < *i; y++) {
		if (strcmp(this->tokens[y]->t, buf) == 0) {
			return;
		}
	}

	/// In case the token is a measurement unit, check the previous token prev_t: In case prev_t
	/// is numeric, concatenate this token with the previous one. Otherwise, proceed normally.
	if (*i > 0) {
		if (this->units_lexicon->search(buf)) {
			if (this->tokens[*i - 1]->typ == 2) {
				if (l + strlen(this->tokens[*i - 1]->t) >= this->tokens[*i - 1]->len) {
					this->tokens[*i - 1]->len = l + strlen(this->tokens[*i - 1]->t) + 1;
					this->tokens[*i - 1]->t = (char *)realloc(this->tokens[*i - 1]->t, this->tokens[*i - 1]->len * sizeof(char));
				}

				uint16_t plen = strlen(this->tokens[*i - 1]->t);

				strcat(this->tokens[*i - 1]->t, buf);
				this->tokens[*i - 1]->t[plen + l] = 0;

				/// Recompute the ID of the previous token and change type and semantics
				this->tokens[*i - 1]->ht_entry = this->tokens_lexicon->insert(this->tokens[*i - 1]->t, 3, 2);
				this->tokens[*i - 1]->id = this->tokens[*i - 1]->ht_entry->get_id();

				this->tokens[*i - 1]->typ = 3;
				this->tokens[*i - 1]->sem = 2;
				return;
			}
		}
	}

	/// In case we do not have more space to store the token, expand the space
	if (*i >= this->num_alloc_tokens) {
		uint32_t temp = this->num_alloc_tokens, x = 0;
		this->num_alloc_tokens *= 2;
		this->tokens = (struct token **)realloc(this->tokens, this->num_alloc_tokens * sizeof(struct token *));
		for (x = temp; x < this->num_alloc_tokens; x++) {
			this->tokens[x] = (struct token *)malloc(sizeof(struct token));
			this->tokens[x]->len = 10;
			this->tokens[x]->t = (char *)malloc(this->tokens[x]->len * sizeof(char));
		}
	}

	if (l >= this->tokens[*i]->len) {
		this->tokens[*i]->len = l + 1;
		this->tokens[*i]->t = (char *)realloc(this->tokens[*i]->t, this->tokens[*i]->len * sizeof(char));
	}

	/// Determine the token type. Check the first character...
	if (buf[0] >= 48 && buf[0] <= 57) {
		this->tokens[*i]->typ = 2;
		this->tokens[*i]->sem = 4;
	} else {
		this->tokens[*i]->typ = 1;
		this->tokens[*i]->sem = 1;
	}

	/// ... and the rest characters
	for (uint32_t y = 1; y < l; y++) {
		if ((buf[y] >= 48 && buf[y] <= 57) || buf[y] == 44 || buf[y] == 46) {
			if (this->tokens[*i]->typ == 1) {
				this->tokens[*i]->typ = 3;
				if (*num_mixed == 0) {
					this->tokens[*i]->sem = 3;
				} else {
					this->tokens[*i]->sem = 5;
				}
				(*num_mixed)++;
			}
		} else {
			if (this->tokens[*i]->typ == 2) {
				this->tokens[*i]->typ = 3;
				if (*num_mixed == 0) {
					this->tokens[*i]->sem = 3;
				} else {
					this->tokens[*i]->sem = 5;
				}
				(*num_mixed)++;
			}
		}
	}

	/// Check if a mixed token is a possible technical spec. Check if the token final characters are
	/// equal to a measurement unit
	if (this->tokens[*i]->typ == 3) {
		char unit[32];
		uint32_t n = 0, es = 0;
		if (buf[0] >= 48 && buf[0] <= 57) {
			for (uint32_t y = 1; y < l; y++) {
				if ((buf[y] >= 48 && buf[y] <= 57) || buf[y] == 44 || buf[y] == 46) {
					if (n > 0) {
						unit[0] = 0;
						break;
					}
				} else {
					if (n == 0) {
						es = y;
					}
					unit[n++] = buf[y];
				}
			}
			unit[n] = 0;

			/// In case a measurement unit is identified in the end, check if the previous characters
			/// are numeric
			if (this->units_lexicon->search(unit)) {
				uint32_t is_ms = 1;
				for (uint32_t y = 0; y < es; y++) {
					if ((buf[y] >= 48 && buf[y] <= 57) || buf[y] == 44 || buf[y] == 46) {
					} else {
						is_ms = 0;
						break;
					}
				}

				if (is_ms == 1) {
					this->tokens[*i]->sem = 2;
				}
			}
		}
	}

	/// In case this is the first token, identify it as a brand name (this always leads to worse results)
//	if (*i == 0) { this->tokens[*i]->sem = 6; }

	this->tokens[*i]->ht_entry = this->tokens_lexicon->insert(buf, this->tokens[*i]->typ, this->tokens[*i]->sem);
	this->tokens[*i]->id = this->tokens[*i]->ht_entry->get_id();

	strcpy(this->tokens[*i]->t, buf);
	(*i)++;
}


/// Product Title Processing:
/// 1. Tokenization
///     i) Case Folding
///    ii) Punctuation (Slashes and Hyphens)
///   iii) Check for duplicates
///    iv) Check for measurement units
///     v) Determine token type (1. Normal, 2. Numeric (possible technical spec), 3. Mixed (possible model descriptor))
///    vi) Determine token semantics (1. Unknown, 2. Technical Spec, 3. Model Descriptor
void forwardIndex::process_title(uint32_t pid, uint32_t vid, uint32_t tlen, char *t, uint32_t clen, char *c) {
	uint32_t i = 0, j = 0, x = 0, y = 0, split = 0, num_mixed = 0, num_tokens = 0, max_num_tokens = 0;
	double distance = 0.0;
	char buf[1024], buf2[1024], combination[128];
	class Combination *res;

	if (ALGORITHM == 1) {
		max_num_tokens = 2 * NUM_COMBINATIONS;
	} else if (ALGORITHM == 2) {
		for (i = 0; i < tlen; i++) {
			if (t[i] == '/' || t[i] == '-') {
				num_tokens++;
				split = 1;
			} else if (t[i] == ' ') {
				num_tokens++;
				if (split == 1) {
					num_tokens++;
					split = 0;
				}
			}
		}
		num_tokens++;
		max_num_tokens = (num_tokens / 2) + 1;
		if (max_num_tokens < NUM_COMBINATIONS) {
			max_num_tokens = NUM_COMBINATIONS;
		}
	} else {
		max_num_tokens = MAX_TOKENS;
	}

	max_num_tokens = MAX_TOKENS;

	/// Phase 1: Tokenization - Initially split the title into tokens
	num_tokens = 0;
	for (i = 0; i < tlen; i++) {
		if (t[i] == '/' || t[i] == '-') {
			buf[x++] = t[i];
			buf2[y] = 0;
			this->insert_token(&num_tokens, y, buf2, &num_mixed);
			if (num_tokens >= max_num_tokens) { break; }
			y = 0;
			split = 1;

		} else if (t[i] == ' ') {
			buf[x] = 0;
			this->insert_token(&num_tokens, x, buf, &num_mixed);
			if (num_tokens >= max_num_tokens) { break; }

			if (split == 1) {
				buf2[y] = 0;
				this->insert_token(&num_tokens, y, buf2, &num_mixed);
				if (num_tokens >= max_num_tokens) { break; }
				split = 0;
			}
			x = 0;
			y = 0;

		} else {
			/// Case folding
			if (t[i] >= 65 && t[i] <= 90) {
				buf[x++] = t[i] + 32;
				buf2[y++] = t[i] + 32;
			} else {
				buf[x++] = t[i];
				buf2[y++] = t[i];
			}
		}
	}
	buf[x] = 0;
	this->insert_token(&num_tokens, x, buf, &num_mixed);

	if (num_tokens < 2) { return; }

	uint32_t found = 0;
	for (i = 0; i < this->num_vendors; i++) {
		if (this->vendor_ids[i] == vid) {
			found = 1;
			break;
		}
	}

	if (found == 0) {
		this->vendor_ids[this->num_vendors++] = vid;
		if (this->num_vendors >= this->num_alloc_vendors) {
			this->num_alloc_vendors *= 2;
			this->vendor_ids = (uint32_t *)realloc(this->vendor_ids, this->num_alloc_vendors * sizeof(uint32_t));
		}
	}
/*
	printf("\n========================\n%d. Title: %s --- Tokens: %d\n\n", pid, t, num_tokens);
	for (i = 0; i < num_tokens; i++) {
		printf("Token %d: %s - ID: %d - Type: %d - Semantics: %d\n", i + 1,
			this->tokens[i]->t, this->tokens[i]->id, this->tokens[i]->typ, this->tokens[i]->sem);
	}
	printf("=======================================\n\n");
	getchar();
*/
	/// Create an array of pointers: each pointer points to one title token
	class Token ** title_tokens = new Token * [num_tokens];

	/// Update Statistics
	this->stats->update_titles(num_tokens);

	for (i = 0; i < num_tokens; i++) {
		title_tokens[i] = this->tokens[i]->ht_entry;
	}

//	printf("Title: %s - Title IDs: |%s|\n", t, title_ids); getchar();

	/// Create a new product node into the forward index
	this->products[this->num_products] = new Product(pid, vid, num_tokens, title_tokens, clen, c);

	/// Phase 2: Create the combinations/permutations
	/// 2-combinations / 2-permutations
	x = 0;
	for (i = 0; i < num_tokens; i++) {
		for (j = i + 1; j < num_tokens; j++) {

			class Token * toks[2];
			toks[0] = title_tokens[i];
			toks[1] = title_tokens[j];
			qsort(toks, 2, sizeof(class Token *), &forwardIndex::cmp_token_id);

			sprintf(combination, "%d %d", toks[0]->get_id(), toks[1]->get_id());

			this->stats->update_combinations(2);

			distance = sqrt(pow(i, 2) + pow(j - 1, 2));

			res = this->combinations_lexicon->insert(combination, distance, 2);

			this->products[this->num_products]->insert_cluster(res);

//			printf("%d. 2-Combination: %s [%d,%d] - Dist: %5.3f)\n", ++x, combination, i, j, distance);
		}
	}

#if NUM_COMBINATIONS >= 3
	/// 3-combinations
	x = 0;
	uint32_t k = 0;
	for (i = 0; i < num_tokens; i++) {
		for (j = i + 1; j < num_tokens; j++) {
			for (k = j + 1; k < num_tokens; k++) {

				class Token * toks[3];
				toks[0] = title_tokens[i];
				toks[1] = title_tokens[j];
				toks[2] = title_tokens[k];
				qsort(toks, 3, sizeof(class Token *), &forwardIndex::cmp_token_id);

				sprintf(combination, "%d %d %d", toks[0]->get_id(), toks[1]->get_id(), toks[2]->get_id());

				this->stats->update_combinations(3);

				distance = sqrt(pow(i, 2) + pow(j - 1, 2) + pow(k - 2, 2));

				res = this->combinations_lexicon->insert(combination, distance, 3);

				this->products[this->num_products]->insert_cluster(res);
/*
				printf("(%d, %d, %d) - ", this->tokens[i]->id, this->tokens[j]->id, this->tokens[k]->id);
				printf("%d. 3-Combination: %s [%d,%d,%d] - Dist: %5.3f, Semantics: %5.3f)\n",
					++x, combination, i, j, k, distance, semantics);
				getchar();
*/
			}
		}
	}
#endif

#if NUM_COMBINATIONS >= 4
	/// 4-combinations
	x = 0;
	uint32_t l = 0;
	for (i = 0; i < num_tokens; i++) {
		for (j = i + 1; j < num_tokens; j++) {
			for (k = j + 1; k < num_tokens; k++) {
				for (l = k + 1; l < num_tokens; l++) {

					class Token * toks [4];
					toks[0] = title_tokens[i];
					toks[1] = title_tokens[j];
					toks[2] = title_tokens[k];
					toks[3] = title_tokens[l];
					qsort(toks, 4, sizeof(class Token *), &forwardIndex::cmp_token_id);

					sprintf(combination, "%d %d %d %d", toks[0]->get_id(), toks[1]->get_id(),
						toks[2]->get_id(), toks[3]->get_id());

					this->stats->update_combinations(4);

					distance = sqrt(pow(i, 2) + pow(j - 1, 2) + pow(k - 2, 2) + pow(l - 3, 2));

					res = this->combinations_lexicon->insert(combination, distance, 4);

					this->products[this->num_products]->insert_cluster(res);
/*
					printf("(%d, %d, %d, %d) - ", this->tokens[i]->id, this->tokens[j]->id,
						this->tokens[k]->id, this->tokens[l]->id);
					printf("%d. 4-Combination: %s [%d, %d, %d, %d] - Dist: %5.3f)\n",
						++x, combination, i, j, k, l, distance);
					getchar();
*/
				}
			}
		}
	}

#endif

#if NUM_COMBINATIONS >= 5
	/// 5-combinations
	x = 0;
	uint32_t m = 0;
	for (i = 0; i < num_tokens; i++) {
		for (j = i + 1; j < num_tokens; j++) {
			for (k = j + 1; k < num_tokens; k++) {
				for (l = k + 1; l < num_tokens; l++) {
					for (m = l + 1; m < num_tokens; m++) {

						class Token * toks [5];
						toks[0] = title_tokens[i];
						toks[1] = title_tokens[j];
						toks[2] = title_tokens[k];
						toks[3] = title_tokens[l];
						toks[4] = title_tokens[m];
						qsort(toks, 5, sizeof(class Token *), &forwardIndex::cmp_token_id);

						sprintf(combination, "%d %d %d %d %d", toks[0]->get_id(), toks[1]->get_id(),
							toks[2]->get_id(), toks[3]->get_id(), toks[4]->get_id());

						this->stats->update_combinations(5);

						distance = sqrt(pow(i, 2) + pow(j - 1, 2) + pow(k - 2, 2) + pow(l - 3, 2) +
							pow(m - 4, 2));

						res = this->combinations_lexicon->insert(combination, distance, 5);

						this->products[this->num_products]->insert_cluster(res);

//						printf("%d. 5-Combination: %s [%d,%d,%d,%d,%d] - Dist: %5.3f)\n",
//							++x, combination, i, j, k, l, m, distance);
					}
				}
			}
		}
	}
#endif

#if NUM_COMBINATIONS >= 6
	/// 6-combinations
	x = 0;
	uint32_t n = 0;
	for (i = 0; i < num_tokens; i++) {
		for (j = i + 1; j < num_tokens; j++) {
			for (k = j + 1; k < num_tokens; k++) {
				for (l = k + 1; l < num_tokens; l++) {
					for (m = l + 1; m < num_tokens; m++) {
						for (n = m + 1; n < num_tokens; n++) {

							class Token * toks [6];
							toks[0] = title_tokens[i];
							toks[1] = title_tokens[j];
							toks[2] = title_tokens[k];
							toks[3] = title_tokens[l];
							toks[4] = title_tokens[m];
							toks[5] = title_tokens[n];
							qsort(toks, 6, sizeof(class Token *), &forwardIndex::cmp_token_id);

							sprintf(combination, "%d %d %d %d %d %d", toks[0]->get_id(), toks[1]->get_id(),
								toks[2]->get_id(), toks[3]->get_id(), toks[4]->get_id(), toks[5]->get_id());

							this->stats->update_combinations(6);

							distance = sqrt(pow(i, 2) + pow(j - 1, 2) + pow(k - 2, 2) + pow(l - 3, 2) +
								pow(m - 4, 2) + pow(n - 5, 2));

							res = this->combinations_lexicon->insert(combination, distance, 6);

							this->products[this->num_products]->insert_cluster(res);

//							printf("%d. 6-Combination: %s [%d,%d,%d,%d,%d,%d] - Dist: %5.3f)\n",
//								++x, combination, i, j, k, l, m, n, distance);
						}
					}
				}
			}
		}
	}
#endif

#if NUM_COMBINATIONS >= 7
	/// 7-combinations
	x = 0;
	uint32_t o = 0;
	for (i = 0; i < num_tokens; i++) {
		for (j = i + 1; j < num_tokens; j++) {
			for (k = j + 1; k < num_tokens; k++) {
				for (l = k + 1; l < num_tokens; l++) {
					for (m = l + 1; m < num_tokens; m++) {
						for (n = m + 1; n < num_tokens; n++) {
							for (o = n + 1; o < num_tokens; o++) {

								class Token *toks [7];
								toks[0] = title_tokens[i];
								toks[1] = title_tokens[j];
								toks[2] = title_tokens[k];
								toks[3] = title_tokens[l];
								toks[4] = title_tokens[m];
								toks[5] = title_tokens[n];
								toks[6] = title_tokens[o];
								qsort(toks, 7, sizeof(class Token *), &forwardIndex::cmp_token_id);

								sprintf(combination, "%d %d %d %d %d %d %d", toks[0]->get_id(),
									toks[1]->get_id(), toks[2]->get_id(), toks[3]->get_id(),
									toks[4]->get_id(), toks[5]->get_id(), toks[6]->get_id());

								this->stats->update_combinations(7);

								distance = sqrt(pow(i, 2) + pow(j - 1, 2) + pow(k - 2, 2) +
									pow(l - 3, 2) + pow(m - 4, 2) + pow(n - 5, 2) + pow(o - 6, 2));

								res = this->combinations_lexicon->insert(combination, distance, 7, toks);

								this->products[this->num_products]->insert_cluster(res);

//								printf("%d. 7-Combination: %s [%d,%d,%d,%d,%d,%d,%d] - Dist: %5.3f)\n",
//									++x, combination, i, j, k, l, m, n, o, distance);
							}
						}
					}
				}
			}
		}
	}
#endif

	this->num_products++;
	if (this->num_products >= this->num_alloc_products) {
		this->num_alloc_products *= 2;
    }
    this->stats->set_num_vendors(this->num_vendors);
}

/// Reform the Tokens Lexicon: Convert the hash table (which is indexed by the token title) into
/// an array indexed by token ID. This allows fast conversion of the combinations (token IDs) into
/// combinations of string literals
void forwardIndex::reform_tokens_lexicon(uint32_t N) {
	printf("Reforming Tokens Lexicon and computing IDFs (%d tokens)...\n",
		this->tokens_lexicon->get_num_nodes());
	this->tokens_index = new Token * [ this->tokens_lexicon->get_num_nodes() + 1 ];
	this->tokens_lexicon->reform(this->tokens_index, N, this->stats);
}


/// 1. Sort the clusters by decreasing score value and declare the representative cluster.
/// 2. Create the global clusters with the same/similar products (from the representative clusters).
/// 3. Create the pairwise matchings file to evaluate the precision of the algorithm.
void forwardIndex::sort_clusters() {

//	this->combinations_lexicon->display();
//	getchar();

	class ProductsClusters * global_clusters = new ProductsClusters(1048576);

	this->stats->compute_final_values();

	for (uint32_t i = 0; i < this->num_products; i++) {
//		printf("=== Scoring Product Title: %d\n", this->products[i]->get_id());
		this->products[i]->sort_clusters(this->stats, this->tokens_index);

		global_clusters->insert(this->products[i]->get_predicted_cluster(), this->products[i], this->stats);

		/// Prepare the Product for efficient validation
		this->products[i]->prepare();
//		getchar();
	}

	printf("Running Verification Stage...\n"); fflush(NULL);

	qsort(this->vendor_ids, this->num_vendors, sizeof(uint32_t), cmp_int);

	if (ALGORITHM == 1) {
//		global_clusters->perform_verification_full(this->stats, this->vendor_ids, this->combinations_lexicon);
		global_clusters->perform_verification(this->stats, this->vendor_ids, this->combinations_lexicon);
	}

	printf("Evaluating...\n"); fflush(NULL);
//	global_clusters->display();
	global_clusters->create_pairwise_matches();

	delete global_clusters;
}


/// Display the lexicon of the forward index
void forwardIndex::display_clusters() {
	this->combinations_lexicon->display();
}

/// Display the contents of the forward index (products, combinations list)
void forwardIndex::display_products() {
	for (uint32_t i = 0; i < this->num_products; i++) {
		this->products[i]->display(this->tokens_lexicon);
	}
}

/// Display the statistics
void forwardIndex::display_statistics() {
	this->stats->get_elapsed_time();
	this->stats->display();
}
