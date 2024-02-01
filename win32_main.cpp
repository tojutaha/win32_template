#include <windows.h>
#include <assert.h>
#include <GL/GL.h>

static HDC GlobalOpenGLDC;
static HGLRC GlobalOpenGLRC;
static bool GlobalRunning;

static void
Win32SetPixelFormat()
{
    int SuggestedPixelFormatIndex = 0;
    PIXELFORMATDESCRIPTOR DesiredPixelFormat = {};
    DesiredPixelFormat.nSize = sizeof(DesiredPixelFormat);
    DesiredPixelFormat.nVersion = 1;
    DesiredPixelFormat.iPixelType = PFD_TYPE_RGBA;
    DesiredPixelFormat.dwFlags = PFD_SUPPORT_OPENGL|PFD_DRAW_TO_WINDOW|PFD_DOUBLEBUFFER;
    DesiredPixelFormat.cColorBits = 32;
    DesiredPixelFormat.cAlphaBits = 8;
    DesiredPixelFormat.cDepthBits = 24;
    DesiredPixelFormat.iLayerType = PFD_MAIN_PLANE;

    SuggestedPixelFormatIndex = ChoosePixelFormat(GlobalOpenGLDC, &DesiredPixelFormat);

    PIXELFORMATDESCRIPTOR SuggestedPixelFormat;
        
    DescribePixelFormat(GlobalOpenGLDC,
                        SuggestedPixelFormatIndex,
                        sizeof(SuggestedPixelFormat), 
                        &SuggestedPixelFormat);

    SetPixelFormat(GlobalOpenGLDC,
                   SuggestedPixelFormatIndex,
                   &SuggestedPixelFormat);
}

static void
Win32InitOpenGL()
{
    Win32SetPixelFormat();

    GlobalOpenGLRC = wglCreateContext(GlobalOpenGLDC);
    wglMakeCurrent(GlobalOpenGLDC, GlobalOpenGLRC);

    GLuint Error = glGetError();
    if(Error != GL_NO_ERROR)
    {
        assert(!"wglMakeCurrent failed");
    }
}

static void
DrawTriangle()
{
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glBegin(GL_TRIANGLES);

    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex2f(-0.5f, -0.5f);

    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex2f(0.5f, -0.5f);

    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex2f(0.0f, 0.5f);

    glEnd();

    SwapBuffers(GlobalOpenGLDC);
}


static void
Win32ProcessPendingMessages()
{
    MSG msg = {};
    while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
    {
        switch(msg.message)
        {
            case WM_QUIT:
            {
                GlobalRunning = false;
            } break;
            default:
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            } break;
        }
    }
}

LRESULT CALLBACK
WindowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_CLOSE:
        case WM_QUIT:
        case WM_DESTROY:
        {
            GlobalRunning = false;
        } break;

        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC hdc = BeginPaint(hwnd, &Paint);
            EndPaint(hwnd, &Paint);
        } break;

        default:
        {
            return DefWindowProc(hwnd, msg, wParam, lParam);
        } break;
    }
    
    return 0;
}

int WINAPI
WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow)
{
    WNDCLASS WindowClass = {};
    WindowClass.lpfnWndProc = WindowProcedure;
    WindowClass.hInstance = hInst;
    WindowClass.lpszClassName = "Win32 Template";

    if(RegisterClass(&WindowClass))
    {
        HWND Window = CreateWindowExA(0, WindowClass.lpszClassName, "Win32 Template",
                                      WS_OVERLAPPEDWINDOW,
                                      CW_USEDEFAULT, CW_USEDEFAULT,
                                      CW_USEDEFAULT, CW_USEDEFAULT,
                                      NULL, NULL, hInst, NULL);
        if(Window)
        {
            GlobalOpenGLDC = GetDC(Window);
            Win32InitOpenGL();

            ShowWindow(Window, nCmdShow);

            GlobalRunning = true;
            while(GlobalRunning)
            {
                Win32ProcessPendingMessages();
                DrawTriangle();

                RECT Rect;
                GetClientRect(Window, &Rect);
                int Width = Rect.right - Rect.left;
                int Height = Rect.bottom - Rect.top;
                glViewport(0, 0, Width, Height);
            }

            wglMakeCurrent(NULL, NULL);
            wglDeleteContext(GlobalOpenGLRC);
            ReleaseDC(Window, GlobalOpenGLDC);
        }
    }

    return 0;
}
