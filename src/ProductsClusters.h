#ifndef ProductSCLUSTERS_H
#define ProductSCLUSTERS_H

class ProductsClusters {;
	private:
		class Cluster **hash_table;
		uint32_t mask;
		uint32_t num_slots;  /// The number of slots of the hash table
		uint32_t num_nodes;	 /// The number of elements stored in the hash table.
		uint32_t num_chains; /// The number of non-empty chains.
		uint32_t mem;

		/// Asistant structure which stores the similarity value between two clusters
		struct similarity {
			uint32_t c1;
			uint32_t c2;
			double sim;
		};

	private:
		uint32_t KazLibHash(char *key);
		uint32_t JZHash(char *key, uint32_t);
		double cos_sim_idf(class Product *, class Product *);

		/// Comparison callback function for QuickSort (sorting similarities by descending similarity)
        static int cmp_similarities (const void * a, const void * b) {
			struct ProductsClusters::similarity * x = (struct ProductsClusters::similarity *)a;
			struct ProductsClusters::similarity * y = (struct ProductsClusters::similarity *)b;
			if(y->sim > x->sim) { return 1; }
			return -1;
		}

	public:
		ProductsClusters();
		ProductsClusters(uint32_t);
		~ProductsClusters();

		void create_pairwise_matches();
		void perform_verification(class statistics *, uint32_t *, class CombinationsLexicon *);
		void perform_verification_full(class statistics *, uint32_t *, class CombinationsLexicon *);
		int32_t search_vendor(class Vendor **, uint32_t, int32_t, int32_t);
		void display();

		class Cluster * search(char *);
		void insert(class Combination *, class Product *, class statistics *);
		class Cluster * insert(char *, class Product *, class statistics *, class CombinationsLexicon *);
};

#endif // ProductSCLUSTERS_H
