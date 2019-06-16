#ifndef CombinationsLexicon_H
#define CombinationsLexicon_H


class CombinationsLexicon {
	private:
		class Combination **hash_table;
		uint32_t mask;
		uint32_t num_slots;  /// The number of slots of the hash table
		uint32_t num_nodes;	 /// The number of elements stored in the hash table.
		uint32_t num_chains; /// The number of non-empty chains.
		uint32_t mem;

	private:
		uint32_t KazLibHash(char *key);
		void delete_tokens(char**, uint32_t);

	public:
		CombinationsLexicon();
		CombinationsLexicon(uint32_t);
		~CombinationsLexicon();

		void display();

		class Combination * search(char *, double);
		class Combination * insert(char *, double, uint32_t);
};

#endif // COMBINATIONS_H
