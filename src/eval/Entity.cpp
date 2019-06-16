#include "Entity.h"

/// Default constructor
Entity::Entity() {
	this->id = 0;
	this->text = NULL;
	this->tokenized_text = NULL;
	this->num_tokens = 0;
	this->num_chars = 0;
	this->vendor_id = 0;
	this->type = 0;
	this->acc_idf = 0.0;
}

/// Destructor
Entity::~Entity() {
	if (this->text) {
		delete [] this->text;
	}

	if (this->tokenized_text) {
		for (uint32_t i = 0; i < this->num_tokens; i++) {
			free(this->tokenized_text[i]);
		}
		free(this->tokenized_text);
	}
}


/// Tokenize a string: Split the string in its component words, store the words in an array and sort
/// the array in lexicographical order.
void Entity::tokenize(class TokensLexicon * tokens_lexicon, uint32_t sw) {
	uint32_t i = 0, j = 0, x = 0, found = 0;
	uint32_t init_tok = 10;
	char buf[1024];

	this->num_chars = strlen(this->text);

//	printf("Title (%d): %s\n", str_len, this->text); fflush(NULL);
	this->tokenized_text = (char **)malloc(init_tok * sizeof(char *));

	for (i = 0; i < this->num_chars; i++) {
		if (this->text[i] == ' ') {
			buf[x] = 0;

			found = 0;
			for (j = 0; j < this->num_tokens; j++) {
				if (strcmp(this->tokenized_text[j], buf) == 0) {
					found = 1;
					break;
				}
			}

			if (found == 0) {
				if (sw == 1) { tokens_lexicon->insert(buf, 0, 0); }

				this->tokenized_text[this->num_tokens] = (char *)malloc((x + 1) * sizeof(char));
				strcpy(this->tokenized_text[this->num_tokens], buf);
				this->num_tokens++;

				if (this->num_tokens >= init_tok) {
					init_tok *= 2;
					this->tokenized_text = (char ** )realloc(this->tokenized_text, init_tok * sizeof(char *));
				}
//				printf("Token %d: %s\n", this->num_tokens, this->tokenized_text[this->num_tokens - 1]);  fflush(NULL);
			}
			x = 0;

		} else {
			buf[x++] = this->text[i];
		}
	}
	buf[x] = 0;

	found = 0;
	for (j = 0; j < this->num_tokens; j++) {
		if (strcmp(this->tokenized_text[j], buf) == 0) {
			found = 1;
			break;
		}
	}

	if (found == 0) {
		if (sw == 1) { tokens_lexicon->insert(buf, 0, 0); }

		this->tokenized_text[this->num_tokens] = (char *)malloc((x + 1) * sizeof(char));
		strcpy(this->tokenized_text[this->num_tokens], buf);
		this->num_tokens++;

		if (this->num_tokens >= init_tok) {
			init_tok *= 2;
			this->tokenized_text = (char ** )realloc(this->tokenized_text, init_tok * sizeof(char *));
		}
//		printf("Token %d: %s\n", this->num_tokens, this->tokenized_text[this->num_tokens - 1]); fflush(NULL);
	}

	/// Sort the tokens in lexicographical ordering to allow fast comparisons
	qsort(this->tokenized_text, this->num_tokens, sizeof(char *), Entity::cmp_string);
}


/// Compute the accumulated IDF based on the IDFs of the component tokens
void Entity::compute_acc_idf(class TokensLexicon * tokens_lexicon) {
	double idf = 0.0;
	for (uint32_t i = 0; i < this->num_tokens; i++) {
		idf = tokens_lexicon->get_node(this->tokenized_text[i])->get_idf();
		this->acc_idf += idf * idf;
	}
}

/// Display the Entity's attributes
void Entity::display() {
	printf("ID: %d, Vendor: %d, Text: %s, Type: %d, IDF: %5.3f, Num Tokens: %d, Char Length:%d\n",
		this->id, this->vendor_id, this->text, this->type, this->acc_idf, this->num_tokens, this->num_chars);
}

/// Accessors
inline uint32_t Entity::get_id() { return this->id; }
inline char * Entity::get_text() { return this->text; }
inline char * Entity::get_token(uint32_t i) { return this->tokenized_text[i]; }
inline uint32_t Entity::get_num_tokens() { return this->num_tokens; }
inline uint32_t Entity::get_num_chars() { return this->num_chars; }
inline uint32_t Entity::get_vendor_id() { return this->vendor_id; }
inline uint32_t Entity::get_type() { return this->type; }
inline uint32_t Entity::get_index() { return this->index; }
inline double Entity::get_acc_idf() { return this->acc_idf; }

/// Mutators
inline void Entity::set_id(uint32_t v) { this->id = v; }
inline void Entity::set_text(char * v, uint32_t l) {
	this->text = new char [l + 1];
	strcpy(this->text, v);
}
inline void Entity::set_vendor_id(uint32_t v) { this->vendor_id = v; }
inline void Entity::set_type(uint32_t v) { this->type = v; }
inline void Entity::set_index(uint32_t v) { this->index = v; }
