#include "stdafx.h"
#include "ToolHolder.h"

namespace kin5ax {

const char *szNO_HOLDER = "NO_HOLDER";

namespace {

/**
 * One entry of the catalogue: the designation and up to three cylinders from the nose downwards.
 * The taper first, then whatever the holder narrows to, then the nose the tool comes out of.
 * Metres, rounded to the millimetre: these are for a collision check, not for a drawing.
 */
struct CCatalogueEntry
{
	const char *szModel;
	double dLength1, dRadius1;
	double dLength2, dRadius2;
	double dLength3, dRadius3;
};

const CCatalogueEntry vCATALOGUE[] = {
	//model              taper/flange        middle              nose
	{"NO_HOLDER",        0.000, 0.0000,      0.000, 0.000,       0.000, 0.000},
	{"BT30 ER20",        0.048, 0.0315,      0.020, 0.021,       0.030, 0.017},
	{"BT40 ER16",        0.065, 0.0445,      0.025, 0.021,       0.060, 0.014},
	{"BT40 ER32",        0.065, 0.0445,      0.025, 0.027,       0.035, 0.025},
	{"BT40 SF12",        0.065, 0.0445,      0.030, 0.021,       0.055, 0.012},
	{"BT40 Weldon 16",   0.065, 0.0445,      0.025, 0.027,       0.040, 0.022},
	{"BT50 ER40",        0.080, 0.0629,      0.035, 0.036,       0.040, 0.034},
	{"CAT40 ER32",       0.065, 0.0445,      0.025, 0.027,       0.035, 0.025},
	{"HSK40A ER16",      0.040, 0.0250,      0.020, 0.019,       0.050, 0.014},
	{"HSK63A ER32",      0.050, 0.0400,      0.025, 0.027,       0.040, 0.025},
	{"HSK63A SF12",      0.050, 0.0400,      0.030, 0.021,       0.060, 0.012},
};

const size_t nCATALOGUE = sizeof(vCATALOGUE)/sizeof(vCATALOGUE[0]);

CToolHolder FromEntry(const CCatalogueEntry &oEntry)
{
	std::vector<CToolHolderSection> v;
	v.push_back(CToolHolderSection(oEntry.dLength1,oEntry.dRadius1));
	v.push_back(CToolHolderSection(oEntry.dLength2,oEntry.dRadius2));
	v.push_back(CToolHolderSection(oEntry.dLength3,oEntry.dRadius3));
	return CToolHolder(oEntry.szModel,v);		//the sections of no length are dropped
}

} // namespace

size_t ToolHolderCatalogueSize()
{
	return nCATALOGUE;
}

CToolHolder ToolHolderFromCatalogue(size_t nIndex)
{
	if(nIndex>=nCATALOGUE)
		return CToolHolder();
	return FromEntry(vCATALOGUE[nIndex]);
}

bool ToolHolderByModel(const std::string &sModel, CToolHolder &oHolder)
{
	//exactly the designation of the catalogue: a host that wants to be forgiving about spelling can
	//be, and this is not the place to guess which holder somebody meant
	for(size_t i=0;i<nCATALOGUE;i++)
		if(sModel==vCATALOGUE[i].szModel){
			oHolder = FromEntry(vCATALOGUE[i]);
			return true;
		}
	return false;
}

} // namespace kin5ax
