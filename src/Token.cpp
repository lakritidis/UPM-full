#include "Token.h"

/// Default Constructor
Token::Token() {
	this->id = 0;
	this->str = NULL;
	this->freq = 0;
	this->idf = 0.0;
	this->type = 0;
	this->sem = 0;
	this->next = NULL;
}

/// Constructor 2
Token::Token(char *v, uint32_t t, uint16_t tp, uint16_t s) {
	this->id = t;

	this->str = new char[strlen(v) + 1];
	strcpy(this->str, v);

	this->freq = 1;
	this->type = tp;
	this->sem = s;
	this->idf = 0.0;
}

/// Destructor
Token::~Token(){
	if (this->str) {
		delete [] this->str;
	}
}

/// Display a Token object
inline void Token::display() {
	printf("Token ID: %d Literal: %s, Freq: %d, IDF: %5.3f, Type: %d, Semantics: %d\n",
		this->id, this->str, this->freq, this->idf, this->type, this->sem);
}

/// Compute the Inverse Document Frequency (IDF) of the token
inline void Token::compute_idf(uint32_t N) {
	this->idf = log10 ((double)N / (double)this->freq );
}

/// Accessors
inline uint32_t Token::get_id() { return this->id; }
inline char* Token::get_str() { return this->str; }
inline uint32_t Token::get_freq() { return this->freq; }
inline uint16_t Token::get_type() { return this->type; }
inline uint16_t Token::get_sem() { return this->sem; }
inline double Token::get_idf() { return this->idf; }
inline Token* Token::get_next() { return this->next; }

/// Mutators
inline void Token::set_id(uint32_t v) { this->id = v; }
inline void Token::set_str(char *v) {
	this->str = new char[strlen(v) + 1];
	strcpy(this->str, v);
}
inline void Token::set_freq(uint32_t v) { this->freq = v; }
inline void Token::set_type(uint16_t v) { this->type = v; }
inline void Token::set_idf(double v) { this->idf = v; }
inline void Token::set_sem(uint16_t v) { this->sem = v; }
inline void Token::set_next(class Token *v) { this->next = v; }
