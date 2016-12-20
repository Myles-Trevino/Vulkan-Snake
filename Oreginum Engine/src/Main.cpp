#include <iostream>
#include "Oreginum/Core.hpp"
#include "Oreginum/Renderer.hpp"
#include "Oreginum/Cuboid.hpp"
#include "Oreginum/Rectangle.hpp"
#include "Oreginum/Keyboard.hpp"
#include "Oreginum/Camera.hpp"

/*
	Use seperate 2D shaders for rectangles.
*/

namespace
{
	Oreginum::Cuboid board, fruit;
	std::vector<Oreginum::Cuboid> snake;
	glm::fvec3 background_color{.1f, .1f, .1f}, board_color{.15f, .15f, .15f},
		fruit_color{1.f, .1f, .3f}, snake_color{.3f, 1.f, .5f};
	enum class Direction { UP, DOWN, LEFT, RIGHT } direction, previous_direction;
	constexpr int board_size{30}, tile_size{1}, tiles{board_size/tile_size};
	float snake_timer, move_time;
}

void print_fps()
{
	static double timer{};
	static int fps{};

	++fps;
	timer += Oreginum::Core::get_delta();
	if(timer > 1)
	{
		std::cout<<fps<<" : "<<Oreginum::Core::get_delta()<<'\n';
		fps = 0, timer = 0;
	}
}

void eat()
{
	fruit.set_translation({rand()%tiles*tile_size, rand()%tiles*tile_size, 0});
	Oreginum::Renderer::clear();
	Oreginum::Renderer::add(&board);
	Oreginum::Renderer::add(&fruit);
	for(Oreginum::Cuboid& r : snake) Oreginum::Renderer::add(&r);
}

void set()
{
	snake.clear();
	snake.push_back({glm::ivec3{glm::ivec2{tiles/2*tile_size}, 0},
		glm::ivec3{tile_size}, snake_color, false});
	eat();

	direction = Direction::RIGHT;
	previous_direction = direction;
	snake_timer = 0;
	move_time = .1f;
}

int WinMain(HINSTANCE current_instance, HINSTANCE previous_instance, LPSTR arguments, int show)
{
	Oreginum::Core::initialize("Oreginum Engine Vulkan Test", {666, 666}, false, true);
	Oreginum::Camera::freeze(true);
	Oreginum::Camera::set_position(glm::fvec3{glm::fvec2{board_size/2.f}, -board_size/2.f});
	fruit = {{}, glm::ivec3{tile_size}, fruit_color, false};
	board = {{0, 0, 1}, glm::ivec3{board_size*tile_size}, board_color, false};
	set();

	while(Oreginum::Core::update())
	{
		//Controls
		if(Oreginum::Keyboard::was_pressed(Oreginum::Key::W)
			&& previous_direction != Direction::DOWN) direction = Direction::UP;
		else if(Oreginum::Keyboard::was_pressed(Oreginum::Key::S)
			&& previous_direction != Direction::UP) direction = Direction::DOWN;
		else if(Oreginum::Keyboard::was_pressed(Oreginum::Key::A)
			&& previous_direction != Direction::RIGHT) direction = Direction::LEFT;
		else if(Oreginum::Keyboard::was_pressed(Oreginum::Key::D)
			&& previous_direction != Direction::LEFT) direction = Direction::RIGHT;

		//Move snake
		if(snake_timer < move_time) snake_timer += Oreginum::Core::get_delta();
		else
		{
			//Eat fruit
			if(snake.back().get_translation() == fruit.get_translation())
			{
				move_time /= 1.01f;
				snake.push_back({snake.back().get_translation(),
					glm::ivec3{tile_size}, snake_color, false});
				eat();
			}

			//Move
			for(int i{}; i < snake.size()-1; ++i)
				snake[i].set_translation(snake[i+1].get_translation());

			if(direction == Direction::UP) snake.back().translate({0, -tile_size, 0});
			else if(direction == Direction::DOWN) snake.back().translate({0, tile_size, 0});
			else if(direction == Direction::LEFT) snake.back().translate({-tile_size, 0, 0});
			else if(direction == Direction::RIGHT) snake.back().translate({tile_size, 0, 0});

			previous_direction = direction;
			snake_timer = 0;
		}

		//Game over
		glm::fvec3 head{snake.back().get_translation()};
		if(head.x < 0 || head.x >= board_size || head.y < 0 || head.y >= board_size) set();
		for(int i{}; i < snake.size()-1; ++i) if(head == snake[i].get_translation()) set();

		print_fps();
	}

	Oreginum::Core::destroy();
}