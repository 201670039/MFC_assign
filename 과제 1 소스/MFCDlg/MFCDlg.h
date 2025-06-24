#pragma once

#include <gdiplus.h>
#include <vector>
#include <thread>
#include <atomic>

using namespace Gdiplus;

class CMFCDlg : public CDialogEx
{
public:
    CMFCDlg(CWnd* pParent = nullptr);

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_MFCDLG_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    HICON m_hIcon;

    GdiplusStartupInput m_gdiplusStartupInput;
    ULONG_PTR m_gdiToken;

    std::vector<CPoint> clickPoints;
    bool isDragging = false;
    int draggingIndex = -1;

    float pointRadius = 10.0f;
    float circleRadius = 0.0f;
    float circleThickness = 3.0f;

    bool isRandomMoving = false;
    std::thread randThread;
    static std::atomic_bool stopRandomMoving;

    void DrawApproxCircle(Graphics& g, PointF center, float r, Pen* pen, Brush* fill = nullptr);
    bool ComputeCircleCenter(PointF& center, float& r);
    void StartRandomMoveThread();

protected:
    virtual BOOL OnInitDialog();
    afx_msg void OnPaint();
    afx_msg void OnDestroy();
    afx_msg void OnLButtonDown(UINT, CPoint pt);
    afx_msg void OnLButtonUp(UINT, CPoint);
    afx_msg void OnMouseMove(UINT, CPoint);
    afx_msg void OnBnClickedReset();
    afx_msg void OnBnClickedRandom();
    DECLARE_MESSAGE_MAP()
};
