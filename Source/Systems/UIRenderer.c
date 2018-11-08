#include "UIRenderer.h"

static System *ThisSystem = NULL;

extern engineECS ECS;
extern engineCore Core;
extern engineScreen Screen;
extern engineRendering Rendering;


#define NULL_ELEMENT (UIElement){coloredRectangle,0,0,0,0,VECTOR3_ZERO,0,0,0,0}
#define STRING_CACHE_SIZE 100

//Element types
typedef enum elementType{coloredRectangle, texturedRectangle, textElement, lineElement}ElementType;
typedef struct UIElement{
    ElementType type;

    int minRectX;
    int minRectY;

    int maxRectX;
    int maxRectY;

    Vector3 color;
    GLuint texture;

    int textW, textH;

    float lineThickness;

}UIElement;
UIElement *UIElements = NULL;
int elementsCount = 0;
int maxElements = 100;


typedef struct StringTexture{
    TTF_Font *font;
    char *text;
    GLuint texture;
    int textW, textH;
    unsigned int usedLastFrame;
}StringTexture;

StringTexture stringCache[STRING_CACHE_SIZE];
int lastString = 0;

GLuint vbo[2];
int currentByteVBO = 0;
int currentElement = 0;
GLsync lastFrameSync = NULL;

//Basic forms
GLfloat baseRectangleVertex[8];
GLfloat baseRectangleUV[8] = {0,0, 0,1, 1,0, 1,1};
GLfloat baseLineVertex[4];
const int size = 4*2*sizeof(GLfloat);

//OpenGL data
GLfloat ProjectionMatrix[4][4] = {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}};

//Lua fonts list
List luaFonts;
char **luaFontNames = NULL;

//Internal functions declarations
GLuint TextToTexture(char *text, TTF_Font* font, int *w, int* h);
void MorphRectangle(GLfloat vertices[8], int minX, int minY, int maxX,int maxY);
void MorphLine(GLfloat vertices[8], int minX, int minY, int maxX,int maxY,float width);

//Initialization function - runs on engine start
void UIRendererInit(){
    ThisSystem = (System*)GetElementAt(ECS.SystemList,GetSystemID("UIRenderer"));
    UIElements = malloc(maxElements * sizeof(UIElement));

    glGenBuffers(2, vbo);
    //Vertex VBO
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, maxElements * size, NULL, GL_STREAM_DRAW);

    //UV VBO
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, maxElements * size, NULL, GL_STATIC_DRAW);
    for(int i=0; i<maxElements; i++){
        glBufferSubData(GL_ARRAY_BUFFER,i*size,size,baseRectangleUV);
    }

    luaFonts = InitList(sizeof(TTF_Font*));
    luaFontNames = malloc(sizeof(char*));
    luaFontNames[0] = NULL;
}

//UI Render Loop - runs each GameLoop iteration
void UIRendererUpdate(){

    //Force wait for the last frame UI rendering to be done before starting this frame
    if(lastFrameSync){
        while(glClientWaitSync(lastFrameSync,GL_SYNC_FLUSH_COMMANDS_BIT,0) == GL_TIMEOUT_EXPIRED);
    }
    lastFrameSync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE,0); 
    glViewport(0,0,Screen.windowWidth,Screen.windowHeight);

    //Define the projection matrix
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

    //Disable depth test
    glDisable(GL_DEPTH_TEST);
    
    //Get uniform locations
    static GLint projLoc = -1;
    if(projLoc < 0)
        projLoc = glGetUniformLocation(Rendering.Shaders[3], "projection");

    static GLint colorLoc = -1;
    if(colorLoc < 0)
        colorLoc = glGetUniformLocation(Rendering.Shaders[3], "color");

    static GLint texLoc = -1;
    if(texLoc < 0)
        texLoc = glGetUniformLocation(Rendering.Shaders[3], "texture");

    //Enable UI shader
    glUseProgram(Rendering.Shaders[3]);
    //Pass active texture ID
    glUniform1i(texLoc, 0);
    //Pass the projection matrix to the shader
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, (const GLfloat*)&ProjectionMatrix[0]);

    //Setup VBOs
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    currentByteVBO = 0;
    currentElement = 0;
    int i;
    for(i=0; i<elementsCount; i++){
        UIElement element = UIElements[i];

        if(element.type == coloredRectangle || element.type == texturedRectangle){

            MorphRectangle(baseRectangleVertex,
                           element.minRectX,
                           element.minRectY,
                           element.maxRectX,
                           element.maxRectY);

            
            if(element.type == texturedRectangle){
                //Pass texture to shader
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, element.texture);
            }else{
                //Pass no texture to shader
                glBindTexture(GL_TEXTURE_2D, 0);
            }

            //Passing uniforms to shader
            glUniform3f(colorLoc, element.color.x, element.color.y, element.color.z);

            //Passing rectangle to the vertex VBO
            void* data = glMapBufferRange(GL_ARRAY_BUFFER,currentByteVBO, size, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
            memcpy(data,baseRectangleVertex,size);
            glUnmapBuffer(GL_ARRAY_BUFFER);
            
            glDrawArrays(GL_TRIANGLE_STRIP, currentElement, 4);
            currentByteVBO+=size;
            currentElement+=4;

        }else if(element.type == textElement){
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, element.texture);

            MorphRectangle(baseRectangleVertex, 
                           element.minRectX,
                           element.minRectY, 
                           element.minRectX + (element.textW)/Screen.gameScale, 
                           element.minRectY + (element.textH)/Screen.gameScale);
            
            //Passing uniforms to shader
            glUniform3f(colorLoc, element.color.x, element.color.y, element.color.z);

            //Passing rectangle to the vertex VBO
            void* data = glMapBufferRange(GL_ARRAY_BUFFER,currentByteVBO, size, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
            memcpy(data,baseRectangleVertex,size);
            glUnmapBuffer(GL_ARRAY_BUFFER);
            
            glDrawArrays(GL_TRIANGLE_STRIP, currentElement, 4);
            currentByteVBO+=size;
            currentElement+=4;
    
        }else if(element.type == lineElement){
            
            MorphLine(baseRectangleVertex,
                      element.minRectX,
                      element.minRectY,
                      element.maxRectX,
                      element.maxRectY,
                      element.lineThickness/(2*Screen.gameScale));


            //Pass no texture to shader
            glBindTexture(GL_TEXTURE_2D, 0);
            //Passing uniforms to shader
            glUniform3f(colorLoc, element.color.x, element.color.y, element.color.z);

            //Passing rectangle to the vertex VBO
            void* data = glMapBufferRange(GL_ARRAY_BUFFER,currentByteVBO, size, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
            memcpy(data,baseRectangleVertex,size);
            glUnmapBuffer(GL_ARRAY_BUFFER);
            
            glDrawArrays(GL_TRIANGLE_STRIP, currentElement, 4);
            currentByteVBO+=size;
            currentElement+=4;
        }
    }

    //Reenable depth test and reset opengl vars
    glEnable(GL_DEPTH_TEST);

    glUseProgram(0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    for(i=0; i<lastString; i++){
        if(stringCache[i].usedLastFrame)
            stringCache[i].usedLastFrame--;
    }

    //Free the list of elements for the next frame
    elementsCount = 0;
}

//Finalization - runs at engine finish
void UIRendererFree(){
    //Close fonts
    ListCellPointer fontCell;
    ListForEach(fontCell, luaFonts){
        TTF_CloseFont(GetElementAsType(fontCell,TTF_Font*));
    }
    //Free font list
    FreeList(&luaFonts);

    //Free font name list
    char *currentFont = luaFontNames[0];
    int i=0;
    while(currentFont){
        i++;
        free(currentFont);
        currentFont = luaFontNames[i];
    }
    free(luaFontNames);

    //Free cached string texures
    for(i=0; i<lastString; i++){
        glDeleteTextures(1, &stringCache[i].texture);
        free(stringCache[i].text);
    }

    //Free UI elements list
    free(UIElements);

    //Free VBO
    glDeleteBuffers(2, vbo);
}

void InsertUIArray(UIElement e){
    if(elementsCount >= maxElements){
        maxElements *=2;
        UIElements = realloc(UIElements, maxElements*sizeof(UIElement));
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, maxElements * size, NULL, GL_STREAM_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
        glBufferData(GL_ARRAY_BUFFER, maxElements * size, NULL, GL_STATIC_DRAW);
        for(int i=0; i<maxElements; i++){
            glBufferSubData(GL_ARRAY_BUFFER,i*size,size,baseRectangleUV);
        }

    }
    UIElements[elementsCount++] = e;
}

void DrawRectangle(Vector3 min, Vector3 max, float r, float g, float b){
    if(!ThisSystem || !ThisSystem->enabled) return;

    UIElement newElement = NULL_ELEMENT;

    newElement.type = coloredRectangle;
    newElement.minRectX = min.x/Screen.gameScale;
    newElement.minRectY = min.y/Screen.gameScale;

    newElement.maxRectX = max.x/Screen.gameScale;
    newElement.maxRectY = max.y/Screen.gameScale;
    newElement.color = (Vector3){r, g, b};

    InsertUIArray(newElement);
}

void DrawRectangleTextured(Vector3 min, Vector3 max, GLuint texture, float r, float g, float b){
    if(!ThisSystem || !ThisSystem->enabled) return;

    UIElement newElement = NULL_ELEMENT;

    newElement.type = texturedRectangle;
    newElement.texture = texture;

    newElement.minRectX = min.x/Screen.gameScale;
    newElement.minRectY = min.y/Screen.gameScale;

    newElement.maxRectX = max.x/Screen.gameScale;
    newElement.maxRectY = max.y/Screen.gameScale;

    newElement.color = (Vector3){r, g, b};

    InsertUIArray(newElement);
}


void DrawTextColored(char *text, Vector3 color, int x, int y, TTF_Font* font){
    if(!ThisSystem || !ThisSystem->enabled) return;
    if(!font || !text || text[0] == '\0') return;

    UIElement newElement = NULL_ELEMENT;

    //Check if this string is in cache and get the least used string
    int i,leastUsedIndex = 0,foundLeastUsed = 0;
    for(i=0; i<lastString; i++){
        if(stringCache[i].usedLastFrame == 0 && !foundLeastUsed){
            leastUsedIndex = i;
            foundLeastUsed = 1;
        }
        if(font == stringCache[i].font && StringCompareEqual(text, stringCache[i].text) ){
            newElement.texture = stringCache[i].texture;
            newElement.textW = stringCache[i].textW;
            newElement.textH = stringCache[i].textH;
            stringCache[i].usedLastFrame++;
            break;
        }
    }

    //If this string is not in cache, create a new one
    if(i == lastString){

        //If the cache is full, replace the least used string texture with the new one
        if(lastString >= STRING_CACHE_SIZE){
            if(!foundLeastUsed){
                PrintLog(Error, "DrawTextColored: Can't render text, string cache is full!\n");
                return;
            }
            i = leastUsedIndex;
            glDeleteTextures(1, &stringCache[i].texture);
            free(stringCache[i].text);
        }else{
            lastString++;
        }

        newElement.texture = TextToTexture(text, font, &newElement.textW, &newElement.textH);

        //Create a new StringTexture
        stringCache[i].font = font;
        stringCache[i].texture = newElement.texture;
        stringCache[i].textW = newElement.textW;
        stringCache[i].textH = newElement.textH;
        stringCache[i].usedLastFrame = 2;

        stringCache[i].text = malloc((strlen(text)+1) * sizeof(char));
        strcpy(stringCache[i].text,text);    
    }
    newElement.type = textElement;

    newElement.minRectX = x/Screen.gameScale;
    newElement.minRectY = y/Screen.gameScale;
    
    newElement.color = (Vector3){color.x, color.y, color.z};

    InsertUIArray(newElement);
}

void DrawPoint(Vector3 pos, float size, GLuint texture, float r, float g, float b){
    Vector3 min = {pos.x-size, pos.y-size,0};
    Vector3 max = {pos.x+size, pos.y+size,0};

    if(texture)
        DrawRectangleTextured(min, max, texture, r, g, b);
    else
        DrawRectangle(min, max, r, g, b);
}

void DrawLine(Vector3 min, Vector3 max, float thickness, float r, float g, float b){
    if(!ThisSystem || !ThisSystem->enabled) return;
    
    UIElement newElement = NULL_ELEMENT;

    newElement.type = lineElement;

    newElement.minRectX = min.x/Screen.gameScale;
    newElement.minRectY = min.y/Screen.gameScale;

    newElement.maxRectX = max.x/Screen.gameScale;
    newElement.maxRectY = max.y/Screen.gameScale;

    newElement.color = (Vector3){r, g, b};
    newElement.lineThickness = thickness;

    InsertUIArray(newElement);
}

void MorphRectangle(GLfloat vertices[8], int minX, int minY, int maxX,int maxY){
    /* 01--45
    // |    |
    // 23--67
    */
    vertices[2] = minX;
    vertices[3] = minY;

    vertices[0] = minX;
    vertices[1] = maxY + 0.375;

    vertices[4] = maxX + 0.375;
    vertices[5] = maxY + 0.375;

    vertices[6] = maxX + 0.375;
    vertices[7] = minY;
}


void MorphLine(GLfloat vertices[8], int minX, int minY, int maxX,int maxY,float width){
    if(maxY<minY){
        int tmpX = maxX;
        int tmpY = maxY;
        maxX = minX;
        maxY = minY;
        minX = tmpX;
        minY = tmpY;
    }
    float length = Distance((Vector3){minX, minY, 0}, (Vector3){maxX, maxY, 0})/2.0;
    float xmid = (maxX+minX)/2.0;
    float ymid = (maxY+minY)/2.0;
    vertices[2] = -length;
    vertices[3] = -width;

    vertices[0] = -length;
    vertices[1] = width;

    vertices[4] = length;
    vertices[5] = width;

    vertices[6] = length;
    vertices[7] = -width;

    Vector3 dir = NormalizeVector(Subtract((Vector3){maxX, maxY, 0}, (Vector3){minX, minY, 0}));
    Matrix3x3 rot = EulerAnglesToMatrix3x3((Vector3){0,0,acos(dot(VECTOR3_FORWARD,dir)) * 180.0/PI});
    //printf("%f\n",acos(dot(VECTOR3_FORWARD,dir)) * 180.0/PI);
    int i;
    for(i=0; i<8; i+=2){
        Vector3 rpoint = RotateVector((Vector3){vertices[i], vertices[i+1], 0}, rot);
        vertices[i] = xmid + rpoint.x;
        vertices[i+1] = ymid + rpoint.y;
    }
}

GLuint TextToTexture(char *text, TTF_Font* font, int *w, int* h){	
	if(!font) return 0;
	if(!text) return 0;
	if(text[0] == '\0') return 0;

    SDL_Color whiteCol = {255,255,255,255};
    SDL_Surface * originalFont = TTF_RenderText_Solid(font, text, whiteCol);
	SDL_Surface * sFont = SDL_ConvertSurfaceFormat(originalFont,SDL_PIXELFORMAT_ARGB8888,0);

	SDL_FreeSurface(originalFont);
    if(!sFont){PrintLog(Error,"TextToTexture: Failed to render text!\n"); return 0;}

    GLuint texture;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sFont->w, sFont->h, 0, GL_BGRA, GL_UNSIGNED_BYTE, sFont->pixels);

    *w = sFont->w;
    *h = sFont->h;

    SDL_FreeSurface(sFont);

    return texture;
}


//Lua interface functions

static int l_LoadFontTTF(lua_State *L){
    if(!ThisSystem || !ThisSystem->enabled) return 0;

    const char* fontPath = luaL_checkstring (L, 1);
    const char* fontName = luaL_checkstring (L, 2);
    int fontSize = luaL_checkinteger (L, 3);

    char fullPath[512+256];
    char *fontID = malloc((256+3) * sizeof(char));

    snprintf(fontID,256+3, "%.251s%d",fontName,fontSize);

    //Check if this font with this size is already loaded
    char *curName = luaFontNames[0];
    int i=0;
    while(curName){
        i++;
        if(StringCompareEqual(curName, fontID)){
            //This font with this size is already loaded
            free(fontID);
            return 0;
        }
        curName = luaFontNames[i];
    }

    //Concatenate path, name and extension, and complete '/' if needed
    if(fontPath[strlen(fontPath)-1] == '/')
        sprintf(fullPath, "%.512s%.251s.ttf",fontPath,fontName);
    else
        sprintf(fullPath, "%.511s/%.251s.ttf",fontPath,fontName);
        

    TTF_Font *newFont = TTF_OpenFont(fullPath,fontSize);
    if(!newFont){
        PrintLog(Error,"LoadFontTTF(Lua): failed to load font! (%s)\n", fullPath);
        return 0;
    }

    char **newFontNames = realloc(luaFontNames,(GetLength(luaFonts)+2) * sizeof(char*)); //+1 for the new string, and another for the NULL
    if(!newFontNames || !newFont){
        PrintLog(Error,"LoadFontTTF(Lua): failed to realloc the font name vector!\n");
        return 0;
    }

    InsertListEnd(&luaFonts, &newFont);

    luaFontNames = newFontNames;
    luaFontNames[GetLength(luaFonts)] = NULL;
    luaFontNames[GetLength(luaFonts)-1] = fontID;

    return 0;
}

//(WIP)
static int l_DrawTextColored (lua_State *L) {
    //Get the arguments
    //Text to be displayed
    const char* text = luaL_checkstring (L, 1);

    //Text color
    if(!lua_istable(L, 2)){
        PrintLog(Warning,"DrawTextColored(Lua): Second argument must be a table with 'r', 'g' and 'b' numbers!\n");
        luaL_checktype(L, 2, LUA_TTABLE); //Check again to cause script error and stop execution
        return 0;
    }
    lua_getfield(L,2, "r");
    lua_getfield(L,2, "g");
    lua_getfield(L,2, "b");

    Vector3 uiColor = {luaL_checknumber(L,-3), luaL_checknumber(L,-2), luaL_checknumber(L,-1)};

    //Text position
    if(!lua_istable(L, 3)){
        PrintLog(Warning,"DrawTextColored(Lua): Third argument must be a table with 'x' and 'y' numbers!\n");
        luaL_checktype(L, 3, LUA_TTABLE); //Check again to cause script error and stop execution
        return 0;
    }
    lua_getfield(L,3, "x");
    lua_getfield(L,3, "y");

    int px = luaL_checknumber(L,-2);
    int py = luaL_checknumber(L,-1);

    //Font index
    int fontIndex = luaL_checkoption(L, 4, NULL, (const char *const*) luaFontNames);

    DrawTextColored((char*)text, uiColor, px, py, GetElementAsType(GetCellAt(luaFonts, fontIndex),TTF_Font*) );
    return 0; //Return number of results
}

static int l_DrawRectangle (lua_State *L) {
    //Get the arguments

    //Minimum rectangle coordinates
    if(!lua_istable(L, 1)){
        PrintLog(Warning,"DrawRectangle(Lua): First argument must be a table with 'x' and 'y' numbers!\n");
        luaL_checktype(L, 1, LUA_TTABLE); //Check again to cause script error and stop execution
        return 0;
    }
    lua_getfield(L,1, "x");
    lua_getfield(L,1, "y");
    Vector3 min = {luaL_checknumber(L,-2), luaL_checknumber(L,-1),0};

    //Maximum rectangle coordinates
    if(!lua_istable(L, 2)){
        PrintLog(Warning,"DrawRectangle(Lua): Second argument must be a table with 'x' and 'y' numbers!\n");
        luaL_checktype(L, 2, LUA_TTABLE); //Check again to cause script error and stop execution
        return 0;
    }
    lua_getfield(L,2, "x");
    lua_getfield(L,2, "y");
    Vector3 max = {luaL_checknumber(L,-2), luaL_checknumber(L,-1),0};

    //Rectangle colors
    if(!lua_istable(L, 3)){
        PrintLog(Warning,"DrawRectangle(Lua): Third argument must be a table with 'r', 'g' and 'b' numbers!\n");
        luaL_checktype(L, 3, LUA_TTABLE); //Check again to cause script error and stop execution
        return 0;
    }
    lua_getfield(L,3, "r");
    lua_getfield(L,3, "g");
    lua_getfield(L,3, "b");
    Vector3 uiColor = {luaL_checknumber(L,-3), luaL_checknumber(L,-2), luaL_checknumber(L,-1)};

    DrawRectangle(min, max, uiColor.x, uiColor.y, uiColor.z);
    return 0; //Return number of results
}

static int l_DrawLine (lua_State *L) {
    //Get the arguments

    //Line start coordinate
    if(!lua_istable(L, 1)){
        PrintLog(Warning,"DrawLine(Lua): First argument must be a table with 'x' and 'y' numbers!\n");
        luaL_checktype(L, 1, LUA_TTABLE); //Check again to cause script error and stop execution
        return 0;
    }
    lua_getfield(L,1, "x");
    lua_getfield(L,1, "y");
    Vector3 start = {luaL_checknumber(L,-2), luaL_checknumber(L,-1),0};


    //Line end coordinate
    if(!lua_istable(L, 2)){
        PrintLog(Warning,"DrawLine(Lua): Second argument must be a table with 'x' and 'y' numbers!\n");
        luaL_checktype(L, 2, LUA_TTABLE); //Check again to cause script error and stop execution
        return 0;
    }
    lua_getfield(L,2, "x");
    lua_getfield(L,2, "y");
    Vector3 end = {luaL_checknumber(L,-2), luaL_checknumber(L,-1),0};

    float thickness = luaL_checknumber (L, 3);

    //Rectangle colors
    if(!lua_istable(L, 4)){
        PrintLog(Warning,"DrawRectangle(Lua): Third argument must be a table with 'r', 'g' and 'b' numbers!\n");
        luaL_checktype(L, 4, LUA_TTABLE); //Check again to cause script error and stop execution
        return 0;
    }
    lua_getfield(L,4, "r");
    lua_getfield(L,4, "g");
    lua_getfield(L,4, "b");
    Vector3 uiColor = {luaL_checknumber(L,-3), luaL_checknumber(L,-2), luaL_checknumber(L,-1)};

    DrawLine(start, end, thickness, uiColor.x, uiColor.y, uiColor.z);
    return 0; //Return number of results
}

void UIRendererRegisterLuaFunctions(){
    lua_pushcfunction(Core.lua, l_DrawTextColored);
    lua_setglobal(Core.lua, "DrawTextColored");

    lua_pushcfunction(Core.lua, l_DrawRectangle);
    lua_setglobal(Core.lua, "DrawRectangle");

    lua_pushcfunction(Core.lua, l_DrawLine);
    lua_setglobal(Core.lua, "DrawLine");

    lua_pushcfunction(Core.lua, l_LoadFontTTF);
    lua_setglobal(Core.lua, "LoadFontTTF");
}