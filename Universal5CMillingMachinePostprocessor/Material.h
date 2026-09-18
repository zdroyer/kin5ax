#pragma once

#include "3DGeometry/r3point.h"
#include "3DGeometry/r3vector.h"
#include "millingtool.h"

typedef vector<unsigned char> uchar_vector;
typedef vector<uchar_vector> uchar_matrix;
typedef vector<uchar_matrix> uchar_box;

enum VOXEL_COLOR{
	VOXEL_BLACK=0x00,
	VOXEL_GRAY=0x01,
	VOXEL_WHITE=0x03};

class CMaterial
{
protected:
	uchar_box m_box;
	bool m_bGraysShouldBeUpdated;
	size_t m_nZ,m_nZInner, m_nX, m_nY;

	double m_dXSize, m_dYSize,m_dZSize;
	double m_dXStart, m_dYStart,m_dZStart;
	double m_dPrecision;
	double m_dPrecisionDiv2;

	size_t GetBoxCount(double dSize, double dPrecision) const;


	unsigned char m_nObserverVectorIndex;
	double m_dPixelSize;
	size_t MinimumIndex(double d, double dSize, size_t nMax) const;
	size_t MaximumIndex(double d, double dSize, size_t nMax) const;
	vector<CR3Point> m_vGrayVoxels;
public:
	void SetObserverVector(const CR3Vector &r3vV);
	void RenderContour() const;
inline bool Is(VOXEL_COLOR color, size_t nX, size_t nY, size_t nZ) const
{
	if(nX>=m_nX ||  nY>=m_nY || nZ>=m_nZ){
		if(color == VOXEL_BLACK)
			return true;
		return false;
	}
	size_t nZ1 = nZ/(sizeof(unsigned char)*4);
	size_t nZ2 = 2*(nZ%(sizeof(unsigned char)*4));
	unsigned char c = (m_box[nX][nY][nZ1]);
	c >>= (unsigned char)nZ2;
	c &= (0x03);
	return c==color;
}

void UpdateGrayVoxels(){
	m_vGrayVoxels.erase(m_vGrayVoxels.begin(),m_vGrayVoxels.end());
	for(size_t i=0;i<m_nX;i++)
		for(size_t j=0;j<m_nY;j++)
			for(size_t k=0;k<m_nZ;k++)
				if(ShouldBeGray(i,j,k)){
					SetVoxelColor(i,j,k,VOXEL_GRAY);
					m_vGrayVoxels.push_back(CR3Point(m_dXStart+(double)i*m_dPrecision
													,m_dYStart+(double)j*m_dPrecision
													,m_dZStart+(double)k*m_dPrecision));
				}
	m_bGraysShouldBeUpdated=false;
}

inline void SetVoxelColor(size_t nX, size_t nY, size_t nZ, VOXEL_COLOR color){
	if(nX>=m_nX ||  nY>=m_nY || nZ>=m_nZ)
		return;
	size_t nZ1 = nZ/(sizeof(unsigned char)*4);
	size_t nZ2 = 2*(nZ%(sizeof(unsigned char)*4));

	unsigned char & rc = m_box[nX][nY][nZ1];
	
	unsigned char c = ((0x03 << (unsigned char)nZ2) ^ 0xFF) ;
	m_box[nX][nY][nZ1] &= c;
	m_box[nX][nY][nZ1] |= (color<<nZ2);
	
}


inline bool ShouldBeGray(size_t nX, size_t nY, size_t nZ) const
{
//	if(nX>=m_nX ||  nY>=m_nY || nZ>=m_nZ2)
//		return false;
//	size_t nZ1 = nZ/(sizeof(unsigned char)/2);
//	size_t nZ2 = nZ%(sizeof(unsigned char)/2);
//	return ((m_box[nX][nY][nZ1]>>nZ2)&(0x03))==VOXEL_GRAY;

	if(Is(VOXEL_BLACK,nX,nY,nZ))
		return false;
	if(Is(VOXEL_BLACK,nX,nY,nZ+1))
		return true;
	if(Is(VOXEL_BLACK,nX,nY,nZ-1))
		return true;
	if(Is(VOXEL_BLACK,nX,nY+1,nZ))
		return true;
	if(Is(VOXEL_BLACK,nX,nY-1,nZ))
		return true;
	if(Is(VOXEL_BLACK,nX+1,nY,nZ))
		return true;
	if(Is(VOXEL_BLACK,nX-1,nY,nZ))
			return true;
	return false;
}


	CMaterial
		(double dXSize=0.7
		,double dYSize = 0.7
		,double dZSize = 0.2
		,double dPrecision=0.05
		,double dXStart = -0.35
		,double dYStart = -0.35
		,double dZStart = 0.0);
	~CMaterial(void);
	void ResetMaterial();

	void GenerateBox
		(double dXSize
		,double dYSize
		,double dZSize
		,double dPrecision
		,double dXStart
		,double dYStart
		,double dZStart);


	void Render3D(bool bFastRendering=false);

	void GetParameters
			(double &dXSize
			,double &dYSize
			,double &dZSize
			,double &dPrecision
			,double &dXStart
			,double &dYStart
			,double &dZStart) const ;

	void SetPixelSize(double dPixelSize);
	void MachineCut(CMillingTool & oCutter);
	void GetIndexesOfBox(size_t &nMinI, size_t &nMaxI
							   ,size_t &nMinJ, size_t &nMaxJ
							   ,size_t &nMinK, size_t &nMaxK
							   ,CR3Box &box) const;
};
