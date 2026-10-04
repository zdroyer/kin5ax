#pragma once

namespace kin5ax {

//One definition for the whole program (C++17 inline variables; the former file-static copies, one per
//translation unit, made the value look adjustable while nothing could change it, KnownBugs #23).
//dEPSILON is an absolute tolerance far below one ULP at millimetre scale, i.e. an exact comparison in
//practice (#56); tolerances by kind (length, angle, relative) are the planned replacement.
inline constexpr double dEPSILON = 1E-15;
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


inline bool is_zero(const double &d,double dTolerance=dEPSILON)
{
	return fabs(d)<dTolerance;
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
