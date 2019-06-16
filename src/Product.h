#ifndef Product_H
#define Product_H

class Product {
	private:
        uint32_t id;
        uint32_t num_tokens;
        class Token ** tokens;

        uint32_t vendor_id;

        uint32_t num_combinations;
        class Combination ** combinations;

		char *real_cluster;
        class Combination * predicted_cluster;

        double idf;

	private:
		/// Comparison callback function for QuickSort (sorting Tokens by ascending IDs)
        static int cmp_tokens (const void * a, const void * b) {
			class Token * x = *(class Token **)a;
			class Token * y = *(class Token **)b;

			if (x->get_id() > y->get_id()) {
				return 1;
			}
			return -1;
		}

	public:
		Product();
		Product(uint32_t, uint32_t, uint32_t, class Token **, uint32_t, char *);
		~Product();

		uint32_t factorial(int32_t);
		uint32_t factorial_frac(uint32_t, uint32_t);
		void insert_cluster(class Combination *);
		void sort_clusters(class statistics *, class Token **);
		void insert_token(uint32_t, char *, uint16_t, uint16_t);
		void prepare();
		void set_predicted_cluster(uint32_t);
		void display(class TokensLexicon *);

		uint32_t get_id();
		uint32_t get_num_tokens();
		uint32_t get_num_combinations();
		uint32_t get_vendor_id();

		void get_title(char *);
		char * get_real_cluster();
		class Combination * get_predicted_cluster();
		class Combination * get_combination(uint32_t);
		class Token * get_token(uint32_t);
		double get_idf();
};

#endif // Product_H
