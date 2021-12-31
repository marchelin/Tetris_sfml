#pragma once

// Grid size
constexpr unsigned char ROWS = 20;
constexpr unsigned char COLUMNS = 10;
constexpr unsigned char CELL_SIZE = 5;
constexpr unsigned char SCREEN_RESIZE = 7;

constexpr unsigned char MOVE_SPEED = 10;
constexpr unsigned char SOFT_DROP_SPEED = 5;
constexpr unsigned char START_FALL_SPEED = 30;

constexpr unsigned char CLEAR_EFFECT_DURATION = 10;
constexpr unsigned char LINES_TO_INCREASE_SPEED = 5;

constexpr unsigned short FRAME_DURATION = 16667;

struct Position
{
	char x;
	char y;
};