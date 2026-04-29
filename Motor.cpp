#include "Motor.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include <stdio.h>
#include <cmath>
#include <algorithm>
#include <sstream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace ElMotorcito;

Motor& Motor::Instance()
{
    static Motor instance;
    return instance;
}

namespace ElMotorcito {
    namespace Collision {

        static ImVec2 NodeCenter(const Nodo& n)
        {
            return { n.position.x + n.size.x * 0.5f,
                     n.position.y + n.size.y * 0.5f };
        }
        static float CircleRadius(const Nodo& n)
        {
            return n.hasCollider ? n.collider.radius
                : std::min(n.size.x, n.size.y) * 0.5f;
        }

        bool CircleCircle(const Nodo& a, const Nodo& b)
        {
            ImVec2 ca = NodeCenter(a), cb = NodeCenter(b);
            float ra = CircleRadius(a), rb = CircleRadius(b);
            float dx = ca.x - cb.x, dy = ca.y - cb.y;
            return dx * dx + dy * dy <= (ra + rb) * (ra + rb);
        }

        bool BoxBox(const Nodo& a, const Nodo& b)
        {
            return a.position.x < b.position.x + b.size.x &&
                a.position.x + a.size.x > b.position.x &&
                a.position.y < b.position.y + b.size.y &&
                a.position.y + a.size.y > b.position.y;
        }

        bool CircleBox(const Nodo& circle, const Nodo& box)
        {
            ImVec2 c = NodeCenter(circle);
            float  r = CircleRadius(circle);
            float  cx = std::max(box.position.x, std::min(c.x, box.position.x + box.size.x));
            float  cy = std::max(box.position.y, std::min(c.y, box.position.y + box.size.y));
            float  dx = c.x - cx, dy = c.y - cy;
            return dx * dx + dy * dy <= r * r;
        }

        bool Test(const Nodo& a, const Nodo& b)
        {
            if (!a.hasCollider || !b.hasCollider) return false;

            ColliderType ta = a.collider.type, tb = b.collider.type;

            if (ta == ColliderType::Circle && tb == ColliderType::Circle)
                return CircleCircle(a, b);
            if (ta == ColliderType::Box && tb == ColliderType::Box)
                return BoxBox(a, b);
            if (ta == ColliderType::Circle && tb == ColliderType::Box)
                return CircleBox(a, b);
            if (ta == ColliderType::Box && tb == ColliderType::Circle)
                return CircleBox(b, a);
            return false;
        }
    }
}

bool Escena::Save(const std::string& path) const
{
    std::ofstream f(path);
    if (!f) return false;
    f << name << "\n" << objects.size() << "\n";
    for (const auto& n : objects)
    {
        f << n.id << " "
            << n.position.x << " " << n.position.y << " "
            << n.size.x << " " << n.size.y << " "
            << n.rotation << " " << n.zOrder << " "
            << n.layer << " " << (int)n.active << " "
            << n.name << "\n";
    }
    return true;
}

bool Escena::Load(const std::string& path)
{
    std::ifstream f(path);
    if (!f) return false;
    Clear();
    std::getline(f, name);
    int count = 0; f >> count;
    for (int i = 0; i < count; ++i)
    {
        Nodo n;
        int active = 1;
        f >> n.id
            >> n.position.x >> n.position.y
            >> n.size.x >> n.size.y
            >> n.rotation >> n.zOrder
            >> n.layer >> active;
        f.ignore();
        std::getline(f, n.name);
        n.active = (active != 0);
        objects.push_back(n);
        if (n.id >= nextNodeId) nextNodeId = n.id + 1;
    }
    return true;
}

void InputManager::BeginFrame()
{
    keysPressed.clear();
    keysReleased.clear();
    mousePressed.clear();
    prevMousePos = mousePos;
}

void InputManager::ProcessEvent(const SDL_Event& e)
{
    if (e.type == SDL_EVENT_KEY_DOWN && !e.key.repeat)
    {
        keysDown.insert(e.key.key);
        keysPressed.insert(e.key.key);
    }
    if (e.type == SDL_EVENT_KEY_UP)
    {
        keysDown.erase(e.key.key);
        keysReleased.insert(e.key.key);
    }
    if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
    {
        mouseDown.insert(e.button.button);
        mousePressed.insert(e.button.button);
    }
    if (e.type == SDL_EVENT_MOUSE_BUTTON_UP)
        mouseDown.erase(e.button.button);
    if (e.type == SDL_EVENT_MOUSE_MOTION)
    {
        mousePos = { e.motion.x, e.motion.y };
        mouseDelta = { e.motion.xrel, e.motion.yrel };
    }
}

bool InputManager::IsKeyDown(SDL_Keycode k) const { return keysDown.count(k); }
bool InputManager::IsKeyPressed(SDL_Keycode k) const { return keysPressed.count(k); }
bool InputManager::IsKeyReleased(SDL_Keycode k) const { return keysReleased.count(k); }
bool InputManager::IsMouseButtonDown(int b)  const { return mouseDown.count(b); }
bool InputManager::IsMouseButtonPressed(int b)  const { return mousePressed.count(b); }

void Motor::RenderNodo(const Nodo& n)
{
    if (!n.active) return;

    if (n.texture)
    {
        SDL_FRect dst = { n.position.x, n.position.y, n.size.x, n.size.y };
        if (n.useSprite)
        {
            SDL_RenderTextureRotated(renderer, n.texture,
                &n.spriteRect, &dst,
                (double)n.rotation, nullptr, SDL_FLIP_NONE);
        }
        else
        {
            SDL_RenderTextureRotated(renderer, n.texture,
                nullptr, &dst,
                (double)n.rotation, nullptr, SDL_FLIP_NONE);
        }
    }
    else
    {
        SDL_SetRenderDrawColor(renderer, 80, 160, 220, 255);
        SDL_FRect rect = { n.position.x, n.position.y, n.size.x, n.size.y };
        SDL_RenderFillRect(renderer, &rect);

        SDL_SetRenderDrawColor(renderer, 200, 220, 255, 255);
        SDL_RenderRect(renderer, &rect);
    }

    if (n.hasCollider)
    {
        SDL_SetRenderDrawColor(renderer, 255, 80, 80, 200);
        if (n.collider.type == ColliderType::Box)
        {
            SDL_FRect r = { n.position.x, n.position.y, n.size.x, n.size.y };
            SDL_RenderRect(renderer, &r);
        }
        else
        {
            float cx = n.position.x + n.size.x * 0.5f;
            float cy = n.position.y + n.size.y * 0.5f;
            float r = n.collider.radius > 0 ? n.collider.radius
                : std::min(n.size.x, n.size.y) * 0.5f;
            const int segs = 24;
            for (int s = 0; s < segs; ++s)
            {
                float a0 = (float)(s) / segs * 2.0f * (float)M_PI;
                float a1 = (float)(s + 1) / segs * 2.0f * (float)M_PI;
                SDL_RenderLine(renderer,
                    cx + cosf(a0) * r, cy + sinf(a0) * r,
                    cx + cosf(a1) * r, cy + sinf(a1) * r);
            }
        }
    }
}

void Motor::RenderScene(Escena& e)
{
    std::vector<Nodo*> sorted;
    sorted.reserve(e.objects.size());
    for (auto& n : e.objects) sorted.push_back(&n);
    std::stable_sort(sorted.begin(), sorted.end(),
        [](const Nodo* a, const Nodo* b) { return a->zOrder < b->zOrder; });

    for (auto* n : sorted) RenderNodo(*n);
}

std::vector<CollisionPair> Motor::DetectCollisions(Escena& e, int layerMask)
{
    std::vector<CollisionPair> result;
    for (int i = 0; i < (int)e.objects.size(); ++i)
    {
        for (int j = i + 1; j < (int)e.objects.size(); ++j)
        {
            const Nodo& a = e.objects[i];
            const Nodo& b = e.objects[j];
            if (layerMask != -1)
            {
                if (!(a.layer & layerMask) || !(b.layer & layerMask)) continue;
            }
            if (Collision::Test(a, b))
                result.push_back({ a.id, b.id });
        }
    }
    return result;
}

void Motor::DrawEscenasPanel(float& panelH, int& newSceneCounter)
{
    ImGuiIO& io = ImGui::GetIO();
    panelH = io.DisplaySize.y * 0.25f;
    ImGui::SetNextWindowPos({ 0, io.DisplaySize.y - panelH });
    ImGui::SetNextWindowSize({ io.DisplaySize.x, panelH });
    ImGui::Begin("Escenas", nullptr,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    ImGui::PushFont(fontTitulo);
    ImGui::Text("Escenas");
    ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 100.0f);
    if (ImGui::Button("Netejar", ImVec2(80, 28)))
        escenasManager.NetejarEscenaActiva();
    ImGui::SameLine();
    if (ImGui::Button("+", ImVec2(28, 28)))
    {
        std::string nm = "Escena " + std::to_string(newSceneCounter++);
        escenasManager.AfegirEscena(nm);
        escenasManager.escenaActivaIndex = (int)escenasManager.escenas.size() - 1;
    }
    ImGui::Separator();
    ImGui::PopFont();

    if (Escena* ae = escenasManager.GetEscenaActiva())
    {
        // Sistema de guardat no funciona v
        static char savePath[256] = "aixonofunciona.jeje";
        ImGui::PushFont(fontTexto);
        ImGui::SetNextItemWidth(200);
        ImGui::InputText("##savePath", savePath, sizeof(savePath));
        ImGui::SameLine();
        if (ImGui::Button("Desar"))  ae->Save(savePath);
        ImGui::SameLine();
        if (ImGui::Button("Carregar")) ae->Load(savePath);
        ImGui::PopFont();
        ImGui::Separator();
    }

    int sceneToDelete = -1;
    for (int i = 0; i < (int)escenasManager.escenas.size(); ++i)
    {
        Escena& escena = escenasManager.escenas[i];
        bool isActive = (i == escenasManager.escenaActivaIndex);
        ImGui::PushStyleColor(ImGuiCol_ChildBg,
            isActive ? ImVec4(0.25f, 0.45f, 0.75f, 1.f) : ImVec4(0.18f, 0.18f, 0.18f, 1.f));

        std::string cardId = "SceneCard##" + std::to_string(i);
        ImGui::BeginChild(cardId.c_str(), ImVec2(140.f, panelH * 0.50f), ImGuiChildFlags_Border);

        if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0))
            escenasManager.escenaActivaIndex = i;

        ImGui::PushFont(fontTexto);
        ImGui::TextUnformatted(escena.name.c_str());
        ImGui::TextDisabled("%d nodo(s)", (int)escena.objects.size());
        ImGui::PopFont();

        if ((int)escenasManager.escenas.size() > 1)
        {
            ImGui::SetCursorPosY(panelH * 0.50f - 28.f);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.65f, 0.15f, 0.15f, 1.f));
            if (ImGui::Button(("Eliminar##Scene" + std::to_string(i)).c_str(), ImVec2(-1, 22)))
                sceneToDelete = i;
            ImGui::PopStyleColor();
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();
        ImGui::SameLine();
    }

    if (sceneToDelete >= 0)
    {
        escenasManager.escenas.erase(escenasManager.escenas.begin() + sceneToDelete);
        int& a = escenasManager.escenaActivaIndex;
        if (a >= (int)escenasManager.escenas.size()) a = (int)escenasManager.escenas.size() - 1;
        if (a < 0) a = 0;
    }
    ImGui::End();
}

void Motor::DrawNodosPanel(float panelH, int& newNodeCounter)
{
    ImGuiIO& io = ImGui::GetIO();
    float pw = io.DisplaySize.x * 0.15f;
    float ph = io.DisplaySize.y - panelH;
    ImGui::SetNextWindowPos({ io.DisplaySize.x - pw, 0 });
    ImGui::SetNextWindowSize({ pw, ph });
    ImGui::Begin("Nodos", nullptr,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    ImGui::PushFont(fontTitulo);
    ImGui::Text("Nodos");
    ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 30.f);
    if (ImGui::Button("+##Afeixir Nodo", ImVec2(28, 28)))
    {
        if (Escena* e = escenasManager.GetEscenaActiva())
            e->AñadirNodo("Nodo " + std::to_string(newNodeCounter++), { 100,100 }, { 64,64 });
    }
    ImGui::Separator();
    ImGui::PopFont();

    Escena* cs = escenasManager.GetEscenaActiva();
    if (!cs) { ImGui::End(); return; }

    int nodeToDelete = -1;
    for (int i = 0; i < (int)cs->objects.size(); ++i)
    {
        Nodo& n = cs->objects[i];
        bool sel = (n.id == selectedNodeId);
        ImGui::PushStyleColor(ImGuiCol_ChildBg,
            sel ? ImVec4(0.25f, 0.55f, 0.35f, 1.f) : ImVec4(0.18f, 0.18f, 0.18f, 1.f));

        std::string cid = "NodeCard##" + std::to_string(n.id);
        ImGui::BeginChild(cid.c_str(), ImVec2(-1, 62.f), ImGuiChildFlags_Border);

        if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0))
            selectedNodeId = n.id;

        ImGui::PushFont(fontTexto);
        ImGui::TextUnformatted(n.name.c_str());
        ImGui::TextDisabled("(%.0f,%.0f) z:%d", n.position.x, n.position.y, n.zOrder);
        ImGui::PopFont();

        ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 22.f);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetTextLineHeightWithSpacing());
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.65f, 0.15f, 0.15f, 1.f));
        if (ImGui::Button(("X##N" + std::to_string(n.id)).c_str(), ImVec2(20, 20)))
            nodeToDelete = n.id;
        ImGui::PopStyleColor();

        ImGui::EndChild();
        ImGui::PopStyleColor();
    }

    if (nodeToDelete >= 0)
    {
        if (selectedNodeId == nodeToDelete) selectedNodeId = -1;
        cs->EliminarNodo(nodeToDelete);
    }

    if (selectedNodeId >= 0)
    {
        ImGui::Separator();
        for (auto& n : cs->objects)
        {
            if (n.id != selectedNodeId) continue;
            ImGui::PushFont(fontTexto);
            ImGui::Text("Inspector: %s", n.name.c_str());
            ImGui::DragFloat2("Pos", &n.position.x, 1.f);
            ImGui::DragFloat2("Tamany", &n.size.x, 1.f);
            // No funciona v
            ImGui::DragFloat("Rot?X", &n.rotation, 1.f);
            ImGui::DragInt("Z", &n.zOrder, 1);
            ImGui::DragInt("Capa", &n.layer, 1);
            ImGui::Checkbox("Actiu?", &n.active);

            ImGui::Checkbox("Colisio?", &n.hasCollider);
            if (n.hasCollider)
            {
                int ct = (int)n.collider.type;
                ImGui::RadioButton("Cercle", &ct, 0); ImGui::SameLine();
                ImGui::RadioButton("Box", &ct, 1);
                n.collider.type = (ColliderType)ct;
                if (n.collider.type == ColliderType::Circle)
                    ImGui::DragFloat("Radio", &n.collider.radius, 1.f, 1.f, 500.f);
            }
            ImGui::PopFont();
            break;
        }
    }

    ImGui::End();
}

int Motor::Init()
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
    {
        printf("Error: SDL_Init(): %s\n", SDL_GetError());
        return -1;
    }

    main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE |
        SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;

    window = SDL_CreateWindow("ElMotorcito",
        (int)(1280 * main_scale), (int)(800 * main_scale), window_flags);
    if (!window) { printf("SDL_CreateWindow: %s\n", SDL_GetError()); return -1; }

    renderer = SDL_CreateRenderer(window, nullptr);
    SDL_SetRenderVSync(renderer, 1);
    if (!renderer) { SDL_Log("SDL_CreateRenderer: %s\n", SDL_GetError()); return 1; }

    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(window);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;

    fontTitulo = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\arial.ttf", 24.f);
    fontTexto = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\arial.ttf", 16.f);

    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    Escena& e1 = escenasManager.AfegirEscena("Escena 1");
    auto& n = e1.AñadirNodo("Test", { 100,100 }, { 64,64 });
    n.hasCollider = true;
    n.collider.type = ColliderType::Box;
    escenasManager.escenaActivaIndex = 0;

    return 0;
}

void Motor::Run()
{
    int newSceneCounter = 2, newNodeCounter = 2;
    ImGuiIO& io = ImGui::GetIO();
    ImVec4 clear_color = { 0.15f, 0.15f, 0.18f, 1.f };

    running = true;
    while (running)
    {
        input.BeginFrame();

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);
            input.ProcessEvent(event);
            if (event.type == SDL_EVENT_QUIT) running = false;
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
                event.window.windowID == SDL_GetWindowID(window))
                running = false;
        }

        if (input.IsKeyPressed(SDLK_ESCAPE)) running = false;

        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        float escenasH = 0.f;
        DrawEscenasPanel(escenasH, newSceneCounter);
        DrawNodosPanel(escenasH, newNodeCounter);

        ImGui::Render();
        SDL_SetRenderScale(renderer,
            io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        SDL_SetRenderDrawColorFloat(renderer,
            clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        SDL_RenderClear(renderer);

        if (Escena* e = escenasManager.GetEscenaActiva())
        {
            RenderScene(*e);

            auto pairs = DetectCollisions(*e);
            for (auto& p : pairs)
            {
                SDL_Log("Collision: %d <-> %d", p.idA, p.idB);
            }
        }

        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);

        if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED)
        {
            SDL_Delay(10); continue;
        }
    }
}

void Motor::Exit()
{
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}