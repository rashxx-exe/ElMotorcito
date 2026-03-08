#pragma once
#include <string>

class Nodo
{
public:
	std::string nombre;
	int id;

	float x = 0.0f;
	float y = 0.0f;
	std::string tipo = "Default";

	Nodo(const std::string& nombre, int id);

	void Guardar(const std::string& rutaEscena) const;
};