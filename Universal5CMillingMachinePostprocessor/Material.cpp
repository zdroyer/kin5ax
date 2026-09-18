#include "StdAfx.h"
#include ".\material.h"


extern CApplicationConfiguration oConfiguration;

CMaterial::CMaterial(double dXSize
					 ,double dYSize
					 ,double dZSize
					 ,double dPrecision
					 ,double dXStart
					,double dYStart
					,double dZStart)
:m_nObserverVectorIndex(0x07)
,m_bGraysShouldBeUpdated(false)
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



void CMaterial::SetObserverVector(const CR3Vector &r3vV)
{
	if(r3vV.IsZero())
		return;
	unsigned char c = 0x00;
	if(r3vV.GetX()>0.0)
		c |= 0x01;
	if(r3vV.GetY()>0)
		c |= 0x02;
	if(r3vV.GetZ()>0)
		c |= 0x04;
	m_nObserverVectorIndex = c;    
}

void CMaterial::Render3D(bool bFastRendering)
{
	glColor3d(0.4,0.25,0.1);
	RenderContour();
	if(bFastRendering)
		return;
	if(m_bGraysShouldBeUpdated)
		UpdateGrayVoxels();
	size_t nVoxelPerPixel = (size_t)(m_dPixelSize / (1.73*m_dPrecision));
	if(nVoxelPerPixel==0)
		nVoxelPerPixel = 1;
	size_t nDX=1,nDY=1,nDZ=1;
	double dDX=(double)nDX, dDY=(double)nDY, dDZ=(double)nDZ;
	double dHalf = dDX * 0.5 * m_dPrecision;

	double dX0 = m_dXStart; 
	double dY0 = m_dYStart;
	double dZ0 = m_dZStart; 
	double dX1 = m_dXStart+dDX*m_dPrecision; 
	double dY1 = m_dYStart+dDY*m_dPrecision;
	double dZ1 = m_dZStart+dDZ*m_dPrecision; 

	double *px0 = &dX0, *py0 = &dY0, *pz0 = &dZ0;
	double *px1 = &dX1, *py1 = &dY1, *pz1 = &dZ1;

	if(m_nObserverVectorIndex & 0x01){
		px0 = &dX1;
		px1 = &dX0;
	}
	if(m_nObserverVectorIndex & 0x02 ){
		py0 = &dY1;
		py1 = &dY0;
	}
	if(m_nObserverVectorIndex & 0x04 ){
		pz0 = &dZ1;
		pz1 = &dZ0;
	}
	double vx = (*px0)-(*px1);
	double vy = (*py0)-(*py1);
	double vz = (*pz0)-(*pz1);
	glShadeModel(GL_FLAT);

	if(nVoxelPerPixel>3){
		for(size_t i =0; i<m_vGrayVoxels.size(); i++){
//			m_vGrayVoxels[i].CopyCoordinates(dX0,dY0,dZ0);
			glBegin(GL_POINTS);
				glNormal3d(vx,vy,vz);						
//				glVertex3d(*px0,*py0,*pz0);
				glVertex3dv(m_vGrayVoxels[i].toGL3dv());
			glEnd();						
		}
		return;
	}
	if(nVoxelPerPixel>1){
		for(size_t i =0; i<m_vGrayVoxels.size(); i++){
			m_vGrayVoxels[i].CopyCoordinates(dX0,dY0,dZ0);
			dX1 = dX0+m_dPrecision;
			dY1 = dY0+m_dPrecision;
			dZ1 = dZ0+m_dPrecision;
			glBegin(GL_POINTS);
				glNormal3d(vx,0.0,0.0);						
				glVertex3d(*px0,dY0+dHalf ,dZ0+dHalf);

				glNormal3d(0.0,0.0,vz);
				glVertex3d(dX0+dHalf,dY0+dHalf,*pz0);

				glNormal3d(0.0,vy,0.0);
				glVertex3d(dX0+dHalf,*py0,dZ0+dHalf);
			glEnd();
		}
		return;
	} 
	for(size_t i =0; i<m_vGrayVoxels.size(); i++){
		m_vGrayVoxels[i].CopyCoordinates(dX0,dY0,dZ0);
		dX1 = dX0+m_dPrecision;
		dY1 = dY0+m_dPrecision;
		dZ1 = dZ0+m_dPrecision;
		glBegin(GL_TRIANGLE_FAN);{
			glNormal3d(vx,0.0,0.0);						
			glVertex3d(*px0,*py0,*pz0);
			glVertex3d(*px0,*py0,*pz1);
			glVertex3d(*px0,*py1,*pz1);
			glVertex3d(*px0,*py1,*pz0);
			glNormal3d(0.0,0.0,vz);
			glVertex3d(*px1,*py1,*pz0);
			glVertex3d(*px1,*py0,*pz0);
			glNormal3d(0.0,vy,0.0);
			glVertex3d(*px1,*py0,*pz1);
			glVertex3d(*px0,*py0,*pz1);			
		}glEnd();

	}
	return;
	glShadeModel(GL_SMOOTH);
/* 
	for(size_t k=0; k < m_nZ; k+=nDZ){
		size_t nZchars = k/(sizeof(unsigned char)*4);
		size_t nZbits = 2*(k%(sizeof(unsigned char)*4));
		unsigned char c = (VOXEL_GRAY<<(unsigned char)nZbits);
		unsigned char c11 = (0x03<<(unsigned char)nZbits);
		for(size_t i=0; i< m_nX ;i+=nDX){
			for(size_t j=0; j< m_nY; j+=nDY){
				//unsigned char c2 = m_box[i][j][nZchars];
				if(!((m_box[i][j][nZchars] ^ c)&c11)){
					if(nVoxelPerPixel>3){
						glBegin(GL_POINTS);
							glNormal3d(vx,vy,vz);						
							glVertex3d(*px0,*py0,*pz0);
						glEnd();						
						continue;

					}
					if(nVoxelPerPixel>1){
						glBegin(GL_POINTS);
							glNormal3d(vx,0.0,0.0);						
							glVertex3d(*px0,dY0+dHalf ,dZ0+dHalf);

							glNormal3d(0.0,0.0,vz);
							glVertex3d(dX0+dHalf,dY0+dHalf,*pz0);

							glNormal3d(0.0,vy,0.0);
							glVertex3d(dX0+dHalf,*py0,dZ0+dHalf);
						glEnd();
					} else {
						glBegin(GL_TRIANGLE_FAN);
							glNormal3d(vx,0.0,0.0);						
							glVertex3d(*px0,*py0,*pz0);
							glVertex3d(*px0,*py0,*pz1);
							glVertex3d(*px0,*py1,*pz1);
							glVertex3d(*px0,*py1,*pz0);
							glNormal3d(0.0,0.0,vz);
							glVertex3d(*px1,*py1,*pz0);
							glVertex3d(*px1,*py0,*pz0);
							glNormal3d(0.0,vy,0.0);
							glVertex3d(*px1,*py0,*pz1);
							glVertex3d(*px0,*py0,*pz1);			
						glEnd();
					}
				}
				dY0 = dY1;
				dY1 = m_dYStart +((double)(j+2)*m_dPrecision);
			}
			dY0=m_dYStart;
			dY1=m_dYStart+(dDY*m_dPrecision);
			
			dX0=dX1;
			dX1=m_dXStart+((double)(i+2)*m_dPrecision);
		}
		dX0=m_dXStart;
		dX1=m_dXStart+(dDX*m_dPrecision);
		dZ0=dZ1;
		dZ1=m_dZStart +((double)(k+2)*m_dPrecision);
	}

	glShadeModel(GL_SMOOTH);
//*/
}

void CMaterial::RenderContour() const
{
	glColor3d(0.4,0.25,0.1);
	glDisable(GL_LIGHTING);
	double dX0 = m_dXStart; 
	double dY0 = m_dYStart;
	double dZ0 = m_dZStart; 

	double dX1 = m_dXStart+m_dXSize; 
	double dY1 = m_dYStart+m_dYSize;
	double dZ1 = m_dZStart+m_dZSize; 

	glBegin(GL_LINE_LOOP);
		glVertex3d(dX0,dY0,dZ0);
		glVertex3d(dX1,dY0,dZ0);
		glVertex3d(dX1,dY1,dZ0);
		glVertex3d(dX0,dY1,dZ0);
	glEnd();
	glBegin(GL_LINE_LOOP);
		glVertex3d(dX0,dY0,dZ1);
		glVertex3d(dX1,dY0,dZ1);
		glVertex3d(dX1,dY1,dZ1);
		glVertex3d(dX0,dY1,dZ1);
	glEnd();
	glBegin(GL_LINES);
		glVertex3d(dX0,dY0,dZ1);
		glVertex3d(dX0,dY0,dZ0);

		glVertex3d(dX1,dY0,dZ1);
		glVertex3d(dX1,dY0,dZ0);

		glVertex3d(dX1,dY1,dZ1);
		glVertex3d(dX1,dY1,dZ0);

		glVertex3d(dX0,dY1,dZ1);
		glVertex3d(dX0,dY1,dZ0);
	glEnd();
	glEnable(GL_LIGHTING);
}

void CMaterial::SetPixelSize(double dPixelSize)
{
	m_dPixelSize = dPixelSize;
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
    return (size_t)abs(floor(d/m_dPrecision));
}
size_t CMaterial::MaximumIndex(double d, double dSize, size_t nMax) const
{
	if(d<=0)
		return 0;
	if(d>=dSize)
		return nMax;
    return (size_t)abs(ceil(d/m_dPrecision));
}
