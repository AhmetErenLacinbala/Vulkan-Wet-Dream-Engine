#pragma once

#include "lve_model.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>
namespace lve {
struct TransformComponent {
    glm::vec3 translation{}; // position offset
    glm::vec3 rotation{};
    glm::vec3 scale{1.f, 1.f, 1.f};
    // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
    // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
    // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
    /*
  transform creates this:
  [1 0 0 tx
   0 1 0 ty
   0 0 1 tz
   0 0 0 1
  ]
  */
    // yxz euler angle to prevent the gimble lock least as posible
    // because objects look up or down less
    glm::mat4 mat4();
    glm::mat3 normalMatrix();
};


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

    id_t const getId() { return id; }

    std::shared_ptr<LveModel> model{};
    glm::vec3 color{};
    TransformComponent transform{};

private:
    LveGameObject(id_t objId) : id{objId} {}

    id_t id;
};
} // namespace lve