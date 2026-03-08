#include "Nodo.h"
#include "json.hpp"
#include <fstream>
#include <filesystem>

Nodo::Nodo(const std::string& nombre, int id) 
	: nombre(nombre), id(id)
{ }

void Nodo::Guardar(const std::string& rutaEscena) const
{
	json json;
    json["nombre"] = nombre;
    json["id"] = std::to_string(id);
    json["x"] = std::to_string(x);
    json["y"] = std::to_string(y);
    json["tipo"] = tipo;

    std::string ruta = rutaEscena + "/Nodos/";
    std::filesystem::create_directories(ruta);

    std::ofstream file(ruta + nombre + ".json");
    if (file.is_open())
    {
        file << json.dump(4);
        file.close();
    }
}
