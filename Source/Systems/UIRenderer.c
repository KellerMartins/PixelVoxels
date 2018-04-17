#include "UIRenderer.h"

static System *ThisSystem;

extern engineECS ECS;
extern engineScreen Screen;
extern engineRendering Rendering;

typedef enum ElementType{coloredRectangle, texturedRectangle, text}ElementType;
typedef struct UIElement{
    ElementType type;

    Vector3 minRect;
    Vector3 maxRect;

    Vector3 color;
    GLuint *texture;

    char** text;

}UIElement;
List UIElements;

Vector3 baseRectangle[4];
GLfloat ProjectionMatrix[4][4] = {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}};

//Runs on engine start
void UIRendererInit(){
    ThisSystem = (System*)GetElementAt(ECS.SystemList,GetSystemID("UIRenderer"));
    UIElements = InitList(sizeof(UIElement));
    UIElement testEl = {coloredRectangle, (Vector3){0,0}, (Vector3){Screen.gameWidth/2,Screen.gameHeight/2},(Vector3){0.1,0.5,0.9}, 0, NULL};
    InsertListStart(&UIElements, &testEl);

    //Define the projection matrix
    float right = Screen.gameWidth;
    float left = 0;
    float top = Screen.gameHeight;
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

//Runs each GameLoop iteration
void UIRendererUpdate(){
    glDisable(GL_DEPTH_TEST);

    glBindFramebuffer(GL_FRAMEBUFFER, Rendering.frameBuffer);
    glViewport(0,0,Screen.gameWidth,Screen.gameHeight);

    ListCellPointer cell;
    ListForEach(cell, UIElements){
        UIElement element = GetElementAsType(cell, UIElement);

        if(element.type == coloredRectangle || element.type == texturedRectangle){
            MorphRectangle(baseRectangle, element.minRect, element.maxRect);

            //Passing rectangle to the vertex VBO
            glBindBuffer(GL_ARRAY_BUFFER, Rendering.vbo[0]);
            glBufferData(GL_ARRAY_BUFFER, 4 * 3 * sizeof(GLfloat), baseRectangle, GL_STREAM_DRAW);
            glEnableVertexAttribArray(0);

            glUseProgram(Rendering.Shaders[3]);

            //Passing uniforms to shader
            glUniformMatrix4fv(glGetUniformLocation(Rendering.Shaders[3], "projection"), 1, GL_FALSE, &ProjectionMatrix[0]);
            if(element.type == coloredRectangle){
                glUniform3f(glGetUniformLocation(Rendering.Shaders[3], "color"), element.color.x, element.color.y, element.color.z);
            }
            
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

            glUseProgram(0);
        }
    }
}

//Runs at engine finish
void UIRendererFree(){
    FreeList(&UIElements);
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

GLuint TextToTexture(char *text, SDL_Color color, TTF_Font* font) 
{	
	if(!font) return 0;
	if(!text) return 0;
	if(text[0] == '\0') return 0;

    SDL_Surface * originalFont = TTF_RenderText_Solid(font, text, color);
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
    SDL_FreeSurface(sFont);

    return texture;
}