#include <3ds.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sf2d.h>

struct Game {
	char board[3][3];
	int player;
	char current_token;
	int filled;
	bool done;
} game;

void drawToken(int x, int y, char token) {
	if(token == 'x') { // draw two lines for X
		sf2d_draw_line(x-20, y-20, x+20, y+20, 4, RGBA8(255, 255, 255, 255));
		sf2d_draw_line(x-20, y+20, x+20, y-20, 4, RGBA8(255, 255, 255, 255));
	} else if(token == 'o'){ // draw circle for O
		sf2d_draw_fill_circle(x, y, 20, RGBA8(255, 255, 255, 255));
	}
}

void drawBoard(char board[3][3]) {
	sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
		sf2d_draw_line(133, 39, 133, 201, 4, RGBA8(255, 255, 255, 255)); // draw the 4 lines for the classic tic-tac-toe board
		sf2d_draw_line(187, 39, 187, 201, 4, RGBA8(255, 255, 255, 255));
		sf2d_draw_line(79, 93, 241, 93, 4, RGBA8(255, 255, 255, 255));
		sf2d_draw_line(79, 147, 241, 147, 4, RGBA8(255, 255, 255, 255));

		int x = 106; // x and y: middle of the top left spot on board
		int y = 66;
		for(int i = 0; i < 3; i++) {
			for(int j = 0; j < 3; j++) {
				if(board[i][j] != '_') { // draw the shapes/tokens if they exist in board[3][3]
					drawToken(x, y, board[i][j]);
				}
				x+=54;
			}
			x = 106;
			y+=54;
		}
	sf2d_end_frame();
}

bool winIsPresent(char board[3][3], char token) {
	// diagonals
	if(board[0][0] == token && board[1][1] == token && board[2][2] == token) {
		printf("%c %s\n", token, "wins!");
		return true;
	} else if (board[0][2] == token && board[1][1] == token && board[2][0] == token) {
		printf("%c %s\n", token, "wins!");
		return true;
	}
	// vertical and horizontal
	int countV = 0;
	int countH = 0;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			countV = board[j][i] == token ? countV+1 : 0;
			countH = board[i][j] == token ? countH+1 : 0;
			if(countH >= 3 || countV >= 3) {
				printf("%c %s\n", token, "wins!");
				return true;
			}
		}
		countV=0;
		countH=0;
	}
	return false;
} // returns whether board array has a win for token

void beginPhase(struct Game *game) {
	if(game->filled >= 9) { // check for tie
		printf("%s\n", "Tie!");
		printf("%s\n", "Press Select to reset game.");
		game->done = true;
		return;
	}

	if(game->player == 1) { // check who's token to place
		game->current_token = 'x';
	} else {
		game->current_token = 'o';
	}
}

void endPhase(int *player) {
	*player = *player == 1 ? 2 : 1;
}

void placeToken(touchPosition touch, struct Game *game) {
	beginPhase(game); // check for tie, and set game's current token

	if(touch.px == 0 && touch.py == 0) return; // no touch

	// go through board 3 x 3 and check if was touched
	int x = 79;
	int y = 39;
	for(int i = 0; i < 3; i++){
		for(int j = 0; j < 3; j++){
			if((touch.px > x && touch.px < x+54) && (touch.py > y && touch.py < y+54)) {
				if(game->board[i][j] != '_'){
					// spot taken - dont place
				} else {
					game->board[i][j] = game->current_token;
					game->filled++;
					if(winIsPresent(game->board, game->current_token)) {
						game->done = true;
						printf("%s\n", "Press Select to reset game.");
						return;
					}
					endPhase(&game->player); // next player
					return;
				}
			}
			x+=54;
		}
		x = 79;
		y+=54;
	}
}

void reset(struct Game *game) {
	printf("\033[3;0H%s\n", "                                 "); // clear "game outcome" line and "Press Select to reset" line
	printf("\033[4;0H%s\n", "                                 ");
	memset(&game->board[0], '_', sizeof(game->board[0])); // reset board;
	memset(&game->board[1], '_', sizeof(game->board[1]));
	memset(&game->board[2], '_', sizeof(game->board[2]));
	game->player = 1; // reset game's members
	game->current_token = 'x';
	game->filled = 0;
	game->done = false;
}

int main(int argc, char **argv)
{
	sf2d_init();
	sf2d_set_clear_color(RGBA8(0x40, 0x40, 0x40, 0xFF));

	//Initialize console on top screen. Using NULL as the second argument tells the console library to use the internal console structure as current one
	consoleInit(GFX_TOP, NULL);

	//
	printf("\x1b[0;0HPress Start to exit.");

	reset(&game); // set the default values for game struct

	u32 kDown;
	// Main loop
	while (aptMainLoop())
	{
		//Scan all the inputs. This should be done once for each frame
		hidScanInput();

		//hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
		kDown = hidKeysDown();

		if (kDown & KEY_START) break; // break in order to return to hbmenu

		touchPosition touch;
		//Read the touch screen coordinates
		hidTouchRead(&touch);

		printf("\x1b[2;0H%s %d %s\n", "Player", game.player, "'s turn");
		drawBoard(game.board); // draw the board 2d array as shapes on bottom screen
		if(!game.done) { // if game is finished allow to reset, otherwise placeToken at touch
			placeToken(touch, &game);
		} else {
			if (kDown & KEY_SELECT) reset(&game);
		}

		sf2d_swapbuffers();

		//Wait for VBlank
		gspWaitForVBlank();
	}

	// Exit services
	sf2d_fini();
	return 0;
}
