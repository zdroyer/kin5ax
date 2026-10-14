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

double CMaterial::GetTotalVolume() const
{
	return (double)(m_nX*m_nY*m_nZ)*GetVoxelVolume();
}

double CMaterial::GetRemovedVolume() const
{
	return (double)GetVoxelCount(VOXEL_BLACK)*GetVoxelVolume();
}

const std::vector<CR3Point> &CMaterial::GetSurfaceVoxels()
{
	if(m_bGraysShouldBeUpdated)
		UpdateGrayVoxels();
	return m_vGrayVoxels;
}

size_t CMaterial::GetSurfaceTriangles(std::vector<double> &vOut) const
{
	//the six sides of a voxel: the step to the neighbour behind the side, and the two edges of the
	//side as the voxel's own axes, ordered so that the first edge crossed with the second points out
	static const int nSides[6][3] = {{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
	static const int nEdges[6][2][3] = {
		{{0,1,0},{0,0,1}}, {{0,0,1},{0,1,0}},		//+x: y then z; -x: z then y
		{{0,0,1},{1,0,0}}, {{1,0,0},{0,0,1}},		//+y: z then x; -y: x then z
		{{1,0,0},{0,1,0}}, {{0,1,0},{1,0,0}}};		//+z: x then y; -z: y then x
	const double p = m_dPrecision;
	size_t nFaces = 0;
	for(size_t i=0;i<m_nX;i++)
		for(size_t j=0;j<m_nY;j++)
			for(size_t k=0;k<m_nZ;k++){
				if(Is(VOXEL_BLACK,i,j,k))
					continue;
				for(int s=0;s<6;s++){
					//the neighbour behind the side, with the outside counting as removed; a step below
					//zero wraps to a huge index, which Is() takes for outside as well
					const size_t ni = i+(size_t)nSides[s][0], nj = j+(size_t)nSides[s][1], nk = k+(size_t)nSides[s][2];
					if(!Is(VOXEL_BLACK,ni,nj,nk))
						continue;
					//the corner of the side nearest the start of the block: the voxel corner, moved one
					//voxel along the side's axis when the side faces the positive way
					double c[3] = {m_dXStart+(double)i*p,m_dYStart+(double)j*p,m_dZStart+(double)k*p};
					for(int a=0;a<3;a++)
						if(nSides[s][a]>0)
							c[a] += p;
					double u[3], v[3];
					for(int a=0;a<3;a++){
						u[a] = c[a]+p*(double)nEdges[s][0][a];
						v[a] = c[a]+p*(double)nEdges[s][1][a];
					}
					double w[3];
					for(int a=0;a<3;a++)
						w[a] = c[a]+p*(double)(nEdges[s][0][a]+nEdges[s][1][a]);
					//two triangles: corner, along the first edge, the far corner; corner, far, along the second
					const double *pTriangles[6] = {c,u,w,c,w,v};
					for(int t=0;t<6;t++)
						for(int a=0;a<3;a++)
							vOut.push_back(pTriangles[t][a]);
					nFaces++;
				}
			}
	return nFaces;
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

bool CMaterial::HasMaterialInNonCuttingPart(const CMillingTool &oTool, CR3Point &r3pFirst) const
{
	if(!oTool.HasNonCuttingPart())
		return false;
	CR3Box oEnvelope = oTool.GetNonCuttingPartBoundingBox();
	if(oEnvelope.IsEmpty())
		return false;
	size_t nMinI=0, nMaxI=m_nX;
	size_t nMinJ=0, nMaxJ=m_nY;
	size_t nMinK=0, nMaxK=m_nZ;
	GetIndexesOfBox(nMinI,nMaxI,nMinJ,nMaxJ,nMinK,nMaxK,oEnvelope);
	for(size_t i = nMinI; i<=nMaxI; i++)
		for(size_t j = nMinJ; j<=nMaxJ; j++)
			for(size_t k = nMinK; k<=nMaxK; k++){
				if(Is(VOXEL_BLACK,i,j,k))
					continue;	//removed already, or outside the block
				CR3Point p(m_dXStart+(double)i*m_dPrecision
						,m_dYStart+(double)j*m_dPrecision
						,m_dZStart+(double)k*m_dPrecision);
				if(oTool.IsInNonCuttingPart(p)){
					r3pFirst = p;
					return true;
				}
			}
	return false;
}

bool CMaterial::HasMaterialInNonCuttingPart(const CMillingTool &oTool) const
{
	CR3Point p;
	return HasMaterialInNonCuttingPart(oTool,p);
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
