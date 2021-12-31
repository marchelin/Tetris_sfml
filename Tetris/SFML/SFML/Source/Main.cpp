#include <iostream>
#include <chrono>
#include <random>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include "Headers/DrawText.hpp"
#include "Headers/Global.hpp"
#include "Headers/GetTetromino.hpp"
#include "Headers/GetWallKickData.hpp"
#include "Headers/Tetromino.hpp"

int main()
{
	bool game_over = 0;
	bool hard_drop_pressed = 0;
	bool rotate_pressed = 0;

	unsigned lag = 0;

	unsigned lines_cleared = 0;
	unsigned char clear_effect_timer = 0;

	unsigned char v_timer = 0;
	unsigned char h_timer = 0;

	unsigned char soft_drop_timer = 0;
	unsigned char current_fall_speed = START_FALL_SPEED;

	unsigned char next_obj;

	std::chrono::time_point<std::chrono::steady_clock> previous_time;

	std::random_device random_device;

	std::default_random_engine random_engine(random_device());

	std::uniform_int_distribution<unsigned short> shape_distribution(0, 6);

	std::vector<bool> clear_lines(ROWS, 0);

	//All the colors for the cells
	std::vector<sf::Color> cell_colors = {

		sf::Color( 38,  38,  38), // #262626 ---> Balck Background

		sf::Color(  3,  65, 174), // #0341AE ---> Cobalt Blue
		sf::Color(114, 203,  59), // #72CB3B ---> Green Apple
		sf::Color(255, 213,   0), // #FFD500 ---> Cyber Yellow
		sf::Color(255, 151,  28), // #FF971C ---> Orange Beer
		sf::Color(255,  50,  19), // #FF3213 ---> RYB Red
		sf::Color(  0, 255, 255), // #00ffff ---> Cyan
		sf::Color(128,   0, 128), // #800080 ---> Purple

		sf::Color(127, 127, 127)  // #7f7f7f ---> Grey Shape Position
	};

	// Game matrix
	std::vector<std::vector<unsigned char>> matrix(COLUMNS, std::vector<unsigned char>(ROWS));

	sf::Event event;

	sf::RenderWindow window(sf::VideoMode(2 * CELL_SIZE * COLUMNS * SCREEN_RESIZE, CELL_SIZE * ROWS * SCREEN_RESIZE), "Tetris", sf::Style::Close);
	window.setView(sf::View(sf::FloatRect(0, 0, 2 * CELL_SIZE * COLUMNS, CELL_SIZE * ROWS)));
	window.setFramerateLimit(60);

	// Text
	sf::Text score;
	sf::Font font;
	font.loadFromFile("Resources/Fonts/Pixeled.ttf");
	score.setFont(font);
	score.setCharacterSize(15);
	score.setFillColor(sf::Color::White);
	score.setPosition(10, 25);
	sf::Vector2<float> score_scale(1.5f, 1.5f);
	score.setScale(score_scale);
	score.setString("Lines: 0");

	// Music
	sf::Music music;
	if (music.openFromFile("Resources/Sound/Music/TetrisTheme.wav"))
	{
		music.setVolume(25.f);
		music.setLoop(true);
		music.play();
	}

	// Sound FX
	sf::SoundBuffer buffer;
	sf::Sound sound;
	if (buffer.loadFromFile("Resources/Sound/FX/line.wav"))
	{
		sound.setBuffer(buffer);
	}

	// Tetromino Random Shape
	Tetromino tetromino(static_cast<unsigned char>(shape_distribution(random_engine)), matrix);

	// Generate random shapes
	next_obj = static_cast<unsigned char>(shape_distribution(random_engine));

	// The current clock time
	previous_time = std::chrono::steady_clock::now();

	while (1 == window.isOpen())
	{
		unsigned delta_time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - previous_time).count();
		lag += delta_time;
		previous_time += std::chrono::microseconds(delta_time);

		while (FRAME_DURATION <= lag)
		{
			lag -= FRAME_DURATION;

			while (1 == window.pollEvent(event))
			{
				switch (event.type)
				{
					case sf::Event::Closed:
					{
						window.close(); // Close window

						break;
					}
					case sf::Event::KeyReleased:
					{
						switch (event.key.code)
						{
							case sf::Keyboard::C:
							case sf::Keyboard::Z:
							{
								rotate_pressed = 0;

								break;
							}
							case sf::Keyboard::Down:
							{
								soft_drop_timer = 0;

								break;
							}
							case sf::Keyboard::Left:
							case sf::Keyboard::Right:
							{
								h_timer = 0;

								break;
							}
							case sf::Keyboard::Space:
							{
								hard_drop_pressed = 0;
								sound.play();
							}
						}
					}
				}
			}

			if (0 == clear_effect_timer)
			{
				if (0 == game_over)
				{
					if (0 == rotate_pressed)
					{
						if (1 == sf::Keyboard::isKeyPressed(sf::Keyboard::C)) // Rotate Right
						{
							rotate_pressed = 1;

							tetromino.rotate(1, matrix);
						}
						else if (1 == sf::Keyboard::isKeyPressed(sf::Keyboard::Z)) // Rotate Left
						{
							rotate_pressed = 1;

							tetromino.rotate(0, matrix);
						}
					}

					if (0 == h_timer)
					{
						if (1 == sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) // Move Left
						{
							h_timer = 1;
							tetromino.move_left(matrix);
						}
						else if (1 == sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) // Move Right
						{
							h_timer = 1;
							tetromino.move_right(matrix);
						}
					}
					else
					{
						h_timer = (1 + h_timer) % MOVE_SPEED;
					}

					if (0 == hard_drop_pressed)
					{
						if (1 == sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
						{
							hard_drop_pressed = 1;

							v_timer = current_fall_speed;

							tetromino.hard_drop(matrix);
						}
					}

					if (0 == soft_drop_timer)
					{
						if (1 == sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
						{
							if (1 == tetromino.move_down(matrix))
							{
								v_timer = 0;
								soft_drop_timer = 1;
							}
						}
					}
					else
					{
						soft_drop_timer = (1 + soft_drop_timer) % SOFT_DROP_SPEED;
					}

					if (current_fall_speed == v_timer)
					{
						if (0 == tetromino.move_down(matrix))
						{
							tetromino.update_matrix(matrix);

							for (unsigned char a = 0; a < ROWS; a++)
							{
								bool clear_line = 1;

								for (unsigned char b = 0; b < COLUMNS; b++)
								{
									if (0 == matrix[b][a])
									{
										clear_line = 0;

										break;
									}
								}

								if (1 == clear_line)
								{
									lines_cleared++; // Increase score

									clear_effect_timer = CLEAR_EFFECT_DURATION;

									clear_lines[a] = 1;

									if (0 == lines_cleared % LINES_TO_INCREASE_SPEED)
									{
										// Increase game speed
										current_fall_speed = std::max<unsigned char>(SOFT_DROP_SPEED, current_fall_speed - 1);
									}
								}
							}

							if (0 == clear_effect_timer)
							{
								game_over = 0 == tetromino.reset(next_obj, matrix);

								// Generate shape
								next_obj = static_cast<unsigned char>(shape_distribution(random_engine));
							}
						}

						v_timer = 0;
					}
					else
					{
						v_timer++;
					}
				}
				else if (1 == sf::Keyboard::isKeyPressed(sf::Keyboard::Enter))
				{
					game_over = 0;
					hard_drop_pressed = 0;
					rotate_pressed = 0;

					lines_cleared = 0;

					current_fall_speed = START_FALL_SPEED;
					v_timer = 0;
					h_timer = 0;
					soft_drop_timer = 0;

					// Clear the matrix
					for (std::vector<unsigned char>& a : matrix)
					{
						std::fill(a.begin(), a.end(), 0);
					}
				}
			}
			else
			{
				clear_effect_timer--;

				if (0 == clear_effect_timer)
				{
					for (unsigned char a = 0; a < ROWS; a++)
					{
						if (1 == clear_lines[a])
						{
							for (unsigned char b = 0; b < COLUMNS; b++)
							{
								matrix[b][a] = 0;

								for (unsigned char c = a; 0 < c; c--)
								{
									matrix[b][c] = matrix[b][c - 1];
									matrix[b][c - 1] = 0;
								}
							}
						}
					}

					game_over = 0 == tetromino.reset(next_obj, matrix);

					next_obj = static_cast<unsigned char>(shape_distribution(random_engine));

					std::fill(clear_lines.begin(), clear_lines.end(), 0);
				}
			}

			if (FRAME_DURATION > lag)
			{
				unsigned char clear_cell_size = static_cast<unsigned char>(2 * round(0.5f * CELL_SIZE * (clear_effect_timer / static_cast<float>(CLEAR_EFFECT_DURATION))));


				sf::RectangleShape cell(sf::Vector2f(CELL_SIZE - 1, CELL_SIZE - 1));
				sf::RectangleShape preview_border(sf::Vector2f(5 * CELL_SIZE, 5 * CELL_SIZE));

				preview_border.setFillColor(sf::Color(0, 0, 0));
				preview_border.setOutlineThickness(-1);
				preview_border.setPosition(CELL_SIZE * (1.5f * COLUMNS - 2.5f), CELL_SIZE * (0.25f * ROWS - 2.5f));

				window.clear(); // Clear window

				// Draw matrix
				for (unsigned char a = 0; a < COLUMNS; a++)
				{
					for (unsigned char b = 0; b < ROWS; b++)
					{
						if (0 == clear_lines[b])
						{
							cell.setPosition(static_cast<float>(CELL_SIZE * a), static_cast<float>(CELL_SIZE * b));

							if (1 == game_over && 0 < matrix[a][b])
							{
								cell.setFillColor(cell_colors[8]);
							}
							else
							{
								cell.setFillColor(cell_colors[matrix[a][b]]);
							}

							window.draw(cell);
						}
					}
				}

				// Set to grey cells background color
				cell.setFillColor(cell_colors[8]);

				if (0 == game_over)
				{
					// Drawing ghost tetrominos
					for (Position& mino : tetromino.get_ghost_minos(matrix))
					{
						cell.setPosition(static_cast<float>(CELL_SIZE * mino.x), static_cast<float>(CELL_SIZE * mino.y));
						window.draw(cell);
					}

					cell.setFillColor(cell_colors[1 + tetromino.get_shape()]);
				}

				// Drawing falling tetrominos
				for (Position& mino : tetromino.get_minos())
				{
					cell.setPosition(static_cast<float>(CELL_SIZE * mino.x), static_cast<float>(CELL_SIZE * mino.y));
					
					window.draw(cell);
				}

				// Drawing the clearing effect
				for (unsigned char a = 0; a < COLUMNS; a++)
				{
					for (unsigned char b = 0; b < ROWS; b++)
					{
						if (1 == clear_lines[b])
						{
							cell.setFillColor(cell_colors[0]);
							cell.setPosition(static_cast<float>(CELL_SIZE * a), static_cast<float>(CELL_SIZE * b));
							cell.setSize(sf::Vector2f(CELL_SIZE - 1, CELL_SIZE - 1));

							window.draw(cell);

							cell.setFillColor(sf::Color(255, 255, 255));
							cell.setPosition(floor(CELL_SIZE * (0.5f + a) - 0.5f * clear_cell_size), floor(CELL_SIZE * (0.5f + b) - 0.5f * clear_cell_size));
							cell.setSize(sf::Vector2f(clear_cell_size, clear_cell_size));

							window.draw(cell);
						}
					}
				}

				cell.setFillColor(cell_colors[1 + next_obj]);
				cell.setSize(sf::Vector2f(CELL_SIZE - 1, CELL_SIZE - 1));

				// Drawing the preview square
				window.draw(preview_border);

				for (Position& mino : get_tetromino(next_obj, static_cast<unsigned char>(1.5f * COLUMNS), static_cast<unsigned char>(0.25f * ROWS)))
				{
					unsigned short next_tetromino_x = CELL_SIZE * mino.x;
					unsigned short next_tetromino_y = CELL_SIZE * mino.y;

					if (0 == next_obj)
					{
						next_tetromino_y += static_cast<unsigned char>(round(0.5f * CELL_SIZE));
					}
					else if (3 != next_obj)
					{
						next_tetromino_x -= static_cast<unsigned char>(round(0.5f * CELL_SIZE));
					}

					cell.setPosition(next_tetromino_x, next_tetromino_y);

					window.draw(cell);
				}

				// Drawing text
				draw_text(static_cast<unsigned short>(CELL_SIZE * (0.5f + COLUMNS)), static_cast<unsigned short>(0.5f * CELL_SIZE * ROWS), "Lines:" + std::to_string(lines_cleared) + "\nSpeed:" + std::to_string(START_FALL_SPEED / current_fall_speed) + 'x', window);
				
				window.display();
			}
		}
	}
}