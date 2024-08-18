#include <iostream>
#include <vector>
#include <chrono>
#include <random>

class GameMap {
public:
    GameMap(int width, int height) : width_(width), height_(height) {
        map_.resize(width * height, ' ');
    }

    void AddWall(int x, int y, char direction) {
        if (IsValidPosition(x, y)) {
            if (direction == 'H') {
                map_[y * width_ + x] = '#'; // Parede horizontal
            } else if (direction == 'V') {
                map_[y * width_ + x] = '@'; // Parede vertical
            }
        }
    }

    void AddFloor(int x, int y) {
        if (IsValidPosition(x, y)) {
            map_[y * width_ + x] = '.';
        }
    }

    void PrintMap() const {
        for (int y = 0; y < height_; ++y) {
            for (int x = 0; x < width_; ++x) {
                std::cout << GetCell(x, y);
            }
            std::cout << std::endl;
        }
    }
    
    char GetCell(int x, int y) const {
        if (IsValidPosition(x, y)) {
            return map_[y * width_ + x];
        }
        return ' ';
    }

    int GetWidth() const { return width_; }
    int GetHeight() const { return height_; }

private:
    int width_;
    int height_;
    std::vector<char> map_;

    bool IsValidPosition(int x, int y) const {
        return x >= 0 && x < width_ && y >= 0 && y < height_;
    }
};

class Target {
public:
    // Construtor
    Target(float x, float y, float z, int health, float lifetime)
        : x_(x), y_(y), z_(z), health_(health), lifetime_(lifetime) {
        if (lifetime_ < 3.0f) {
            lifetime_ = 3.0f; // Garante que o tempo de vida seja no mínimo 3 segundos
        }
        creation_time_ = std::chrono::steady_clock::now();
    }

    // Métodos de acesso
    float GetX() const { return x_; }
    float GetY() const { return y_; }
    float GetZ() const { return z_; }
    int GetHealth() const { return health_; }
    float GetLifetime() const { return lifetime_; }

    // Método para verificar se o alvo ainda está vivo
    bool IsAlive() const {
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<float> elapsed = now - creation_time_;
        return elapsed.count() < lifetime_ && health_ > 0;
    }

    // Método para verificar se o alvo deve ser removido
    bool ShouldBeRemoved() const {
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<float> elapsed = now - creation_time_;
        return elapsed.count() >= lifetime_;
    }

    // Método para reduzir a vida do alvo quando ele é atingido
    void Hit() {
        if (health_ > 0) {
            printf("AHHHHHHHH!\n");
            --health_;
        }
    }

private:
    float x_, y_, z_; // Posição no mundo
    int health_; // Vida
    float lifetime_; // Tempo de vida em segundos
    std::chrono::steady_clock::time_point creation_time_; // Tempo de criação do alvo
};