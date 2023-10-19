#pragma once

#include "lve_model.hpp"

#include <memory>

struct Transform2dComponent {
    glm::vec2 translation{}; // position offset
    float rotation;
    glm::vec2 scale{1.f,1.f};
    glm::mat2 mat2() {
        float s = glm::sin(rotation);
        float c = glm::cos(rotation);
        glm::mat2 rotMatrix {{c, s}, {-s, c}};
        glm::mat2 scaleMat{{scale.x, 0.f},{0.f, scale.y}};
        return rotMatrix*scaleMat; };
};

namespace lve {
class LveGameObject {
public:
    using id_t = unsigned int;

    static LveGameObject createGameObject() {
        static id_t currentId = 0;
        return LveGameObject{currentId++};
    }

    LveGameObject(const LveGameObject &) = delete;
    LveGameObject &operator=(const LveGameObject &) = delete;
    LveGameObject(LveGameObject &&) = default;
    LveGameObject &operator=(LveGameObject &&) = default;

    id_t const getId (){ return id; }

    std::shared_ptr<LveModel> model{};
    glm::vec3 color{};
    Transform2dComponent transform2d{};

private:
    LveGameObject(id_t objId) : id{objId} {}

    id_t id;
};
} // namespace lve