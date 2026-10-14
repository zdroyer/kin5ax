#pragma once

#include "R3Point.h"
#include "ProgressCallback.h"
#include <cstddef>

//What every cutting simulation offers whoever drives it: a step, a run, where it is, and the gouges it
//met. CMaterialSimulation replays a solved trajectory on a chain; CToolPathSimulation replays a path
//with no chain at all. The driver - the application, the C interface - does not care which.
//A gouge is a location where, after the cut, the shank or the holder of the tool is in material that
//is still there. It does not stop the simulation; the counts tell the driver how bad the path is.
namespace kin5ax {

class CCuttingSimulation
{
public:
	CCuttingSimulation();
	virtual ~CCuttingSimulation();

	//back to the first location; the material is not touched (CMaterial::ResetMaterial is the driver's)
	virtual void Reset() = 0;
	//the next location: the tool goes there, the material is cut. False when the path is exhausted
	//(nothing is done then).
	virtual bool Step() = 0;
	//all remaining locations. pProgress (may be NULL) receives the fraction done and may stop the run
	//with Continue() == false; false when stopped, true when the path is exhausted.
	bool Run(CProgressCallback *pProgress);

	size_t GetGougeCount() const;
	bool HasGouged() const;
	//the location and the place of the first gouge (only meaningful when GetGougeCount() is not zero)
	size_t GetFirstGougeStep() const;
	const CR3Point & GetFirstGougePoint() const;

	//the locations of the path, with and without a solution
	virtual size_t GetStepCount() const = 0;
	//the locations passed so far
	virtual size_t GetStepIndex() const = 0;
	virtual bool IsFinished() const = 0;

protected:
	//for a Step that found the shank or the holder in material: the step it happened at, and where
	void NoteGouge(size_t nStep, const CR3Point &r3pWhere);
	void ForgetGouges();

private:
	size_t m_nGougeCount;
	size_t m_nFirstGougeStep;
	CR3Point m_r3pFirstGouge;
};

} // namespace kin5ax
