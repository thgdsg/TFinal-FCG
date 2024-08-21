//     Universidade Federal do Rio Grande do Sul
//             Instituto de Informática
//       Departamento de Informática Aplicada
//
//    INF01047 Fundamentos de Computação Gráfica
//               Prof. Eduardo Gastal
//
//                   LABORATÓRIO 4
//

// Arquivos "headers" padrões de C podem ser incluídos em um
// programa C++, sendo necessário somente adicionar o caractere
// "c" antes de seu nome, e remover o sufixo ".h". Exemplo:
//    #include <stdio.h> // Em C
//  vira
//    #include <cstdio> // Em C++
//
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>

// Headers abaixo são específicos de C++
#include <map>
#include <stack>
#include <string>
#include <vector>
#include <limits>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <mutex>

// Headers das bibliotecas OpenGL
#include <glad/glad.h>   // Criação de contexto OpenGL 3.3
#include <GLFW/glfw3.h>  // Criação de janelas do sistema operacional

// Headers da biblioteca GLM: criação de matrizes e vetores.
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>

// Headers da biblioteca para carregar modelos obj
#include <tiny_obj_loader.h>
#include <stb_image.h>

// Headers locais, definidos na pasta "include/"
#include "utils.h"
#include "matrices.h"
#include "classes.h"

// Estrutura que representa um modelo geométrico carregado a partir de um
// arquivo ".obj". Veja https://en.wikipedia.org/wiki/Wavefront_.obj_file .
struct ObjModel
{
    tinyobj::attrib_t                 attrib;
    std::vector<tinyobj::shape_t>     shapes;
    std::vector<tinyobj::material_t>  materials;

    // Este construtor lê o modelo de um arquivo utilizando a biblioteca tinyobjloader.
    // Veja: https://github.com/syoyo/tinyobjloader
    ObjModel(const char* filename, const char* basepath = NULL, bool triangulate = true)
    {
        printf("Carregando objetos do arquivo \"%s\"...\n", filename);

        // Se basepath == NULL, então setamos basepath como o dirname do
        // filename, para que os arquivos MTL sejam corretamente carregados caso
        // estejam no mesmo diretório dos arquivos OBJ.
        std::string fullpath(filename);
        std::string dirname;
        if (basepath == NULL)
        {
            auto i = fullpath.find_last_of("/");
            if (i != std::string::npos)
            {
                dirname = fullpath.substr(0, i+1);
                basepath = dirname.c_str();
            }
        }

        std::string warn;
        std::string err;
        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename, basepath, triangulate);

        if (!err.empty())
            fprintf(stderr, "\n%s\n", err.c_str());

        if (!ret)
            throw std::runtime_error("Erro ao carregar modelo.");

        for (size_t shape = 0; shape < shapes.size(); ++shape)
        {
            if (shapes[shape].name.empty())
            {
                fprintf(stderr,
                        "*********************************************\n"
                        "Erro: Objeto sem nome dentro do arquivo '%s'.\n"
                        "Veja https://www.inf.ufrgs.br/~eslgastal/fcg-faq-etc.html#Modelos-3D-no-formato-OBJ .\n"
                        "*********************************************\n",
                    filename);
                throw std::runtime_error("Objeto sem nome.");
            }
            printf("- Objeto '%s'\n", shapes[shape].name.c_str());
        }

        printf("OK.\n");
    }
};

//  Função para desenhar o crosshair
void drawCrosshair(GLuint shaderProgram);
void LoadCrosshairShader();

// Carrega Texturas
void LoadTextureImage(const char* filename);

// Colisões
bool CheckCollisionWithSphere(const glm::vec4& cameraPos, const Target& target);
bool CheckCollisionWithWorld(const glm::vec4& cameraPosition);
bool CheckSphereCollisionWithSphere(const Target& target1, const Target& target2);

// Declaração de funções utilizadas para pilha de matrizes de modelagem.
void PushMatrix(glm::mat4 M);
void PopMatrix(glm::mat4& M);

// Declaração de várias funções utilizadas em main().  Essas estão definidas
// logo após a definição de main() neste arquivo.
void BuildTrianglesAndAddToVirtualScene(ObjModel*); // Constrói representação de um ObjModel como malha de triângulos para renderização
void ComputeNormals(ObjModel* model); // Computa normais de um ObjModel, caso não existam.
void LoadShadersFromFiles(); // Carrega os shaders de vértice e fragmento, criando um programa de GPU
void DrawVirtualObject(const char* object_name); // Desenha um objeto armazenado em g_VirtualScene
GLuint LoadShader_Vertex(const char* filename);   // Carrega um vertex shader
GLuint LoadShader_Fragment(const char* filename); // Carrega um fragment shader
void LoadShader(const char* filename, GLuint shader_id); // Função utilizada pelas duas acima
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id); // Cria um programa de GPU
void PrintObjModelInfo(ObjModel*); // Função para debugging

void RenderGameMap(GameMap& gameMap, glm::mat4 view, glm::mat4 projection); // Função para renderizar o mapa
void DrawTarget(const Target& target); // Função para desenhar um alvo
void HandleMouseClick(GLFWwindow* window, double xpos, double ypos, glm::mat4 view, glm::mat4 projection);
glm::vec4 ScreenToWorld(GLFWwindow* window, double xpos, double ypos, glm::mat4 view, glm::mat4 projection);
bool IsTargetHit(const Target& target, const glm::vec4& cameraPos, const glm::vec4& rayDir, glm::mat4 view, glm::mat4 projection);
void SpawnTarget();
void SpawnTarget_mov();
float RandomFloat(float min, float max);
void UpdateCountdown();

//funções de renderização de objetos controlados pelo jogador
void RenderGun(glm::vec4 camera_up_vector, glm::vec4 camera_view_vector,glm::vec4 camera_position_c);
void RenderPlayer(glm::vec4 camera_position_c, glm::vec4 camera_view_vector, glm::vec4 camera_up_vector);

// Declaração de funções auxiliares para renderizar texto dentro da janela
// OpenGL. Estas funções estão definidas no arquivo "textrendering.cpp".
void TextRendering_Init();
float TextRendering_LineHeight(GLFWwindow* window);
float TextRendering_CharWidth(GLFWwindow* window);
void TextRendering_PrintString(GLFWwindow* window, const std::string &str, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrix(GLFWwindow* window, glm::mat4 M, float x, float y, float scale = 1.0f);
void TextRendering_PrintVector(GLFWwindow* window, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProduct(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProductMoreDigits(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProductDivW(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);

// Funções abaixo renderizam como texto na janela OpenGL algumas matrizes e
// outras informações do programa. Definidas após main().
void TextRendering_ShowFramesPerSecond(GLFWwindow* window);

// Funções callback para comunicação com o sistema operacional e interação do
// usuário. Veja mais comentários nas definições das mesmas, abaixo.
void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
void ErrorCallback(int error, const char* description);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

// Definimos uma estrutura que armazenará dados necessários para renderizar
// cada objeto da cena virtual.
struct SceneObject
{
    std::string  name;        // Nome do objeto
    size_t       first_index; // Índice do primeiro vértice dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    size_t       num_indices; // Número de índices do objeto dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    GLenum       rendering_mode; // Modo de rasterização (GL_TRIANGLES, GL_TRIANGLE_STRIP, etc.)
    GLuint       vertex_array_object_id; // ID do VAO onde estão armazenados os atributos do modelo
    glm::vec3    bbox_min; // Axis-Aligned Bounding Box do objeto
    glm::vec3    bbox_max;
};

// Abaixo definimos variáveis globais utilizadas em várias funções do código.

// A cena virtual é uma lista de objetos nomeados, guardados em um dicionário
// (map).  Veja dentro da função BuildTrianglesAndAddToVirtualScene() como que são incluídos
// objetos dentro da variável g_VirtualScene, e veja na função main() como
// estes são acessados.
std::map<std::string, SceneObject> g_VirtualScene;

// Pilha que guardará as matrizes de modelagem.
std::stack<glm::mat4>  g_MatrixStack;

// Razão de proporção da janela (largura/altura). Veja função FramebufferSizeCallback().
float g_ScreenRatio = 1.0f;

// Ângulos de Euler que controlam a rotação de um dos cubos da cena virtual
float g_AngleX = 0.0f;
float g_AngleY = 0.0f;
float g_AngleZ = 0.0f;

// "g_LeftMouseButtonPressed = true" se o usuário está com o botão esquerdo do mouse
// pressionado no momento atual. Veja função MouseButtonCallback().
bool g_LeftMouseButtonPressed = false;
bool g_RightMouseButtonPressed = false; // Análogo para botão direito do mouse
bool g_MiddleMouseButtonPressed = false; // Análogo para botão do meio do mouse

// Variáveis globais que armazenam a última posição do cursor do mouse, para
// que possamos calcular quanto que o mouse se movimentou entre dois instantes
// de tempo. Utilizadas no callback CursorPosCallback() abaixo.
double g_LastCursorPosX, g_LastCursorPosY;
double g_LastClickTime = 0.0;
const double DMG_COOLDOWN = 0.1;
double lastSpawnTime = 0.0;
double lastSpawnTime2 = 0.0;
double lastShotTime = 0.0;
int countdownTime = 60;
auto startTime = std::chrono::steady_clock::now();
std::mutex spawnMutex;

// Variáveis que definem a câmera em coordenadas esféricas, controladas pelo
// usuário através do mouse (veja função CursorPosCallback()). A posição
// efetiva da câmera é calculada dentro da função main(), dentro do loop de
// renderização.
float g_CameraTheta = 0.0f; // Ângulo no plano ZX em relação ao eixo Z
float g_CameraPhi = 0.0f;   // Ângulo em relação ao eixo Y
float g_CameraDistance = 3.5f; // Distância da câmera para a origem

// Variável que controla o tipo de projeção utilizada: perspectiva ou ortográfica.
bool g_UsePerspectiveProjection = true;
bool g_ThirdPersonCamera = false;
bool g_Fullscreen = false;
GLFWmonitor* g_Monitor = nullptr;
int g_WindowPosX, g_WindowPosY, g_WindowWidth, g_WindowHeight;

//variavel que controla o movimento
bool PRESS_W = false, PRESS_A = false, PRESS_S = false, PRESS_D = false, PRESS_SHIFT = false, PRESS_R = false;
bool press_space = false, press_p = false;
bool fim_jogo = false;
// Variáveis que definem um programa de GPU (shaders). Veja função LoadShadersFromFiles().
GLuint g_GpuProgramID_obj = 0;
GLint g_model_uniform;
GLint g_view_uniform;
GLint g_projection_uniform;
GLint g_object_id_uniform;
GLint g_bbox_min_uniform;
GLint g_bbox_max_uniform;

GLuint g_GpuProgramID_crosshair = 0;
GLint g_model_uniform_crosshair;
GLint g_view_uniform_crosshair;
GLint g_projection_uniform_crosshair;

// Número de texturas carregadas pela função LoadTextureImage()
GLuint g_NumLoadedTextures = 0;

// Lista de alvos
std::vector<Target> targets;
Player jogador;
 //pontuacao
int main(int argc, char* argv[])
{
    // Nova variável, velocidade
    float camera_speed = 4.0f;
    // Nova variável, tempo anterior
    float prev_time = (float)glfwGetTime();
    // Nova variável, tempo que passou
    float delta_t = 0.0f;
    // Nova variável, gravidade
    float gravity = 9.8f;
    // Nova variável, posição do chão
    float ground_level = 0.5f;
    // Nova variável, altura do pulo
    float jump_force = 5.0f;

    // Criação do GameMap
    GameMap gameMap;


    // Inicializamos a biblioteca GLFW, utilizada para criar uma janela do
    // sistema operacional, onde poderemos renderizar com OpenGL.
    int success = glfwInit();
    if (!success)
    {
        fprintf(stderr, "ERROR: glfwInit() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    // Definimos o callback para impressão de erros da GLFW no terminal
    glfwSetErrorCallback(ErrorCallback);

    // Pedimos para utilizar OpenGL versão 3.3 (ou superior)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    // Pedimos para utilizar o perfil "core", isto é, utilizaremos somente as
    // funções modernas de OpenGL.
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Criamos uma janela do sistema operacional, com 800 colunas e 600 linhas
    // de pixels, e com título "INF01047 ...".
    GLFWwindow* window;
    window = glfwCreateWindow(1600, 900, "INF01047 - Trabalho Final", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        fprintf(stderr, "ERROR: glfwCreateWindow() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    // Definimos a função de callback que será chamada sempre que o usuário
    // pressionar alguma tecla do teclado ...
    glfwSetKeyCallback(window, KeyCallback);
    // ... ou clicar os botões do mouse ...
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    // ... ou movimentar o cursor do mouse em cima da janela ...
    glfwSetCursorPosCallback(window, CursorPosCallback);
    // ... ou rolar a "rodinha" do mouse.
    glfwSetScrollCallback(window, ScrollCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Indicamos que as chamadas OpenGL deverão renderizar nesta janela
    glfwMakeContextCurrent(window);

    // Carregamento de todas funções definidas por OpenGL 3.3, utilizando a
    // biblioteca GLAD.
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    // Definimos a função de callback que será chamada sempre que a janela for
    // redimensionada, por consequência alterando o tamanho do "framebuffer"
    // (região de memória onde são armazenados os pixels da imagem).
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    FramebufferSizeCallback(window, 1600, 900); // Forçamos a chamada do callback acima, para definir g_ScreenRatio.

    // Imprimimos no terminal informações sobre a GPU do sistema
    const GLubyte *vendor      = glGetString(GL_VENDOR);
    const GLubyte *renderer    = glGetString(GL_RENDERER);
    const GLubyte *glversion   = glGetString(GL_VERSION);
    const GLubyte *glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION);

    printf("GPU: %s, %s, OpenGL %s, GLSL %s\n", vendor, renderer, glversion, glslversion);

    // Carregamos os shaders de vértices e de fragmentos que serão utilizados
    // para renderização. Veja slides 180-200 do documento Aula_03_Rendering_Pipeline_Grafico.pdf.
    //
    LoadShadersFromFiles();
    LoadCrosshairShader();

    // Carregamos duas imagens para serem utilizadas como textura
    LoadTextureImage("../../data/teste_chao.jpg");      // TextureImage0
    LoadTextureImage("../../data/target_movimento.jpg");      // TextureImage1
    LoadTextureImage("../../data/target_parado.jpg");      // TextureImage2
    LoadTextureImage("../../data/Color.bmp");  //TextureImage3
    LoadTextureImage("../../data/teste_parede.jpg");  //TextureImage4
    LoadTextureImage("../../data/Mario_Albedo.png");  //TextureImage5


    // Construímos a representação de objetos geométricos através de malhas de triângulos
    ObjModel spheremodel("../../data/sphere.obj");
    ComputeNormals(&spheremodel);
    BuildTrianglesAndAddToVirtualScene(&spheremodel);


    ObjModel planemodel("../../data/plane.obj");
    ComputeNormals(&planemodel);
    BuildTrianglesAndAddToVirtualScene(&planemodel);

    ObjModel awpmodel("../../data/AWP_Dragon_Lore.obj");
    ComputeNormals(&awpmodel);
    BuildTrianglesAndAddToVirtualScene(&awpmodel);

    ObjModel playermodel("../../data/Mario.obj");
    ComputeNormals(&playermodel);
    BuildTrianglesAndAddToVirtualScene(&playermodel);
    
    if ( argc > 1 )
    {
        ObjModel model(argv[1]);
        BuildTrianglesAndAddToVirtualScene(&model);
    }

    // Inicializamos o código para renderização de texto.
    TextRendering_Init();

    // Habilitamos o Z-buffer. Veja slides 104-116 do documento Aula_09_Projecoes.pdf.
    glEnable(GL_DEPTH_TEST);

    // Habilitamos o Backface Culling. Veja slides 8-13 do documento Aula_02_Fundamentos_Matematicos.pdf, slides 23-34 do documento Aula_13_Clipping_and_Culling.pdf e slides 112-123 do documento Aula_14_Laboratorio_3_Revisao.pdf.
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);
    //glFrontFace(GL_CCW);
    
    // Computamos a posição da câmera utilizando coordenadas esféricas.  As
    // variáveis g_CameraDistance, g_CameraPhi, e g_CameraTheta são
    // controladas pelo mouse do usuário. Veja as funções CursorPosCallback()
    // e ScrollCallback().
    float r = g_CameraDistance;
    float y = r*sin(g_CameraPhi);
    float z = r*cos(g_CameraPhi)*cos(g_CameraTheta);
    float x = r*cos(g_CameraPhi)*sin(g_CameraTheta);

    // Abaixo definimos as varáveis que efetivamente definem a câmera virtual.
    // Veja slides 195-227 e 229-234 do documento Aula_08_Sistemas_de_Coordenadas.pdf.
    glm::vec4 camera_position_c  = glm::vec4(x,y,z,1.0f); // Ponto "c", centro da câmera
    glm::vec4 camera_lookat_l    = glm::vec4(0.0f,0.0f,0.0f,1.0f); // Ponto "l", para onde a câmera (look-at) estará sempre olhando
    glm::vec4 camera_view_vector = camera_lookat_l - camera_position_c; // Vetor "view", sentido para onde a câmera está virada
    glm::vec4 camera_up_vector   = glm::vec4(0.0f,1.0f,0.0f,0.0f); // Vetor "up" fixado para apontar para o "céu" (eito Y global)
    glm::vec4 camera_position_third_person;

    // Novo vetor, velocidade da câmera 
    glm::vec4 camera_velocity   = glm::vec4(0.0f,0.0f,0.0f,0.0f);
    startTime = std::chrono::steady_clock::now();
    // Ficamos em um loop infinito, renderizando, até que o usuário feche a janela
    while (!glfwWindowShouldClose(window))
    {
        // Aqui executamos as operações de renderização

        // Definimos a cor do "fundo" do framebuffer como branco.  Tal cor é
        // definida como coeficientes RGBA: Red, Green, Blue, Alpha; isto é:
        // Vermelho, Verde, Azul, Alpha (valor de transparência).
        // Conversaremos sobre sistemas de cores nas aulas de Modelos de Iluminação.
        //
        //           R     G     B     A
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

        // "Pintamos" todos os pixels do framebuffer com a cor definida acima,
        // e também resetamos todos os pixels do Z-buffer (depth buffer).
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Pedimos para a GPU utilizar o programa de GPU criado acima (contendo
        // os shaders de vértice e fragmentos).
        glUseProgram(g_GpuProgramID_obj);

        glm::mat4 view;
        if (g_ThirdPersonCamera)
        {
            // Distância da câmera em terceira pessoa
            float third_person_distance = 1.0f;

            // Calcule a posição da câmera em terceira pessoa
            camera_position_third_person = camera_position_c - third_person_distance * (camera_view_vector/norm(camera_view_vector));
            camera_position_third_person.y += 0.5f; // Ajuste a altura da câmera em terceira pessoa
            camera_position_third_person.x -= 0.5f; 
            // Verifique se a câmera está abaixo da altura mínima permitida
            if (camera_position_third_person.y < 0.5f)
            {
                camera_position_third_person.y = 0.5f;
            }

            view = Matrix_Camera_View(camera_position_third_person, camera_view_vector, camera_up_vector);
            RenderPlayer(camera_position_c, camera_view_vector, camera_up_vector);
        }
        else
        {
            view = Matrix_Camera_View(camera_position_c, camera_view_vector, camera_up_vector);
        }

        // Agora computamos a matriz de Projeção.
        glm::mat4 projection;

        // Note que, no sistema de coordenadas da câmera, os planos near e far
        // estão no sentido negativo! Veja slides 176-204 do documento Aula_09_Projecoes.pdf.
        float nearplane = -0.01f;  // Posição do "near plane"
        float farplane  = -30.0f; // Posição do "far plane"

        // Projeção Perspectiva.
        // Para definição do field of view (FOV), veja slides 205-215 do documento Aula_09_Projecoes.pdf.
        float field_of_view = 3.141592 / 2.5f;
        projection = Matrix_Perspective(field_of_view, g_ScreenRatio, nearplane, farplane);


        glm::mat4 model = Matrix_Identity(); // Transformação identidade de modelagem

        // Enviamos as matrizes "view" e "projection" para a placa de vídeo
        // (GPU). Veja o arquivo "shader_vertex.glsl", onde estas são
        // efetivamente aplicadas em todos os pontos.
        glUniformMatrix4fv(g_view_uniform       , 1 , GL_FALSE , glm::value_ptr(view));
        glUniformMatrix4fv(g_projection_uniform , 1 , GL_FALSE , glm::value_ptr(projection));

        #define SPHERE 0
        #define SPHERE_PARADA 1
        #define PLANE  2
        #define GUN 3
        #define PLANE_PAREDE 4
        #define MARIO 5
        
        // Remove alvos expirados
        targets.erase(
            std::remove_if(targets.begin(), targets.end(), [](const Target& target) {
                return target.ShouldBeRemoved();
            }),
            targets.end()
        );

        // Renderiza os alvos
        for (auto& target : targets) {
            if (target.IsAlive()) {
                if (target.GetType() == 1) {
                    target.UpdatePosition();
                }
                DrawTarget(target);
                if (CheckCollisionWithSphere(camera_position_c, target)) {
                    // Calcule a direção oposta à colisão
                    glm::vec4 posicao_target = glm::vec4(target.GetX(), target.GetY(), target.GetZ(), 1.0f);
                    glm::vec4 direction = (camera_position_c - posicao_target)/(norm(camera_position_c - posicao_target));
                    
                    // Mova a câmera na direção oposta até que não haja mais colisão
                    while (CheckCollisionWithSphere(camera_position_c, target)) {
                        camera_position_c += glm::vec4(direction.x * 0.01f, direction.y * 0.01f, direction.z * 0.01f, 0.0f); // Ajuste o valor 0.01f conforme necessário
                    }
                }
                // Verifica colisão entre targets e separa-os
                for (auto& target2 : targets) {
                    if (&target != &target2 && target2.IsAlive()) {
                        if (CheckSphereCollisionWithSphere(target, target2)) {
                            // Calcule a direção oposta à colisão
                            glm::vec4 posicao_target1 = glm::vec4(target.GetX(), target.GetY(), target.GetZ(), 1.0f);
                            glm::vec4 posicao_target2 = glm::vec4(target2.GetX(), target2.GetY(), target2.GetZ(), 1.0f);
                            glm::vec4 direction = (posicao_target2 - posicao_target1) / glm::length(posicao_target2 - posicao_target1);
                            
                            // Mova o target2 na direção oposta até que não haja mais colisão
                            while (CheckSphereCollisionWithSphere(target, target2)) {
                                target2.SetX(target2.GetX() + direction.x * 0.1f);
                                target2.SetY(target2.GetY() + direction.y * 0.1f);
                                target2.SetZ(target2.GetZ() + direction.z * 0.1f); // Ajuste o valor 0.01f conforme necessário
                            }
                        }
                    }
                }
            }

        }

        RenderGun(camera_up_vector, camera_view_vector, camera_position_c);
       
        RenderGameMap(gameMap, view, projection);
        
        // Copiado do FAQ
        float current_time = (float)glfwGetTime();
        delta_t = current_time  - prev_time;
        prev_time = current_time;

        // Vetores W e U
        glm::vec4 w_vector = -camera_view_vector / norm(camera_view_vector);
        glm::vec4 u_vector = crossproduct(camera_up_vector, w_vector) / norm(crossproduct(camera_up_vector, w_vector));

        // Movimentação da câmera
        if(press_space && camera_position_c.y == ground_level){
            camera_velocity.y = jump_force;
            camera_position_c.y += camera_velocity.y * delta_t;
        }
        // Direção (para normalizar o movimento diagonal)
        glm::vec4 direction(0.0f);

        if(!fim_jogo){
            // Verifica se 5 segundos se passaram
            if (current_time - lastSpawnTime >= RandomFloat(2.0f, 4.0f))
            {
                std::lock_guard<std::mutex> lock(spawnMutex);
                // Chame a função SpawnTarget
                SpawnTarget();

                // Atualize o tempo da última chamada
                lastSpawnTime = current_time;
            }
            if (current_time - lastSpawnTime2 >= RandomFloat(6.0f, 10.0f))
            {
                std::lock_guard<std::mutex> lock(spawnMutex);
                // Chame a função SpawnTarget
                SpawnTarget_mov();

                // Atualize o tempo da última chamada
                lastSpawnTime2 = current_time;
            }
        }

        if(PRESS_W)
            direction += -w_vector;
        if(PRESS_S)
            direction += w_vector;
        if(PRESS_A)
            direction += -u_vector;
        if(PRESS_D)
            direction += u_vector;
            
        // Normaliza a direção do movimento
        if ((norm(direction) * norm(direction)) > 0.0f) {
            direction /= norm(direction);
        }
        direction.y = 0.0f;

        // Atualiza a posição da câmera, considerando o botão shift
        if(!PRESS_SHIFT){
            camera_position_c += camera_speed * delta_t * direction;
        }
        else{
            camera_position_c += (camera_speed/2.0f) * delta_t * direction;
        }

        if(CheckCollisionWithWorld(camera_position_c)){
            camera_position_c -= camera_speed * delta_t * direction;
        }
        if(PRESS_R){
            startTime = std::chrono::steady_clock::now();
            fim_jogo = false;
            jogador.resetScore();
        }
        
        if(g_LeftMouseButtonPressed){
            if (current_time - lastShotTime >= 0.5) {
                lastShotTime = current_time;
                HandleMouseClick(window, g_LastCursorPosX, g_LastCursorPosY, view, projection);
            }
        }

        // Cálculo vx,vy,vz e aplicação no view vector
        float vx = cos(g_CameraPhi) * sin(g_CameraTheta);
        float vy = sin(g_CameraPhi);
        float vz = cos(g_CameraTheta) * cos(g_CameraPhi);
        camera_view_vector = glm::vec4(-vx, vy, -vz, 0.0f);

        if (camera_position_c.y > ground_level)
        {
            camera_velocity.y -= gravity * delta_t;
            camera_position_c.y += camera_velocity.y * delta_t;
        }
        if (camera_position_c.y <= ground_level)
        {
            camera_position_c.y = ground_level;
            camera_velocity.y = 0;
        }
        if(!press_space && camera_position_c.y == ground_level){
            camera_velocity.y = 0;
        }

        // Atualização da posição da câmera
        float newPlayerX = camera_position_c.x + camera_velocity.x * delta_t;
        float newPlayerY = camera_position_c.y + camera_velocity.y * delta_t;
        float newPlayerZ = camera_position_c.z + camera_velocity.z * delta_t;

        // Imprimimos na tela informação sobre o número de quadros renderizados
        // por segundo (frames per second).
        TextRendering_ShowFramesPerSecond(window);

        model = Matrix_Identity();
        view = Matrix_Identity();
        projection = Matrix_Identity();

        glUniformMatrix4fv(g_view_uniform_crosshair, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(g_projection_uniform_crosshair, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(g_model_uniform_crosshair, 1, GL_FALSE, glm::value_ptr(model));
        glDisable(GL_DEPTH_TEST);
        drawCrosshair(g_GpuProgramID_crosshair);
        glEnable(GL_DEPTH_TEST);

        std::string scoreAtual = "Pontos: " + std::to_string(jogador.getScore());
        TextRendering_PrintString(window,scoreAtual,-0.95f,0.9f,3.0f);
        UpdateCountdown();
        std::string tempo_restante = "Tempo Restante: " + std::to_string(countdownTime);
        TextRendering_PrintString(window,tempo_restante,-0.95f,0.7f,3.0f);

        if(fim_jogo){
            std::string fim = "Fim de Jogo!\nPressione 'R' para reiniciar!";
            TextRendering_PrintString(window,fim,-0.95f,0.5f,3.0f);
        }
        // O framebuffer onde OpenGL executa as operações de renderização não
        // é o mesmo que está sendo mostrado para o usuário, caso contrário
        // seria possível ver artefatos conhecidos como "screen tearing". A
        // chamada abaixo faz a troca dos buffers, mostrando para o usuário
        // tudo que foi renderizado pelas funções acima.
        // Veja o link: https://en.wikipedia.org/w/index.php?title=Multiple_buffering&oldid=793452829#Double_buffering_in_computer_graphics
        glfwSwapBuffers(window);

        // Verificamos com o sistema operacional se houve alguma interação do
        // usuário (teclado, mouse, ...). Caso positivo, as funções de callback
        // definidas anteriormente usando glfwSet*Callback() serão chamadas
        // pela biblioteca GLFW.f
        glfwPollEvents();
    }

    // Finalizamos o uso dos recursos do sistema operacional
    glfwTerminate();

    // Fim do programa
    return 0;
}
// Função que carrega uma imagem para ser utilizada como textura
void LoadTextureImage(const char* filename)
{
    printf("Carregando imagem \"%s\"... ", filename);

    // Primeiro fazemos a leitura da imagem do disco
    stbi_set_flip_vertically_on_load(true);
    int width;
    int height;
    int channels;
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 3);

    if (data == NULL)
    {
        fprintf(stderr, "ERROR: Cannot open image file \"%s\".\n", filename);
        std::exit(EXIT_FAILURE);
    }

    printf("OK (%dx%d).\n", width, height);

    // Agora criamos objetos na GPU com OpenGL para armazenar a textura
    GLuint texture_id;
    GLuint sampler_id;
    glGenTextures(1, &texture_id);
    glGenSamplers(1, &sampler_id);

    // Veja slides 95-96 do documento Aula_20_Mapeamento_de_Texturas.pdf
    glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Parâmetros de amostragem da textura.
    glSamplerParameteri(sampler_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glSamplerParameteri(sampler_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Agora enviamos a imagem lida do disco para a GPU
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

    GLuint textureunit = g_NumLoadedTextures;
    glActiveTexture(GL_TEXTURE0 + textureunit);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindSampler(textureunit, sampler_id);

    stbi_image_free(data);

    g_NumLoadedTextures += 1;
}


// Função que desenha um objeto armazenado em g_VirtualScene. Veja definição
// dos objetos na função BuildTrianglesAndAddToVirtualScene().
void DrawVirtualObject(const char* object_name)
{
    // "Ligamos" o VAO. Informamos que queremos utilizar os atributos de
    // vértices apontados pelo VAO criado pela função BuildTrianglesAndAddToVirtualScene(). Veja
    // comentários detalhados dentro da definição de BuildTrianglesAndAddToVirtualScene().
    glBindVertexArray(g_VirtualScene[object_name].vertex_array_object_id);

    // Setamos as variáveis "bbox_min" e "bbox_max" do fragment shader
   // com os parâmetros da axis-aligned bounding box (AABB) do modelo.
    glm::vec3 bbox_min = g_VirtualScene[object_name].bbox_min;
    glm::vec3 bbox_max = g_VirtualScene[object_name].bbox_max;
    glUniform4f(g_bbox_min_uniform, bbox_min.x, bbox_min.y, bbox_min.z, 1.0f);
    glUniform4f(g_bbox_max_uniform, bbox_max.x, bbox_max.y, bbox_max.z, 1.0f);
    // Pedimos para a GPU rasterizar os vértices dos eixos XYZ
    // apontados pelo VAO como linhas. Veja a definição de
    // g_VirtualScene[""] dentro da função BuildTrianglesAndAddToVirtualScene(), e veja
    // a documentação da função glDrawElements() em
    // http://docs.gl/gl3/glDrawElements.
    glDrawElements(
        g_VirtualScene[object_name].rendering_mode,
        g_VirtualScene[object_name].num_indices,
        GL_UNSIGNED_INT,
        (void*)(g_VirtualScene[object_name].first_index * sizeof(GLuint))
    );

    // "Desligamos" o VAO, evitando assim que operações posteriores venham a
    // alterar o mesmo. Isso evita bugs.
    glBindVertexArray(0);
}

// Função que carrega os shaders de vértices e de fragmentos que serão
// utilizados para renderização. Veja slides 180-200 do documento Aula_03_Rendering_Pipeline_Grafico.pdf.
//
void LoadShadersFromFiles()
{
    // Note que o caminho para os arquivos "shader_vertex.glsl" e
    // "shader_fragment.glsl" estão fixados, sendo que assumimos a existência
    // da seguinte estrutura no sistema de arquivos:
    //
    //    + FCG_Lab_01/
    //    |
    //    +--+ bin/
    //    |  |
    //    |  +--+ Release/  (ou Debug/ ou Linux/)
    //    |     |
    //    |     o-- main.exe
    //    |
    //    +--+ src/
    //       |
    //       o-- shader_vertex.glsl
    //       |
    //       o-- shader_fragment.glsl
    //
    GLuint vertex_shader_id = LoadShader_Vertex("../../src/shader_vertex.glsl");
    GLuint fragment_shader_id = LoadShader_Fragment("../../src/shader_fragment-tarefa1.glsl");

    // Deletamos o programa de GPU anterior, caso ele exista.
    if ( g_GpuProgramID_obj != 0 )
        glDeleteProgram(g_GpuProgramID_obj);

    // Criamos um programa de GPU utilizando os shaders carregados acima.
    g_GpuProgramID_obj = CreateGpuProgram(vertex_shader_id, fragment_shader_id);

    // Buscamos o endereço das variáveis definidas dentro do Vertex Shader.
    // Utilizaremos estas variáveis para enviar dados para a placa de vídeo
    // (GPU)! Veja arquivo "shader_vertex.glsl" e "shader_fragment.glsl".
    g_model_uniform      = glGetUniformLocation(g_GpuProgramID_obj, "model"); // Variável da matriz "model"
    g_view_uniform       = glGetUniformLocation(g_GpuProgramID_obj, "view"); // Variável da matriz "view" em shader_vertex.glsl
    g_projection_uniform = glGetUniformLocation(g_GpuProgramID_obj, "projection"); // Variável da matriz "projection" em shader_vertex.glsl
    g_object_id_uniform  = glGetUniformLocation(g_GpuProgramID_obj, "object_id"); // Variável "object_id" em shader_fragment.glsl
    g_bbox_min_uniform = glGetUniformLocation(g_GpuProgramID_obj, "bbox_min");
    g_bbox_max_uniform = glGetUniformLocation(g_GpuProgramID_obj, "bbox_max");
    

    // Variáveis em "shader_fragment.glsl" para acesso das imagens de textura
    glUseProgram(g_GpuProgramID_obj);
    glUniform1i(glGetUniformLocation(g_GpuProgramID_obj, "TextureImage0"), 0);
    glUniform1i(glGetUniformLocation(g_GpuProgramID_obj, "TextureImage1"), 1);
    glUniform1i(glGetUniformLocation(g_GpuProgramID_obj, "TextureImage2"), 2);
    glUniform1i(glGetUniformLocation(g_GpuProgramID_obj, "TextureImage3"), 3);
    glUniform1i(glGetUniformLocation(g_GpuProgramID_obj, "TextureImage4"), 4);
    glUniform1i(glGetUniformLocation(g_GpuProgramID_obj, "TextureImage5"), 5);
    glUseProgram(0);
}

// Função que pega a matriz M e guarda a mesma no topo da pilha
void PushMatrix(glm::mat4 M)
{
    g_MatrixStack.push(M);
}

// Função que remove a matriz atualmente no topo da pilha e armazena a mesma na variável M
void PopMatrix(glm::mat4& M)
{
    if ( g_MatrixStack.empty() )
    {
        M = Matrix_Identity();
    }
    else
    {
        M = g_MatrixStack.top();
        g_MatrixStack.pop();
    }
}

// Função que computa as normais de um ObjModel, caso elas não tenham sido
// especificadas dentro do arquivo ".obj"
void ComputeNormals(ObjModel* model)
{
    if ( !model->attrib.normals.empty() )
        return;

    // Primeiro computamos as normais para todos os TRIÂNGULOS.
    // Segundo, computamos as normais dos VÉRTICES através do método proposto
    // por Gouraud, onde a normal de cada vértice vai ser a média das normais de
    // todas as faces que compartilham este vértice.

    size_t num_vertices = model->attrib.vertices.size() / 3;

    std::vector<int> num_triangles_per_vertex(num_vertices, 0);
    std::vector<glm::vec4> vertex_normals(num_vertices, glm::vec4(0.0f,0.0f,0.0f,0.0f));

    for (size_t shape = 0; shape < model->shapes.size(); ++shape)
    {
        size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

        for (size_t triangle = 0; triangle < num_triangles; ++triangle)
        {
            assert(model->shapes[shape].mesh.num_face_vertices[triangle] == 3);

            glm::vec4  vertices[3];
            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];
                const float vx = model->attrib.vertices[3*idx.vertex_index + 0];
                const float vy = model->attrib.vertices[3*idx.vertex_index + 1];
                const float vz = model->attrib.vertices[3*idx.vertex_index + 2];
                vertices[vertex] = glm::vec4(vx,vy,vz,1.0);
            }

            const glm::vec4  a = vertices[0];
            const glm::vec4  b = vertices[1];
            const glm::vec4  c = vertices[2];

            // PREENCHA AQUI o cálculo da normal de um triângulo cujos vértices
            // estão nos pontos "a", "b", e "c", definidos no sentido anti-horário.
            const glm::vec4 ac = c - a;
            const glm::vec4 ab = b - a;
            const glm::vec4  n = crossproduct(ab, ac);

            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];
                num_triangles_per_vertex[idx.vertex_index] += 1;
                vertex_normals[idx.vertex_index] += n;
                model->shapes[shape].mesh.indices[3*triangle + vertex].normal_index = idx.vertex_index;
            }
        }
    }

    model->attrib.normals.resize( 3*num_vertices );

    for (size_t i = 0; i < vertex_normals.size(); ++i)
    {
        glm::vec4 n = vertex_normals[i] / (float)num_triangles_per_vertex[i];
        n /= norm(n);
        model->attrib.normals[3*i + 0] = n.x;
        model->attrib.normals[3*i + 1] = n.y;
        model->attrib.normals[3*i + 2] = n.z;
    }
}

// Constrói triângulos para futura renderização a partir de um ObjModel.
void BuildTrianglesAndAddToVirtualScene(ObjModel* model)
{
    GLuint vertex_array_object_id;
    glGenVertexArrays(1, &vertex_array_object_id);
    glBindVertexArray(vertex_array_object_id);

    std::vector<GLuint> indices;
    std::vector<float>  model_coefficients;
    std::vector<float>  normal_coefficients;
    std::vector<float>  texture_coefficients;

    for (size_t shape = 0; shape < model->shapes.size(); ++shape)
    {
        size_t first_index = indices.size();
        size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();
        const float minval = std::numeric_limits<float>::min();
        const float maxval = std::numeric_limits<float>::max();

        glm::vec3 bbox_min = glm::vec3(maxval, maxval, maxval);
        glm::vec3 bbox_max = glm::vec3(minval, minval, minval);

        for (size_t triangle = 0; triangle < num_triangles; ++triangle)
        {
            assert(model->shapes[shape].mesh.num_face_vertices[triangle] == 3);

            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];

                indices.push_back(first_index + 3*triangle + vertex);

                const float vx = model->attrib.vertices[3*idx.vertex_index + 0];
                const float vy = model->attrib.vertices[3*idx.vertex_index + 1];
                const float vz = model->attrib.vertices[3*idx.vertex_index + 2];
                //printf("tri %d vert %d = (%.2f, %.2f, %.2f)\n", (int)triangle, (int)vertex, vx, vy, vz);
                model_coefficients.push_back( vx ); // X
                model_coefficients.push_back( vy ); // Y
                model_coefficients.push_back( vz ); // Z
                model_coefficients.push_back( 1.0f ); // W

                bbox_min.x = std::min(bbox_min.x, vx);
                bbox_min.y = std::min(bbox_min.y, vy);
                bbox_min.z = std::min(bbox_min.z, vz);
                bbox_max.x = std::max(bbox_max.x, vx);
                bbox_max.y = std::max(bbox_max.y, vy);
                bbox_max.z = std::max(bbox_max.z, vz);

                // Inspecionando o código da tinyobjloader, o aluno Bernardo
                // Sulzbach (2017/1) apontou que a maneira correta de testar se
                // existem normais e coordenadas de textura no ObjModel é
                // comparando se o índice retornado é -1. Fazemos isso abaixo.

                if ( idx.normal_index != -1 )
                {
                    const float nx = model->attrib.normals[3*idx.normal_index + 0];
                    const float ny = model->attrib.normals[3*idx.normal_index + 1];
                    const float nz = model->attrib.normals[3*idx.normal_index + 2];
                    normal_coefficients.push_back( nx ); // X
                    normal_coefficients.push_back( ny ); // Y
                    normal_coefficients.push_back( nz ); // Z
                    normal_coefficients.push_back( 0.0f ); // W
                }

                if ( idx.texcoord_index != -1 )
                {
                    const float u = model->attrib.texcoords[2*idx.texcoord_index + 0];
                    const float v = model->attrib.texcoords[2*idx.texcoord_index + 1];
                    texture_coefficients.push_back( u );
                    texture_coefficients.push_back( v );
                }
            }
        }

        size_t last_index = indices.size() - 1;

        SceneObject theobject;
        theobject.name           = model->shapes[shape].name;
        theobject.first_index    = first_index; // Primeiro índice
        theobject.num_indices    = last_index - first_index + 1; // Número de indices
        theobject.rendering_mode = GL_TRIANGLES;       // Índices correspondem ao tipo de rasterização GL_TRIANGLES.
        theobject.vertex_array_object_id = vertex_array_object_id;
        theobject.bbox_min = bbox_min;
        theobject.bbox_max = bbox_max;

        g_VirtualScene[model->shapes[shape].name] = theobject;
    }

    GLuint VBO_model_coefficients_id;
    glGenBuffers(1, &VBO_model_coefficients_id);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_model_coefficients_id);
    glBufferData(GL_ARRAY_BUFFER, model_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, model_coefficients.size() * sizeof(float), model_coefficients.data());
    GLuint location = 0; // "(location = 0)" em "shader_vertex.glsl"
    GLint  number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
    glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if ( !normal_coefficients.empty() )
    {
        GLuint VBO_normal_coefficients_id;
        glGenBuffers(1, &VBO_normal_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_normal_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, normal_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, normal_coefficients.size() * sizeof(float), normal_coefficients.data());
        location = 1; // "(location = 1)" em "shader_vertex.glsl"
        number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    if ( !texture_coefficients.empty() )
    {
        GLuint VBO_texture_coefficients_id;
        glGenBuffers(1, &VBO_texture_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_texture_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, texture_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, texture_coefficients.size() * sizeof(float), texture_coefficients.data());
        location = 2; // "(location = 1)" em "shader_vertex.glsl"
        number_of_dimensions = 2; // vec2 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    GLuint indices_id;
    glGenBuffers(1, &indices_id);

    // "Ligamos" o buffer. Note que o tipo agora é GL_ELEMENT_ARRAY_BUFFER.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(GLuint), indices.data());
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // XXX Errado!
    //

    // "Desligamos" o VAO, evitando assim que operações posteriores venham a
    // alterar o mesmo. Isso evita bugs.
    glBindVertexArray(0);
}

// Carrega um Vertex Shader de um arquivo GLSL. Veja definição de LoadShader() abaixo.
GLuint LoadShader_Vertex(const char* filename)
{
    // Criamos um identificador (ID) para este shader, informando que o mesmo
    // será aplicado nos vértices.
    GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);

    // Carregamos e compilamos o shader
    LoadShader(filename, vertex_shader_id);

    // Retorna o ID gerado acima
    return vertex_shader_id;
}

// Carrega um Fragment Shader de um arquivo GLSL . Veja definição de LoadShader() abaixo.
GLuint LoadShader_Fragment(const char* filename)
{
    // Criamos um identificador (ID) para este shader, informando que o mesmo
    // será aplicado nos fragmentos.
    GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

    // Carregamos e compilamos o shader
    LoadShader(filename, fragment_shader_id);

    // Retorna o ID gerado acima
    return fragment_shader_id;
}

// Função auxilar, utilizada pelas duas funções acima. Carrega código de GPU de
// um arquivo GLSL e faz sua compilação.
void LoadShader(const char* filename, GLuint shader_id)
{
    // Lemos o arquivo de texto indicado pela variável "filename"
    // e colocamos seu conteúdo em memória, apontado pela variável
    // "shader_string".
    std::ifstream file;
    try {
        file.exceptions(std::ifstream::failbit);
        file.open(filename);
    } catch ( std::exception& e ) {
        fprintf(stderr, "ERROR: Cannot open file \"%s\".\n", filename);
        std::exit(EXIT_FAILURE);
    }
    std::stringstream shader;
    shader << file.rdbuf();
    std::string str = shader.str();
    const GLchar* shader_string = str.c_str();
    const GLint   shader_string_length = static_cast<GLint>( str.length() );

    // Define o código do shader GLSL, contido na string "shader_string"
    glShaderSource(shader_id, 1, &shader_string, &shader_string_length);

    // Compila o código do shader GLSL (em tempo de execução)
    glCompileShader(shader_id);

    // Verificamos se ocorreu algum erro ou "warning" durante a compilação
    GLint compiled_ok;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compiled_ok);

    GLint log_length = 0;
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);

    // Alocamos memória para guardar o log de compilação.
    // A chamada "new" em C++ é equivalente ao "malloc()" do C.
    GLchar* log = new GLchar[log_length];
    glGetShaderInfoLog(shader_id, log_length, &log_length, log);

    // Imprime no terminal qualquer erro ou "warning" de compilação
    if ( log_length != 0 )
    {
        std::string  output;

        if ( !compiled_ok )
        {
            output += "ERROR: OpenGL compilation of \"";
            output += filename;
            output += "\" failed.\n";
            output += "== Start of compilation log\n";
            output += log;
            output += "== End of compilation log\n";
        }
        else
        {
            output += "WARNING: OpenGL compilation of \"";
            output += filename;
            output += "\".\n";
            output += "== Start of compilation log\n";
            output += log;
            output += "== End of compilation log\n";
        }

        fprintf(stderr, "%s", output.c_str());
    }

    // A chamada "delete" em C++ é equivalente ao "free()" do C
    delete [] log;
}

// Esta função cria um programa de GPU, o qual contém obrigatoriamente um
// Vertex Shader e um Fragment Shader.
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id)
{
    // Criamos um identificador (ID) para este programa de GPU
    GLuint program_id = glCreateProgram();

    // Definição dos dois shaders GLSL que devem ser executados pelo programa
    glAttachShader(program_id, vertex_shader_id);
    glAttachShader(program_id, fragment_shader_id);

    // Linkagem dos shaders acima ao programa
    glLinkProgram(program_id);

    // Verificamos se ocorreu algum erro durante a linkagem
    GLint linked_ok = GL_FALSE;
    glGetProgramiv(program_id, GL_LINK_STATUS, &linked_ok);

    // Imprime no terminal qualquer erro de linkagem
    if ( linked_ok == GL_FALSE )
    {
        GLint log_length = 0;
        glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_length);

        // Alocamos memória para guardar o log de compilação.
        // A chamada "new" em C++ é equivalente ao "malloc()" do C.
        GLchar* log = new GLchar[log_length];

        glGetProgramInfoLog(program_id, log_length, &log_length, log);

        std::string output;

        output += "ERROR: OpenGL linking of program failed.\n";
        output += "== Start of link log\n";
        output += log;
        output += "\n== End of link log\n";

        // A chamada "delete" em C++ é equivalente ao "free()" do C
        delete [] log;

        fprintf(stderr, "%s", output.c_str());
    }

    // Os "Shader Objects" podem ser marcados para deleção após serem linkados 
    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);

    // Retornamos o ID gerado acima
    return program_id;
}

// Definição da função que será chamada sempre que a janela do sistema
// operacional for redimensionada, por consequência alterando o tamanho do
// "framebuffer" (região de memória onde são armazenados os pixels da imagem).
void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    // Indicamos que queremos renderizar em toda região do framebuffer. A
    // função "glViewport" define o mapeamento das "normalized device
    // coordinates" (NDC) para "pixel coordinates".  Essa é a operação de
    // "Screen Mapping" ou "Viewport Mapping" vista em aula ({+ViewportMapping2+}).
    glViewport(0, 0, width, height);

    // Atualizamos também a razão que define a proporção da janela (largura /
    // altura), a qual será utilizada na definição das matrizes de projeção,
    // tal que não ocorra distorções durante o processo de "Screen Mapping"
    // acima, quando NDC é mapeado para coordenadas de pixels. Veja slides 205-215 do documento Aula_09_Projecoes.pdf.
    //
    // O cast para float é necessário pois números inteiros são arredondados ao
    // serem divididos!
    g_ScreenRatio = (float)width / height;
}

// Função callback chamada sempre que o usuário aperta algum dos botões do mouse
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        // Se o usuário pressionou o botão esquerdo do mouse, guardamos a
        // posição atual do cursor nas variáveis g_LastCursorPosX e
        // g_LastCursorPosY.  Também, setamos a variável
        // g_LeftMouseButtonPressed como true, para saber que o usuário está
        // com o botão esquerdo pressionado.
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_LeftMouseButtonPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        // Quando o usuário soltar o botão esquerdo do mouse, atualizamos a
        // variável abaixo para false.
        g_LeftMouseButtonPressed = false;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        // Se o usuário pressionou o botão esquerdo do mouse, guardamos a
        // posição atual do cursor nas variáveis g_LastCursorPosX e
        // g_LastCursorPosY.  Também, setamos a variável
        // g_RightMouseButtonPressed como true, para saber que o usuário está
        // com o botão esquerdo pressionado.
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_RightMouseButtonPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
    {
        // Quando o usuário soltar o botão esquerdo do mouse, atualizamos a
        // variável abaixo para false.
        g_RightMouseButtonPressed = false;
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS)
    {
        // Se o usuário pressionou o botão esquerdo do mouse, guardamos a
        // posição atual do cursor nas variáveis g_LastCursorPosX e
        // g_LastCursorPosY.  Também, setamos a variável
        // g_MiddleMouseButtonPressed como true, para saber que o usuário está
        // com o botão esquerdo pressionado.
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_MiddleMouseButtonPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE)
    {
        // Quando o usuário soltar o botão esquerdo do mouse, atualizamos a
        // variável abaixo para false.
        g_MiddleMouseButtonPressed = false;
    }
}

// Função callback chamada sempre que o usuário movimentar o cursor do mouse em
// cima da janela OpenGL.
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    // Abaixo executamos o seguinte: caso o botão esquerdo do mouse esteja
    // pressionado, computamos quanto que o mouse se movimento desde o último
    // instante de tempo, e usamos esta movimentação para atualizar os
    // parâmetros que definem a posição da câmera dentro da cena virtual.
    // Assim, temos que o usuário consegue controlar a câmera.

    //if (g_LeftMouseButtonPressed)
    //{
        // Deslocamento do cursor do mouse em x e y de coordenadas de tela!
        float dx = xpos - g_LastCursorPosX;
        float dy = ypos - g_LastCursorPosY;

        // Atualizamos parâmetros da câmera com os deslocamentos
        g_CameraTheta -= 0.01f * dx;
        g_CameraPhi -= 0.01f * dy;

        // Em coordenadas esféricas, o ângulo phi deve ficar entre -pi/2 e +pi/2.
        float phimax = 3.141592f / 2;
        float phimin = -phimax;

        if (g_CameraPhi > phimax)
            g_CameraPhi = phimax;

        if (g_CameraPhi < phimin)
            g_CameraPhi = phimin;

        // Atualizamos as variáveis globais para armazenar a posição atual do
        // cursor como sendo a última posição conhecida do cursor.
        g_LastCursorPosX = xpos;
        g_LastCursorPosY = ypos;
    //}
}

// Função callback chamada sempre que o usuário movimenta a "rodinha" do mouse.
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    // Atualizamos a distância da câmera para a origem utilizando a
    // movimentação da "rodinha", simulando um ZOOM.
    g_CameraDistance -= 0.1f*yoffset;

    // Uma câmera look-at nunca pode estar exatamente "em cima" do ponto para
    // onde ela está olhando, pois isto gera problemas de divisão por zero na
    // definição do sistema de coordenadas da câmera. Isto é, a variável abaixo
    // nunca pode ser zero. Versões anteriores deste código possuíam este bug,
    // o qual foi detectado pelo aluno Vinicius Fraga (2017/2).
    const float verysmallnumber = std::numeric_limits<float>::epsilon();
    if (g_CameraDistance < verysmallnumber)
        g_CameraDistance = verysmallnumber;
}

// Definição da função que será chamada sempre que o usuário pressionar alguma
// tecla do teclado. Veja http://www.glfw.org/docs/latest/input_guide.html#input_key
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mod)
{
    // ====================
    // Não modifique este loop! Ele é utilizando para correção automatizada dos
    // laboratórios. Deve ser sempre o primeiro comando desta função KeyCallback().
    for (int i = 0; i < 10; ++i)
        if (key == GLFW_KEY_0 + i && action == GLFW_PRESS && mod == GLFW_MOD_SHIFT)
            std::exit(100 + i);
    // ====================

    // Se o usuário pressionar a tecla ESC, fechamos a janela.
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    // Troca a câmera pra 3a pessoa (lookat)
    if (key == GLFW_KEY_L && action == GLFW_PRESS)
    {
        g_ThirdPersonCamera = !g_ThirdPersonCamera;
    }

    // Se o usuário apertar a tecla espaço, resetamos os ângulos de Euler para zero.
    if (key == GLFW_KEY_SPACE)
    {
        if (action == GLFW_PRESS)
            press_space = true;
        else if (action == GLFW_RELEASE)
            press_space = false;
    }

    //controles de movimento da camera
    if (key == GLFW_KEY_W && action == GLFW_PRESS)
    {
        PRESS_W = true;
    }
    if (key == GLFW_KEY_A && action == GLFW_PRESS)
    {
        PRESS_A = true;
    }
    if (key == GLFW_KEY_S && action == GLFW_PRESS)
    {
        PRESS_S = true;
    }
    if (key == GLFW_KEY_D && action == GLFW_PRESS)
    {
        PRESS_D = true;
    }
    if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS)
    {
        PRESS_SHIFT = true;
    }
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        PRESS_R = true;
    }
    if (key == GLFW_KEY_F && action == GLFW_PRESS)
    {
        g_Fullscreen = !g_Fullscreen;

        if (g_Fullscreen)
        {
            // Save the current window position and size
            glfwGetWindowPos(window, &g_WindowPosX, &g_WindowPosY);
            glfwGetWindowSize(window, &g_WindowWidth, &g_WindowHeight);

            // Get the primary monitor and its video mode
            g_Monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode* mode = glfwGetVideoMode(g_Monitor);

            // Set the window to fullscreen
            glfwSetWindowMonitor(window, g_Monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        }
        else
        {
            // Restore the window to its previous position and size
            glfwSetWindowMonitor(window, nullptr, g_WindowPosX, g_WindowPosY, g_WindowWidth, g_WindowHeight, 0);
        }
    }

    //ve se libera o movimento
    if (key == GLFW_KEY_W && action == GLFW_RELEASE)
    {
        PRESS_W = false;
    }
    if (key == GLFW_KEY_A && action == GLFW_RELEASE)
    {
        PRESS_A = false;
    }
    if (key == GLFW_KEY_S && action == GLFW_RELEASE)
    {
        PRESS_S = false;
    }
    if (key == GLFW_KEY_D && action == GLFW_RELEASE)
    {
        PRESS_D = false;
    }
    if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE)
    {
        PRESS_SHIFT = false;
    }
    if (key == GLFW_KEY_R && action == GLFW_RELEASE)
    {
        PRESS_R = false;
    }
}

// Definimos o callback para impressão de erros da GLFW no terminal
void ErrorCallback(int error, const char* description)
{
    fprintf(stderr, "ERROR: GLFW: %s\n", description);
}

// Escrevemos na tela o número de quadros renderizados por segundo (frames per
// second).
void TextRendering_ShowFramesPerSecond(GLFWwindow* window)
{

    // Variáveis estáticas (static) mantém seus valores entre chamadas
    // subsequentes da função!
    static float old_seconds = (float)glfwGetTime();
    static int   ellapsed_frames = 0;
    static char  buffer[20] = "?? fps";
    static int   numchars = 7;

    ellapsed_frames += 1;

    // Recuperamos o número de segundos que passou desde a execução do programa
    float seconds = (float)glfwGetTime();

    // Número de segundos desde o último cálculo do fps
    float ellapsed_seconds = seconds - old_seconds;

    if ( ellapsed_seconds > 1.0f )
    {
        numchars = snprintf(buffer, 20, "%.2f fps", ellapsed_frames / ellapsed_seconds);
    
        old_seconds = seconds;
        ellapsed_frames = 0;
    }

    float lineheight = TextRendering_LineHeight(window);
    float charwidth = TextRendering_CharWidth(window);

    TextRendering_PrintString(window, buffer, 1.0f-(numchars + 1)*charwidth, 1.0f-lineheight, 1.0f);
}

// Função para debugging: imprime no terminal todas informações de um modelo
// geométrico carregado de um arquivo ".obj".
// Veja: https://github.com/syoyo/tinyobjloader/blob/22883def8db9ef1f3ffb9b404318e7dd25fdbb51/loader_example.cc#L98
void PrintObjModelInfo(ObjModel* model)
{
  const tinyobj::attrib_t                & attrib    = model->attrib;
  const std::vector<tinyobj::shape_t>    & shapes    = model->shapes;
  const std::vector<tinyobj::material_t> & materials = model->materials;

  printf("# of vertices  : %d\n", (int)(attrib.vertices.size() / 3));
  printf("# of normals   : %d\n", (int)(attrib.normals.size() / 3));
  printf("# of texcoords : %d\n", (int)(attrib.texcoords.size() / 2));
  printf("# of shapes    : %d\n", (int)shapes.size());
  printf("# of materials : %d\n", (int)materials.size());

  for (size_t v = 0; v < attrib.vertices.size() / 3; v++) {
    printf("  v[%ld] = (%f, %f, %f)\n", static_cast<long>(v),
           static_cast<const double>(attrib.vertices[3 * v + 0]),
           static_cast<const double>(attrib.vertices[3 * v + 1]),
           static_cast<const double>(attrib.vertices[3 * v + 2]));
  }

  for (size_t v = 0; v < attrib.normals.size() / 3; v++) {
    printf("  n[%ld] = (%f, %f, %f)\n", static_cast<long>(v),
           static_cast<const double>(attrib.normals[3 * v + 0]),
           static_cast<const double>(attrib.normals[3 * v + 1]),
           static_cast<const double>(attrib.normals[3 * v + 2]));
  }

  for (size_t v = 0; v < attrib.texcoords.size() / 2; v++) {
    printf("  uv[%ld] = (%f, %f)\n", static_cast<long>(v),
           static_cast<const double>(attrib.texcoords[2 * v + 0]),
           static_cast<const double>(attrib.texcoords[2 * v + 1]));
  }

  // For each shape
  for (size_t i = 0; i < shapes.size(); i++) {
    printf("shape[%ld].name = %s\n", static_cast<long>(i),
           shapes[i].name.c_str());
    printf("Size of shape[%ld].indices: %lu\n", static_cast<long>(i),
           static_cast<unsigned long>(shapes[i].mesh.indices.size()));

    size_t index_offset = 0;

    assert(shapes[i].mesh.num_face_vertices.size() ==
           shapes[i].mesh.material_ids.size());

    printf("shape[%ld].num_faces: %lu\n", static_cast<long>(i),
           static_cast<unsigned long>(shapes[i].mesh.num_face_vertices.size()));

    // For each face
    for (size_t f = 0; f < shapes[i].mesh.num_face_vertices.size(); f++) {
      size_t fnum = shapes[i].mesh.num_face_vertices[f];

      printf("  face[%ld].fnum = %ld\n", static_cast<long>(f),
             static_cast<unsigned long>(fnum));

      // For each vertex in the face
      for (size_t v = 0; v < fnum; v++) {
        tinyobj::index_t idx = shapes[i].mesh.indices[index_offset + v];
        printf("    face[%ld].v[%ld].idx = %d/%d/%d\n", static_cast<long>(f),
               static_cast<long>(v), idx.vertex_index, idx.normal_index,
               idx.texcoord_index);
      }

      printf("  face[%ld].material_id = %d\n", static_cast<long>(f),
             shapes[i].mesh.material_ids[f]);

      index_offset += fnum;
    }

    printf("shape[%ld].num_tags: %lu\n", static_cast<long>(i),
           static_cast<unsigned long>(shapes[i].mesh.tags.size()));
    for (size_t t = 0; t < shapes[i].mesh.tags.size(); t++) {
      printf("  tag[%ld] = %s ", static_cast<long>(t),
             shapes[i].mesh.tags[t].name.c_str());
      printf(" ints: [");
      for (size_t j = 0; j < shapes[i].mesh.tags[t].intValues.size(); ++j) {
        printf("%ld", static_cast<long>(shapes[i].mesh.tags[t].intValues[j]));
        if (j < (shapes[i].mesh.tags[t].intValues.size() - 1)) {
          printf(", ");
        }
      }
      printf("]");

      printf(" floats: [");
      for (size_t j = 0; j < shapes[i].mesh.tags[t].floatValues.size(); ++j) {
        printf("%f", static_cast<const double>(
                         shapes[i].mesh.tags[t].floatValues[j]));
        if (j < (shapes[i].mesh.tags[t].floatValues.size() - 1)) {
          printf(", ");
        }
      }
      printf("]");

      printf(" strings: [");
      for (size_t j = 0; j < shapes[i].mesh.tags[t].stringValues.size(); ++j) {
        printf("%s", shapes[i].mesh.tags[t].stringValues[j].c_str());
        if (j < (shapes[i].mesh.tags[t].stringValues.size() - 1)) {
          printf(", ");
        }
      }
      printf("]");
      printf("\n");
    }
  }

  for (size_t i = 0; i < materials.size(); i++) {
    printf("material[%ld].name = %s\n", static_cast<long>(i),
           materials[i].name.c_str());
    printf("  material.Ka = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].ambient[0]),
           static_cast<const double>(materials[i].ambient[1]),
           static_cast<const double>(materials[i].ambient[2]));
    printf("  material.Kd = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].diffuse[0]),
           static_cast<const double>(materials[i].diffuse[1]),
           static_cast<const double>(materials[i].diffuse[2]));
    printf("  material.Ks = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].specular[0]),
           static_cast<const double>(materials[i].specular[1]),
           static_cast<const double>(materials[i].specular[2]));
    printf("  material.Tr = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].transmittance[0]),
           static_cast<const double>(materials[i].transmittance[1]),
           static_cast<const double>(materials[i].transmittance[2]));
    printf("  material.Ke = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].emission[0]),
           static_cast<const double>(materials[i].emission[1]),
           static_cast<const double>(materials[i].emission[2]));
    printf("  material.Ns = %f\n",
           static_cast<const double>(materials[i].shininess));
    printf("  material.Ni = %f\n", static_cast<const double>(materials[i].ior));
    printf("  material.dissolve = %f\n",
           static_cast<const double>(materials[i].dissolve));
    printf("  material.illum = %d\n", materials[i].illum);
    printf("  material.map_Ka = %s\n", materials[i].ambient_texname.c_str());
    printf("  material.map_Kd = %s\n", materials[i].diffuse_texname.c_str());
    printf("  material.map_Ks = %s\n", materials[i].specular_texname.c_str());
    printf("  material.map_Ns = %s\n",
           materials[i].specular_highlight_texname.c_str());
    printf("  material.map_bump = %s\n", materials[i].bump_texname.c_str());
    printf("  material.map_d = %s\n", materials[i].alpha_texname.c_str());
    printf("  material.disp = %s\n", materials[i].displacement_texname.c_str());
    printf("  <<PBR>>\n");
    printf("  material.Pr     = %f\n", materials[i].roughness);
    printf("  material.Pm     = %f\n", materials[i].metallic);
    printf("  material.Ps     = %f\n", materials[i].sheen);
    printf("  material.Pc     = %f\n", materials[i].clearcoat_thickness);
    printf("  material.Pcr    = %f\n", materials[i].clearcoat_thickness);
    printf("  material.aniso  = %f\n", materials[i].anisotropy);
    printf("  material.anisor = %f\n", materials[i].anisotropy_rotation);
    printf("  material.map_Ke = %s\n", materials[i].emissive_texname.c_str());
    printf("  material.map_Pr = %s\n", materials[i].roughness_texname.c_str());
    printf("  material.map_Pm = %s\n", materials[i].metallic_texname.c_str());
    printf("  material.map_Ps = %s\n", materials[i].sheen_texname.c_str());
    printf("  material.norm   = %s\n", materials[i].normal_texname.c_str());
    std::map<std::string, std::string>::const_iterator it(
        materials[i].unknown_parameter.begin());
    std::map<std::string, std::string>::const_iterator itEnd(
        materials[i].unknown_parameter.end());

    for (; it != itEnd; it++) {
      printf("  material.%s = %s\n", it->first.c_str(), it->second.c_str());
    }
    printf("\n");
  }
}

void LoadCrosshairShader() {
    GLuint vertex_shader_id = LoadShader_Vertex("../../src/crosshair_vertex_shader.glsl");
    GLuint fragment_shader_id = LoadShader_Fragment("../../src/crosshair_fragment_shader.glsl");

    // Deletamos o programa de GPU anterior, caso ele exista.
    if (g_GpuProgramID_crosshair != 0)
        glDeleteProgram(g_GpuProgramID_crosshair);

    g_GpuProgramID_crosshair = CreateGpuProgram(vertex_shader_id, fragment_shader_id);
    g_model_uniform_crosshair = glGetUniformLocation(g_GpuProgramID_crosshair, "model"); // Variável da matriz "model"
    g_view_uniform_crosshair = glGetUniformLocation(g_GpuProgramID_crosshair, "view"); // Variável da matriz "view" em shader_vertex.glsl
    g_projection_uniform_crosshair = glGetUniformLocation(g_GpuProgramID_crosshair, "projection"); // Variável da matriz "projection" em shader_vertex.glsl
}

void drawCrosshair(GLuint shaderProgram) {
    float crosshairVertices[] = {
        // Triângulo para a linha horizontal esquerda
        -0.025f,  0.0035f,
        -0.025f, -0.0035f,
        -0.005f, -0.0035f,
        -0.005f,  0.0035f,

         // Triângulo para a linha horizontal direita
          0.005f,  0.0035f,
          0.005f, -0.0035f,
          0.025f, -0.0035f,
          0.025f,  0.0035f,

          // Triângulo para a linha vertical superior
          -0.0025f,  0.007f,
           0.0025f,  0.007f,
           0.0025f,  0.035f,
          -0.0025f,  0.035f,

          // Triângulo para a linha vertical inferior
          -0.0025f, -0.035f,
           0.0025f, -0.035f,
           0.0025f,  -0.007f,
          -0.0025f,  -0.007f,
    };

    GLubyte indices[] = {
        // Índices para os triângulos da linha horizontal esquerda
        0, 1, 2,
        0, 2, 3,

        // Índices para os triângulos da linha horizontal direita
        4, 5, 6,
        4, 6, 7,

        // Índices para os triângulos da linha vertical superior
        8, 9, 10,
        8, 10, 11,

        // Índices para os triângulos da linha vertical inferior
        12, 13, 14,
        12, 14, 15,
    };

    // Criação de um VAO, VBO e EBO
    GLuint VBO_NDC_coefficients_id;
    glGenBuffers(1, &VBO_NDC_coefficients_id);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_NDC_coefficients_id);

    GLuint vertex_array_object_id;
    glGenVertexArrays(1, &vertex_array_object_id);
    glBindVertexArray(vertex_array_object_id);

    glBufferData(GL_ARRAY_BUFFER, sizeof(crosshairVertices), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(crosshairVertices), crosshairVertices);

    GLuint location = 0; // "(location = 0)" em "shader_vertex.glsl"
    GLint  number_of_dimensions = 2; // vec4 em "shader_vertex.glsl"
    glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(location);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLuint indices_id;
    glGenBuffers(1, &indices_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(indices), indices);

    glDrawElements(GL_TRIANGLES, 24, GL_UNSIGNED_BYTE, 0);

    glBindVertexArray(0);

}

// Função para renderizar o mapa
void RenderGameMap(GameMap& gameMap, glm::mat4 view, glm::mat4 projection) {
    glUseProgram(g_GpuProgramID_obj);
    glUniformMatrix4fv(g_view_uniform, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(g_projection_uniform, 1, GL_FALSE, glm::value_ptr(projection));

    std::vector<glm::mat4> modelos = gameMap.getModels();

    for(size_t i = 0; i < modelos.size(); ++i){
        glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(modelos[i]));

        if(i != 0){
            glUniform1i(g_object_id_uniform, PLANE_PAREDE);
        } else {
            glUniform1i(g_object_id_uniform, PLANE);
        }
        
        DrawVirtualObject("the_plane");
    }


    // Desative o shader program se necessário
    glUseProgram(0);
}

// Função para renderizar a arma
void RenderGun(glm::vec4 camera_up_vector, glm::vec4 camera_view_vector,glm::vec4 camera_position_c){
    // Desenhamos o modelo da arma
        glm::vec4 camera_right = glm::vec4(glm::normalize(glm::cross(glm::vec3(camera_up_vector), glm::vec3(camera_view_vector))), 0.0f); // Calcula a direção "direita" da câmera, com w = 0
        glm::vec4 object_position = glm::vec4(camera_position_c.x, camera_position_c.y - 0.5, camera_position_c.z,1.0f) + (-camera_right*0.2f); // Define a posição do objeto para a direita da câmera

        // Criar uma matriz de rotação para alinhar o objeto com a direção da câmera
        glm::vec3 forward = glm::normalize(glm::vec3(camera_view_vector)); // Direção para onde a câmera está apontando
        glm::vec3 right = glm::normalize(glm::cross(glm::vec3(camera_up_vector), forward)); // Vetor 'right' do objeto
        glm::vec3 up = glm::cross(forward, right); // Vetor 'up' do objeto alinhado com a câmera

        glm::mat4 rotation_matrix = glm::mat4(
            glm::vec4(right, 0.0f),
            glm::vec4(up, 0.0f),
            glm::vec4(forward, 0.0f), // Negativo para alinhar a frente do objeto com a direção da câmera
            glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
        );

        glm::mat4 model = Matrix_Identity() *
                            Matrix_Translate(object_position.x, object_position.y, object_position.z) *
                            rotation_matrix * // Aplica a rotação para alinhar com a direção da câmera
                            Matrix_Scale(0.025, 0.025, 0.025);

        glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, GUN);
        DrawVirtualObject("AWP");
}

void RenderPlayer(glm::vec4 camera_position_c, glm::vec4 camera_view_vector, glm::vec4 camera_up_vector) {
    // Desenhamos o modelo da arma
    glm::vec4 object_position = glm::vec4(camera_position_c.x, camera_position_c.y - 0.5, camera_position_c.z,1.0f);

    // Criar uma matriz de rotação para alinhar o objeto com a direção da câmera, mantendo o eixo Y constante
    glm::vec4 forward = glm::normalize(camera_view_vector);
    forward.y = 0.0f; // Manter o eixo Y constante
    forward = glm::normalize(forward); // Normalizar novamente após ajustar o eixo Y

    glm::vec4 right = crossproduct(camera_up_vector, forward); // Vetor 'right' do objeto
    glm::vec4 up = crossproduct(forward, right); // Vetor 'up' do objeto alinhado com a câmera

    glm::mat4 rotation_matrix = glm::mat4(
        glm::vec4(right),
        glm::vec4(up),
        glm::vec4(forward),
        glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
    );

    glm::mat4 model = Matrix_Identity() *
                        Matrix_Translate(object_position.x, object_position.y, object_position.z) *
                        rotation_matrix * // Aplica a rotação para alinhar com a direção da câmera
                        Matrix_Scale(0.01f, 0.01f, 0.01f);
    
    
    glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
    glUniform1i(g_object_id_uniform, MARIO);
    DrawVirtualObject("Mario");
}

// Função para desenhar o alvo
void DrawTarget(const Target& target) {
    if (target.IsAlive()) {
        glm::mat4 model = glm::mat4(1.0f);
        model = Matrix_Translate(target.GetX(), target.GetY(), target.GetZ()) * Matrix_Scale(0.5f, 0.5f, 0.5f);

        glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
        if (target.GetType() == 1) {
            glUniform1i(g_object_id_uniform, SPHERE);
        }
        else {
            glUniform1i(g_object_id_uniform, SPHERE_PARADA);
        }
    

        DrawVirtualObject("the_sphere"); 
    }
}

void HandleMouseClick(GLFWwindow* window, double xpos, double ypos, glm::mat4 view, glm::mat4 projection) {
    glm::vec4 rayDirection = ScreenToWorld(window, xpos, ypos, view, projection);
    glm::vec4 cameraPosition = glm::inverse(view)[3];

    for (auto& target : targets) {
        if (IsTargetHit(target, cameraPosition, rayDirection, view, projection)) {
            target.Hit();
            if(target.GetHealth() <= 0){
                jogador.addScore(10 + 10*target.GetType());
            }
            break;
        }
    }
}

glm::vec4 ScreenToWorld(GLFWwindow* window, double xpos, double ypos, glm::mat4 view, glm::mat4 projection) {
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    float x = (2.0f * static_cast<float>(xpos)) / width - 1.0f;
    float y = 1.0f - (2.0f * static_cast<float>(ypos)) / height;
    float z = 1.0f;

    glm::vec3 ray_nds(x, y, z);
    glm::vec4 ray_clip(ray_nds.x, ray_nds.y, -1.0, 1.0);

    glm::vec4 ray_eye = glm::inverse(projection) * ray_clip;
    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);

    glm::vec4 ray_wor = glm::vec4(glm::inverse(view) * ray_eye);
    ray_wor = ray_wor/norm(ray_wor);

    return ray_wor;
}

bool IsTargetHit(const Target& target, const glm::vec4& cameraPos, const glm::vec4& rayDir, glm::mat4 view, glm::mat4 projection) {
    glm::vec4 targetPos(target.GetX(), target.GetY(), target.GetZ(), 1.0f);

    // Transform target position to clip space
    glm::vec4 clipSpacePos = projection * view * targetPos;

    // Perform perspective divide to get normalized device coordinates (NDC)
    glm::vec3 ndcSpacePos = glm::vec3(clipSpacePos) / clipSpacePos.w;

    // Calculate the distance from the center of the screen in NDC
    glm::vec2 ndcCenter = glm::vec2(0.0f, 0.0f); // Center of the screen in NDC is (0, 0)
    glm::vec2 targetNDCPos = glm::vec2(ndcSpacePos.x, ndcSpacePos.y);
    float dx = targetNDCPos.x - ndcCenter.x;
    float dy = targetNDCPos.y - ndcCenter.y;
    float distance = sqrt(dx * dx + dy * dy);

    // Consider the radius of the target in NDC
    float targetRadiusNDC = 0.5f / clipSpacePos.w;

    //printf("Distance from NDC center: %f\n", distance);
    //printf("Target radius in NDC: %f\n", targetRadiusNDC);

    return distance < targetRadiusNDC;
}

// Função para gerar um número float aleatório entre min e max em intervalos de 0.5
float RandomFloat(float min, float max) {
    int range = static_cast<int>((max - min) * 2) + 1;
    return min + (rand() % range) * 0.5f;
}

// Função para spawnar um alvo em uma posição aleatória
void SpawnTarget() {
    srand(static_cast<unsigned int>(time(0)));

    // Gera coordenadas aleatórias para x e z
    float x = RandomFloat(-6.0f, 6.0f);
    float z = RandomFloat(-6.0f, 6.0f);
    float y = RandomFloat(1.0f, 4.0f); // Altura fixa

    // Cria um novo alvo
    Target newTarget(x, y, z, 1, RandomFloat(5.0f,15.0f),1,0);

    // Adiciona o novo alvo à lista de alvos
    targets.push_back(newTarget);
}

// Função para spawnar um alvo (em movimento) em uma posição aleatória
void SpawnTarget_mov() {
    srand(static_cast<unsigned int>(time(0)));

    // Gera coordenadas aleatórias para x e z
    float x = RandomFloat(-6.0f, 6.0f);
    float z = RandomFloat(-6.0f, 6.0f);
    float y = RandomFloat(1.0f, 4.0f); // Altura muda

    // Cria um novo alvo
    Target newTarget(x, y, z, 1, RandomFloat(10.0f,15.0f),1,1);

    // Adiciona o novo alvo à lista de alvos
    targets.push_back(newTarget);
}

void UpdateCountdown() {
    auto now = std::chrono::steady_clock::now();
    std::chrono::duration<float> elapsed = now - startTime;
    countdownTime = 60 - static_cast<int>(elapsed.count());
    if (countdownTime < 0) {
        countdownTime = 0;
    }
    if (countdownTime == 0) {
        fim_jogo = true;
    }
}