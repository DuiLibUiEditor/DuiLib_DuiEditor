// UIWindowEx.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "DuiEditor.h"
#include "UIWindowEx.h"

#include "MainFrm.h"
#include "DuiEditorViewDesign.h"
#include "DlgInsertControl.h"
#include "DlgCustomControl.h"

#include "wm.c"
// CUIWindowEx

IMPLEMENT_DYNAMIC(CUIWindowEx, CUIWindow)

CUIWindowEx::CUIWindowEx()
{

}

CUIWindowEx::~CUIWindowEx()
{
}


BEGIN_MESSAGE_MAP(CUIWindowEx, CUIWindow)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_CONTEXTMENU()
	ON_WM_DESTROY()
	ON_WM_SETCURSOR()
	ON_WM_MOUSEWHEEL()
	ON_WM_SIZE()
END_MESSAGE_MAP()



// CUIWindowEx ��Ϣ�������


BOOL CUIWindowEx::SelectItem(xml_node node)
{
	CControlUI *pControlUI = (CControlUI *)node.get_tag();
	return SelectItem(pControlUI);
}

BOOL CUIWindowEx::SelectItem(CControlUI *pControl)
{
	if(pControl)
	{
		m_tracker.RemoveAll();
		m_tracker.Add(xml_node((xml_node_struct *)pControl->GetTag()), pControl->GetPos());
		Invalidate();
		return TRUE;
	}

	m_tracker.RemoveAll();
	//m_tracker.Add(xml_node((xml_node_struct *)m_pManager->GetUiFrom()->GetTag()), m_pManager->GetUiFrom()->GetPos());
	Invalidate();
	return FALSE;
}

void CUIWindowEx::AddNewControlFromToolBox(xml_node nodeToolBox, CPoint point)
{
	//��ǰ�ڵ㣬�ӿؼ�����ȡ
	xml_node nodeCurrent = m_pManager->GetTreeView()->GetSelectXmlNode();					//��ѡ�еĿؼ�

	CRect rcClient;
	GetClientRect(rcClient);
	if(!rcClient.PtInRect(point))
	{
		return;
	}

	CControlUI *pNewControl = NULL;
	xml_node nodeContainer;

	CString strClass = nodeToolBox.name();

	//��ǰ�ڵ�������
	if(g_duiProp.IsBaseFromContainer(nodeCurrent.name()))
	{
		nodeContainer = nodeCurrent;
	}
	else
	{
		//��ǰ�ڵ㲻������, ����Ѱ��
		nodeContainer = m_pManager->FindContainer(nodeCurrent); //pControl;
	}

	CContainerUI *pParent = (CContainerUI *)nodeContainer.get_tag();
	CControlUI   *pCurControl = (CControlUI *)nodeCurrent.get_tag();

	//�ҵ�FORM������, �����������Ŀؼ���������, ����
	if(m_pManager->GetUiFrom() == pParent)
	{
		if(!g_duiProp.IsBaseFromContainer(strClass))
		{
			AfxMessageBox(_T("�ؼ���Ҫ������������."));
			return;
		}
	}


	CUITracker tracker;
	CRect rc;
	if(tracker.TrackRubberBand(this, point, TRUE))
	{
		rc.left = point.x;
		rc.top = point.y;
		rc.right = rc.left + tracker.m_rect.Width();
		rc.bottom = rc.top + tracker.m_rect.Height();
	}

	pNewControl = CUIBuilder::CreateControl(strClass);
	BOOL bCustomControl = FALSE;
	CString strCuscomControlParent;
	if(!pNewControl) 
	{
		//����������ɹ�, ��Ϊ���Զ���ؼ�.
		CDlgCustomControl dlg;
		if(dlg.DoModal() != IDOK)
			return;

		pNewControl = CUIBuilder::CreateControl(dlg.m_strParentControl);
		if(!pNewControl)	return;

		strClass = dlg.m_strClassName;
		strCuscomControlParent = dlg.m_strParentControl;
		bCustomControl = TRUE;
	}

	CDlgInsertControl dlg;
	dlg.nodeParent = nodeContainer;
	dlg.nodeControl = nodeCurrent;
	if(dlg.DoModal() != IDOK)	return;

	xml_node nodeNewControl;
	switch (dlg.m_nPosition)
	{
	case 0: //��ǰ�����·�����ؼ�
		{
			if(!pParent->Add(pNewControl)) { delete pNewControl; return; }		
			nodeNewControl = nodeContainer.append_child(strClass);	//д���ĵ�
		}
		break;
	case 1:	//��ǰ�ؼ��·������ֵܿؼ�
		{
			if(! pParent->AddAt(pNewControl, pParent->GetItemIndex(pCurControl)) ) { delete pNewControl; return; }
			nodeNewControl = nodeContainer.append_child(strClass);	//д���ĵ�
		}
		break;
	case 2:	//��ǰ�ؼ��Ϸ������ֵܿؼ�
		{
			int nIndex = pParent->GetItemIndex(pCurControl);
			if(nIndex == 0) //��һ��
			{
				if(! pParent->AddAt(pNewControl, nIndex) ) { delete pNewControl; return; }
				pParent->SetItemIndex(pNewControl, 0);
			}
			else
			{
				if(! pParent->AddAt(pNewControl, nIndex-1) ) { delete pNewControl; return; }
			}
			nodeNewControl = nodeContainer.insert_child_before(strClass, nodeCurrent);	//д���ĵ�
		}
		break;
	}

	if(bCustomControl)
	{
		nodeNewControl.attribute_auto(_T("custombasedfrom")).set_value(strCuscomControlParent);
	}
	
	//�����ĵ��Ϳؼ���˫��ָ��
	pNewControl->SetTag((UINT_PTR)nodeNewControl.internal_object());
	nodeNewControl.set_tag((UINT_PTR)pNewControl);

	if(m_pManager->GetView()->m_nFormatInsert == 0)	//�Զ���λ
	{
		RECT rcNew;
		rcNew.left = 0;
		rcNew.top = 0;
		rcNew.right = rc.right - rc.left;
		rcNew.bottom = rc.bottom - rc.top;

		pNewControl->SetAttribute(_T("pos"), RectToString(&rcNew));

		g_duiProp.AddAttribute(nodeNewControl, _T("pos"), RectToString(&rcNew),			NULL);
		g_duiProp.AddAttribute(nodeNewControl, _T("width"), rcNew.right-rcNew.left,		NULL);
		g_duiProp.AddAttribute(nodeNewControl, _T("height"), rcNew.bottom-rcNew.top,	NULL);
	}
	else if(m_pManager->GetView()->m_nFormatInsert == 1)	//���Զ�λ
	{
		CRect rcParent = pParent->GetPos();
		CRect rcNew;
		rcNew.left = rc.left - rcParent.left;
		rcNew.right = rcNew.left + rc.right - rc.left;
		rcNew.top = rc.top - rcParent.top;
		rcNew.bottom = rcNew.top + rc.bottom - rc.top;

		pNewControl->SetAttribute(_T("pos"), RectToString(&rcNew));

		g_duiProp.AddAttribute(nodeNewControl, _T("float"), _T("true"),					NULL);
		g_duiProp.AddAttribute(nodeNewControl, _T("pos"), RectToString(&rcNew),			NULL);
		g_duiProp.AddAttribute(nodeNewControl, _T("width"), rcNew.right-rcNew.left,		NULL);
		g_duiProp.AddAttribute(nodeNewControl, _T("height"), rcNew.bottom-rcNew.top,	NULL);
	}
	else if(m_pManager->GetView()->m_nFormatInsert == 2)	//��Զ�λ
	{
		CRect rcParent = pNewControl->GetParent()->GetPos();
		CRect rcNew;
		rcNew.left = rc.left - rcParent.left;
		rcNew.right = rcNew.left + rc.right - rc.left;
		rcNew.top = rc.top - rcParent.top;
		rcNew.bottom = rcNew.top + rc.bottom - rc.top;

		pNewControl->SetAttribute(_T("pos"), RectToString(&rcNew));

		g_duiProp.AddAttribute(nodeNewControl, _T("float"),			_T("true"),				NULL);
		g_duiProp.AddAttribute(nodeNewControl, _T("relativepos"),	_T("50,50,0,0"),		NULL);
		g_duiProp.AddAttribute(nodeNewControl, _T("pos"),			RectToString(&rcNew),	NULL);
		g_duiProp.AddAttribute(nodeNewControl, _T("width"),			rcNew.right-rcNew.left, NULL);
		g_duiProp.AddAttribute(nodeNewControl, _T("height"),		rcNew.bottom-rcNew.top, NULL);
	}

	//����ؼ�Ĭ������
	LPCTSTR pDefaultAttributes = GetManager()->GetDefaultAttributeList(strClass);
	if( pDefaultAttributes ) 
	{
		pNewControl->ApplyAttributeList(pDefaultAttributes);
	}

	//����ؼ���ǰ����

	//����ؼ���
	if(dlg.m_nPosition == 2)
		m_pManager->GetTreeView()->AddNewControl(nodeNewControl, nodeCurrent, TVI_BEFORE);
	else
		m_pManager->GetTreeView()->AddNewControl(nodeNewControl, nodeCurrent, TVI_NEXT);

	//ˢ�����Դ���
	g_pPropWnd->InitProp(nodeNewControl);

	//ˢ�¿ؼ�
	m_pManager->UpdateControlUI(nodeNewControl);

	//ֻѡ������ӵĿؼ�
	m_tracker.RemoveAll();
	m_tracker.Add(nodeNewControl, pNewControl->GetPos());

	//����command history
	m_pManager->GetCmdHistory()->AddNewControl(nodeNewControl);

	m_pManager->GetDocument()->SetModifiedFlag(TRUE);
}

void CUIWindowEx::OnSelectingControl(CControlUI *pControl, const CRect &rcTracker)
{
	xml_node nodeControl((xml_node_struct *)pControl->GetTag());
	if(!nodeControl)
		return;
	for (xml_node node=nodeControl.first_child(); node; node=node.next_sibling())
	{
		CControlUI *p = (CControlUI *)node.get_tag();
		if(!p)	return;
		CRect rc = p->GetPos();
		CRect rc1;
		if(rc1.IntersectRect(rc, &rcTracker))
		{
			m_listTrackerSeleted.push_back(p);
		}

		OnSelectingControl(p, rcTracker);
	}
}


int CUIWindowEx::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CUIWindow::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_tracker.m_nHandleSize = 6;
	m_tracker.m_nStyle = CUITracker::dottedLine|CUITracker::resizeInside;
	m_tracker.m_pManager = m_pManager;

	//����һ�����ع��, �˹�������ڿؼ���ֱ����������
	//::CreateCaret(m_hWnd,(HBITMAP) NULL, 0, 0);

	//////////////////////////////////////////////////////////////////////////

	CDuiEditorDoc *pDoc = m_pManager->GetDocument();

	m_PaintManager.Init(m_hWnd);
//	m_PaintManager.AddPreMessageFilter(this);
	m_PaintManager.SetResourcePath(pDoc->m_strSkinDir);

	CUIBuilder builder;

	CControlUI* pRoot=NULL;
	pRoot = builder.Create(pDoc->m_doc, NULL, &m_PaintManager);
	if (pRoot==NULL)
	{
		MessageBox(_T("Loading resource files error."),_T("Duilib"),MB_OK|MB_ICONERROR);
		return 0;
	}

	//���ڸ�����һ��containner, ������ڲ��������ɴ��ڶ���
	CUIFormView *p = new CUIFormView;
	xml_node nodeWindow = pDoc->m_doc.child(_T("Window"));
	p->SetTag((UINT_PTR)nodeWindow.internal_object());
	nodeWindow.set_tag((UINT_PTR)p);
	p->Add(pRoot);

	m_PaintManager.AttachDialog(p);

//	m_PaintManager.AttachDialog(pRoot);
//	m_PaintManager.AddNotifier(this);
	return 0;
}


void CUIWindowEx::OnPaint()
{
	CDC *pDC = GetDC();

	m_tracker.Draw(pDC, NULL);

	CRect rectClient;
	GetClientRect(rectClient);
	if(m_pManager->GetView()->m_bViewGrid)
	{
		for(int i=rectClient.left; i<rectClient.right; i+=10)
		{
			for(int j=rectClient.top; j<rectClient.bottom; j+=10)
				pDC->SetPixel(i, j, RGB(0,0,0));
		}
	}


	if(m_pManager->GetView()->m_bViewMouse)
	{
		CPoint point;
		GetCursorPos(&point);
		ScreenToClient(&point);
		if(rectClient.PtInRect(point))
		{
			CPen pen(PS_DOT, 1, RGB(0,0,0));
			CPen *pOldPen = pDC->SelectObject(&pen);

			pDC->MoveTo(0, point.y);
			pDC->LineTo(999, point.y);
			pDC->MoveTo(point.x, 0);
			pDC->LineTo(point.x, 999);

			CRect rcMouseText;
			rcMouseText.left	= point.x;
			rcMouseText.right	= point.x + 100;
			rcMouseText.top		= point.y - 30;
			rcMouseText.bottom	= point.y;

			pDC->SetBkMode(TRANSPARENT);
			CString strText;
			CPoint pt2 = point;
			strText.Format(_T("( %d, %d )"), pt2.x, pt2.y);
			pDC->DrawText(strText, rcMouseText, DT_CENTER|DT_VCENTER|DT_SINGLELINE);

			pDC->SelectObject(pOldPen);
		}
	}

	ReleaseDC(pDC);
}

LRESULT CUIWindowEx::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRet = 0;

	if(m_pManager->GetView()->m_bShowUiPreview)
		m_PaintManager.MessageHandler(message, wParam, lParam, lRet);
	else
	{
		if(message == WM_PAINT)
		{
			m_PaintManager.MessageHandler(message, wParam, lParam, lRet);
		}
	}

	if(!(message >= WM_AFXFIRST && message <= WM_AFXLAST)	&&
		message != WM_TIMER		&&
		message != WM_SETCURSOR)
	{
		CString temp;
		temp.Format(_T("message=%s, uint=%X, wParam=%d, lParam=%d"), GetMessageText(message), message, wParam, lParam);
		//InsertMsg(temp);
	}

	return CWnd::WindowProc(message, wParam, lParam);
	//return __super::WindowProc(message, wParam, lParam);
}

void CUIWindowEx::OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/)
{
	// TODO: �ڴ˴������Ϣ����������
}

void CUIWindowEx::OnDestroy()
{
	__super::OnDestroy();

	//::DestroyCaret(); //ɾ�����ع��
}

void CUIWindowEx::OnLButtonDown(UINT nFlags, CPoint pt)
{
	xml_node nodeToolBox((xml_node_struct *)g_pToolBox->GetCurSelTag());
	if(nodeToolBox) //��ӿؼ�
	{ 
		m_tracker.RemoveAll();
		AddNewControlFromToolBox(nodeToolBox, pt);	
		g_pToolBox->SetCurSel(TREENODETYPE_POINTER);
		__super::OnLButtonDown(nFlags, pt);
		return;
	}	

	CControlUI* pControl = GetManager()->FindControl(pt);
	if(pControl==NULL) 
	{
		m_tracker.RemoveAll();
		__super::OnLButtonDown(nFlags, pt);
		return;
	}

	xml_node nodeClick;
	//DWORD dt = GetTickCount();
	while (TRUE)
	{
		if(!pControl)	break;
		//if(GetTickCount() - dt > 1000) break; //�����ؼ��г�ʱʱ��

		nodeClick = xml_node((xml_node_struct *)pControl->GetTag());
		if(nodeClick)	break;
		else
		{
			pControl = pControl->GetParent();
		}
	}

	if(!nodeClick)
	{
		m_tracker.RemoveAll();
		__super::OnLButtonDown(nFlags, pt);
		return;
	}

	pControl = (CControlUI*)nodeClick.get_tag();
	if(pControl==NULL) 
	{
		m_tracker.RemoveAll();
		__super::OnLButtonDown(nFlags, pt);
		return;
	}

	int nHit = m_tracker.HitTest(pt);
	m_tracker.m_nHitTest = nHit;
	if((nFlags&MK_CONTROL)==0&& nHit==CUITracker::hitNothing)
		m_tracker.RemoveAll();
	if(nHit==CUITracker::hitNothing)
	{
		m_tracker.Add(nodeClick, pControl->GetPos());
		m_pManager->GetTreeView()->SelectXmlNode(nodeClick);
	}
	else
		m_tracker.SetFocus(nodeClick);

	//���ü���������
// 	CControlUI *pFocusControl = (CControlUI *)m_tracker.m_pFocused->m_node.get_tag();
// 	if(pFocusControl)
// 	{
// 		CRect rcFocusControl = pFocusControl->GetPos();
// 		::SetCaretPos(rcFocusControl.left, rcFocusControl.bottom);
// 	}

	if(nHit > 0 && (g_duiProp.IsBaseFromControlUI(nodeClick.name()) || g_duiProp.IsWindowForm(nodeClick.name())))
	{
		m_tracker.Track(this, pt, TRUE);
	}
	else
	{
		CUITracker tracker;
		tracker.TrackRubberBand(this, pt, TRUE);
		CRect rc = tracker.m_rect;
		rc.NormalizeRect();

		//���������ѡ�ؼ�, ������һ�����������
		if(g_duiProp.IsBaseFromContainer(nodeClick.name()))
		{
			m_listTrackerSeleted.clear();
			OnSelectingControl(pControl, rc);
			if(m_listTrackerSeleted.size() > 0)
			{
				m_tracker.RemoveAll();
				std::list<CControlUI *>::iterator it;
				for (it=m_listTrackerSeleted.begin(); it!=m_listTrackerSeleted.end(); it++)
				{
					CControlUI *p = (CControlUI *)(*it);
					xml_node n((xml_node_struct *)p->GetTag());
					m_tracker.Add(n, p->GetPos());
				}
			}
		}
	}

	Invalidate();
	__super::OnLButtonDown(nFlags, pt);
}


void CUIWindowEx::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

	__super::OnLButtonUp(nFlags, point);
}


void CUIWindowEx::OnRButtonDown(UINT nFlags, CPoint point)
{
	g_pToolBox->SetDefaultPoint();
	::SetCursor(LoadCursor(NULL, IDC_ARROW));

	__super::OnRButtonDown(nFlags, point);
}


void CUIWindowEx::OnRButtonUp(UINT nFlags, CPoint point)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

	__super::OnRButtonUp(nFlags, point);
}


void CUIWindowEx::OnMouseMove(UINT nFlags, CPoint point)
{
	if(m_pManager->GetView()->m_bViewMouse)
	{
		Invalidate();
	}
	__super::OnMouseMove(nFlags, point);
}


void CUIWindowEx::OnMouseLeave()
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

	__super::OnMouseLeave();
}



BOOL CUIWindowEx::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if(m_pManager->GetView()->m_bShowUiPreview)
		return FALSE;

	xml_node nodeToolBox((xml_node_struct *)g_pToolBox->GetCurSelTag());
	if(nodeToolBox)
	{
		::SetCursor(LoadCursor(NULL, IDC_CROSS));
		return TRUE;
	}

	if(m_tracker.SetCursor(pWnd, nHitTest, CPoint(0,0), CSize(0,0)))
		return TRUE;

	return __super::OnSetCursor(pWnd, nHitTest, message);
}


BOOL CUIWindowEx::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	Invalidate();
	return __super::OnMouseWheel(nFlags, zDelta, pt);
}



void CUIWindowEx::OnSize(UINT nType, int cx, int cy)
{
	CUIWindow::OnSize(nType, cx, cy);

	// TODO: �ڴ˴������Ϣ����������
	//InsertMsg(_T("WM_SIZE"));
}
