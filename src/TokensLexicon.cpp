#include "TokensLexicon.h"

/// Default Constructor
TokensLexicon::TokensLexicon() { }

/// Constructor 2
TokensLexicon::TokensLexicon(uint32_t size) {
	this->hash_table = new Token *[size];

	for (uint32_t i = 0; i < size; i++) {
		this->hash_table[i] = NULL;
	}

	this->num_nodes = 0;
	this->num_chains = 0;
	this->num_slots = size;
	this->mask = size - 1;
}

/// Destructor
TokensLexicon::~TokensLexicon() {
	class Token * q;
	for (uint32_t i = 0; i < this->num_slots; i++) {
		while (this->hash_table[i] != NULL) {
			q = this->hash_table[i]->get_next();
			delete this->hash_table[i];
			this->hash_table[i] = q;
		}
	}
	delete [] this->hash_table;
}

/// Load some measurement units and their multiples and submultiples into the hash table
void TokensLexicon::load_units() {
	/// Submultiples (micro, pico, nano, milli/mega) & Multiples (kilo, mega, giga, tera)
	const char * smul[] = { "m", "n", "", "k", "g", "t" };
	uint32_t smul_size = (sizeof (smul) / sizeof (const char *));

	const char * u[] = { "b", "hz", "bps", "'", "m", "btu" };
	uint32_t u_size = (sizeof (u) / sizeof (const char *));

	char buf[32];
	uint32_t i = 0, j = 0, l = 0;

	for (i = 0; i < smul_size; i++) {
		for (j = 0; j < u_size; j++) {
			strcpy(buf, smul[i]);
			strcat(buf, u[j]);
			l = strlen(smul[i]) + strlen(u[j]);
			buf[l] = 0;

			this->insert(buf, 0, 0);
		}
	}
}

/// Search the hash table for a given token. If exists, increase its frequency by 1 and return
/// the pointer to the object. Return NULL otherwise.
class Token * TokensLexicon::search(char *t) {
	uint32_t HashValue = KazLibHash(t) & this->mask;
	if (this->hash_table[HashValue] != NULL) {
		class Token *q;

		/// Traverse the linked list that represents the chain.
		for (q = this->hash_table[HashValue]; q != NULL; q = q->get_next()) {
			if (strcmp(q->get_str(), t) == 0) {
				q->set_freq( q->get_freq() + 1 );
				return q; /// Return the pointer and exit
			}
		}
	}

	return NULL; /// Entry not found, return NULL
}

/// Search the hash table for a given token. If exists, return its frequency.
inline class Token * TokensLexicon::get_node(char * t) {
	uint32_t HashValue = KazLibHash(t) & this->mask;
	if (this->hash_table[HashValue] != NULL) {
		class Token * q;

		/// Traverse the linked list that represents the chain.
		for (q = this->hash_table[HashValue]; q != NULL; q = q->get_next()) {
			if (strcmp(q->get_str(), t) == 0) {
				return q; /// Return the pointer and exit
			}
		}
	}

	return NULL; /// Literal not found, return 0
}

/// Insert an element into the hash table
class Token * TokensLexicon::insert(char * t, uint16_t type, uint16_t sem) {
	class Token * res;

	/// In case the combination exists in the hash table, increase its frequency and return
	if ((res = this->search(t)) != NULL) {
		return res;
	}

	/// In case neither the combination nor its permutations exist in the hash table, insert
	/// Create a new record and re-assign the linked list's head
	uint32_t HashValue = KazLibHash(t) & this->mask;

	class Token * record = new Token(t, ++this->num_nodes, type, sem);
	record->set_next(this->hash_table[HashValue]);
	this->hash_table[HashValue] = record;

	return record;
}

/// Display the Hash table elements to stdout
void TokensLexicon::display() {
	class Token * q;

	/// Iterate down the Hash Table and display non NULL keys.
	for (uint32_t i = 0; i < this->num_slots; i++) {
		if (this->hash_table[i] != NULL) {
			for (q = this->hash_table[i]; q != NULL; q = q->get_next()) {
				q->display();
			}
		}
	}
}

/// Move the Hash table into a standard table
void TokensLexicon::reform(class Token **thtn, uint32_t N, class statistics * stats) {
	class Token * q;

	stats->set_num_tokens(this->num_nodes);

	/// Iterate down the Hash Table and display non NULL keys.
	for (uint32_t i = 0; i < this->num_slots; i++) {
		if (this->hash_table[i] != NULL) {
			for (q = this->hash_table[i]; q != NULL; q = q->get_next()) {
				q->compute_idf(N);
				stats->update_zone_tokens( q->get_sem(), q->get_freq() );
				thtn[ q->get_id() ] = q;
			}
		}
	}
}

/// Traverse the Hash table and compute the IDFs for all tokens
void TokensLexicon::compute_idfs(uint32_t N) {
	class Token * q;

	/// Iterate down the Hash Table and display non NULL keys.
	for (uint32_t i = 0; i < this->num_slots; i++) {
		if (this->hash_table[i] != NULL) {
			for (q = this->hash_table[i]; q != NULL; q = q->get_next()) {
				q->compute_idf(N);
//				q->display(); getchar();
			}
		}
	}
}


/// The Hash Function
uint32_t TokensLexicon::KazLibHash (char *key) {
   static unsigned long randbox[] = {
       0x49848f1bU, 0xe6255dbaU, 0x36da5bdcU, 0x47bf94e9U,
       0x8cbcce22U, 0x559fc06aU, 0xd268f536U, 0xe10af79aU,
       0xc1af4d69U, 0x1d2917b5U, 0xec4c304dU, 0x9ee5016cU,
       0x69232f74U, 0xfead7bb3U, 0xe9089ab6U, 0xf012f6aeU,
	};

	char *str = key;
	uint32_t acc = 0;

	while (*str) {
		acc ^= randbox[(*str + acc) & 0xf];
		acc = (acc << 1) | (acc >> 31);
		acc &= 0xffffffffU;
		acc ^= randbox[((*str++ >> 4) + acc) & 0xf];
		acc = (acc << 2) | (acc >> 30);
		acc &= 0xffffffffU;
	}
	return acc;
}

inline uint32_t TokensLexicon::get_num_chains() { return this->num_chains; }
inline uint32_t TokensLexicon::get_num_nodes() { return this->num_nodes; }
inline uint32_t TokensLexicon::get_num_slots() { return this->num_slots; }
