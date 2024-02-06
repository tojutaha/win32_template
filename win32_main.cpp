#include <windows.h>
#include <vector>
#include <assert.h>
#include <GL/GL.h>

#define WGL_CONTEXT_MAJOR_VERSION_ARB             0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB             0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB               0x2093
#define WGL_CONTEXT_FLAGS_ARB                     0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB              0x9126
#define WGL_CONTEXT_DEBUG_BIT_ARB                 0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB    0x0002
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002
#define WGL_DRAW_TO_WINDOW_ARB			          0x2001
#define WGL_ACCELERATION_ARB			          0x2003
#define WGL_SUPPORT_OPENGL_ARB			          0x2010
#define WGL_DOUBLE_BUFFER_ARB			          0x2011
#define WGL_PIXEL_TYPE_ARB			              0x2013
#define WGL_TYPE_RGBA_ARB			              0x202B
#define WGL_FULL_ACCELERATION_ARB		          0x2027
#define WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB          0x20A9

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))
#define DEBUG 1

#define WindowWidth 1024
#define WindowHeight 768
#define QuadSize 4
int NumQuadsX = WindowWidth / QuadSize;
int NumQuadsY = WindowHeight / QuadSize;
GLfloat QuadSizeX = 2.0f / NumQuadsX;
GLfloat QuadSizeY = 2.0f / NumQuadsY;

static HDC GlobalOpenGLDC;
static HGLRC GlobalOpenGLRC;
static GLuint GlobalShaderProgram;
static bool GlobalRunning;

typedef ptrdiff_t GLsizeiptr;
typedef ptrdiff_t GLintptr;
typedef int GLsizei;
typedef char GLchar;

typedef void WINAPI gl_gen_buffers (GLsizei n, GLuint *buffers);
typedef void WINAPI gl_delete_buffers (GLsizei n, const GLuint *buffers);
typedef void WINAPI gl_bind_buffer (GLenum target, GLuint buffer);
typedef void WINAPI gl_buffer_data (GLenum target, GLsizeiptr size, const void *data, GLenum usage);
typedef void WINAPI gl_attach_shader (GLuint program, GLuint shader);
typedef void WINAPI gl_compile_shader (GLuint shader);
typedef GLuint WINAPI gl_create_program (void);
typedef GLuint WINAPI gl_create_shader (GLenum type);
typedef void WINAPI gl_delete_program (GLuint program);
typedef void WINAPI gl_delete_shader (GLuint shader);
typedef void WINAPI gl_link_program (GLuint program);
typedef void WINAPI gl_shader_source (GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
typedef void WINAPI gl_get_programiv (GLuint program, GLenum pname, GLint *params);
typedef void WINAPI gl_get_program_info_log (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void WINAPI gl_get_shaderiv (GLuint shader, GLenum pname, GLint *params);
typedef void WINAPI gl_get_shader_info_log (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void WINAPI gl_use_program (GLuint program);

typedef void WINAPI gl_disable_vertex_attrib_array (GLuint index);
typedef void WINAPI gl_enable_vertex_attrib_array (GLuint index);
typedef void WINAPI gl_vertex_attrib_pointer (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);

typedef void WINAPI gl_gen_vertex_arrays (GLsizei n, GLuint *arrays);
typedef void WINAPI gl_bind_vertex_array (GLuint array);
typedef void WINAPI gl_buffer_sub_data (GLenum target, GLintptr offset, GLsizeiptr size, const void *data);

typedef const char * WINAPI wgl_get_extensions_string_ext(void);
typedef BOOL WINAPI wgl_swap_interval_ext(int interval);
typedef HGLRC WINAPI wgl_create_context_attribs_arb(HDC hDC, HGLRC hShareContext, const int *attribList);
typedef BOOL WINAPI wgl_get_pixel_format_attrib_iv_arb(HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int *piAttributes, int *piValues);
typedef BOOL WINAPI wgl_get_pixel_format_attrib_fv(HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int *piAttributes, FLOAT *pfValues);
typedef BOOL WINAPI wgl_choose_pixel_format_arb(HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);

typedef GLint WINAPI gl_get_uniform_location (GLuint program, const GLchar *name);
typedef void WINAPI gl_uniform_matrix4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

static gl_gen_buffers *glGenBuffers;
static gl_delete_buffers *glDeleteBuffers;
static gl_bind_buffer *glBindBuffer;
static gl_buffer_data *glBufferData;
static gl_attach_shader *glAttachShader;
static gl_compile_shader *glCompileShader;
static gl_create_program *glCreateProgram;
static gl_create_shader *glCreateShader;
static gl_delete_program *glDeleteProgram;
static gl_delete_shader *glDeleteShader;
static gl_shader_source *glShaderSource;
static gl_link_program *glLinkProgram;
static gl_get_programiv *glGetProgramiv;
static gl_get_program_info_log *glGetProgramInfoLog;
static gl_get_shaderiv *glGetShaderiv;
static gl_get_shader_info_log *glGetShaderInfoLog;
static gl_use_program *glUseProgram;
static gl_disable_vertex_attrib_array *glDisableVertexAttribArray;
static gl_enable_vertex_attrib_array *glEnableVertexAttribArray;
static gl_vertex_attrib_pointer *glVertexAttribPointer;

static gl_gen_vertex_arrays *glGenVertexArrays;
static gl_bind_vertex_array *glBindVertexArray;
static gl_buffer_sub_data *glBufferSubData;

static wgl_get_extensions_string_ext *wglGetExtensionsStringEXT;
static wgl_swap_interval_ext *wglSwapIntervalEXT;
static wgl_create_context_attribs_arb *wglCreateContextAttribsARB;
static wgl_choose_pixel_format_arb *wglChoosePixelFormatARB;

static gl_get_uniform_location *glGetUniformLocation;
static gl_uniform_matrix4fv *glUniformMatrix4fv;

#define GL_DEBUG_CALLBACK(Name) void Name(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
typedef GL_DEBUG_CALLBACK(GLDEBUGPROC);
typedef void WINAPI gl_debug_message_callback_arb(GLDEBUGPROC callback, const void *userParam);
static gl_debug_message_callback_arb *glDebugMessageCallbackARB;

struct VertexData
{
    GLfloat x, y;
    GLfloat r, g, b;
};

struct Color
{
    GLfloat r, g, b;
};

std::vector<std::vector<int>> Board(NumQuadsX, std::vector<int>(NumQuadsY, 0));
std::vector<int> Indices;
std::vector<VertexData> Vertices;

#include "opengl.cpp"

static void
Win32SetPixelFormat()
{
    int SuggestedPixelFormatIndex = 0;
    GLuint ExtendedPick = 0;
    
    if(wglChoosePixelFormatARB)
    {                
        int IntAtrribList[] = {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
            WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
            WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
            WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE,
            0,
        };
        
        wglChoosePixelFormatARB(GlobalOpenGLDC, IntAtrribList, 0, 1, &SuggestedPixelFormatIndex, &ExtendedPick);
    }
    
    if(!ExtendedPick)
    {
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
    }
            
    SetPixelFormat(GlobalOpenGLDC, SuggestedPixelFormatIndex, 0);
}

static void
Win32LoadWGLExtensions()
{
	WNDCLASSA WindowClass = {0};
	WindowClass.lpfnWndProc = DefWindowProcA;
	WindowClass.hInstance = GetModuleHandle(0);
	WindowClass.lpszClassName = "DummyWGL";
	WindowClass.style = CS_OWNDC;
	if(RegisterClassA(&WindowClass))
	{
		HWND DummyWindow = CreateWindowExA(
				0,
				WindowClass.lpszClassName,
				"DummyWGLWindow",
				0,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				0,
				0,
				WindowClass.hInstance,
				0
				);

		if(DummyWindow)
		{
			HDC DummyDC = GetDC(DummyWindow);

			PIXELFORMATDESCRIPTOR DesiredPixelFormat = {0};
			DesiredPixelFormat.nSize = sizeof(DesiredPixelFormat);
			DesiredPixelFormat.nVersion = 1;
			DesiredPixelFormat.iPixelType = PFD_TYPE_RGBA;
			DesiredPixelFormat.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
			DesiredPixelFormat.cColorBits = 32;
			DesiredPixelFormat.cAlphaBits = 8;
			DesiredPixelFormat.iLayerType = PFD_MAIN_PLANE;

			int SuggestedPixelFormatIndex = ChoosePixelFormat(DummyDC, &DesiredPixelFormat);
			SetPixelFormat(DummyDC, SuggestedPixelFormatIndex, &DesiredPixelFormat);

			HGLRC DummyRC = wglCreateContext(DummyDC);
			if(DummyRC)
			{
				wglMakeCurrent(DummyDC, DummyRC);

				wglChoosePixelFormatARB = (wgl_choose_pixel_format_arb *)wglGetProcAddress("wglChoosePixelFormatARB");
                wglCreateContextAttribsARB = (wgl_create_context_attribs_arb *)wglGetProcAddress("wglCreateContextAttribsARB");
                wglGetExtensionsStringEXT = (wgl_get_extensions_string_ext *)wglGetProcAddress("wglGetExtensionsStringEXT");
                
				wglMakeCurrent(0, 0);
				wglDeleteContext(DummyRC);
			}
			ReleaseDC(DummyWindow, DummyDC);
			DestroyWindow(DummyWindow);
		}
		UnregisterClassA(WindowClass.lpszClassName, WindowClass.hInstance);
	}
}

static void
Win32InitOpenGL()
{
	Win32LoadWGLExtensions();
    Win32SetPixelFormat();
	GlobalOpenGLRC = wglCreateContext(GlobalOpenGLDC);
    wglMakeCurrent(GlobalOpenGLDC, GlobalOpenGLRC);

	glGenBuffers = (gl_gen_buffers *)wglGetProcAddress("glGenBuffers");
	glDeleteBuffers = (gl_delete_buffers *)wglGetProcAddress("glDeleteBuffers");
	glBindBuffer = (gl_bind_buffer *)wglGetProcAddress("glBindBuffer");
	glBufferData = (gl_buffer_data *)wglGetProcAddress("glBufferData");

	glAttachShader = (gl_attach_shader *)wglGetProcAddress("glAttachShader");
	glCompileShader = (gl_compile_shader *)wglGetProcAddress("glCompileShader");
	glCreateProgram = (gl_create_program *)wglGetProcAddress("glCreateProgram");
	glCreateShader = (gl_create_shader *)wglGetProcAddress("glCreateShader");
	glDeleteProgram = (gl_delete_program *)wglGetProcAddress("glDeleteProgram");
	glDeleteShader = (gl_delete_shader *)wglGetProcAddress("glDeleteShader");
	glShaderSource = (gl_shader_source *)wglGetProcAddress("glShaderSource");
	glLinkProgram = (gl_link_program *)wglGetProcAddress("glLinkProgram");

	glGetProgramiv = (gl_get_programiv *)wglGetProcAddress("glGetProgramiv");
	glGetProgramInfoLog = (gl_get_program_info_log *)wglGetProcAddress("glGetProgramInfoLog");
	glGetShaderiv = (gl_get_shaderiv *)wglGetProcAddress("glGetShaderiv");
	glGetShaderInfoLog = (gl_get_shader_info_log *)wglGetProcAddress("glGetShaderInfoLog");

	glUseProgram = (gl_use_program *)wglGetProcAddress("glUseProgram");
	glDisableVertexAttribArray = (gl_disable_vertex_attrib_array *)wglGetProcAddress("glDisableVertexAttribArray");
	glEnableVertexAttribArray = (gl_enable_vertex_attrib_array *)wglGetProcAddress("glEnableVertexAttribArray");
	glVertexAttribPointer = (gl_vertex_attrib_pointer *)wglGetProcAddress("glVertexAttribPointer");

	glGenVertexArrays = (gl_gen_vertex_arrays *)wglGetProcAddress("glGenVertexArrays");
	glBindVertexArray = (gl_bind_vertex_array *)wglGetProcAddress("glBindVertexArray");
	glBufferSubData = (gl_buffer_sub_data *)wglGetProcAddress("glBufferSubData");

	glGetUniformLocation = (gl_get_uniform_location *)wglGetProcAddress("glGetUniformLocation");
	glUniformMatrix4fv = (gl_uniform_matrix4fv *)wglGetProcAddress("glUniformMatrix4fv");

	glDebugMessageCallbackARB = (gl_debug_message_callback_arb *)wglGetProcAddress("glDebugMessageCallbackARB");

	if(wglCreateContextAttribsARB)
	{
		int Attribs[] = {
			WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
			WGL_CONTEXT_MINOR_VERSION_ARB, 3,
			WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB
#if DEBUG
            |WGL_CONTEXT_DEBUG_BIT_ARB,
#else
            ,
#endif
			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
			0,
		};

		wglMakeCurrent(NULL, NULL);

		HGLRC NewOpenGLRC = wglCreateContextAttribsARB(GlobalOpenGLDC, 0, Attribs);
		if(NewOpenGLRC)
		{
			wglDeleteContext(GlobalOpenGLRC);
			wglMakeCurrent(GlobalOpenGLDC, NewOpenGLRC);
			GlobalOpenGLRC = NewOpenGLRC;
		}
		else
		{
			assert(!"wglCreateContextAttribsARB failed.");
		}
	}
	else
	{
		GlobalOpenGLRC = wglCreateContext(GlobalOpenGLDC);
		if(GlobalOpenGLRC)
		{
			wglMakeCurrent(GlobalOpenGLDC, GlobalOpenGLRC);
		}
		else
		{
			assert(!"wglCreateContext failed.");
		}
	}

    glClearColor(0, 0, 0, 0);

    char *Header = R"HEADER(
	#version 330 core
    )HEADER";

	char *VertexCode = R"VERTEXCODE(
	layout(location = 0) in vec3 VertP;
	layout (location = 1) in vec3 VertColor;

	out vec3 ResultColor;

	uniform mat4 Projection;

	void main()
	{
	    gl_Position = Projection * vec4(VertP, 1.0);
		ResultColor = VertColor;
	}
	)VERTEXCODE";

	char *FragmentCode = R"FRAGMENTCODE(
	in vec3 ResultColor;

	out vec4 FragColor;

	void main()
	{
	    FragColor = vec4(ResultColor, 1.0);
	}
	)FRAGMENTCODE";

	GlobalShaderProgram = OpenGLCompileProgram(Header, VertexCode, FragmentCode);
	glUseProgram(GlobalShaderProgram);

#if DEBUG
	if(glDebugMessageCallbackARB)
	{
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallbackARB(OpenGLDebugCallback, 0);
	}

	GLuint Error = glGetError();
	if (Error != GL_NO_ERROR)
	{
		assert(!"Win32OpenGLInit failed");
	}
#endif

	float projectionMatrix[16];
	CreateOrthoProjection(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f, projectionMatrix);

	GLint projectionLocation = glGetUniformLocation(GlobalShaderProgram, "Projection");
	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, projectionMatrix);
	if(projectionLocation == -1)
	{
		assert(!"Uniform variable Projection was not found in the program");
	}

    for(int x = 0; x < NumQuadsX; ++x)
    {
        for(int y = 0; y <NumQuadsY; ++y)
        {
            GLuint QuadIndex = (x + y * NumQuadsX) * 6;
            Indices.push_back(QuadIndex + 0);
            Indices.push_back(QuadIndex + 1);
            Indices.push_back(QuadIndex + 2);
            Indices.push_back(QuadIndex + 3);
            Indices.push_back(QuadIndex + 4);
            Indices.push_back(QuadIndex + 5);
        }
    }
}

static void
RestartGame()
{
    for(int x = 0; x < NumQuadsX; ++x)
    {
        for(int y = 0; y < NumQuadsY; ++y)
        {
            Board[x][y] = rand() % 2;
        }
    }
}

static void
UpdateAndRender(GLuint VAO, GLuint EBO, GLuint VertexBuffer)
{
    // Update
    std::vector<std::vector<int>> NewBoard(NumQuadsX, std::vector<int>(NumQuadsY, 0));
    for(int i = 1; i < NumQuadsX - 1; ++i)
    {
        for(int j = 1; j < NumQuadsY - 1; ++j)
        {
            int liveNeighbours = 0;
            for(int x = -1; x <= 1; ++x)
            {
                for(int y = -1; y <= 1; ++y)
                {
                    liveNeighbours += Board[i + x][j + y];
                }
            }

            liveNeighbours -= Board[i][j];
            if((Board[i][j] == 1) && (liveNeighbours < 2))
            {
                NewBoard[i][j] = 0;
            }
            else if((Board[i][j] == 1) && (liveNeighbours > 3))
            {
                NewBoard[i][j] = 0;
            }
            else if((Board[i][j] == 0) && (liveNeighbours == 3))
            {
                NewBoard[i][j] = 1;
            }
            else
            {
                NewBoard[i][j] = Board[i][j];
            }
        }
    }

    Board = NewBoard;

    // Render
    Vertices.clear();
    glClear(GL_COLOR_BUFFER_BIT);

    for(int x = 0; x < NumQuadsX; ++x)
    {
        for(int y = 0; y < NumQuadsY; ++y)
        {
            GLfloat NormalizedX = -1.0f + x * QuadSizeX;
            GLfloat NormalizedY = -1.0f + y * QuadSizeY;

            Color color = {0.0f, 0.0f, 0.0f};
            if(Board[x][y] == 1)
            {
                color = {0.0f, 1.0f, 0.0f};
            }

            VertexData vertexData;

            //First triangle
            vertexData = {NormalizedX, NormalizedY, color.r, color.g, color.b};
            Vertices.push_back(vertexData);

            vertexData = {NormalizedX + QuadSizeX, NormalizedY, color.r, color.g, color.b};
            Vertices.push_back(vertexData);

            vertexData = {NormalizedX, NormalizedY + QuadSizeY, color.r, color.g, color.b};
            Vertices.push_back(vertexData);

            // Second triangle
            vertexData = {NormalizedX + QuadSizeX, NormalizedY, color.r, color.g, color.b};
            Vertices.push_back(vertexData);

            vertexData= {NormalizedX + QuadSizeX, NormalizedY + QuadSizeY, color.r, color.g, color.b};
            Vertices.push_back(vertexData);

            vertexData = {NormalizedX, NormalizedY + QuadSizeY, color.r, color.g, color.b};
            Vertices.push_back(vertexData);
        }
    }

	// Load the vertex data
	glBufferData(GL_ARRAY_BUFFER, Vertices.size() * sizeof(VertexData), Vertices.data(), GL_DYNAMIC_DRAW);

	// Draw the quads
	glDrawElements(GL_TRIANGLES, (GLsizei)Indices.size(), GL_UNSIGNED_INT, 0);

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

            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
                int VKCode = (int)msg.wParam;

                if(VKCode == 'R')
                {
                    RestartGame();
                }

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
	WindowClass.style = CS_OWNDC;

    if(RegisterClass(&WindowClass))
    {
        HWND Window = CreateWindowExA(0, WindowClass.lpszClassName,
                                      WindowClass.lpszClassName,
                                      WS_OVERLAPPEDWINDOW,
                                      CW_USEDEFAULT, CW_USEDEFAULT,
                                      WindowWidth, WindowHeight,
                                      NULL, NULL, hInst, NULL);
        if(Window)
        {
			GlobalOpenGLDC = GetDC(Window);
			Win32InitOpenGL();
			GLuint VertexBuffer;
			GLuint VAO;
			GLuint EBO;
			OpenGLGenBuffers(&VAO, &EBO, &VertexBuffer);

            ShowWindow(Window, nCmdShow);

            GlobalRunning = true;

            RestartGame();

            while(GlobalRunning)
            {
                Win32ProcessPendingMessages();

                RECT Rect;
                GetClientRect(Window, &Rect);
                int Width = Rect.right - Rect.left;
                int Height = Rect.bottom - Rect.top;
                glViewport(0, 0, Width, Height);

                UpdateAndRender(VAO, EBO, VertexBuffer);
            }

			glDeleteBuffers(1, &VertexBuffer);
			glDeleteBuffers(1, &EBO);
			glDeleteBuffers(1, &VAO);

            wglMakeCurrent(NULL, NULL);
            wglDeleteContext(GlobalOpenGLRC);
            ReleaseDC(Window, GlobalOpenGLDC);
        }
    }

    return 0;
}
