#pragma once
#include <SDL3/SDL.h>
#include <vector>
#include <map>
#include <unordered_set>
#include <functional>
#include <string>
#include <algorithm>
#include <fstream>
#include "imgui.h"
#include "Vector.h"

namespace ElMotorcito
{
    enum class ColliderType { Circle, Box };

    struct Collider
    {
        ColliderType type = ColliderType::Box;
        float radius = 0.0f;
        ImVec2 offset = { 0, 0 };
    };

    struct Nodo
    {
        int         id = -1;
        std::string name = "Nodo";
        ImVec2      position = { 0, 0 };
        ImVec2      size = { 50, 50 };
        float       rotation = 0.0f;
        int         zOrder = 0;
        bool        active = true;
        int         layer = 0;

        bool     hasCollider = false;
        Collider collider;

        SDL_Texture* texture = nullptr;
        SDL_FRect    spriteRect = { 0,0,0,0 };
        bool         useSprite = false;

        Nodo() = default;
        Nodo(int i, const std::string& n, ImVec2 pos, ImVec2 s)
            : id(i), name(n), position(pos), size(s) {
        }
    };

    struct CollisionPair { int idA, idB; };

    namespace Collision
    {
        bool CircleCircle(const Nodo& a, const Nodo& b);
        bool BoxBox(const Nodo& a, const Nodo& b);
        bool CircleBox(const Nodo& circle, const Nodo& box);
        bool Test(const Nodo& a, const Nodo& b);
    }

    struct Escena
    {
        std::string       name;
        std::vector<Nodo> objects;
        int               nextNodeId = 0;

        Nodo& AñadirNodo(const std::string& nodeName, ImVec2 pos, ImVec2 size)
        {
            objects.emplace_back(nextNodeId++, nodeName, pos, size);
            return objects.back();
        }
        Nodo& AñadirNodo()
        {
            Nodo n; n.id = nextNodeId++;
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
        void Clear() { objects.clear(); nextNodeId = 0; }

        bool Save(const std::string& path) const;
        bool Load(const std::string& path);
    };

    struct EscenasManager
    {
        std::vector<Escena> escenas;
        int escenaActivaIndex = -1;

        Escena* GetEscenaActiva()
        {
            if (escenaActivaIndex >= 0 &&
                escenaActivaIndex < (int)escenas.size())
                return &escenas[escenaActivaIndex];
            return nullptr;
        }
        Escena& AfegirEscena(const std::string& nom)
        {
            Escena e; e.name = nom;
            escenas.push_back(e);
            return escenas.back();
        }
        void CambiarEscena(int index)
        {
            if (index >= 0 && index < (int)escenas.size())
                escenaActivaIndex = index;
        }
        void NetejarEscenaActiva()
        {
            if (Escena* e = GetEscenaActiva()) e->Clear();
        }
    };

    class InputManager
    {
    public:
        void BeginFrame();
        void ProcessEvent(const SDL_Event& e);

        bool IsKeyDown(SDL_Keycode k)    const;
        bool IsKeyPressed(SDL_Keycode k) const;
        bool IsKeyReleased(SDL_Keycode k)const;

        bool IsMouseButtonDown(int btn)  const;
        bool IsMouseButtonPressed(int btn) const;
        ImVec2 GetMousePos()             const { return mousePos; }
        ImVec2 GetMouseDelta()           const { return mouseDelta; }

    private:
        std::unordered_set<SDL_Keycode> keysDown, keysPressed, keysReleased;
        std::unordered_set<int>         mouseDown, mousePressed;
        ImVec2 mousePos = { 0,0 };
        ImVec2 mouseDelta = { 0,0 };
        ImVec2 prevMousePos = { 0,0 };
    };

    class Motor
    {
        Motor(const Motor&) = delete;
        Motor& operator=(const Motor&) = delete;

    private:
        Motor() = default;

        float            main_scale = 1.0f;
        SDL_Window* window = nullptr;
        SDL_WindowFlags  window_flags = 0;
        SDL_Renderer* renderer = nullptr;
        bool             running = false;

        ImFont* fontTitulo = nullptr;
        ImFont* fontTexto = nullptr;

        EscenasManager escenasManager;
        InputManager   input;
        int            selectedNodeId = -1;

        void RenderNodo(const Nodo& n);
        void RenderScene(Escena& e);

        std::vector<CollisionPair> DetectCollisions(Escena& e, int layerMask = -1);

        void DrawEscenasPanel(float& panelH, int& newSceneCounter);
        void DrawNodosPanel(float panelH, int& newNodeCounter);

    public:
        static Motor& Instance();
        int  Init();
        void Run();
        void Exit();

        SDL_Renderer* GetRenderer() const { return renderer; }
        SDL_Window* GetWindow()   const { return window; }
        InputManager& GetInput() { return input; }
    };

}