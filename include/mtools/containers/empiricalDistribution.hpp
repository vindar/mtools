/** @file empiricalDistribution.hpp */
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
#include "../misc/stringfct.hpp"
#include "../io/serialization.hpp"
#include "../io/fileio.hpp"
#include "../io/logfile.hpp"

#include <string>
#include <vector>



namespace mtools
{


/** 
 * Class representing the empirical distribution of an integer valued random variable. 
 * 
 * The spacing L parameter determine how much memory the object takes:
 * 
 * realization in ]-L,L[ are grouped separately 
 * realizations in [L, 3L[ and ]-3L, -L] are groupe by 2.
 * realizations in [3L, 7L[ and ]-7L, -3L] are groupe by 4.
 * realizations in [7L, 15L[ and ]-15L, -7L] are groupe by 8.
 * 
 **/
class IntegerEmpiricalDistribution
	{

	public:


		static const uint64 DEFAULT_LOGSPACING = 16;	// default spacing is 2^16 = 65536. 


		/**
		 * Constructor. Create an empty distribution. 
		 *
		 * @param	logspacing	logarithm of the spacing parameter (should be between 2 and 62).  The
		 * 						object saves every value on [0, L[, then every 2 values on [L, 3L[, every 4
		 * 						values on [3L, 7L[, every 8 values on [7L, 15L[  ...
		 **/
		IntegerEmpiricalDistribution(uint64 logspacing = DEFAULT_LOGSPACING) : EXP(logspacing),  _tab_plus(), _tab_minus(), _cdf_plus(), _cdf_minus(), _nb_plus(0), _nb_minus(0), _nb_plus_infinity(0), _nb_minus_infinity(0), _minval(std::numeric_limits<int64>::max()), _maxval(std::numeric_limits<int64>::min())
			{
			MTOOLS_INSURE(logspacing >= 2);
			MTOOLS_INSURE(logspacing <= 62);
			}


		/**
		 * Default copy constructor
		 **/
		IntegerEmpiricalDistribution(const IntegerEmpiricalDistribution &) = default;


		/**
		* Default move constructor
		**/
		IntegerEmpiricalDistribution(IntegerEmpiricalDistribution &&) = default;


		/**
		* Default assignement operator
		**/
		IntegerEmpiricalDistribution & operator=(const IntegerEmpiricalDistribution &) = default;



		/** Resets this object to the empty emprical distribution. */
		void reset()
			{
			_tab_plus.clear();
			_tab_minus.clear();
			_cdf_plus.clear();
			_cdf_minus.clear();
			_nb_plus = 0;
			_nb_minus = 0;
			_nb_plus_infinity = 0;
			_nb_minus_infinity = 0;
			_minval = std::numeric_limits<int64>::max();
			_maxval = std::numeric_limits<int64>::min();
			}


		/**
		 * serialize this object into an archive.
		 *
		 * @param [in,out]	ar	The archive to use.
		 **/
		void serialize(OBaseArchive & ar) const 
			{
			ar << "IntegerEmpiricalDistribution"; 
			ar & EXP;
			ar.newline();
			ar & _tab_plus;
			ar.newline();
			ar & _tab_minus;
			ar.newline();
			ar & _nb_plus;
			ar & _nb_minus;
			ar & _nb_plus_infinity;
			ar & _nb_minus_infinity;
			ar & _minval;
			ar & _maxval;
			}


		/**
		 * deserialize this object from an archive.
		 *
		 * @param [in,out]	ar	The archive to use.
		 **/
		void deserialize(IBaseArchive & ar)
			{
			reset();
			ar & EXP;
			MTOOLS_INSURE(EXP >= 2);
			MTOOLS_INSURE(EXP <= 62);
			ar & _tab_plus;
			ar & _tab_minus;
			ar & _nb_plus;
			ar & _nb_minus;
			ar & _nb_plus_infinity;
			ar & _nb_minus_infinity;
			ar & _minval;
			ar & _maxval;
			}


		/**
	    * Saves the object into a file. 
		*
		* @param	filename	Name of the file, 
		* @param	index   	optional index to append to the filename. 
		**/
		void save(std::string filename, uint32 index = 0) const
			{
			if (index != 0) filename += std::string("-") + mtools::toString(index);
			OFileArchive ar(filename);
			ar & (*this);
			}


		/**
		 * Saves a the distribution in human readable CSV format.
		 *
		 * @param	filename	Name of the file,
		 * @param	index   	optional index to append to the filename.
		 **/
		void save_csv_format(std::string filename, uint32 index = 0) const
			{
			if (index != 0) filename += std::string("-") + mtools::toString(index);
			LogFile out(filename, false, true, false);
			out << "********************************************************\n";
			out << "* Empirical distribution of an integer random variable *\n";
			out << "********************************************************\n\n";
			out << " - number of realizations recorded = " << nbInsertion() << "\n";
			out << " - minimal recorded (finite) value = " << minVal() << "\n";
			out << " - maximal recorded (finite) value = " << maxVal() << "\n";
			if (nbMinusInfinity()>0) out << " - number of realization that are -\\infty = " << nbMinusInfinity() << "\n";
			if (nbPlusInfinity()>0) out << " - number of realization that are +\\infty = " << nbPlusInfinity() << "\n";
			out << " - empirical mean E[X] = " << expectation(ROUND_MIDDLE) << "\n";
			out << " - empirical variance V[X] = " << variance() << "\n\n\n";
			out << "list of entries.\n";
			out << "format : position x (or interval [xmin,xmax]) , number of entries at x (or in I)\n\n";
			if (nbMinusInfinity() > 0) { out << "-\\infty , " << nbMinusInfinity() << "\n"; }
			int64 min;
			uint64 ls; 
			for (size_t i = 1; i < _tab_minus.size(); i++)
				{
				size_t j = _tab_minus.size() - i;
				if (_tab_minus[j] >0)
					{
					_rangeIndex(-((int64)j), min, ls);
					if (ls == 0) out << min; else out << "[" << min << ", " << min + (1LL << ls) - 1 << "]";
					out << ", " << _tab_minus[j] << "\n";
					}
				}
			for (size_t i = 0; i < _tab_plus.size(); i++)
				{
				_rangeIndex(i, min, ls);
				if (_tab_plus[i] >0)
					{
					if (ls == 0) out << min; else out << "[" << min << ", " << min + (1LL << ls) - 1 << "]";
					out << ", " << _tab_plus[i] << "\n";
					}
				}
			if (nbPlusInfinity() > 0) { out << "+\\infty , " << nbPlusInfinity() << "\n"; }
			out << "********************************************************\n";
			out << "*           end of empirical distribution file         *\n";
			out << "********************************************************\n\n";
		}

		/**
		* Append the content of a file to the current emprical distribution. 
		*
		* THE SPACING MUST BE THE SAME !
		* 
		* @param	filename	Filename of the file.
		* @param	index   	Optional index to append to the filename.
		**/
		void load_and_append(std::string filename, uint32 index = 0)
			{
			if (index != 0) filename += std::string("-") + mtools::toString(index);
			IntegerEmpiricalDistribution ED;
			IFileArchive ar(filename);
			ar & ED;
			_trySetLogSpacing(ED.logspacing());
			MTOOLS_INSURE(ED.logspacing() == logspacing());
			merge(ED);
			}


		/**
		 * Append all the files with a given filename and all possible index into this object.
		 * 
		 * THE SPACINGS MUST BE THE SAME !
		 *
		 * @param	filename	name of the file (without path!)
		 * @param	path		path of the file. 
		 *
		 * @return	number of files added to the object.  
		 **/
		size_t load_and_append_bunch(const std::string & filename, const std::string & path = ".")
			{
			std::vector<std::string> tabfile;
			mtools::getFileList(path, filename + "-*", false, tabfile, false, true, false);
			if (doFileExist(path + "/" + filename)) tabfile.push_back(filename);
			for (size_t i = 0; i < tabfile.size(); i++)
				{
				load_and_append(path + "/" + tabfile[i], 0);
				}
			return tabfile.size(); 
			}


		/**
		* Add the point of another empirical distribution to this object. Same as operator+=(ED);
		*
		* THE SPACING MUST BE THE SAME !
		* 
		* @param	ED	The emprirical distribution whose points should be added to this one.
		**/
		void merge(const IntegerEmpiricalDistribution & ED)
			{
			if (ED.isEmpty()) return;
			_trySetLogSpacing(ED.logspacing());
			MTOOLS_INSURE(ED.logspacing() == logspacing());
			if (ED._tab_plus.size() > _tab_plus.size()) _tab_plus.resize(ED._tab_plus.size(), 0); 
			for (size_t i = 0; i < ED._tab_plus.size(); i++) { _tab_plus[i] += ED._tab_plus[i]; }
			if (ED._tab_minus.size() > _tab_minus.size()) _tab_minus.resize(ED._tab_minus.size(), 0);
			for (size_t i = 0; i < ED._tab_minus.size(); i++) { _tab_minus[i] += ED._tab_minus[i]; }
			_nb_plus += ED._nb_plus;
			_nb_minus += ED._nb_minus;
			_nb_plus_infinity += ED._nb_plus_infinity;
			_nb_minus_infinity += ED._nb_minus_infinity;
			if (ED._minval < _minval) { _minval = ED._minval; }
			if (ED._maxval > _maxval) { _maxval = ED._maxval; }
			}


		/**
		 * Add the point of another empirical distribution to this object. Same as merge(ED); 
		 *
		 * @param	ED	The emprirical distribution whose points should be added to this one. 
		 **/
		void operator+=(const IntegerEmpiricalDistribution & ED)
			{
			merge(ED);
			}


		/**
		 * Query if this object is empty.
		 *
		 * @return	true if the empirical distribution does not contain any point and false if it does.
		 **/
		MTOOLS_FORCEINLINE bool isEmpty() const
			{
			return ((_nb_plus == 0) && (_nb_minus == 0) && (_nb_plus_infinity == 0) && (_nb_minus_infinity == 0));
			}


		/**
		* Return the logarithm in base 2 of the spacing.
		* 
		* this means that L = 2^logspacing() and the object stores every value on [0, L[, every 2 values on [L, 3L[, every 4 values on [3L, 7L[, every 8 values on [7L, 15L[  ...
		*
		* @return	the logarithm of the spacing. 
		**/
		MTOOLS_FORCEINLINE uint64 logspacing() const { return EXP; }


		/**
		* The spacing L = 2^logspacing().
		* 
		* The object stores every value on [0, L[, every 2 values on [L, 3L[, every 4	values on [3L, 7L[, every 8 values on [7L, 15L[  ...
		*
		* @return	the spacing L.
		**/
		MTOOLS_FORCEINLINE uint64 spacing() const { return (1ull << EXP); }


		/**
		 * Return the minimum value inserted (or std::numeric_limits<int64>::max() if there are none). 
		 **/
		MTOOLS_FORCEINLINE int64 minVal() const { return _minval; }


		/**
		* Return the maximum value inserted (or std::numeric_limits<int64>::min() if there are none).
		**/
		MTOOLS_FORCEINLINE int64 maxVal() const { return _maxval; }


		/**
		* Return the total number of insertion performed.
		**/
		MTOOLS_FORCEINLINE uint64 nbInsertion() const { return _nb_plus + _nb_minus + _nb_plus_infinity + _nb_minus_infinity; }


		/**
		* Return the number of (strictly) positive value inserted. 
		**/
		MTOOLS_FORCEINLINE uint64 nbPositive() const { return _nb_plus - nbZero();  }


		/**
		* Return the empirical probability of being (strictly) positive and finite.
		**/
		MTOOLS_FORCEINLINE double probaPositive() const			
			{
			uint64 N = nbInsertion();
			return (N == 0) ? 0.0 : ((double)nbPositive()) / N;
			}


		/**
		* Return the number of (strictly) negative value inserted.
		**/
		MTOOLS_FORCEINLINE uint64 nbNegative() const { return _nb_minus; }


		/**
		* Return the empirical probability of being (strictly) negative and finite.
		**/
		MTOOLS_FORCEINLINE double probaNegative() const
			{
			uint64 N = nbInsertion();
			return (N == 0) ? 0.0 : ((double)nbNegative()) / N;
			}


		/**
		* Return the number of values that are zero.
		**/
		MTOOLS_FORCEINLINE uint64 nbZero() const { return ((_tab_plus.size() > 0) ? _tab_plus[0] : 0); }


		/**
		* Return the empirical probability of being equal to 0.
		**/
		MTOOLS_FORCEINLINE double probaZero() const
			{
			uint64 N = nbInsertion();
			return (N == 0) ? 0.0 : ((double)nbZero()) / N;
			}


		/**
		* Return the number of values inserted that are equal to +infty.
		**/
		MTOOLS_FORCEINLINE uint64 nbPlusInfinity() const { return _nb_plus_infinity; }


		/**
		* Return the empirical probability of being equal to +infty.
		**/
		MTOOLS_FORCEINLINE double probaPlusInfinity() const
		{
			uint64 N = nbInsertion();
			return (N == 0) ? 0.0 : ((double)nbPlusInfinity()) / N;
		}


		/**
		* Return the number of values inserted that are equal to -infty.
		**/
		MTOOLS_FORCEINLINE uint64 nbMinusInfinity() const { return _nb_minus_infinity; }



		/**
		* Return the empirical probability of being equal to -infty.
		**/
		MTOOLS_FORCEINLINE double probaMinusInfinity() const
		{
			uint64 N = nbInsertion();
			return (N == 0) ? 0.0 : ((double)nbMinusInfinity()) / N;
		}


		/**
		* Insert a new realization in the distribution.
		* same as operator[](val).
		*
		* @param	val	The value to insert.
		**/
		MTOOLS_FORCEINLINE void insert(const int64 val)
			{
			if (val < _minval) _minval = val;
			if (val > _maxval) _maxval = val;
			uint64 hb;
			if (val >= 0)
				{
				uint64 index = _posInArray_u((uint64)val, hb);
				if (index >= _tab_plus.size()) { _tab_plus.resize(index + 1, 0); }
				_tab_plus[index]++;
				_nb_plus++;
				return;
				}
			uint64 index = _posInArray_u((uint64)(-val), hb);
			if (index >= _tab_minus.size()) { _tab_minus.resize(index + 1, 0); }
			_tab_minus[index]++;
			_nb_minus++;
			return;
			}


		/**
		* Insert a new realization int the distribution. 
		* Same as insert(val).
		*
		* @param	val	The value to insert. 
		**/
		MTOOLS_FORCEINLINE void operator[](const int64 val) { insert(val); }


		/**
		* Inserts a plus or minus infinity value.
		*
		* @param	positiveinfinity	true for +infty  and false for -infty.
		**/
		MTOOLS_FORCEINLINE void insert_infinity(bool positiveinfinity) { if (positiveinfinity) insert_plus_infinity(); else insert_minus_infinity(); }


		/**
		* Inserts a +infty value.
		*
		* @param	sign	true to sign.
		**/
		MTOOLS_FORCEINLINE void insert_plus_infinity() { _nb_plus_infinity++; }


		/**
		* Inserts a -infty value.
		*
		* @param	sign	true to sign.
		**/
		MTOOLS_FORCEINLINE void insert_minus_infinity() { _nb_minus_infinity++; }


		/**
		 * (re)-calculates the empirical CDF. 
		 *
		 * 	!!! This method should be called after new insertions and prior to calling  any
		 *  query method below !!!. 
		 **/
		void recomputeCDF() const
			{
			uint64 acc = 0;
			if (_tab_minus.size() > 1)
				{ 
				if (_cdf_minus.size() < _tab_minus.size()) { _cdf_minus.resize(_tab_minus.size()); }
				for (size_t i = _tab_minus.size() - 1; i > 0; i--)
					{
					acc += _tab_minus[i]; 
					_cdf_minus[i] = acc;
					}
				}
			else { _cdf_minus.clear(); }
			if (_tab_plus.size() > 0)
				{
				if (_cdf_plus.size() < _tab_plus.size()) { _cdf_plus.resize(_tab_plus.size()); }
				for (size_t i = 0; i < _tab_plus.size(); i++)
					{
					acc += _tab_plus[i];
					_cdf_plus[i] = acc;
					}
				}
			else { _cdf_plus.clear(); }
			MTOOLS_ASSERT(acc = _nb_plus + _nb_minus);
			}


		/**
		 * Compute the CDF: P(X <= j).
		 *
		 * Make sure recomputeCDF() has been called after the last modification of the object. 
		 *
		 * @param	j	position to query
		 * @param	rounding	The rounding mode (may be one of ROUND_BELOW, ROUND_MIDDLE, ROUND_ABOVE).
		 *
		 * @return	P( X <= j). 
		 **/
		MTOOLS_FORCEINLINE double cdf(int64 j, int rounding = ROUND_MIDDLE) const
			{
			if (nbInsertion() == 0) return 0.0;
			uint64 hb;
			int64 i = _posInArray(j, hb);
			switch (rounding)
				{
				case ROUND_BELOW: 
					{ 
					return (hb == 0) ? ((double)(_nb_minus_infinity + _cdf(i)))/(nbInsertion()) : ((double)(_nb_minus_infinity + _cdf(i - 1))) / (nbInsertion());
					}
				case ROUND_ABOVE: 
					{ 
					return ((double)(_nb_minus_infinity + _cdf(i))) / (nbInsertion()); 
					}
				case ROUND_MIDDLE: 
					{ 
					return (hb == 0) ? ((double)(_nb_minus_infinity + _cdf(i))) / (nbInsertion()) : ((double)(2*_nb_minus_infinity + _cdf(i - 1) + _cdf(i))) / (2*nbInsertion());
					}
				}
			MTOOLS_ERROR("incorrect rounding mode");
			return 0.0;
			}


		/**
		* Compute the tail distribution: P(X > j).
		*
		* Make sure recomputeCDF() has been called after the last modification of the object.
		*
		* @param	j	position to query
		* @param	rounding	The rounding mode (may be one of ROUND_BELOW, ROUND_MIDDLE, ROUND_ABOVE).
		*
		* @return	P( X > j).
		**/
		MTOOLS_FORCEINLINE double tail(int64 j, int rounding = ROUND_MIDDLE) const
			{
			if (nbInsertion() == 0) return 0.0;
			if (rounding == ROUND_BELOW) rounding = ROUND_ABOVE; else if (rounding == ROUND_ABOVE) rounding = ROUND_BELOW;
			return 1.0 - cdf(j, rounding); 
			}


		/**
		 * Compute the empirical density: P(X = j).
		 *
		 * @param	j			position to query.
		 *
		 * @return	P( X = j).
		 **/
		MTOOLS_FORCEINLINE double density(int64 j) const
			{
			if (nbInsertion() == 0) return 0.0; 
			uint64 hb;
			int64 i = _posInArray(j, hb);
			return ((double)_tab(i)) / (nbInsertion() << hb);
			}


		/**
		 * Compute the expectation of the random variable. (slow).
		 * 
		 * The infinite values are discarded (ie we compute the expectation conditionally on the r.v.
		 * being finite).  
		 * 
		 * @param	rounding	The rounding mode (may be one of ROUND_BELOW, ROUND_MIDDLE, ROUND_ABOVE).
		 *
		 * @return	The empirical mean.
		 **/
		double expectation(int rounding = ROUND_MIDDLE) const
			{
			if (rounding == ROUND_MIDDLE) return (expectation(ROUND_ABOVE) + expectation(ROUND_BELOW)) / 2;
			if (_nb_plus + _nb_minus == 0) return 0.0;
			const int64 L = (int64)spacing();
			int64 sum = 0;
			int64 pos = 1;
			int64 step = 1;
			int64 off = 1;
			switch (rounding)
				{
				case ROUND_BELOW:
					{
					for (size_t i = 1; i < _tab_plus.size(); i++)
						{
						sum += ((int64)_tab_plus[i] * pos);
						pos += step;
						off++;
						if (off == L) { off = 0;  step <<= 1; }
						}
					pos = -1;
					step = 1;
					off = 1;
					for (size_t i = 1; i < _tab_minus.size(); i++)
						{
						sum += ((int64)_tab_minus[i] * (pos - step +1));
						pos -= step;
						off++;
						if (off == L) { off = 0;  step <<= 1; }
						}
					break;
					}
				case ROUND_ABOVE:
					{
					for (size_t i = 1; i < _tab_plus.size(); i++)
						{
						sum += ((int64)_tab_plus[i] * (pos + step - 1));
						pos += step;
						off++;
						if (off == L) { off = 0;  step <<= 1; }
						}
					pos = -1;
					step = 1;
					off = 1;
					for (size_t i = 1; i < _tab_minus.size(); i++)
						{
						sum += ((int64)_tab_minus[i] * pos);
						pos -= step;
						off++;
						if (off == L) { off = 0;  step <<= 1; }
						}
					break;
					}
				default: { MTOOLS_ERROR("incorrect rounding mode"); return 0.0; }
				}
			return (double)sum / (_nb_plus + _nb_minus);
			}


		/**
		 * Compute the variance of the random variable. (slow).
		 * 
		 * The infinite values are discarded (ie we compute the variance conditionally on the r.v.
		 * being finite).
		 *
		 * @return	The empirical variance.
		 **/
		double variance() const
			{
			const double e = expectation(ROUND_MIDDLE);
			const double e2 = moment(2.0, ROUND_MIDDLE);
			return e2 - e*e;
			}


		/**
		 * Compute the k'th moment of the random variable. (slow).
		 * 
		 * The infinite values are discarded (ie we compute the moment conditionnally on the r.v. being
		 * finite).
		 *
		 * @param	k			compute E[X^k]
		 * @param	rounding	The rounding mode (may be one of ROUND_BELOW, ROUND_MIDDLE, ROUND_ABOVE).
		 *
		 * @return	the empirical moment.
		 **/
		double moment(double k, int rounding = ROUND_MIDDLE) const
			{
			if (rounding == ROUND_MIDDLE) return (moment(k,ROUND_ABOVE) + moment(k,ROUND_BELOW)) / 2;
			if (_nb_plus + _nb_minus == 0) return 0.0;
			if (k < 0) { if (rounding == ROUND_BELOW) rounding = ROUND_ABOVE; else if (rounding == ROUND_ABOVE) rounding = ROUND_BELOW; }
			const int64 L = (int64)spacing();
			double sum = 0;
			int64 pos = 1;
			int64 step = 1;
			int64 off = 1;
			switch (rounding)
			{
			case ROUND_BELOW:
			{
				for (size_t i = 1; i < _tab_plus.size(); i++)
					{
					sum += (_tab_plus[i] * pow(pos,k));
					pos += step;
					off++;
					if (off == L) { off = 0;  step <<= 1; }
					}
				pos = -1;
				step = 1;
				off = 1;
				for (size_t i = 1; i < _tab_minus.size(); i++)
					{
					sum += (_tab_minus[i] * pow(pos - step + 1 , k));
					pos -= step;
					off++;
					if (off == L) { off = 0;  step <<= 1; }
					}
				break;
				}
			case ROUND_ABOVE:
				{
				for (size_t i = 1; i < _tab_plus.size(); i++)
					{
					sum += (_tab_plus[i] * pow(pos + step - 1, k));
					pos += step;
					off++;
					if (off == L) { off = 0;  step <<= 1; }
					}
				pos = -1;
				step = 1;
				off = 1;
				for (size_t i = 1; i < _tab_minus.size(); i++)
					{
					sum += (_tab_minus[i] * pow(pos, k));
					pos -= step;
					off++;
					if (off == L) { off = 0;  step <<= 1; }
					}
				break;
				}
			default: { MTOOLS_ERROR("incorrect rounding mode"); return 0.0; }
			}
			return sum / (_nb_plus + _nb_minus);
		
		}


		/**
		 * Print information about this object into an std::string. 
		 **/
		std::string toString() const
			{
			std::string s("IntegerEmpiricalDistribution [L="); s += mtools::toString(spacing()) + "]";
			if (isEmpty()) return s +" EMPTY !\n"; 
			s += std::string("\n - memory usage : ") + toStringMemSize(memoryFootprint()) + "\n";
			s += std::string(" - number of entries = ") + mtools::toString(nbInsertion()) + "]\n";
			s += std::string(" - range of values = [") + mtools::toString(minVal()) + " , " + mtools::toString(maxVal())  +  ")\n";
			s += std::string(" - E[X]   = ") + mtools::toString(expectation(ROUND_MIDDLE)) + "   (min " + mtools::toString(expectation(ROUND_MIDDLE)) + "  , max " + mtools::toString(expectation(ROUND_ABOVE)) + ")\n";
			s += std::string(" - Var[X] = ") + mtools::toString(variance()) + "\n";
			s += std::string(" - P(X = -infty) = ") + mtools::toString(probaMinusInfinity()) + "  \t(" + mtools::toString(nbMinusInfinity()) + " values)\n";
			s += std::string(" - P(X = +infty) = ") + mtools::toString(probaPlusInfinity()) + "   \t(" + mtools::toString(nbPlusInfinity()) + " values)\n";
			s += std::string(" - P(X = 0) = ") + mtools::toString(probaZero()) + "   \t(" + mtools::toString(nbZero()) + " values)\n";
			s += std::string(" - P(X < 0) = ") + mtools::toString(probaNegative()) + "   \t(" + mtools::toString(nbNegative()) + " values)\n";
			s += std::string(" - P(X > 0) = ") + mtools::toString(probaPositive()) + "   \t(" + mtools::toString(nbPositive()) + " values)\n";
			return s;
			}


		/**
		 * Return the number of bytes used by this object. 
		 **/
		uint64 memoryFootprint() const
			{
			return sizeof(IntegerEmpiricalDistribution) + sizeof(uint64)*(_tab_plus.capacity() + _tab_minus.capacity() + _cdf_plus.capacity() + _cdf_minus.capacity());
			}


		static const int ROUND_BELOW = 0;		// rounding modes. 
		static const int ROUND_MIDDLE = 1;		//
		static const int ROUND_ABOVE = 2;		//




	private:


		/* return _cdf_plus/minus[i] extended for all value of i */
		MTOOLS_FORCEINLINE uint64 _cdf(int64 i) const
			{
			if (i >= 0)
				{
				if (i >= (int64)_cdf_plus.size()) return (_nb_minus + _nb_plus);
				return _cdf_plus[i];
				}
			i = -i;
			if (i >= (int64)_cdf_minus.size()) return 0;
			return _cdf_minus[i];
			}


		/* return _tab_plus/minus[i] extended for all value of i */
		MTOOLS_FORCEINLINE uint64 _tab(int64 i) const
			{
			if (i >= 0)
				{
				if (i >= (int64)_tab_plus.size()) return 0;
				return _tab_plus[i];
				}
			i = -i;
			if (i >= (int64)_tab_minus.size()) return 0;
			return _tab_minus[i];
			}


		/* change the spacing if the object is empty */
		MTOOLS_FORCEINLINE void _trySetLogSpacing(const uint64 logspacing) { if (isEmpty()) EXP = logspacing; }


		/**
		 * Return the position in the array where the value should be stored.
		 *
		 * @param	val		   	The value (non-negative)
		 * @param [out]	hb		Placeholder to store the logarithm of the spacing corresponding to this position.
		 *
		 * @return	the corresponding index in the array.
		 **/
		MTOOLS_FORCEINLINE uint64 _posInArray_u(const uint64 val, uint64 & hb) const
			{
			const uint64 q = (val >> EXP);					// q = value / L
			if (!q) { hb = 0;  return val; }				// shortcut to gain speed for small value; 
			hb = highestBit(q + 1) - 1;						// highest bit set starting with index 0
			const uint64 base = ((1ull << hb) - 1) << EXP;	// base = (2^hb -1)*L
			const uint64 off = (val - base) >> hb;			// off = (val - base)/(2^hb)
			return (hb << EXP) + off;						// final position L*HB + off
			}


		/**
		* Same as _posInArray_u() but also work for negative val. 
		**/
		MTOOLS_FORCEINLINE int64 _posInArray(const int64 val, uint64 & hb) const
			{
			if (val >= 0) return (int64)_posInArray_u((uint64)val, hb);
			return -((int64)_posInArray_u((uint64)(-val),hb));
			}


		/**
		* Return the range of values saved at position i in the array.
		*
		* Once the method returns, the range of value is exactly [min, min + 2^logstep [.
		*
		* @param	i		   	Position of the index whose range is queried.
		* @param [out]	min	   	Variable to put the minum value of the range.
		* @param [out]	logstep	logarithm (in base 2) of the size of the range.
		**/
		MTOOLS_FORCEINLINE void _rangeIndex(const uint64 i, uint64 & min, uint64 & logstep) const
			{
			logstep = (i >> EXP);						// logstep = i/L
			if (!logstep) { min = i; return; }				// shortcut to gain speed for small value; 
			const uint64 si = (1ull << logstep);		// si = 2^logstep
			const uint64 base = (si - 1) << EXP;		// base = (si - 1) * L
			const uint64 off = (i - (logstep << EXP));	// off = i - logstep*L
			min = base + (off << logstep);				// final position base + off * 2^logstep
			}


		/**
		* Return the range of values saved at position i in the array.
		* (als for negative values)
		* 
		* Once the method returns, the range of value is exactly [min, min + 2^logstep [.
		*
		* @param	i		   	Position of the index whose range is queried.
		* @param [out]	min	   	Variable to put the minum value of the range.
		* @param [out]	logstep	logarithm (in base 2) of the size of the range.
		**/
		MTOOLS_FORCEINLINE void _rangeIndex(const int64 i, int64 & min, uint64 & logstep) const
		{
		uint64 umin;
		if (i >= 0)
			{
			_rangeIndex((uint64)i, umin, logstep);			
			min = umin;
			return;
			}
		_rangeIndex((uint64)(-i), umin, logstep);
		min = -((int64)umin) - (int64)(1LL << logstep)  + 1;
		return;
		}

		uint64 EXP;						// L = 2^EXP,save every value on [0,L[, every 2 value on [L,3L[, every 4 values on [3L, 7L[, every 8 values on [7L,15L[ ...

		std::vector<uint64> _tab_plus;	// array for non-negative values. 
		std::vector<uint64> _tab_minus; // array for (strictly) negative values. 

		mutable std::vector<uint64> _cdf_plus;	// array for the cdf (positive or null values)
		mutable std::vector<uint64> _cdf_minus;	// array for the cdf (negative values)

		uint64 _nb_plus;				// number of entries that are positive or zero. 
		uint64 _nb_minus;				// number of entries that are strictly negative. 

		uint64 _nb_plus_infinity;		// number of entries that are +infinity
		uint64 _nb_minus_infinity;		// number of entries that are -infinity

		int64 _minval;					// minimum value recorded;
		int64 _maxval;					// maximum value recorded; 

	};



}

/* end of file */




