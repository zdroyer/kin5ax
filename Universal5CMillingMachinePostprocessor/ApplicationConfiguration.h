#pragma once


class CApplicationConfiguration
{
public:
	CApplicationConfiguration(void);
	~CApplicationConfiguration(void);


	size_t m_nRefreshPeriod;

	bool m_bPerspectiveProjection;
	bool m_bFixedMaterialFrame;
	bool m_bShowMaterial;
	bool m_bShowMaterialFrame;
	bool m_bShowMaterialContour;
	bool m_bShowMaterialHolder;

	bool m_bShowTool;
	bool m_bShowToolFrame;
	bool m_bShowToolBoundingBox;
	bool m_bShowToolTrajectory;

	bool m_bShowLinksFrames;
	bool m_bShowLinks;
	bool m_bShowParts;

	bool m_bShowBase;
	bool m_bShowBaseFrame;

	bool m_bStopSimulation;

	double m_dInputPrecision;
	double m_dUnit;


	DWORD m_nSimulationInterval;

};
