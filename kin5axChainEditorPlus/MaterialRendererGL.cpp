#include "StdAfx.h"
#include "MaterialRendererGL.h"

//one bit per axis, set when the observer looks from the positive side: the faces of a voxel that face him
static unsigned char ObserverIndex(const CR3Vector &r3vObserver)
{
	if(r3vObserver.IsZero())
		return 0x07;
	unsigned char c = 0x00;
	if(r3vObserver.GetX()>0.0)
		c |= 0x01;
	if(r3vObserver.GetY()>0.0)
		c |= 0x02;
	if(r3vObserver.GetZ()>0.0)
		c |= 0x04;
	return c;
}

void CMaterialRendererGL::Render(CMaterial &oMaterial, const CR3Vector &r3vObserver, double dPixelSize, bool bFastRendering)
{
	glColor3d(0.4,0.25,0.1);
	RenderContour(oMaterial);
	if(bFastRendering)
		return;
	const std::vector<CR3Point> &vSurface = oMaterial.GetSurfaceVoxels();
	const double dPrecision = oMaterial.GetPrecision();
	size_t nVoxelPerPixel = (size_t)(dPixelSize / (1.73*dPrecision));
	if(nVoxelPerPixel==0)
		nVoxelPerPixel = 1;
	const double dHalf = 0.5*dPrecision;

	//the near corner (x0,y0,z0) and the far corner (x1,y1,z1) of a voxel as seen by the observer
	double dX0 = 0.0, dY0 = 0.0, dZ0 = 0.0;
	double dX1 = dPrecision, dY1 = dPrecision, dZ1 = dPrecision;
	double *px0 = &dX0, *py0 = &dY0, *pz0 = &dZ0;
	double *px1 = &dX1, *py1 = &dY1, *pz1 = &dZ1;
	const unsigned char nObserver = ObserverIndex(r3vObserver);
	if(nObserver & 0x01){
		px0 = &dX1;
		px1 = &dX0;
	}
	if(nObserver & 0x02){
		py0 = &dY1;
		py1 = &dY0;
	}
	if(nObserver & 0x04){
		pz0 = &dZ1;
		pz1 = &dZ0;
	}
	//the normals of the visible faces
	const double vx = (*px0)-(*px1);
	const double vy = (*py0)-(*py1);
	const double vz = (*pz0)-(*pz1);
	glShadeModel(GL_FLAT);

	if(nVoxelPerPixel>3){
		//many voxels per pixel: one point per surface voxel
		for(size_t i=0;i<vSurface.size();i++){
			glBegin(GL_POINTS);
				glNormal3d(vx,vy,vz);
				glVertex3dv(vSurface[i].toGL3dv());
			glEnd();
		}
		return;
	}
	if(nVoxelPerPixel>1){
		//a point in the middle of each visible face
		for(size_t i=0;i<vSurface.size();i++){
			vSurface[i].CopyCoordinates(dX0,dY0,dZ0);
			dX1 = dX0+dPrecision;
			dY1 = dY0+dPrecision;
			dZ1 = dZ0+dPrecision;
			glBegin(GL_POINTS);
				glNormal3d(vx,0.0,0.0);
				glVertex3d(*px0,dY0+dHalf,dZ0+dHalf);
				glNormal3d(0.0,0.0,vz);
				glVertex3d(dX0+dHalf,dY0+dHalf,*pz0);
				glNormal3d(0.0,vy,0.0);
				glVertex3d(dX0+dHalf,*py0,dZ0+dHalf);
			glEnd();
		}
		return;
	}
	//the three visible faces of each surface voxel
	for(size_t i=0;i<vSurface.size();i++){
		vSurface[i].CopyCoordinates(dX0,dY0,dZ0);
		dX1 = dX0+dPrecision;
		dY1 = dY0+dPrecision;
		dZ1 = dZ0+dPrecision;
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

void CMaterialRendererGL::RenderContour(const CMaterial &oMaterial)
{
	double dXSize, dYSize, dZSize, dPrecision, dX0, dY0, dZ0;
	oMaterial.GetParameters(dXSize,dYSize,dZSize,dPrecision,dX0,dY0,dZ0);
	const double dX1 = dX0+dXSize, dY1 = dY0+dYSize, dZ1 = dZ0+dZSize;

	glColor3d(0.4,0.25,0.1);
	glDisable(GL_LIGHTING);
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
