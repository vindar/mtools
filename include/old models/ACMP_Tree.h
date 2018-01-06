/****************************************************************************************************
* TEMPLATE CLASS ACMP_Tree			                                       Version 1.0, Vindar 2014 *
*                                                                                                   *
*                                                                                                   *
* Classe utilisé pour simuler un "Additive Cluster Merging Process" sur un arbre                    *
*                                                                                                   *
*                                                                                                   *
* - l'arbre est cree au fur est a mesure (a l'aide d'une fonction template initNode qui doit        *
*   renvoyer le nombre de fils du site ainsi que fixer son poids initial ).                         *
*                                                                                                   *
* - L'operation principale est exploreBall qui prend un site actif est explore sa boule associée    *
*   Si une fusion est possible lors de l'exploration de la boule celle ci est realisée. Sinon le    *
*   site est marqué comme inactif.                                                                  *
*                                                                                                   *
* - L'algorithme est donc le suivant:                                                               *
*                                                                                                   *
*   1) On part juste d'un arbre constitué d'une racine. A chaque fois qu'un site est construit et a *
*      un poids >= 1, il est ajouté a la liste des sites actifs.                                    *
*                                                                                                   *
*   2) Si aucun site n'est actif, on peut utiliser constructTree() pour construire le premier site  *
*      non construit de hauteur minimale.                                                           *
*                                                                                                   *
*   3) On explore avec ExploreBall() le site actif minimale pour la relation d'ordre sur les sites  *
*      actifs choisie par le second parametre template (ie cmp_depth ou cmp_weight ). On regarde    *
*      sa boule en cherchant a realiser une fusion (si au passge on decouvre de nouveau endroit     *
*      de l'arbre cela a pour consequence d'activer tout les nouveau site decouvert de poids >= 1)  *
*      - Si une fusion est possible, on la realise.                                                 *
*      - Sinon, on desactive le site apres avoir fini d'explorer la boule                           *
*                                                                                                   *
* A chque fois que le nombre de site actif est nul, on a une configuration stable de l'arbre !      *
*                                                                                                   *
*                                                                                                   *
* Classe associés                                                                                   *
*                                                                                                   *
* class ACMP_Node : represente un site de l'arbre.                                                  *
* class ACMP_Cluster : represente un cluster pour la configuration actuelle.                        *
*                                                                                                   *
* Ces deux classes possedent un certain nombre de methode (constante) permettant de mieux etudier   *
* la structure de l'arbre et les clusters actuels                                                   *
*                                                                                                   *
*                                                                                                   *
* EXAMPLE:                                                                                          *
*                                                                                                   *

#include "mylib_all.h"

MT2004_64 gen;

uint16 initNode(double & v, ACMP::pcACMP_Node N) {if (gen.rand_double0() < 0.60) {v = 1.0;} else {v=0.0;}return 1;} // arbre unaire (N) avec bernouilli 0.6

int main (int argc, char *argv[])
	{
	ACMP::ACMP_Tree<initNode,ACMP::cmp_depth> T;
	while(1) {
			 uint64 cut = 0; for(int i=0;i<100000;i++) {if (T.isStable()) cut++; T.constructTree(); T.exploreBall(10000);}
			 T.printInfo(); out << "Configuration stables trouves : " << cut << "\n"; if (cut != 0) {out <<"************************************************\n";} out << "\n\n\n";
			 }
	return 0;
	}

*                                                                                                   *
* standalone, no cpp file                                                                           *
****************************************************************************************************/
#ifndef _ACMP_TREE_H_
#define _ACMP_TREE_H_

#include "crossplatform.h"
#include <set>

namespace mylib
{

namespace models
{

namespace ACMP
{

	// Definitions forward
	class	ACMP_Node;	
	typedef ACMP_Node * pACMP_Node;
	typedef const ACMP_Node * pcACMP_Node;
	class	ACMP_Cluster;	
	typedef ACMP_Cluster * pACMP_Cluster;
	typedef const ACMP_Cluster * pcACMP_Cluster;
	




	/**************************************************************************************************
	***************************************************************************************************
	*
	* CLASS ACMP_Cluster 
	*
	* - typedef associés : - pACMP_Cluster  : pointeur vers un objet ACMP_Cluster
	*                      - pcACMP_Cluster : pointeur vers un objet constant ACMP_Cluster
	*
	* Represente un cluster de sites dans l'arbre. 
	*
	* on peut recuperer des informations à propos du cluster (poids, nombre de sites...). On peut recuperer 
	* le premier site dans le cluster ce qui permet d'iterer ensuite sur tout les site composant le cluster.
	* On peut se deplacer vers le cluster suivant/precedent dans la liste des clusters. 
	*
    ***************************************************************************************************
	***************************************************************************************************/
	class ACMP_Cluster
		{
		public: 
			// *****************************
			// Info sur le cluster
			// *****************************

			// renvoi le poids du cluster
			inline double weight() const {return clusterweight;}

			// renvoi le nombre de site du cluster
			inline uint64 size() const {return nb;}

			// renvoi l'ID du cluster
			inline uint64 ID() const {return id;}

			// renvoi le premier noeud du cluster dans l'arbre (utiliser ensuite ACMP_Node::nextNodeInCluster() pour iterer sur tout les sites du cluster)
			inline pcACMP_Node startNode() const {return clusterstart;}

			// renvoi le prochain cluster dans la liste des clusters
			inline pcACMP_Cluster nextCluster() const {return next;}

			// renvoi le cluster precedent dans lac liste des clusters
			inline pcACMP_Cluster prevCluster() const {return prev;}

			// recupere des information sur un cluster donné: profondeur min et max des sites ainsi que poids min et max des sites qui composent le cluster
			void getInfo(uint32 & dmin,uint32 & dmax,double & wmin,double & wmax) const;

			// affiche des informations sur le cluster
			void printInfo(bool detailled = false) const;


		private:
			// *****************************
			// Private Stuff
			// *****************************

			template<uint16 (*initNode)(double & weight,pcACMP_Node N),bool (*cmp_fct)(pcACMP_Node,pcACMP_Node)> friend class ACMP_Tree; // the main class is a friend 
			friend class ACMP_Node;																											 // ainsi que le la classe ACMP_Node

			double		clusterweight;		// poids total des elements dans le cluster
			uint64		nb;					// nombre d'element dans le cluster
			uint64		id;					// identifiant du cluster
			pACMP_Node	clusterstart;		// pointeur vers le premier element du cluster
			pACMP_Cluster prev;				// pointeur vrs l'element preceent dans la chaine des clusters
			pACMP_Cluster next;				// pointeur vers l'element suivant dans la chaine des clusters

			// cree un cluster et attache le apres prevCN
			ACMP_Cluster(pACMP_Cluster prevCN, double weight, pACMP_Node startN, uint64 n,uint64 Id) : clusterweight(weight), clusterstart(startN), nb(n), prev(prevCN),next(NULL),id(Id)
				{
				if (prevCN != NULL) {if (prevCN->next != NULL) {prevCN->next->prev = this; next = prevCN->next;} prevCN->next = this;}
				return;
				}

			// unlink le cluster et change root vers la valeur de la nouvelle racine si elle est modifiée
			void unlink(pACMP_Cluster & end)
				{
				if (prev != NULL) {prev->next =  next;}
				if (next != NULL) {next->prev =  prev;} else {end = prev;}
				}

			ACMP_Cluster(const ACMP_Cluster &);			// pas de copie
			ACMP_Cluster & operator=(ACMP_Cluster &);		// ni d'assignation
		};











	/**************************************************************************************************
	***************************************************************************************************
	*
	* CLASS ACMP_Node
	*
	* - typedef associés : - pACMP_Node  : pointeur vers un objet ACMP_Node
	*                      - pcACMP_Node : pointeur vers un objet constant ACMP_Node
	*
	* Represente un site de l'arbre. 
	*
	* On peut recuperer des informations sur le site (poids initial, cluster associé, actif ou non,
	* nb de fils crees ou non...). 
	* On peut se deplacer vers le pere/fils/freres...
	*
	***************************************************************************************************
	***************************************************************************************************/
	class ACMP_Node
		{
		public:

			// *****************************
			// Infos sur le site
			// *****************************

			// renvoi le degree du site
			inline uint16 degree() const	{return(p_nbson + ((p_father != NULL) ? 1 : 0));}

			// renvoi le nombre d'enfant du site
			inline uint16 nbson() const	{return p_nbson;}

			// renvoi true si les enfants sont deja construit
			inline bool areSonConstructed() const {return(p_firstson != NULL);}

			// renvoi la profondeur du site
			inline uint32 depth() const	{return p_depth;}

			// renvoi le nombre de frere du site (0 si a la racine)
			inline uint16 nbbrother() const {if (p_father == NULL) {return 0;} return p_father->p_nbson;}

			// renvoi son propre numero d'enfant par rapport à son pere (ie dans [0,nbbrother()-1] ). renvoi 0 pour la racine
			inline uint16 brotherindex() const {if (p_father == NULL) {return 0;} ptrdiff_t pd = this - (p_father->p_firstson); return((uint16)pd);}

			// renvoi la taille du cluster du site associé au site (ie la somme des poids des sites dans le cluster)
			inline double weight() const {if (p_cluster == NULL) {return p_weight;} return p_cluster->clusterweight;}

			// renvoi le poids du site (seulement du site, pas du cluster entier)
			inline double siteWeight() const {return p_weight;} 

			// renvoi un pointeur vers le cluster associé au site (renvoi NULL si le site est isolé)
			inline pcACMP_Cluster getCluster() const {return p_cluster;}

			// renvoi un pointer vers le prochain noeud dans le meme cluster (renvoi NULL si pas de suivant ou si le site est isolé)
			inline pcACMP_Node nextNodeInCluster() const {return p_clusternext;}

			// renvoi true si le site est actuellement actif
			inline bool isActive() const {return (p_actif!=0);}

			// affiche des infos sur le site
			void printInfo(bool detailled = false) const
				{
				if (p_father == NULL) {out << "Root: ";} else { if (p_firstson ==NULL) out << "Leaf: "; else {out << "Node: ";}}
				out << (uint64)(this) << "   weight: " << weight() << "   degree: " << degree() << "   depth: " << depth();
				if (p_cluster == NULL) {out << "   site isole ";} else {out << "   cluster " << p_cluster->id << "  de poids " << p_cluster->clusterweight << " ";}
				if (isActive()) {out << " **** ACTIVE ****";} out << "\n";
				if (detailled) {out << "   - pere : " << (uint64)(p_father) << "\n"; if (p_firstson != NULL) {out << "   - fils : "; for(uint32 i=0;i<p_nbson; i++) {out << (uint64)(p_firstson + i) << "  ";} out << "\n";}}
				}


			// *****************************
			// Deplacement dans l'arbre
			// *****************************

			// renvoi un pointeur vers le pere. renvoi NULL si on est a la racine
			inline pcACMP_Node father() const	{return p_father;}															

			// renvoi un pointeur vers le frere suivant. NULL si on est au dernier frere. 
			inline pcACMP_Node nextbrother() const {if (p_father == NULL) return NULL;  if (brotherindex() == (p_father->p_nbson - 1)) {return NULL;} return(this + 1);}		
	
			// renvoi un pointeur vers le frere numero i avec i dans [0,nbbrother()-1]
			inline pcACMP_Node brother(uint16 i) const  {if (p_father == NULL) return NULL;  return(p_father->p_firstson + i);}
	
			// renvoi un pointeur vers le premier fils. Renvoi NULL si les enfants n'ont pas été crée ou si pas d'enfant
			inline pcACMP_Node firstson() const {return p_firstson;}

			// renvoi un pointeur vers le fils no i avec i dans [0,nbson()-1], renvoi NULL si les fils ne sont pas deja construit
			inline pcACMP_Node son(uint32 i) const {if (p_firstson == NULL) {return NULL;} return(p_firstson + i);}

			// renvoi le prochain site adjacent (effectue un cycle autour du site)
			inline pcACMP_Node nextAdjacent()  const {if (p_adjrotation == p_nbson) {if (p_father == NULL) {if (p_nbson==0) {out << "ACMP_Node::nextAdjacent() error : the tree is reduced to a single root node!\n"; exit(0);} p_adjrotation = 1; return firstson();} p_adjrotation = 0; return p_father;} pcACMP_Node N = son(p_adjrotation); p_adjrotation++; return N;}

			// reset la position de compteur du prochain site adjacent de maniere a ce que le prochain 
			// appel de nextAdjacent() renvoi le pere (ou le 1er fils si on est a la racine).
			inline void resetNextAdjacent() const {p_adjrotation = p_nbson; return;}




		private:
			// *****************************
			// Private Stuff
			// *****************************
			template<uint16 (*initNode)(double & weight,pcACMP_Node N),bool (*cmp_fct)(pcACMP_Node,pcACMP_Node)> friend class ACMP_Tree; // the main class is a friend 
			friend class ACMP_Cluster;

			// les versions privé non-const de deplacement dans l'arbre
			inline pACMP_Node firstson() {return p_firstson;}
			inline pACMP_Node son(uint32 i) {if (p_firstson == NULL) {return NULL;} return(p_firstson + i);}
			inline pACMP_Node nextAdjacent()  {if (p_adjrotation == p_nbson) {if (p_father == NULL) {if (p_nbson==0) {out << "ACMP_Node::nextAdjacent() error : the tree is reduced to a single root node!\n"; exit(0);} p_adjrotation = 1; return firstson();} p_adjrotation = 0; return p_father;} pACMP_Node N = son(p_adjrotation); p_adjrotation++; return N;}

			// ctor / dtor
			ACMP_Node() {return;}					// ctor en privé
			ACMP_Node(const ACMP_Node & N);			// pas de copie possible
			ACMP_Node & operator=(ACMP_Node & N);		// ni d'affectation

			// membres
			double	  p_weight;				// le poids du site
			pACMP_Cluster p_cluster;			// pointeur vers le descripteur du cluster dans la liste chainée des clusters, NULL si le site est isolé
			pACMP_Node  p_clusternext;		// pointeur vers l'element suivant dans le cluster. NULL si dernier element ou le site est isolé
			pACMP_Node  p_father;			// pointeur vers le pere. NULL si on est a la racine
			pACMP_Node  p_firstson;			// pointeur vers le premier fils. NULL si pas d'enfants ou pas encore construits
			uint32    p_depth;				// profondeur du site dans l'arbre. 
			uint16	  p_nbson;				// nombre d'enfant. 
			mutable uint16 p_adjrotation;	// prochain voisin a renvoyer lors de l'appel de nextAdjacent() : i -> son(i) et p_nbson -> father()
			uint16	  p_actif;				// 0 si le site est inactif
		};






	// implementation de ACMP_Cluster::getInfo
	inline void ACMP_Cluster::getInfo(uint32 & dmin,uint32 & dmax,double & wmin,double & wmax) const
		{
		pcACMP_Node N = clusterstart;
		dmin = N->depth(); dmax = N->depth(); wmin = N->p_weight; wmax = N->p_weight;
		N = N->p_clusternext;
		while(N != NULL)
			{
			if (N->depth() < dmin) {dmin = N->depth();}
			if (N->depth() > dmax) {dmax = N->depth();}
			if (N->p_weight < wmin) {wmin = N->p_weight;}
			if (N->p_weight > wmax) {wmax = N->p_weight;}
			N = N->p_clusternext;
			}
		}


	// implementation de ACMP_Cluster::printInfo
	inline void ACMP_Cluster::printInfo(bool detailled) const
		{
		uint32 dmin,dmax; double wmin,wmax;
		getInfo(dmin,dmax,wmin,wmax);
		out << "Cluster " << id << " depth [" << dmin << " , " << dmax << "]  weight " << clusterweight << "  [" << wmin << " , " << wmax << "]  nb sites = " << nb << "\n";
		if (detailled) {pACMP_Node N = clusterstart; while(N != NULL)  {out << "   - "; N->printInfo();  N = N->p_clusternext;} out << "\n";}
		return;
		}






	/***********************************************************************************
	*
	* Fonction de comparaison de 2 node via leur profondeur
	* -> renvoi true si depth(N1) < depth(N2)
	*
	***********************************************************************************/
	inline bool cmp_depth(pcACMP_Node N1,pcACMP_Node N2)
		{
		if ((N1 == NULL)||(N2 == NULL)) {out << "ERROR in cmp_leaf !!!\n\n"; exit(0);}
		if (N1->depth() < N2->depth()) {return true;}
		if (N1->depth() > N2->depth()) {return false;}
		return(N1 < N2);
		}



	/***********************************************************************************
	*
	* Fonction de comparaison de 2 nodes via le poids de leur cluster respectifs
	* -> renvoi true si weight(N1) < weight(N2)
	*
	***********************************************************************************/
	inline bool cmp_weight(pcACMP_Node N1,pcACMP_Node N2)
		{
		if ((N1 == NULL)||(N2 == NULL)) {out << "ERROR in cmp_actif !!!\n\n"; exit(0);}
		if (N1->weight() < N2->weight()) {return true;}
		if (N1->weight() > N2->weight()) {return false;}
		if (N1->depth() < N2->depth()) {return true;}
		if (N1->depth() > N2->depth()) {return false;}
		return(N1 < N2);
		}








	/**************************************************************************************************
	***************************************************************************************************
	*
	* CLASS ACMP_Tree < initNode , cmp_fct >
	*
	* Classe principale pour simuler un processus de fusion additive de cluster sur un arbre. 
	*
	* template parameters:
	*
	*     - uint16 initNode(double & weight,pcACMP_Node N) : Renvoi le nombre de fils d'un Node N. Le node lui meme est deja construit 
	*                                                       met est marqué comme ayant 0 fils. Le reste de l'arbre est coherent et on peut
	*                                                       se deplacer dedans sans probleme.
	*
	*     - bool cmp_fct(pcACMP_Node,pcACMP_Node) : Fonction de comparaison utlisée pour trier la liste des sites actifs (on recommence
	*                                             toujours les explorations depuis le site actif minimal pour cette ordre)
	*                                             Par defaut : cmp_fct = cmp_weight  (on trie par poids croissant du cluster associé)
	*                                             autre choix possible = cmp_depth   (on trie par profondeur croissante)
	*
	*
	* On peut lire un certain nombre d'information sur l'objet (etat du processus, taille de l'arbre, nombre de site actifs, etc...)
	* 
	***************************************************************************************************
	***************************************************************************************************/

	template<uint16 (*initNode)(double & weight,pcACMP_Node N),bool (*cmp_fct)(pcACMP_Node,pcACMP_Node) = cmp_weight> class ACMP_Tree
		{
		public: 

		/* ctor */
		ACMP_Tree() {initLeafSet(); initActiveSet(); initCluster(); initTree();}

		/* dtor */
		~ACMP_Tree() {destroyTree(); destroyCluster(); destroyActiveSet(); destroyLeafSet();}


		/****************************************************************************
		* Informations sur l'objet
		****************************************************************************/

		/* reset the object */
		void reset() {destroyTree(); destroyCluster(); destroyActiveSet(); destroyLeafSet(); initLeafSet(); initActiveSet(); initCluster(); initTree();}

		/* Renvoi le nombre total de site construits dans l'arbre */
		inline uint64 nbSite() const {return p_sizeTree;}

		/* Renvoi le nombre total de sites qui sont incomplet (ie dont les enfants ne sont pas construits) */
		inline uint64 nbChildless() const {return p_leafSet->size();}

		/* renvoi la profondeur max construite dans l'arbre */
		inline uint64 maxDepth() const {return p_maxdepth;}

		/* renvoi la profondeur min des sites dont les enfants ne sont pas encore construits renvoi maxDepth()+1 si il n'y en a aucun */
		inline uint64 minDepthChildless() const {if (p_leafSet->size() == 0) {return(maxDepth()+1);} return (*p_leafSet->begin())->depth();}

		/* Renvoi un pointeur vers la racine de l'arbre */
		inline pcACMP_Node getRoot() const {return p_root;}

		/* Renvoi le nombre de sites qui sont actifs */
		inline uint64 nbActive() const {return p_activeSet->size();}

		/* Renvoi true si la configuration actuelle est stable : ie si il n'y a pas de site actifs */
		bool isStable() const {return(nbActive() == 0);}

		/* renvoi le site minimal actif trie selon l'ordre choisit par le parametre template com_fct (NULL si aucun site actif) */
		inline pcACMP_Node minActive() const {if (p_activeSet->size() == 0) {return NULL;} return(*(p_activeSet->begin()));}

		/* renvoi le site maximal actif trie selon l'ordre choisit par le parametre template com_fct (NULL si aucun site actif) */
		inline pcACMP_Node maxActive() const {if (p_activeSet->size() == 0) {return NULL;} return(*(p_activeSet->rbegin()));}

		/* Renvoi le nombre de sites isolés */
		inline uint64 nbSiteIsolated() const {return(p_sizeTree - p_nbSiteInCluster);}

		/* Renvoi le nombre de sites qui appartiennent a un cluster de taille au moins 2 */
		inline uint64 nbSitesInCluster() const {return p_nbSiteInCluster;}

		/* Renvoi le nombre de cluster (de taille au moins 2) */
		inline uint64 nbClusters() const {return p_nbcluster;}
		
		/* Renvoi un pointeur vers le cluster de poids maximal */
		inline pcACMP_Cluster ClusterMaxWeight() const {return p_clusterMaxWeight;}

		/* Renvoi un pointeur vers le cluster contenant le plus de sites */
		inline pcACMP_Cluster ClusterMaxSize() const {return p_clusterMaxSize;}

		/* Renvoi un pointeur vers le dernier cluster de la liste des cluster */
		inline pcACMP_Cluster lastCluster() const {return this->clusterend;}

		/* Renvoi true si le site est actif */
		inline bool isActive(pcACMP_Node) const {return(N->p_actif != 0);}


		/****************************************************************************
		* Affichage de l'etat
		****************************************************************************/

		/* Affiche le sous arbre partant d'un noeud (pour debogage)*/
		void printSubTree(pcACMP_Node N,std::string tab = std::string(""))  const {out << tab << " - "; N->printInfo(); if (N->p_firstson != NULL) {for(uint32 i=0;i<N->p_nbson;i++) printSubTree(N->p_firstson + i,tab + "    ");}return; }

		/* Affiche l'arbre entier (pour debogage)*/
		void printTree() const {printSubTree(p_root);} 

		/* Affiche des infos sur la liste chainée des clusters */
		void printClusterList(bool detailled = false) const 
			{
			out << "Cluster list : " << nbClusters() << " clusters of  " << p_totcluster << " created\n\n"; if (nbClusters() == 0) {return;}
			if (nbClusters() != 0) 
				{
				out << "Cluster de poids max    : ";
				out << ClusterMaxWeight()->clusterweight << "\n"; 
				ClusterMaxWeight()->printInfo(); out << "\n";
				out << "Cluster de cardinal max : " << ClusterMaxSize()->nb << "\n"; 
				ClusterMaxSize()->printInfo(); out << "\n";
				}
			if (detailled) 
				{
				pACMP_Cluster pC = p_clusterend; 
				while(pC != NULL) {out << "   - "; pC->printInfo(); pC= pC->prev;}
				}
			}


		/* Affichage de l'etat general de l'objet */
		void printInfo() const
			{
			out << "Arbre : - sites cree: " << nbSite() << "\n";
			out << "        - feuilles  : " << nbChildless() << "\n";
			out << "        - depth min : " << minDepthChildless() << "\n";
			out << "        - depth max : " << maxDepth() << "\n\n";
			out << "sites actifs :" << nbActive() << "\n";
			if (nbActive() != 0)
				{
				out << "- site min \n"; minActive()->printInfo();
				out << "- site max \n"; maxActive()->printInfo();
				out << "\n";
				}
			out << "Sites isolés     : " << nbSiteIsolated() << "\n";
			out << "Sites en cluster : " << nbSitesInCluster() << "\n";
			printClusterList(false);
			out << "\n\n\n";
			}


		/****************************************************************************
		* Action sur l'objet, recherche de collisions
		****************************************************************************/


		/* Construit l'arbre jusqu'a obtenir un site actif 
		 * ne fait rien si il y a deja un site actif */
		void constructTree() {while(nbActive() == 0) {constructLowerLeaf();}}


		/* Explore la boule autour du site minimal parmi les sites actifs
		 *
		 * Ne fait rien s'il n'y a aucun site actif
		 * Interrompt l'exploration avant sa fin si le nombre de site crées et activé augnmente de plus de incr_actif
		 *
		 * - renvoi  -1 : pas de site actifs
		 *           0  : le site a été desactivé (exploration complete de la boule autour du site sans realiser de fusion)
		 *           1  : le site reste activé car une fusion a ete réalisé avant la fin de l'exploration
		 *			 2  : le site reste activé car on a interommpu l'exploration (plus de incr_actif nouveau site actifs crees)
		 *
		 * mettre incr-actif = -1 pour pas de limite. 
		 */
		int exploreBall(uint64 incr_actif)
			{
			incr_actif += nbActive();
			pACMP_Node N = nextActive();
			if (N==NULL) {return -1;}
			makeSon(N);	//assure que les enfants ont ete crées
			for(uint16 i = 0;i<N->degree();i++) // explore la boule entiere
				{
				pACMP_Node M = N->nextAdjacent();
				int res = s_exploreBall(M,N,N,1,N->weight(),incr_actif);
				if (res > 0) {return res;}
				}
			removeActive(N);	// toute la boule verifie et aucun site activé !
			N->resetNextAdjacent();
			return 0;
			}






		/****************************************************************************
		* PRIVATE PART
		****************************************************************************/
private: 

		ACMP_Tree(const ACMP_Tree &);				// pas de copie de l'objet
		ACMP_Tree & operator=(const ACMP_Tree &);	//



		/************************************************************
		Gestion de l'algorithme de fusion
		************************************************************/

		// sous fonction recursive
		// recherche une collision a partir d'un site actif donné
		// N = site actuel
		// A = site ancetre d'ou on vient
		// C = site central
		// d = distance d(C,N)
		// wC = poids du cluster de C
		// maxact : le nombre de site actif a ne pas depasser
		// renvoi true si une fusion a eu lieu ou on a decouvert trop de nouveau site actifs
		int s_exploreBall(pACMP_Node N,pACMP_Node A,pACMP_Node C,uint64 d,double wC,uint64 maxact)
			{
			if (N->weight() >= d) // le site actuel N contient C dans sa boule d'activation, il faut donc fusionner les deux sites 
				{
				if (clusterFusion(C,N)) {return 1;} // si la fusion reussi on s'arrete, sion, c'est qu'il etait deja dans le meme cluster
				}
			if (nbActive() > maxact) {return 2;} // si le nb de site actif est trop grand on arete
			if ((d+1) <= wC) // si on ne sort pas de la boule d'activation de C en faisant un pas deplus, alors on y va 
				{
				makeSon(N);	// assure que les enfants on deja ete crées
				for(uint16 i = 0;i<N->degree();i++)
					{
					pACMP_Node M = N->nextAdjacent();
					if (M != A) {int res = s_exploreBall(M,N,C,d+1,wC,maxact); if (res > 0) {return res;}}
					}
				}
			return 0;
			}


		/************************************************************
		Gestion de l'arbre 
		************************************************************/
		pACMP_Node		p_root;		// la racine de l'arbre
		uint64			p_sizeTree;	// taille de l'arbre (nombre de site crees)
		uint64			p_maxdepth; // profondeur max d'un site de l'arbre

		/* initialisation : Cree la racine de l'arbre */
		void initTree()
			{
			p_sizeTree=1;
			p_maxdepth=0;
			p_root = new ACMP_Node;
			p_root->p_father = NULL; 
			p_root->p_firstson = NULL;
			p_root->p_depth = 0;	
			p_root->p_nbson = 0;
			p_root->p_actif = 0;
			p_root->p_cluster = NULL;
			p_root->p_clusternext = NULL;
			p_root->resetNextAdjacent();
			p_root->p_weight = 0.0;
			p_root->p_nbson = initNode(p_root->p_weight,p_root);
			if (p_root->p_nbson > 0)		{addLeaf(p_root);}
			if (p_root->p_weight >= 1.0)	{addActive(p_root);}
			}

		/* destruction de l'arbre */
		void destroyTree()
			{
			if (p_root->p_firstson != NULL) {destroySubTree(p_root);}
			delete p_root;
			p_sizeTree=0;
			p_maxdepth=0;
			p_root = NULL;
			}

		/* destruction d'un sous-arbre strict */
		void destroySubTree(pACMP_Node N)
			{
			if (N == NULL) {return;}
			if (N->p_firstson == NULL) {return;}
			for(uint16 i=0; i<N->p_nbson; i++) {destroySubTree(N->p_firstson + i);}
			delete [] (N->p_firstson);
			N->p_firstson = NULL;
			p_sizeTree -= N->p_nbson; // et p_maxdepth n'est plus forcement correct
			return;
			}

		/*cree les enfants d'un site */
		inline void makeSon(pACMP_Node N)
			{
			if ((N->p_firstson != NULL)||(N->p_nbson == 0)) {return;}
			if (N->depth()+1 > p_maxdepth) {p_maxdepth = N->depth()+1;}
			removeLeaf(N);
			N->p_firstson = new ACMP_Node[N->p_nbson];
			p_sizeTree += N->p_nbson;
			for(uint16 i=0;i<N->p_nbson;i++)
				{
				N->p_firstson[i].p_father = N; 
				N->p_firstson[i].p_firstson = NULL;
				N->p_firstson[i].p_nbson = 0;
				N->p_firstson[i].p_depth = N->depth()+1;
				N->p_firstson[i].p_actif = 0;
				N->p_firstson[i].p_cluster = NULL;
				N->p_firstson[i].p_clusternext = NULL;
				N->p_firstson[i].resetNextAdjacent();
				N->p_firstson[i].p_weight = 0.0;
				N->p_firstson[i].p_nbson = initNode(N->p_firstson[i].p_weight,N->p_firstson + i);
				if (N->p_firstson[i].p_nbson > 0)		{addLeaf(N->p_firstson + i);}
				if (N->p_firstson[i].p_weight >= 1.0)	{addActive(N->p_firstson + i);}
				}
			}


		/************************************************************
		Gestion des clusters
		************************************************************/
		pACMP_Cluster	p_clusterend;		// le debut de la liste des clusters
		size_t			p_nbcluster;		// nombre de cluster dnas la liste
		size_t			p_totcluster;		// nombre total de cluster crees
		uint64			p_nbSiteInCluster;	// nombre de sites qui sont dans un cluster
		pACMP_Cluster	p_clusterMaxSize;	// le cluster ayant le plus de sites
		pACMP_Cluster	p_clusterMaxWeight;	// le cluster ayant le poids maximal

		/* initialisation */
		void initCluster()
			{
			p_clusterend = NULL;
			p_nbcluster = 0;
			p_totcluster = 0;
			p_nbSiteInCluster = 0;
			p_clusterMaxSize = NULL;
			p_clusterMaxWeight = NULL;
			}

		/* destruction */
		void destroyCluster()
			{
			while(p_clusterend != NULL) {pACMP_Cluster nc = p_clusterend->prev; delete p_clusterend; p_clusterend = nc;}
			p_nbcluster = 0;
			p_totcluster = 0;
			p_nbSiteInCluster = 0;
			p_clusterMaxSize = NULL;
			p_clusterMaxWeight = NULL;
			}

		/* fusionne 2 clusters ensemble, renvoi true si une fusion a bien eu lieu
		 * (c-a-d si N1 et N2 sont bien dans des clusters differents) */
		bool clusterFusion(pACMP_Node N1,pACMP_Node N2)
			{
			if ((N1->p_cluster == NULL)&&(N2->p_cluster == NULL)) // on fusionne deux sites isolés
				{
				removeActive(N1); removeActive(N2); // on desactive les sites N1 et N2
				p_clusterend = new ACMP_Cluster(p_clusterend,N1->weight()+N2->weight(),N1,2,p_totcluster);
				p_totcluster++;
				p_nbcluster++; // un cluster de plus
				N1->p_cluster = p_clusterend;
				N2->p_cluster = p_clusterend;
				N1->p_clusternext = N2;
				addActive(N1); addActive(N2); // on reactive les deux sites du cluster forme
				p_nbSiteInCluster += 2; // 2 sites de plus dans des clusters
				if ((p_clusterMaxSize == NULL)||(p_clusterMaxSize->nb < p_clusterend->nb)) {p_clusterMaxSize = p_clusterend;}
				if ((p_clusterMaxWeight == NULL)||(p_clusterMaxWeight->clusterweight < p_clusterend->clusterweight)) {p_clusterMaxWeight = p_clusterend;}
				return true;
				}
			if (N1->p_cluster == NULL) {pACMP_Node T = N1; N1 = N2; N2 = T;} // on inverse N1 et N2 
			if (N2->p_cluster == NULL) // on fusionne le site N2 avec le cluster N1
				{
				pACMP_Cluster pCI1 = N1->p_cluster;
				removeActive(N2);																			// supprime N2 des sites actifs
				pACMP_Node N = pCI1->clusterstart; while(N != NULL) {removeActive(N); N = N->p_clusternext;}	// supprime tout les sites du cluster de N1 des sites actifs
				N2->p_cluster = pCI1;
				N2->p_clusternext = pCI1->clusterstart;
				pCI1->nb += 1;
				pCI1->clusterweight += N2->p_weight;
				pCI1->clusterstart = N2;
				while(N2 != NULL) {addActive(N2); N2 = N2->p_clusternext;}			// on active tout les sites du nouveau cluster
				p_nbSiteInCluster++; // un site de plus dans un cluster
				if (p_clusterMaxSize->nb < pCI1->nb) {p_clusterMaxSize = pCI1;}
				if (p_clusterMaxWeight->clusterweight < pCI1->clusterweight) {p_clusterMaxWeight = pCI1;}
				return true;
				}
			// on fusionne deux vrai clusters ensemble
			pACMP_Cluster pCI1 = N1->p_cluster; // les infos sur le cluster 1		
			pACMP_Cluster pCI2 = N2->p_cluster; // les infos sur le cluster 2
			if (pCI1 == pCI2) {return false;}	// rien a faire si les deux sites sont deja dans le meme cluster
			pACMP_Node M1 = pCI1->clusterstart; while(M1 != NULL) {removeActive(M1); M1 = M1->p_clusternext;}	// inactive tout les sites du cluster de N1 
			pACMP_Node M2 = pCI2->clusterstart; while(M2 != NULL) {removeActive(M2); M2 = M2->p_clusternext;}	// inactive tout les sites du cluster de N2
			pCI1->clusterweight += pCI2->clusterweight;	// update taille du cluster 1
			pCI1->nb += pCI2->nb;;						// update nombre de sites du cluster 1
			if (p_clusterMaxSize->nb < pCI1->nb) {p_clusterMaxSize = pCI1;}
			if (p_clusterMaxWeight->clusterweight < pCI1->clusterweight) {p_clusterMaxWeight = pCI1;}
			pCI1->id = min(pCI1->id,pCI2->id);			// le nouvel id est le min des id
			pACMP_Node N = pCI2->clusterstart;  // on se place au debut du cluster 2
			while(N->p_clusternext != NULL) {N->p_cluster = pCI1; addActive(N); N = N->p_clusternext;} N->p_cluster = pCI1;	 addActive(N); // on itere sur le cluster 2 en changeant le champs p_cluster vers celui du cluster 1 et en activant le site
			N->p_clusternext = pCI1->clusterstart;	N = N->p_clusternext;// on chaine le cluster 1 a la fin du cluster 2
			while(N != NULL) {addActive(N); N = N->p_clusternext;} // on active aussi les site du cluster 1
			pCI1->clusterstart = pCI2->clusterstart;// update du point de depart du cluster 1
			pCI2->unlink(p_clusterend); // on enleve le cluster 2 de la chaine
			delete pCI2; // on libere la mémoire pour le cluster 2
			p_nbcluster--;	// un cluster de moins
			return true;
			}


		/************************************************************
		Gestion des sites actifs
		************************************************************/
		std::set<pACMP_Node,bool (*)(pcACMP_Node,pcACMP_Node)> * p_activeSet;	// ensemble des sites actifs triés par cmp_actif

		/* initialisation */
		void initActiveSet() {p_activeSet = new std::set<pACMP_Node,bool (*)(pcACMP_Node,pcACMP_Node)>(cmp_fct); return;}

		/* destruction */
		void destroyActiveSet() {delete p_activeSet; p_activeSet = NULL;}

		/* Recupere le site actif de taille minimale, renvoi NULL si aucun site actif */
		inline pACMP_Node nextActive() {if (p_activeSet->size() == 0) {return NULL;} return(*(p_activeSet->begin()));}

		/* Inactive un site (cela ne change pas le statut des autres sites appartenant au même cluster */
		inline void removeActive(pACMP_Node N) {if (N->p_actif != 0) {p_activeSet->erase(N); N->p_actif = 0;}  return ;}

		/* Active un site, ne tient pas compte des autres sites du cluster eventuel */
		inline void addActive(pACMP_Node N) {if (N->p_actif == 0) {p_activeSet->insert(N); N->p_actif = 1;} return;}

	
		/************************************************************
		Gestion des feuilles
		************************************************************/
		std::set<pACMP_Node,bool (*)(pcACMP_Node,pcACMP_Node)> * p_leafSet;	// ensemble des sites feuilles dont les enfants ne sont pas construit, trie par cmp_leaf

		/* initialisation */
		void initLeafSet() {p_leafSet = new std::set<pACMP_Node,bool (*)(pcACMP_Node,pcACMP_Node)>(cmp_depth); return;}

		/* destruction */
		void destroyLeafSet() {delete p_leafSet; p_leafSet = NULL;}
		
		/* Ajoute une feuille */
		inline void addLeaf(pACMP_Node N) {p_leafSet->insert(N); return;}

		/* Supprime une feuille */
		inline void removeLeaf(pACMP_Node N) {p_leafSet->erase(N); return;}

		/* construit une feuille de hauteur minimale */
		inline void constructLowerLeaf() {if (p_leafSet->size() == 0) {out << "constructLowerLeaf() : NO MORE LEAF\n"; exit(0);} makeSon(*(p_leafSet->begin()));}

		};


}
}
}


#endif
/* end of file _ACMP_TREE_H_ */






