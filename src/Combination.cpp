#include "Combination.h"

/// Default Constructor
Combination::Combination() {
	this->str = NULL;
	this->num_tokens = 0;
	this->freq = 0;
	this->dist_acc = 0.0;

	this->next = NULL;
}

/// Constructor 2
Combination::Combination(char * v, double dis, uint32_t nt) {

	this->str = new char[strlen(v) + 1];
	strcpy(this->str, v);

	this->num_tokens = nt;
	this->freq = 1;
	this->dist_acc = dis;

//	printf("%s nwords: %d - semantics: %5.2f\n", v, this->nwords, this->type_score);
//	getchar();
}

/// Destructor
Combination::~Combination(){
	if (this->str) {
		delete [] this->str;
	}
}

/// Display a Combination object
inline void Combination::display(class TokensLexicon *h, char *t) {
	if (!h && !t) {
		printf("Cluster: %s, Words: %d, Frequency: %d, Avg Distance: %5.3f",
			this->str, this->num_tokens, this->freq, this->dist_acc / this->freq);
	} else if (h && t) {
		printf("Title: %s - Cluster: %s, Words: %d, Frequency: %d, Avg Distance: %5.3f",
			t, this->str, this->num_tokens, this->freq, this->dist_acc / this->freq);
	}
}


/// Compute the score of the cluster
double Combination::compute_score(class statistics *stats, class Token **tindex) {

	char * s = this->str;
	char buf[10];
	uint32_t i = 0, x = 0, y = 0, id = 0, zone_lengths[NUM_ZONES] = { 0 }, w = 0, l = strlen(s);
	double t_score = 0.0, token_weight = 0.0, zone_weight = 0.0, denom = 0.0;

	/// Retrieve the cluster's tokens
	class Token * toks [ this->num_tokens ];
	for (i = 0; i < l; i++) {
		if (s[i] == ' ') {
			buf[x] = 0;
			x = 0;
			id = atoi(buf);
			toks[y++] = tindex[id];
		} else {
			buf[x++] = s[i];
		}
	}
	buf[x] = 0;
	id = atoi(buf);
	toks[y++] = tindex[id];

	/// Compute zone lengths for this combination
	for (i = 0; i < this->num_tokens; i++) {
		zone_lengths [ toks[i]->get_sem() ]++;
	}

//	for (i = 0; i < this->num_tokens; i++) { printf("%s ", toks[i]->get_str()); } printf("\n");
//	for (i = 0; i < NUM_ZONES; i++) { printf("Zone %d: %d\n", i, zone_lengths[i]); }
//	getchar();

	/// IR Score
	for (i = 0; i < this->num_tokens; i++) {
		w = toks[i]->get_sem();

		zone_weight = 1.0 / zone_lengths[w];

		denom = 1.00 - B_PAR + B_PAR * this->num_tokens;

		token_weight = zone_weight / denom;

		t_score += token_weight * toks[i]->get_idf();
//		printf("%5.3f\n",t_score);
	}


	/// Average Distance form the beginning of the titles
	double avg_dist = this->dist_acc / this->freq;

	double score =
		(this->num_tokens / (A_PAR + avg_dist)) *	/// Distance
		pow(t_score, 2) *				/// IR score
		log10 (this->freq);				/// Frequency


//	printf("Combination: %s\nFrequency: %d, Average Distance: %5.3f, IR Score: %5.3f\n",
//		this->str, this->freq, avg_dist, t_score); getchar();

	return score;
}


/// Increase the frequency of the Combination by a defined value
inline void Combination::increase_frequency(uint32_t v) { this->freq += v; }

/// Add a new distance value to the distances accumulator
inline void Combination::increase_dist_acc(double v) { this->dist_acc += v; }

/// Accessors
inline char* Combination::get_str() { return this->str; }
inline uint32_t Combination::get_frequency() { return this->freq; }
inline uint32_t Combination::get_num_tokens() { return this->num_tokens; }
inline double Combination::get_dist_acc() { return this->dist_acc; }
inline double Combination::get_avg_dist() { return this->dist_acc / this->freq; }
inline Combination* Combination::get_next() { return this->next; }

/// Mutators
inline void Combination::set_str(char *v) {
	this->str = new char[strlen(v) + 1];
	strcpy(this->str, v);
}
inline void Combination::set_frequency(uint32_t v) { this->freq = v; }
inline void Combination::set_dist_acc(double v) { this->dist_acc = v; }
inline void Combination::set_next(class Combination * v) { this->next = v; }
