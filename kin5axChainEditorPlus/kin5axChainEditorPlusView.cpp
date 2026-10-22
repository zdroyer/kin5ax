// kin5axChainEditorPlusView.cpp : implementation of the CKin5axChainEditorPlusView class
//

#include "stdafx.h"
#include "ArgsMyException.h"

#include "kin5axChainEditorPlus.h"

#include "kin5axChainEditorPlusDoc.h"
#include "kin5axChainEditorPlusView.h"
#include ".\kin5axChainEditorPlusView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CKin5axChainEditorPlusView

extern CApplicationConfiguration oConfiguration;

IMPLEMENT_DYNCREATE(CKin5axChainEditorPlusView, CView)

BEGIN_MESSAGE_MAP(CKin5axChainEditorPlusView, CView)
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

// CKin5axChainEditorPlusView construction/destruction

CKin5axChainEditorPlusView::CKin5axChainEditorPlusView()
{
	// TODO: add construction code here

}

CKin5axChainEditorPlusView::~CKin5axChainEditorPlusView()
{
}

BOOL CKin5axChainEditorPlusView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CKin5axChainEditorPlusView drawing

void CKin5axChainEditorPlusView::OnDraw(CDC* pDC)
{
	CKin5axChainEditorPlusDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	CR3Vector r3vMaterialObserver = m_o3DViewManipulator.GetObserverVector(pDoc->m_oChain.GetActualToMaterialInvertedTransformation());
	double dPixelSize = m_o3DViewManipulator.GetPixelSize(0.0);
	glMatrixMode(GL_MODELVIEW);
	pDoc->Render3D(m_o3DViewManipulator.IsInAction(),r3vMaterialObserver,dPixelSize);

	//the circle and the cross of the manipulator go into the same buffer as the scene, before it
	//is swapped, so that they stay on the screen for as long as the mouse is held down
	m_o3DViewManipulator.DrawIndicators();
	SwapBuffers(*pDC);
}


// CKin5axChainEditorPlusView diagnostics

#ifdef _DEBUG
void CKin5axChainEditorPlusView::AssertValid() const
{
	CView::AssertValid();
}

void CKin5axChainEditorPlusView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CKin5axChainEditorPlusDoc* CKin5axChainEditorPlusView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CKin5axChainEditorPlusDoc)));
	return (CKin5axChainEditorPlusDoc*)m_pDocument;
}
#endif //_DEBUG


// CKin5axChainEditorPlusView message handlers

int CKin5axChainEditorPlusView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	try{
		CreateGLRenderingContext();
		InitGL();
		return 0;
	} catch (CGeneratorException &e){
		this->GetParent()->MessageBox(e.Message().c_str());
	}
	return -1;
}

void CKin5axChainEditorPlusView::OnMouseMove(UINT nFlags, CPoint point)
{
	if(m_o3DViewManipulator.OnMouseMove(nFlags,point))
		Invalidate(FALSE);
	//CView::OnMouseMove(nFlags, point);
}

void CKin5axChainEditorPlusView::OnLButtonDown(UINT nFlags, CPoint point)
{
	CView::OnLButtonDown(nFlags, point);
	m_o3DViewManipulator.OnLButtonDown(nFlags,point);
	Invalidate(FALSE);
}

void CKin5axChainEditorPlusView::OnLButtonUp(UINT nFlags, CPoint point)
{
	CView::OnLButtonUp(nFlags, point);
	m_o3DViewManipulator.OnLButtonUp(nFlags,point);
	Invalidate(FALSE);
}

void CKin5axChainEditorPlusView::OnRButtonDown(UINT nFlags, CPoint point)
{
	CView::OnRButtonDown(nFlags, point);
	m_o3DViewManipulator.OnRButtonDown(nFlags,point);
	Invalidate(FALSE);

}


void CKin5axChainEditorPlusView::OnRButtonUp(UINT nFlags, CPoint point)
{
	CView::OnRButtonUp(nFlags, point);
	m_o3DViewManipulator.OnRButtonUp(nFlags,point);
	Invalidate(FALSE);
}

BOOL CKin5axChainEditorPlusView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default
	return TRUE;
	//return CView::OnEraseBkgnd(pDC);
}

void CKin5axChainEditorPlusView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);
	m_o3DViewManipulator.OnSize(nType,cx,cy);

}

void CKin5axChainEditorPlusView::OnDestroy()
{
	CView::OnDestroy();
	wglDeleteContext(m_hGLrc);	
}
void CKin5axChainEditorPlusView::CreateGLRenderingContext(void)
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


void CKin5axChainEditorPlusView::InitGL(void)
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

void CKin5axChainEditorPlusView::On3dviewOriginalOrienation()
{
	m_o3DViewManipulator.ResetOrientation();
	Invalidate(FALSE);
}

void CKin5axChainEditorPlusView::On3dviewOriginalView()
{
	m_o3DViewManipulator.ResetView();
	Invalidate(FALSE);
}

void CKin5axChainEditorPlusView::On3dviewOriginalPosition()
{
	m_o3DViewManipulator.ResetPosition();
	Invalidate(FALSE);
}

void CKin5axChainEditorPlusView::On3dviewOriginalScale()
{
	m_o3DViewManipulator.ResetScale();
	m_o3DViewManipulator.ResetPosition();
	Invalidate(FALSE);
}

void CKin5axChainEditorPlusView::On3dviewZoomOut()
{
	m_o3DViewManipulator.ZoomOut();
	Invalidate(FALSE);
}

void CKin5axChainEditorPlusView::On3dviewZoomIn()
{
	m_o3DViewManipulator.ZoomIn();
	Invalidate(FALSE);
}

void CKin5axChainEditorPlusView::On3dviewOrthogonalProjection()
{
	oConfiguration.m_bPerspectiveProjection = false;
	m_o3DViewManipulator.UpdateProjectionMatrix();
	GetParent()->GetMenu()->CheckMenuRadioItem(ID_3DVIEW_ORTHOGONAL_PROJECTION,ID_3DVIEW_PERSPECTIVE_PROJECTION,ID_3DVIEW_ORTHOGONAL_PROJECTION,MF_BYCOMMAND);
	Invalidate(FALSE);
	
}

void CKin5axChainEditorPlusView::On3dviewPerspectiveProjection()
{
	oConfiguration.m_bPerspectiveProjection = true;
	m_o3DViewManipulator.UpdateProjectionMatrix();
	GetParent()->GetMenu()->CheckMenuRadioItem(ID_3DVIEW_ORTHOGONAL_PROJECTION,ID_3DVIEW_PERSPECTIVE_PROJECTION,ID_3DVIEW_PERSPECTIVE_PROJECTION,MF_BYCOMMAND);
	Invalidate(FALSE);

}

void CKin5axChainEditorPlusView::On3dviewOptimalSize()
{
	CKin5axChainEditorPlusDoc* pDoc = GetDocument();
	m_o3DViewManipulator.SetOptimalScale(pDoc->m_oChain.GetBoundingBox().GetMaxSize());
	Invalidate(FALSE);

}
