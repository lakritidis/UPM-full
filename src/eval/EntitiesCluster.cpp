#include "EntitiesCluster.h"

/// Default constructor
EntitiesCluster::EntitiesCluster(uint32_t i) {
	this->id = i;
	this->num_entities = 0;
	this->num_vendors = 0;
	this->num_alloc_vendors = 100;
	this->vendors = (class EntityVendor **)malloc(this->num_alloc_vendors * sizeof(class EntityVendor *));
	this->leader = NULL;
}

/// Destructor
EntitiesCluster::~EntitiesCluster() {
	if (this->vendors) {
		for (uint32_t i = 0; i < this->num_vendors; i++) {
			delete this->vendors[i];
		}
		free(this->vendors);
	}
}

/// Insert a new Entity into the cluster (indexed by vendor id)
uint32_t EntitiesCluster::insert_entity(class Entity * e) {
	uint32_t i = 0, found = 0;

	for (i = 0; i < this->num_vendors; i++) {
		if (this->vendors[i]->get_id() == e->get_vendor_id()) {
			this->vendors[i]->insert_entity(e);
			found = 1;
			break;
		}
	}

	if (found == 0) {
		this->vendors[this->num_vendors] = new EntityVendor( e->get_vendor_id() );
		this->vendors[this->num_vendors]->insert_entity(e);
		this->num_vendors++;

		if (this->num_vendors >= this->num_alloc_vendors) {
			this->num_alloc_vendors *= 2;
			this->vendors = (class EntityVendor **)realloc(this->vendors,
					this->num_alloc_vendors * sizeof(class EntityVendor *));
		}
	}

	this->num_entities++;
	return this->num_entities;
}

/// Check if there is a Entity with a specific vendor ID in the cluster
bool EntitiesCluster::has_vendor(uint32_t v_id) {
	if (this->search_vendor(v_id, 0, this->num_vendors - 1) == -1) {
		return false;
	}
	return true;
}

/// Binary search for a vendor ID in the vendors array
int32_t EntitiesCluster::search_vendor(uint32_t v_id, int32_t l, int32_t r) {
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

/// Prepare the cluster for the verification stage
void EntitiesCluster::prepare(class TokensLexicon * lex) {
	/// Sort the vendors by vendor ID (this will later allow fast - linear intersection)

	if (this->num_vendors > 1) {
		qsort(this->vendors, this->num_vendors, sizeof(EntityVendor *), cmp_vendors);
	}

	for (uint32_t i = 0; i < this->num_vendors; i++) {
		this->vendors[i]->prepare(this->leader, lex);
	}
}


/// Display the cluster contents
void EntitiesCluster::display() {
	printf("== CLUSTER Leader: %s, Num Entities: %d, EntitiesVendors: %d\n",
		this->leader->get_text(), this->num_entities, this->num_vendors);
	for (uint32_t i = 0; i < this->num_vendors; i++) {
		printf("\t %d. ", i); this->vendors[i]->display();
	}
	printf("\n");
}

/// Create the matching pairs (Entity1,Entity2) for this cluster and write them to the input file.
uint32_t EntitiesCluster::create_pairwise_matches(FILE *fp) {
	uint32_t i = 0, j = 0, k = 0, l = 0, entity1_id = 0, entity2_id = 0, num = 0;

	for (i = 0; i < this->num_vendors; i++) {
		for (j = 0; j < this->vendors[i]->get_num_entities(); j++) {
			if (this->vendors[i]->get_entity(j)) {
				num++;
			}
		}
	}

	num = 0;
	for (i = 0; i < this->num_vendors; i++) {
		for (j = 0; j < this->vendors[i]->get_num_entities(); j++) {
			if (this->vendors[i]->get_entity(j)) {
				/// Match the jth entity of the ith vendor with all the other products of this vendor
				for (k = j + 1; k < this->vendors[i]->get_num_entities(); k++) {
					if (this->vendors[i]->get_entity(k)) {
						entity1_id = this->vendors[i]->get_entity(j)->get_id();
						entity2_id = this->vendors[i]->get_entity(k)->get_id();

						if (entity1_id < entity2_id) {
							fwrite( &entity1_id, sizeof(uint32_t), 1, fp);
							fwrite( &entity2_id, sizeof(uint32_t), 1, fp);
						} else {
							fwrite( &entity2_id, sizeof(uint32_t), 1, fp);
							fwrite( &entity1_id, sizeof(uint32_t), 1, fp);
						}
//						printf("\t\t[%d, %d]\n", entity1_id, entity2_id);
						num++;
					}
				}

				/// Match the jth entity of the ith vendor with all the other products of all the
				/// other vendors
				for (k = i + 1; k < this->num_vendors; k++) {
					for (l = 0; l < this->vendors[k]->get_num_entities(); l++) {
						if (this->vendors[k]->get_entity(l)) {
							entity1_id = this->vendors[i]->get_entity(j)->get_id();
							entity2_id = this->vendors[k]->get_entity(l)->get_id();

							if (entity1_id < entity2_id) {
								fwrite( &entity1_id, sizeof(uint32_t), 1, fp);
								fwrite( &entity2_id, sizeof(uint32_t), 1, fp);
							} else {
								fwrite( &entity2_id, sizeof(uint32_t), 1, fp);
								fwrite( &entity1_id, sizeof(uint32_t), 1, fp);
							}
//							printf("\t\t[%d, %d]\n", entity1_id, entity2_id);
							num++;
						}
					}
				}
			}
		}
	}
	return num;
//	printf("Written matches: %d\n", num);
//	getchar();
}

/// Merge two clusters into one
void EntitiesCluster::merge_with(class EntitiesCluster * m) {

	for (uint32_t v = 0; v < m->get_num_vendors(); v++) {
		for (uint32_t p = 0; p < m->get_vendor(v)->get_num_entities(); p++) {
//			printf("(%d, %d)\n", x, y); m->get_entity(x, y)->display(); getchar();
			this->insert_entity( m->get_entity(v, p) );
		}
	}
}

/// "Delete" a Entity from the cluster (we just ground the pointer)
inline void EntitiesCluster::delete_entity(uint32_t v, uint32_t i) {
	this->vendors[v]->delete_entity(i);
}

/// Accessors
inline uint32_t EntitiesCluster::get_id() { return this->id; }
inline uint32_t EntitiesCluster::get_num_entities() { return this->num_entities; }
inline uint32_t EntitiesCluster::get_num_vendors() { return this->num_vendors; }
inline class Entity * EntitiesCluster::get_entity(uint32_t v, uint32_t i) { return this->vendors[v]->get_entity(i); }
inline class Entity * EntitiesCluster::get_entity(uint32_t i) {
	uint32_t num = 0;
	for (uint32_t x = 0; x < this->num_vendors; x++) {
		for (uint32_t y = 0; y < this->vendors[x]->get_num_entities(); y++) {
			if (num == i) {
				return this->vendors[x]->get_entity(y);
			}
			num++;
		}
	}
	return NULL;
}

inline class Entity * EntitiesCluster::get_leader() { return this->leader; }
inline class EntityVendor * EntitiesCluster::get_vendor(uint32_t v) { return this->vendors[v]; }

/// Mutators
inline void EntitiesCluster::set_id(uint32_t v) { this->id = v; }
inline void EntitiesCluster::set_leader(class Entity * v) { this->leader = v; }
