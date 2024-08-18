#include "classes.h"

// FUNÇÃO RETIRADA DO CHATGPT
// Testa colisão entre camera e modelo
/*bool CheckCollisionWithCamera(glm::vec4 camera_position_c, Model model) {
    glm::vec4 worldCenter = model.modelMatrix * glm::vec4(model.center, 1.0f);
    float distance = glm::length(glm::vec3(camera_position_c) - glm::vec3(worldCenter));
    return distance < model.radius;
}*/

bool CheckCollisionWithMap(const GameMap& gameMap, float playerX, float playerY, float playerZ) {
    int mapX = static_cast<int>(playerX);
    int mapZ = static_cast<int>(playerZ);

    if (!gameMap.IsValidPosition(mapX, mapZ)) {
        return true; // Out of bounds
    }

    char cell = gameMap.GetCell(mapX, mapZ);
    return cell == '#'; // Collision if the cell is a wall
}