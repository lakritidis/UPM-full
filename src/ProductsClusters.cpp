#include "ProductsClusters.h"

/// Default Constructor
ProductsClusters::ProductsClusters() {
}

/// Constructor 2
ProductsClusters::ProductsClusters(uint32_t size) {
	this->hash_table = new Cluster * [size];

	for (uint32_t i = 0; i < size; i++) {
		this->hash_table[i] = NULL;
	}

	this->num_nodes = 0;
	this->num_chains = 0;
	this->num_slots = size;
	this->mask = size - 1;
	this->mem = size * sizeof(class ProductsCluster *) + sizeof(ProductsClusters);
}

/// Destructor
ProductsClusters::~ProductsClusters() {
	class Cluster * q;
	for (uint32_t i = 0; i < this->num_slots; i++) {
		while (this->hash_table[i] != NULL) {
			q = this->hash_table[i]->get_next();
			delete this->hash_table[i];
			this->hash_table[i] = q;
		}
	}
	delete [] this->hash_table;
}

/// Search for an element in the hash table

class Cluster * ProductsClusters::search(char * t) {
	/// Find the hash value of the input term
	uint32_t HashValue = KazLibHash(t) & this->mask;

	/// Now search in the hash table to check whether this term exists or not
	if (this->hash_table[HashValue] != NULL) {
		class Cluster *q;

		/// Traverse the linked list that represents the chain.
		for (q = this->hash_table[HashValue]; q != NULL; q = q->get_next()) {
			if (strcmp(q->get_str(), t) == 0) {
				return q; /// Return and exit
			}
		}
	}
	return NULL;
}

/// Insert an element into the hash table
void ProductsClusters::insert(class Combination * t, class Product * prod, class statistics * stats) {
	uint32_t nprods = 0;

	/// Find the hash value of the input cluster
	uint32_t HashValue = KazLibHash(t->get_str()) & this->mask;

	/// Now search in the hash table to check whether this term exists or not
	if (this->hash_table[HashValue] != NULL) {
		class Cluster *q;

		/// Traverse the linked list that represents the chain.
		for (q = this->hash_table[HashValue]; q != NULL; q = q->get_next()) {
			if (strcmp(q->get_str(), t->get_str()) == 0) {

				nprods = q->insert_product(prod);

				stats->update_clusters(nprods);

				return; /// Return and exit
			}
		}
	} else {
		this->num_chains++;
	}

	this->num_nodes++;

	/// Create a new record and re-assign the linked list's head
	class Cluster * record = new Cluster( t );
	record->insert_product(prod);

	stats->increase_clusters();

	/// Reassign the chain's head
	record->set_next(this->hash_table[HashValue]);
	this->hash_table[HashValue] = record;

	this->mem += sizeof(Cluster);
}

/// Create a new cluster and insert a product there (called from the verification stage when a new cluster is created)
class Cluster * ProductsClusters::insert(char * t, class Product * prod, class statistics * stats, class CombinationsLexicon *clex) {
	/// Find the hash value of the input cluster
	uint32_t HashValue = KazLibHash(t) & this->mask;

	this->num_nodes++;

	class Combination * new_comb = clex->insert(t, 0.0, 0);

	/// Create a new record and re-assign the linked list's head
	class Cluster * record = new Cluster( new_comb );
	record->insert_product(prod);
	record->set_rep_product(prod);

	stats->increase_clusters();

	/// Reassign the chain's head
	record->set_next(this->hash_table[HashValue]);
	this->hash_table[HashValue] = record;

	this->mem += sizeof(Cluster);
	return record;
}

/// Display the Hash table elements to stdout
void ProductsClusters::display() {
	class Cluster * q;

	/// Iterate down the Hash Table and display non NULL keys.
	for (uint32_t i = 0; i < this->num_slots; i++) {
		if (this->hash_table[i] != NULL) {
			for (q = this->hash_table[i]; q != NULL; q = q->get_next()) {
				q->display();
			}
		}
	}
}

/// Create the pairwise matches file to evaluate the precision of the algorithm.
void ProductsClusters::create_pairwise_matches() {
	char filepath[1024];

	sprintf(filepath, "%s%s_%s_matches", RESULTS_PATH, DATASET, ALGORITHMS[ALGORITHM]);
	FILE *fp = fopen(filepath, "wb");

	if (fp) {
		class Cluster * q;

		/// Iterate through the Hash Table and display non NULL keys.
		for (uint32_t i = 0; i < this->num_slots; i++) {
			if (this->hash_table[i] != NULL) {
				for (q = this->hash_table[i]; q != NULL; q = q->get_next()) {
					if (q->get_active() == 1) {
						q->create_pairwise_matches(fp);
					}
				}
			}
		}

		fclose(fp);
	} else {
		printf("Cound not open matchings file %s\n", filepath);
		return;
	}
}

/// Compute the cosine similarity (with idf weighting) between two Product titles
double ProductsClusters::cos_sim_idf(class Product *p1, class Product *p2) {
	double idf_1 = p1->get_idf(), idf_2 = p2->get_idf(), idf_i = 0.0, idf = 0.0;
	uint32_t it_1 = 0, it_2 = 0;

	while(it_1 < p1->get_num_tokens() && it_2 < p2->get_num_tokens()) {
		if (p1->get_token(it_1)->get_id() == p2->get_token(it_2)->get_id()) {
			idf_i += pow(p1->get_token(it_1)->get_idf(), 2);
			it_1++;
			it_2++;
		} else if (p1->get_token(it_1)->get_id() < p2->get_token(it_2)->get_id()) {
			it_1++;
		} else {
			it_2++;
		}
	}

	idf = idf_i / (sqrt(idf_1) * sqrt(idf_2));
	return idf;
}

/// Validation Stage: Refine clusters
void ProductsClusters::perform_verification_full(class statistics * stats, uint32_t *v_ids, class CombinationsLexicon *clex) {
	class Cluster * q, * r;
	uint32_t c = 0, v = 0, p = 0, tv = 0, temp_clusters_size = this->num_nodes;
	uint32_t num_vendors = stats->get_num_vendors(), num_clusters = 0, onum_clusters = 0, num_sims = 0;
	char title[1024];
	double sim = 0.0;

	class Vendor ** deleted_products = new Vendor * [num_vendors];
	for (v = 0; v < num_vendors; v++) {
		deleted_products[v] = new Vendor(v_ids[v]);
	}

	/// CLUSTER SPLIT PHASE
	printf("\t Split phase...\n"); fflush(NULL);

	/// Traverse the Hash Table and prepare the clusters (declare representative products, sort the
	/// vendors, etc.). Store the clusters into a temp array.
	class Cluster ** temp_clusters = (class Cluster **)malloc(temp_clusters_size * sizeof(class Cluster **));
	for (c = 0; c < this->num_slots; c++) {
		if (this->hash_table[c] != NULL) {
			for (q = this->hash_table[c]; q != NULL; q = q->get_next()) {
				q->prepare();
//				q->display();
				temp_clusters[num_clusters++] = q;
			}
		}
	}

	/// After the prepapation of the clusters, delete all (except 1) the products from violating vendors.
	for (c = 0; c < num_clusters; c++) {
		q = temp_clusters[c];

		for (v = 0; v < q->get_num_vendors(); v++) {
			if (q->get_vendor(v)->get_num_products() > 1) {
				tv = this->search_vendor(deleted_products, q->get_vendor(v)->get_id(), 0, num_vendors - 1);

				for (p = 1; p < q->get_vendor(v)->get_num_products(); p++) {
					deleted_products[tv]->insert_product( q->get_vendor(v)->get_product(p) );
					q->delete_product(v, p);
				}
			}
		}
	}

	/// Transform each deleted product into a singleton cluster (i.e. cluster with one product)
	onum_clusters = num_clusters;

	for (v = 0; v < num_vendors; v++) {
		for (p = 0; p < deleted_products[v]->get_num_products(); p++) {
			if (num_clusters >= temp_clusters_size) {
				temp_clusters_size *= 2;
				temp_clusters = (class Cluster **)realloc(temp_clusters, temp_clusters_size * sizeof(class Cluster **));
			}

			sprintf(title, "%s X%d", deleted_products[v]->get_product(p)->get_combination(0)->get_str(), num_clusters);

			temp_clusters[num_clusters++] = this->insert(title, deleted_products[v]->get_product(p), stats, clex);
		}
		delete deleted_products[v];
	}
	delete [] deleted_products;


	/// CLUSTER MERGE PHASE
	printf("\t Merge phase...\n"); fflush(NULL);

	/// UNFORTUNATELY WE CANNOT SUSTAIN THIS AMOUNT OF MEMORY FOR A LARGE NUMBER OF DELETED PRODUCTS
	/// AND/OR CLUSTERS (I.E. > 10^5)
	num_sims = (num_clusters - onum_clusters) * num_clusters;
	struct similarity * similarities = (struct similarity *) malloc(num_sims * sizeof(struct similarity));
	tv = 0;

	/// Compute the similarities of each recently created cluster with all clusters
	for (c = onum_clusters; c < num_clusters; c++) {
		q = temp_clusters[c];
		for (v = 0; v < num_clusters; v++) {
			r = temp_clusters[v];
//			printf("%d-%d\n", c, v); fflush(NULL);
			if (r != q) {
				if ( !r->has_vendor( q->get_rep_product()->get_vendor_id() ) ) {

					sim = this->cos_sim_idf(q->get_rep_product(), r->get_rep_product());

					if (sim >= SIMILARITY_THRESHOLD) {
						similarities[tv].c1 = v;
						similarities[tv].c2 = c;
						similarities[tv].sim = sim;
						tv++;
						if (tv >= num_sims) {
							num_sims *= 2;
							similarities = (struct similarity *) realloc
									(similarities, num_sims * sizeof(struct similarity));
						}
					}
				}
			}
		}
	}
	qsort (similarities, tv, sizeof(struct ProductsClusters::similarity), cmp_similarities);


//	for (c = 0; c < tv; c++) {
//		printf("(%d,%d - %5.3f)\n", similarities[c].c1, similarities[c].c2, similarities[c].sim);
//		getchar();
//	}

	for (c = 0; c < tv; c++) {
		q = temp_clusters[ similarities[c].c1 ];
		r = temp_clusters[ similarities[c].c2 ];

//		printf("===================================================================\n");
//		printf("== Cluster Before merging: %d\n", similarities[c].c1);
//		q->display();
//		printf("\n== will be merged with Cluster: %d\n", similarities[c].c2);
//		r->display();

		if (q->merge_with(r) == 1) {
			temp_clusters[ similarities[c].c2 ] = temp_clusters[ similarities[c].c1 ];
//			printf("== Cluster %d After merging:\n", similarities[c].c1);
//			q->display();
		} else {
//			printf("== Could not merge\n");
		}

//		getchar();
	}

	free(similarities);
	free(temp_clusters);
}


/// Alternative (Time & Memory Efficient) Validation Stage: Refine clusters
void ProductsClusters::perform_verification(class statistics * stats, uint32_t *v_ids, class CombinationsLexicon *clex) {
	class Cluster * q , * r;
	class Product * cand;
	class Vendor * u;
	uint32_t c = 0, v = 0, p = 0, t = 0, temp_clusters_size = this->num_nodes;
	uint32_t num_vendors = stats->get_num_vendors(), num_clusters = 0, onum_clusters = 0;
	double sim = 0.0, max_sim = 0.0;
	char title[1024];

	class Vendor ** evicted_vendors = new Vendor * [num_vendors];
	for (v = 0; v < num_vendors; v++) {
		evicted_vendors[v] = new Vendor(v_ids[v]);
	}

	/// CLUSTER SPLIT PHASE
	printf("\t Split phase...\n"); fflush(NULL);

	/// Traverse the Hash Table and prepare the clusters (declare representative products, sort the
	/// vendors, etc.). Store the clusters into a temp array.
	class Cluster ** temp_clusters = (class Cluster **)malloc(temp_clusters_size * sizeof(class Cluster **));
	for (c = 0; c < this->num_slots; c++) {
		if (this->hash_table[c] != NULL) {
			for (q = this->hash_table[c]; q != NULL; q = q->get_next()) {
				q->prepare();
//				q->display();
				temp_clusters[num_clusters++] = q;
			}
		}
	}

	/// After the prepapation of the clusters, delete all (except 1) the products from violating vendors.
	for (c = 0; c < num_clusters; c++) {
		q = temp_clusters[c];

		for (v = 0; v < q->get_num_vendors(); v++) {
			u = q->get_vendor(v);

			if (u->get_num_products() > 1) {
				t = this->search_vendor(evicted_vendors, u->get_id(), 0, num_vendors - 1);

				for (p = 1; p < u->get_num_products(); p++) {
					evicted_vendors[t]->insert_product( u->get_product(p) );
					q->delete_product(v, p);
				}
			}
		}
	}


	/// CLUSTER MERGE PHASE
	printf("\t Merge phase...\n"); fflush(NULL);

	/// For each cluster, find the most similar product from each vendor: If the vendor is valid and
	/// the similarity exceeds the threshold, insert the product into the cluster.
	for (c = 0; c < num_clusters; c++) {
		q = temp_clusters[c];

		for (v = 0; v < num_vendors; v++) {
			u = evicted_vendors[v];

			if ( !q->has_vendor( u->get_id() ) ) {
				max_sim = 0.0;
				t = 0;
				cand = NULL;

				for (p = 0; p < u->get_num_products(); p++) {
					if (u->get_product(p)) {
						sim = this->cos_sim_idf(q->get_rep_product(), u->get_product(p));

						if (sim > max_sim) {
							max_sim = sim;
							if (sim > SIMILARITY_THRESHOLD) {
								t = p;
								cand = u->get_product(p);
							}
						}
					}
				}

				if (cand) {
					q->insert_product(cand);
					u->delete_product(t);
				}
			}
		}
	}

	/// Take care of the rest of the evicted products which were not inserted into a cluster. That
	/// is, we will create new clusters and insert the most similar products there.
	onum_clusters = num_clusters;

	for (v = 0; v < num_vendors; v++) {
		u = evicted_vendors[v];

		for (p = 0; p < u->get_num_products(); p++) {
			if (u->get_product(p)) {
				if (v == 0) {
					if (num_clusters >= temp_clusters_size) {
						temp_clusters_size *= 2;
						temp_clusters = (class Cluster **)realloc
								(temp_clusters, temp_clusters_size * sizeof(class Cluster *));
					}

					sprintf(title, "%s X%d", u->get_product(p)->get_combination(0)->get_str(), num_clusters);

					temp_clusters[num_clusters++] = this->insert(title, u->get_product(p), stats, clex);

				} else {
					r = NULL;
					max_sim = 0.0;

					for (c = onum_clusters; c < num_clusters; c++) {
						q = temp_clusters[c];

						if (!q->has_vendor( u->get_id() )) {
							sim = this->cos_sim_idf(q->get_rep_product(), u->get_product(p));
							if (sim > max_sim) {
								max_sim = sim;
								if (sim > SIMILARITY_THRESHOLD) {
									r = q;
								}
							}
						}
					}

					if (r) {
						r->insert_product(u->get_product(p));
					} else {
						if (num_clusters >= temp_clusters_size) {
							temp_clusters_size *= 2;
							temp_clusters = (class Cluster **)realloc
								(temp_clusters, temp_clusters_size * sizeof(class Cluster *));
						}

						sprintf(title, "%s X%d", u->get_product(p)->get_combination(0)->get_str(), num_clusters);

						temp_clusters[num_clusters++] = this->insert(title, u->get_product(p), stats, clex);
					}
				}
			}
			u->delete_product(p);
		}
	}


	for (v = 0; v < num_vendors; v++) {
		delete evicted_vendors[v];
	}
	delete [] evicted_vendors;
	free(temp_clusters);
}


/// Given a (sorted) array of vendors and an ID, (binary) search for the vendor in the array.
int32_t ProductsClusters::search_vendor(class Vendor ** ven, uint32_t v_id, int32_t l, int32_t r) {
	if (r >= l) {
		int32_t mid = l + (r - l) / 2;

		if (ven[mid]->get_id() == v_id) {
			return mid;
		} else if (ven[mid]->get_id() > v_id) {
			return this->search_vendor(ven, v_id, l, mid - 1);
		} else {
			return this->search_vendor(ven, v_id, mid + 1, r);
		}
	}

    return -1;
}


/// The Hash Function
uint32_t ProductsClusters::KazLibHash (char *key) {
   static unsigned long randbox[] = {
       0x49848f1bU, 0xe6255dbaU, 0x36da5bdcU, 0x47bf94e9U,
       0x8cbcce22U, 0x559fc06aU, 0xd268f536U, 0xe10af79aU,
       0xc1af4d69U, 0x1d2917b5U, 0xec4c304dU, 0x9ee5016cU,
       0x69232f74U, 0xfead7bb3U, 0xe9089ab6U, 0xf012f6aeU,
	};

	char *str = key;
	uint32_t acc = 0;

	while (*str) {
		acc ^= randbox[(*str + acc) & 0xf];
		acc = (acc << 1) | (acc >> 31);
		acc &= 0xffffffffU;
		acc ^= randbox[((*str++ >> 4) + acc) & 0xf];
		acc = (acc << 2) | (acc >> 30);
		acc &= 0xffffffffU;
	}
	return acc;
}

uint32_t ProductsClusters::JZHash (char * key, uint32_t len) {
	/// hash = hash XOR ((left shift L bits) AND (right shift R bits) AND key value)
	uint32_t hash = 1315423911;
	uint32_t i = 0;

	for (i = 0; i < len; key++, i++) {
		hash ^= ((hash << 5) + (*key) + (hash >> 2));
	}
	return hash;
}

