#ifndef ENTITY_H
#define ENTITY_H


class Entity {
	private:
		uint32_t id;
		char * text;
		char ** tokenized_text;
		uint32_t num_tokens;
		uint32_t num_chars;
		uint32_t vendor_id;
		uint32_t type;
		uint32_t index;
		double acc_idf;

	private:
		/// Comparison callback function for QuickSort
		static int cmp_string(const void * a, const void * b) {
			char * x = *(char **)a;
			char * y = *(char **)b;

			return strcmp(x,y);
		}

	public:
		Entity();
		~Entity();

		void tokenize(class TokensLexicon *, uint32_t);
		void compute_acc_idf(class TokensLexicon *);
		void display();

		void set_id(uint32_t);
		void set_text(char *, uint32_t);
		void set_vendor_id(uint32_t);
		void set_type(uint32_t);
		void set_index(uint32_t);

		uint32_t get_id();
		char * get_text();
		char * get_token(uint32_t);
		uint32_t get_num_tokens();
		uint32_t get_num_chars();
		uint32_t get_vendor_id();
		uint32_t get_type();
		uint32_t get_index();
		double get_acc_idf();
};

#endif // ENTITY_H
