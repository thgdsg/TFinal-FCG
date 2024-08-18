#include <iostream>
#include <vector>

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

