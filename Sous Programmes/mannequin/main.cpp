//SDL Libraries
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

//OpenGL Libraries
#include <GL/glew.h>
#include <GL/gl.h>

//GML libraries
#include <conio.h>


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>

#include <cstdint>

#include "Cube.h"
#include "Sphere.h"
#include "Cylinder.h"

#include <vector>
#include <stack>

#include "Shader.h"
#include "logger.h"

#define WIDTH     800
#define HEIGHT    800
#define FRAMERATE 60
#define TIME_PER_FRAME_MS  (1.0f/FRAMERATE * 1e3)
#define INDICE_TO_PTR(x) ((void*)(x))


struct Material {
    GLuint TEXTUREID;
    const float ka;
    const float kd;
    const float ks;
    const float alpha;
};

struct Objet {
    GLuint Vboid;
    Geometry* shape;
    Material* material;
    glm::mat4 matrix = glm::mat4(1.0f);
    Shader* shader;
    std::vector<Objet*> children;
};



struct Light {
    glm::vec3 pos = glm::vec3(1.0f);
    glm::vec3 color = glm::vec3(0.0f,0.0f,255.0f);
};

struct Mannequin {
    Objet forme;
    int chute;
    int dir;
    bool EnVie;
};

void Draw(Objet go, Shader* shader, std::stack<glm::mat4> matrices) {
    uint32_t nbVertices = go.shape->getNbVertices();
    matrices.push(matrices.top() * go.matrix);
    glm::mat4 mvp = matrices.top();
    
    glUseProgram(shader->getProgramID());
    {
        glBindBuffer(GL_ARRAY_BUFFER, go.Vboid);

        //VBO
        GLint vPosition = glGetAttribLocation(shader->getProgramID(), "vPosition");
        glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(vPosition);

        GLint vColor = glGetAttribLocation(shader->getProgramID(), "vColor");
        glVertexAttribPointer(vColor, 3, GL_FLOAT, GL_FALSE, 0, INDICE_TO_PTR(nbVertices * 3 * sizeof(float)));
        glEnableVertexAttribArray(vColor);

        GLint vUV = glGetAttribLocation(shader->getProgramID(), "vUV");
        glVertexAttribPointer(vUV, 2, GL_FLOAT, GL_FALSE, 0, INDICE_TO_PTR(nbVertices * (3 + 3) * sizeof(float)));
        glEnableVertexAttribArray(vUV);

        GLint uMVP = glGetUniformLocation(shader->getProgramID(), "uMVP");
        glUniformMatrix4fv(uMVP, 1, GL_FALSE, glm::value_ptr(mvp));
        
        

        glDrawArrays(GL_TRIANGLES, 0, nbVertices);

    }
    glUseProgram(0);

    for (int i = 0; i<go.children.size(); i++) {
        Draw( *go.children[i] ,shader,matrices);
    }

    matrices.pop();
}

void tomber(Mannequin &M, Shader* shader, std::stack<glm::mat4> matrices){
    if (M.EnVie) {
        Draw(M.forme, shader, matrices);
        if (M.chute > 0&&M.chute<=90) {
            M.forme.matrix = glm::rotate(M.forme.matrix, glm::radians(1.0f), glm::vec3(cos(M.dir), 0.0, sin(M.dir)));
            M.chute++;
        }
        if (M.chute > 90) {
            M.EnVie = false;
            M.forme.matrix = glm::rotate(M.forme.matrix, glm::radians(-90.0f), glm::vec3(cos(M.dir), 0.0,sin(M.dir)));
            M.chute = 1;
        }
    }
}

int main(int argc, char* argv[])
{
    ////////////////////////////////////////
    //SDL2 / OpenGL Context initialization : 
    ////////////////////////////////////////

    //Initialize SDL2
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0)
    {
        ERROR("The initialization of the SDL failed : %s\n", SDL_GetError());
        return 0;
    }

    //Create a Window
    SDL_Window* window = SDL_CreateWindow("VR Camera",                           //Titre
        SDL_WINDOWPOS_UNDEFINED,               //X Position
        SDL_WINDOWPOS_UNDEFINED,               //Y Position
        WIDTH, HEIGHT,                         //Resolution
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN); //Flags (OpenGL + Show)

//Initialize OpenGL Version (version 3.0)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    //Initialize the OpenGL Context (where OpenGL resources (Graphics card resources) lives)
    SDL_GLContext context = SDL_GL_CreateContext(window);

    //Tells GLEW to initialize the OpenGL function with this version
    glewExperimental = GL_TRUE;
    glewInit();


    //Start using OpenGL to draw something on screen
    glViewport(0, 0, WIDTH, HEIGHT); //Draw on ALL the screen

    //The OpenGL background color (RGBA, each component between 0.0f and 1.0f)
    glClearColor(0.0, 0.0, 0.0, 1.0); //Full Black

    glEnable(GL_DEPTH_TEST); //Active the depth test 

    
 
    Sphere head(32,32);
    const float* headpositions = head.getVertices();
    const float* headcolors = head.getNormals();
    const float* headuv = head.getUVs();
    uint32_t headnbVertices = head.getNbVertices();
    
    Cube body;
    const float* bodypositions = body.getVertices();
    const float* bodycolors = body.getNormals();
    const float* bodyuv = body.getUVs();
    uint32_t bodynbVertices = body.getNbVertices();

    Cylinder leg(10);
    const float* legpositions = leg.getVertices();
    const float* legcolors = leg.getNormals();
    const float* leguv = leg.getUVs();
    uint32_t legnbvertices = leg.getNbVertices();

    
    SDL_Surface* img = IMG_Load("./Images/truc.png");
    SDL_Surface* rgbImg = SDL_ConvertSurfaceFormat(img, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(img);


    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, rgbImg->w, rgbImg->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)rgbImg->pixels);

    glGenerateMipmap(GL_TEXTURE_2D);

    SDL_FreeSurface(rgbImg);
    

    GLuint sphere;
    glGenBuffers(1, &sphere);
    glBindBuffer(GL_ARRAY_BUFFER, sphere);

    glBufferData(GL_ARRAY_BUFFER, headnbVertices * (3 + 3 + 2) * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, headnbVertices * 3 * sizeof(float), headpositions);
    glBufferSubData(GL_ARRAY_BUFFER, headnbVertices * 3 * sizeof(float), headnbVertices * 3 * sizeof(float), headcolors);
    glBufferSubData(GL_ARRAY_BUFFER, headnbVertices * (3 + 3) * sizeof(float), headnbVertices * 2 * sizeof(float), headuv);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLuint cube;
    glGenBuffers(1, &cube);
    glBindBuffer(GL_ARRAY_BUFFER, cube);

    glBufferData(GL_ARRAY_BUFFER, bodynbVertices * (3 + 3 + 2) * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, bodynbVertices * 3 * sizeof(float), bodypositions);
    glBufferSubData(GL_ARRAY_BUFFER, bodynbVertices * 3 * sizeof(float), bodynbVertices * 3 * sizeof(float), bodycolors);
    glBufferSubData(GL_ARRAY_BUFFER, bodynbVertices * (3 + 3) * sizeof(float), bodynbVertices * 2 * sizeof(float), bodyuv);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLuint cylinder;
    glGenBuffers(1, &cylinder);
    glBindBuffer(GL_ARRAY_BUFFER, cylinder);

    glBufferData(GL_ARRAY_BUFFER, legnbvertices * (3 + 3 + 2) * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, legnbvertices * 3 * sizeof(float), legpositions);
    glBufferSubData(GL_ARRAY_BUFFER, legnbvertices * 3 * sizeof(float), legnbvertices * 3 * sizeof(float), legcolors);
    glBufferSubData(GL_ARRAY_BUFFER, legnbvertices * (3 + 3) * sizeof(float), legnbvertices * 2 * sizeof(float), leguv);
    glBindBuffer(GL_ARRAY_BUFFER, 0);



    const char* vertPath = "Shaders/color.vert";
    const char* fragPath = "Shaders/color.frag";

    FILE* vert = fopen(vertPath, "r");
    FILE* frag = fopen(fragPath, "r");

    Shader* shader = Shader::loadFromFiles(vert, frag);

    fclose(vert);
    fclose(frag);

    if (shader == nullptr) {
        std::cerr << "The shader 'color' did not compile correctly. Exiting." << std::endl;
        return EXIT_FAILURE;
    }

    Objet tete;
    tete.Vboid = sphere;
    tete.shape = &head;

    Objet corps;
    corps.Vboid = cube;
    corps.shape = &body;
    corps.children.push_back(&tete);

    Objet jambe;
    jambe.Vboid = cylinder;
    jambe.shape = &leg;
    jambe.children.push_back(&corps);

    Objet socle;
    socle.Vboid = sphere;
    socle.shape = &head;
    socle.children.push_back(&jambe);
    
    jambe.matrix = glm::rotate(jambe.matrix, glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0));
    corps.matrix = glm::rotate(corps.matrix, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));


    tete.matrix = glm::scale(tete.matrix, glm::vec3(0.5, 0.5, 0.5));
    corps.matrix = glm::scale(corps.matrix, glm::vec3(2.0, 0.7, 2.0));
    jambe.matrix = glm::scale(jambe.matrix, glm::vec3(0.64, 0.64,1.96));
    jambe.matrix = glm::scale(jambe.matrix, glm::vec3(1.0, 0.1, 1.0));
    socle.matrix = glm::scale(socle.matrix, glm::vec3(0.4, 0.4, 0.4));


    tete.matrix = glm::translate(tete.matrix, glm::vec3(0.0, 1.5, 0.0));
    corps.matrix = glm::translate(corps.matrix, glm::vec3(0.0,0.7,0.0));
    jambe.matrix = glm::translate(jambe.matrix, glm::vec3(0.0, 0.0,- 0.5));
    socle.matrix = glm::translate(socle.matrix, glm::vec3(0.0, -1.5, 0.0));

    
    






    int indice = 1;
    int compteur = 0;

    std::vector<Mannequin> Liste_Mannequin_En_Vie;
    std::vector<Mannequin> Liste_Mannequin_Mort;


    Mannequin M1;
    M1.EnVie = true;
    M1.forme = socle;
    M1.chute = 1;
    M1.dir = 0;
    

    Mannequin M2 = M1;
    M2.EnVie = false;
    M2.dir = 0;
    M2.forme.matrix = glm::translate(M2.forme.matrix, glm::vec3(-2.0, 0.0, -2.0));
    M2.forme.matrix = glm::rotate(M2.forme.matrix, glm::radians(45.0f), glm::vec3(0.0, 1.0, 0.0));

    Mannequin M3 = M1;
    M3.EnVie = false;
    M3.dir = 0;
    M3.forme.matrix = glm::translate(M3.forme.matrix, glm::vec3(1.5, 0.0, 0.0));
    M3.forme.matrix = glm::rotate(M3.forme.matrix, glm::radians(45.0f), glm::vec3(0.0, 1.0, 0.0));
    
    

    Liste_Mannequin_Mort.push_back(M1);
    Liste_Mannequin_Mort.push_back(M2);
    Liste_Mannequin_Mort.push_back(M3);
    
    bool isOpened = true;

    //Main application loop
    while (isOpened)
    {
        //Time in ms telling us when this frame started. Useful for keeping a fix framerate
        uint32_t timeBegin = SDL_GetTicks();

        //Fetch the SDL events
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_WINDOWEVENT:
                switch (event.window.event)
                {
                case SDL_WINDOWEVENT_CLOSE:
                    isOpened = false;
                    break;
                default:
                    break;
                }
                break;
                //We can add more event, like listening for the keyboard or the mouse. See SDL_Event documentation for more details
            }
        }

        //Clear the screen : the depth buffer and the color buffer
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        glm::mat4 projection(1.0f);
        glm::mat4 view(1.0f);
        
        
        

        
        
        glm::mat4 vp = projection * view ;
        std::stack<glm::mat4> matrices;
        matrices.push(vp);
       
        for(int i =0;i<Liste_Mannequin_En_Vie.size();i++)
        {
            tomber(Liste_Mannequin_En_Vie[i], shader, matrices);
            if (Liste_Mannequin_En_Vie[i].EnVie==false)
            {
                Liste_Mannequin_Mort.push_back(Liste_Mannequin_En_Vie[i]);
                Liste_Mannequin_En_Vie.erase(Liste_Mannequin_En_Vie.begin() + i);
                i = i - 1;

            }
        }

        compteur = compteur + 1;
        if (compteur == 80 && Liste_Mannequin_En_Vie.size() < 2) {
            int i = rand() % Liste_Mannequin_Mort.size();
            Liste_Mannequin_Mort[i].EnVie = true;
            Liste_Mannequin_En_Vie.push_back(Liste_Mannequin_Mort[i]);
            Liste_Mannequin_Mort.erase(Liste_Mannequin_Mort.begin() + i );
            compteur = 0;
        }
        
        //Draw(socle, shader, matrices);

        


        //Display on screen (swap the buffer on screen and the buffer you are drawing on)
        SDL_GL_SwapWindow(window);

        //Time in ms telling us when this frame ended. Useful for keeping a fix framerate
        uint32_t timeEnd = SDL_GetTicks();

        //We want FRAMERATE FPS
        if (timeEnd - timeBegin < TIME_PER_FRAME_MS)
            SDL_Delay((uint32_t)(TIME_PER_FRAME_MS)-(timeEnd - timeBegin));
    }

    glDeleteBuffers(1, &sphere);
    glDeleteBuffers(1, &cube);
    glDeleteBuffers(1, &cylinder);
    delete shader;

    //Free everything
    if (context != NULL)
        SDL_GL_DeleteContext(context);
    if (window != NULL)
        SDL_DestroyWindow(window);

    return 0;
}
