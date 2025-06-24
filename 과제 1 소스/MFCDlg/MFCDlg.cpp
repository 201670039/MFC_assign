#include "pch.h"
#include "framework.h"
#include "MFC.h"
#include "MFCDlg.h"
#include "afxdialogex.h"
#include <random>
#include <cmath>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

std::atomic_bool CMFCDlg::stopRandomMoving(false);

BEGIN_MESSAGE_MAP(CMFCDlg, CDialogEx)
    ON_WM_PAINT()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
    ON_WM_DESTROY()
    ON_BN_CLICKED(IDC_BTN_RESET, &CMFCDlg::OnBnClickedReset)
    ON_BN_CLICKED(IDC_BTN_RANDOM, &CMFCDlg::OnBnClickedRandom)
END_MESSAGE_MAP()

CMFCDlg::CMFCDlg(CWnd* pParent)
    : CDialogEx(IDD_MFCDLG_DIALOG, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    GdiplusStartupInput input;
    GdiplusStartup(&m_gdiToken, &input, nullptr);
}

void CMFCDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BOOL CMFCDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    SetIcon(m_hIcon, TRUE);
    SetIcon(m_hIcon, FALSE);
    return TRUE;
}

void CMFCDlg::OnDestroy()
{
    GdiplusShutdown(m_gdiToken);
    CDialogEx::OnDestroy();
}

void CMFCDlg::OnPaint()
{
    CPaintDC dc(this);
    Graphics g(dc);
    g.SetSmoothingMode(SmoothingModeAntiAlias);

    Gdiplus::FontFamily fontFamily(L"Arial");
    Gdiplus::Font font(&fontFamily, 10, FontStyleRegular, UnitPixel);

    SolidBrush textBrush(Color(255, 0, 0, 0));
    SolidBrush fillBrush(Color(255, 0, 0, 0));
    Pen pointPen(Color(255, 0, 0, 0), 1);

    for (const auto& pt : clickPoints)
    {
        PointF center((REAL)pt.x, (REAL)pt.y);
        DrawApproxCircle(g, center, pointRadius, &pointPen, &fillBrush);

        CString msg;
        msg.Format(_T("(%d, %d)"), pt.x, pt.y);
        g.DrawString(msg, -1, &font, PointF(center.X + 6, center.Y - 12), &textBrush);
    }

    if (clickPoints.size() == 3 && circleRadius > 0.0f)
    {
        PointF center;
        float r;
        if (ComputeCircleCenter(center, r))
        {
            Pen circlePen(Color(255, 0, 0, 0), circleThickness);
            DrawApproxCircle(g, center, circleRadius, &circlePen, nullptr);
        }
    }
}

void CMFCDlg::DrawApproxCircle(Graphics& g, PointF center, float r, Pen* pen, Brush* fill)
{
    const int N = 100;
    PointF pts[N + 1];
    for (int i = 0; i <= N; ++i)
    {
        float angle = static_cast<float>(i) / N * 2.0f * 3.1415926f;
        pts[i] = PointF(center.X + r * cosf(angle), center.Y + r * sinf(angle));
    }

    if (fill) g.FillPolygon(fill, pts, N + 1);
    if (pen) g.DrawLines(pen, pts, N + 1);
}

bool CMFCDlg::ComputeCircleCenter(PointF& center, float& r)
{
    if (clickPoints.size() != 3) return false;

    CPoint A = clickPoints[0], B = clickPoints[1], C = clickPoints[2];
    float a = B.x - A.x, b = B.y - A.y;
    float c = C.x - A.x, d = C.y - A.y;
    float e = a * (A.x + B.x) + b * (A.y + B.y);
    float f = c * (A.x + C.x) + d * (A.y + C.y);
    float g = 2 * (a * (C.y - B.y) - b * (C.x - B.x));

    if (fabs(g) < 1e-5) return false;

    center.X = (d * e - b * f) / g;
    center.Y = (a * f - c * e) / g;
    r = sqrtf((A.x - center.X) * (A.x - center.X) + (A.y - center.Y) * (A.y - center.Y));
    return true;
}
void CMFCDlg::OnLButtonDown(UINT, CPoint pt)
{
    for (int i = 0; i < clickPoints.size(); ++i)
    {
        int dx = pt.x - clickPoints[i].x;
        int dy = pt.y - clickPoints[i].y;
        if (dx * dx + dy * dy <= pointRadius * pointRadius)
        {
            isDragging = true;
            draggingIndex = i;
            SetCapture();
            return;
        }
    }

    if (clickPoints.size() < 3)
    {
        clickPoints.push_back(pt);

        if (clickPoints.size() == 3)
        {
            PointF center;
            float r;
            if (ComputeCircleCenter(center, r))
                circleRadius = r;
        }

        Invalidate(); // 배경 지우고 깨끗하게 다시 그림
    }
}




void CMFCDlg::OnLButtonUp(UINT, CPoint)
{
    isDragging = false;
    draggingIndex = -1;
    ReleaseCapture();
}


void CMFCDlg::OnMouseMove(UINT, CPoint pt)
{
    if (isDragging && draggingIndex >= 0)
    {
        // 점을 이동시킴
        clickPoints[draggingIndex] = pt;

        // 중심 및 반지름 재계산
        PointF center;
        float r;
        if (ComputeCircleCenter(center, r))
        {
            circleRadius = r;
        }

        Invalidate(); // 이전 원 제거 후 다시 그림
    }
}





void CMFCDlg::OnBnClickedReset()
{
    stopRandomMoving = true;         // 랜덤 이동 중단
    isRandomMoving = false;          // 상태 초기화

    clickPoints.clear();             // 클릭 지점 초기화
    isDragging = false;
    draggingIndex = -1;

    circleRadius = 0.0f;             // 원 반지름 초기화

    Invalidate();                    // 화면 전체 다시 그리기 (배경까지 지움)
    UpdateWindow();                  // 바로 갱신
}

void CMFCDlg::StartRandomMoveThread()
{
    isRandomMoving = true;
    stopRandomMoving = false;

    randThread = std::thread([this]()
        {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dx(50, 500);
            std::uniform_int_distribution<> dy(50, 400);

            for (int i = 0; i < 10 && !stopRandomMoving; ++i)
            {
                for (int j = 0; j < 3; ++j)
                    clickPoints[j] = CPoint(dx(gen), dy(gen));

                PointF center;
                float r;
                if (ComputeCircleCenter(center, r))
                    circleRadius = r;

                Invalidate(FALSE);
                UpdateWindow();
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }

            isRandomMoving = false;
        });

    randThread.detach();
}

void CMFCDlg::OnBnClickedRandom()
{
    if (clickPoints.size() == 3 && !isRandomMoving)
        StartRandomMoveThread();
}
