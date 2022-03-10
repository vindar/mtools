/***********************************************
* Project : MultiBulletProblem
* date : Mon Dec  4 17:00:23 2017
***********************************************/

// *** Library mtools ***
#include "mtools/mtools.hpp"  
using namespace mtools;




class MultiBulletProblem;  // forward declaration.


						   /* class used for comparison operator in particle_set. */
struct MultiBulletComparator
{
	MultiBulletComparator(MultiBulletProblem * obj) : _obj(obj) {}
	bool operator()(int64 indexa, int64 indexb) const;

private:
	MultiBulletProblem * _obj;
};



/**
* Solve the multi-particle bullet problem on the real line.
*
* There are a total of N particles. Each particle has intial position (x,t) where x denotes its
* initial position on the real line and t denotes its (non-negative) apparition time . Each particle
* also has a speed v (which may be positive or negative). After it apparition time, a particle move
* continuously on the x axis with speed v. When two particles collide, a tmeplate functor is called
* to decide which particles are removed/retained.
*
* It is possible for two particle to be a the same position but TWO PARTICLE CANNOT HAVE THE SAME
* POSITION AND THE SAME SPEED (exclusion princple :)
*
* Particle may be added to the system using the addParticle() method. This may be use to add additional
* particles even after running the dynamic for some time but in this case, all particle added must have
* an apparition time larger than the current time of the system.
*
* Running the dynamics (for some time) is done by calling the compute() method, possibly with a custom
* template functor to decide what to do on collisions.
*
* The method results() return a vector describing the (current) fate of each particle in the system.
*
**/
class MultiBulletProblem
{

public:


	typedef std::pair<double, double> Particle; // a particle is represented by its position (first) and its speed (second)


												/** Default constructor. */
	MultiBulletProblem() : _particle_set(MultiBulletComparator(this))
	{
		reset();
	}



	/** Resets the simulation removing all particle and setting the current time to 0. */
	void reset()
	{
		_index_to_iterator.clear();
		_particle_set.clear();
		_event_col.clear();
		_event_add.clear();
		_particle_tab.clear();
		_res.clear();
		_T = 0.0;
	}


	/**
	* Curent time until which the simulation was run.
	*
	* @return	A double.
	**/
	double currentTime() const { return _T; }


	/**
	* Adds a bunch of particles. The time of appartition of every particle cannot be smaller than
	* the current time.
	*
	* TWO PARTICLES IN THE SYSTEM SHOULD NEVER HAVE THE SAME EXACT STATE (position, speed).
	*
	* @param	tab	vector of particles to add, first part of the pair it the position and speed of
	* 				the particle and second part is its apparition time.
	*
	* @return	base index of the particle added (i.e. index of particle corresponding to tab[0]).
	**/
	size_t addParticles(const std::vector<std::pair<Particle, double> > & tab)
	{
		const size_t l = _particle_tab.size();
		const size_t n = tab.size();
		_particle_tab.resize(l + n);
		_res.resize(l + n);
		_index_to_iterator.resize(l + n);
		const size_t a = _event_add.size();
		_event_add.resize(a + n);
		std::memmove((void *)(&(_event_add[a])), (void *)(&(_event_add[0])), n * sizeof(std::pair<double, int64>));
		for (size_t i = 0; i < n; i++)
		{
			const double t = tab[i].second;						// apparition time
			MTOOLS_INSURE(t >= _T);								// particle added must have apparition time at least the current time. 
			_particle_tab[l + i] = tab[i];						// add the particle in the array of particles. 
			_event_add[i] = { t, l + i };
			_res[l + i].second = -2;							// particle not yet created. 
			_index_to_iterator[l + i] = _particle_set.end();	// particle not yet created.
		}
		std::sort(_event_add.begin(), _event_add.end(), [](const std::pair<double, int64> & a, const std::pair<double, int64> & b) -> bool { if (a.first > b.first) return true; else return false; });
		return l;
	}


	/**
	* Query the result vector that describe the fate of each particle.
	*
	* @return	res[i].first is equal to -2 if the particle is not yet created.
	* 			                         -1 if the particle is not yet destroyed.
	* 			                         j>=0 the index of the particle that destroyed it otherwise.
	* 			if it was destroyed, then res.second = (x,t) contain the position x and time t of
	* 			destruction.
	**/
	const std::vector< std::pair<int64, fVec2> > & results() const { return _res; }


private:

	struct MultiBulletProblem_default_functor
		{
		/** The default collision function remove both particles. */
		MTOOLS_FORCEINLINE std::pair<bool, bool> operator()(fVec2 pos, int64 index_left, int64 index_right) { return{ true,true }; }
		};

public:

	/**
	* Run the dynamic up to a given time.
	*
	* @param	stoptime	Time to reach. If negative the dynamic is run until all collisions and
	* 						apparitions are done.
	* @param	fun			The function to call to determine which particle get destroyed when a
	* 						collision occur.
	* 						Signature: std::pair<bool,bool> fun(const fVec2 & pos, int64 index_left, int64 index_right)
	* 						    - pos = (x, t) position and time of the collision
	* 							- index_left: index of the left particle in the collision.
	* 							- index_right: index of the right particle in the collision.
	* 							- return (delete_left, delete_right) where delete_dir is set to true to
	* 						             remove the corresponding particle.
	**/
	template<class collision_test_functor = MultiBulletProblem_default_functor>	void compute(double stoptime = -1.0, collision_test_functor fun = multiBulletProblem_default_functor)
	{
		double max_time = _T;

		while (1)
		{
			int64 index1 = -1, index2 = -1;
			double new_time = mtools::INF;

			if (_event_col.size() > 0)
			{
				auto ev = _event_col.begin();
				if ((stoptime < 0.0) || (ev->first <= stoptime))
				{
					new_time = ev->first;
					index1 = ev->second.first;
					index2 = ev->second.second;
				}
			}
			if (_event_add.size() > 0)
			{
				auto ev = _event_add.back();
				if ((stoptime < 0.0) || (ev.first <= stoptime))
				{
					if (ev.first < new_time)
					{
						new_time = ev.first;
						index1 = -1;
						index2 = ev.second;
					}
				}
			}

			if (index2 < 0) break;	// we are done. 

			max_time = new_time;	// record the time

			if (index1 < 0)
			{ // it is an apparition time. 
				_event_add.pop_back(); // remove the event


				MTOOLS_INSURE(new_time == _particle_tab[index2].second);	// mostly useless consistency check  		

				_move_particles_alive(new_time);			// run the dynamic up to the apparition time.

				auto r = _particle_set.insert(index2);	// insert the particle in the set of particle alive. 
				MTOOLS_INSURE(r.second);				// make sure that no other particle already has the same position and speed. 
				_index_to_iterator[index2] = r.first;	// record the position of the particle for fast lookup. 
				_res[index2].first = -1;				// note in the result tab that the particle is alive

														// add a collision event to its left neighbour if any. 
				if (r.first != _particle_set.begin())
				{
					auto lit = r.first; lit--; // left neighour
					double ct = _T + _compute_collision(_particle_tab[*lit].first, _particle_tab[index2].first);	// compute the collision time with the left neighour
					if ((ct > _T) && (ct != mtools::INF)) { _event_col.insert({ ct,{ *lit, index2 } }); }			// insert the collition in the event map.
				}

				// add a collision event to its right neighbour if any. 
				auto rit = r.first; rit++;
				if (rit != _particle_set.end())
				{
					double ct = _T + _compute_collision(_particle_tab[index2].first, _particle_tab[*rit].first);	// compute the collision time with the right neighour
					if ((ct > _T) && (ct != mtools::INF)) { _event_col.insert({ ct,{ index2, *rit } }); }			// insert the collition in the event map.
				}
			}
			else
			{ // it is a collision time betwen particles index1 and index2. 
				_event_col.erase(_event_col.begin()); // remove the event.

				mm_it it1 = _index_to_iterator[index1];	// iterator to the first particle
				mm_it it2 = _index_to_iterator[index2];	// iterator to the second particle 

				if ((it1 != _particle_set.end()) && (it2 != _particle_set.end()))
				{ // both particle are still alive

					if (std::next(it1) != it2)
					{ // it1 must be on the left and it2 on the right. 
						auto itmp = it1; it1 = it2; it2 = itmp;
						int64 tmp = index1; index1 = index2; index2 = tmp;
					}
					MTOOLS_INSURE(std::next(it1) == it2); // make sure that both particles are adjacent. 

					const double col_pos = _particle_tab[index1].first.first + (_particle_tab[index1].first.second * (new_time - _T));	// collision position. 
					const std::pair<bool, bool> er = fun(fVec2(col_pos, new_time), index1, index2); // query which particles to remove. 

					if (er.first && er.second)
					{ // both particles are removed. 

						_res[index1].first = index2;					// store the collision result. 
						_res[index1].second = { col_pos, new_time };	//
						_res[index2].first = index1;					//
						_res[index2].second = { col_pos, new_time };	//

																		// add a collision between the two new neighours once it1 and it2 are removed if they exist
						auto rit2 = it2; rit2++;
						if ((it1 != _particle_set.begin()) && (rit2 != _particle_set.end()))
						{ // yes, there are two neighbours							
							auto lit1 = it1; lit1--;
							double ct = _T + _compute_collision(_particle_tab[*lit1].first, _particle_tab[*rit2].first);	// compute the collision time 
							if (ct != mtools::INF) { _event_col.insert({ ct,{ *lit1, *rit2 } }); }							// insert it
						}

						_particle_set.erase(it1);
						_particle_set.erase(it2);
						_index_to_iterator[index1] = _particle_set.end();
						_index_to_iterator[index2] = _particle_set.end();
					}
					else
					{
						if (er.first)
						{ // the left particle is removed but not the right one. 
							_res[index1].first = index2;					// store the collision result. 
							_res[index1].second = { col_pos, new_time };	//

																			// add a collision between it2 and the left neighour of it1 (if it exist).
							if (it1 != _particle_set.begin())
							{
								auto lit1 = it1; lit1--;
								double ct = _T + _compute_collision(_particle_tab[*lit1].first, _particle_tab[*it2].first);	// compute the collision time 
								if (ct != mtools::INF) { _event_col.insert({ ct,{ *lit1, *it2 } }); }							// insert it
							}

							_particle_set.erase(it1);
							_index_to_iterator[index1] = _particle_set.end();
						}
						else
						{
							if (er.second)
							{ // the right particle is removed but not the left one. 
								_res[index2].first = index1;					// store the collision result. 
								_res[index2].second = { col_pos, new_time };	//

																				// add a collision between it1 and the right neighour of it2 (if it exist).
								auto rit2 = it2; rit2++;
								if (rit2 != _particle_set.end())
								{
									double ct = _T + _compute_collision(_particle_tab[*it1].first, _particle_tab[*rit2].first);	// compute the collision time 
									if (ct != mtools::INF) { _event_col.insert({ ct,{ *it1, *rit2 } }); }							// insert it
								}

								_particle_set.erase(it2);
								_index_to_iterator[index2] = _particle_set.end();
							}
							else
							{ // neither particle are removed (but they switch places). 
								_move_particles_alive(new_time);										// run the dynamic up to the apparition time (and reorder). 
								_particle_tab[index1].first.first = _particle_tab[index2].first.first;	// fix rounding error by making sure both particles at the same position

								it1 = _index_to_iterator[index1];										// reload position in map
								it2 = _index_to_iterator[index2];										//
								if (std::next(it1) != it2) { auto t = it1; it1 = it2;  it2 = t; }		// order them
								MTOOLS_INSURE(std::next(it1) == it2);									// and make sure they are still adjacent
								_fixmap(it1, it2);														// fix the map if needed. 

																										// add new collision to the left
								if (it1 != _particle_set.begin())
								{
									auto lit = it1; lit--; // left neighour
									double ct = _T + _compute_collision(_particle_tab[*lit].first, _particle_tab[*it1].first);	// compute the collision time with the left neighour
									if (ct != mtools::INF) { _event_col.insert({ ct,{ *lit, *it1 } }); }								// insert the collition in the event map.
								}

								// add a collision to the right
								auto rit = it2; rit++;
								if (rit != _particle_set.end())
								{
									double ct = _T + _compute_collision(_particle_tab[*it2].first, _particle_tab[*rit].first);	// compute the collision time with the right neighour
									if ((ct > _T) && (ct != mtools::INF)) { _event_col.insert({ ct,{ *it2, *rit } }); }				// insert the collition in the event map.
								}
							}
						}
					}
				}
			}
		}

		// finish running the dynamic. 
		if (stoptime < 0.0) { stoptime = max_time; }
		MTOOLS_INSURE(max_time >= _T);														// consistency checks. 
		MTOOLS_INSURE(stoptime >= max_time);												//
		MTOOLS_INSURE((_event_col.size() == 0) || (_event_col.begin()->first > stoptime));	//
		_move_particles_alive(stoptime);
		return;
	}




private:

	friend struct MultiBulletComparator;	

	static MultiBulletProblem_default_functor multiBulletProblem_default_functor;	// unique instance of the class. 

	typedef std::set<int64, MultiBulletComparator>::iterator mm_it;				// typedef for an iterator in the map of Particles alives. 


																				/* move all particle alive up to time new_time. No crossing must happen in between. */
	MTOOLS_FORCEINLINE void _move_particles_alive(const double new_time)
	{
		const double delta = new_time - _T;
		MTOOLS_INSURE(delta >= 0.0);
		if (delta > 0.0)
		{
			int64 prev_index = -1;
			double prev_pos = 0.0;
			for (auto it = _particle_set.begin(); it != _particle_set.end(); ++it)
			{ // iterate over all particle alive
				const int64 index = (*it);	 // index of the particle to move
				const double np = _particle_tab[index].first.first + (_particle_tab[index].first.second * delta);	// new position 
				if ((prev_index >= 0) && (np <= prev_pos))
				{ // problem... 
					_particle_tab[index].first.first = prev_pos; // rectify rounding errors. 
					MTOOLS_INSURE(_particle_tab[index].first.second != _particle_tab[prev_index].first.second); // should not have the same speed. 
					if (_particle_tab[prev_index].first.second > _particle_tab[index].first.second)
					{ // need to swap to respect the ordering wrt operator()
						auto pit = it; pit--;
						*((int64*)&(*it)) = prev_index;	// dirty hack to swap keys in place.
						*((int64*)&(*pit)) = index;			// 
						_index_to_iterator[index] = pit;	// update the lookup vector
						_index_to_iterator[prev_index] = it;//
					}
					else
					{
						prev_index = index;
					}
				}
				else
				{ // no problem
					_particle_tab[index].first.first = np;
					prev_pos = np;
					prev_index = index;
				}

			}
		}
		_T = new_time;
		return;
	}


	MTOOLS_FORCEINLINE void _fixmap(const mm_it it1, const mm_it it2)
	{
		const int64 i1 = *it1;
		const int64 i2 = *it2;
		MultiBulletComparator comp(this);
		if (comp(i2, i1))
		{ // switch needed. 
			*((int64*)&(*it1)) = i2;		// dirty hack to swap keys in place.
			*((int64*)&(*it2)) = i1;		// 
			_index_to_iterator[i1] = it2;	// update the lookup vector
			_index_to_iterator[i2] = it1;	//
		}
	}


	/**
	* Compute the position of the collision of two particles.
	*
	* @param	p1	The first particle.
	* @param	p2	The second particle.
	*
	* @return	the collision time (when both particle start at time 0) or mtools::INF if collision
	* 			never occur in the futur.
	**/
	MTOOLS_FORCEINLINE double _compute_collision(const Particle & p1, const Particle & p2)
	{
		const double dv = p1.second - p2.second;
		if (dv == 0.0) return mtools::INF;
		const double t = (p2.first - p1.first) / dv;
		return (((t < 0.0) || (t == mtools::INF) || (t != t)) ? mtools::INF : t);
	}




	std::vector<mm_it>									_index_to_iterator;		// convert an index to the corresponding iterator in the set of particle (or ::end if the particle is not currently alive). 
	std::set<int64, MultiBulletComparator>				_particle_set;			// the set of all particles currently alive ordered by their position and speed. 

	std::multimap<double, std::pair<int64, int64> >		_event_col;				// time when a collision occurs
	std::vector<std::pair<double, int64> >				_event_add;				// time when a particle arrives
	std::vector< std::pair<Particle, double> >			_particle_tab;			// vector of all particle with their (position, speed) and arrival time. 

	std::vector< std::pair<int64, fVec2> >				_res;					// vector that hold the collision results. 

	double _T;																	// current time until which the dynamics when run. 
};



MultiBulletProblem::MultiBulletProblem_default_functor MultiBulletProblem::multiBulletProblem_default_functor;  // unique instance of the default comparison functor. 


MTOOLS_FORCEINLINE bool MultiBulletComparator::operator()(int64 indexa, int64 indexb) const
{
	if (_obj->_particle_tab[indexa].first.first  < _obj->_particle_tab[indexb].first.first) return true;
	if (_obj->_particle_tab[indexa].first.first  > _obj->_particle_tab[indexb].first.first) return false;
	if (_obj->_particle_tab[indexa].first.second < _obj->_particle_tab[indexb].first.second) return true;
	return false;
}









void drawbullet(double L, const std::vector<std::pair<MultiBulletProblem::Particle, double> > & init_vec, const std::vector< std::pair<int64, fVec2> > & res, int pw = 1)
{

	FigureCanvas<5> canvas(3);
	cout << "Creating... ";
	
	for (size_t i = 0; i < init_vec.size(); i++)
		{
	//	canvas(FigureDot({ init_vec[i].first.first, init_vec[i].second }, RGBc::c_Red),1);
		}

	for (size_t i = 0; i < res.size(); i++)
		{
		if (res[i].first >= 0)
			{
			const RGBc color = (init_vec[i].first.second == 0.0) ? RGBc::c_Red : RGBc::c_Black;
			canvas(Figure::Line({ init_vec[i].first.first, init_vec[i].second }, res[i].second, color, 0),2);
			}
		else
			{
			fVec2 P = { init_vec[i].first.first, init_vec[i].second };
			fVec2 Q = P;
			Q.Y() += 2*L;
			Q.X() += 2* L * init_vec[i].first.second;
			canvas(Figure::Line(P,Q, RGBc::c_Green), 1);
			}
		}


	cout << "ok !\n\n";
	auto PF = makePlot2DFigure(canvas, 3);
	Plotter2D plotter;
	plotter[PF];
	plotter.autorangeXY();
	plotter.plot();

}





MT2004_64 gen;



struct SFun
{

	/**
	* The default collision function remove both particles.
	**/
	MTOOLS_FORCEINLINE std::pair<bool, bool> operator()(fVec2 pos, int64 index_left, int64 index_right)
	{
		const double pp = 0.5;
		const bool kf = (Unif(gen) < pp);
		const bool ks = (Unif(gen) < pp);
		return { kf, ks };



		bool a = Unif_1(gen);

		//if (a) return {true , false};  else return{ false, true }; 

		bool b = Unif_1(gen);
		return{ a,b };

	}

};

SFun sfun;




std::vector< std::pair<MultiBulletProblem::Particle, double> > init_vec;
std::vector< int > partner;
std::vector<double> dietime;

struct QFun
{
	/**
	* The default collision function remove both particles.
	**/
	MTOOLS_FORCEINLINE std::pair<bool, bool> operator()(fVec2 pos, int64 index_left, int64 index_right)
		{
		bool left = false, right = false;
		if (dietime[index_left] > 0.0) left = true;
		if (dietime[index_right] > 0.0) right = true;
		if (left || right) return { left, right };
		
		const double t = pos.Y();
		dietime[index_left] = t;
		dietime[index_right] = t;
		if (partner[index_left] >= 0) { dietime[partner[index_left]] = t; }
		if (partner[index_right] >= 0) { dietime[partner[index_right]] = t; }

		return { true, true };
		}

};

QFun qfun;

void quantumBullet(int LL, double p)
	{
	const double q = p + (1 - p) / 2;

	BinomialLaw B1(LL, p);		// number of 0






	init_vec.resize(LL);
	partner.resize(LL);
	dietime.resize(LL);

	double x = 0;
	ExponentialLaw ExpLaw(1);

	std::vector<int> stack;

	for (int i = 0; i < LL; i++)
		{
		double a = Unif(gen);
		if (a < p)
			{
			init_vec[i] = { { x, 0.0 }, 0 }; // horizontal line
			partner[i] = -2;
			}
		else
			{
			if (a < q)
				{
				init_vec[i] = { { x, 1.0 }, 0 }; // positif particle
				partner[i] = -1;
				stack.push_back(i);
				}
			else
				{
				init_vec[i] = { { x, -1.0 }, 0 }; // negative particle
				if (stack.size() == 0) { partner[i] = -1; } else { const int j = stack.back(); partner[i] = j; partner[j] = i;  stack.pop_back(); }
				}
			}
		dietime[i] = -1.0;
		x += ExpLaw(gen);
		}


	MultiBulletProblem MBP;

	mtools::Chronometer();
	MBP.addParticles(init_vec);
	cout << "Particle added in " << mtools::durationToString(mtools::Chronometer(), true) << "\n";

	mtools::Chronometer();
	MBP.compute(-1.0, qfun);
	cout << "Computation in " << mtools::durationToString(mtools::Chronometer(), true) << "\n";

	const std::vector< std::pair<int64, fVec2> > & res = MBP.results();

		{
		FigureCanvas<5> canvas(3);
		cout << "Creating... ";

		for (size_t i = 0; i < res.size(); i++)
			{

			if (dietime[i] > 0)
				{
				const RGBc color = (init_vec[i].first.second == 0.0) ? RGBc::c_Red : RGBc::c_Black;
				fVec2 P = { init_vec[i].first.first, init_vec[i].second };
				fVec2 Q = P;
				Q.Y() += dietime[i];
				Q.X() += dietime[i] * init_vec[i].first.second;
				canvas(Figure::Line(P,Q, color, 0), 2);

				if (partner[i] > 0)
					{
					double ct = std::abs<double>(init_vec[i].first.first - init_vec[partner[i]].first.first)/2;
					fVec2 R = P;
					R.Y() += ct;
					R.X() += ct * init_vec[i].first.second;
					canvas(Figure::Line(Q, R, RGBc::c_Yellow.getMultOpacity(0.5), 0), 2);

					}

				}
			else
				{
				fVec2 P = { init_vec[i].first.first, init_vec[i].second };
				fVec2 Q = P;
				Q.X() += 2 * LL * init_vec[i].first.second;
				Q.Y() += 2 * LL;
				canvas(Figure::Line(P, Q, RGBc::c_Green), 1);
				}
			}

		cout << "ok !\n\n";
		auto PF = makePlot2DFigure(canvas, 3);
		Plotter2D plotter;
		plotter[PF];
		plotter.autorangeXY();
		plotter.plot();
		}
	}




int main(int argc, char *argv[])
{
	MTOOLS_SWAP_THREADS(argc, argv);  // swap main/fltk threads on OSX
	parseCommandLine(argc, argv, true); // parse the command line, interactive mode


	double L = arg("L ",10000000.0); 
	double p = arg("p", 0.11);

	quantumBullet((int)L, p);
	return 0;

	std::vector< std::pair<MultiBulletProblem::Particle, double> > init_vec;



	const double q = p + (1 - p) / 2;


	for (int i = 0; i < (int)L; i++)
	{
		double x = L * Unif(gen);
		//double t = 100000 * Unif(gen);

		
		double a = Unif(gen);
		if (a < p) init_vec.push_back({ { x, 0.0 }, 0 }); else
		if (a < q) init_vec.push_back({ { x, 1.0 }, 0 }); else
		init_vec.push_back({ { x, -1.0 }, 0 });
	
	//	init_vec.push_back({ { x,  2 * Unif(gen) - 1 }, 0 });
	}


	MultiBulletProblem MBP;

	mtools::Chronometer();
	MBP.addParticles(init_vec);
	cout << "Particle added in " << mtools::durationToString(mtools::Chronometer(), true) << "\n";

	mtools::Chronometer();
	MBP.compute(-1.0, sfun);
	cout << "Computation in " << mtools::durationToString(mtools::Chronometer(), true) << "\n";

	const std::vector< std::pair<int64, fVec2> > & res = MBP.results();

	drawbullet(L, init_vec, res);

	return 0;
}

