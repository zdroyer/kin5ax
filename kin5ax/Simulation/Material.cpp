#include "../stdafx.h"
#include "Material.h"

namespace kin5ax {



CMaterial::CMaterial(double dXSize
					 ,double dYSize
					 ,double dZSize
					 ,double dPrecision
					 ,double dXStart
					,double dYStart
					,double dZStart)
:m_bGraysShouldBeUpdated(false)
{
	GenerateBox(dXSize,dYSize,dZSize,dPrecision,dXStart,dYStart,dZStart);
}

void CMaterial::GetParameters
					(double &dXSize
					,double &dYSize
					,double &dZSize
					,double &dPrecision
					,double &dXStart
					,double &dYStart
					,double &dZStart) const 
{
	dXSize = m_dXSize;
	dYSize = m_dYSize;
	dZSize = m_dZSize;

	dPrecision = m_dPrecision;

	dXStart = m_dXStart;
	dYStart = m_dYStart;
	dZStart = m_dZStart;
}


CMaterial::~CMaterial(void)
{
}


size_t CMaterial::GetBoxCount(double dSize, double dPrecision) const
{
	double d = fabs(dSize/dPrecision);
	size_t nD = (size_t)d;
	if(d == nD)
		return nD;
	return nD+1;
}
void CMaterial::ResetMaterial()
{
	for(size_t i=0; i< m_nX ;i++)
		for(size_t j=0; j< m_nY; j++)
			for(size_t k=0; k < m_nZ; k++)
				SetVoxelColor(i,j,k,VOXEL_WHITE);

	for(size_t i=0; i< m_nX ;i++)
		for(size_t j=0; j< m_nY; j++)
			for(size_t k=0; k < m_nZ; k++)
				if(ShouldBeGray(i,j,k))
					SetVoxelColor(i,j,k,VOXEL_GRAY);	

}
void CMaterial::GenerateBox(double dXSize,
							double dYSize,
							double dZSize, 
							double dPrecision,
							double dXStart,
							double dYStart,
							double dZStart)
{
	m_dXSize = dXSize;
	m_dYSize = dYSize;
	m_dZSize = dZSize;

	m_dXStart = dXStart;
	m_dYStart = dYStart;
	m_dZStart = dZStart;


	m_dPrecision = dPrecision;
	m_dPrecisionDiv2 = m_dPrecision*0.5;

	m_nX = GetBoxCount(m_dXSize, m_dPrecision);
	m_nY = GetBoxCount(m_dYSize, m_dPrecision);
	m_nZ = GetBoxCount(m_dZSize,dPrecision);
	
	m_nZInner = m_nZ/(sizeof(unsigned char)*4);
	if(m_nZ%(sizeof(unsigned char)*4))
		m_nZInner++;
	m_box = uchar_box(m_nX,uchar_matrix(m_nY,uchar_vector(m_nZInner,0xFF)));

	m_vGrayVoxels.erase(m_vGrayVoxels.begin(),m_vGrayVoxels.end());
	m_vGrayVoxels.reserve(2*((m_nX*m_nY)+(m_nX*m_nZ)+(m_nY*m_nZ)));
	UpdateGrayVoxels();
	for(size_t i=0; i< m_nX ;i++)
		for(size_t j=0; j< m_nY; j++)
			for(size_t k=0; k < m_nZ; k++)
				if(ShouldBeGray(i,j,k))
					SetVoxelColor(i,j,k,VOXEL_GRAY);
	

}



void CMaterial::GetDimensions(size_t &nX, size_t &nY, size_t &nZ) const
{
	nX = m_nX;
	nY = m_nY;
	nZ = m_nZ;
}

size_t CMaterial::GetVoxelCount(VOXEL_COLOR color) const
{
	size_t n = 0;
	for(size_t i=0;i<m_nX;i++)
		for(size_t j=0;j<m_nY;j++)
			for(size_t k=0;k<m_nZ;k++)
				if(Is(color,i,j,k))
					n++;
	return n;
}

const std::vector<CR3Point> &CMaterial::GetSurfaceVoxels()
{
	if(m_bGraysShouldBeUpdated)
		UpdateGrayVoxels();
	return m_vGrayVoxels;
}

void CMaterial::MachineCut(CMillingTool & oCutter)
{
	CR3Box b =  oCutter.GetCuttingPartBoundingBox();
	size_t nMinI=0, nMaxI=m_nX;
	size_t nMinJ=0, nMaxJ=m_nY;
	size_t nMinK=0, nMaxK=m_nZ;
	GetIndexesOfBox(nMinI,nMaxI,nMinJ,nMaxJ,nMinK,nMaxK,b);
	for(size_t i = nMinI; i<=nMaxI; i++)
		for(size_t j = nMinJ; j<=nMaxJ; j++)
			for(size_t k = nMinK; k<=nMaxK; k++){
				CUT_TYPE nType = oCutter.IsCut(CR3Point(m_dXStart+(double)i*m_dPrecision,
							m_dYStart+(double)j*m_dPrecision,
							m_dZStart+(double)k*m_dPrecision));
				if(is_point_cut(nType)){
					m_bGraysShouldBeUpdated=true;
					SetVoxelColor(i,j,k,VOXEL_BLACK);
					SetVoxelColor(i,j-1,k,VOXEL_BLACK);
					SetVoxelColor(i,j-1,k-1,VOXEL_BLACK);
					SetVoxelColor(i,j,k-1,VOXEL_BLACK);
					SetVoxelColor(i-1,j,k,VOXEL_BLACK);
					SetVoxelColor(i-1,j-1,k,VOXEL_BLACK);
					SetVoxelColor(i-1,j-1,k-1,VOXEL_BLACK);
					SetVoxelColor(i-1,j,k-1,VOXEL_BLACK);
				}

			}
	//todo: collisions
}

void CMaterial::GetIndexesOfBox(size_t &nMinI, size_t &nMaxI
							   ,size_t &nMinJ, size_t &nMaxJ
							   ,size_t &nMinK, size_t &nMaxK
							   ,CR3Box &box) const
{
	CR3Point min = box.GetMinPoint();
	CR3Point max = box.GetMaxPoint();
	nMinI = MinimumIndex(min.GetX()-m_dXStart,m_dXSize,m_nX);
	nMaxI = MaximumIndex(max.GetX()-m_dXStart,m_dXSize,m_nX);
	nMinJ = MinimumIndex(min.GetY()-m_dYStart,m_dYSize,m_nY);
	nMaxJ = MaximumIndex(max.GetY()-m_dYStart,m_dYSize,m_nY);
	nMinK = MinimumIndex(min.GetZ()-m_dZStart,m_dZSize,m_nZ);
	nMaxK = MaximumIndex(max.GetZ()-m_dZStart,m_dZSize,m_nZ);
}

size_t CMaterial::MinimumIndex(double d, double dSize, size_t nMax) const
{
	if(d<=0)
		return 0;
	if(d>=dSize)
		return nMax;
    return (size_t)fabs(floor(d/m_dPrecision));
}
size_t CMaterial::MaximumIndex(double d, double dSize, size_t nMax) const
{
	if(d<=0)
		return 0;
	if(d>=dSize)
		return nMax;
    return (size_t)fabs(ceil(d/m_dPrecision));
}

} // namespace kin5ax
