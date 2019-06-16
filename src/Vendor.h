#ifndef VENDOR_H
#define VENDOR_H


class Vendor {
	private:
		uint32_t id;
		uint32_t num_alloc_products;
		uint32_t num_products;

		struct product {
			class Product * p;
			uint32_t order;
			double score;
		} * products;

	private:

	public:
		Vendor();
		Vendor(uint32_t);
		~Vendor();

		void display();
		void insert_product(class Product *);
		void delete_product(uint32_t);
		void compute_similarity_scores(class Product *);

		uint32_t get_id();
		uint32_t get_num_products();
		class Product * get_product(uint32_t);
};

#endif // VENDOR_H
