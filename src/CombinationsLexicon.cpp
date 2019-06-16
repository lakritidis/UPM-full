#include "CombinationsLexicon.h"

/// Default Constructor
CombinationsLexicon::CombinationsLexicon() {
	this->hash_table = NULL;
	this->num_nodes = 0;
	this->num_chains = 0;
	this->num_slots = 0;
	this->mask = 0;
	this->mem = 0;
}

/// Constructor 2
CombinationsLexicon::CombinationsLexicon(uint32_t size) {
	this->hash_table = new Combination *[size];

	for (uint32_t i = 0; i < size; i++) {
		this->hash_table[i] = NULL;
	}

	this->num_nodes = 0;
	this->num_chains = 0;
	this->num_slots = size;
	this->mask = size - 1;
	this->mem = size * sizeof(class combination *) + sizeof(CombinationsLexicon);
}

/// Destructor
CombinationsLexicon::~CombinationsLexicon() {
	class Combination * q;
	for (uint32_t i = 0; i < this->num_slots; i++) {
		while (this->hash_table[i] != NULL) {
			q = this->hash_table[i]->get_next();
			delete this->hash_table[i];
			this->hash_table[i] = q;
		}
	}
	delete [] this->hash_table;
}

/// Search the hash table for a given combination. If exists, increase its frequency
/// by 1 and return the pointer to the object. Return NULL otherwise.
class Combination * CombinationsLexicon::search(char *t, double d) {
	uint32_t HashValue = KazLibHash(t) & this->mask;
	if (this->hash_table[HashValue] != NULL) {
		class Combination *q;

		/// Traverse the linked list that represents the chain.
		for (q = this->hash_table[HashValue]; q != NULL; q = q->get_next()) {
			if (strcmp(q->get_str(), t) == 0) {
				q->increase_frequency(1);
				q->increase_dist_acc(d);
				return q; /// Return the pointer and exit
			}
		}
	}
	return NULL; /// Literal not found, return NULL
}


/// Insert an element into the hash table
class Combination * CombinationsLexicon::insert(char *t, double d, uint32_t nt) {

	class Combination *res;
	if ((res = this->search(t, d)) != 0) {
		return res;
	}

	/// If the record does not exist in the hash table, create a new one & re-assign the list's head
	uint32_t HashValue = KazLibHash(t) & this->mask;

	class Combination * record = new Combination(t, d, nt);
	record->set_next(this->hash_table[HashValue]);
	this->hash_table[HashValue] = record;

	this->mem += sizeof(record);

	return record;
}


/// Display the Hash table elements to stdout
void CombinationsLexicon::display() {
	class Combination * q;

	/// Iterate through the Hash Table and display non NULL keys.
	for (uint32_t i = 0; i < this->num_slots; i++) {
		if (this->hash_table[i] != NULL) {
			for (q = this->hash_table[i]; q != NULL; q = q->get_next()) {
				q->display(NULL, NULL);
			}
		}
	}
}


/// The Hash Function
uint32_t CombinationsLexicon::KazLibHash (char *key) {
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


void CombinationsLexicon::delete_tokens (char**tokens, uint32_t ntok) {
	for (uint32_t i = 0; i < ntok; i++) {
		delete [] tokens[i];
	}
	delete [] tokens;
}
