#pragma once
#include <vector>
#include <string>
#include "Nodo.h"

class Escena
{
private:
    std::string nombre;

public:
    std::vector<Nodo> nodos;

    Escena(const std::string& nombre);

    const std::string& GetNombre() const;

    Nodo& CrearNodo();
};