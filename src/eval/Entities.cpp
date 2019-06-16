#include "Entities.h"

/// Default Constructor
Entities::Entities() {
	this->num_entities = 0;
	this->num_alloc_entities = 0;
	this->e = NULL;
	this->tokens_lexicon = NULL;
	this->v_ids = NULL;
	this->num_vendors = 0;
	for (uint32_t i = 0; i < 10; i++) {
		this->num_algo_matches[i] = 0;
	}
}

/// Constructor
Entities::Entities(uint32_t n) {
	this->num_entities = 0;
	this->num_alloc_entities = 1000000;
	this->e = new Entity * [this->num_alloc_entities]();
	this->tokens_lexicon = NULL;
	this->v_ids = NULL;
	this->num_vendors = 0;
	for (uint32_t i = 0; i < 10; i++) {
		this->num_algo_matches[i] = 0;
	}
}

/// Destructor
Entities::~Entities() {
	if (this->num_entities > 0) {
		for (uint32_t i = 0; i < this->num_entities; i++) {
			delete this->e[i];
		}
		delete [] this->e;
	}

	if (this->tokens_lexicon) {
		delete this->tokens_lexicon;
	}

	for (uint32_t i = LOW_THRESHOLD; i < HIGH_THRESHOLD; i++) {
		for (uint32_t j = 0; j < this->csim_matches[i]->num_alloc_matches; j++) {
			free(this->csim_matches[i]->matches_array[j]);
		}
		free(this->csim_matches[i]->matches_array);
		free(this->csim_matches[i]);
	}
	free(this->csim_matches);

	if (v_ids) {
		free(v_ids);
	}
}

/// Read the entities from an input file
void Entities::read_from_file(char * filepath) {
	uint32_t nread = 0, product_id = 0, len = 0, v_id = 0, cluster_len = 0, nven = 100, f = 0;
	char buf[1024], title[2048];

	this->v_ids = (uint32_t *)malloc(nven * sizeof(uint32_t));

	FILE *fp = fopen(filepath, "rb");
	if (fp) {
		while (!feof(fp)) {
			nread = fread(&product_id, sizeof(uint32_t), 1, fp);
			if (nread == 0) {
				break;
			}

			this->e[this->num_entities] = new Entity();
			this->e[this->num_entities]->set_id(product_id);
			this->e[this->num_entities]->set_index(0);

			nread = fread(&len, sizeof(uint32_t), 1, fp);
			nread = fread(title, sizeof(char), len, fp);
			title[len] = 0;

			this->e[this->num_entities]->set_text(title, len);

			nread = fread(&v_id, sizeof(uint32_t), 1, fp);
			this->e[this->num_entities]->set_vendor_id(v_id);

			this->num_entities++;

			nread = fread(&cluster_len, sizeof(uint32_t), 1, fp);
			nread = fread(buf, sizeof(char), cluster_len, fp);
			buf[cluster_len] = 0;

			f = 0;
			for (uint32_t i = 0; i < this->num_vendors; i++) {
				if (this->v_ids[i] == v_id) {
					f = 1; break;
				}
			}

			if (f == 0) {
				this->v_ids[this->num_vendors++] = v_id;
				if (this->num_vendors >= nven) {
					nven *= 2;
					this->v_ids = (uint32_t *)realloc(this->v_ids, nven * sizeof(uint32_t));
				}
			}
		}

		qsort(this->v_ids, this->num_vendors, sizeof(uint32_t), cmp_int);
		fclose(fp);
	} else {
		printf("Error Opening %s file...\n", filepath); fflush(NULL);
		exit(-1);
	}
}

/// Open the output files for writing
void Entities::prepare_output() {
	char buf[1024], null_t[5];
	uint32_t null_len = 4;
	strcpy(null_t, "null");

	this->csim_matches = (struct matches **)malloc(HIGH_THRESHOLD * sizeof(matches *));

	for (uint32_t i = LOW_THRESHOLD; i < HIGH_THRESHOLD; i++) {
		sprintf(buf, "%s%s_%s%d_matches", RESULTS_PATH, DATASET, ALGORITHMS[ALGORITHM], i + 1);
		this->output_files[i] = fopen(buf, "wb");

		fwrite(&null_len, sizeof(uint32_t), 1, this->output_files[i]);
		fwrite(&null_t, sizeof(char), null_len, this->output_files[i]);
		fwrite(&i, sizeof(uint32_t), 1, this->output_files[i]);

		this->csim_matches[i] = (struct matches *)malloc(sizeof(struct matches));
		this->csim_matches[i]->num_matches = 0;
		this->csim_matches[i]->num_alloc_matches = 1048576;
		this->csim_matches[i]->matches_array = (struct match **)malloc
				(csim_matches[i]->num_alloc_matches * sizeof(struct match *));

		for (uint32_t j = 0; j < this->csim_matches[i]->num_alloc_matches; j++) {
			this->csim_matches[i]->matches_array[j] = (struct match *)malloc(sizeof(struct match));
		}
	}
}

/// Prepare all the required data
void Entities::prepare() {
	uint32_t sw = 0;

	if (ALGORITHM == 4 || ALGORITHM == 7 || ALGORITHM == 10 || ALGORITHM == 12 || ALGORITHM >= 13) {
		sw = 1;
	}

	if (sw == 1) {
		printf("\tInitializing Tokens Lexicon...\n"); fflush(NULL);
		this->tokens_lexicon = new TokensLexicon(1048576);
	}

	printf("\tTokenizing Entities...\n"); fflush(NULL);
	for (uint32_t i = 0; i < this->num_entities; i++) {
		this->e[i]->tokenize(this->tokens_lexicon, sw);
	}

	if (sw == 1) {
		printf("\tComputing Tokens IDFs...\n"); fflush(NULL);
		this->tokens_lexicon->compute_idfs(this->num_entities);

		printf("\tComputing Entities Accumulated IDFs...\n"); fflush(NULL);
		for (uint32_t i = 0; i < this->num_entities; i++) {
			this->e[i]->compute_acc_idf(this->tokens_lexicon);
		}
	}
}

/// Write the values of the pairwise matches to the appropriate file
void Entities::finalize() {
	uint32_t null_len = 4, i = 0;

	for (i = LOW_THRESHOLD; i < HIGH_THRESHOLD; i++) {
		/// Write the remaining matches which are still in memory
		for (uint32_t j = 0; j < csim_matches[i]->num_matches; j++) {
			fwrite( &(this->csim_matches[i]->matches_array[j]->e1_id), sizeof(uint32_t), 1, this->output_files[i]);
			fwrite( &(this->csim_matches[i]->matches_array[j]->e2_id), sizeof(uint32_t), 1, this->output_files[i]);
		}

		/// Go to the beginning of the file and write the number of matches
		fseek(this->output_files[i], sizeof(uint32_t) + null_len * sizeof(char), SEEK_SET);
		fwrite( &(this->num_algo_matches[i]), sizeof(uint32_t), 1, this->output_files[i]);
		fclose(this->output_files[i]);
	}
}

/// Insert a pairwise similarity value between two entities into the intimate matches array
void Entities::insert_match(class Entity * e1, class Entity * e2, double sim) {
#ifndef EFFICIENCY_TESTS
	double simt = 0.0;

	/// For 10 similarity thresholds
	for (uint32_t k = LOW_THRESHOLD; k < HIGH_THRESHOLD; k++) {
		/// Similarity threshold
		simt = (double)(k + 1) / 10;

		if (sim >= simt) {
			this->num_algo_matches[k]++;
			this->csim_matches[k]->matches_array[ this->csim_matches[k]->num_matches ]->e1_id = e1->get_id();
			this->csim_matches[k]->matches_array[ this->csim_matches[k]->num_matches ]->e2_id = e2->get_id();
			this->csim_matches[k]->num_matches++;

			/// In case the array becomes full, write it to disk in the appropriate file
			if (this->csim_matches[k]->num_matches >= this->csim_matches[k]->num_alloc_matches) {
				for (uint32_t x = 0; x < this->csim_matches[k]->num_matches; x++) {
					fwrite( &(csim_matches[k]->matches_array[x]->e1_id), sizeof(uint32_t), 1, this->output_files[k]);
					fwrite( &(csim_matches[k]->matches_array[x]->e2_id), sizeof(uint32_t), 1, this->output_files[k]);
				}
				this->csim_matches[k]->num_matches = 0;
			}
		}
	}
#endif
}


/// ////////////////////////////////////////////////////////////////////////////////////////////////
/// PART A: CLUSTERING ALGORITHMS //////////////////////////////////////////////////////////////////

/// Cosine Similarity between two entities: This is reused in all clustering algorithms
double Entities::cosine_similarity(class Entity * e_1, class Entity * e_2) {
	uint32_t it_1 = 0, it_2 = 0;
	double idf_i = 0.0;

	/// The tokens are sorted in lexicographical order, hence, the computation of their
	/// intersection is of linear complexity O(e_1->num_tokens + e_2->num_tokens).
	while(it_1 < e_1->get_num_tokens() && it_2 < e_2->get_num_tokens()) {
//		printf("\t\t%s == %s\n", e_1->get_token(it_1), e_2->get_token(it_2));
		if (strcmp(e_1->get_token(it_1), e_2->get_token(it_2)) == 0) {
			idf_i += pow(this->tokens_lexicon->get_node(e_1->get_token(it_1))->get_idf(), 2);
//			printf("\t\tCommon Token: %s (IDF: %5.3f)\n",  e_1->get_token(it_1), idf_i);
			it_1++;
			it_2++;
		} else if (strcmp(e_1->get_token(it_1), e_2->get_token(it_2)) < 0) {
			it_1++;
		} else {
			it_2++;
		}
	}
	return idf_i / (sqrt(e_1->get_acc_idf()) * sqrt(e_2->get_acc_idf()));
}

/// Given a (sorted) array of vendors and an ID, (binary) search for the vendor in the array.
int32_t Entities::search_vendor(class EntityVendor ** ven, uint32_t v_id, int32_t l, int32_t r) {
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

/// Verification Stage
void Entities::perform_verification() {
	int32_t t = 0;
	uint32_t i = 0, j = 0, k = 0, onum_clusters = 0;
	double sim = 0.0, max_sim = 0.0;

	class Entity * e;
	class EntityVendor * v;
	class EntitiesCluster * c, * r;
	class EntityVendor ** evicted_vendors = new EntityVendor * [this->num_vendors];
	for (i = 0; i < this->num_vendors; i++) {
		evicted_vendors[i] = new EntityVendor(v_ids[i]);
	}

	/// For each cluster organize the entities by vendor
	for (i = 0; i < this->num_clusters; i++) {
		this->clusters[i]->prepare(this->tokens_lexicon);
	}

	/// After the preparation of the clusters, delete all (except 1) the products from violating vendors.
	for (i = 0; i < this->num_clusters; i++) {
		for (j = 0; j < this->clusters[i]->get_num_vendors(); j++) {
			v = this->clusters[i]->get_vendor(j);

			if (v->get_num_entities() > 1) {
				t = this->search_vendor(evicted_vendors, v->get_id(), 0, this->num_vendors - 1);

				for (k = 1; k < v->get_num_entities(); k++) {
					evicted_vendors[t]->insert_entity( v->get_entity(k) );
					this->clusters[i]->delete_entity(j, k);
				}
			}
		}
	}

	/// For each cluster, find the most similar product from each vendor: If the vendor is valid and
	/// the similarity exceeds the threshold, insert the product into the cluster.
	for (i = 0; i < this->num_clusters; i++) {
		c = this->clusters[i];

		for (j = 0; j < this->num_vendors; j++) {
			v = evicted_vendors[j];

			if ( !c->has_vendor( v->get_id() ) ) {
				max_sim = 0.0;
				t = 0;
				e = NULL;

				for (k = 0; k < v->get_num_entities(); k++) {
					if (v->get_entity(k)) {
						sim = this->cosine_similarity(c->get_leader(), v->get_entity(k));
//						printf("%s == Vs == %s (Sim: %5.3f)\n", c->get_leader()->get_text(),
//								v->get_entity(k)->get_text(), sim); getchar();

						if (sim > max_sim) {
							max_sim = sim;
							if (sim > SIMILARITY_THRESHOLD) {
								t = k;
								e = v->get_entity(k);
							}
						}
					}
				}

				if (e) {
					c->insert_entity(e);
					v->delete_entity(t);
				}
			}
		}
	}

	/// Take care of the rest of the evicted products which were not inserted into a cluster. That
	/// is, we will create new clusters and insert the most similar products there.
	onum_clusters = this->num_clusters;

	for (j = 0; j < this->num_vendors; j++) {
		v = evicted_vendors[j];

		for (k = 0; k < v->get_num_entities(); k++) {
			if ( v->get_entity(k) ) {

				if (j == 0) {
					if (this->num_clusters >= this->num_alloc_clusters) {
						this->num_alloc_clusters *= 2;
						this->clusters = (class EntitiesCluster **)realloc
							(this->clusters, this->num_alloc_clusters * sizeof(class EntitiesCluster *));
					}

					this->clusters[this->num_clusters] = new EntitiesCluster(this->num_clusters);
					this->clusters[this->num_clusters]->insert_entity( v->get_entity(k) );
					this->clusters[this->num_clusters]->set_leader( v->get_entity(k) );
					this->num_clusters++;

				} else {
					r = NULL;
					max_sim = 0.0;

					for (i = onum_clusters; i < this->num_clusters; i++) {
						if (! this->clusters[i]->has_vendor( v->get_id() )) {
							sim = this->cosine_similarity(this->clusters[i]->get_leader(), v->get_entity(k));

//							printf("%s == Vs == %s ( Sim: %5.3f )\n", this->clusters[i]->get_leader()->get_text(),
//									v->get_entity(k)->get_text(), sim); getchar();

							if (sim > max_sim) {
								max_sim = sim;
								if (sim > SIMILARITY_THRESHOLD) {
									r = this->clusters[i];
								}
							}
						}
					}

					if (r) {
						r->insert_entity(v->get_entity(k));
					} else {
						if (this->num_clusters >= this->num_alloc_clusters) {
							this->num_alloc_clusters *= 2;
							this->clusters = (class EntitiesCluster **)realloc
								(this->clusters, this->num_alloc_clusters * sizeof(class EntitiesCluster *));
						}

						this->clusters[this->num_clusters] = new EntitiesCluster(this->num_clusters);
						this->clusters[this->num_clusters]->insert_entity( v->get_entity(k) );
						this->clusters[this->num_clusters]->set_leader( v->get_entity(k) );
						this->num_clusters++;
					}
				}
			}
			v->delete_entity(k);
		}
	}

	for (i = 0; i < num_vendors; i++) {
		delete evicted_vendors[i];
	}
	delete [] evicted_vendors;
}


/// Leader Clustering: The input argument determines if the verification stage will be executed or not
void Entities::leader_clustering(bool verification) {
	uint32_t i = 0, j = 0, k = 0;
	uint32_t cand = 0;
	double max_similarity = 0, simt = 0.0, sim = 0.0;

#ifdef __linux__
	struct timespec ts, te;
	double duration = 0.0, t_duration = 0.0;
#endif

	this->num_alloc_clusters = 1000;

	/// For 10 similarity thresholds
	for (k = LOW_THRESHOLD; k < HIGH_THRESHOLD; k++) {
#ifdef __linux__
		clock_gettime(CLOCK_REALTIME, &ts);
#endif

		simt = (double)(k + 1) / 10;
		printf("\tSimilarity Threshold :%5.1f", simt); fflush(NULL);

		this->num_algo_matches[k] = 0;
		this->num_clusters = 0;
		cand = 0;
		max_similarity = 0;

		this->clusters = (class EntitiesCluster **)malloc
				(num_alloc_clusters * sizeof(class EntitiesCluster *));

		/// Execute the algorithm
		for (i = 0; i < this->num_entities; i++) {
			cand = 0;
			max_similarity = 0;

			/// Find the most similar cluster
			for (j = 0; j < this->num_clusters; j++) {
				sim = this->cosine_similarity(this->e[i], clusters[j]->get_leader());

				if (sim > max_similarity) {
					max_similarity = sim;
					cand = j;
				}
			}

			/// If the maximum similarity exceeds the threshold, insert the entity into the cluster
			if (max_similarity > simt) {
				clusters[cand]->insert_entity( this->e[i] );

			/// otherwise, create a new cluster and insert the entity into the new cluster
			} else {
				this->clusters[this->num_clusters] = new EntitiesCluster(this->num_clusters);
				this->clusters[this->num_clusters]->insert_entity(this->e[i]);
				this->clusters[this->num_clusters]->set_leader(this->e[i]);
				this->num_clusters++;

				if (this->num_clusters >= this->num_alloc_clusters) {
					this->num_alloc_clusters *= 2;
					this->clusters = (class EntitiesCluster **)realloc
							(this->clusters, this->num_alloc_clusters * sizeof(class EntitiesCluster *));
				}
			}
		}

		if (verification) {
			this->perform_verification();
		}

#ifdef __linux__
		clock_gettime(CLOCK_REALTIME, &te);
		duration = (te.tv_sec - ts.tv_sec) + (double)(te.tv_nsec - ts.tv_nsec) / (double)(1000000000);
		t_duration += duration;
		printf("\t -> %10.3f sec (%10.3f sec)\n", duration, t_duration); fflush(NULL);
#else
		printf("\n"); fflush(NULL);
#endif

		/// Store the pairwise matches to the appropriate file
		for (i = 0; i < this->num_clusters; i++) {
			this->num_algo_matches[k] += this->clusters[i]->create_pairwise_matches( this->output_files[k] );
			delete clusters[i];
		}
		free(this->clusters);
	}
}

/// DBSCAN Assistant function: It Finds the neighbors of the given entity q by computing the Cos-Sim with all entities
uint32_t Entities::dbscan_range_query(class Entity *** n, uint32_t * num_an, class Entity *q , double t) {
	uint32_t i = 0, num_neighbors = 0;
	double sim = 0.0;
	class Entity * e;

	for (i = 0; i < this->num_entities; i++) {
		e = this->e[i];

		sim = this->cosine_similarity(q, e);

		if (sim > t) {
			(*n)[num_neighbors] = e;
			num_neighbors++;

			if (num_neighbors >= (*num_an)) {
				(*num_an) *= 2;
				*n = (class Entity **)realloc(*n, *num_an * sizeof(class Entity *));
			}
		}

	}
	return num_neighbors;
}

/// DBSCAN Clustering: The input argument determines if the verification stage will be executed or not
void Entities::dbscan(bool verification) {
	uint32_t i = 0, j = 0, k = 0, l = 0, cand = 0;
	uint32_t n_nbors = 0, n_new_nbors = 0;
	uint32_t n_alloc_nbors = 1000, n_alloc_new_nbors = 1000;
	double simt = 0.0;
	class Entity * e;

#ifdef __linux__
	struct timespec ts, te;
	double duration = 0.0, t_duration = 0.0;
#endif

	this->num_alloc_clusters = 1000;

	/// For 10 similarity thresholds
	for (k = LOW_THRESHOLD; k < HIGH_THRESHOLD; k++) {
#ifdef __linux__
		clock_gettime(CLOCK_REALTIME, &ts);
#endif

		simt = (double)(k + 1) / 10;
		printf("\tSimilarity Threshold :%5.1f", simt); fflush(NULL);

		for (i = 0; i < this->num_entities; i++) {
			this->e[i]->set_type(0);
		}

		this->num_algo_matches[k] = 0;
		this->num_clusters = 0;

		this->clusters = (class EntitiesCluster **)malloc
				(this->num_alloc_clusters * sizeof(class EntitiesCluster *));

		class Entity ** nbors = (class Entity **)malloc(n_alloc_nbors * sizeof(class Entity *));
		class Entity ** new_nbors = (class Entity **)malloc(n_alloc_new_nbors * sizeof(class Entity *));

		/// For each product
		for (i = 0; i < this->num_entities; i++) {
			e = this->e[i];
			n_nbors = 0;
//			printf("Checking Product %d with similarity %2.1f, type=%d.\n", i, simt, e->get_type());
//			e->display();

			/// If it has not been previously processed in inner loop
			if (e->get_type() == 0) {

				n_nbors = this->dbscan_range_query(&nbors, &n_alloc_nbors, e, simt);

				/// Make this point a noise (outlier)
				if (n_nbors < DBSCAN_MINPOINTS) {
					e->set_type(2);
//					printf("Found only %d neighbors, marking as OUTLIER\n", n_nbors);

				/// otherwise continue
				} else {
//					printf("Found %d neighbors, CONTINUE\n", n_nbors);

					/// Create a new cluster and put e there
					e->set_type(1);
					this->clusters[this->num_clusters] = new EntitiesCluster(this->num_clusters);
					this->clusters[this->num_clusters]->set_leader(e);
					this->clusters[this->num_clusters]->insert_entity(e);
					this->num_clusters++;

//printf("Num clusters: %d\n", num_clusters);
					if (this->num_clusters >= this->num_alloc_clusters) {
						this->num_alloc_clusters *= 2;
						this->clusters = (class EntitiesCluster **)realloc
								(this->clusters, this->num_alloc_clusters * sizeof(class EntitiesCluster **));
					}

					/// Expand neighbors
					for (j = 0; j < n_nbors; j++) {
//						printf("\t%d. ", j); nbors[j]->display();
						if (nbors[j]->get_type() == 0 || nbors[j]->get_type() == 2) {

							/// Insert the entity into the cluster
							cand = this->num_clusters - 1;
							nbors[j]->set_type(1);
							this->clusters[cand]->insert_entity(nbors[j]);

							n_new_nbors = this->dbscan_range_query
									(&new_nbors, &n_alloc_new_nbors, nbors[j], simt);
//							printf("%d\n", n_new_nbors);

							if (n_new_nbors >= DBSCAN_MINPOINTS) {
								for (l = 0; l < n_new_nbors; l++) {
									nbors[n_nbors++] = new_nbors[l];

									if (n_nbors >= n_alloc_nbors) {
										n_alloc_nbors *= 2;
										nbors = (class Entity **)realloc
												(nbors, n_alloc_nbors * sizeof(class Entity *));
									}
								}
							}
//							printf("Now there are %d neighbors, CONTINUE\n", n_nbors);
						}
					}
//					getchar();
				}
			}
		}

//		printf("==== Num clusters: %d\n", num_clusters);

		if (verification) {
			this->perform_verification();
		}

#ifdef __linux__
		clock_gettime(CLOCK_REALTIME, &te);
		duration = (te.tv_sec - ts.tv_sec) + (double)(te.tv_nsec - ts.tv_nsec) / (double)(1000000000);
		t_duration += duration;
		printf("\t -> %10.3f sec (%10.3f sec)\n", duration, t_duration); fflush(NULL);
#else
		printf("\n"); fflush(NULL);
#endif

		/// Store the pairwise matches to the appropriate file
		for (i = 0; i < this->num_clusters; i++) {
			this->num_algo_matches[k] += this->clusters[i]->create_pairwise_matches( this->output_files[k] );
			delete this->clusters[i];
		}
		free(this->clusters);
		free(nbors);
		free(new_nbors);
	}
}

/// Hierarchical Clustering: Linkage Distance - 1: Single Linkage, 2: Complete Linkage, 3: Average Linkage
double Entities::linkage_sim(class EntitiesCluster * c1, class EntitiesCluster * c2, uint32_t type) {
/*
	double sim = 0.0, sum_sim = 0.0, min_sim = 100000.0, max_sim = 0.0;

	for (uint32_t v1 = 0; v1 < c1->get_num_vendors(); v1++) {
		for (uint32_t p1 = 0; p1 < c1->get_vendor(v1)->get_num_entities(); p1++) {
			for (uint32_t v2 = 0; v2 < c2->get_num_vendors(); v2++) {
				for (uint32_t p2 = 0; p2 < c2->get_vendor(v2)->get_num_entities(); p2++) {
					sim = this->cosine_similarity(c1->get_vendor(v1)->get_entity(p1),
						c2->get_vendor(v2)->get_entity(p2));

					sum_sim += sim;
					if (sim > max_sim) { max_sim = sim; }
					if (sim < min_sim) { min_sim = sim; }
				}
			}
		}
	}

	if (type == 1) {
		return max_sim;
	} else if (type == 2) {
		return min_sim;
	}
	return sum_sim / (double)(c1->get_num_entities() * c2->get_num_entities());
*/
	return this->cosine_similarity(c1->get_leader(), c2->get_leader());
}

/// Hierarchical Clustering: The input argument determines if the verification stage will be executed or not
void Entities::hierarchical_clustering(bool verification) {
	uint32_t i = 0, j = 0, k = 0;
	int32_t x = 0, cand_1 = -1, cand_2 = -1;
	double simt = 0.0, sim = 0.0, max_sim = 0.0;
	bool continue_merging = true;
	class EntitiesCluster ** temp_clusters = NULL;

	struct max_similarity {
		int32_t num_cluster;
		double max_sim;
	};

#ifdef __linux__
	struct timespec ts, te;
	double duration = 0.0, t_duration = 0.0;
#endif

	/// For 10 similarity thresholds
	for (k = LOW_THRESHOLD; k < HIGH_THRESHOLD; k++) {
#ifdef __linux__
		clock_gettime(CLOCK_REALTIME, &ts);
#endif

		simt = (double)(k + 1) / 10;
		printf("\tSimilarity Threshold :%5.1f", simt); fflush(NULL);

		this->num_algo_matches[k] = 0;

		/// Agglomerative clustering: Initially each entity is placed in its own singleton cluster
		/// Namely, n_clusters=n_entities. These singleton clusters will progressively merge.
		this->num_clusters = this->num_entities;
		temp_clusters = new EntitiesCluster * [this->num_entities];

		for (i = 0; i < this->num_entities; i++) {
			temp_clusters[i] = new EntitiesCluster(i);
			temp_clusters[i]->insert_entity(this->e[i]);
			temp_clusters[i]->set_leader(this->e[i]);
		}

		/// An array which for each cluster, stores the most similar cluster and the similarity value
		struct max_similarity * max_similarities = (struct max_similarity *)malloc
				(this->num_entities * sizeof(struct max_similarity));

		for (i = 0; i < this->num_entities; i++) {
			max_similarities[i].num_cluster = -1;
			max_similarities[i].max_sim = 0.0;
		}

		/// Initially, for each (singleton) cluster compute the most similar (singleton) cluster
		for (i = 0; i < this->num_entities; i++) {
			max_sim = 0.0;
			for (j = 0; j < this->num_entities; j++) {
				if (i != j) {
					sim = this->linkage_sim(temp_clusters[i], temp_clusters[j], 1);
					if (sim > max_sim) {
						max_sim = sim;
						max_similarities[i].num_cluster = j;
						max_similarities[i].max_sim = max_sim;
					}
				}
			}
		}

		/// Now start merging the clusters
		continue_merging = true;
		while(continue_merging) {
			continue_merging = false;
			max_sim = 0.0;
			cand_1 = -1;
			cand_2 = -1;

			max_sim = 0.0;

			/// Find the pair of the most similar clusters...
			for (i = 0; i < this->num_entities; i++) {
				if (max_similarities[i].num_cluster >= 0 && max_similarities[i].max_sim >= max_sim) {
					if (temp_clusters[max_similarities[i].num_cluster]) {
						max_sim = max_similarities[i].max_sim;
						cand_1 = i;
						cand_2 = max_similarities[i].num_cluster;
					}
				}
			}

			/// ... and if their similarity exceeds the threshold...
			if (max_sim > simt) {
				continue_merging = true;

//				printf("Merging cluster %d (%s) with\ncluster %d (%s)\n\n",
//					cand_1, temp_clusters[cand_1]->get_leader()->get_text(), cand_2,
//					temp_clusters[cand_2]->get_leader()->get_text());

				/// ... merge them into one. Here we replace the first cluster of the pair with the
				/// merged one...
				temp_clusters[cand_1]->merge_with(temp_clusters[cand_2]);

				/// ... and we delete the second one...
				delete temp_clusters[cand_2];
				temp_clusters[cand_2] = NULL;
				max_similarities[cand_2].num_cluster = -1;
				max_similarities[cand_2].max_sim = 0.0;
				this->num_clusters--;

				/// For the new merged cluster, compute its distances (similarities) with all the
				/// other clusters and preserve only the most similar cluster.
				max_sim = 0.0;
				for (i = 0; i < this->num_entities; i++) {
					x = i;
					if (x != cand_1 && temp_clusters[i]) {
						sim = this->linkage_sim(temp_clusters[cand_1], temp_clusters[i], 1);
//						printf("Computing Similarity between %d and %d = %5.3f\n", cand_1, i, sim); fflush(NULL);
						if (sim >= max_sim) {
							max_sim = sim;
							max_similarities[cand_1].num_cluster = i;
							max_similarities[cand_1].max_sim = max_sim;
						}
					}
				}
			}
		}

		/// Make the temprary clusters real clusters
		this->num_alloc_clusters = this->num_clusters;

		this->clusters = (class EntitiesCluster **)malloc
				(this->num_alloc_clusters * sizeof(class EntitiesCluster *));

		x = 0;
		for (i = 0; i < this->num_entities; i++) {
			if (temp_clusters[i]) {
				this->clusters[x++] = temp_clusters[i];
			}
		}

		if (verification) {
			this->perform_verification();
		}

#ifdef __linux__
		clock_gettime(CLOCK_REALTIME, &te);
		duration = (te.tv_sec - ts.tv_sec) + (double)(te.tv_nsec - ts.tv_nsec) / (double)(1000000000);
		t_duration += duration;
		printf("\t -> %10.3f sec (%10.3f sec)\n", duration, t_duration); fflush(NULL);
#else
		printf("\n"); fflush(NULL);
#endif

		/// Store the pairwise matches to the appropriate file
		for (i = 0; i < this->num_clusters; i++) {
			this->num_algo_matches[k] += this->clusters[i]->create_pairwise_matches( this->output_files[k] );
			delete this->clusters[i];
		}

		free(this->clusters);
		delete [] temp_clusters;
		free(max_similarities);
	}
}


/// ////////////////////////////////////////////////////////////////////////////////////////////////
/// PART B: STRING SIMILARITY METRICS //////////////////////////////////////////////////////////////

/// Cosine Similarity
void Entities::cosine_sim() {
	uint32_t i = 0, j = 0, it_1 = 0, it_2 = 0, common_tokens = 0;
	double sim = 0.0;
	class Entity * e_1, * e_2;

	/// For each pair of entities
	for (i = 0; i < this->num_entities; i++) {
		for (j = i; j < this->num_entities; j++) {

			/// If the two entities are from the same vendor, do not compare
			if (this->e[i]->get_vendor_id() == this->e[j]->get_vendor_id()) {
				sim = 0.0;
			} else {
				it_1 = 0; it_2 = 0;
				common_tokens = 0;
				e_1 = this->e[i];
				e_2 = this->e[j];

				/// The tokens are sorted in lexicographical order, hence, the computation of their
				/// intersection is of linear complexity O(e_1->num_tokens + e_2->num_tokens).
				while(it_1 < e_1->get_num_tokens() && it_2 < e_2->get_num_tokens()) {

					if (strcmp(e_1->get_token(it_1), e_2->get_token(it_2)) == 0) {
						common_tokens++;
						it_1++;
						it_2++;
					} else if (strcmp(e_1->get_token(it_1), e_2->get_token(it_2)) < 0) {
						it_1++;
					} else {
						it_2++;
					}

				}
				sim = (double)common_tokens / ( sqrt(e_1->get_num_tokens()) * sqrt(e_2->get_num_tokens()) );
				this->insert_match(e_1, e_2, sim);
			}
		}
	}
}

/// Cosine Similarity with IDF token weights
void Entities::cosine_sim_idf() {
	uint32_t i = 0, j = 0;
	double sim = 0.0;

	/// For each pair of entities
	for (i = 0; i < this->num_entities; i++) {
		for (j = i; j < this->num_entities; j++) {

			/// If the two entities are from the same vendor, do not compare
			if (this->e[i]->get_vendor_id() == this->e[j]->get_vendor_id()) {
				sim = 0.0;
			} else {
				sim = this->cosine_similarity(this->e[i], this->e[j]);
				this->insert_match(this->e[i], this->e[j], sim);
			}
		}
	}
}

/// Fuzzy Cosine Similarity
void Entities::cosine_sim_fuzzy() {
}

/// Jaccard Index
void Entities::jaccard_index() {
	uint32_t i = 0, j = 0, it_1 = 0, it_2 = 0, common_tokens = 0, union_tokens = 0;
	double sim = 0.0;
	class Entity * e_1, * e_2;

	/// For each pair of entities
	for (i = 0; i < this->num_entities; i++) {
		for (j = i; j < this->num_entities; j++) {

			/// If the two entities are from the same vendor, do not compare
			if (this->e[i]->get_vendor_id() == this->e[j]->get_vendor_id()) {
				sim = 0.0;
			} else {
				it_1 = 0; it_2 = 0;
				common_tokens = 0;
				e_1 = this->e[i];
				e_2 = this->e[j];

				/// The tokens are sorted in lexicographical order, hence, the computation of their
				/// intersection is of linear complexity O(e_1->num_tokens + e_2->num_tokens).
				while(it_1 < e_1->get_num_tokens() && it_2 < e_2->get_num_tokens()) {
					if (strcmp(e_1->get_token(it_1), e_2->get_token(it_2)) == 0) {
						common_tokens++;
						it_1++;
						it_2++;
					} else if (strcmp(e_1->get_token(it_1), e_2->get_token(it_2)) < 0) {
						it_1++;
					} else {
						it_2++;
					}
				}

				union_tokens = e_1->get_num_tokens() + e_2->get_num_tokens() - common_tokens;
				sim = (double)common_tokens / (double)union_tokens;
				this->insert_match(e_1, e_2, sim);
			}
		}
	}
}

/// Jaccard Index with IDF token weights
void Entities::jaccard_index_idf() {
	uint32_t i = 0, j = 0, it_1 = 0, it_2 = 0;
	double sim = 0.0, idf_i = 0.0;
	class Entity * e_1, * e_2;

	/// For each pair of entities
	for (i = 0; i < this->num_entities; i++) {
		for (j = i; j < this->num_entities; j++) {

			/// If the two entities are from the same vendor, do not compare
			if (this->e[i]->get_vendor_id() == this->e[j]->get_vendor_id()) {
				sim = 0.0;
			} else {
				it_1 = 0; it_2 = 0;
				idf_i = 0.0;
				e_1 = this->e[i];
				e_2 = this->e[j];

				/// The tokens are sorted in lexicographical order, hence, the computation of their
				/// intersection is of linear complexity O(e_1->num_tokens + e_2->num_tokens).
				while(it_1 < e_1->get_num_tokens() && it_2 < e_2->get_num_tokens()) {
					if (strcmp(e_1->get_token(it_1), e_2->get_token(it_2)) == 0) {
						idf_i += pow(this->tokens_lexicon->get_node(e_1->get_token(it_1))->get_idf(), 2);
						it_1++;
						it_2++;
					} else if (strcmp(e_1->get_token(it_1), e_2->get_token(it_2)) < 0) {
						it_1++;
					} else {
						it_2++;
					}
				}

				sim = idf_i / ( e_1->get_acc_idf() + e_2->get_acc_idf() - idf_i);
				this->insert_match(e_1, e_2, sim);
			}
		}
	}
}

/// Fuzzy Jaccard Index
void Entities::jaccard_index_fuzzy() {
}

/// Dice Similarity
void Entities::dice_sim() {
	uint32_t i = 0, j = 0, it_1 = 0, it_2 = 0, common_tokens = 0;
	double sim = 0.0;
	class Entity * e_1, * e_2;

	/// For each pair of entities
	for (i = 0; i < this->num_entities; i++) {
		for (j = i; j < this->num_entities; j++) {

			/// If the two entities are from the same vendor, do not compare
			if (this->e[i]->get_vendor_id() == this->e[j]->get_vendor_id()) {
				sim = 0.0;
			} else {
				it_1 = 0; it_2 = 0;
				common_tokens = 0;
				e_1 = this->e[i];
				e_2 = this->e[j];

				/// The tokens are sorted in lexicographical order, hence, the computation of their
				/// intersection is of linear complexity O(e_1->num_tokens + e_2->num_tokens).
				while(it_1 < e_1->get_num_tokens() && it_2 < e_2->get_num_tokens()) {
					if (strcmp(e_1->get_token(it_1), e_2->get_token(it_2)) == 0) {
						common_tokens++;
						it_1++;
						it_2++;
					} else if (strcmp(e_1->get_token(it_1), e_2->get_token(it_2)) < 0) {
						it_1++;
					} else {
						it_2++;
					}
				}

				sim = 2 * (double)common_tokens / ( e_1->get_num_tokens() + e_2->get_num_tokens() );
				this->insert_match(e_1, e_2, sim);
			}
		}
	}
}


/// Dice Similarity with IDF token weights
void Entities::dice_sim_idf() {
	uint32_t i = 0, j = 0, it_1 = 0, it_2 = 0;
	double sim = 0.0, idf_i = 0.0;
	class Entity * e_1, * e_2;

	/// For each pair of entities
	for (i = 0; i < this->num_entities; i++) {
		for (j = i; j < this->num_entities; j++) {

			/// If the two entities are from the same vendor, do not compare
			if (this->e[i]->get_vendor_id() == this->e[j]->get_vendor_id()) {
				sim = 0.0;
			} else {
				it_1 = 0; it_2 = 0;
				idf_i = 0.0;
				e_1 = this->e[i];
				e_2 = this->e[j];

				/// The tokens are sorted in lexicographical order, hence, the computation of their
				/// intersection is of linear complexity O(e_1->num_tokens + e_2->num_tokens).
				while(it_1 < e_1->get_num_tokens() && it_2 < e_2->get_num_tokens()) {
					if (strcmp(e_1->get_token(it_1), e_2->get_token(it_2)) == 0) {
						idf_i += pow(this->tokens_lexicon->get_node(e_1->get_token(it_1))->get_idf(), 2);
						it_1++;
						it_2++;
					} else if (strcmp(e_1->get_token(it_1), e_2->get_token(it_2)) < 0) {
						it_1++;
					} else {
						it_2++;
					}
				}
				sim = 2 * idf_i / ( e_1->get_acc_idf() + e_2->get_acc_idf() );
				this->insert_match(e_1, e_2, sim);
			}
		}
	}
}

/// Fuzzy Dice Similarity
void Entities::dice_sim_fuzzy() {
}

/// Min3
uint32_t Entities::min3 (uint32_t x, uint32_t y, uint32_t z) {
	return std::min(x, std::min(y,z));
}

/// Levenshtein Distance (Edit distance)
uint32_t Entities::levenshtein_dis(char * s_1, uint32_t l_1, char  *s_2, uint32_t l_2) {
	uint32_t x, y, last, old, col[l_1 + 1];

	for (y = 1; y <= l_1; y++) {
		col[y] = y;
	}

	for (x = 1; x <= l_2; x++) {
		col[0] = x;
		for (y = 1, last = x - 1; y <= l_1; y++) {
			old = col[y];
			col[y] = this->min3(col[y] + 1, col[y - 1] + 1, last + (s_1[y - 1] == s_2[x - 1] ? 0 : 1));
			last = old;
		}
	}

	return(col[l_1]);
}

/// Levenshtein Similarity (Edit similarity)
double Entities::levenshtein_sim(char * s_1, uint32_t l_1, char  *s_2, uint32_t l_2) {
	return 1 - (double)this->levenshtein_dis(s_1, l_1, s_2, l_2) / (double)std::max(l_1, l_2);
}

/// Edit similarity for multiple entities
void Entities::edit_sim() {
	uint32_t i = 0, j = 0;
	double sim = 0.0;
	class Entity * e_1, * e_2;

	/// For each pair of entities
	for (i = 0; i < this->num_entities; i++) {
		for (j = i; j < this->num_entities; j++) {

			/// If the two entities are from the same vendor, do not compare
			if (this->e[i]->get_vendor_id() == this->e[j]->get_vendor_id()) {
				sim = 0.0;
			} else {
				e_1 = this->e[i];
				e_2 = this->e[j];

				sim = this->levenshtein_sim(e_1->get_text(), e_1->get_num_chars(), e_2->get_text(), e_2->get_num_chars());
				this->insert_match(e_1, e_2, sim);
			}
		}
	}
}


/// Accessors
inline uint32_t Entities::get_num_entities() { return this->num_entities; }
