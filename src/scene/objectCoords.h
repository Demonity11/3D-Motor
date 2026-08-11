#ifndef OBJECT_COORDS_H
#define OBJECT_COORDS_H

#include <vector>
#include <glm/glm.hpp>

auto getCilinderVertices(glm::vec3 p0, glm::vec3 p, glm::vec4 color, float radius, std::vector<float>& vertexData)						-> int;
auto getRingsVertices(glm::vec3 p0, glm::vec3 p, glm::vec4 color, std::vector<float>& vertexData)										-> int;
auto getSphereVertices(glm::vec3 translation, glm::vec4 color, float radius, std::vector<float>& vertexData)							-> int;
auto getGridVertices(std::vector<float>& vertexData)																					-> int;
auto getPlaneVertices(glm::vec3 normalP0, glm::vec3 normalP, glm::vec3 point, glm::vec4 color, std::vector<float>& vertexData)			-> int;
auto getLineVertices(glm::vec3 point, glm::vec3 dVecP0, glm::vec3 dVecP, glm::vec4 color, float radius, std::vector<float>& vertexData) -> int;
auto getConeVertices(glm::vec3 direction, glm::vec3 apex, glm::vec4 color, float radius, float height, std::vector<float>& vertexData)  -> int;
auto getEnvironmentVertices(std::vector<float>& vertexData, bool firstRun = false)														-> void;
auto getNewCoordSystem(glm::vec3& direction, glm::vec3& right, glm::vec3& up)															-> void;

#endif
