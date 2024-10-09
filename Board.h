#pragma once
#ifndef BOARD_H
#define BOARD_H

#include <vector>
#include <fstream>
#include <SFML/Graphics.hpp>

enum SWIPE_DIRECTION {
	UP = 1,
	RIGHT = 2,
	DOWN = 3,
	LEFT = 4,
	NOT_SET = 5,
};

enum MOVE_STRATEGY {
	RANDOM = 1,

};

struct BoardStatus {
	// The vector that holds the 16 numbers, index are as shown below
	// (0,0), (0,1), (0,2), (0,3)
	// (1,0), (1,1), (1,2), (1,3)
	// (2,0), (2,1), (2,2), (2,3)
	// (3,0), (3,1), (3,2), (3,3)
	std::vector<std::vector<int>> board = std::vector<std::vector<int>>(4, std::vector<int>(4, 0));
	bool is_dead = false;
	int score = 0;
	SWIPE_DIRECTION last_swipe_direction = SWIPE_DIRECTION::NOT_SET;
	// Total valid swipes
	int moves = 0;
	// The total non-zero numbers on the board_
	int total_numbers = 0;

	BoardStatus() {
		board = std::vector<std::vector<int>>(4, std::vector<int>(4, 0));
		is_dead = false;
		score = 0;
		last_swipe_direction = SWIPE_DIRECTION::NOT_SET;
		moves = 0;
		total_numbers = 0;
	}

	BoardStatus(const BoardStatus& b) {
		board = b.board;
		is_dead = b.is_dead;
		score = b.score;
		last_swipe_direction = b.last_swipe_direction;
		moves = b.moves;
		total_numbers = b.total_numbers;
	}
};

class Board {
public:
	Board(sf::RenderWindow* app, sf::Font* font, float top_left_x, float top_left_y, float window_width);

	// std::cout the board in console
	void PrintBoard(const BoardStatus& status);

	// Reset the board to the initial condition
	void ResetBoard(BoardStatus& status);

	// Swipe the board
	static void Swipe(SWIPE_DIRECTION direction, BoardStatus& status);

	// Draw tiles
	void Draw();

	// Write highest score to file. Let app close file to call this function.
	void SaveToFile();

	// Load status from file
	void LoadFromFile(BoardStatus& status);

	int GetScore(const BoardStatus& status);
	int GetBestScore();

	BoardStatus& GetCurrentBoard();

	void ToggleInputMode();

	bool IsHumanInput();

	void BotMove(BoardStatus& status);
	void HumanMove(SWIPE_DIRECTION direction, BoardStatus& status);

	void ChangeBotMoveInterval(float interval);
	float GetBotMoveInterval();

private:
	// Return where to put the new number as a pair of row, column index
	static std::pair<int, int> DetermineNewNumberPosition(const BoardStatus& status);

	// Merge to the right(back or end) on a 1-d vector.
	// Merge to the left can be done 1) flip vector, 2) MergeToBack, 3) flip vector again
	static void MergeToBack(std::vector<int>& vector, BoardStatus& status);

	// Check if a vector can be merged to back.
	static bool CanMergeToBack(const std::vector<int>& vector);

	// Rewrite the vector in back order
	static void FlipVector(std::vector<int>& vector);

	// Transform the vector
	static void Transform2DVector(std::vector<std::vector<int>>& matrix);

	// Return the tile fill color according to the number
	sf::Color GetTileColor(int number);

	// Return the text fill color according to the number
	sf::Color GetTextColor(int number);

	// Return the text size according to the number
	int GetTextSize(int number);

	// Update hightest score in the record file
	void UpdateHighestScoreInFile();

	// Add a new number to the board. Called inside SWIPE() function.
	static void AddNewNumber(BoardStatus& status);

	// Check if a swipe in the direction on a BoardStatus is allowed
	static bool CanSwipeInDirection(SWIPE_DIRECTION direction, const BoardStatus& status);

	// Return all possible moves of the given status in a vector
	static std::vector<SWIPE_DIRECTION> FindPossibleMoves(const BoardStatus& status);

	// Simulate a given board status till end using random moves. For thread use.
	static void SimulateDirectionScore(BoardStatus status,
		                               SWIPE_DIRECTION direction,
		                               std::vector<int>& direction_scores,
		                               int thread_id,
		                               int num_of_simulations);

private:
	// Tiles display
	std::vector<sf::RectangleShape> tiles_;

	// Texts to display numbers
	std::vector<sf::Text> texts_;

	// Window pointer
	sf::RenderWindow* app_;

	// Font pointer
	sf::Font* font_;

	// Highest score ever
	int highest_score_ = 0;

	// Board top left x
	float top_left_x_;

	// Board top left y
	float top_left_y_;

	// Window width, each tile should be 1/4 of window width
	float window_width_;

	// BoardStatus object
	BoardStatus status_;

	// Is input mode == human?
	bool is_human_input_ = true;

	// Bot move time interval in seconds
	float bot_move_interval_ = 1.f;

	// Bot move strategy
	MOVE_STRATEGY bot_move_strategy_ = MOVE_STRATEGY::RANDOM;
};

#endif // BOARD_H