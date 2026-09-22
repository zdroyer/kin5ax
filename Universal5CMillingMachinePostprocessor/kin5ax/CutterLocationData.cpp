#include "stdafx.h"
#include "CutterLocationData.h"


CCutterLocationData::CCutterLocationData(void)
:m_pData(0)
, m_nErrors(0)
, m_nActual(0)
, m_nLocations(0)
, m_dTrajectoryLength(0.0)
, m_dTotalOrientationChangeRadians(0.0)
{
}

CCutterLocationData::~CCutterLocationData(void)
{
	delete []m_pData;
}

void CCutterLocationData::LoadFile(const string & sFileName, const string &sFileTitle)
{
	int nFileHandle = _open(sFileName.c_str(),_O_TEXT|_O_RDONLY);
	if( nFileHandle == -1 )
		MY_THROW(CArgsMyException,"Problem with opening of file: "+sFileName+"\n"+strerror(errno)); 
	long nFileLength = _filelength(nFileHandle);
	if(nFileLength==-1)
		MY_THROW(CArgsMyException,"Problem with obtaining size of file: "+sFileName+"\n"+strerror(errno)); 
	if( (size_t)nFileLength > m_vLines.max_size() ){
		_close(nFileHandle);
		MY_THROW(CArgsMyException,"Size of file: "+sFileName+" is too big"); 
	}
	delete []m_pData;
	m_pData = new char[nFileLength+1];
	m_pData[nFileLength]=0;
	m_vLines.clear();
	int nCharsRead = _read(nFileHandle,m_pData,(unsigned int)nFileLength);
	_close(nFileHandle);
	if( nCharsRead == -1 ){
		MY_THROW(CArgsMyException,"Cannot read file: "+sFileName+"\n"+strerror(errno)); 
	}
	
	m_sFileName = sFileName;
	m_sFileTitle = sFileTitle;
	string sOldLocale(setlocale(LC_NUMERIC,NULL));
	
	setlocale(LC_NUMERIC,"English");
	m_pData[nCharsRead]=0;
	m_nErrors=0;
	m_nActual=0;
	m_nLocations=0;
	m_dTotalOrientationChangeRadians = 0.0;
	m_dTrajectoryLength = 0.0;
	m_oBox = CR3Box();
	CR3Vector r3vLast;
	for(int i=0;i<nCharsRead;){
		char *pNewLine = strchr(&(m_pData[i]),'\n');
		int nNewLineIndex = nCharsRead-i;
		if(NULL != pNewLine){
			nNewLineIndex = int(pNewLine - &(m_pData[i]));
		}
		if(i+nNewLineIndex<nCharsRead){
			m_pData[i+nNewLineIndex]=0;
			if(m_vLines.size()){
				if(m_vLines.back().m_status==CL_LINE_OK)
					r3vLast = m_vLines.back().m_v;
				m_vLines.push_back(ParseLine(i,m_vLines.size()));			
			} else {
				m_vLines.push_back(ParseLine(i,m_vLines.size()));
				r3vLast = m_vLines.back().m_v;
			}
			if(m_vLines.back().m_status==CL_LINE_OK) {
				m_oBox.ExpandToPoint(m_vLines.back().m_p);
				m_nLocations++;
				CR3Vector &back = m_vLines.back().m_v;
				m_dTotalOrientationChangeRadians+= r3vLast.UndirectedAngle(m_vLines.back().m_v);
			} else {
				if(m_vLines.back().m_status==CL_LINE_ERROR)
					m_nErrors++;
			}
			i+=(nNewLineIndex+1);
		} else break;		
	}
//	m_dTotalOrientationChange = m_dTotalOrientationChange);
	setlocale(LC_NUMERIC,sOldLocale.c_str());
	SetTransformation(m_m4x4P,m_m4x4V,false);
}



int CCutterLocationData::ReadDouble(const char *s,double &d) const
{

	size_t len = strlen(s);
	if(len==0)
		return 0;
	char *end;
	d = strtod(s,&end);
	if(errno==ERANGE)
		return 0;
	return (int)(end-s);
	
}

int CCutterLocationData::ReadDelims(const char *s) const
{
	size_t len = strlen(s);
	int nRes = 0;
	for(size_t i=0; i<len; i++)
		if(isdigit(s[i]) || s[i]=='-')
			return nRes;
		else
			nRes++;
	return nRes;
}

CCutterLocationData::CCLLine CCutterLocationData::ParseLine(int nLineBeginIndex, size_t nLineNumber)
{
	static double d[6];
	static char c[6];
	int begin = nLineBeginIndex;
	int end = begin + (int)strlen(&m_pData[begin]);
	if(begin >= end)
		return CCutterLocationData::CCLLine(CL_LINE_EMPTY,nLineNumber,nLineBeginIndex);
	if(_strnicmp(&m_pData[begin],"MSYS/",5)==0)
		return CCutterLocationData::CCLLine(CL_LINE_MSYS,nLineNumber,nLineBeginIndex);
	if(_strnicmp(&m_pData[begin],"GOTO/",5)==0){
		begin+=5;
		int nValues = 0;
		for(; nValues<6 && begin<end ;nValues++){
			int nFields = ReadDelims(&(m_pData[begin]));
			begin+=nFields;
			if(begin>=end)
				break;
			nFields = ReadDouble(&(m_pData[begin]),d[nValues]);
			begin+=nFields;
			if( begin>end || nFields<1)
				break;
		}
	if(nValues<6 )
		return CCutterLocationData::CCLLine(CL_LINE_ERROR,nLineNumber,nLineBeginIndex);
	CR3Point p(d[0],d[1],d[2]);
	CR3Vector v(d[3],d[4],d[5]);
	if(v.IsZero())
		return CCutterLocationData::CCLLine(CL_LINE_ERROR,nLineNumber,nLineBeginIndex);
	return CCutterLocationData::CCLLine
		(CL_LINE_OK
		,nLineNumber
		,nLineBeginIndex
		,p
		,v.GetUnitVector());
	} else {
		return CCutterLocationData::CCLLine(CL_LINE_ERROR,nLineNumber,nLineBeginIndex);
	}
}

void CCutterLocationData::SetTransformation(const CTransformation &trP, const CTransformation &trV,bool bInvertedOrientation)
{
	m_m4x4P = trP;
	m_m4x4V = trV;
	if(bInvertedOrientation){
		m_m4x4V.SetAt(3,3,-1.0);
		m_m4x4V.Standardize();
	}
	m_oBoxTransformed = CR3Box();
	CR3Point p0;
	m_dTrajectoryLength=0.0;
	for(size_t i = 0; i<m_vLines.size(); i++)
		if(m_vLines[i].m_status==CL_LINE_OK){
			p0 = m_m4x4P * m_vLines[i].m_p;
			break;
		}
	for(size_t i = 0; i<m_vLines.size(); i++)
		if(m_vLines[i].m_status==CL_LINE_OK){
			CR3Point p =  m_m4x4P * m_vLines[i].m_p;
			m_oBoxTransformed.ExpandToPoint(p);
			m_dTrajectoryLength+=d_E(p0,p);
			p0=p;            
		}

}

void CCutterLocationData::ResetLocationIterator(void)
{
	m_nActual=0;
}
bool CCutterLocationData::HasMoreLocations() const
{
	for(size_t i=m_nActual;i<m_vLines.size();i++){
		if(m_vLines[i].m_status==CL_LINE_OK)
			return true;
	} 
    return false;
}

bool CCutterLocationData::GetNextLocation(CR3Point & p, CR3Vector & v)
{
	if(m_nActual>=m_vLines.size()){
		//m_nActual=0;
        return false;
	}
	for(;m_nActual<m_vLines.size();m_nActual++){
		if(m_vLines[m_nActual].m_status==CL_LINE_OK){
			p = m_m4x4P * m_vLines[m_nActual].m_p;
			v = m_m4x4V * m_vLines[m_nActual].m_v;
			m_nActual++;
			return true;
		}
	}
	return false;
}

bool CCutterLocationData::GetLocation(CR3Point & p, CR3Vector & v, size_t nLineNumber) const
{
	if(nLineNumber>=m_vLines.size())
		return false;
	if(m_vLines[nLineNumber].m_status==CL_LINE_OK){
		p = m_m4x4P * m_vLines[nLineNumber].m_p;
		v = m_m4x4V * m_vLines[nLineNumber].m_v;
		return true;
	}
	return false;
}

const char *CCutterLocationData::GetLineText(size_t index) const
{
	if(index>=m_vLines.size() || m_pData==0)
		MY_THROW(CArgsMyException,"Invalid line number");
	return &(m_pData[m_vLines[index].m_nIndexOfString]);
}
