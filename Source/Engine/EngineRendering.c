#include "EngineRendering.h"

/////////////////////////////////External data//////////////////////////////////

//From Engine/EngineCore.c
extern engineCore Core;
extern engineTime Time;
extern engineScreen Screen;

////////////////////////////////////////////////////////////////////////////////

engineRendering Rendering;

int InitRenderer(){
    //Initialize renderer
	Core.renderer = SDL_CreateRenderer(Core.window, -1, SDL_RENDERER_ACCELERATED);
	SDL_RenderSetLogicalSize(Core.renderer, Screen.gameWidth, Screen.gameHeight);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");


	////----------------- OpenGL initializations -----------------
	//Setting OpenGL version
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 2 );

	//Creating OpenGL context
	Core.glContext = SDL_GL_CreateContext(Core.window);
    if (Core.glContext == NULL)
    {
        printf("Cannot create OpenGL context with error: %s\n",SDL_GetError());
        return 0;
    }

	//Initialize GLEW
	glewExperimental = GL_TRUE; 
	GLenum glewError = glewInit();
	if( glewError != GLEW_OK )
	{
		printf( "Error initializing GLEW! %s\n", glewGetErrorString( glewError ) );
        return 0;
	}

	//Unset Vsync
	if( SDL_GL_SetSwapInterval( 0 ) < 0 )
	{
		printf( "Warning: Unable to unset VSync! SDL Error: %s\n", SDL_GetError() );
	}

	//Initialize OpenGL features
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);
    glDepthFunc(GL_LEQUAL);

	Rendering.cameraPosition = (Vector3){0,0,0};
	Rendering.clearScreenColor = (SDL_Color){0,0,0,0};

    //Framebuffer
    glGenFramebuffers(1, &Rendering.frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, Rendering.frameBuffer);

    //Screen Render Texture
    glGenTextures(1, &Rendering.screenTexture);
    glBindTexture(GL_TEXTURE_2D, Rendering.screenTexture);
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, Screen.gameWidth, Screen.gameHeight, 0,GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenRenderbuffers(1, &Rendering.depthRenderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, Rendering.depthRenderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, Screen.gameWidth, Screen.gameHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, Rendering.depthRenderBuffer);

    // Set "screenTexture" as our colour attachement #0
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, Rendering.screenTexture, 0);

    // Set the list of draw buffers.
    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
		printf("InitEngine: Error generating framebuffer! (%x)\n",glCheckFramebufferStatus(GL_FRAMEBUFFER));
        return 0;
	}

    glBindFramebuffer(GL_FRAMEBUFFER,0);

    // VAO Generation and binding
    glGenVertexArrays(1, &Rendering.vao);
    glBindVertexArray(Rendering.vao);

    // VBO generation and binding
	// VBOs for 3D rendering
    glGenBuffers(3, Rendering.vbo3D);

    glBindBuffer(GL_ARRAY_BUFFER, Rendering.vbo3D[0]); //Vertex
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, Rendering.vbo3D[1]); //Color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, Rendering.vbo3D[2]); //Normal
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

	//VBOS for 2D rendering
	glGenBuffers(2, Rendering.vbo2D);

    glBindBuffer(GL_ARRAY_BUFFER, Rendering.vbo3D[0]); //Vertex
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, Rendering.vbo3D[1]); //UV
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	
    //Compile shaders
    if(!CompileAndLinkShader("Shaders/ScreenVert.vs","Shaders/ScreenFrag.fs",0)) printf(">Failed to compile/link Screen shader! Description above\n\n");
    else printf(">Compiled/linked Screen shader sucessfully!\n");

    if(!CompileAndLinkShader("Shaders/VoxelVert.vs","Shaders/VoxelFrag.fs",1)) printf(">Failed to compile/link VoxelVert shader! Description above\n\n");
    else printf(">Compiled/linked Voxel shader sucessfully!\n");

	if(!CompileAndLinkShader("Shaders/VoxelSmallVert.vs","Shaders/VoxelSmallFrag.fs",2)) printf(">Failed to compile/link VoxelSmall shader! Description above\n\n");
    else printf(">Compiled/linked VoxelSmall shader sucessfully!\n");

	if(!CompileAndLinkShader("Shaders/UIVert.vs","Shaders/UIFrag.fs",3)) printf(">Failed to compile/link UI shader! Description above\n\n");
    else printf(">Compiled/linked UI shader sucessfully!\n");

	//Load voxel palette
	LoadVoxelPalette("Assets/Game/Textures/magicaPalette.png");

    return 1;
}

////----------------- Rendering Functions //-----------------

Vector3 PositionToGameScreenCoords(Vector3 position){
	Vector3 screenPos;
	position = (Vector3){roundf(position.x),roundf(position.y),roundf(position.z)};

	//Position to screen projection
	screenPos.x = (int)(((position.x) - (position.y))*2 + roundf(-Rendering.cameraPosition.x)) + 0.375;
    screenPos.y = (int)(((position.x) + (position.y)) + (position.z + Rendering.cameraPosition.z )*2 + roundf(-Rendering.cameraPosition.y)) + 0.375;
	screenPos.z = (position.z)/256.0;

	//Transforming to screen coordinates
	screenPos.x = Screen.windowWidth/2 + (screenPos.x/(float)Screen.gameWidth) * Screen.windowWidth;
	screenPos.y = Screen.windowHeight/2 + (screenPos.y/(float)Screen.gameHeight) * Screen.windowHeight;

	return screenPos;
}

Vector3 PositionToCameraCoords(Vector3 position){
	Vector3 screenPos;
	position = (Vector3){roundf(position.x),roundf(position.y),roundf(position.z)};

	//Position to screen projection
	screenPos.x = (int)(((position.x) - (position.y))*2 + 0.375);
    screenPos.y = (int)(((position.x) + (position.y)) + (position.z)*2) + 0.375;
	screenPos.z = (position.z)/256.0;

	return screenPos;
}

void ClearRender(SDL_Color col){
    glBindFramebuffer(GL_FRAMEBUFFER, Rendering.frameBuffer);

    glClearColor(col.r/255.0, col.g/255.0, col.b/255.0,0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderToScreen(){

    //Define the projection matrix
	GLfloat ProjectionMatrix[4][4] = {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}};
    float right = Screen.windowWidth;
    float left = 0;
    float top = Screen.windowHeight;
    float bottom = 0;
    float near = -0.1;
    float far = 0.1;
    
    ProjectionMatrix[0][0] = 2.0f/(right-left);
    ProjectionMatrix[1][1] = 2.0f/(top-bottom);
    ProjectionMatrix[2][2] = -2.0f/(far-near);
    ProjectionMatrix[3][3] = 1;
    ProjectionMatrix[3][0] = -(right + left)/(right - left);
    ProjectionMatrix[3][1] = -(top + bottom)/(top - bottom);
    ProjectionMatrix[3][2] = -(far + near)/(far - near);

    glViewport(0,0,Screen.windowWidth,Screen.windowHeight);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Rendering.screenTexture);

    glUseProgram(Rendering.Shaders[0]);
	glUniform1i(glGetUniformLocation(Rendering.Shaders[0], "fbo_texture"), 0);
	glUniformMatrix4fv(glGetUniformLocation(Rendering.Shaders[0], "projection"), 1, GL_FALSE, (const GLfloat*)&ProjectionMatrix[0]);

    glUniform1f(glGetUniformLocation(Rendering.Shaders[0], "pWidth"), 1.0/(float)Screen.gameWidth);
    glUniform1f(glGetUniformLocation(Rendering.Shaders[0], "pHeight"), 1.0/(float)Screen.gameHeight);

    glUniform1f(glGetUniformLocation(Rendering.Shaders[0], "vignettePower"), 0.25);
    glUniform1f(glGetUniformLocation(Rendering.Shaders[0], "redShiftPower"), 2);    
    glUniform1f(glGetUniformLocation(Rendering.Shaders[0], "redShiftSpread"), 0);
    
	GLfloat quadVertex[8] = {0, Screen.windowHeight, 0, 0, Screen.windowWidth, Screen.windowHeight, Screen.windowWidth, 0};
    GLfloat quadUV[8] = {0,1, 0,0, 1,1, 1,0};

	//Passing rectangle to the vertex VBO
	glBindBuffer(GL_ARRAY_BUFFER, Rendering.vbo2D[0]);
	glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(GLfloat), quadVertex, GL_STREAM_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	//Passing rectangle uvs the uv VBO
	glBindBuffer(GL_ARRAY_BUFFER, Rendering.vbo2D[1]);
	glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(GLfloat), quadUV, GL_STREAM_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisable( GL_TEXTURE_2D );
    glUseProgram(0);
}


//Debug text renderer, use UIRenderer for the game
void RenderTextDebug(char *text, SDL_Color color, int x, int y, TTF_Font* font) 
{	
	if(!font) return;
	if(!text) return;
	if(text[0] == '\0') return;

    SDL_Surface * originalFont = TTF_RenderText_Solid(font, text, color);
	SDL_Surface * sFont = SDL_ConvertSurfaceFormat(originalFont,SDL_PIXELFORMAT_ARGB8888,0);

	SDL_FreeSurface(originalFont);
    if(!sFont){printf("Failed to render text!\n"); return;}

    //Define the projection matrix
	GLfloat ProjectionMatrix[4][4] = {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}};
    float right = Screen.windowWidth;
    float left = 0;
    float top = Screen.windowHeight;
    float bottom = 0;
    float near = -0.1;
    float far = 0.1;
    
    ProjectionMatrix[0][0] = 2.0f/(right-left);
    ProjectionMatrix[1][1] = 2.0f/(top-bottom);
    ProjectionMatrix[2][2] = -2.0f/(far-near);
    ProjectionMatrix[3][3] = 1;
    ProjectionMatrix[3][0] = -(right + left)/(right - left);
    ProjectionMatrix[3][1] = -(top + bottom)/(top - bottom);
    ProjectionMatrix[3][2] = -(far + near)/(far - near);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    
    GLuint texture;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sFont->w, sFont->h, 0, GL_BGRA, 
                    GL_UNSIGNED_BYTE, sFont->pixels);

	GLfloat quadVertex[8] = {x, y + sFont->h + 0.375, x, y, x + sFont->w + 0.375, y + sFont->h + 0.375, x + sFont->w + 0.375, y};
    GLfloat quadUV[8] = {0,0, 0,1, 1,0, 1,1};

	//Passing rectangle to the vertex VBO
	glBindBuffer(GL_ARRAY_BUFFER, Rendering.vbo2D[0]);
	glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(GLfloat), quadVertex, GL_STREAM_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	//Passing rectangle uvs the uv VBO
	glBindBuffer(GL_ARRAY_BUFFER, Rendering.vbo2D[1]);
	glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(GLfloat), quadUV, GL_STREAM_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glUseProgram(Rendering.Shaders[3]);

	//Passing uniforms to shader
	glUniform1i(glGetUniformLocation(Rendering.Shaders[3], "texture"), 0);
	glUniformMatrix4fv(glGetUniformLocation(Rendering.Shaders[3], "projection"), 1, GL_FALSE, (const GLfloat*)&ProjectionMatrix[0]);
	glUniform3f(glGetUniformLocation(Rendering.Shaders[3], "color"), 1.0f, 1.0f, 1.0f);
	
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glUseProgram(0);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    
    glDeleteTextures(1, &texture);
    SDL_FreeSurface(sFont);
}

const GLchar *LoadShaderSource(char *filename) {
    if(!filename) return NULL;

    FILE *file = fopen(filename, "r");             // open 
    fseek(file, 0L, SEEK_END);                     // find the end
    size_t size = ftell(file);                     // get the size in bytes
    GLchar *shaderSource = calloc(sizeof(GLchar), size + 1);        // allocate enough bytes
    rewind(file);                                  // go back to file beginning
    fread(shaderSource, size, sizeof(char), file); // read each char into ourblock
    fclose(file);                                  // close the stream

    return shaderSource;
}

int CompileAndLinkShader(char *vertPath, char *fragPath, unsigned shaderIndex){
    //Create an empty vertex shader handle
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const GLchar *vShaderSource = LoadShaderSource(vertPath);

    //Send the vertex shader source code to GL
    glShaderSource(vertexShader, 1, &vShaderSource, 0);

    //Compile the vertex shader
    glCompileShader(vertexShader);

    GLint isCompiled = 0;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);

        //The maxLength includes the NULL character
        GLchar *infoLog = (GLchar *) malloc(maxLength * sizeof(GLchar));
        glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &infoLog[0]);
        
        glDeleteShader(vertexShader);
        free((void*)vShaderSource);

        printf("Vertex Shader Info Log:\n%s\n",infoLog);
        
        free(infoLog);
        
        return 0;
    }

    //Create an empty fragment shader handle
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar *fShaderSource = LoadShaderSource(fragPath);

    //Send the fragment shader source code to GL
    glShaderSource(fragmentShader, 1, &fShaderSource, 0);

    //Compile the fragment shader
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

        //The maxLength includes the NULL character
        GLchar *infoLog = (GLchar *) malloc(maxLength * sizeof(GLchar));
        glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &infoLog[0]);
        
        //We don't need the shader anymore.
        glDeleteShader(fragmentShader);
        free((void*)fShaderSource);

        glDeleteShader(vertexShader);
        free((void*)vShaderSource);

        printf("Fragment Shader Info Log:\n%s\n",infoLog);
        
        free(infoLog);

        return 0;
    }

    //Vertex and fragment shaders are successfully compiled.
    //Now time to link them together into a program.
    //Get a program object.
    Rendering.Shaders[shaderIndex] = glCreateProgram();

    //Attach our shaders to our program
    glAttachShader(Rendering.Shaders[shaderIndex], vertexShader);
    glAttachShader(Rendering.Shaders[shaderIndex], fragmentShader);

    //Link our program
    glLinkProgram(Rendering.Shaders[shaderIndex]);

    //Note the different functions here: glGetProgram* instead of glGetShader*.
    GLint isLinked = 0;
    glGetProgramiv(Rendering.Shaders[shaderIndex], GL_LINK_STATUS, (int *)&isLinked);
    if(isLinked == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetProgramiv(Rendering.Shaders[shaderIndex], GL_INFO_LOG_LENGTH, &maxLength);

        //The maxLength includes the NULL character
        GLchar *infoLog = (GLchar *) malloc(maxLength * sizeof(GLchar));
        glGetProgramInfoLog(Rendering.Shaders[shaderIndex], maxLength, &maxLength, &infoLog[0]);
        
        //We don't need the program anymore.
        glDeleteProgram(Rendering.Shaders[shaderIndex]);
        //Don't leak shaders either.
        glDeleteShader(vertexShader);
        free((void*)vShaderSource);
        glDeleteShader(fragmentShader);
        free((void*)fShaderSource);

        printf("Shader linkage Info Log:\n%s\n",infoLog);
        
        free(infoLog);
        return 0;
    }

    //Always detach shaders after a successful link.
    glDetachShader(Rendering.Shaders[shaderIndex], vertexShader);
    glDetachShader(Rendering.Shaders[shaderIndex], fragmentShader);
    free((void*)vShaderSource);
    free((void*)fShaderSource);

    return 1;
}

void ReloadShaders(){
    int i;
    for(i=0;i<sizeof(Rendering.Shaders) / sizeof(Rendering.Shaders[0]); i++){
        glDeleteProgram(Rendering.Shaders[i]);
    }

    if(!CompileAndLinkShader("Shaders/ScreenVert.vs","Shaders/ScreenFrag.fs",0)){
        printf(">Failed to compile/link Screen shader! Description above\n\n");
	}else{ 
        printf(">Compiled/linked Screen shader sucessfully!\n\n");
	}

    if(!CompileAndLinkShader("Shaders/VoxelVert.vs","Shaders/VoxelFrag.fs",1)){ 
        printf(">Failed to compile/link Voxel shader! Description above\n\n");
	}else{
        printf(">Compiled/linked Voxel shader sucessfully!\n\n");
	}

	if(!CompileAndLinkShader("Shaders/VoxelSmallVert.vs","Shaders/VoxelSmallFrag.fs",2)){
		printf(">Failed to compile/link VoxelSmall shader! Description above\n\n");
	}   
    else{
        printf(">Compiled/linked VoxelSmall shader sucessfully!\n\n");
	}

	if(!CompileAndLinkShader("Shaders/UIVert.vs","Shaders/UIFrag.fs",3)){
		printf(">Failed to compile/link UI shader! Description above\n\n");
    }else{
		printf(">Compiled/linked UI shader sucessfully!\n");
	}
}

void LoadVoxelPalette(char path[]){
    SDL_Surface * palSurf = IMG_Load(path);
    if(!palSurf){ printf(">Error loading palette!\n"); return; }

    int i;
    Uint8 r,g,b,a;

    for(i=0;i<255;i++){
        Uint32 *sPix = (Uint32 *)(palSurf->pixels + i* palSurf->format->BytesPerPixel);

        SDL_GetRGBA(*sPix,palSurf->format,&r,&g,&b,&a);
        Pixel color = {b,g,r,a};
        Rendering.voxelColors[i+1] = color;
    }
    SDL_FreeSurface(palSurf);
    printf(">Loaded palette sucessfully!\n");
}

void MoveCamera(float x, float y, float z){
    Rendering.cameraPosition.x +=x*Time.deltaTime;
    Rendering.cameraPosition.y +=y*Time.deltaTime;
    Rendering.cameraPosition.z +=z*Time.deltaTime;
    //printf("CamPos: |%2.1f|%2.1f|%2.1f|\n",cameraPosition.x,cameraPosition.y,cameraPosition.z);
}

void TranslateCamera(float x, float y, float z){
    Rendering.cameraPosition.x = x;
    Rendering.cameraPosition.y = y;
    Rendering.cameraPosition.z = z;
}