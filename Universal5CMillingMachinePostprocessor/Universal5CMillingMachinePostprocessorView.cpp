// Universal5CMillingMachinePostprocessorView.cpp : implementation of the CUniversal5CMillingMachinePostprocessorView class
//

#include "stdafx.h"
#include "ArgsMyException.h"

#include "Universal5CMillingMachinePostprocessor.h"

#include "Universal5CMillingMachinePostprocessorDoc.h"
#include "Universal5CMillingMachinePostprocessorView.h"
#include ".\universal5cmillingmachinepostprocessorview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CUniversal5CMillingMachinePostprocessorView

extern CApplicationConfiguration oConfiguration;

IMPLEMENT_DYNCREATE(CUniversal5CMillingMachinePostprocessorView, CView)

BEGIN_MESSAGE_MAP(CUniversal5CMillingMachinePostprocessorView, CView)
	ON_WM_CREATE()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_COMMAND(ID_3DVIEW_ORIGINAL_ORIENATION, On3dviewOriginalOrienation)
	ON_COMMAND(ID_3DVIEW_ORIGINAL_VIEW, On3dviewOriginalView)
	ON_COMMAND(ID_3DVIEW_ORIGINAL_POSITION, On3dviewOriginalPosition)
	ON_COMMAND(ID_3DVIEW_ORIGINAL_SCALE, On3dviewOriginalScale)
	ON_COMMAND(ID_3DVIEW_ZOOM_OUT, On3dviewZoomOut)
	ON_COMMAND(ID_3DVIEW_ZOOM_IN, On3dviewZoomIn)
	ON_COMMAND(ID_3DVIEW_ORTHOGONAL_PROJECTION, On3dviewOrthogonalProjection)
	ON_COMMAND(ID_3DVIEW_PERSPECTIVE_PROJECTION, On3dviewPerspectiveProjection)
	ON_COMMAND(ID_3DVIEW_OPTIMAL_SIZE, On3dviewOptimalSize)
END_MESSAGE_MAP()

// CUniversal5CMillingMachinePostprocessorView construction/destruction

CUniversal5CMillingMachinePostprocessorView::CUniversal5CMillingMachinePostprocessorView()
{
	// TODO: add construction code here

}

CUniversal5CMillingMachinePostprocessorView::~CUniversal5CMillingMachinePostprocessorView()
{
}

BOOL CUniversal5CMillingMachinePostprocessorView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CUniversal5CMillingMachinePostprocessorView drawing

void CUniversal5CMillingMachinePostprocessorView::OnDraw(CDC* pDC)
{
	CUniversal5CMillingMachinePostprocessorDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	pDoc->m_oMaterial.SetObserverVector(m_o3DViewManipulator.GetObserverVector(pDoc->m_oChain.GetActualToMaterialInvertedTransformation()));
	pDoc->m_oMaterial.SetPixelSize(m_o3DViewManipulator.GetPixelSize(0.0));
	glMatrixMode(GL_MODELVIEW);		
	pDoc->Render3D(m_o3DViewManipulator.IsInAction());

	glColor3d(1.0,0.0,0.0);
	GLUquadricObj *pq =  gluNewQuadric();

	SwapBuffers(*pDC);
	m_o3DViewManipulator.Paint(*pDC);
}


// CUniversal5CMillingMachinePostprocessorView diagnostics

#ifdef _DEBUG
void CUniversal5CMillingMachinePostprocessorView::AssertValid() const
{
	CView::AssertValid();
}

void CUniversal5CMillingMachinePostprocessorView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CUniversal5CMillingMachinePostprocessorDoc* CUniversal5CMillingMachinePostprocessorView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CUniversal5CMillingMachinePostprocessorDoc)));
	return (CUniversal5CMillingMachinePostprocessorDoc*)m_pDocument;
}
#endif //_DEBUG


// CUniversal5CMillingMachinePostprocessorView message handlers

int CUniversal5CMillingMachinePostprocessorView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	try{
		CreateGLRenderingContext();
		InitGL();
		return 0;
	} catch (CGeneratorException e){
		this->GetParent()->MessageBox(e.GetMessage().c_str());
	}
	return -1;
}

void CUniversal5CMillingMachinePostprocessorView::OnMouseMove(UINT nFlags, CPoint point)
{
	if(m_o3DViewManipulator.OnMouseMove(nFlags,point))
		Invalidate(FALSE);
	//CView::OnMouseMove(nFlags, point);
}

void CUniversal5CMillingMachinePostprocessorView::OnLButtonDown(UINT nFlags, CPoint point)
{
	CView::OnLButtonDown(nFlags, point);
	m_o3DViewManipulator.OnLButtonDown(nFlags,point);
	Invalidate(FALSE);
}

void CUniversal5CMillingMachinePostprocessorView::OnLButtonUp(UINT nFlags, CPoint point)
{
	CView::OnLButtonUp(nFlags, point);
	m_o3DViewManipulator.OnLButtonUp(nFlags,point);
	Invalidate(FALSE);
}

void CUniversal5CMillingMachinePostprocessorView::OnRButtonDown(UINT nFlags, CPoint point)
{
	CView::OnRButtonDown(nFlags, point);
	m_o3DViewManipulator.OnRButtonDown(nFlags,point);
	Invalidate(FALSE);

}


void CUniversal5CMillingMachinePostprocessorView::OnRButtonUp(UINT nFlags, CPoint point)
{
	CView::OnRButtonUp(nFlags, point);
	m_o3DViewManipulator.OnRButtonUp(nFlags,point);
	Invalidate(FALSE);
}

BOOL CUniversal5CMillingMachinePostprocessorView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default
	return TRUE;
	//return CView::OnEraseBkgnd(pDC);
}

void CUniversal5CMillingMachinePostprocessorView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);
	m_o3DViewManipulator.OnSize(nType,cx,cy);

}

void CUniversal5CMillingMachinePostprocessorView::OnDestroy()
{
	CView::OnDestroy();
	wglDeleteContext(m_hGLrc);	
}
void CUniversal5CMillingMachinePostprocessorView::CreateGLRenderingContext(void)
{
    PIXELFORMATDESCRIPTOR pfd = { 
    sizeof(PIXELFORMATDESCRIPTOR),  //  size of this pfd 
    1,                     // version number 
    PFD_DRAW_TO_WINDOW |   // support window 
    PFD_SUPPORT_OPENGL |   // support OpenGL 
    PFD_GENERIC_FORMAT |
    PFD_DOUBLEBUFFER,      // double buffered 
    PFD_TYPE_RGBA,         // RGBA type 
    32,                    // 32-bit color depth 
    0, 0, 0, 0, 0, 0,      // color bits ignored 
    8,                     // no alpha buffer 
    0,                     // shift bit ignored 
    8,                     // no accumulation buffer 
    0, 0, 0, 0,            // accum bits ignored 
    64,                    // 64-bit z-buffer	 
    8,                     // stencil buffer 
    8,                     // auxiliary buffer 
    PFD_MAIN_PLANE,        // main layer 
    0,                     // reserved 
    0, 0, 0                // layer masks ignored 
    }; 

	CPaintDC dc(this);
    int nPixelFormat; 

	if ( (nPixelFormat = ::ChoosePixelFormat(dc, &pfd)) == 0 ) 
		MY_THROW(CGeneratorException,"OpenGL not initialized: /nChoosePixelFormat failed."); 
 
	if (::SetPixelFormat(dc, nPixelFormat, &pfd) == FALSE) 
		MY_THROW(CGeneratorException,"OpenGL not initialized: /nSetPixelFormat failed."); 
	m_hGLrc = ::wglCreateContext(dc);	// Create an OpenGL rendering context 
	if(m_hGLrc==NULL){			
		MY_THROW(CGeneratorException,"OpenGL not initialized: /nCannot create rendering context.");
	}
	if(::wglMakeCurrent(dc,m_hGLrc) == FALSE ){
		MY_THROW(CGeneratorException,"OpenGL not initialized: /nCannot associate rendering context with window context.");
	}
}


void CUniversal5CMillingMachinePostprocessorView::InitGL(void)
{
	glShadeModel(GL_SMOOTH); // Enables Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f); // Depth Buffer Setup
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NEAREST);
//	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NEAREST ); 
	//glBlendFunc(GL_SRC_ALPHA,GL_ONE);
	glEnable(GL_DEPTH_TEST); // Enables Depth Testing
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); 

	//	glEnable(GL_TEXTURE_2D);
	//glDepthFunc(GL_LEQUAL); 

	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_NORMALIZE);
	glEnable(GL_LIGHTING);

	glPolygonMode(GL_FRONT, GL_FILL);
	glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_FALSE);
	glLightModelf(GL_LIGHT_MODEL_TWO_SIDE,GL_FALSE);
	
	GLfloat ambient [] = { 0.2f, 0.2f, 0.2f, 1.0f };
	GLfloat difusse [] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat material_specular [] = { 0.2f, 0.2f, 0.2f, 1.0f };
	GLfloat material_shininess [] = { 50.0f };

	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material_specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, material_shininess);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, ambient);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	GLfloat position [] = { 20.0f, 10.0f, 80.0f, 1.0f };
	glLightfv(GL_LIGHT0,GL_POSITION, position);
	glLightfv(GL_LIGHT0,GL_AMBIENT,ambient);
	glLightfv(GL_LIGHT0,GL_DIFFUSE,difusse);
	glEnable(GL_LIGHT0);

}

void CUniversal5CMillingMachinePostprocessorView::On3dviewOriginalOrienation()
{
	m_o3DViewManipulator.ResetOrientation();
	Invalidate(FALSE);
}

void CUniversal5CMillingMachinePostprocessorView::On3dviewOriginalView()
{
	m_o3DViewManipulator.ResetView();
	Invalidate(FALSE);
}

void CUniversal5CMillingMachinePostprocessorView::On3dviewOriginalPosition()
{
	m_o3DViewManipulator.ResetPosition();
	Invalidate(FALSE);
}

void CUniversal5CMillingMachinePostprocessorView::On3dviewOriginalScale()
{
	m_o3DViewManipulator.ResetScale();
	m_o3DViewManipulator.ResetPosition();
	Invalidate(FALSE);
}

void CUniversal5CMillingMachinePostprocessorView::On3dviewZoomOut()
{
	m_o3DViewManipulator.ZoomOut();
	Invalidate(FALSE);
}

void CUniversal5CMillingMachinePostprocessorView::On3dviewZoomIn()
{
	m_o3DViewManipulator.ZoomIn();
	Invalidate(FALSE);
}

void CUniversal5CMillingMachinePostprocessorView::On3dviewOrthogonalProjection()
{
	oConfiguration.m_bPerspectiveProjection = false;
	m_o3DViewManipulator.UpdateProjectionMatrix();
	GetParent()->GetMenu()->CheckMenuRadioItem(ID_3DVIEW_ORTHOGONAL_PROJECTION,ID_3DVIEW_PERSPECTIVE_PROJECTION,ID_3DVIEW_ORTHOGONAL_PROJECTION,MF_BYCOMMAND);
	Invalidate(FALSE);
	
}

void CUniversal5CMillingMachinePostprocessorView::On3dviewPerspectiveProjection()
{
	oConfiguration.m_bPerspectiveProjection = true;
	m_o3DViewManipulator.UpdateProjectionMatrix();
	GetParent()->GetMenu()->CheckMenuRadioItem(ID_3DVIEW_ORTHOGONAL_PROJECTION,ID_3DVIEW_PERSPECTIVE_PROJECTION,ID_3DVIEW_PERSPECTIVE_PROJECTION,MF_BYCOMMAND);
	Invalidate(FALSE);

}

void CUniversal5CMillingMachinePostprocessorView::On3dviewOptimalSize()
{
	CUniversal5CMillingMachinePostprocessorDoc* pDoc = GetDocument();
	m_o3DViewManipulator.SetOptimalScale(pDoc->m_oChain.GetBoundingBox().GetMaxSize());
	Invalidate(FALSE);

}
