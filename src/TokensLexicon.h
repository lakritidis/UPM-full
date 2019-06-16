#ifndef TokensLexicon_H
#define TokensLexicon_H

class TokensLexicon {
	private:
		class Token **hash_table;
		uint32_t mask;
		uint32_t num_slots;  /// The number of slots of the hash table
		uint32_t num_nodes;	 /// The number of elements stored in the hash table.
		uint32_t num_chains; /// The number of non-empty chains.

	private:
		uint32_t KazLibHash(char *key);

	public:
		TokensLexicon();
		TokensLexicon(uint32_t);
		~TokensLexicon();

		void load_units();
		void display();
		void reform(class Token **, uint32_t, class statistics *);
		void compute_idfs(uint32_t);

		class Token * search(char *);
		class Token * insert(char *, uint16_t, uint16_t);
		class Token * get_node(char *);

		uint32_t get_num_nodes();
		uint32_t get_num_chains();
		uint32_t get_num_slots();
};

#endif // COMBINATION_H
