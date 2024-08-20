#version 330 core

// Atributos de fragmentos recebidos como entrada ("in") pelo Fragment Shader.
// Neste exemplo, este atributo foi gerado pelo rasterizador como a
// interpolação da posição global e a normal de cada vértice, definidas em
// "shader_vertex.glsl" e "main.cpp".
in vec4 position_world;
in vec4 normal;

// Posição do vértice atual no sistema de coordenadas local do modelo.
in vec4 position_model;

// Coordenadas de textura obtidas do arquivo OBJ (se existirem!)
in vec2 texcoords;

// Matrizes computadas no código C++ e enviadas para a GPU
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Identificador que define qual objeto está sendo desenhado no momento
#define SPHERE 0
#define SPHERE_PARADA  1
#define PLANE  2

uniform int object_id;

// Parâmetros da axis-aligned bounding box (AABB) do modelo
uniform vec4 bbox_min;
uniform vec4 bbox_max;


// Variáveis para acesso das imagens de textura
uniform sampler2D TextureImage0;
uniform sampler2D TextureImage1;
uniform sampler2D TextureImage2;



// O valor de saída ("out") de um Fragment Shader é a cor final do fragmento.
out vec4 color;

// Constantes
#define M_PI   3.14159265358979323846
#define M_PI_2 1.57079632679489661923

void main()
{
    // Obtemos a posição da câmera utilizando a inversa da matriz que define o
    // sistema de coordenadas da câmera.
    vec4 origin = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 camera_position = inverse(view) * origin;

    // O fragmento atual é coberto por um ponto que percente à superfície de um
    // dos objetos virtuais da cena. Este ponto, p, possui uma posição no
    // sistema de coordenadas global (World coordinates). Esta posição é obtida
    // através da interpolação, feita pelo rasterizador, da posição de cada
    // vértice.
    vec4 p = position_world;

    // Normal do fragmento atual, interpolada pelo rasterizador a partir das
    // normais de cada vértice.
    vec4 n = normalize(normal);

    // Vetor que define o sentido da fonte de luz em relação ao ponto atual.
    vec4 l = normalize(vec4(1.0,1.0,0.0,0.0));

    // Vetor que define o sentido da câmera em relação ao ponto atual.
    vec4 v = normalize(camera_position - p);

    float lambert = max(0,dot(n,l));

    // Coordenadas de textura U e V
    float U = 0.0;
    float V = 0.0;

        

    if ( object_id == SPHERE )
    {
        // PREENCHA AQUI as coordenadas de textura da esfera, computadas com
        // projeção esférica EM COORDENADAS DO MODELO. Utilize como referência
        // o slides 134-150 do documento Aula_20_Mapeamento_de_Texturas.pdf.
        // A esfera que define a projeção deve estar centrada na posição
        // "bbox_center" definida abaixo.

        // Você deve utilizar:
        //   função 'length( )' : comprimento Euclidiano de um vetor
        //   função 'atan( , )' : arcotangente. Veja https://en.wikipedia.org/wiki/Atan2.
        //   função 'asin( )'   : seno inverso.
        //   constante M_PI
        //   variável position_model

        vec4 bbox_center = (bbox_min + bbox_max) / 2.0;
        // Vetor da posição do vértice em relação ao centro da bounding box
        vec3 position_rel_center = vec3(position_model - bbox_center);

        // Comprimento Euclidiano do vetor (raio da esfera)
        float r = length(position_rel_center);

        // Coordenadas esféricas
        float theta = atan(position_rel_center.y, position_rel_center.x); // Ângulo azimutal (longitude)
        float phi = asin(position_rel_center.z / r);                      // Ângulo polar (latitude)

        // Normalização das coordenadas U e V no intervalo [0, 1]
        U = (theta + M_PI) / (2.0 * M_PI); // U varia de 0 a 1
        V = (phi + M_PI / 2.0) / M_PI;     // V varia de 0 a 1
       
        vec3 Kd1 = texture(TextureImage1, vec2(U,V)).rgb;
       

        color.rgb = Kd1 * (lambert + 0.15);
    }
    else if ( object_id == PLANE )
    {
        U = texcoords.x;
        V = texcoords.y;

        vec3 Kd0 = texture(TextureImage0, vec2(U,V)).rgb;

         color.rgb = Kd0 * (lambert + 0.2);
    }

    else if( object_id == SPHERE_PARADA)
    {
                // PREENCHA AQUI as coordenadas de textura da esfera, computadas com
        // projeção esférica EM COORDENADAS DO MODELO. Utilize como referência
        // o slides 134-150 do documento Aula_20_Mapeamento_de_Texturas.pdf.
        // A esfera que define a projeção deve estar centrada na posição
        // "bbox_center" definida abaixo.

        // Você deve utilizar:
        //   função 'length( )' : comprimento Euclidiano de um vetor
        //   função 'atan( , )' : arcotangente. Veja https://en.wikipedia.org/wiki/Atan2.
        //   função 'asin( )'   : seno inverso.
        //   constante M_PI
        //   variável position_model

        vec4 bbox_center = (bbox_min + bbox_max) / 2.0;
        // Vetor da posição do vértice em relação ao centro da bounding box
        vec3 position_rel_center = vec3(position_model - bbox_center);

        // Comprimento Euclidiano do vetor (raio da esfera)
        float r = length(position_rel_center);

        // Coordenadas esféricas
        float theta = atan(position_rel_center.y, position_rel_center.x); // Ângulo azimutal (longitude)
        float phi = asin(position_rel_center.z / r);                      // Ângulo polar (latitude)

        // Normalização das coordenadas U e V no intervalo [0, 1]
        U = (theta + M_PI) / (2.0 * M_PI); // U varia de 0 a 1
        V = (phi + M_PI / 2.0) / M_PI;     // V varia de 0 a 1
       
        vec3 Kd1 = texture(TextureImage2, vec2(U,V)).rgb;
       

        color.rgb = Kd1 * (lambert + 0.15);
    
    }


    // NOTE: Se você quiser fazer o rendering de objetos transparentes, é
    // necessário:
    // 1) Habilitar a operação de "blending" de OpenGL logo antes de realizar o
    //    desenho dos objetos transparentes, com os comandos abaixo no código C++:
    //      glEnable(GL_BLEND);
    //      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // 2) Realizar o desenho de todos objetos transparentes *após* ter desenhado
    //    todos os objetos opacos; e
    // 3) Realizar o desenho de objetos transparentes ordenados de acordo com
    //    suas distâncias para a câmera (desenhando primeiro objetos
    //    transparentes que estão mais longe da câmera).
    // Alpha default = 1 = 100% opaco = 0% transparente
    color.a = 1;

    // Cor final com correção gamma, considerando monitor sRGB.
    // Veja https://en.wikipedia.org/w/index.php?title=Gamma_correction&oldid=751281772#Windows.2C_Mac.2C_sRGB_and_TV.2Fvideo_standard_gammas
    color.rgb = pow(color.rgb, vec3(1.0,1.0,1.0)/2.2);
} 

