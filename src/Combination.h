#ifndef Combination_H
#define Combination_H

class Combination {
	private:
		char *str;
		uint32_t num_tokens;

		uint32_t freq;
		double dist_acc;

		class Combination *next;

	public:
		Combination();
		Combination(char *, double, uint32_t);
		~Combination();

		void display(class TokensLexicon *, char *);
		void increase_frequency(uint32_t);
		void increase_dist_acc(double);

		double compute_score(class statistics *, class Token **);

		char *get_str();
		uint32_t get_frequency();
		uint32_t get_num_tokens();
		double get_dist_acc();
		double get_avg_dist();
		double get_type_score();
		class Combination *get_next();
		class Token *get_token(uint32_t);

		void set_str(char *);
		void set_frequency(uint32_t);
		void set_dist_acc(double);
		void set_type_score(double);
		void set_next(class Combination *);
};

#endif // COMBINATION_H
