#include <cmath>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "classes.h"
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

// Função para detectar colisão da câmera com as paredes do cenário
bool CheckCollisionWithWorld(const glm::vec4& cameraPosition) {
    const float minX = -6.8f;
    const float maxX = 6.8f;
    const float minZ = -6.8f;
    const float maxZ = 6.8f;

    if (cameraPosition.x < minX || cameraPosition.x > maxX ||
        cameraPosition.z < minZ || cameraPosition.z > maxZ) {
        return true; // Colisão detectada
    }
    return false; // Nenhuma colisão detectada
}

bool CheckSphereCollisionWithSphere(const Target& target1, const Target& target2) {
    // Obter as coordenadas dos centros dos alvos
    float x1 = target1.GetX();
    float y1 = target1.GetY();
    float z1 = target1.GetZ();
    
    float x2 = target2.GetX();
    float y2 = target2.GetY();
    float z2 = target2.GetZ();
    
    // Calcular a distância entre os centros dos alvos
    float distance = std::sqrt((x2 - x1) * (x2 - x1) +
                               (y2 - y1) * (y2 - y1) +
                               (z2 - z1) * (z2 - z1));
    
    // Obter os raios dos alvos
    float radius1 = 0.5f;
    float radius2 = 0.5f;
    
    // Verificar se a distância é menor ou igual à soma dos raios
    if (distance <= (radius1 + radius2)) {
        return true; // Colisão detectada
    }
    return false; // Nenhuma colisão detectada
}