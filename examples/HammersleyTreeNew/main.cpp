/***********************************************
 * Project : HammersleyTree
 * 
 * Graphical representation of the generalized 
 * 'Hammersley Tree process' for unary, binary
 * and geometric trees.
 *
 * New version using the Figure class for display. 
 * 
 ***********************************************/

#include "mtools/mtools.hpp"  
using namespace mtools;


const int MAXPROGENY = 50;  // maximum number of lives of any point
int typelaw;                // type of reproduction law used : [1] = hammersley (unary tree), 2 = binary tree, 3 = geometric law
double X, T;                // the process is defined on [0,X]x[0,T]
double pgeom;               // parameter of the geometric reproduction law

bool createsource;			// flag whether we create sources on (.,0) 
double sourcerate;          // rate of the poison process for the source on the line (.,0)

bool createsink;            // flag whether we create sinks (X,.)

MT2004_64 gen;  // random generator


/* return the number of life of the point*/
inline int life(double x, double t)
    {
    if (typelaw == 1) return 1;
    if (typelaw == 2) return 2;
    int n = 1; while (Unif(gen) < pgeom) { n++; } 
    if (n >= MAXPROGENY) { MTOOLS_ERROR("error : the geometric rv is too large (larger than MAXPROGENY)"); }
    return n;
    }


/* forward declaration */
struct PoissonPoint;
typedef PoissonPoint* pPoissonPoint;

/* strucure describing a point  */
struct PoissonPoint
    {

    /* ctors */
    PoissonPoint() : x(0.0), t(0.0) { init(); }
    PoissonPoint(double xx, double tt) : x(xx), t(tt) { init(); }
    PoissonPoint(const PoissonPoint &) = default;
    PoissonPoint & operator=(const PoissonPoint &) = default;

    /* position */
    double x,t;     // Space time position of the point
  
    /* total number of lifes */ 
    int life() { return _life; }

    /* number of life remaining */
    int remaining() const { return(_life - _firstavail); }

    /* number of life used */
    int used() const { return(_firstavail); }

    /* reference to a pointer to the father */
    pPoissonPoint & father() { return _father; }

    /* reference to a pointer to the ith son */
    pPoissonPoint & son(int i) { return _son[i]; }

    /* reference to the last son */
    pPoissonPoint & lastused() { return _son[_firstavail-1]; }


    /* set the next son */
    void setNextSon(pPoissonPoint pson) { MTOOLS_ASSERT(_firstavail < _life); _son[_firstavail] = pson; _firstavail++; }

    /* print to a string */
    std::string toString() const 
        { 
        std::string s("("); s += mtools::doubleToStringNice(x) + "," + mtools::doubleToStringNice(t) + ") [" + mtools::toString(_life) + "]";
        return s;
        }

    /* non-const address of the object (dirty-;) */
    pPoissonPoint adr() const { return const_cast<pPoissonPoint>(this); }

    /* init the object */
    void init() 
            {
            _father = nullptr;              // no father
            memset(_son, 0, sizeof(_son));  // nor son yet
            _life = ::life(x, t);           // get the number of lives
            _firstavail = 0;
            }

    pPoissonPoint _father;          // pointer to the father
    pPoissonPoint _son[MAXPROGENY]; // list of pointer to the sons
    int _life;                      // number of life of the point
    int _firstavail;                // number of son used
    };


/* point ordering by time of arrival */
struct PPPCompareTime
    {
    bool  operator()(const PoissonPoint & P1, const PoissonPoint & P2)
        {
        if (P1.t < P2.t) return true;
        if (P1.t > P2.t) return false;
        MTOOLS_ERROR("should not happen");
	return false;
        }

    bool  operator()(const pPoissonPoint & P1, const pPoissonPoint & P2)
        {
        if (P1->t < P2->t) return true;
        if (P1->t > P2->t) return false;
        MTOOLS_ERROR("should not happen");
	return false;
        }
    };


/* point ordering by value */
struct PPPCompareSpace
    {
    bool  operator()(const PoissonPoint & P1, const PoissonPoint & P2)
        {
        if (P1.x < P2.x) return true;
        if (P1.x > P2.x) return false;
        MTOOLS_ERROR("should not happen");
	return false;
        }

    bool  operator()(const pPoissonPoint & P1, const pPoissonPoint & P2)
        {
        if (P1->x < P2->x) return true;
        if (P1->x > P2->x) return false;
        MTOOLS_ERROR("should not happen");
	return false;
        }
    };


std::set<PoissonPoint,PPPCompareTime> PPPSet;       // the set of points
typedef std::set<PoissonPoint, PPPCompareTime>::iterator itPPPSet;

std::set<pPoissonPoint, PPPCompareTime> PPPRoots;    // the set of roots
typedef std::set<pPoissonPoint, PPPCompareTime>::iterator itPPPRoots;

std::set<pPoissonPoint, PPPCompareSpace> PPPLeafs;   // the set of leafs
typedef std::set<pPoissonPoint, PPPCompareSpace>::iterator itPPPLeafs;



/** Creates PPP set. */
void createPPPSet()
    {
    PPPSet.clear();
    mtools::PoissonLaw Pl(X*T);
    int64 N = (int64)Pl(gen); // number of points in [0,X]x[0,T]
    cout << "Generating PPP with " << N << " points on [" << 0 << "," << X << "]x[" << 0 << "," << T << "]... ";
    for (int64 k = 0;k < N;k++) { PPPSet.insert(PoissonPoint(Unif(gen)*X, Unif(gen)*T)); }
    cout << "ok\n\n";
    }


/** Creates the sources wth sourcerate density */
void createSources()
    {
    if (sourcerate <= 0.0) return;  // do nothing ion this case
    mtools::PoissonLaw Pl(X*sourcerate);
    int64 N = (int64)Pl(gen); // number of source point
    cout << "Generating Sources with rate " << sourcerate << " -> " << N << " points on [" << 0 << "," << X << "] ";
    std::set<double> setsource;
    for (int64 k = 0;k < N;k++) { setsource.insert(Unif(gen)*X); }
    int k = 1;
    for (auto it = setsource.begin(); it != setsource.end(); it++)
        {
        PPPSet.insert(PoissonPoint((*it),-k));
        k++;
        }
    cout << "ok\n\n";
    }



/** Creates sinks with density 1/(sourcerate + x) dx  */
void createSinks()
    {
    if ((!createsink)||(pgeom <= 0.0)) return; // do nothing if sink not wanted
    if (sourcerate <= 0.0) { sourcerate = 1e-300; }
    double M = (1 / pgeom)*log(1.0 + (pgeom / sourcerate)*T);
    mtools::PoissonLaw Pl(M);
    int64 N = (int64)Pl(gen); // number of sink point
    cout << "Generating Sink  with rate 1/(" << sourcerate << " + " << pgeom << " x) dx  -> " << N << " points on [" << 0 << "," << T << "] ";
    std::set<double> setsink;
    for (int64 k = 0;k < N;k++) 
        {
        double y = (exp(pgeom*Unif(gen)*M) - 1.0)*sourcerate / pgeom;
        setsink.insert(y);
        }
    for (auto it = setsink.begin(); it != setsink.end(); it++)
        {
        PPPSet.insert(PoissonPoint(X+N,(*it)));
        N--;
        }
    cout << "ok\n\n";
    }


/** Creates the sinks (linear intensity). */
void createLinearSinks(double rate)
    {
    mtools::PoissonLaw Pl(T*rate);
    int64 N = (int64)Pl(gen); // number of source point
    cout << "Generating linear sinks with rate " << sourcerate << " -> " << N << " points on [" << 0 << "," << X << "] ";
    std::set<double> setsinks;
    for (int64 k = 0;k < N;k++) { setsinks.insert(Unif(gen)*X); }
    for (auto it = setsinks.begin(); it != setsinks.end(); it++)
        {
        PPPSet.insert(PoissonPoint(X + N, (*it)));
        N--;
        }
    cout << "ok\n\n";
    }



/** Creates the genealogical tree. */
void createTree()
    {
    cout << "Constructing the tree... ";
    PPPRoots.clear();
    PPPLeafs.clear();
    itPPPSet it = PPPSet.begin();
    do  // we add all the source (or at least the first point in time)
        {
        PPPRoots.insert(it->adr());
        PPPLeafs.insert(it->adr());
        ++it;
        }
    while(it->t <= 0.0);
    while (it != PPPSet.end())
        { // add the point 
        auto res = PPPLeafs.insert(it->adr());  // add the point as a new leaf
        MTOOLS_ASSERT(res.second == true);      // must be a real insertion
        itPPPLeafs lit = res.first;             // iterator to the newly inserted element
        if (lit == PPPLeafs.begin())    
            { // we are at the beginning
            PPPRoots.insert(it->adr()); // add as a new root
            }
        else
            { // not at the root so we attach
            --lit; // iterator to the father
            pPoissonPoint ppoint = it->adr();    // the current point
            pPoissonPoint pfather = (*lit);      // the father
            ppoint->father() = pfather;   // set the father of of the point
            pfather->setNextSon(ppoint);  // set the point in the father
            if (pfather->remaining() == 0) PPPLeafs.erase(lit); // remove from the leaf list it if there is no more son left
            }
        ++it; // go to the next point in time
        }
    cout << "ok!\n";
    cout << " - " << PPPSet.size() << " points\n";
    cout << " - " << PPPRoots.size() << " roots\n";
    cout << " - " << PPPLeafs.size() << " leafs\n\n";
    }


/* draw the atoms */
template<int N> void drawPoints(FigureCanvas<N> & canvas, float op = 1.0f)
    {
    cout << "drawing the points... ";
    for (auto it = PPPSet.begin(); it != PPPSet.end(); it++)
        {
        pPoissonPoint pp = it->adr();
		RGBc coul = ((pp->father() != nullptr) && (pp->father()->lastused() == pp)) ? RGBc::c_Red.getMultOpacity(op) :	coul = RGBc::c_Blue.getMultOpacity(op);
		if ((pp->x < X)&&(pp->t > 0)) canvas(FigureDot({ pp->x, pp->t }, 2, coul),1);
        }
	cout << "ok!\n\n";
    }


/* draw the lines */
template<int N> void drawLines(FigureCanvas<N> & canvas, float op = 1.0f)
    {
    cout << "drawing the lines... ";
    for (auto it = PPPSet.begin(); it != PPPSet.end(); it++)
        {
        pPoissonPoint pp = it->adr();
        RGBc coul = RGBc::c_Black;
		coul.multOpacity(op);
        // horizontal lines
        if (pp->father() == nullptr)
			{ // line going to the left border
			if (pp->t > 0.0) canvas(FigureHorizontalLine(pp->t, 0.0, std::min<double>(X, pp->x), coul),0);
			}
        else 
			{ // normal horizontal line
			canvas(FigureHorizontalLine(pp->t, pp->father()->x, std::min<double>(X, pp->x), coul), 0);
			}
        // vertical lines
        if (pp->remaining() > 0) 
			{ // line going to the top
			if (pp->x < X) canvas(FigureVerticalLine(pp->x, std::max<double>(0, pp->t), T, coul), 0);
			}
        else 
			{ // normal vertical line
			canvas(FigureVerticalLine(pp->x, std::max<double>(0,pp->t), pp->lastused()->t, coul), 0);
			}        
		}
	cout << "ok!\n\n";
	}



/* color the trees */
/*
void drawTrees(Image & image, float op = 0.3)
    {
    cout << "drawing the trees... ";
    std::vector<double> mintab,maxtab;
    mintab.resize(LY+1); 
    maxtab.resize(LY+1);

    int kk = 0;
    for (auto it = PPPRoots.begin(); it != PPPRoots.end(); it++)
        { // iterate over the roots of the trees
        pPoissonPoint proot = (*it)->adr(); // root of the tree
        pPoissonPoint p, p2;
        iVec2 Q, Q2;
        int j, j2;
        // fill mintab
        for (int k = 0; k <= LY; k++) { mintab[k] = X + 1; } // first we erase
        p2 = p = proot;
        Q2 = Q = toImage(p->x,p->t,image);
        j2 = j = (int)Q.Y();
        while(p->remaining() == 0)
            {
            p2 = p->lastused();
            Q2 = toImage(p2->x,p2->t,image);
            j2 = (int)Q2.Y();
            for(int l = std::max(0,j); l <= j2; l++) { mintab[l] = p->x; }
            p = p2;
            Q = Q2;
            j = j2;
            }
        for (int l = std::max(0, j); l <= LY; l++) { mintab[l] = p->x; }
        // fill maxtab
        for (int k = 0; k <= LY; k++) { maxtab[k] = -1.0; } // first we erase
        p2 = p = proot;
        Q2 = Q = toImage(p->x,p->t,image);
        j2 = j = (int)Q.Y();
        while (p->used() != 0)
            {
            p2 = p->son(0);
            Q2 = toImage(p2->x,p2->t,image);
            j2 = (int)Q2.Y();
            for (int l = std::max(0, j); l <= j2; l++) { maxtab[l] = p->x; }
            p = p2;
            Q = Q2;
            j = j2;
            }
        for (int l = std::max(0, j); l <= LY; l++) { maxtab[l] = p->x; }
        // fill the image
        RGBc coul;
		if (kk % 2 == 0) { coul = RGBc::c_Red.getMultOpacity(op); } else { coul = RGBc::c_Green.getMultOpacity(op); }
        bool colored = false;
        for (int i = 0; i < LY; i++)
            {
           if (mintab[i] < maxtab[i]) 
               { 
               Q = toImage(mintab[i], 0, image);
               Q2 = toImage(maxtab[i], 0, image);
			   image.draw_line({ Q.X() + 1, LY -2 - i }, { Q2.X() , LY - 2 -i }, coul,true); 
               colored = true;
               }
            }
        if (colored) { kk++; }
        }
    cout << "ok!\n\n";
    return;
    }
*/



int main(int argc, char *argv[])
    {
    MTOOLS_SWAP_THREADS(argc, argv);
    parseCommandLine(argc, argv, true); // parse the command line, interactive mode

	// type of lines
    typelaw = arg("distr").info("Offspring distribution (1=unary, 2=binary, 3=geometric)");
    if ((typelaw < 1) || (typelaw > 2)) 
        { 
        typelaw = 3; 
        pgeom = arg("p", 1.0/3.0).info("parameter of the geometric rv");
        }
    else { pgeom = 0.25; }

	X = arg("X", 50).info("interval length");
	T = arg("T", 50).info("time length");

	createPPPSet();     // create the PPP set    

	// sources  
	createsource = arg("source").info("create source ?");
	if (createsource)
		{
		sourcerate = arg("sourcerate", 1.0).info("source rate");
		createSources();     // create the sources points
		}

	// sinks
    createsink = arg("sink").info("create sinks ?"); 
	if (createsink)
		{
		if (typelaw == 1)
			{
			double sinkrate = arg("sinkrate", 1/sourcerate).info("sink rate");
			createLinearSinks(sinkrate);
			}
		else
			{
			createSinks();
			}
		}

    createTree();       // construct the genealogy 

	auto canvas = makeFigureCanvas(2);
    drawLines(canvas);
    drawPoints(canvas);

	Plotter2D plotter;
	auto PF = makePlot2DFigure(canvas, 4, "Hammersley's Tree Figure");
	plotter[PF];
	plotter.autorangeXY();
	plotter.plot();

    return 0;
	}
	
/* end of file main.cpp */

