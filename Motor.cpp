#include "Motor.h"
#include "Vector.h"

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include <stdio.h>
#include <unordered_map>
#include <sstream>



#define SDL_RenderFillRectF SDL_RenderFillRect

#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

using namespace ElMotorcito;

Motor& Motor::Instance()
{
    static Motor instance;
    return instance;
}

int Motor::Init() {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
    {
        printf("Error: SDL_Init(): %s\n", SDL_GetError());
        return -1;
    }

    main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;

    window = SDL_CreateWindow("Dear ImGui SDL3+OpenGL3 example", (int)(1280 * main_scale), (int)(800 * main_scale), window_flags);
    if (window == nullptr)
    {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return -1;
    }

    renderer = SDL_CreateRenderer(window, nullptr);
    SDL_SetRenderVSync(renderer, 1);
    if (renderer == nullptr)
    {
        SDL_Log("Error: SDL_CreateRenderer(): %s\n", SDL_GetError());
        return 1;
    }

    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(window);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;

    fontTitulo = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\arial.ttf", 24.0f);
    fontTexto = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\arial.ttf", 16.0f);

    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    Escena& e1 = escenasManager.AfegirEscena("Escena 1");
    e1.AñadirNodo("Nodo Test", ImVec2(100, 100), ImVec2(64, 64));
    escenasManager.escenaActivaIndex = 0;

    return 0;
}

void Motor::Run() {

	bool escenas = true;
    bool nodos = true;

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    running = true;
    while (running) {

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT)
                running = false;
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window))
                running = false;


        }


        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        if (escenas)
        {
            float widthEscenas = io.DisplaySize.x;
            float heightEscenas = io.DisplaySize.y * 0.25f;

            ImVec2 pos = ImVec2(0, io.DisplaySize.y - heightEscenas);
            ImVec2 size = ImVec2(widthEscenas, heightEscenas);

            ImGui::SetNextWindowPos(pos);
            ImGui::SetNextWindowSize(size);
            ImGui::Begin("Escenas", nullptr, ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoCollapse);

            ImGui::PushFont(fontTitulo);

            ImGui::Text("Escenas");
            ImGui::SameLine();


            float buttonWidth = 30.0f;
            float buttonHeight = 30.0f;
            float spacing = ImGui::GetStyle().ItemSpacing.x;

            ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - buttonWidth - spacing);

            if (ImGui::Button("+", ImVec2(buttonWidth, buttonHeight)))
            {
                Escena* currentScene = escenasManager.GetEscenaActiva();
                if (currentScene)
                {
                    Nodo& nuevo = currentScene->AñadirNodo("Nuevo nodo", ImVec2(100, 100), ImVec2(64, 64));
                }
            }

            ImGui::Separator();

            ImGui::PopFont();

            Escena* currentScene = escenasManager.GetEscenaActiva();

            ImGui::End();
        }
        
        if (nodos)
        {
            float widthNodos = io.DisplaySize.x * 0.15f;
            float heightNodos = io.DisplaySize.y - (io.DisplaySize.y * 0.25f);

            ImVec2 pos = ImVec2(io.DisplaySize.x - widthNodos, 0);
            ImVec2 size = ImVec2(widthNodos, heightNodos);

            ImGui::SetNextWindowPos(pos);
            ImGui::SetNextWindowSize(size);


            ImGui::Begin("Nodos", nullptr, ImGuiWindowFlags_NoResize | 
                                            ImGuiWindowFlags_NoMove |
                                            ImGuiWindowFlags_NoCollapse);

            ImGui::PushFont(fontTitulo);

            ImGui::Text("Nodos");
            ImGui::SameLine();

            float buttonWidth = 30.0f;
            float buttonHeight = 30.0f;
            float spacing = ImGui::GetStyle().ItemSpacing.x;

            ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - buttonWidth - spacing);

            if (ImGui::Button("+", ImVec2(buttonWidth, buttonHeight)))
            {
                if (Escena* e = escenasManager.GetEscenaActiva())
                    e->AñadirNodo("Nodo nou", ImVec2(100, 100), ImVec2(64, 64));
            }

            ImGui::Separator();

            ImGui::PopFont();

            ImGui::Text("Nodos");

            ImGui::End();
        }

        // [ Rendering ]
        ImGui::Render();
        SDL_SetRenderScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        SDL_SetRenderDrawColorFloat(renderer, clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        SDL_RenderClear(renderer);

        // [ Objects Rendering ]

        // [ ImGui Rendering ]
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);

        // [ Delay ]
        // [If using SDL_MAIN_USE_CALLBACKS: all code below would likely be your SDL_AppIterate() function]
        if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED)
        {
            SDL_Delay(10);
            continue;
        }
    }
}

void Motor::Exit() {

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
