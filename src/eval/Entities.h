#ifndef ENTITIES_H
#define ENTITIES_H

struct match {
	uint32_t e1_id;
	uint32_t e2_id;
};

class Entities {
	private:
		uint32_t num_entities;
		uint32_t num_alloc_entities;
		uint32_t num_algo_matches[10];
		class Entity ** e;
		class TokensLexicon * tokens_lexicon;

		FILE * output_files[9];

		struct matches {
			uint32_t num_alloc_matches;
			uint32_t num_matches;
			struct match ** matches_array;
		} ** csim_matches;

		uint32_t num_vendors;
		uint32_t * v_ids;

		uint32_t num_clusters;
		uint32_t num_alloc_clusters;
		class EntitiesCluster ** clusters;
	private:
		static int cmp_int(const void * a, const void * b) {
			uint32_t x = * (uint32_t *)a;
			uint32_t y = * (uint32_t *)b;
			return x - y;
		}

		void insert_match(class Entity *, class Entity *, double);

		uint32_t min3(uint32_t, uint32_t, uint32_t);
		uint32_t levenshtein_dis(char *, uint32_t, char *, uint32_t);
		double levenshtein_sim(char *, uint32_t, char *, uint32_t);
		double linkage_sim(class EntitiesCluster *, class EntitiesCluster *, uint32_t);
		double cosine_similarity(class Entity *, class Entity *);
		uint32_t dbscan_range_query(class Entity ***, uint32_t *, class Entity *, double);
		int32_t search_vendor(class EntityVendor **, uint32_t, int32_t, int32_t);

		void perform_verification();

	public:
		Entities();
		Entities(uint32_t);
		~Entities();

		void read_from_file(char *);
		void prepare_output();
		void prepare();
		void finalize();

		/// Clustering Algorithms
		void leader_clustering(bool);
		void spectral_clustering(bool);
		void dbscan(bool);
		void hierarchical_clustering(bool);

		/// String Similarity Metrics
		void cosine_sim();
		void cosine_sim_idf();
		void cosine_sim_fuzzy();
		void jaccard_index();
		void jaccard_index_idf();
		void jaccard_index_fuzzy();
		void dice_sim();
		void dice_sim_idf();
		void dice_sim_fuzzy();
		void edit_sim();

		/// Accessors
		uint32_t get_num_entities();
};

#endif // ENTITIES_H
