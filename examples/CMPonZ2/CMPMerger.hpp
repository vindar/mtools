/***********************************************
* CMPMerger.hpp
* 
* Compute the CMP on a weighted graph.
*
* The graph must have it site type T derive from 
* the CMPHook<T> base class hook and must also 
* implement the 3 methods:    
* - 'int nbneighbour() const'
* - 'T * neighbour(int index) const'
* - 'double radius() const'
***********************************************/
#pragma once

#include <set>
#include <list>



/**
 * Forward declaration of a CMP cluster structure.
 **/
template<typename T> struct CMPCluster;


/**
* Typedef for an iterator on a list of CMP clusters. 
**/
template<typename T> using cmpit = typename std::list< CMPCluster<T> >::iterator;


/**
 * CMP cluster structure.
 *
 * @tparam  T   The type of site of the graph. 
 **/
template<typename T> struct CMPCluster
    {
    CMPCluster(T * site) : weight(site->radius()), size(1), first(site), last(site), listSons(), listFathers(), height(0) {}

    double  weight; // total weight of the cluster
    uint64  size;   // number of sites in the cluster
    T *     first;  // pointer to the first site of the cluster
    T *     last;   // pointer to the last site in the cluster
    mutable std::list<cmpit<T> > listSons;    // list of sons (clusters that descend from this cluster)
    mutable std::list<cmpit<T> > listFathers; // list of father (clusters which contain this cluster in their action radius)
    mutable uint64 height;                    // height of the cluster in the cluster hierarchy
    };


/**
 * Comparison operator (less) between clusters iterators: sorted first by weight, then by size,
 * and finally by memory address.
 *
 * @tparam  T   Type of site of the graph.
 **/
template<typename T> struct compareCMPCluster
    {
    inline bool operator()(cmpit<T> C1, cmpit<T> C2) const
        {
        if (C1->weight < C2->weight) { return true; } else if (C1->weight > C2->weight) { return false; }
        if (C1->size < C2->size) { return true; } else if (C1->size > C2->size) { return false; }
        return (((size_t)(&(*C1))) < ((size_t)(&(*C2))));
        }
    };


// forward declaration
template<typename T> class CMPMerger;

/**
 * CMP Hook structure. Any site of type T must have this struct as a base and implement the 3
 * methods:
 * 
 * - 'int nbneighbour() const'  (return the number of neighbour of the site)
 * - 'T * neighbour(int index) const' (return a pointer to a given neighour of the site, 0-indexed).  
 * - 'double radius() const' (return the radius/weight of the site)
 * 
 * @tparam  T   The forward site type.
 * @tparam  NBCOLOR Number of colors associated with each site (default = 1).
 **/
template<typename T,int NBCOLOR = 1> struct CMPHook
    {

    friend class CMPMerger<T>; // friend class

    public:

        /**
        * Constructor
        **/
        CMPHook() : _cmp_tag(0), _cmp_next(nullptr), _cmp_prev(nullptr), _cmp_color{ RGBc::c_TransparentWhite } //,_frontierTab(nullptr) 
        {}

        /**
        * Destructor
        **/
        ~CMPHook() 
            { 
            //delete _frontierTab; 
            }

        /**
        * Return an iterator to the cluster containing this site in the clusterSet
        **/
        inline cmpit<T> CMP_cluster() const { return _cmp_cluster; }

        /**
         * Total weight of the cluster containing this site
         **/
        inline double CMP_weight() const { auto r = ((T*)(this))->radius(); if (r < 1.0) return r; else return _cmp_cluster->weight; }

        /**
         * Number of sites in the cluster containing this site
         **/
        inline double CMP_size() const { if (((T*)(this))->radius() < 1.0) return 1; else return _cmp_cluster->size; }

        /**
        * Pointer to the first site of the cluster containing this site
        **/
        inline T * CMP_first() const { if (((T*)(this))->radius() < 1.0) return ((T*)this); else return _cmp_cluster->first; }

        /**
        * Pointer to the last site of the cluster containing this site
        **/
        inline T * CMP_last() const { if (((T*)(this))->radius() < 1.0) return ((T*)this); else return _cmp_cluster->last; }

        /**
        * Pointer to the next site in the same cluster (nullptr if none)
        **/
        inline T * CMP_next() const { return _cmp_next; }

        /**
        * Pointer to the previous site in the same cluster (nullptr if none)
        **/
        inline T * CMP_previous() const { return _cmp_prev; }


        /**
         * Reference to the ith color (0-indexed) associated with this site (default color = transparent
         * white).
         **/
        inline RGBc & CMP_color(int index = 0) { return _cmp_color[index]; }

    private:

        uint64          _cmp_tag;       // used for the exploration trail
        T *             _cmp_next;	    // previous site in the cluster
        T *             _cmp_prev;	    // next site in the cluster
        cmpit<T>        _cmp_cluster;   // pointer to the cluster structure    

        RGBc           _cmp_color[NBCOLOR];     // the color associated with the site. 
    };




/**
 * CMP Merger class. Compute the CMP of a graph.
 *
 * @tparam  T   Site type parameter, must have CMPHook<T> as base class and must implement 
 *              the 3 methods:
 *              - int nbneighbour()   
 *              - T* neighbour()   
 *              - double radius()
 **/
template<typename T> class CMPMerger
    {


    public:

        /**
         * Constructor. Initialize the graph and compute its CMP
         * 
         * After the constructor return the CMP is constructed the clusterset contain all clusters of
         * the CMP ordered by weight. The cluster structure is well formed and contain in particular the
         * oriented graph structure on the clusters.
         * 
         * The only members which are not yet set (and thus remain to their default values) are the color
         * of the sites.
         *
         * @param [in,out]  root    the root site of the graph.
         **/
        CMPMerger(T * root) : ctag1(1), boundary1(&boundA), boundary2(&boundB), _graphSize(0),_absHeight(0),_maxHeight(0),_maxHeight2(0),_maxSize(0),_maxSize2(0),_maxWeight(0),_maxWeight2(0),_nbtrivialcluster(0),_nbnonatomiccluster(0),_nbisolatedcluster(0)
            {
            boundary1->push_back(root);
            root->_cmp_tag = ctag1; // mark as visited
            while (boundary1->size() > 0)
                { // boundary1 contain all the sites at distance exactly d from startNode
                boundary2->clear(); // clear the vector in which we save the next boundary
                for (size_t i = 0; i < boundary1->size(); i++) // iterate over all those sites
                    {
                    T * site = (*boundary1)[i]; // the current site                    
                    clusterList.push_front(CMPCluster<T>(site));  // insert a new cluster in the list of cluster
                    site->_cmp_cluster = clusterList.begin();     // attach the cluster to the site
                    clusterActiveSet.insert(clusterList.begin()); // insert in the active set
                    _graphSize++; // increase the graph size                  
                    int nbc = site->nbneighbour(); // number of neighour
                    for (int k = 0; k < nbc; k++)
                        {
                        T * N = site->neighbour(k);
                        if (N->_cmp_tag != ctag1) { N->_cmp_tag = ctag1; boundary2->push_back(N); } // not yet explored so we add it
                        }
                    }
                swapBoundaries();
                }
            // initialization completed. We can now perform the CMP. 
            while (clusterActiveSet.size() > 1) // keep iterating while there is at least two active clusters. 
                {
                exploreAround(); // explore the activation ball around the first cluster in the active set 
                }
            clusterActiveSet.clear();   // clear the cluster list 
            for (auto it = clusterList.begin(); it != clusterList.end(); ++it) { clusterActiveSet.insert(it); } // recreate the set with all the clusters ordered by size. 
            for (auto it = clusterActiveSet.begin(); it != clusterActiveSet.end(); ++it) 
                { 
                if ((*it)->weight < 1.0) { _nbtrivialcluster++; } else { if ((*it)->size > 1) { _nbnonatomiccluster++; } else { _nbisolatedcluster++; } } // count cluster type
                makeClusterLinks(*it); // create the links in the set of cluster and compute the heights
                } 
            auto maxit = clusterActiveSet.rbegin();
            _maxHeight = (*maxit)->height;
            _maxSize   = (*maxit)->size;
            _maxWeight = (*maxit)->weight;
            (++maxit);
            if (maxit != clusterActiveSet.rend())
                {
                _maxHeight2 = (*maxit)->height;
                _maxSize2   = (*maxit)->size;
                _maxWeight2 = (*maxit)->weight;
                }
            }


        /** Destructor. */
        ~CMPMerger() {}


        /**
         * Query the graph size.
         *
         * @return  The number of vertex in the graph.
         **/
        uint64 graphSize() const { return _graphSize;  }


        /**
        * Return the set of clusters. Clusters are ordered by weight (then size if equality ). 
        *
        * @return  Set of pointers to clusters of the CMP. 
        **/
        std::set< cmpit<T>, compareCMPCluster<T> > & clusterSet() { return clusterActiveSet; }


        /**
         * Maximum height of a cluster (size of the longest descending chain from this cluster).
         * Can be different from the height of the largest cluster !
         **/
        uint64 absoluteHeight() const { return _absHeight; }


        /**
        * Height of the largest cluster
        **/
        uint64 maxHeight() const { return _maxHeight; }


        /**
        * Height of the second largest cluster
        **/
        uint64 maxHeight2() const { return _maxHeight2; }


        /**
        * Size (number of site) of the largest cluster.
        **/
        uint64 maxSize() const { return _maxSize; }


        /**
        * Size (number of site) of the second largest cluster.
        **/
        uint64 maxSize2() const { return _maxSize2; }


        /**
        * Weight of the largest cluster.
        **/
        double maxWeight() const { return _maxWeight; }


        /**
        * Weight of the second largest cluster.
        **/
        double maxWeight2() const { return _maxWeight2; }


        /**
        * Return a pointer to the largest cluster.
        **/
        cmpit<T> largestCluster() const { return (*(clusterActiveSet.rbegin())); }


        /**
        * Return a pointer to the second largest cluster.
        **/
        cmpit<T> secondCluster() const { auto it = (clusterActiveSet.rbegin())++; return (*it); }


        /**
        * Return true if the master cluster is a father to every other cluster of the CMP.
        *
        * @return  true if the lagest cluster contain every other one in its radius.
        **/
        bool isMasterCluster() const { return((largestCluster()->listSons.size()) == (nbClusters() - 1)); }


        /**
         *Count the number of clusters
         *
         * @return  the number of clusters in the CMP
         **/
        uint64 nbClusters() const { return clusterActiveSet.size(); }


        /**
         * Return the number of trivial clusters i.e. cluster consisting of only 1 site with radius
         * strictly less than 1.0.
         *
         * @return  The number of trivial isolated sites.
         **/
        uint64 nbTrivialClusters() const { return _nbtrivialcluster; }


        /**
         * Return the number of isolated clusters i.e. clusters with only 1 site in them but whose
         * radius is at least 1 (i.e. could potentially be in a cluster).
         *
         * @return  An uint64.
         **/
        uint64 nbIsolatedClusters() const { return _nbisolatedcluster; }


        /**
        * Count the number of non-trivial clusters
        *
        * @return  the number of clusters in the CMP that have at least two sites.
        **/
        uint64 nbNonAtomicClusters() const { return _nbnonatomiccluster; }


        /**
         * Return some information about the CMP. 
         *
         * @return  A std::string that represents this object.
         **/
        std::string toString() const
            {
            std::string s("CMP\n");
            s += "- graph size: " + mtools::toString(graphSize()) + "\n";
            s += "- number of clusters: " + mtools::toString(nbClusters()) + "\n";
            s += "   - number of trivial site: " + mtools::toString(nbTrivialClusters()) + "\n";
            s += "   - number of single clusters: " + mtools::toString(nbIsolatedClusters()) + "\n";
            s += "   - number of compounded clusters: " + mtools::toString(nbNonAtomicClusters()) + "\n";
            s += "- maximum height: " + mtools::toString(maxHeight()) + "\n";
            auto it = clusterActiveSet.rbegin();
            s += "- Largest cluster:\n";
            if (isMasterCluster()) { s += "    - *** MASTER CLUSTER : contains ever other cluster in its action radius ***\n"; }
            s += "    - size: " + mtools::toString((*it)->size) + "\n";
            s += "    - weight: " + mtools::toString((*it)->weight) + "\n";
            s += "    - height: " + mtools::toString((*it)->height) + "\n";
            if (clusterActiveSet.size() > 1) 
                {
                it++;
                s += "- 2nd largest cluster:\n";
                s += "    - size: " + mtools::toString((*it)->size) + "\n";
                s += "    - weight: " + mtools::toString((*it)->weight) + "\n";
                s += "    - height: " + mtools::toString((*it)->height) + "\n";
                }
            if (clusterActiveSet.size() > 2)
                {
                it++;
                s += "- 3th largest cluster:\n";
                s += "    - size: " + mtools::toString((*it)->size) + "\n";
                s += "    - weight: " + mtools::toString((*it)->weight) + "\n";
                s += "    - height: " + mtools::toString((*it)->height) + "\n";
                }
            return s;
            }


        /**
         * Color every site of a cluster with a given color using the over operation.
         *
         * @param   C       The cluster to color
         * @param   color   The color to use
         * @param   colorIndex      Zero-based index of the color.
         **/
        void colorCluster(cmpit<T> C, RGBc color, int colorIndex = 0)
            {
            T* p = C->first;
            while (p != nullptr) { p->_cmp_color[colorIndex] = color.over(p->_cmp_color[colorIndex]); p = p->_cmp_next; }
            }


        /**
         * Color the action radius of a cluster. Use the over operation.
         *
         * @param   C               The cluster to color.
         * @param   color           The color to use.
         * @param   includeCluster  true to color also the cluster and false otherwise.
         * @param   colorIndex      Zero-based index of the color.
         **/
        void colorRadius(cmpit<T> C, RGBc color, bool includeCluster, int colorIndex = 0)
            {
            const double W = C->weight;   // the cluster action radius. 
            ctag1++; // new unique tag
            boundary1->clear();         // clear the vector
            T * p = C->first;
            while (p != nullptr) // loop over the cluster sites
                {
                p->_cmp_tag = ctag1; // tag them
                if (includeCluster) { p->_cmp_color[colorIndex] = color.over(p->_cmp_color[colorIndex]); } // color them if needed
                p = p->_cmp_next; 
                } 
            p = C->first;
            while (p != nullptr)
                { // we iterate again over the site of the cluster
                int nbc = p->nbneighbour(); // number of neighour of p
                for (int k = 0; k < nbc; k++)
                    { // iterate over the neighbours
                    T * q = p->neighbour(k);
                    if (q->_cmp_tag != ctag1) { q->_cmp_tag = ctag1; boundary1->push_back(q); } // mark and add the site at distance 1 to boundary 1
                    }
                p = p->_cmp_next;
                }
            double d = 1.0;
            while ((d <= W) && (boundary1->size() > 0))
                { // boundary1 contain all the sites at distance exactly d from startNode
                boundary2->clear(); // clear the vector in which we save the next boundary
                for (size_t i = 0; i < boundary1->size(); i++) // iterate over all those sites
                    {
                    T * q = (*boundary1)[i]; // the current site
                    q->_cmp_color[colorIndex] = color.over(q->_cmp_color[colorIndex]); // color it
                    if (((double)(d + 1)) <= W)
                        { // we add the neighbours to the next boundary
                        int nbc = q->nbneighbour(); // number of neighour
                        for (int k = 0; k < nbc; k++) 
                            {
                            T * N = q->neighbour(k);
                            if (N->_cmp_tag != ctag1) { N->_cmp_tag = ctag1; boundary2->push_back(N); } // not yet explored so we add it
                            }
                        }
                    }
                swapBoundaries();
                d += 1.0;
                }
            return;
            }


        /**
         * Color the stabilisator of a cluster. Use the over operation.
         *
         * @param   C               The cluster to color.
         * @param   color           The color to use.
         * @param   includeCluster  true to color also the cluster and false otherwise.
         * @param   colorIndex      Zero-based index of the color.
         **/
        void colorStabilizer(cmpit<T> C, RGBc color, bool includeCluster, int colorIndex = 0)
            {
            ctag1++; // new ctag
            std::set< cmpit<T>, compareCMPCluster<T> > cSet;
            recInsertChild(cSet, C); // construct the list of all cluster which descend from Cit
            for (auto it = cSet.begin(); it != cSet.end(); it++)
                { // iterate over all the site of all cluster which constitute the stabiliser
                T * p = (*it)->first;
                while (p != nullptr) 
                    {
                    if (p->_cmp_tag != ctag1)
                        { // not yet visited
                        if ((p->_cmp_cluster != C) || (includeCluster))
                            {
                            p->_cmp_color[colorIndex] = color.over(p->_cmp_color[colorIndex]);
                            }
                        }
                    p->_cmp_tag = ctag1; 
                    p = p->_cmp_next; 
                    } 
                }
            /*
            // ok, the whole stabilizer is tagged with ctag1
            ctag1++; // next tag
            boundary1->clear();     // clear the vector
            T * p = Cit->first;  p->_cmp_tag = ctag1; boundary1->push_back(p); // starting point of exploration
            while (boundary1->size() > 0)
                {
                boundary2->clear(); // clear the vector in which we save the next boundary
                for (size_t i = 0; i < boundary1->size(); i++) // iterate over all those sites
                    {
                    T * q = (*boundary1)[i]; // the current site
                    q->_cmp_color = (blendover ? color.over(q->_cmp_color) : color);   // set the color
                    for (int j = 0;j < q->nbneighbour(); j++)
                        {
                        T * N = q->neighbour(j);
                        if (N->_cmp_tag < (ctag1 - 1)) { if (q->CMP_edge(j) < edgeval) q->CMP_edge(j) = edgeval; } // set the edges values
                        else if (N->_cmp_tag == (ctag1 - 1)) { N->_cmp_tag = ctag1; boundary2->push_back(N); } // not yet explored so we add it
                        }
                    }
                swapBoundaries();
                }
            */
            }


        /**
         * Color every site of the graph. Use the over operation.
         *
         * @param   color           The color to use.
         * @param   colorIndex      Zero-based index of the color.
         **/
        void colorGraph(RGBc color, int colorIndex = 0)
            {
            ctag1++; // new tag
            T * root = (*(clusterActiveSet.begin()))->first; // take any site as the root
            root->_cmp_tag = ctag1;     // mark it
            boundary1->push_back(root); // and push it in the boundary
            while (boundary1->size() > 0)
                { 
                boundary2->clear(); // clear the vector in which we save the next boundary
                for (size_t i = 0; i < boundary1->size(); i++) // iterate over all those sites
                    {
                    T * site = (*boundary1)[i]; // the current site
                    site->_cmp_color[colorIndex] = color.over(site->_cmp_color[colorIndex]); // color it
                    int nbc = site->nbneighbour(); // number of neighour
                    for (int k = 0; k < nbc; k++)
                        {
                        T * N = site->neighbour(k);
                        if (N->_cmp_tag != ctag1) { N->_cmp_tag = ctag1; boundary2->push_back(N); } // not yet explored so we add it
                        }
                    }
                swapBoundaries();
                }
            }



        /**
         * Return a color from the jet palette according to the cluster size wrt the max cluster size
         **/
        RGBc rgbSize(cmpit<T> C) { return RGBc::jetPalette((*C).size, 1, _maxSize); }


        /**
        * Return a color from the jet palette according to the cluster size wrt the second max cluster size
        **/
        RGBc rgbSize2(cmpit<T> C) { return RGBc::jetPalette((*C).size, 1, _maxSize2); }


        /**
        * Return a color from the jet palette according to the cluster weight wrt the max cluster weight
        **/
        RGBc rgbWeight(cmpit<T> C) { double m = _maxWeight; if (m <= 0.0) { m = 1; } return(RGBc::jetPalette((*C).weight/m)); }


        /**
        * Return a color from the jet palette according to the cluster weight wrt the second max cluster weight
        **/
        RGBc rgbWeight2(cmpit<T> C) { double m = _maxWeight2; if (m <= 0.0) { m = 1; } return(RGBc::jetPalette((*C).weight/m)); } 


        /**
        * Return a color from the jet palette according to the cluster height wrt the max cluster height
        **/
        RGBc rgbHeight(cmpit<T> C) { return RGBc::jetPalette((*C).height, 0, _maxHeight); }






        /**
        * Go to the next element in the set of cluster satisfying certain properties.
        * up: from smaller to larger clusters.
        * Used for the loop macro
        **/
        typename std::set< cmpit<T>, compareCMPCluster<T> >::iterator _nextCmpClusterUp(typename std::set< cmpit<T>, compareCMPCluster<T> >::iterator it, bool skiptrivial, bool skipisolated, bool skiplargest)
            {
            while (1)
                {
                if (it == clusterActiveSet.end()) return it;
                auto nit = it; ++nit; if ((nit == clusterActiveSet.end()) && (skiplargest)) { return nit; }
                if ((*it)->size > 1) return it;
                if ((*it)->weight >= 1.0) { if (!skipisolated) return it; } else { if (!skiptrivial) return it; }
                it = nit;
                }
            }


        /**
        * Go to the next element in the set of cluster satisfying certain properties.
        * down: from larger to smaller clusters.
        * Used for the loop macro
        **/
        typename std::set< cmpit<T>, compareCMPCluster<T> >::reverse_iterator _nextCmpClusterDown(typename std::set< cmpit<T>, compareCMPCluster<T> >::reverse_iterator it, bool skiptrivial, bool skipisolated, bool skiplargest)
            {
            while (1)
                {
                if (it == clusterActiveSet.rend()) return it;
                if ((it == clusterActiveSet.rbegin()) && (skiplargest)) { ++it; continue; }
                if ((*it)->size > 1) return it;
                if ((*it)->weight >= 1.0) { if (!skipisolated) return it; } else { if (!skiptrivial) return it; }
                ++it;
                }
            }



    private:


        /* no copy */
        CMPMerger(const CMPMerger &) = delete;
        CMPMerger & operator=(const CMPMerger &) = delete;


        /**
         * Recursively construct the set of all descendent of a given cluster
         **/
        void recInsertChild(std::set< cmpit<T>, compareCMPCluster<T> > & cSet, cmpit<T> Cit)
            {
            cSet.insert(Cit);
            for (auto it = Cit->listSons.begin(); it != Cit->listSons.end(); ++it) { recInsertChild(cSet, *it); }
            }


         /**
          * Explore the activation ball around the first cluster in the set of active clusters.
          * Perform a merging if possible, otherwise remove the cluster from the active set.
          **/
         void exploreAround()
            {
            auto it = clusterActiveSet.begin(); // get the smallest active cluster
            const double W = (*it)->weight;   // the cluster action radius. 
            if (W < 1.0) { clusterActiveSet.erase(it); return; } // cannot merge, remove it from the list of active clusters and return
            ctag1++; // new unique tag
            boundary1->clear();         // clear the vector
            T * p = (*it)->first;
            while (p != nullptr) { p->_cmp_tag = ctag1; p = p->_cmp_next; } // tag all sites of C 
            p = (*it)->first;
            while (p != nullptr)
                { // we iterate again over the site of the cluster
                int nbc = p->nbneighbour(); // number of neighour of p
                for(int k = 0; k < nbc; k++)
                    { // iterate over the neighbours
                    T * q = p->neighbour(k);
                    if (q->_cmp_tag != ctag1) { q->_cmp_tag = ctag1; boundary1->push_back(q); } // mark and add the site at distance 1 to boundary 1
                    }
                p = p->_cmp_next;
                }
            double d = 1.0;
            while((d <= W)&&(boundary1->size() > 0))
                { // boundary1 contain all the sites at distance exactly d from startNode
                boundary2->clear(); // clear the vector in which we save the next boundary
                for (size_t i = 0; i < boundary1->size(); i++) // iterate over all those sites
                    {
                    T * q = (*boundary1)[i]; // the current site
                    if (q->CMP_weight() >= d)
                        { // ok, we merge
                        merge((*it), q->_cmp_cluster);
                        return;
                        }
                    if (((double)(d + 1)) <= W)
                        { // we add the neighbours to the next boundary
                        int nbc = q->nbneighbour(); // number of neighour
                        for (int k = 0; k < nbc; k++)
                            {
                            T * N = q->neighbour(k);
                            if (N->_cmp_tag != ctag1) { N->_cmp_tag = ctag1; boundary2->push_back(N); } // not yet explored so we add it
                            }
                        }
                    }
                swapBoundaries();
                d+= 1.0;
                }
            clusterActiveSet.erase(it); // remove the cluster from the list of active clusters. 
            return;
             }


        /**
         * Merge two clusters and add the resulting cluster in the set of active clusters
         **/
       inline void merge(cmpit<T> C1, cmpit<T> C2)
            {
            MTOOLS_ASSERT(C1 != C2);
            clusterActiveSet.erase(C1); // remove both clusters from the active set
            clusterActiveSet.erase(C2); //
            if (C1->size > C2->size)
                {
                C1->last->_cmp_next = C2->first; // chain C2 after C1
                C2->first->_cmp_prev = C1->last; //
                T * p = C2->first; while (p != nullptr) { p->_cmp_cluster = C1; p = p->_cmp_next; } // and update the cluster pointers
                C1->weight = C1->weight + C2->weight;   // update info
                C1->size = C1->size + C2->size;         //
                C1->last = C2->last;                    // 
                clusterList.erase(C2);                  // erase the cluster who disapeared
                clusterActiveSet.insert(C1);            // and put back the new cluster in the active set
                return;
                }
            C2->last->_cmp_next = C1->first; // chain C1 after C2
            C1->first->_cmp_prev = C2->last; //
            T * p = C1->first; while (p != nullptr) { p->_cmp_cluster = C2; p = p->_cmp_next; } // and update the cluster pointers
            C2->weight = C1->weight + C2->weight;   // update info
            C2->size = C1->size + C2->size;         //
            C2->last = C1->last;                    // 
            clusterList.erase(C1);                  // erase the cluster who disapeared
            clusterActiveSet.insert(C2);            // and put back the new cluster in the active set
            return;
            }


       /**
       * Create the list of children of a given cluster and compute its heights. 
       **/
       void makeClusterLinks(const cmpit<T> & Cit)
           {
           double W = Cit->weight;   // the cluster action radius. 
           if (W < 1.0) { return; } // bottom site: nothing to do
           ctag1++; // new unique tag
           std::set< cmpit<T>, compareCMPCluster<T> > cSet; // set of children
           boundary1->clear();     // clear the vector
           T * p = Cit->first;
           while (p != nullptr) { p->_cmp_tag = ctag1; boundary1->push_back(p); p = p->_cmp_next; } // insert and tag all sites of the cluster  
           double d = 0.0;
           while ((d <= W) && (boundary1->size() > 0))
               { // boundary1 contain all the sites at distance exactly d from startNode
               boundary2->clear(); // clear the vector in which we save the next boundary
               for (size_t i = 0; i < boundary1->size(); i++) // iterate over all those sites
                   {
                   T * q = (*boundary1)[i]; // the current site
                   if (q->_cmp_cluster != Cit) { cSet.insert(q->_cmp_cluster); }
                   if (((double)(d + 1)) <= W)
                       { // we add the neighbours to the next boundary
                       int nbc = q->nbneighbour(); // number of neighour
                       for (int k = 0; k < nbc; k++)
                           {
                           T * N = q->neighbour(k);
                           if (N->_cmp_tag != ctag1) { N->_cmp_tag = ctag1; boundary2->push_back(N); } // not yet explored so we add it
                           }
                       }
                   }
               swapBoundaries();
               d += 1.0;
               }
           // cSet now contain the set of child cluster of Cit
           uint64 h = 0;
           for (auto it = cSet.begin(); it != cSet.end(); ++it)
               {
               if (((*it)->height + 1) > h) { h = (*it)->height + 1; }
               Cit->listSons.push_back(*it);    // add the the list of son
               (*it)->listFathers.push_back(Cit); // and define C as a father. 
               }
           Cit->height = h;
           if (h > _absHeight) { _absHeight = h; }
           return;
           }


       int64 ctag1; // current tag

       std::list<CMPCluster<T> > clusterList;                          // list of all clusters (some may be null)
       std::set< cmpit<T>, compareCMPCluster<T> > clusterActiveSet;   // set of cluster pointer sorted by size and activity

       std::vector<T*> boundA, boundB;
       std::vector<T*> *boundary1, *boundary2;
       inline void swapBoundaries() { auto temp = boundary1; boundary1 = boundary2; boundary2 = temp; }

       uint64 _graphSize;
       uint64 _absHeight;
       uint64 _maxHeight;
       uint64 _maxHeight2;
       uint64 _maxSize;
       uint64 _maxSize2;
       double _maxWeight;
       double _maxWeight2;
       uint64 _nbtrivialcluster;
       uint64 _nbnonatomiccluster;
       uint64 _nbisolatedcluster;

    };



    /**
    * MACRO: Loop over the clusters of the CMP, going from smaller to largest clusters
    **/
    #define CMP_CLUSTERLOOP_UP(_cmp_, _it_,_skiptrivial_,_skipisolated_,_skiplargest_)  for (auto _it_ = (_cmp_)._nextCmpClusterUp((_cmp_).clusterSet().begin(), _skiptrivial_, _skipisolated_, _skiplargest_) ; _it_ != (_cmp_).clusterSet().end(); it = (_cmp_)._nextCmpClusterUp(++_it_, _skiptrivial_, _skipisolated_, _skiplargest_) )


    /**
    * MACRO: Loop over the clusters of the CMP, going from largest to smaller clusters
    **/
    #define CMP_CLUSTERLOOP_DOWN(_cmp_, _it_,_skiptrivial_,_skipisolated_,_skiplargest_)  for (auto _it_ = (_cmp_)._nextCmpClusterDown( (_cmp_).clusterSet().rbegin(), _skiptrivial_, _skipisolated_, _skiplargest_); _it_ != (_cmp_).clusterSet().rend(); it = (_cmp_)._nextCmpClusterDown(++_it_, _skiptrivial_, _skipisolated_, _skiplargest_) )


/* end of file */



