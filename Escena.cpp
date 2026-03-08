#include "Escena.h"
#include <filesystem>

static int nextNodeID = 0;

Escena::Escena(const std::string& nombre)
    : nombre(nombre)
{
    // Crear estructura de carpetas de la escena
    std::string ruta = "Escenas/" + nombre;
    std::filesystem::create_directories(ruta + "/Nodos");
}

const std::string& Escena::GetNombre() const
{
    return nombre;
}

Nodo& Escena::CrearNodo()
{
    std::string nombreNodo = "Nodo_" + std::to_string(nextNodeID);
    nodos.emplace_back(nombreNodo, nextNodeID);

    // Guardar en disco al crearlo
    nodos.back().Guardar("Escenas/" + nombre);

    nextNodeID++;

    return nodos.back();
}