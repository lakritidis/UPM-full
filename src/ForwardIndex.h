#ifndef FORWARDINDEX_H
#define FORWARDINDEX_H

/// * Tokens Lexicon: A dictionary data structure to map tokens to token IDs.
/// * Tokens Index: A data structure which stores all the tokens indexed by their IDs.
/// * Units Lexicon: A dictionary data structure which stores measurement units and their multiples
///   and submultiples.
/// * Combinations Lexicon: A dictionary data structure which stores the combinations and their
///   statistics.
/// * Products: The actual forward index: a list of products along with their combinations

class forwardIndex {

	private:
		class TokensLexicon * tokens_lexicon;
		class Token ** tokens_index;

		class TokensLexicon *units_lexicon;

		class CombinationsLexicon *combinations_lexicon;

		class statistics *stats;

		uint32_t num_products;
		uint32_t num_alloc_products;
		class Product ** products;

		/// Space for temporary storage of the title tokens without successive allocations & frees.
		/// The space is allocated once at Forward Index initialization, and freed upon destruction.
		uint32_t num_alloc_tokens;
		struct token {
			uint32_t id;
			char *t;
			uint16_t len;
			uint16_t typ;
			uint16_t sem;
			class Token * ht_entry;
		} ** tokens;

		uint32_t num_vendors;
		uint32_t num_alloc_vendors;
		uint32_t * vendor_ids;

	private:
		void create_cluster(uint32_t, char *);
		void insert_token (uint32_t *, uint32_t, char *, uint32_t *);

		static int cmp_token_id(const void * a, const void * b) {
			class Token * x = *(class Token **)a;
			class Token * y = *(class Token **)b;
			return x->get_id() - y->get_id();
		}

		static int cmp_int(const void * a, const void * b) {
			uint32_t x = * (uint32_t *)a;
			uint32_t y = * (uint32_t *)b;
			return x - y;
		}

	public:
		forwardIndex();
		forwardIndex(uint32_t);
		~forwardIndex();

		void process_title(uint32_t, uint32_t, uint32_t, char *, uint32_t, char *);
		void reform_tokens_lexicon(uint32_t);
		void sort_clusters();

		void display_clusters();
		void display_products();
		void display_statistics();
};

#endif // FORWARDINDEX_H
