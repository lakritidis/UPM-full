#ifndef STATISTICS_H
#define STATISTICS_H


class statistics {
	private:
		double duration;
#ifdef __linux__
		struct timespec ts;
#elif _WIN32

#endif
		uint32_t num_combinations;
		uint64_t combinations_length_acc;
		double avg_combinations_length;

		uint32_t num_titles;
		uint32_t max_title_length;
		uint32_t min_title_length;
		uint64_t title_length_acc;
		double avg_title_length;

		uint32_t num_vendors;
		uint32_t num_tokens;

		uint32_t num_clusters;
		uint32_t max_cluster_size;
		uint32_t min_cluster_size;

		uint64_t cluster_size_acc;
		double avg_cluster_size;

		uint32_t zone_token_acc[NUM_ZONES];   /// How many hot tokens do we have
		uint32_t zone_product_acc[NUM_ZONES]; /// How many products with hot tokens do we have

	public:
		statistics();
		~statistics();

		void display();

		void get_elapsed_time();

		void compute_final_values();

		/// Mutators
		void update_combinations(uint32_t);
		void update_titles(uint32_t);
		void update_clusters(uint32_t);
		void increase_clusters();
		void update_zone_tokens(uint32_t, uint32_t);
		void update_zone_products(uint32_t);
		void set_num_vendors(uint32_t);
		void set_num_tokens(uint32_t);

		/// Accessors
		uint32_t get_num_combinations();
		double get_avg_combinations_length();
		uint32_t get_num_titles();
		uint32_t get_max_title_length();
		uint32_t get_min_title_length();
		double get_avg_title_length();
		uint32_t get_num_vendors();
		uint32_t get_num_tokens();
		uint32_t get_num_clusters();
		uint32_t get_max_cluster_size();
		uint32_t get_min_cluster_size();
		uint32_t get_num_zone_tokens(uint32_t);
		double get_avg_cluster_size();
		double get_avg_zone_tokens(uint32_t);
		double get_avg_zone_products(uint32_t);
};

#endif // STATISTICS_H
