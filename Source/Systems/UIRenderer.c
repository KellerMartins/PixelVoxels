#include "UIRenderer.h"

static System *ThisSystem = NULL;

extern engineECS ECS;
extern engineScreen Screen;
extern engineRendering Rendering;



//Element types
typedef enum elementType{coloredRectangle, texturedRectangle, textElement}ElementType;
typedef struct UIElement{
    ElementType type;

    Vector3 minRect;
    Vector3 maxRect;

    Vector3 color;
    GLuint *texture;

    char* text;
    TTF_Font* font;

}UIElement;
List UIElements;

//Basic forms
Vector3 baseRectangleVertex[4];
GLfloat baseRectangleUV[8] = {0,0, 0,1, 1,0, 1,1};

//OpenGL data
GLuint uvVBO;
GLfloat ProjectionMatrix[4][4] = {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}};

//Internal functions declarations
GLuint TextToTexture(char *text, Vector3 color, TTF_Font* font, int *w, int* h);



//Initialization function - runs on engine start
void UIRendererInit(){
    ThisSystem = (System*)GetElementAt(ECS.SystemList,GetSystemID("UIRenderer"));
    UIElements = InitList(sizeof(UIElement));

    //Create and bind the UV VBO
    glGenBuffers(1, &uvVBO);
    glBindBuffer(GL_ARRAY_BUFFER, uvVBO);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

    //Pass the default UV data
    glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(GLfloat), baseRectangleUV, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

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
}

//UI Render Loop - runs each GameLoop iteration
void UIRendererUpdate(){
    glDisable(GL_DEPTH_TEST);

    glBindFramebuffer(GL_FRAMEBUFFER, Rendering.frameBuffer);
    glViewport(0,0,Screen.windowWidth,Screen.windowHeight);

    ListCellPointer cell;
    ListForEach(cell, UIElements){
        UIElement element = GetElementAsType(cell, UIElement);

        if(element.type == coloredRectangle || element.type == texturedRectangle){
            MorphRectangle(baseRectangleVertex, element.minRect, element.maxRect);

            //Pass no texture to shader
            glBindTexture(GL_TEXTURE_2D, 0);

            //Passing rectangle to the vertex VBO
            glBindBuffer(GL_ARRAY_BUFFER, Rendering.vbo[0]);
            glBufferData(GL_ARRAY_BUFFER, 4 * 3 * sizeof(GLfloat), baseRectangleVertex, GL_STREAM_DRAW);
            glEnableVertexAttribArray(0);

            glBindBuffer(GL_ARRAY_BUFFER, uvVBO);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(1);

            glUseProgram(Rendering.Shaders[3]);

            //Passing uniforms to shader
            glUniformMatrix4fv(glGetUniformLocation(Rendering.Shaders[3], "projection"), 1, GL_FALSE, &ProjectionMatrix[0]);
            if(element.type == coloredRectangle){
                glUniform3f(glGetUniformLocation(Rendering.Shaders[3], "color"), element.color.x, element.color.y, element.color.z);
            }
            
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

            glUseProgram(0);
        }else if(element.type == textElement){

            glEnable(GL_BLEND);
            
            int textW, textH;
            GLuint textTexture = TextToTexture(element.text, element.color, element.font, &textW, &textH);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textTexture);

            MorphRectangle(baseRectangleVertex, element.minRect, (Vector3){element.minRect.x + (textW + 0.375)/Screen.gameScale, element.minRect.y + (textH + 0.375)/Screen.gameScale});
            

            //Passing rectangle to the vertex VBO
            glBindBuffer(GL_ARRAY_BUFFER, Rendering.vbo[0]);
            glBufferData(GL_ARRAY_BUFFER, 4 * 3 * sizeof(GLfloat), baseRectangleVertex, GL_STREAM_DRAW);
            glEnableVertexAttribArray(0);

            //Passing rectangle uvs the uv VBO
            glBindBuffer(GL_ARRAY_BUFFER, uvVBO);
            glEnableVertexAttribArray(1);

            glUseProgram(Rendering.Shaders[3]);

            //Passing uniforms to shader
            glUniform1i(glGetUniformLocation(Rendering.Shaders[3], "texture"), 0);
            glUniformMatrix4fv(glGetUniformLocation(Rendering.Shaders[3], "projection"), 1, GL_FALSE, &ProjectionMatrix[0]);
            glUniform3f(glGetUniformLocation(Rendering.Shaders[3], "color"), 1.0f, 1.0f, 1.0f);
            
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glUseProgram(0);

            glDeleteTextures(1, &textTexture);

            glDisable(GL_BLEND);

            free(element.text);
        }
    }

    glEnable(GL_DEPTH_TEST);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //Free the list of elements for the next frame
    FreeList(&UIElements);
}

//Finalization - runs at engine finish
void UIRendererFree(){
    FreeList(&UIElements);
}

void DrawRectangle(Vector3 min, Vector3 max, float r, float g, float b){
    if(!ThisSystem || !ThisSystem->enabled) return;

    UIElement newElement;

    newElement.type = coloredRectangle;
    newElement.minRect = (Vector3){min.x/Screen.gameScale, min.y/Screen.gameScale, 0};
    newElement.maxRect = (Vector3){max.x/Screen.gameScale, max.y/Screen.gameScale, 0};
    newElement.color = (Vector3){r, g, b};

    InsertListEnd(&UIElements, &newElement);
}

void DrawTextColored(char *text, SDL_Color color, int x, int y, TTF_Font* font){
    if(!ThisSystem || !ThisSystem->enabled) return;
    if(!font || !text || text[0] == '\0') return;

    UIElement newElement;

    newElement.text = malloc((strlen(text)+1) * sizeof(char));
    strcpy(newElement.text, text);

    newElement.type = textElement;
    newElement.font = font;
    newElement.minRect = (Vector3){x/Screen.gameScale, y/Screen.gameScale, 0};
    newElement.color = (Vector3){color.r/255.0f, color.g/255.0f, color.b/255.0f};

    InsertListEnd(&UIElements, &newElement);
}

void MorphRectangle(Vector3 vertices[4], Vector3 min, Vector3 max){
    /* 0---2
    // |   |
    // 1---3
    */
    vertices[1] = min;
    vertices[1].z = 0;

    vertices[0].x = min.x;
    vertices[0].y = max.y;
    vertices[0].z = 0;

    vertices[2] = max;
    vertices[2].z = 0;

    vertices[3].x = max.x;
    vertices[3].y = min.y;
    vertices[3].z = min.z;
}

GLuint TextToTexture(char *text, Vector3 color, TTF_Font* font, int *w, int* h){	
	if(!font) return 0;
	if(!text) return 0;
	if(text[0] == '\0') return 0;

    SDL_Color whiteCol = {255,255,255,255};
    SDL_Surface * originalFont = TTF_RenderText_Solid(font, text, whiteCol);
	SDL_Surface * sFont = SDL_ConvertSurfaceFormat(originalFont,SDL_PIXELFORMAT_ARGB8888,0);

	SDL_FreeSurface(originalFont);
    if(!sFont){printf("Failed to render text!\n"); return 0;}

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