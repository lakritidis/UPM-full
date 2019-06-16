#include "ClustersSimilarity.h"


/// Default Constructor
ClustersSimilarity::ClustersSimilarity() {
	this->c1 = NULL;
	this->c2 = NULL;
	this->similarity = 0.0;
}

/// Constructor 1
ClustersSimilarity::ClustersSimilarity(class EntitiesCluster * clu1, class EntitiesCluster * clu2) {
	this->c1 = clu1;
	this->c2 = clu2;
	this->similarity = 0.0;
}

/// Destructor
ClustersSimilarity::~ClustersSimilarity() {

}

/// Accessors
inline class EntitiesCluster * ClustersSimilarity::get_cluster_1() { return this->c1; }
inline class EntitiesCluster * ClustersSimilarity::get_cluster_2() { return this->c2; }
inline double ClustersSimilarity::get_similarity() { return this->similarity; }
inline double ClustersSimilarity::get_score() { return this->similarity; }

/// Mutators
inline void ClustersSimilarity::set_cluster_1(class EntitiesCluster * c) { this->c1 = c; }
inline void ClustersSimilarity::set_cluster_2(class EntitiesCluster * c) { this->c2 = c; }
inline void ClustersSimilarity::set_similarity(double s) { this->similarity = s; }
inline void ClustersSimilarity::set_score(double s) { this->similarity = s; }
