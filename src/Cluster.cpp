#include "Cluster.h"

/// Default constructor
Cluster::Cluster() {
	this->source = NULL;
	this->num_products = 0;
	this->num_vendors = 0;
	this->num_alloc_vendors = 0;
	this->vendors = NULL;
	this->rep_product = NULL;
	this->active = 0;
	this->next = NULL;
}

/// Constructor 2
Cluster::Cluster(class Combination * s) {
	this->source = s;
	this->num_products = 0;
	this->num_vendors = 0;
	this->num_alloc_vendors = 4;
	this->vendors = (class Vendor **)malloc(this->num_alloc_vendors * sizeof(class Vendor *));
	this->rep_product = NULL;
	this->active = 1;
	this->next = NULL;
}

/// Destructor
Cluster::~Cluster() {
	if (this->vendors) {
		for (uint32_t i = 0; i < this->num_vendors; i++) {
			delete this->vendors[i];
		}
		free(this->vendors);
	}
}

/// Insert a new Product into the cluster (indexed by vendor id)
uint32_t Cluster::insert_product(class Product * p) {
	uint32_t i = 0, found = 0;

	for (i = 0; i < this->num_vendors; i++) {
		if (this->vendors[i]->get_id() == p->get_vendor_id()) {
			this->vendors[i]->insert_product(p);
			found = 1;
			break;
		}
	}

	if (found == 0) {
		this->vendors[this->num_vendors] = new Vendor(p->get_vendor_id());
		this->vendors[this->num_vendors]->insert_product(p);
		this->num_vendors++;

		if (this->num_vendors >= this->num_alloc_vendors) {
			this->num_alloc_vendors *= 2;
			this->vendors = (class Vendor **)realloc(this->vendors,
					this->num_alloc_vendors * sizeof(class Vendor *));
		}
	}

	this->num_products++;
	return this->num_products;
}

/// Check if there is a Product with a specific vendor ID in the cluster
bool Cluster::has_vendor(uint32_t v_id) {
	if (this->search_vendor(v_id, 0, this->num_vendors - 1) == -1) {
		return false;
	}
	return true;
}

/// Binary search for a vendor ID in the vendors array
int32_t Cluster::search_vendor(uint32_t v_id, int32_t l, int32_t r) {
	if (r >= l) {
		int32_t mid = (r + l) / 2;

		if (this->vendors[mid]->get_id() == v_id) {
			return mid;
		} else if (this->vendors[mid]->get_id() > v_id) {
			return this->search_vendor(v_id, l, mid - 1);
		} else {
			return this->search_vendor(v_id, mid + 1, r);
		}
	}

    return -1;
}

/// Compute the cosine similarity (with idf weighting) between two Product titles
double Cluster::cos_sim_idf(class Product *p1, class Product *p2) {
	double idf_1 = p1->get_idf(), idf_2 = p2->get_idf(), idf_i = 0.0, idf = 0.0;
	uint32_t it_1 = 0, it_2 = 0;

	while(it_1 < p1->get_num_tokens() && it_2 < p2->get_num_tokens()) {
		if (p1->get_token(it_1)->get_id() == p2->get_token(it_2)->get_id()) {
			idf_i += pow(p1->get_token(it_1)->get_idf(), 2);
			it_1++;
			it_2++;
		} else if (p1->get_token(it_1)->get_id() < p2->get_token(it_2)->get_id()) {
			it_1++;
		} else {
			it_2++;
		}
	}

	idf = idf_i / (sqrt(idf_1) * sqrt(idf_2));
	return idf;
}

/// Compute the cluster's clustroid ie product which has the minimum distance from all other products
void Cluster::compute_clustroid() {
	uint32_t v1 = 0, v2 = 0, p1 = 0, p2 = 0;
	double max_sim = 0.0, sum_sim = 0.0;

	this->rep_product = NULL;
	if (this->num_products == 1) {
		this->rep_product = this->vendors[0]->get_product(0);
	} else if (this->num_products == 2) {
		this->rep_product = NULL;
	} else {
		for (v1 = 0; v1 < this->num_vendors; v1++) {
			for (p1 = 0; p1 < this->vendors[v1]->get_num_products(); p1++) {
				sum_sim = 0.0;

				for (v2 = 0; v2 < this->num_vendors; v2++) {
					if (v1 != v2) {
						for (p2 = 0; p2 < this->vendors[v1]->get_num_products(); p2++) {
							sum_sim += this->cos_sim_idf(this->vendors[v1]->get_product(p1), this->vendors[v1]->get_product(p2));
						}
					}
				}

				if (sum_sim > max_sim) {
					max_sim = sum_sim;
					this->rep_product = this->vendors[v1]->get_product(p1);
				}
			}
		}
	}

	max_sim = 0.0;
	if (!this->rep_product) {
//	printf("No clustroid - num products: %d, num_vendors: %d\n", this->num_products, this->num_vendors); getchar();
		for (v1 = 0; v1 < this->num_vendors; v1++) {
			for (p1 = 0; p1 < this->vendors[v1]->get_num_products(); p1++) {
				if (vendors[v1]->get_product(p1)->get_num_tokens() > max_sim) {
					max_sim = this->vendors[v1]->get_product(p1)->get_num_tokens();
					this->rep_product = this->vendors[v1]->get_product(p1);
				}
			}
		}
	} else {
//		printf("dfgdfgfgfd"); getchar();
	}
}

/// Prepare the cluster for the verification stage
void Cluster::prepare() {
	/// Sort the vendors by vendor ID (this will later allow fast - linear intersection)
	if (this->num_vendors > 1) {
		qsort(this->vendors, this->num_vendors, sizeof(Vendor *), cmp_vendors);
	}

	/// Find the representative product:  it is the product with the longest title (this essentially
	/// means that we pick the product which has the greatest similarity with the cluster's label).
	this->compute_clustroid();
}

/// Merge this Cluster with the argument cluster
uint32_t Cluster::merge_with(class Cluster * c) {
	uint32_t p = 0, v = 0;

	/// First check if merging is possible: Merging is impossible if c has a product from a vendor
	/// which already resides in this cluster
	for (v = 0; v < c->get_num_vendors(); v++) {
		if ( this->has_vendor(c->get_vendor(v)->get_id()) ) {
			return 0;
		}
	}

	/// Merging is imposiible if one or both clusters are inactive
	if (this->active == 0 || c->get_active() == 0) {
		return 0;
	}

	/// Insert all the products of c in this cluster
	for (v = 0; v < c->get_num_vendors(); v++) {
		for (p = 0; p < c->get_vendor(v)->get_num_products(); p++) {
			if (c->get_product(v, p)) {
				this->insert_product( c->get_product(v, p) );
			}
		}
	}

	/// Since c, has been merged to this cluster, delete (deactivate it)
	c->set_active(0);
	return 1;
}

/// Display the cluster contents
void Cluster::display() {
	printf("Cluster: %s, Num Products: %d, Vendors: %d\n",
		this->source->get_str(), this->num_products, this->num_vendors);
	for (uint32_t i = 0; i < this->num_vendors; i++) {
		this->vendors[i]->display();
	}
	printf("\n");
}

/// Create the matching pairs (Product1,Product2) for this cluster and write them to the input file.
void Cluster::create_pairwise_matches(FILE *fp) {
	uint32_t i = 0, j = 0, k = 0, l = 0, product1_id = 0, product2_id = 0, len = 0, num = 0;
	len = strlen(this->source->get_str());

	for (i = 0; i < this->num_vendors; i++) {
		for (j = 0; j < this->vendors[i]->get_num_products(); j++) {
			if (this->vendors[i]->get_product(j)) {
				num++;
			}
		}
	}

	fwrite(&len, sizeof(uint32_t), 1, fp);
	fwrite(this->source->get_str(), sizeof(char), len, fp);
	fwrite(&num, sizeof(uint32_t), 1, fp);

//	printf("%d Matches for cluster %s (Population: %d - Vendors: %d)\n",
//			num, this->source->get_str(), this->num_products, this->num_vendors);

	num = 0;
	for (i = 0; i < this->num_vendors; i++) {
		for (j = 0; j < this->vendors[i]->get_num_products(); j++) {
			if (this->vendors[i]->get_product(j)) {
				for (k = j + 1; k < this->vendors[i]->get_num_products(); k++) {
					if (this->vendors[i]->get_product(k)) {
						product1_id = this->vendors[i]->get_product(j)->get_id();
						product2_id = this->vendors[i]->get_product(k)->get_id();
						if (product1_id < product2_id) {
							fwrite( &product1_id, sizeof(uint32_t), 1, fp);
							fwrite( &product2_id, sizeof(uint32_t), 1, fp);
						} else {
							fwrite( &product2_id, sizeof(uint32_t), 1, fp);
							fwrite( &product1_id, sizeof(uint32_t), 1, fp);
						}
//						printf("\t\t[%d, %d]\n", product1_id, product2_id);
						num++;
					}
				}

				for (k = i + 1; k < this->num_vendors; k++) {
					for (l = 0; l < this->vendors[k]->get_num_products(); l++) {
						if (this->vendors[k]->get_product(l)) {
							product1_id = this->vendors[i]->get_product(j)->get_id();
							product2_id = this->vendors[k]->get_product(l)->get_id();
							if (product1_id < product2_id) {
								fwrite( &product1_id, sizeof(uint32_t), 1, fp);
								fwrite( &product2_id, sizeof(uint32_t), 1, fp);
							} else {
								fwrite( &product2_id, sizeof(uint32_t), 1, fp);
								fwrite( &product1_id, sizeof(uint32_t), 1, fp);
							}
//							printf("\t\t[%d, %d]\n", product1_id, product2_id);
							num++;
						}
					}
				}
			}
		}
	}
//	printf("Written matches: %d\n", num);
//	getchar();
}

/// "Delete" a Product from the cluster (we just ground the pointer)
inline void Cluster::delete_product(uint32_t v, uint32_t i) {
	this->vendors[v]->delete_product(i);
}

/// Accessors
inline char * Cluster::get_str() { return this->source->get_str(); }
inline uint32_t Cluster::get_num_products() { return this->num_products; }
inline uint32_t Cluster::get_num_vendors() { return this->num_vendors; }
inline class Product * Cluster::get_product(uint32_t v, uint32_t i) { return this->vendors[v]->get_product(i); }
inline class Product * Cluster::get_rep_product() { return this->rep_product; }
inline class Vendor * Cluster::get_vendor(uint32_t v) { return this->vendors[v]; }
inline uint32_t Cluster::get_active() { return this->active; }
inline class Cluster * Cluster::get_next() { return this->next; }

/// Mutators
inline void Cluster::set_active(uint32_t v) { this->active = v; }
inline void Cluster::set_next(class Cluster * n) { this->next = n; }
inline void Cluster::set_rep_product(class Product * v) { this->rep_product = v; }
