#define GL_ARRAY_BUFFER                   0x8892
#define GL_STATIC_DRAW                    0x88E4
#define GL_DYNAMIC_DRAW                   0x88E8
#define GL_STREAM_DRAW                    0x88E0

#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31

#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82

#define GL_DEBUG_OUTPUT_SYNCHRONOUS       0x8242
#define GL_DEBUG_SEVERITY_HIGH            0x9146
#define GL_DEBUG_SEVERITY_MEDIUM          0x9147
#define GL_DEBUG_SEVERITY_LOW             0x9148
#define GL_DEBUG_TYPE_MARKER              0x8268
#define GL_DEBUG_TYPE_PUSH_GROUP          0x8269
#define GL_DEBUG_TYPE_POP_GROUP           0x826A
#define GL_DEBUG_SEVERITY_NOTIFICATION    0x826B
GL_DEBUG_CALLBACK(OpenGLDebugCallback)
{
    if(severity == GL_DEBUG_SEVERITY_HIGH)
    {
        char *ErrorMessage = (char *)message;
        assert(!"OpenGL Error encountered");
    }
}

static void
OpenGLGenBuffers(GLuint *VAO, GLuint *VertexBuffer, GLuint *ColorBuffer)
{
    glGenBuffers(1, VertexBuffer);
    glGenBuffers(1, ColorBuffer);
	glGenVertexArrays(1, VAO);

	glBindVertexArray(*VAO);

    glBindBuffer(GL_ARRAY_BUFFER, *VertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void *)0);
	glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, *ColorBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Colors), Colors, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void *)0);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

    GLenum	Error = glGetError();
    if (Error != GL_NO_ERROR)
    {
        assert(!"OpenGLGenBuffers failed");
    }
}

static GLuint
OpenGLCompileProgram(char *Header, char *VertexCode, char *FragmentCode)
{
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLchar *VertexShaderCode[] = {
        Header,
		VertexCode,
	};
	glShaderSource(VertexShaderID, ArrayCount(VertexShaderCode), VertexShaderCode, 0);
	glCompileShader(VertexShaderID);

	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	GLchar *FragmentShaderCode[] = {
        Header,
		FragmentCode,
	};
	glShaderSource(FragmentShaderID, ArrayCount(FragmentShaderCode), FragmentShaderCode, 0);
	glCompileShader(FragmentShaderID);

	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	GLint Success = 0;
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Success);
	if(Success == GL_FALSE)
	{
		GLsizei Ignored;
		char VertexErrors[4096];
		glGetShaderInfoLog(VertexShaderID, sizeof(VertexErrors), &Ignored, VertexErrors);
		assert(!"Shader compilation failed.");
	}

	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Success);
	if(Success == GL_FALSE)
	{
		GLsizei Ignored;
		char FragmentErrors[4096];
		glGetShaderInfoLog(FragmentShaderID, sizeof(FragmentErrors), &Ignored, FragmentErrors);
		assert(!"Shader compilation failed.");
	}

	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Success);
	if(Success == GL_FALSE)
	{
		GLsizei Ignored;
		char ProgramErrors[4096];
		glGetProgramInfoLog(ProgramID, sizeof(ProgramErrors), &Ignored, ProgramErrors);
		assert(!"Shader linking failed.");
	}

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void 
CreateOrthoProjection(float Left, float Right, float Bottom, float Top, 
                      float NearPlane, float FarPlane, float* ProjectionMatrix)
{
	float tx = -(Right + Left) / (Right - Left);
    float ty = -(Top + Bottom) / (Top - Bottom);
    float tz = -(FarPlane + NearPlane) / (FarPlane - NearPlane);

    ProjectionMatrix[0] = 2.0f / (Right - Left);
    ProjectionMatrix[1] = 0.0f;
    ProjectionMatrix[2] = 0.0f;
    ProjectionMatrix[3] = 0.0f;

    ProjectionMatrix[4] = 0.0f;
    ProjectionMatrix[5] = 2.0f / (Top - Bottom);
    ProjectionMatrix[6] = 0.0f;
    ProjectionMatrix[7] = 0.0f;

    ProjectionMatrix[8] = 0.0f;
    ProjectionMatrix[9] = 0.0f;
    ProjectionMatrix[10] = -2.0f / (FarPlane - NearPlane);
    ProjectionMatrix[11] = 0.0f;

    ProjectionMatrix[12] = tx;
    ProjectionMatrix[13] = ty;
    ProjectionMatrix[14] = tz;
    ProjectionMatrix[15] = 1.0f;
}
