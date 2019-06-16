#include "EntityVendor.h"


/// Default Constructor
EntityVendor::EntityVendor() {
	this->id = 0;
	this->num_entities = 0;
	this->num_alloc_entities = 0;
	this->entities = NULL;
}

/// Constructor 2
EntityVendor::EntityVendor(uint32_t i) {
	this->id = i;
	this->num_entities = 0;
	this->num_alloc_entities = 2;
	this->entities = (struct EntityVendor::entity *)malloc
			(this->num_alloc_entities * sizeof(struct EntityVendor::entity));

}

/// Destructor
EntityVendor::~EntityVendor() {
	if (this->entities) {
		free (this->entities);
	}
}

/// Insert a product into the corresponding array
void EntityVendor::insert_entity(class Entity * e) {
	this->entities[this->num_entities].e = e;
	this->entities[this->num_entities].order = this->num_entities;
	this->entities[this->num_entities].score = 0.0;

	this->num_entities++;
	if (this->num_entities >= this->num_alloc_entities) {
		this->num_alloc_entities *= 2;
		this->entities = (struct EntityVendor::entity *)realloc(this->entities,
				this->num_alloc_entities * sizeof(struct EntityVendor::entity));
	}
}

/// Prepare
void EntityVendor::prepare(class Entity * e, class TokensLexicon * lex) {
	for (uint32_t i = 0; i < this->num_entities; i++) {
		if (e == this->entities[i].e) {
			this->entities[i].score = 1.0;
		} else {
			this->entities[i].score = this->cos_sim_idf(this->entities[i].e, e, lex);
		}
	}
	qsort(this->entities, this->num_entities, sizeof(struct EntityVendor::entity), cmp_entities);
}

/// Compute the IDF similarity of the entities of the vendor with the product in the argument.
double EntityVendor::cos_sim_idf(class Entity * e_1, class Entity * e_2, class TokensLexicon * lex) {
	uint32_t it_1 = 0, it_2 = 0;
	double idf_i = 0.0;

	/// The tokens are sorted in lexicographical order, hence, the computation of their
	/// intersection is of linear complexity O(e_1->num_tokens + e_2->num_tokens).
	while(it_1 < e_1->get_num_tokens() && it_2 < e_2->get_num_tokens()) {
//		printf("\t\t%s == %s\n", e_1->get_token(it_1), e_2->get_token(it_2));
		if (strcmp(e_1->get_token(it_1), e_2->get_token(it_2)) == 0) {
			idf_i += pow(lex->get_node(e_1->get_token(it_1))->get_idf(), 2);
//			printf("\t\tCommon Token: %s (IDF: %5.3f)\n",  e_1->get_token(it_1), idf_i);
			it_1++;
			it_2++;
		} else if (strcmp(e_1->get_token(it_1), e_2->get_token(it_2)) < 0) {
			it_1++;
		} else {
			it_2++;
		}
	}
	return idf_i / (sqrt(e_1->get_acc_idf()) * sqrt(e_2->get_acc_idf()));
}


/// Delete a product from the corresponding array
void EntityVendor::delete_entity(uint32_t i) {
	this->entities[i].e = NULL;
}

/// Display the EntityVendor data and its provided products
void EntityVendor::display() {
	printf("Vendor ID: %d, Products: %d\n", this->id, this->num_entities);
	for (uint32_t i = 0; i < this->num_entities; i++) {
		if (this->entities[i].e) {
			printf("\t\t%d. ", i); this->entities[i].e->display();
		} else {
			printf("\t\t%d. Entity has been deleted\n", i);
			getchar();
		}
	}
}

/// Accessors
inline uint32_t EntityVendor::get_id() { return this->id; }
inline uint32_t EntityVendor::get_num_entities() { return this->num_entities; }
inline class Entity * EntityVendor::get_entity(uint32_t i) { return this->entities[i].e; }
