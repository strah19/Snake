#include "Ember/Ember.h"
#include "Ember/Core/Application.h"
#include "Ember/Animation/Spritesheet.h"
#include "Ember/Assets/Font.h"

#include <list>
#include <chrono>
#include <random>

const int snake_grid_width = 16;
const int snake_grid_height = 16;

const int grid_width_size = 32;
const int grid_height_size = 32;

class Snake : public Ember::Application {
public:
	void OnCreate() {
		CreateHead();
		PushToSnake();
		GiveFoodNewLocation();

		text.Initialize(renderer, "res/Ubuntu-Regular.ttf", 24);
		texture.Initialize("res/snake-graphics.png", renderer);
		spritesheet.Initialize(texture, 4, 5);
		spritesheet.SelectSprite(2, 1);
	}

	virtual ~Snake() { 	}

	void PushToSnake() {
		snake.push_back(SnakePiece(&snake.back(), snake.back().last_position, snake.back().past_vel));
	}

	void CreateHead() {
		snake.push_back(SnakePiece(NULL, { (snake_grid_width * grid_width_size) / 2, (snake_grid_height * grid_height_size) / 2 }, velocity));
		head = &snake.front();
	}

	void GiveFoodNewLocation() {
		std::random_device dev;
		std::mt19937 rng(dev());

		current_food_location = head->position;
		while (current_food_location == head->position) {
			std::uniform_int_distribution<std::mt19937::result_type> random_loc(0, snake_grid_width - 1);
			current_food_location.x = random_loc(rng) * grid_width_size;

			random_loc = std::uniform_int_distribution<std::mt19937::result_type>(0, snake_grid_height - 1);
			current_food_location.y = random_loc(rng) * grid_height_size;
		}
	}

	bool CheckIfHeadHitsBody() {
		for (auto& s : snake) {
			if (s.parent != NULL) {
				if (head->position == s.position)
					return true;
			}
		}
		return false;
	}

	bool CheckBorders() {
		return (head->position.x < 0 || head->position.y < 0 ||
			head->position.x >= (snake_grid_width * grid_width_size) || head->position.y >= (snake_grid_height * grid_height_size));
	}

	void OnUserUpdate() {
		window->Update();

		if (animation_counter % animation_delay == 0) {
			head->last_position = head->position;
			head->position += velocity;
			head->past_vel = head->vel;
			head->vel = velocity;
			if (!trickle_down)
				food_animation++;
			if (trickle_down)
				food_animation--;

			for (auto& s : snake) {
				s.Update();
			}

			animation_counter = 0;
		}

		if (food_animation > max_food)
			trickle_down = true;

		if (food_animation == 0)
			trickle_down = false;

		animation_counter++;

		game_over = CheckBorders() || CheckIfHeadHitsBody();

		if (head->position == current_food_location) {
			GiveFoodNewLocation();
			score++;
			PushToSnake();
		}

		if (game_over) {
			Restart();
		}

		renderer->Clear(background_color);

		bool light_square = false;
		for (int i = 0; i < snake_grid_width; i++) {
			for (int j = 0; j < snake_grid_height; j++) {
				Ember::Color color = (light_square) ? Ember::Color({50, 170, 0, 255}) : Ember::Color({100, 200, 0, 255});
				renderer->Rectangle(Ember::Rect({ i * grid_width_size, j * grid_height_size, grid_width_size, grid_height_size }), color);
				light_square = !light_square;
			}
			light_square = !light_square;
		}

		std::stringstream ss;
		ss << "Score: ";
		ss << score;
		text.UpdateText(ss.str());
		text.SetPosition(properties->width / 2 - (text.GetSize().x / 2), 10);
		text.UpdateColor({ 255, 255, 255, 255 });
		text.Render();

		int snake_size = 0;
		for (auto& s : snake) {
			if (s.vel.x == 0)
				spritesheet.SelectSprite(2, 1);
			else if (s.vel.y == 0)
				spritesheet.SelectSprite(1, 0);

			if (s.parent) {
				if (s.parent->vel != s.vel) {
					if ((s.vel.x < 0 && s.parent->vel.y > 0) || (s.vel.y < 0 && s.parent->vel.x > 0))
						spritesheet.SelectSprite(0, 0);
					else if ((s.vel.x > 0 && s.parent->vel.y > 0) || (s.vel.y < 0 && s.parent->vel.x < 0))
						spritesheet.SelectSprite(2, 0);
					else if ((s.vel.x > 0 && s.parent->vel.y < 0) || (s.vel.y > 0 && s.parent->vel.x < 0))
						spritesheet.SelectSprite(2, 2);
					else if ((s.vel.x < 0 && s.parent->vel.y < 0) || (s.vel.y > 0 && s.parent->vel.x > 0))
						spritesheet.SelectSprite(0, 1);
				}
				if (snake_size == snake.size() - 1) {
					if (snake.back().parent->vel == Ember::IVec2(velocity_values.x, 0))
						spritesheet.SelectSprite(4, 2);
					else if (snake.back().parent->vel == Ember::IVec2(-velocity_values.x, 0))
						spritesheet.SelectSprite(3, 3);
					else if (snake.back().parent->vel == Ember::IVec2(0, velocity_values.y))
						spritesheet.SelectSprite(4, 3);
					else if (snake.back().parent->vel == Ember::IVec2(0, -velocity_values.y))
						spritesheet.SelectSprite(3, 2);
				}
			}
			else {
				if (s.vel == Ember::IVec2(velocity_values.x, 0))
					spritesheet.SelectSprite(4, 0);
				else if (s.vel == Ember::IVec2(-velocity_values.x, 0))
					spritesheet.SelectSprite(3, 1);
				else if (s.vel == Ember::IVec2(0, velocity_values.y))
					spritesheet.SelectSprite(4, 1);
				else if (s.vel == Ember::IVec2(0, -velocity_values.y))
					spritesheet.SelectSprite(3, 0);
			}

			texture.Draw(Ember::Rect({ s.position.x, s.position.y, grid_width_size, grid_height_size }), spritesheet.ReturnSourceRect());
			snake_size++;
		}

		spritesheet.SelectSprite(0, 3);
		texture.Draw(Ember::Rect({ current_food_location.x - food_animation / 2, current_food_location.y - food_animation / 2, 
			grid_width_size + food_animation, grid_height_size + food_animation }), spritesheet.ReturnSourceRect());

		renderer->Show();
	}

	void Restart() {
		velocity = { 0, velocity_values.y };
		score = 0;
		snake.clear();
		CreateHead();
		PushToSnake();
		game_over = false;
	}

	bool Keyboard(Ember::KeyboardEvents& keyboard) {
		if (keyboard.scancode == Ember::EmberKeyCode::Escape) {
			window->Quit();		
		}
		else if (keyboard.scancode == Ember::EmberKeyCode::D)
			velocity = { velocity_values.x, 0 };
		else if (keyboard.scancode == Ember::EmberKeyCode::A)
			velocity = { -velocity_values.x, 0 };
		else if (keyboard.scancode == Ember::EmberKeyCode::W)
			velocity = { 0, -velocity_values.y };
		else if (keyboard.scancode == Ember::EmberKeyCode::S)
			velocity = { 0, velocity_values.y };
		return true;
	}

	void Log() {
		system("CLS");
		for (auto& s : snake) {
			std::cout << "Snake Piece: " << std::endl;
			std::cout << s.position << std::endl;
			std::cout << "Address: " << &s << std::endl;
			std::cout << "Parent: " << &s.parent << std::endl;
		}
	}

	void UserDefEvent(Ember::Event& event) {
		Ember::EventDispatcher dispatch(&event);
		dispatch.Dispatch<Ember::KeyboardEvents>(EMBER_BIND_FUNC(Keyboard));
	}
private:
	struct SnakePiece {
		SnakePiece* parent = NULL;
		Ember::IVec2 position;
		Ember::IVec2 last_position;
		Ember::IVec2 vel;
		Ember::IVec2 past_vel;

		SnakePiece(SnakePiece* parent, const Ember::IVec2& position, const Ember::IVec2& vel) : parent(parent), position(position), last_position(position), vel(vel), past_vel(vel) {  }

		void Update() {
			if (parent != NULL) {
				last_position = position;
				position = parent->last_position;
				past_vel = vel;
				vel = parent->past_vel;
			}
		}
	};
	std::list<SnakePiece> snake;
	SnakePiece* head = nullptr;

	Ember::Color background_color = { 0, 0, 0, 255 };

	Ember::IVec2 current_food_location = { 0, 0 };
	bool game_over = false;

	int animation_delay = 10;
	int animation_counter = 0;
	Ember::IVec2 velocity_values = { (snake_grid_width * grid_width_size) / snake_grid_width, (snake_grid_height * grid_height_size) / snake_grid_height };
	Ember::IVec2 velocity = { 0, -velocity_values.y };

	Ember::Texture texture;
	Ember::SpriteSheet spritesheet;

	int food_animation = 0;
	const int max_food = 15;
	bool trickle_down = false;

	int score = 0;
	Ember::Font text;
};

int main(int argc, char** argv) {
	Snake snake;

	snake.Initialize("Snake", false, (snake_grid_width * grid_width_size), (snake_grid_height * grid_height_size));
	snake.Run();

	return 0;
}