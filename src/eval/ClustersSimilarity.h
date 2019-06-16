#ifndef CLUSTERSSIMILARITY_H
#define CLUSTERSSIMILARITY_H

class ClustersSimilarity {
	private:
		class EntitiesCluster * c1;
		class EntitiesCluster * c2;
		double similarity;

	public:
		ClustersSimilarity();
		ClustersSimilarity(class EntitiesCluster *, class EntitiesCluster *);
		~ClustersSimilarity();

		void set_similarity(double);
		void set_score(double);
		void set_cluster_1(class EntitiesCluster *);
		void set_cluster_2(class EntitiesCluster *);

		double get_similarity();
		double get_score();
		class EntitiesCluster * get_cluster_1();
		class EntitiesCluster * get_cluster_2();
};

#endif // CLUSTERSSIMILARITY_H
