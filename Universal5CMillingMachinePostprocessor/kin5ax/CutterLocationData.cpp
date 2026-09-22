#include "stdafx.h"
#include "CutterLocationData.h"

//case-insensitive comparison of the first n characters (the keywords GOTO/ and MSYS/)
namespace kin5ax {

static int strncmp_nocase(const char *a, const char *b, size_t n)
{
	for(size_t i=0;i<n;i++){
		int ca = tolower((unsigned char)a[i]), cb = tolower((unsigned char)b[i]);
		if(ca!=cb)
			return ca-cb;
		if(ca==0)
			return 0;
	}
	return 0;
}


CCutterLocationData::CCutterLocationData(void)
: m_nErrors(0)
, m_nActual(0)
, m_nLocations(0)
, m_dTrajectoryLength(0.0)
, m_dTotalOrientationChangeRadians(0.0)
{
}

CCutterLocationData::~CCutterLocationData(void)
{
}

void CCutterLocationData::LoadFile(const std::string & sFileName, const std::string &sFileTitle)
{
	FILE *pFile = fopen(sFileName.c_str(),"rb");
	if(pFile==NULL)
		MY_THROW(CArgsMyException,"Problem with opening of file: "+sFileName+"\n"+strerror(errno));
	if(fseek(pFile,0,SEEK_END)!=0){
		fclose(pFile);
		MY_THROW(CArgsMyException,"Problem with obtaining size of file: "+sFileName+"\n"+strerror(errno));
	}
	long nFileLength = ftell(pFile);
	if(nFileLength<0 || (unsigned long)nFileLength > m_vLines.max_size()){
		fclose(pFile);
		MY_THROW(CArgsMyException,"Problem with obtaining size of file: "+sFileName);
	}
	rewind(pFile);
	std::vector<char> vData((size_t)nFileLength+1,0);
	size_t nCharsRead = fread(&vData[0],1,(size_t)nFileLength,pFile);
	bool bError = ferror(pFile)!=0;
	fclose(pFile);
	if(bError)
		MY_THROW(CArgsMyException,"Cannot read file: "+sFileName+"\n"+strerror(errno));
	vData.resize(nCharsRead+1);
	vData[nCharsRead]=0;
	m_vData.swap(vData);
	m_vLines.clear();

	m_sFileName = sFileName;
	m_sFileTitle = sFileTitle;
	std::string sOldLocale(setlocale(LC_NUMERIC,NULL));
	setlocale(LC_NUMERIC,"C");	//the decimal separator of the file is the point
	m_nErrors=0;
	m_nActual=0;
	m_nLocations=0;
	m_dTotalOrientationChangeRadians = 0.0;
	m_dTrajectoryLength = 0.0;
	m_oBox = CR3Box();
	CR3Vector r3vLast;
	//every line, the last one with or without its newline; the line ends become terminators
	for(size_t i=0;i<nCharsRead;){
		size_t nEnd = i;
		while(nEnd<nCharsRead && m_vData[nEnd]!='\n')
			nEnd++;
		size_t nNext = nEnd+1;
		m_vData[nEnd]=0;
		if(nEnd>i && m_vData[nEnd-1]=='\r')
			m_vData[nEnd-1]=0;
		m_vLines.push_back(ParseLine(i,m_vLines.size()));
		if(m_vLines.back().m_status==CL_LINE_OK) {
			m_oBox.ExpandToPoint(m_vLines.back().m_p);
			if(m_nLocations>0)
				m_dTotalOrientationChangeRadians+= r3vLast.UndirectedAngle(m_vLines.back().m_v);
			r3vLast = m_vLines.back().m_v;
			m_nLocations++;
		} else if(m_vLines.back().m_status==CL_LINE_ERROR)
			m_nErrors++;
		i = nNext;
	}
	setlocale(LC_NUMERIC,sOldLocale.c_str());
	SetTransformation(m_m4x4P,m_m4x4V,false);
}

size_t CCutterLocationData::ReadDouble(const char *s,double &d) const
{
	if(*s==0)
		return 0;
	char *end;
	errno = 0;	//strtod only sets it, never clears it
	d = strtod(s,&end);
	if(errno==ERANGE && fabs(d)==HUGE_VAL)
		return 0;
	return (size_t)(end-s);
}

size_t CCutterLocationData::ReadDelims(const char *s) const
{
	size_t nRes = 0;
	for(; s[nRes]!=0; nRes++)
		if(isdigit((unsigned char)s[nRes]) || s[nRes]=='-' || s[nRes]=='+' || s[nRes]=='.')
			break;
	return nRes;
}

CCutterLocationData::CCLLine CCutterLocationData::ParseLine(size_t nLineBeginIndex, size_t nLineNumber)
{
	const char *szLine = &m_vData[nLineBeginIndex];
	size_t nLength = strlen(szLine);
	if(nLength==0)
		return CCutterLocationData::CCLLine(CL_LINE_EMPTY,nLineNumber,nLineBeginIndex);
	if(strncmp_nocase(szLine,"MSYS/",5)==0)
		return CCutterLocationData::CCLLine(CL_LINE_MSYS,nLineNumber,nLineBeginIndex);
	if(strncmp_nocase(szLine,"GOTO/",5)!=0)
		return CCutterLocationData::CCLLine(CL_LINE_ERROR,nLineNumber,nLineBeginIndex);
	double d[6];
	size_t nPos = 5;
	int nValues = 0;
	for(; nValues<6 && nPos<nLength; nValues++){
		nPos += ReadDelims(szLine+nPos);
		if(nPos>=nLength)
			break;
		size_t nUsed = ReadDouble(szLine+nPos,d[nValues]);
		if(nUsed==0)
			break;
		nPos += nUsed;
	}
	if(nValues<6)
		return CCutterLocationData::CCLLine(CL_LINE_ERROR,nLineNumber,nLineBeginIndex);
	CR3Point p(d[0],d[1],d[2]);
	CR3Vector v(d[3],d[4],d[5]);
	if(v.IsZero())
		return CCutterLocationData::CCLLine(CL_LINE_ERROR,nLineNumber,nLineBeginIndex);
	return CCutterLocationData::CCLLine(CL_LINE_OK,nLineNumber,nLineBeginIndex,p,v.GetUnitVector());
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
	if(index>=m_vLines.size() || m_vData.empty())
		MY_THROW(CArgsMyException,"Invalid line number");
	return &(m_vData[m_vLines[index].m_nIndexOfString]);
}

} // namespace kin5ax
