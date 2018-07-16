#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include <ncurses.h>

/* Number of microseconds per tick */
#define TICK_DELAY 1000000

/* Contains the application state */
typedef struct {
	uint64_t generation;
	bool quit;
	bool paused;
	bool extinct;
} GameState;

WINDOW* init_ncurses(void);
void handle_input(GameState* state);
bool update_board(bool* curr_iter, bool* next_iter, uint32_t w, uint32_t h);
void display_board(const bool* board, uint32_t w, uint32_t h);
void display_ui(const GameState* game_state, uint32_t w, uint32_t h);
void init_board(bool* board, uint32_t w, uint32_t h);
int idx_at(uint32_t x, uint32_t y, uint32_t w, uint32_t h);

void print_command_line_arguments(void);
bool parse_arguments(int argc, char const* argv[]);

int main(int argc, char const* argv[]) {
	GameState game_state = {.generation = 1, .quit = false, .paused = false };
	WINDOW* root = init_ncurses();
	if (!parse_arguments(argc, argv)) {
		print_command_line_arguments();
		return 0;
	}
	const uint32_t w = 10;
	const uint32_t h = 10;
	bool* board0 = calloc(sizeof(bool), w * h);
	bool* board1 = calloc(sizeof(bool), w * h);
	bool* boards[2] = {board0, board1};
	uint8_t current_board_idx = 0;
	init_board(boards[current_board_idx], w, h);
	display_board(boards[current_board_idx], w, h);
	while (!game_state.quit) {
		handle_input(&game_state);
		if (!game_state.paused) {
			uint8_t next_board_idx = (current_board_idx + 1) % 2;

			if (update_board(boards[current_board_idx], boards[next_board_idx], w, h)) {
				game_state.generation++;
			} else {
				game_state.extinct = true;
			}
			display_board(boards[current_board_idx], w, h);
			
			current_board_idx = next_board_idx;
		}
		display_ui(&game_state, w, h);
		usleep(TICK_DELAY);
	}
	return 0;
}

WINDOW* init_ncurses(void) {
	WINDOW *root = initscr(); /* initialize the curses library */
	cbreak();             /* Line buffering disabled pass on everything to me */
	noecho();             /* Do not echo out input */
	nodelay(root, true);  /* Make getch non-blocking */
	refresh();
	return root;
}

bool update_board(bool* curr_iter, bool* next_iter, uint32_t w, uint32_t h) {
	bool life_survived_to_next_gen = false;
	for (uint32_t x = 0; x < w; x++) {
		for (uint32_t y = 0; y < h; y++) {
			uint8_t l = curr_iter[idx_at(x - 1, y, w, h)] ? 1 : 0;
			uint8_t r = curr_iter[idx_at(x + 1, y, w, h)] ? 1 : 0;
			uint8_t t = curr_iter[idx_at(x, y + 1, w, h)] ? 1 : 0;
			uint8_t b = curr_iter[idx_at(x, y - 1, w, h)] ? 1 : 0;
			
			uint8_t bl = curr_iter[idx_at(x - 1, y - 1, w, h)] ? 1 : 0;
			uint8_t tr = curr_iter[idx_at(x + 1, y + 1, w, h)] ? 1 : 0;
			uint8_t tl = curr_iter[idx_at(x - 1, y + 1, w, h)] ? 1 : 0;
			uint8_t br = curr_iter[idx_at(x + 1, y - 1, w, h)] ? 1 : 0;

			uint8_t neighbors = l + r + t + b + bl + tr + tl + br;

			// Reproduction
			if (!next_iter[idx_at(x, y, w, h)]) {
				if (neighbors == 3) {
					next_iter[idx_at(x, y, w, h)] = true;
				}
				continue;
			}

			// Under population
			if (neighbors < 2) {
				next_iter[idx_at(x, y, w, h)] = false;
			}
			// Overpopulation
			if (neighbors > 3) {
				next_iter[idx_at(x, y, w, h)] = false;	
			}
			// Detect if the next generation exists at all
			if (next_iter[idx_at(x, y, w, h)]) {
				life_survived_to_next_gen = true;
			}
		}
	}
	return life_survived_to_next_gen;
}

void display_board(const bool* board, uint32_t w, uint32_t h) {
	for (uint32_t x = 0; x < w; x++) {
		for (uint32_t y = 0; y < h; y++) {
			if (board[idx_at(x, y, w, h)]) {
				mvaddch(y, x, 'O' | A_BOLD);
			} else {
				mvaddch(y, x, '#' | A_DIM);
			}
		}
	}
}

void display_ui(const GameState* game_state, uint32_t w, uint32_t h) {
	mvprintw(h, 0, "Generation: %u", game_state->generation);
	mvprintw(h + 1, 0, "Q: quit, P: pause/unpause");
	refresh();
}

void init_board(bool* board, uint32_t w, uint32_t h) {
	// Fill board randomly 
	srand48(time(NULL));
	for (uint32_t x = 0; x < w; x++) {
		for (uint32_t y = 0; y < h; y++) {
			board[idx_at(x, y, w, h)] = (drand48() < 0.2) ? true : false;
		}
	}
}

void handle_input(GameState* state) {
	char ch = getch();
	switch (ch) {
		case 'q':
			state->quit = true;
			break;
		case 'p':
			state->paused = !state->paused;
			break;
	}
}

int idx_at(uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
	return (y % h) * w + (x % w);
}

void print_command_line_arguments(void) {
	mvprintw(0, 0, "Dimensions: -d 4 4 \n");
}

bool parse_arguments(int argc, char const* argv[]) {
	// TODO: Implement.
	return true;
}