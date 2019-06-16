#ifndef Cluster_H
#define Cluster_H


class Cluster {
	private:
		class Combination * source;
		uint32_t num_products;

		uint32_t num_vendors;
		uint32_t num_alloc_vendors;
		class Vendor ** vendors;

		class Product * rep_product;

		uint32_t active;

		class Cluster *next;

	private:
		/// Comparison callback function for QuickSort (sorting Vendors by ascending IDs)
        static int cmp_vendors (const void * a, const void * b) {
			class Vendor * x = *(class Vendor **)a;
			class Vendor * y = *(class Vendor **)b;

			if (x->get_id() > y->get_id()) {
				return 1;
			}
			return -1;
		}

		int32_t search_vendor(uint32_t, int32_t, int32_t);

	public:
		Cluster();
		Cluster(class Combination *);
		~Cluster();

		uint32_t insert_product(class Product *);
		void delete_product(uint32_t, uint32_t);
		void create_pairwise_matches(FILE *);
		void prepare();
		bool has_vendor(uint32_t);
		uint32_t merge_with(class Cluster *);
		void compute_clustroid();
		double cos_sim_idf(class Product *, class Product *);
		void display();

		char * get_str();
		uint32_t get_num_products();
		uint32_t get_num_vendors();
		class Vendor * get_vendor(uint32_t);
		class Product * get_product(uint32_t, uint32_t);
		class Product * get_rep_product();

		uint32_t get_active();
		class Cluster * get_next();

		void set_rep_product(class Product *);
		void set_next(class Cluster *);
		void set_active(uint32_t);
};

#endif // Cluster_H
