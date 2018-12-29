#pragma once
#include "Texture2D.h"
#include "Shader.h"

class Shader;
using std::shared_ptr;
struct Material {
	const std::string name;

	shared_ptr<Shader> shader;
	shared_ptr<Texture2D> albedo, roughness, metalness, normal;
	Material(
		std::string name, 
		shared_ptr<Shader> shader, 
		shared_ptr<Texture2D> albedo,
		shared_ptr<Texture2D> roughness,
		shared_ptr<Texture2D> metalness, 
		shared_ptr<Texture2D> normal)
		:	name(name), 
			shader(shader), 
			albedo(albedo), 
			roughness(roughness),
			metalness(metalness),
			normal(normal) {}
};
