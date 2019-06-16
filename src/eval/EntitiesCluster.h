#ifndef ENTITIESCLUSTER_H
#define ENTITIESCLUSTER_H

class EntitiesCluster {
	private:
		uint32_t id;
		uint32_t num_entities;

		uint32_t num_vendors;
		uint32_t num_alloc_vendors;
		class EntityVendor ** vendors;

		class Entity * leader;

	private:
		/// Comparison callback function for QuickSort (sort Vendors by ascending IDs)
        static int cmp_vendors (const void * a, const void * b) {
			class EntityVendor * x = *(class EntityVendor **)a;
			class EntityVendor * y = *(class EntityVendor **)b;

			if (x->get_id() > y->get_id()) {
				return 1;
			}
			return -1;
		}

		int32_t search_vendor(uint32_t, int32_t, int32_t);

	public:
		EntitiesCluster(uint32_t);
		~EntitiesCluster();

		uint32_t insert_entity(class Entity *);
		void delete_entity(uint32_t, uint32_t);
		uint32_t create_pairwise_matches(FILE *);
		void prepare(class TokensLexicon *);
		void merge_with(class EntitiesCluster *);
		bool has_vendor(uint32_t);
		void display();

		uint32_t get_id();
		uint32_t get_num_entities();
		uint32_t get_num_vendors();
		class EntityVendor * get_vendor(uint32_t);
		class Entity * get_entity(uint32_t);
		class Entity * get_entity(uint32_t, uint32_t);
		class Entity * get_leader();

		void set_id(uint32_t);
		void set_leader(class Entity *);
};

#endif // ENTITIESCLUSTER_H
