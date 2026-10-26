#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float2.hpp>
#include <tiny_obj_loader.h>
class Vertex {
public:
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoord;
};


class Model {
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string warnings;
	std::string errors;

	tinyobj::LoadObj(&attributes, &shapes, &materials, &warnings, &errors, modelPath.c_str(), modelMtlBaseDir.c_str());
};