#pragma once

#include "R3Point.h"
#include "R3Vector.h"
#include "R3Box.h"
#include "Transformation.h"


enum CL_LINE_STATUS{
	CL_LINE_OK,
	CL_LINE_EMPTY,
	CL_LINE_MSYS,
	CL_LINE_ERROR
};

class CCutterLocationData
{
public:
	struct CCLLine{
		CL_LINE_STATUS m_status;
		size_t m_nLineNumber;
		CR3Point m_p;
		CR3Vector m_v;	
		size_t m_nIndexOfString;
		string GetStatus() const
		{
			switch(m_status){
				case CL_LINE_EMPTY:
					return string("EMPTY");
				case CL_LINE_ERROR:
					return string("ERROR");
				case CL_LINE_MSYS:
					return string("MSYS");
				default:
					return string("OK");
			}
		}
		CCLLine(CL_LINE_STATUS status,size_t lineNumber,size_t indexOfString,CR3Point p, CR3Vector v)
			:m_status(status)
			,m_nLineNumber(lineNumber)
			,m_nIndexOfString(indexOfString)
			,m_p(p)
			,m_v(v){}

			CCLLine(CL_LINE_STATUS status,size_t lineNumber,size_t indexOfString)
			:m_status(status)
			,m_nLineNumber(lineNumber)
			,m_nIndexOfString(indexOfString){}
					

	};
	CCutterLocationData(void);
	~CCutterLocationData(void);
	void LoadFile(const string & sFileName, const string &sFileTitle);
	void ResetLocationIterator(void);
	bool GetNextLocation(CR3Point & p, CR3Vector & v);
	
protected:
	CR3Box m_oBox, m_oBoxTransformed;

	CCLLine ParseLine(int nLineBeginIndex, size_t nLineNumber);
	int ReadDouble(const char *s,double &d) const;
	int ReadDelims(const char *s) const;

	vector<CCLLine> m_vLines;
	size_t m_nActual;
	size_t m_nErrors;
	size_t m_nLocations;
	double m_dTrajectoryLength;
	double m_dTotalOrientationChangeRadians;

	char *m_pData;
	CTransformation m_m4x4P,m_m4x4V;

	string m_sFileName;
	string m_sFileTitle;

public:
	bool HasMoreLocations() const;
	CR3Box GetBoundingBox(bool bTransformed) const
	{
		if(bTransformed)
			return m_oBoxTransformed;
		return m_oBox;
	}
	void SetChars(char cX,char cY,char cZ,char cI,char cJ,char cK);
	void GetChars(char &cX,char &cY,char &cZ,char &cI,char &cJ,char &cK) const;
	void SetTransformation(const CTransformation &trP, const CTransformation &trV,bool bInvertedOrientation);
	bool GetLocation(CR3Point & p, CR3Vector & v, size_t nLineNumber) const;
	//text of the line of the loaded file (index as in GetLine)
	const char *GetLineText(size_t index) const;

	double GetTrajectoryLength() const
	{ return m_dTrajectoryLength; }
	
	double GetTotalOrientationChangeRadians() const
 	{ return m_dTotalOrientationChangeRadians; }
	
	size_t GetLocationsCount() const
	{ return m_nLocations; }
	
	size_t GetLinesCount() const
	{ return m_vLines.size();}
	
	size_t GetErrorsCount() const
	{ return m_nErrors; }
	
	const CCutterLocationData::CCLLine &GetLine(size_t index) const{
		if(index>m_vLines.size())
			MY_THROW(CArgsMyException,"Invalid line number");
		return m_vLines[index];
	}

	const string GetFileName() const 
	{ return m_sFileName;}

	const string GetFileTitle() const 
	{ return m_sFileTitle;}

};
