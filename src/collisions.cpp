#include "classes.h"
//#include "matrices.h"
// Headers das bibliotecas OpenGL
//#include <glad/glad.h>   // Criação de contexto OpenGL 3.3
//#include <GLFW/glfw3.h>  // Criação de janelas do sistema operacional

// Headers da biblioteca GLM: criação de matrizes e vetores.
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

bool CheckCollisionWithSphere(const glm::vec4& cameraPos, const Target& target) {
    // Calcular a diferença entre os componentes dos vetores
    float dx = cameraPos.x - target.GetX();
    float dy = cameraPos.y - target.GetY();
    float dz = cameraPos.z - target.GetZ();

    // Calcular a distância euclidiana
    float distance = std::sqrt(dx * dx + dy * dy + dz * dz);

    // Verificar se a distância é menor ou igual a 0.5
    return distance <= 1;
}