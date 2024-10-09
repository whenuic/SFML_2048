#include "Board.h"
#include <iostream>
#include <thread>


// Record file name.
const std::string kRecordFileName = "Record.txt";

Board::Board(sf::RenderWindow* app, sf::Font* font, float top_left_x, float top_left_y, float window_width) :
	app_(app),
	font_(font),
	top_left_x_(top_left_x),
	top_left_y_(top_left_y),
	window_width_(window_width),
	bot_move_interval_(0.1f) {

	// BoardStatus tmp_status = status_;
	
	sf::RectangleShape tile(sf::Vector2f(window_width_/4, window_width_/4));
	tile.setFillColor(sf::Color(205,193,180,255));
	tile.setOutlineThickness(-10);
	tile.setOutlineColor(sf::Color(187, 173, 160, 255));
	tile.setOrigin(window_width_/8, window_width_/8);
	tile.setPosition(window_width_/8, window_width_/8);
	for (int i = 0; i < 4 * 4; i++) {
		tiles_.push_back(tile);
	}
	
	sf::Text text;
	text.setFont(*font_);
	text.setFillColor(sf::Color::Red);
	text.setCharacterSize(120);
	text.setString("0");
	for (int i = 0; i < 4 * 4; i++) {
		texts_.push_back(text);
	}

	std::ifstream record_file(kRecordFileName);
	record_file >> highest_score_;
	record_file.close();
	ResetBoard(GetCurrentBoard());
}

void Board::UpdateHighestScoreInFile() {
	// Step 1: copy record file content to temp variable
	std::ifstream record_file(kRecordFileName);
	int highest_score_copy;
	std::vector<std::vector<int>> board_copy(4, std::vector<int>(4, 0));
	int score_copy;
	int total_numbers_copy;
	int swipe_copy;
	int moves_copy;
	int is_dead_copy;

	record_file >> highest_score_copy;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			record_file >> board_copy[i][j];
		}
	}
	record_file >> score_copy;
	record_file >> total_numbers_copy;
	record_file >> swipe_copy;
	record_file >> moves_copy;
	record_file >> is_dead_copy;
	record_file.close();

	// Step 2: Write updated highest_score to the record file
	std::ofstream record_file_write(kRecordFileName);
	// 1. Store highest score
	record_file_write << highest_score_ << std::endl;
	// 2. Store board_copy
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			record_file_write << board_copy[i][j] << " ";
		}
		record_file_write << std::endl;
	}
	// 3. Store score_
	record_file_write << score_copy << std::endl;
	// 4. Store total_numbers_
	record_file_write << total_numbers_copy << std::endl;
	// 5. Store last_swipe_directions_
	record_file_write << swipe_copy << std::endl;
	// 6. Store moves_
	record_file_write << moves_copy << std::endl;
	// 7. Store is_dead_
	record_file_write << is_dead_copy;
}

void Board::LoadFromFile(BoardStatus& status) {
	std::ifstream record_file(kRecordFileName);
	record_file >> highest_score_;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			record_file >> status.board[i][j];
		}
	}
	record_file >> status.score;
	record_file >> status.total_numbers;
	int swipe;
	record_file >> swipe;
	if (swipe == 1) status.last_swipe_direction = SWIPE_DIRECTION::UP;
	if (swipe == 2) status.last_swipe_direction = SWIPE_DIRECTION::RIGHT;
	if (swipe == 3) status.last_swipe_direction = SWIPE_DIRECTION::DOWN;
	if (swipe == 4) status.last_swipe_direction = SWIPE_DIRECTION::LEFT;
	if (swipe == 5) status.last_swipe_direction = SWIPE_DIRECTION::NOT_SET;
	record_file >> status.moves;
	int is_dead;
	record_file >> is_dead;
	status.is_dead = is_dead == 1 ? true : false;
}

void Board::ResetBoard(BoardStatus& status) {
	status.total_numbers = 0;

	for (auto& r : status.board) {
		for (auto& c : r) {
			c = 0;
		}
	}
	for (int i = 0; i < 2; i++) {
		AddNewNumber(status);
	}

	status.score = 0;
	status.is_dead = false;
	status.last_swipe_direction = SWIPE_DIRECTION::NOT_SET;
	status.moves = 0;
}

bool Board::CanSwipeInDirection(SWIPE_DIRECTION direction, const BoardStatus& status) {
	if (direction == SWIPE_DIRECTION::RIGHT) {
		for (auto r : status.board) {
			if (CanMergeToBack(r)) {
				return true;
			}
		}
	} else if (direction == SWIPE_DIRECTION::LEFT) {
		for (auto r : status.board) {
			FlipVector(r);
			if (CanMergeToBack(r)) {
				return true;
			}
		}
	} else if (direction == SWIPE_DIRECTION::UP) {
		BoardStatus tmp_board_status(status);
		Transform2DVector(tmp_board_status.board);
		for (auto r : tmp_board_status.board) {
			FlipVector(r);
			if (CanMergeToBack(r)) {
				return true;
			}
		}
	} else if (direction == SWIPE_DIRECTION::DOWN) {
		BoardStatus tmp_board_status(status);
		Transform2DVector(tmp_board_status.board);
		for (auto r : tmp_board_status.board) {
			if (CanMergeToBack(r)) {
				return true;
			}
		}
	}
	return false;
}

void Board::HumanMove(SWIPE_DIRECTION direction, BoardStatus& status) {
	Swipe(direction, status);
	if (status.score > highest_score_) {
		highest_score_ = status.score;
		UpdateHighestScoreInFile();
	}
	
}

void Board::Swipe(SWIPE_DIRECTION direction, BoardStatus& status) {
	if (!CanSwipeInDirection(direction, status)) {
		std::cout << "This swipe doesn't change the board." << std::endl;
		return;
	}

	if (direction == SWIPE_DIRECTION::RIGHT) {
		for (auto& r : status.board) {
			MergeToBack(r, status);
		}
	} else if (direction == SWIPE_DIRECTION::LEFT) {
		for (auto& r : status.board) {
			FlipVector(r);
			MergeToBack(r, status);
			FlipVector(r);
		}
	} else if (direction == SWIPE_DIRECTION::UP) {
		Transform2DVector(status.board);
		for (auto& r : status.board) {
			FlipVector(r);
			MergeToBack(r, status);
			FlipVector(r);
		}
		Transform2DVector(status.board);
	} else if (direction == SWIPE_DIRECTION::DOWN) {
		Transform2DVector(status.board);
		for (auto& r : status.board) {
			MergeToBack(r, status);
		}
		Transform2DVector(status.board);
	}
	status.last_swipe_direction = direction;
	status.moves++;
	AddNewNumber(status);
}

void Board::Transform2DVector(std::vector<std::vector<int>>& matrix) {
	int r = matrix.size();
	int c = matrix[0].size();
	if (r == 0 || c == 0) {
		return;
	}
	std::vector<std::vector<int>> aux_matrix(c, std::vector<int>(r, 0));
	for (int i = 0; i < r; i++) {
		for (int j = 0; j < c; j++) {
			aux_matrix[j][i] = matrix[i][j];
		}
	}
	for (int i = 0; i < r; i++) {
		for (int j = 0; j < c; j++) {
			matrix[i][j] = aux_matrix[i][j];
		}
	}
}

void Board::FlipVector(std::vector<int>& vector) {
	std::vector<int> aux_vector;
	for (int i = vector.size() - 1; i >= 0; i--) {
		aux_vector.push_back(vector[i]);
	}
	for (int i = 0; i < vector.size(); i++) {
		vector[i] = aux_vector[i];
	}
}

bool Board::CanMergeToBack(const std::vector<int>& vector) {
	// There are two conditions,
	// 1) numbers can be shifted
	// OR
	// 2) has two non-zero numbers that can be merged
	
	bool has_non_zero_merge = false;
	int first_zero_pos = - 1;
	int first_number_pos = - 1;
	int latest_non_zero_number = 0;

	for (int i = vector.size() - 1; i >= 0; i--) {
		if (vector[i] != 0) {
			if (first_number_pos == -1) {
				first_number_pos = i;
			}
			if (first_zero_pos > i) {
				// This means can be shift due to more zeros on the back of this number.
				return true;
			}
			if (latest_non_zero_number == vector[i]) {
				// has two non-zero numbers that can be merged
				return true;
			}
			latest_non_zero_number = vector[i];
		} else {
			if (first_zero_pos == -1) {
				first_zero_pos = i;
			}
		}
	}
	return false;
}

void Board::MergeToBack(std::vector<int>& vector, BoardStatus& status) {
	// Step 0: Check if this can be merged
	if (!CanMergeToBack(vector)) {
		return;
	}

	// Step 1: create aux vector in back order and remove all zeros, reset the original vector
	std::vector<int> aux_vector;
	aux_vector.clear();
	for (int i = vector.size() - 1; i >= 0; i--) {
		if (vector[i] != 0) {
			aux_vector.push_back(vector[i]);
			vector[i] = 0;
		}
	}

	// Step 2: Merge
	int put_pos = vector.size() - 1;
	int check_pos = 0;
	while (check_pos <= aux_vector.size() - 1) {
		int aux_size = aux_vector.size() - 2;
		if (check_pos <= aux_size) {
			if (aux_vector[check_pos] == aux_vector[check_pos + 1]) {
				int score_gain = aux_vector[check_pos] * 2;
				vector[put_pos] = aux_vector[check_pos] * 2;
				check_pos += 2;
				put_pos--;
				status.total_numbers--;
				status.score += score_gain;
			} else {
				vector[put_pos] = aux_vector[check_pos];
				check_pos++;
				put_pos--;
			}
		} else {
			vector[put_pos] = aux_vector[check_pos];
			put_pos--;
			check_pos++;
		}
	}
}

void Board::AddNewNumber(BoardStatus& status) {
	// Step 1: Generate new number
	int seed_number = rand() % 100;
	int number = seed_number < 90 ? 2 : 4;

	// Step 2: Determine new number position
	std::pair<int, int> new_number_index = DetermineNewNumberPosition(status);

	// Step 3: Assign new number to the board
	if (new_number_index.first != -1) {
		if (status.board[new_number_index.first][new_number_index.second] == 0) {
			status.total_numbers++;
			status.board[new_number_index.first][new_number_index.second] = number;
		}
	} else {
		std::cerr << "Board is full.";
	}
}

std::pair<int, int> Board::DetermineNewNumberPosition(const BoardStatus& status) {
	if (status.total_numbers >= 16) {
		return { -1, -1 };
	}
	// Strategy 1 : return the first available position, top first, left first
	/*for (int r = 0; r < 4; r++) {
		for (int c = 0; c < 4; c++) {
			if (status.board[r][c] == 0) {
				return { r, c };
			}
		}
	}*/

	// Strategy 2: Return the random available position
	int available_position_num = 4 * 4 - status.total_numbers;
	int return_index = rand() % available_position_num;
	int count = 0;
	for (int r = 0; r < 4; r++) {
		for (int c = 0; c < 4; c++) {
			if (status.board[r][c] == 0) {
				if (count == return_index) {
					return { r,c };
				} else {
					count++;
				}
			}
		}
	}
}

void Board::PrintBoard(const BoardStatus& status) {
	std::cout << "Score: " << std::to_string(status.score) << '\t'
			  << "Highest score ever: " << highest_score_ << '\t'
		      << "Total moves : " << std::to_string(status.moves) << std::endl;
	for (const auto& r : status.board) {
		for (const auto& c : r) {
			std::cout << c << '\t';
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
}

BoardStatus& Board::GetCurrentBoard() {
	return status_;
}

sf::Color Board::GetTileColor(int number) {
	if (number == 2) return sf::Color(238, 228, 218, 255);
	if (number == 4) return sf::Color(238, 225, 201, 255);
	if (number == 8) return sf::Color(243, 178, 122, 255);
	if (number == 16) return sf::Color(246, 150, 100, 255);
	if (number == 32) return sf::Color(247, 124, 95, 255);
	if (number == 64) return sf::Color(247, 95, 59, 255);
	if (number == 128) return sf::Color(237, 208, 115, 255);
	if (number == 256) return sf::Color(237, 204, 98, 255);
	if (number == 512) return sf::Color(237, 201, 80, 255);
	if (number == 1024) return sf::Color(237, 197, 63, 255);
	if (number == 2048) return sf::Color(238, 228, 218, 255); // need to check
	if (number == 4096) return sf::Color(238, 225, 201, 255); // need to check
	return sf::Color(205, 193, 180, 255);
}

sf::Color Board::GetTextColor(int number) {
	if (number == 2 || number == 4) return sf::Color(119, 110, 101, 255);
	if (number >= 8 && number <= 1024) return sf::Color(249, 246, 242, 255);
	if (number == 2048) return sf::Color(238, 0, 0, 255); // need to check
	if (number == 4096) return sf::Color::Black; // need to check
	return sf::Color::White;
}

int Board::GetTextSize(int number) {
	if (number < 10) return 110;
	if (number < 100) return 100;
	if (number < 1000) return 100;
	return 78;
}

void Board::Draw() {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			int idx = i * 4 + j;
			int number = status_.board[i][j];
			tiles_[idx].setPosition(window_width_/8 + j * window_width_ / 4, top_left_y_ + window_width_/8 + i * window_width_ / 4);
			tiles_[idx].setFillColor(GetTileColor(number));

			texts_[idx].setString(std::to_string(number));
			texts_[idx].setCharacterSize(GetTextSize(number));
			texts_[idx].setFillColor(GetTextColor(number));
			texts_[idx].setOrigin(texts_[idx].getGlobalBounds().width/2, texts_[idx].getGlobalBounds().height/2);
			texts_[idx].setPosition(window_width_ / 8 + j * window_width_ / 4 - 5, top_left_y_ + window_width_ / 8 + i * window_width_ / 4 - 25);
			
		}
	}
	for (auto t : tiles_) {
		app_->draw(t);
	}
	for (auto t : texts_) {
		if (t.getString() == "0") {
			continue;
		}
		app_->draw(t);
	}
}

void Board::SaveToFile() {
	std::ofstream record_file(kRecordFileName);
	// 1. Store highest score
	record_file << highest_score_ << std::endl;
	// 2. Store board
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			record_file << status_.board[i][j] << " ";
		}
		record_file << std::endl;
	}
	// 3. Store score
	record_file << status_.score << std::endl;
	// 4. Store total_numbers
	record_file << status_.total_numbers << std::endl;
	// 5. Store last_swipe_directions
	record_file << status_.last_swipe_direction << std::endl;
	// 6. Store moves
	record_file << status_.moves << std::endl;
	// 7. Store is_dead
	int is_dead = status_.is_dead ? 1 : 0;
	record_file << is_dead;
}

int Board::GetScore(const BoardStatus& status) {
	return status.score;
}

int Board::GetBestScore() {
	return highest_score_;
}

void Board::ToggleInputMode() {
	is_human_input_ = !is_human_input_;
}

bool Board::IsHumanInput() {
	return is_human_input_;
}

std::vector<SWIPE_DIRECTION> Board::FindPossibleMoves(const BoardStatus& status) {
	std::vector<SWIPE_DIRECTION> possible_moves;
	if (CanSwipeInDirection(SWIPE_DIRECTION::DOWN, status)) possible_moves.push_back(SWIPE_DIRECTION::DOWN);
	if (CanSwipeInDirection(SWIPE_DIRECTION::UP, status)) possible_moves.push_back(SWIPE_DIRECTION::UP);
	if (CanSwipeInDirection(SWIPE_DIRECTION::LEFT, status)) possible_moves.push_back(SWIPE_DIRECTION::LEFT);
	if (CanSwipeInDirection(SWIPE_DIRECTION::RIGHT, status)) possible_moves.push_back(SWIPE_DIRECTION::RIGHT);
	return possible_moves;
}

void Board::SimulateDirectionScore(BoardStatus status,
	                               SWIPE_DIRECTION direction,
	                               std::vector<int>& direction_scores,
	                               int thread_id,
	                               int num_of_simulations) {
	Swipe(direction, status);
	for (int i = 0; i < num_of_simulations; i++) {
		BoardStatus status_copy(status);
		while (!status_copy.is_dead) {
			std::vector<SWIPE_DIRECTION> move_candidates = FindPossibleMoves(status_copy);
			if (move_candidates.empty()) break;
			Swipe(move_candidates[rand() % move_candidates.size()], status_copy);
		}
		direction_scores[thread_id] += status_copy.score;
	}
	
}

void Board::BotMove(BoardStatus& status) {
	if (status.is_dead) return;

	std::vector<SWIPE_DIRECTION> possible_moves = FindPossibleMoves(status);	
	if (possible_moves.empty()) {
		status.is_dead = true;
		return;
		// std::cout << "This must be a problem." << std::endl;
	}

	// Bot move strategy 1: random move
	// int random_move_index = rand() % possible_moves.size();
	// Swipe(possible_moves[random_move_index], status);

	// Bot Move strategy 2: monte carlo move
	std::vector<int> direction_scores(possible_moves.size(), 0);
	int number_of_simulations = 100;
	std::vector<std::thread> threads;
	for (int i = 0; i < possible_moves.size(); i++) {
		threads.push_back(std::thread(SimulateDirectionScore,
			                          status,
			                          possible_moves[i],
			                          std::ref(direction_scores),
			                          i,
			                          number_of_simulations));
	}
	for (int i = 0; i < threads.size(); i++) {
		threads[i].join();
	}

	int highest_score_index = -1;
	int highest_score = 0;
	for (int i = 0; i < direction_scores.size(); i++) {
		if (direction_scores[i] > highest_score) {
			highest_score = direction_scores[i];
			highest_score_index = i;
		}
	}
	Swipe(possible_moves[highest_score_index], status);
	if (status.score > highest_score_) {
		highest_score_ = status.score;
		UpdateHighestScoreInFile();
	}
}

void Board::ChangeBotMoveInterval(float interval) {
	if (interval < 0) {
		std::cout << "Bot move interval must be greater than 0." << std::endl;
		return;
	}
	bot_move_interval_ = interval;
}

float Board::GetBotMoveInterval() {
	return bot_move_interval_;
}