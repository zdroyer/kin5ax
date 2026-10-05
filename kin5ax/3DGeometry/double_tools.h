#pragma once

namespace kin5ax {

//One definition for the whole program (C++17 inline variables; the former file-static copies, one per
//translation unit, made the value look adjustable while nothing could change it, KnownBugs #23).
//dEPSILON is the working tolerance of every comparison below: an absolute one, in the length unit of the
//chain file for lengths and in radians for angles. One picometre at the metre scale of the models: a few
//hundred of the last bits of a double even at the 8 m travel of the largest of them, and three orders
//below the 1E-9 the solver demonstrates. It was 1E-15 until 2026-10-05, which is less than one of those
//last bits at that size, so the comparisons were exact ones in disguise (#56).
//Still a stopgap: the right value follows the unit of the file (in millimetres this one is too small
//again), and lengths, angles and dimensionless degeneracy checks want tolerances of their own. #56 stays
//open for that; the plan is a CTolerances in CPostprocessorSettings.
inline constexpr double dEPSILON = 1E-12;
inline constexpr double dPi = 3.1415926535897932384626433832795;
inline constexpr double dPiDiv2 = 1.5707963267948966192313216916398;
inline constexpr double d2Pi = 6.283185307179586476925286766559;
inline bool are_equal(const double &d1, const double d2,double dTolerance=dEPSILON)
{
	return fabs(d1-d2)<=dTolerance;
}

inline bool are_not_equal(const double &d1, const double d2,double dTolerance=dEPSILON)
{
	return fabs(d1-d2)>dTolerance;
}


//inclusive, like are_equal, so that is_zero and is_not_zero are complements (KnownBugs #56)
inline bool is_zero(const double &d,double dTolerance=dEPSILON)
{
	return fabs(d)<=dTolerance;
}
inline bool is_not_zero(const double &d,double dTolerance=dEPSILON)
{
	return fabs(d)>dTolerance;
}

inline bool is_less_or_equal(const double &d1, const double &d2, double dTolerance=dEPSILON)
{
	if(d1<=d2+dTolerance)
		return true;
	return false;
}
inline bool is_more_or_equal(const double &d1, const double &d2, double dTolerance=dEPSILON)
{
	if(d1+dTolerance>=d2)
		return true;
	return false;
}

inline bool is_between_or_equal(const double &d,const double &d1, const double &d2, double dTolerance=dEPSILON)
{
	if(d+dTolerance<d1)
		return false;
	if(d>d2+dTolerance)
		return false;
	return true;
}

inline double degrees_to_radians(double degree){
	static double dPiDiv180 = acos(0.0)/90.0;
	double radians = degree * dPiDiv180;
	return radians;
}

inline double radians_to_degrees(double radians){
	static double d180DivPi = 90.0/acos(0.0);
	double degrees =  radians * d180DivPi;
	return degrees;
}

inline int round_to_int(double d){
	int n = (int)d;
	double e = d - (double)n;
	if(e>=0.5)
		return n+1;
	if(e<=-0.5)
		return n-1;
	return n;
}

inline bool is_in_0_1(double d, double dTolerance=dEPSILON)
{
	if(d<-dTolerance)
		return false;
	if(d>1+dTolerance)
		return false;
	return true;
}
inline bool is_not_in_0_1(double d, double dTolerance=dEPSILON)
{
	if(d<-dTolerance)
		return true;
	if(d>1+dTolerance)
		return true;
	return false;
}

} // namespace kin5ax
