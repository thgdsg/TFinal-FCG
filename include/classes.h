#include <iostream>
#include <vector>
#include <array>
#include <chrono>
#include <random>
#include <cmath>
#include <glm/mat4x4.hpp>

class GameMap {
public:
    GameMap(){
       
        //matriz modelo padrao do the_plane
        glm::mat4 model = Matrix_Identity();

        //coloca o chão
        map_.push_back(model*Matrix_Scale(7,0,7));
        // Paredes verticais
        map_.push_back(Matrix_Scale(7.0f, 2.0f, 0.1f) * Matrix_Translate(0.0f, 1.0f, 70.0f) * Matrix_Rotate_X(glm::radians(90.0f)));
        map_.push_back(Matrix_Scale(7.0f, 2.0f, 0.1f) * Matrix_Translate(0.0f, 1.0f, -70.0f) * Matrix_Rotate_X(glm::radians(90.0f)));
        
        // Parede ao longo do eixo X (esquerda e direita)
        map_.push_back(Matrix_Scale(0.1f, 2.0f, 7.0f) * Matrix_Translate(70.0f, 1.0f, 0.0f) * Matrix_Rotate_X(glm::radians(90.0f))* Matrix_Rotate_Z(glm::radians(90.0f)));
        map_.push_back(Matrix_Scale(0.1f, 2.0f, 7.0f) * Matrix_Translate(-70.0f, 1.0f, 0.0f)* Matrix_Rotate_X(glm::radians(90.0f)) * Matrix_Rotate_Z(glm::radians(90.0f)));
    }
    std::vector<glm::mat4> getModels(){ return map_;}

private:
    std::vector<glm::mat4> map_;
};

class Target {
public:
    Target(float x, float y, float z, int health, float lifetime, float radius,int type)
        : x_(x), y_(y), z_(z), health_(health), lifetime_(lifetime), radius_(radius),type_(type) {
        if (lifetime_ < 3.0f) {
            lifetime_ = 3.0f; // Garante que o tempo de vida seja no mínimo 3 segundos
        }
        creation_time_ = std::chrono::steady_clock::now();
        SetBezierControlPoints(); // Define os pontos de controle no momento da criação
    }

    void UpdatePosition() {
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<float> elapsed = now - creation_time_;

        // Normaliza o tempo para [0, 1] baseado em um ciclo de 4 segundos (para um círculo completo)
        float t = fmod(elapsed.count(), 4.0f) / 4.0f;

        int segment = static_cast<int>(t * 4) % 4;  // 4 segmentos para o círculo completo
        float localT = (t * 4) - segment;           // Tempo local dentro do segmento atual

        // Pegar os pontos de controle para o segmento atual
        auto& p0 = controlPoints[segment][0];
        auto& p1 = controlPoints[segment][1];
        auto& p2 = controlPoints[segment][2];
        auto& p3 = controlPoints[segment][3];

        // Calcular a posição do alvo na curva de Bézier
        x_ = bezier(localT, p0[0], p1[0], p2[0], p3[0]);
        y_ = bezier(localT, p0[1], p1[1], p2[1], p3[1]);
        z_ = bezier(localT, p0[2], p1[2], p2[2], p3[2]);
    }

    // Métodos de acesso
    float GetX() const { return x_; }
    float GetY() const { return y_; }
    float GetZ() const { return z_; }
    int GetHealth() const { return health_; }
    float GetLifetime() const { return lifetime_; }
    int GetType()const { return type_; }

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
            --health_;
            printf("HP Left: %d\n", health_);
        }
    }

private:
    void SetBezierControlPoints() {
        // Define os pontos de controle baseados na posição inicial do target (x_, y_, z_)
        controlPoints = { {
                // Segmento 1
                {{{x_ + radius_, y_, z_}, {x_ + radius_, y_, z_ + radius_ * 0.5522847f}, {x_ + radius_ * 0.5522847f, y_, z_ + radius_}, {x_, y_, z_ + radius_}}},
                // Segmento 2
                {{{x_, y_, z_ + radius_}, {x_ - radius_ * 0.5522847f, y_, z_ + radius_}, {x_ - radius_, y_, z_ + radius_ * 0.5522847f}, {x_ - radius_, y_, z_}}},
                // Segmento 3
                {{{x_ - radius_, y_, z_}, {x_ - radius_, y_, z_ - radius_ * 0.5522847f}, {x_ - radius_ * 0.5522847f, y_, z_ - radius_}, {x_, y_, z_ - radius_}}},
                // Segmento 4
                {{{x_, y_, z_ - radius_}, {x_ + radius_ * 0.5522847f, y_, z_ - radius_}, {x_ + radius_, y_, z_ - radius_ * 0.5522847f}, {x_ + radius_, y_, z_}}}
            } };
    }

    float bezier(float t, float p0, float p1, float p2, float p3) const {
        float u = 1 - t;
        float tt = t * t;
        float uu = u * u;
        float uuu = uu * u;
        float ttt = tt * t;

        return uuu * p0 + 3 * uu * t * p1 + 3 * u * tt * p2 + ttt * p3;
    }

    float x_, y_, z_; // Posição no mundo
    int health_; // Vida
    float lifetime_; // Tempo de vida em segundos
    float radius_; // Raio do círculo
    int type_;//tipo do circulo (1, se move, 0, fica parado)
    std::chrono::steady_clock::time_point creation_time_; // Tempo de criação do alvo

    // Pontos de controle das curvas de Bézier (4 segmentos, cada um com 4 pontos)
    std::array<std::array<std::array<float, 3>, 4>, 4> controlPoints;
};

class Player {
public:
    Player() : score_(0) {}

    Player(int score) : score_(score) {}

    int getScore() const { return score_; }
    void setScore(int score){score_ = score;}
    void addScore(int points) { score_ += points; }

    void resetScore() { score_ = 0; }

private:
    int score_;
};