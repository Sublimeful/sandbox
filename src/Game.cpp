#include <SDL2/SDL_keycode.h>
#include <exception>
#include <stdio.h>
#include <vector>
#include <iostream>

#include "Game.hpp"
#include "Material.hpp"

void swap(char* element_1, char* element_2) {
  char temp = *element_1;
  *element_1 = *element_2;
  *element_2 = temp;
}

void Game::render() {
  renderer->render_grid(grid);
  renderer->present();
}

void Game::update()
{
  int size = brush_size / 2;

  if(gamepad.mouseState & SDL_BUTTON(SDL_BUTTON_LEFT))
  {
    for(int i = -size; i <= size; i++)
    {
      for(int j = -size; j <= size; j++)
      {
        int r = gamepad.mouseY/size_p + i;
        int c = gamepad.mouseX/size_p + j;
        if(r >= size_h || r < 0 || c >= size_w || c < 0) break;
        grid[r][c] = current_material.type;
      }
    }
  }
  for(int r = size_h - 1; r >= 0; r--)
  {
    for(int c = 0; c < size_w; c++)
    {
      if(grid[r][c] == WATER)
      {
        flow(r, c);
        continue;
      }
      if(grid[r][c] != AIR && grid[r][c] != STONE)
      {
        drop(r, c);
      }
    }
  }
}

void Game::input()
{
  SDL_Event event;
  while(SDL_PollEvent(&event))
  {
    if(event.type == SDL_QUIT)
    {
      running = false; return;
    }
    switch(event.type)
    {
      case SDL_KEYDOWN:
        switch(event.key.keysym.sym)
        {
          case SDLK_1:
            switchMaterial(static_cast<MaterialType>(0));
            break;
          case SDLK_2:
            switchMaterial(static_cast<MaterialType>(1));
            break;
          case SDLK_3:
            switchMaterial(static_cast<MaterialType>(2));
            break;
          case SDLK_4:
            switchMaterial(static_cast<MaterialType>(3));
            break;
          case SDLK_5:
            switchMaterial(static_cast<MaterialType>(4));
            break;
          case 61: // Plus sign
            brush_size++;
            break;
          case 45: // Minus sign
            if (brush_size == 0) break;
            brush_size--;
            break;
        }
    }
  }
  gamepad.mouseState = SDL_GetMouseState(&gamepad.mouseX, &gamepad.mouseY);
}

void Game::run() {
  /* Main game loop */
  while(running)
  {
    timer->update();
    if(timer->deltaTime() >= 1.0f / 60) {
      input();
      update();
      render();
      timer->reset();
    }
  }
}

void Game::switchMaterial(MaterialType material_type) {
  current_material = materials[material_type];
}

void Game::drop(int r, int c) {
  if(r+1 >= size_h) return;
  char* current = &grid[r][c];
  char* bottom = &grid[r+1][c];
  char* bottom_left = &grid[r+1][c-1];
  char* bottom_right = &grid[r+1][c+1];
  MaterialType m_current = cast(*current);
  MaterialType m_bottom = cast(*bottom);
  MaterialType m_bottom_left = cast(*bottom_left);
  MaterialType m_bottom_right = cast(*bottom_right);
  if(bottom && weight_values[m_current] > weight_values[m_bottom]) {
    swap(bottom, current);
  }
  else if(bottom_left && weight_values[m_current] > weight_values[m_bottom_left]) {
    swap(bottom_left, current);
  }
  else if(bottom_right && weight_values[m_current] > weight_values[m_bottom_right]) {
    swap(bottom_right, current);
  }
}

void Game::flow(int r, int c) {
  if(r+1 >= size_h) return;
  if(grid[r+1][c] == AIR)
  {
    std::swap(grid[r+1][c], grid[r][c]);
  }
  if(grid[r+1][c-1] == AIR)
  {
    std::swap(grid[r+1][c-1], grid[r][c]);
  }
  if(grid[r+1][c+1] == AIR)
  {
    std::swap(grid[r+1][c+1], grid[r][c]);
  }
  if(grid[r][c+1] == AIR)
  {
    std::swap(grid[r][c+1], grid[r][c]);
  }
  if(grid[r][c-1] == AIR)
  {
    std::swap(grid[r][c-1], grid[r][c]);
  }
}

Game::Game(int display_width, int display_height) :
  display_width(display_width),
  display_height(display_height),
  size_w(display_width / size_p),
  size_h(display_height / size_p),
  grid(size_h, std::vector<char>(size_w)),
  running(true),
  brush_size(2)
{
  /* Initialize windows and stuff */
  SDL_Init(SDL_INIT_EVERYTHING);

  window = SDL_CreateWindow("Sandbox", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, display_width, display_height, 0);
  renderer = new Renderer(window, display_width, display_height, size_p, materials);
  timer = Timer::instance();

  /* Fill grid with AIR */
  for (int r = 0; r < size_h; r++) {
    for (int c = 0; c < size_w; c++) {
      grid[r].push_back(AIR);
    }
  }
  
  /* Set weight values */
  weight_values = std::unordered_map<MaterialType, int>
  ({
    {AIR, 0},
    {WATER, 1},
    {SAND, 5},
    {DIRT, 5},
    {STONE, 10}
  });

  /* Set current material */
  current_material = materials[0];
}

Game::~Game() {}