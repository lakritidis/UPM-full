#ifndef ENTITYVENDOR_H
#define ENTITYVENDOR_H

class EntityVendor {
	private:
		uint32_t id;
		uint32_t num_alloc_entities;
		uint32_t num_entities;

		struct entity {
			class Entity * e;
			uint32_t order;
			double score;
		} * entities;

	private:
		/// Comparison callback function for QuickSort (score-based cluster sorting)
        static int cmp_entities(const void * a, const void * b) {
			struct EntityVendor::entity * x = (struct EntityVendor::entity *)a;
			struct EntityVendor::entity * y = (struct EntityVendor::entity *)b;

			if (x->score > y->score) {
				return -1;
			}
			return 1;
		}

	public:
		EntityVendor();
		EntityVendor(uint32_t);
		~EntityVendor();

		void display();
		void insert_entity(class Entity *);
		void delete_entity(uint32_t);
		void prepare(class Entity *, class TokensLexicon *);
		double cos_sim_idf(class Entity *, class Entity *, class TokensLexicon *);

		uint32_t get_id();
		uint32_t get_num_entities();
		class Entity * get_entity(uint32_t);
};

#endif // ENTITYVENDOR_H
