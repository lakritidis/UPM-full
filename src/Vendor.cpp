#include "Vendor.h"

/// Default Constructor
Vendor::Vendor() {
	this->id = 0;
	this->num_products = 0;
	this->num_alloc_products = 0;
	this->products = NULL;
}

/// Constructor 2
Vendor::Vendor(uint32_t i) {
	this->id = i;
	this->num_products = 0;
	this->num_alloc_products = 2;
	this->products = (struct Vendor::product *)malloc
			(this->num_alloc_products * sizeof(struct Vendor::product));
}

/// Destructor
Vendor::~Vendor() {
	if (this->products) {
		free (this->products);
	}
}

/// Insert a product into the corresponding array
void Vendor::insert_product(class Product *p) {
	this->products[this->num_products].p = p;
	this->products[this->num_products].order = this->num_products;
	this->products[this->num_products].score = 0.0;

	this->num_products++;
	if (this->num_products >= this->num_alloc_products) {
		this->num_alloc_products *= 2;
		this->products = (struct Vendor::product *)realloc(this->products,
				this->num_alloc_products * sizeof(struct Vendor::product));
	}
}

/// Delete a product from the corresponding array
void Vendor::delete_product(uint32_t i) {
	this->products[i].p = NULL;
}

/// Display the vendor data and its provided products
void Vendor::display() {
	printf("Vendor ID: %d, Products: %d\n", this->id, this->num_products);
	for (uint32_t i = 0; i < this->num_products; i++) {
		if (this->products[i].p) {
			char title[1024];
			this->products[i].p->get_title(title);

			printf("\tProduct %d, ID: %d, Title: %s\n", i, this->products[i].p->get_id(), title);
		} else {
			printf("\tProduct %d has been deleted\n", i);
			getchar();
		}
	}
}


/// Accessors
inline uint32_t Vendor::get_id() { return this->id; }
inline uint32_t Vendor::get_num_products() { return this->num_products; }
inline class Product * Vendor::get_product(uint32_t i) { return this->products[i].p; }
