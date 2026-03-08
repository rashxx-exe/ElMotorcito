#pragma once


#include <SDL3/SDL.h>
#include <vector>
#include "imgui.h"
#include <string>
#include <algorithm> 
#include <map>
#include <unordered_set>
#include <fstream>
#include "Vector.h"

namespace ElMotorcito
{

	struct Nodo
	{
		int id = -1;
		std::string name = "Nodo";
		ImVec2 position = ImVec2(0, 0);
		ImVec2 size = ImVec2(50, 50);
		bool active = true;

		Nodo() = default;

		Nodo(int i, const std::string& n, ImVec2 pos, ImVec2 s)
			: id(i), name(n), position(pos), size(s) {
		}
	};


    struct Escena
    {
        std::string name;
        std::vector<Nodo> objects;

        Nodo& AñadirNodo(const Nodo& obj)
        {
            objects.push_back(obj);
            return objects.back();
        }

        Nodo& AñadirNodo(const std::string& name, ImVec2 pos, ImVec2 size)
        {
            int newId = static_cast<int>(objects.size());
            objects.emplace_back(newId, name, pos, size);
            return objects.back();
        }

        Nodo& AñadirNodo()
        {
            int newId = static_cast<int>(objects.size());
            Nodo n;
            n.id = newId;
            objects.push_back(n);
            return objects.back();
        }

        void EliminarNodo(int id)
        {
            objects.erase(
                std::remove_if(objects.begin(), objects.end(), 
                    [id](const Nodo& n) { return n.id == id; }),
                objects.end());
        }
        void Clear() { objects.clear(); }
    };

    struct EscenasManager
    {
        std::vector<Escena> escenas;
        int escenaActivaIndex = -1;

        Escena* GetEscenaActiva()
        {
            if (escenaActivaIndex >= 0 && escenaActivaIndex < escenas.size())
                return &escenas[escenaActivaIndex];
            return nullptr;
        }

        Escena& AfegirEscena(const std::string& nom) {
            Escena e; e.name = nom;
            escenas.push_back(e);
            return escenas.back();
        }

        void CambiarEscena(int index)
        {
            if (index >= 0 && index < escenas.size())
                escenaActivaIndex = index;
        }

        void NetejarEscenaActiva() {
            if (Escena* e = GetEscenaActiva()) e->Clear();
        }
    };

	class Motor 
	{
        Motor(const Motor&) = delete;
        Motor& operator = (const Motor&) = delete;

		private:

			Motor() = default;

			float main_scale = 0.0;
			SDL_Window* window = NULL;
			SDL_WindowFlags window_flags = NULL;
			SDL_Renderer* renderer = NULL;
			bool running = false;

            ImFont* fontTitulo = nullptr;
            ImFont* fontTexto = nullptr;

			EscenasManager escenasManager;
            int selectedNodeId = -1;

		public:
            static Motor& Instance();

			int Init();
			void Run();
			void Exit();

			SDL_Renderer* GetRenderer() const { return renderer; }
			SDL_Window* GetWindow() const { return window;  }
	};
}
