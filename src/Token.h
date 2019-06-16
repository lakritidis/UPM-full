#ifndef Token_H
#define Token_H

class Token {
	private:
		uint32_t id;
		char *str;
		uint32_t freq;
		uint16_t type;
		uint16_t sem;
		double idf;

		class Token *next;

	public:
		Token();
		Token(char *, uint32_t, uint16_t, uint16_t);
		~Token();

		void display();
		void compute_idf(uint32_t);

		uint32_t get_id();
		char *get_str();
		uint32_t get_freq();
		uint16_t get_type();
		uint16_t get_sem();
		double get_idf();
		class Token *get_next();

		void set_id(uint32_t);
		void set_str(char *);
		void set_freq(uint32_t);
		void set_type(uint16_t);
		void set_sem(uint16_t);
		void set_idf(double);
		void set_next(class Token *);
};

#endif // COMBINATION_H
