/** @file RWtreeGraph.hpp */
//
// Copyright 2015 Arvind Singh
//
// This file is part of the mtools library.
//
// mtools is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with mtools  If not, see <http://www.gnu.org/licenses/>.


#pragma once

#include "../misc/internal/mtools_export.hpp"
#include "../misc/error.hpp"
#include "../misc/misc.hpp"
#include "../misc/stringfct.hpp"

#include <string>


/****************************************************************************************************
* TEMPLATE CLASS RW_TreeGraph                                              Version 3.2, Vindar 2011 *
*                                                                                                   *
* Used for representing a walk on a regular or random tree. Each Node contain an element of some 
* template object T (whichh must be a simple struct) 
* 
*                                                                                                   *
* CLASS PARAMETERS:                                                                                 *
*    - T         : template parameter representing the object contained in each node of the tree    *
*                  must be struct type.                                                             *
*                  optimized for sizeof(T) = 16 in 32 bit mode and sizzeof(T) = 4 in 64 bit mode    *
*    - initroot  : template function for initializing the root of the tree                          *
*    - initfct   : template function for intializing a node of the tree                             *
*    - SizeMB    : size in MB of memory to allocate ofr the object                                  *
*    - ratiokept : ratio of site kept when doing cleanup (e.g. 0.7 = 70% site kept)                 *
*                                                                                                   *                                                                                         *
*    - the number of children of each node is between 0 and 65535                                   *
*    - efficient access to the T value of the actual position and neigbour (very fast)              *
*    - keep in mind the number of site visited by the walk and the number of step done.             *
*    - when cleanup occur, only the sites father away from the actual position are destroyed        *
*                                                                                                   *
* EXAMPLE :                                                                                         *
*                                                                                                   *   
    // Simulation of a Digging RW on a binary tree
    #include "mylib_all.h"

    // binary tree, initial value 0
    void initNode(const char & fatherValue, uint16 NoBro, uint16 NbBro, uint64 depth,
                  char & siteValue , uint16 & siteNbSon)  {siteValue = 0; siteNbSon = 2;}

    // same for the root
    void initRoot(char & rootValue , uint16 & rootNbSon) {rootValue = 0; rootNbSon = 2;}

    int main(void)
        { 
        // T of type char, 1GB of RAM, keep 70% of sites at cleanup
        RW_TreeGraph<char,initRoot,initNode> G(1024,0.70); 
        MT2004_64 gen; ExTab tab(1000000); //RNG and a tab to keep the result
        double p = 0.59; // p = 0.5857864 critical value
        try {
            for(int i = 0;i< 200000000; i++) // 2*10^8 step
                {
                tab.Add((double)G.Depth());
                if (G.Value() == 0) {G.Value() = 1; G.MoveFather();}
                else
                    {
                    double a = Unif(gen);
                    if (a < (1.0-p))  {G.MoveFather();} 
                    else {if (a < ((1-p) + p/2)) {G.MoveSon(0);}  else {G.MoveSon(1);}}
                    }
                }
            }
        catch(mylib::customexc & e) {out << "exception caught !\n" << e.what() << "\n\n";}
        Plotter P; P.insert(tab.PlotMin()); P.insert(tab.PlotMax()); P.insert(tab.PlotMed());
        P.setrange(); P.plot(); P.removeall();
        out << G.Stats();
        return 0;
        }
*                                                                                                  *
* standalone, no cpp file                                                                          *
****************************************************************************************************/


namespace mtools
{


/*************************************************************************************************************
 * The template class RW_TreeGraph class                                                                     *
 *                                                                                                           *
 * Template parameters:  RW_TreeGraph<T,initroot,initnode>                                                   *
 *                                                                                                           *
 * T: the class/struct associated with each site. Must be a simple object, the ctor/dtor are not correctly   *
 *      called.                                                                                              *
 *                                                                                                           *
 * void initroot(T & valueroot,uint16 & nbchildren)                                                          *
 *    - valueroot : (must be modified) to equal to initial value associated with the root node               *                             
 *    - nbchildren: (must be modified) to reflect the nb of children of the root                             *
 *                                                                                                           *
 * void initfct(const T & valfather,uint16 noBro,uint16 nbBro,uint64 depth,T & value, uint16 & nbchildren)   *
 *    - valfather : (for info) the value of the father node                                                  *
 *    - noBro     : (for info) this node is the son number noBro of its father (in [0,nbBro[ )               *
 *    - nbBro     : (for info) this node is part of a family of nbBro brothers                               *          
 *    - depth     : (for info) the absolute depth of this site (always >0 since the root has a special fct)  *
 *    - value     : (must be modified) to equal to initial value of this node                                *
 *    - nbchildren: (must be modified) to reflect the nb of children of this node                            *
 *                                                                                                           *
 ************************************************************************************************************/
template<typename T, void initrootfct(T &,uint16 &), void initfct(const T &,uint16,uint16,uint64,T &,uint16 &)> 
class RW_TreeGraph
{

    static const size_t SITE_DESTROYED = ((size_t)0);
    static const size_t SITE_NOT_CREATED = ((size_t)1);
    static const size_t SITE_FIRST_POS = ((size_t)2);


public:

    /*****************************************************
     * Construct the object                              *
     * - sizemb: approx size in MB of memory to use.     *
     * - ratiokept: in (0,1), the ratio of site we keep  *
     *              when doing a cleanup to free some    *
     *              memory (default = half the sites)    *
     *                                                   *
     * Raise an exception if the parameter are invalid   *
     * or if we run out of memory                        *
     ****************************************************/
    RW_TreeGraph(uint32 SizeMB,double ratiokept = 0.5)    {init(SizeMB,ratiokept);}

  
    /*****************************************************
     * Reset the object in the same state as just after  *
     * its construction.  (much faster than recreating a *
     * whole new object)                                 *
     ****************************************************/
    void Reset()                {doreset();}


    /*****************************************************
     * Move toward the father                            *
     * - do nothing if already at the root               *
     * - raise an exception if we run out of memory      *
     ****************************************************/
    inline void MoveFather()
    {
        steps++;
        size_t f = tab[tab_pos].father(); 
        if (f == SITE_NOT_CREATED) {return;} // we are at the root
        if (tab[f].father() == SITE_DESTROYED)   {MTOOLS_ERROR("TreeGraph::MoveFather(), the father was previously destroyed !");} // cannot go back
        tab_pos = f; --depth;
        return;
    }


    /***************************************************** 
     * Move toward the no th son                         *
     * the son are indexed from 0 to NbSon-1             *
     * - raise an exception if no >= NbSon()             *
     * - raise an exception if we run out of memory      *
     ****************************************************/
    inline void MoveSon(uint16 No)
    {
        steps++;
        if (No >= tab[tab_pos].nbson) {MTOOLS_ERROR("TreeGraph::MoveSon(No), No son too large!");}
        tab_pos = tab[tab_pos].son() + No; ++depth;
        if (tab[tab_pos].flagvis() == false) {tab[tab_pos].setflagvis(); visited++;}
        if (tab[tab_pos].nbson !=0)
            {
            size_t s = tab[tab_pos].son();
            if (s == SITE_DESTROYED) {MTOOLS_ERROR("TreeGraph::MoveSons(), the son was previously destroyed !");}
            if (s == SITE_NOT_CREATED) {CreateSons();}
            }
        return;
    }


    /*****************************************************
    * Query the number of son of the actual position     *
    * THIS METHOD NEVER FAILS                            *
    *****************************************************/
    inline uint16 NbSon() const {return tab[tab_pos].nbson;}


    /*****************************************************
    * Query the depth of the actual position             *
    * - 0 for the original root of the tree              *
    * THIS METHOD NEVER FAILS                            *
    *****************************************************/
    inline uint64 Depth() const {return depth;}


    /*****************************************************
    * Query the number of steps that the walk has done   *
    * (a jump for the root to the root also count)       *
    * THIS METHOD NEVER FAILS                            *
    *****************************************************/
    inline uint64 NbSteps() const {return steps;}


   /******************************************************
    * Return the total number of site visited by the walk*
    * THIS METHOD NEVER FAILS                            *
    *****************************************************/
    inline uint64 NbVisited() {return visited;}


    /*****************************************************
    * query the value at the actual position             *
    * THIS METHOD NEVER FAILS                            *
    *****************************************************/
    inline T &       Value()          {return tab[tab_pos].value;}
    inline const T & Value() const    {return tab[tab_pos].value;}


    /*****************************************************
    * query the value associated with the father         *
    * - if we are at the root, it is the value of the    *
    *   site itself                                      *
    * THIS METHOD NEVER FAILS                            *
    *****************************************************/
    inline T &       ValueFather()          {if (tab[tab_pos].father() == SITE_NOT_CREATED) {return tab[tab_pos].value;} else {return tab[tab[tab_pos].father()].value;}}
    inline const T & ValueFather() const    {if (tab[tab_pos].father() == SITE_NOT_CREATED) {return tab[tab_pos].value;} else {return tab[tab[tab_pos].father()].value;}}


    /*****************************************************
    * query the value associated with the no th son      *
    * the son are indexed from 0 to NbSon-1              *
    * - raise an exception if no >= NbSon() but otherwise*
    *   it will not fail                                 *
    *****************************************************/
    inline T &       ValueSon(uint16 No)        {if (No >= tab[tab_pos].nbson) {MTOOLS_ERROR("TreeGraph::ValueSon(No), No son too large!");} return tab[tab[tab_pos].son() + No].value;}
    inline const T & ValueSon(uint16 No) const  {if (No >= tab[tab_pos].nbson) {MTOOLS_ERROR("TreeGraph::ValueSon(No), No son too large!");} return tab[tab[tab_pos].son() + No].value;}


    /*****************************************************
    * return the total number of site that have been     *
    * created (this is larger than the total number of   *
    * site that have been visited)                       *
    * (see Visited() for the nb of site visited)         *
    * THIS METHOD NEVER FAILS                            *
    *****************************************************/
    inline uint64   NbCreated()      {return nb_created;}
    
    
    /*****************************************************
    * Return the total number of site that the object can*
    * allocate in memory                                 *
    * THIS METHOD NEVER FAILS                            *
    *****************************************************/
    inline uint64   MemoryMaxSites() {return((uint64)(tab_size-2));}
    

    /*****************************************************
    * Return the number of site in memory currently used *
    * THIS METHOD NEVER FAILS                            *
    *****************************************************/
    inline uint64   MemoryUsedSites() {return((uint64)(tab_free-2));}


    /*****************************************************
    * Return the depth of the memory root                *
    * ie the minimal depth that the walk can go          *
    * THIS METHOD NEVER FAILS                            *
    *****************************************************/
    inline uint64   MemoryRootDepth() {return memrootdepth;}


    /*****************************************************
    * Return the number of cleanup that have been made   *
    * THIS METHOD NEVER FAILS                            *
    *****************************************************/
    inline uint64   NbCleanUp()       {return nbcleanup;}


    /*****************************************************
    * Print some informations concerning the object in a *
    * std::string.                                       *
    * THIS METHOD NEVER FAILS                            *
    *****************************************************/
    std::string Stats()
    {
        std::string s;
        s += "*****************************************************\n";
        s += "TreeGraph object statistics\n\n";
        s += "- Memory allocated         : " + mtools::toString(((sizeof(TreeNode)*tab_size)+(sizeof(size_t)*(tab_size+5)))/(1024*1024)) + "Mb\n";
        s += "- Number of cleanup done   : " + mtools::toString(NbCleanUp()) + "\n";
        s += "- Number of step performed : " + mtools::toString(NbSteps()) + "\n"; 
        s += "- Depth of the memory root : " + mtools::toString(MemoryRootDepth()) + "\n";
        s += "- Depth of the actual pos  : " + mtools::toString(Depth()) + "\n"; 
        s += "- total Nb of site created : " + mtools::toString(NbCreated()) + "\n";
        s += "- total Nb of site visited : " + mtools::toString(NbVisited()) + "\n";
        s += "- Number of nodes in use   : " + mtools::toString(MemoryUsedSites()) + " / " + mtools::toString(MemoryMaxSites()) + " (" + mtools::toString((int)(((double)MemoryUsedSites())/((double)MemoryMaxSites())*100.0)) + "% occupied)\n";
        s += "*****************************************************\n";
        return s;                    
    }

    
    /*****************************************************
     * print the tree in an std::string                  *
     * FOR DEBUGGING PURPOSES ONLY !                     *
     * must provide the conversion function              *
     * char tochar(const T value &) to convert the value *
     * of type T into a char.                            *
     *                                                   *
     * should only be used with very small tree !!!      *
     ****************************************************/
    template<char tochar(const T &)> std::string tostring() {return CreateBox<tochar>(SITE_FIRST_POS).tostring();}


    /*****************************************************
     * destructor                                        *
     ****************************************************/
    ~RW_TreeGraph() {delete [] repart; delete [] tab; return;}


private: 

     /******************
      * Node structure *
      *****************/
    class TreeNode
        {
        private:
        size_t      pfather;
        size_t      pson;
        public:

        /****************
        * number of son *
        ****************/
        uint16      nbson;

        /****************
        * the value     *
        ****************/
        T           value;

        /**************************************
         * getter/setter for the father link  *
         *************************************/
        inline size_t   father()            const        {return(pfather >> 2);}
        inline void     setfather(size_t pos)            {pfather = (pfather&((size_t)3)) | (pos << 2);}

        /**************************************
         * getter/setter for the firstson link*
         *************************************/
        inline size_t   son()               const        {return(pson >> 2);}
        inline void     setson(size_t pos)               {pson = (pson&((size_t)3)) | (pos << 2);}

        /**************************************
         * getter/setter for lastbrother flag *
         *************************************/
        inline bool     lastbrother() const              {return((pfather &((size_t)1))!=0);}
        inline void     setlastbrother()                 {pfather |= ((size_t)1);}

        /**************************************
         * getter/setter for direction flag   *
         *************************************/
        inline bool     dirflag() const                  {return((pfather &((size_t)2))!=0);}
        inline void     setdirflag()                     {pfather |= ((size_t)2);}
        inline void     unsetdirflag()                   {pfather &= (~((size_t)2));}

        /**************************************
         * getter/setter for flag 1           *
         *************************************/
        inline bool     flag1() const                    {return((pson &((size_t)1))!=0);}
        inline void     setflag1()                       {pson |= ((size_t)1);}
        inline void     unsetflag1()                     {pson &= (~((size_t)1));}

        /**************************************
         * getter/setter for the visited flag *
         *************************************/
        inline bool     flagvis() const                  {return((pson &((size_t)2))!=0);}
        inline void     setflagvis()                     {pson |= ((size_t)2);}


        /*************************************
        * - set all flag to 0                *
        * - set the father to fath           *
        * - set the son to SITE_NOT_CREATED  *
        * - do NOT change nbson NOR the value*
        *************************************/
        inline void makesonof(size_t fath)
            {
            pfather = ((fath) << 2);
            pson = ((SITE_NOT_CREATED) << 2);
            }
        };

    /*******************
     * the variables   *
     ******************/
    size_t * repart;    // array for the repartition of the depth
    TreeNode * tab;     // the array of nodes itself
    size_t tab_size;    // size of the array tab
    size_t median;      // size of the median (approx. number of site we keep when we cut the tree)
    size_t tab_free;    // first free index in the array
    size_t tab_pos;     // actual position in the array
    uint64 depth;       // depth of the actual position
    uint64 visited;     // nb of site actually visited by the walk

    uint64 memrootdepth;// depth of the root
    uint64 nbcleanup;   // nb of cleanup done
    uint64 nb_created;  // nb of site created
    uint64 steps;       // nb of step performed by the walk

    /************
    * no copy   *
    ************/
    RW_TreeGraph(const RW_TreeGraph &);
    RW_TreeGraph & operator=(const RW_TreeGraph &);


    /*****************
    * the ctor code  *
    *****************/
    void init(uint32 SizeMB,double ratiokept)
    {
        tab_size = (((size_t)SizeMB)*1024*1024)/(sizeof(TreeNode)+sizeof(size_t));
        if (tab_size < 267000) {MTOOLS_ERROR("TreeGraph::TreeGraph(), SizeMB too small. Must be enough to create at least 267000 nodes !");}
        median = (size_t)(((double)tab_size)*ratiokept);
        if (median < 133000) {median = 133000;}
        if ((median + 133000) > tab_size) {median = tab_size - 133000;}
        try {
            tab = new TreeNode[tab_size];
            repart = new size_t[tab_size+5];
            }
        catch(...) {MTOOLS_ERROR("TreeGraph::TreeGraph(), Out of memory !");}
        if ((tab == NULL)||(repart == NULL)) {MTOOLS_ERROR("TreeGraph::TreeGraph(), Out of memory !");}
        doreset();
        return;
    }


    /*********************
    * the reset code     * 
    *********************/
    void doreset()
    {
        memrootdepth = 0;
        nbcleanup = 0;
        nb_created = 1;
        tab_pos = SITE_FIRST_POS;                  // actual position at the root
        tab_free = tab_pos+1;                                   // first free position
        depth = 0;                                              // depth = 0;
        visited = 1;                                            // visited = 1;
        steps = 0;                                              // no step performed yet.
        tab[tab_pos].makesonof(SITE_NOT_CREATED);  // root father is SITE_NOT_CREATED, and no flag
        tab[tab_pos].setlastbrother();                          // the root is its last brother
        tab[tab_pos].setflagvis();                              // the root is visited
        tab[tab_pos].nbson = 0;                                 // just in case...
        initrootfct(tab[tab_pos].value,tab[tab_pos].nbson);     // initialize the value of the root and its number of children
        if (tab[tab_pos].nbson != 0) {CreateSons();}            // initialize the children if any
        return;
    }


    /**********************************************************************
    * create the son of the actual node                                   *
    * - the son must NOT have been created before (no checking of that)   *
    * - the number of son must NOT be zero (no checking of that)          *
    **********************************************************************/
    inline void CreateSons()
    {
        uint16 n = tab[tab_pos].nbson;
        nb_created += n;
        while((tab_free + ((size_t)n)) > tab_size) {MakeRoom();} // make enough room to accomodate all the children
        tab[tab_pos].setson(tab_free);  // set the link from the node to the new children
        for(uint16 i = 0; i < n; i++)
            {
            tab[tab_free].makesonof(tab_pos); // set the link of the child to the father node and its son to SITE_NOT_CREATED and no flags
            tab[tab_free].nbson = 0;
            initfct(tab[tab_pos].value,i,n,depth,tab[tab_free].value,tab[tab_free].nbson); // initialise the value and the number of sons
            tab_free++;
            }
        tab[tab_free-1].setlastbrother();   // set the flag for the last brother;
        return;
    }


    /************************************************************ 
     * make some room by removing some nodes and condense       *
     * all the remaining tree in beginning of the array         *
     *                                                          *
     * - if no exception is thrown:                             *
     *   - at least 1 site has been freed                       *
     *   - the tree is consistent, the actual position still    *
     *     has a father (we are not at the root)                *
     *                                                          *
     * when calling this function, make sure before that:       *
     *   - the father of the actual positon exist (not at the   *
     *     root)                                                *
     ***********************************************************/
    void MakeRoom()
    {
        if (tab[tab_pos].father() < SITE_FIRST_POS) {MTOOLS_ERROR("TreeGraph::MakeRoom(), actual position at the root!");}
        size_t nroot = CutTree(median); // cut the tree (set flag1 for the Nodes we keep) and return the new root
        CondenseTree(nroot);            // condense the cutted tree at the beginning of the array
        nbcleanup++;                    // a cleanup has been completed
        return;
    }


    /****************************************************************************
     * condense a subtree at the beginning of the array                         *
     *                                                                          *
     * 1) The subtree to keep MUST be exactely the set of nodes in the array    *
     *    whose flag1 value is set.                                             *
     * 2) This set must form a valid subtree of the original tree such that:    *
     *     a) the subtree is rooted at nroot.                                   *
     *     b) if X is in the subtree, then either all or none of the sons of X  *
     *        are in the subtree.                                               *
     *                                                                          *
     * If no exception is thrown, after the functon return, the TreeGraph object*
     * is in a consistent state (tab_pos and tab_free have been updated)        *
     * Moreover, we are sure that at lesat 1 site has been freed                *
     ***************************************************************************/
    void CondenseTree(size_t nroot)
    {
    // for the new root node, make it a root
    if (tab[nroot].father() >= SITE_FIRST_POS) {tab[nroot].setfather(SITE_DESTROYED);} // 
    tab[nroot].setlastbrother();
    // initialization of the loop
    size_t pf = SITE_FIRST_POS;
    size_t pa = nroot;
    size_t newpos = SITE_NOT_CREATED;
    // the main loop
    while(pa < tab_free)
        {
        if (tab[pa].flag1())    // is pa a node of the cutted tree ?
            {
            if (pa == tab_pos) {newpos = pf;} // note if we are at the new actual position
            // destroy the links to the children if they exist and are not in the new cutted tree
            size_t s = tab[pa].son(); if (s >= SITE_FIRST_POS) {if (tab[s].flag1() == false) {tab[pa].setson(SITE_DESTROYED);}}
            // move the node if we have to move it, in this cas, we update all the pointer to this node
            if (pa != pf) 
                {
                size_t f = tab[pa].father(); if (f >= SITE_FIRST_POS) {if (tab[f].son() == pa) {tab[f].setson(pf);}} // update the pointer from the father (if any)
                size_t s = tab[pa].son(); if (s >= SITE_FIRST_POS) {for(size_t i=0;i<((size_t)(tab[pa].nbson));i++) {tab[s + i].setfather(pf);}} // update the pointers from the sons (if any)
                tab[pf] = tab[pa];
                }
            pf++;
            }
        pa++;
        }
    // check that we are ok
    if (newpos == SITE_NOT_CREATED) {MTOOLS_ERROR("TreeGraph::CondenseTree(), the actual position is not in the subtree !");}
    if (tab[newpos].father() == SITE_DESTROYED) { MTOOLS_ERROR("TreeGraph::CondenseTree(), the actual position has no father !");}
    if (tab[newpos].nbson !=0) {if (tab[newpos].son() == SITE_DESTROYED) {MTOOLS_ERROR("TreeGraph::CondenseTree(), son of the actual position invalid !");}}
    if (tab_free <= pf) { MTOOLS_ERROR("TreeGraph::CondenseTree(), no node removed !");}
    // update the global variables
    tab_pos = newpos;   // the new index of the position
    tab_free = pf;      // the new first free index in the array
    return;
    }


    /********************************************************************
     * does a cutting of the tree                                       *
     *                                                                  *
     * i.e. set the flag1 to true for all sites that must be kept       *
     * the set of site with flag1 set verify condition 1) and 2) of the *
     * CondenseTree() function.                                         *
     * return the root of the subtree with flag1 set.                   *
     *******************************************************************/
    size_t CutTree(size_t limit)
    {
        // CLEAR THE REPART TAB
        for(size_t i=0;i<(tab_size+5);i++) {repart[i] = 0;}
        // CLEAR FLAG1 AND DIRFLAG IN THE ARRAY
        for(size_t i=0;i<tab_size;i++) {tab[i].unsetflag1(); tab[i].unsetdirflag();} 
        // MAKE DIRECTION PATH AND GET DISTANCE OF THE ACTUAL POSITION TO THE ROOT
        size_t d = 0;  size_t p = tab_pos;
        while(p >= SITE_FIRST_POS) {tab[p].setdirflag(); p = tab[p].father(); d++;} d--;
        // FILL THE REPART TAB
        p = SITE_FIRST_POS;             // start at the root
        while(p!=SITE_NOT_CREATED)      // iterate until we have visited all the site
            {
            repart[d]++;                // add the distance for the node
            Next(d,p,SITE_FIRST_POS);   // get the next node and its distance
            }
        // FIND THE MEDIAN
        size_t cumul = 0; size_t i = 0; while(cumul < limit) {cumul += repart[i]; i++;}
        if (i < 3) { MTOOLS_ERROR("TreeGraph::CutTree(), cannot keep any site, median too small!");}
        // FIND THE NEW ROOT AT DISTANCE <= i-2
        size_t nroot = tab_pos; d = 0;
        for(size_t j=0;j<(i-2);j++) {if (tab[nroot].father() >= SITE_FIRST_POS) {d++; nroot = tab[nroot].father();}}
        // SAVE THE DEPTH OF THE NEW MEMORY ROOT
        memrootdepth = depth - d;
        // ITERATE ON THE SUBTREE STARTING AT nroot AND FLAG THE NODE WE KEEP
        p = nroot;      // start at this new root
        while(p!=SITE_NOT_CREATED)      // iterate
            {
            if (d < i) {tab[p].setflag1();}       // keep the nodes at distance < i
            else {tab[p].setson(SITE_DESTROYED);} // otherwise, remove the children so we do not visit them
            Next(d,p,SITE_FIRST_POS);             // get the next node and its distance
            }
        // RETURN THE NEW ROOT
        return nroot;
    }


    /****************************************************
    * move to the next node and update the position     *
    * p and the depth value (relative to the direction  *
    * path if any).                                     *
    * return SITE_NOT_CREATED when we are finished      *
    *****************************************************/
    inline void Next(size_t & depth,size_t & p,size_t root)
        {
            if (tab[p].son() >= SITE_FIRST_POS) {goson(depth,p); return;} // if there is a son, we go there
            if (p==root) {p = SITE_NOT_CREATED; return;}                  // if at the root we are done
            while(tab[p].lastbrother())                                   // find the first ancestor with a brother which is not the root
                {
                gofather(depth,p);
                if (p==root) {p = SITE_NOT_CREATED; return;}
                }
            gobrother(depth,p);
        }


    /***********************************************
     * go to the first son and update the depth    *
     * relatively to the direction path if any     *
     * do not check that the son exist !           *
     **********************************************/
    inline void goson(size_t & depth,size_t & p)        {p = tab[p].son(); if (tab[p].dirflag()) {depth--;} else {depth++;}}

    /***********************************************
     * go to the next brother and update the depth *
     * relatively to the direction path if any     *
     * do not check that the brother exist !       *
     **********************************************/
    inline void gobrother(size_t & depth,size_t & p)    {if (tab[p].dirflag()) {depth += 2;} else {if (tab[p+1].dirflag()) {depth -= 2;}} p++;}

    /***********************************************
     * go to the father and update the depth       *
     * relatively to the direction path if any     *
     * do not check that the father exist !        *
     **********************************************/
    inline void gofather(size_t & depth,size_t & p)     {if (tab[p].dirflag()) {depth++;} else {depth--;} p = tab[p].father();}


    /***********************************************
    * The TreeBox class used for creating a string *
    * representing the tree                        *
    ***********************************************/
    class Treebox
    {
    public:
        Treebox(int x=1,int y=1)       {lx = x; ly=y; buf = new char[lx*ly]; memset(buf,' ',lx*ly);}
        Treebox(const Treebox & B) {lx = B.lx; ly = B.ly; buf = new char[lx*ly]; memcpy(buf,B.buf,lx*ly);}
        ~Treebox() {delete [] buf;}
        std::string tostring()     {std::string s; s.reserve((lx+1)*ly + 1); for(int j=0;j<ly;j++) {for(int i=0;i<lx;i++) {s += get(i,j);} s+= '\n';} return s;}
        inline char & get(int x,int y) {return buf[x + lx*y];}
        inline void include(Treebox & B,int x,int y) {for(int j=0;j< B.ly;j++) {for(int i=0;i< B.lx;i++) {get(i+x,j+y) = B.get(i,j);}}}
        inline void replace(Treebox & B) {delete [] buf; lx = B.lx; ly = B.ly; buf = new char[lx*ly]; memcpy(buf,B.buf,lx*ly);}
        int lx,ly;
        private:
        Treebox & operator=(const Treebox &);
        char * buf;
    };


    /*********************
     * used by CreateBox *
     ********************/
    template<char tochar(const T &)>void makactupos(Treebox & B, size_t pos)
        {
        B.get(2,0) = tochar(tab[pos].value);
        if (tab[pos].father() >= SITE_FIRST_POS)   {B.get(0,0) = '-';}
        if (tab[pos].father() == SITE_DESTROYED)   {B.get(0,0) = '!';}
        if (tab[pos].father() == SITE_NOT_CREATED) {B.get(0,0) = '>';}
        if (pos == tab_pos) {B.get(1,0) = '['; B.get(3,0) = ']';} else {B.get(1,0) = '-'; B.get(3,0) = '-';}
        }


    /********************************************* 
     * create the box object associated          *
     * with the subtree rooted a pos             *
     ********************************************/
    template<char tochar(const T &)>Treebox CreateBox(size_t pos)
        {
        if (tab[pos].nbson == 0) {Treebox B(4,1); makactupos<tochar>(B,pos); return B;}
        if (tab[pos].son() == SITE_NOT_CREATED) {Treebox B(6,1); B.get(4,0) = '-'; B.get(5,0) = '?'; makactupos<tochar>(B,pos); return B;}
        if (tab[pos].son() == SITE_DESTROYED)   {Treebox B(6,1); B.get(4,0) = '-'; B.get(5,0) = 'X'; makactupos<tochar>(B,pos); return B;}
        if (tab[pos].nbson == 1)
            {
            Treebox S = CreateBox<tochar>(tab[pos].son());
            Treebox B(6 + S.lx,S.ly);
            B.include(S,6,0); 
            B.get(4,0) = '-'; B.get(5,0) = '-'; makactupos<tochar>(B,pos);
            return B;
            }
        Treebox * S = new Treebox[tab[pos].nbson];
        int yy = 0;
        int xx = 0;
        for(uint16 i = 0;i<tab[pos].nbson; i++) {S[i].replace(CreateBox<tochar>(tab[pos].son()+i)); yy += S[i].ly + 1; if (S[i].lx > xx) {xx = S[i].lx;}}
        Treebox B(xx+6,yy-1);
        int py=0; for(uint16 i = 0;i<tab[pos].nbson; i++)  {B.include(S[i],6,py); py = py + S[i].ly + 1;}
        B.get(4,0) = '-'; for(int i=0;i< B.ly - S[tab[pos].nbson-1].ly + 1;i++) {B.get(5,i) = '|';}
        makactupos<tochar>(B,pos);
        delete [] S;
        return B;
        }

};


}


/* end of file TreeGraph.h */



