# UPM
Unsupervised Products Matching via Clustering, Combinatorics and Post-Processing Verification

This project implements the **UPM** unsupervised algorithm for matching products by considering their titles only. **UPM** performs morphological analysis of the titles, identifies crucial semantics (such as models and attributes), and then creates variable-sized combinations of tokens from each title. The combinations are later scored according to several criteria and the most appropriate combination will constitute the cluster of a product. In addition, **UPM** includes a post-processing verification stage that refines the initial clusters by correcting the erroneous matches. This verification stage is applicable to all clustering algorithms and enhances their performance by a substantial margin.

To evaluate the performance of **UPM**, this code also contains implementations of 3 popular generic clustering methods, namely, **Agglomeratice (Hierarchical) Clustering**, **DBSCAN**, and **Leader Clustering**. These algorithms can be executed either in their original form, or in combination with the aforementioned refinement stage (these are the **refined** versions). We also implemented several string similarity measures, including **Cosine Similarity**, **Jaccard index**, **Dice Coefficient**, and **Edit Similarity (Distance)**. Regarding the first three measures, there are two flavors: i) the original (baseline) form, and ii) the weighted form, where the plain token counts are replaced by IDF token weights.

The effectiveness and the efficiency of all algorithms can be evaluated by using 18 datasets that originate from two popular product comparison systems, [PriceRunner](https://www.pricerunner.com/) and [Skroutz](https://www.skroutz.gr/). The datasets are located within the `datasets` folder in a compressed form and must be decompressed in place before usage. Each dataset consists of two binary files:
 * The file where the products titles are stored along with their vendors. The file has a binary form and it is accessed from the lines 141-162 of `main.cpp` in the code.
 * The "matches" file which contains pairs of matching products. These pairs where drawn by either PriceRunner, or Skroutz, according to the classification of the two products of the pair by these two systems. An example on how to read the contents of this file is given in lines 50-77 within the `Evaluation.cpp` of the code.

We also provide these datasets in XML format to facilitate access by other types of applications.

The execution of this code is controlled by the lines 26-106 of `main.cpp`. More specifically:
  * In lines 30-40 the user selects the location of the input dataset. The location must be expressed in an absolute path manner for both Linux (lines 36-40) and Windows (lines 36-41).

  * In line 96 the user defines the value of K (maximum number of tokens in the constructed k-combinations).
  
  * In lines 97-98 the user defines the value of a and b parameters (the best results are obtained for a=1 and b=1).

  * In line 99 the user determines the value of the similarity threshold of the verification stage.

  * In line 94 the user can switch among the implementations of the algorithms. Acceptable values are:
    - `1` for UPM,
    - `3` for Cosine Similarity,
    - `4` for Cosine Similarity with IDF token weights,
    - `6` for Jaccard index,
    - `7` for Jaccard index with IDF token weights,
    - `9` for Dice Coefficient,
    - `10` for Dice Coefficient with IDF token weights,
    - `12` for Edit Similarity,
    - `13` for Leader Clustering,
    - `14` for Refined Leader Clustering (i.e. Leader Clustering with the verification stage),
    - `17` for DBSCAN,
    - `18` for Refined DBSCAN (i.e. DBSCAN with the verification stage),
    - `19` for Hierarchical (Agglomerative) Clustering,
    - `20` for Refined Hierarchical (Agglomerative) Clustering (i.e. Agglomerative Clustering with the verification stage),


  * In line 67 the user selects the input dataset. In particular:
    - For the PriceRunner repository the acceptable values are:
      1. `cpus` for CPUs,
      2. `cameras` for Digital Cameras,
      3. `dishwashers` for Dishwashers,
      4. `microwaves` for Microwave Ovens,
      5. `mobile` for Mobile Phones,
      6. `refrigerators` for Fridges, Freezers, and Refrigerators,
      7. `tvs` for Televisions,
      8. `washers` for Washing Machines
      9. `aggregate` for the aggregate dataset which includes all the above products
    - For the Skroutz repository:
      1. `acs` for Air Conditioners,
      2. `batetries` for Car Batteries,
      3. `cookers` for Cookers & Ovens,
      4. `cpus` for CPUs,
      5. `cameras` for Digital Cameras,
      6. `refrigerators` for Fridges, Freezers, and Refrigerators,
      7. `tvs` for Televisions,
      8. `watches` for Mens and Womens Watches
      9. `aggregate` for the aggregate dataset which includes all the above products


This code is completely freeware and it should be used for  scientific and non-commercial purposes.

Leonidas Akritidis, 17/06/2019

