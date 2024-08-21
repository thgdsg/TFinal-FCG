#include "classes.h"

// FUNÇÃO RETIRADA DO CHATGPT
// Testa colisão entre camera e modelo
/*bool CheckCollisionWithCamera(glm::vec4 camera_position_c, Model model) {
    glm::vec4 worldCenter = model.modelMatrix * glm::vec4(model.center, 1.0f);
    float distance = glm::length(glm::vec3(camera_position_c) - glm::vec3(worldCenter));
    return distance < model.radius;
}*/

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