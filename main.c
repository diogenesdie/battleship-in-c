/**
 * @file main.c
 * @author Diógenes Dietrich de Morais
 * @author Gustavo Reis Bauer
 * @brief BattleShip
 * @version 1.0
 * @date 2022-05-18
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <locale.h>
#include <string.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#define NUMBER_OF_COLUMNS 20
#define NUMBER_OF_ROWS 20
#define BOAT_SIZE 5
#define MAX_BOATS_PER_PLAYER 3
#define MAX_SHOOTS_PER_PLAYER 30

typedef enum boat_orientation {
    VERTICAL,
    HORIZONTAL
} boat_orientation_t;

typedef struct point {
    bool is_shot;
    bool has_boat;
} point_t;

typedef struct board {
    point_t points[NUMBER_OF_COLUMNS][NUMBER_OF_ROWS];
    point_t *boats[MAX_BOATS_PER_PLAYER][BOAT_SIZE];
    size_t boats_count;
} board_t;

typedef struct player {
    char name[100];
    size_t shoots;
    size_t missed_shots;
    size_t boats_pieces_sunk;
    size_t wins;
    board_t board;
} player_t;

typedef struct game {
    player_t *shooting_player;
    player_t *target_player;

    player_t player1;
    player_t player2;

    bool is_game_over;
    bool play_again;
} game_t;

/**
 * @brief pause the code execution for a certain time 
 * 
 * @param int seconds 
 * @return void
 */
void sleep_for_seconds( int seconds ){
    #ifdef _WIN32
        Sleep(seconds * 1000);
    #else
        sleep(seconds);
    #endif
}

/**
 * @brief Clears the console
 * 
 * @param void
 * @return void
 */
void clear_screen(){
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

/**
 * @brief Get the row number equivalent to the given char
 * 
 * @param row_name 
 * @return int 
 */
int get_row_number(char row_name){
    int row_number = toupper(row_name) - 'A';
    return row_number;
}

/**
 * @brief Get the x from buffer and convert it to a point
 * 
 * @param x 
 * @return void
 */
void get_x_from_buffer(int *x){
    do {
        scanf("%d", x);

        if (*x < 0 || *x >= NUMBER_OF_COLUMNS) {
            printf("Invalid value for x. Type again:");
        }
    } while (*x < 1 || *x > NUMBER_OF_COLUMNS);
    
    --(*x);
}

/**
 * @brief Get the y from buffer and convert it to a point
 * 
 * @param y 
 * @return void
 */
void get_y_from_buffer(int *y){
    char row;

    do {
        fflush(stdin);
        scanf("%c", &row);

        *y = get_row_number(row);

        if (*y < 0 || *y >= NUMBER_OF_ROWS) {
            printf("Invalid value for y. Type again: ");
        }
    } while (*y < 0 || *y >= NUMBER_OF_ROWS);
}

/**
 * @brief Initializes the points on the board and sets the number of boats to 0
 * 
 * @param board 
 * @return void
 */
void initialize_board( board_t *board ){
    for( int i = 0; i < NUMBER_OF_COLUMNS; i++ ){
        for( int j = 0; j < NUMBER_OF_ROWS; j++ ){
            board->points[i][j] = (point_t){.is_shot = false, .has_boat = false};
        }
    }

    board->boats_count = 0;
}

/**
 * @brief Set a point as shot
 * 
 * @param board 
 * @param x 
 * @param y 
 * @return bool
 */
bool shoot( board_t *board, int x, int y ){
    if( board->points[x][y].is_shot ){
        return false;
    } else {
        board->points[x][y].is_shot = true;
        return true;
    }
}

/**
 * @brief Checks if a boat can be placed at the point
 * 
 * @param board 
 * @param x 
 * @param y 
 * @param orientation 
 * @return bool
 */
bool can_place_boat( board_t *board, int x, int y,  boat_orientation_t orientation ){
    if( orientation == VERTICAL ){
        if( y + BOAT_SIZE > NUMBER_OF_ROWS ){
            return false;
        }
        for( int i = 0; i < BOAT_SIZE; i++ ){
            if( board->points[x][y + i].has_boat ){
                return false;
            }
        }
    } else {
        if( x + BOAT_SIZE > NUMBER_OF_COLUMNS ){
            return false;
        }
        for( int i = 0; i < BOAT_SIZE; i++ ){
            if( board->points[x + i][y].has_boat ){
                return false;
            }
        }
    }
    
    return true;
}

/**
 * @brief Places a boat on the board
 * 
 * @param board 
 * @param x 
 * @param y 
 * @param orientation 
 * @return bool
 */
bool place_boat( board_t *board, int x, int y, boat_orientation_t orientation ){
    if( can_place_boat( board, x, y, orientation ) ){
        if( orientation == VERTICAL ){
            for( int i = 0; i < BOAT_SIZE; i++ ){
                board->points[x][y + i].has_boat = true;
                board->boats[board->boats_count][i] = &board->points[x][y + i];
            }
        } else {
            for( int i = 0; i < BOAT_SIZE; i++ ){
                board->points[x + i][y].has_boat = true;
                board->boats[board->boats_count][i] = &board->points[x + i][y];
            }
        }

        board->boats_count++;
        return true;
    }

    return false;
}

/**
 * @brief Show the board in the console
 * 
 * @param board 
 * @param show_boats 
 * @return void
 */
void print_board( board_t *board, bool show_boats ){
    char row;

    printf("  ");
    for( int i = 0; i < NUMBER_OF_COLUMNS; i++ ){
        printf(" %2d ", i+1);
    }
    printf("\n");
    for( int i = 0; i < NUMBER_OF_ROWS; i++ ){
        row = (char) (i + 65);
        printf("%c  ", row);
        for( int j = 0; j < NUMBER_OF_COLUMNS; j++ ){
            if( board->points[j][i].is_shot && board->points[j][i].has_boat ){
                printf(" X  ");
            } else if( board->points[j][i].has_boat && show_boats ){
                printf(" >  ");
            } else if( !board->points[j][i].has_boat && board->points[j][i].is_shot ){
                printf(" O  ");
            } else {
                printf(" ~  ");
            }
        }
        printf("\n");
    }
}

/**
 * @brief Initializes the player 
 * 
 * @param player 
 * @return void
 */
void initialize_player( player_t *player ){
    player->shoots = 0;
    player->missed_shots = 0;
    player->boats_pieces_sunk = 0;

    initialize_board( &player->board );
} 

/**
 * @brief Ask the players to enter their name
 * 
 * @param player1 
 * @param player2
 * @return void 
 */
void ask_names( player_t *player1, player_t *player2 ){
    char name[100];

    clear_screen();
    printf("Player 1, enter your name: ");
    fflush(stdin);
    scanf("%s", name);
    strncpy( player1->name, name, 100 );

    strncpy( name, "", 100 );

    printf("Player 2, enter your name: ");
    fflush(stdin);
    scanf("%s", name);
    strncpy( player2->name, name, 100 );
    
    clear_screen();
}

/**
 * @brief Ask the player the position of their boats
 * 
 * @param player
 * @return void 
 */
void ask_boats_positions( player_t **player ){
    int x, y;

    char input_orientation;
    boat_orientation_t orientation;
    bool success, is_orientation_valid = false;

    for ( size_t i = 0; i < MAX_BOATS_PER_PLAYER; ++i ){
        do {
            print_board( &(*player)->board, true );
            printf("%s, please enter the coordinates of the boat's starting point X: ", (*player)->name);
            get_x_from_buffer( &x );
            printf("%s, please enter the coordinates of the boat's starting point Y: ", (*player)->name);
            get_y_from_buffer( &y );
            do {
                printf("%s, please enter the orientation of the boat ([v]vertical/[h]horizontal): ", (*player)->name);
                fflush(stdin);
                scanf("%c", &input_orientation);

                if ( input_orientation == 'v' || input_orientation == 'V' ){
                    orientation = VERTICAL;
                    is_orientation_valid = true;
                } else if ( input_orientation == 'h' || input_orientation == 'H' ){
                    orientation = HORIZONTAL;
                    is_orientation_valid = true;
                } else {
                    printf("Invalid orientation.\n");
                    is_orientation_valid = false;
                }
            } while ( !is_orientation_valid );

            success = place_boat( &(*player)->board, x, y, orientation );
            if( !success ){
                printf("Invalid coordinates, please try again.\n");
            } else {
                print_board( &(*player)->board, true );
            }
        } while( !success );

    }
}

/**
 * @brief Ask the player for the shot coordinates
 * 
 * @param player 
 * @param x 
 * @param y 
 * @return void
 */
void ask_shoot_coordinates( player_t **player, int *x, int *y ){
    printf("You have %d shoots left.\n", MAX_SHOOTS_PER_PLAYER - (*player)->shoots);
    printf("%s, please enter the coordinates of the shoot X: ", (*player)->name);
    get_x_from_buffer( x );
    printf("%s, please enter the coordinates of the shoot Y: ", (*player)->name);
    get_y_from_buffer( y );
}

/**
 * @brief Perform the shot and show the result on the console
 * 
 * @param shooting_player 
 * @param target_player 
 * @param x 
 * @param y 
 * @return void
 */
void shoot_and_print_result( player_t **shooting_player, player_t **target_player, int x, int y ){
    clear_screen();

    if( !shoot( &(*target_player)->board, x, y ) ){
        printf("You already shot there.\n");
        return;    
    }

    print_board( &(*target_player)->board, false );

    if( (*target_player)->board.points[x][y].has_boat ){
        printf("%s, you hit a boat!\n", (*shooting_player)->name);
        ++(*shooting_player)->boats_pieces_sunk;  
    } else {
        printf("%s, you missed!\n", (*shooting_player)->name);
        ++(*shooting_player)->missed_shots;
    }

    ++(*shooting_player)->shoots;
}

/**
 * @brief Swap players turn
 * 
 * @param game 
 * @return void
 */
void swap_players( game_t *game ){
    player_t *aux = game->target_player;
    game->target_player = game->shooting_player;
    game->shooting_player = aux;
}

/**
 * @brief Initializes the game
 * 
 * @param game 
 * @return void
 */
void new_game( game_t *game ){
    initialize_player( &game->player1 );
    initialize_player( &game->player2 );

    game->target_player = &game->player1;
    game->shooting_player = &game->player2;
    game->is_game_over = false;
    game->play_again   = true;
}

/**
 * @brief Show the countdowns before the game starts
 * 
 * @param void
 * @return void
 */
void start_game_countdown(){
    printf("Press ENTER to start the game.\n");
    fflush(stdin);
    getchar();
    clear_screen();

    printf("Starting game in 5...\n");
    sleep_for_seconds(1);
    printf("4...\n");
    sleep_for_seconds(1);
    printf("3...\n");
    sleep_for_seconds(1);
    printf("2...\n");
    sleep_for_seconds(1);
    printf("1...\n");
    sleep_for_seconds(1);
    clear_screen();
}

/**
 * @brief get the number of sunken ships from the player's board
 * 
 * @param target_player 
 * @return int 
 */
int boats_sunk( player_t **target_player ){
    int boats_sunk = 0;
    for ( size_t i = 0; i < MAX_BOATS_PER_PLAYER; ++i ){
        bool is_boat_sunk = true;

        for ( size_t j = 0; j < BOAT_SIZE; ++j ){
            if( !(*target_player)->board.boats[i][j]->is_shot ){
                is_boat_sunk = false;
                break;
            }
        }

        if( is_boat_sunk ){
            ++boats_sunk;
        }
    }

    return boats_sunk;
}

/**
 * @brief Main function
 * 
 * @param void
 * @return int 
 */
int main(void) {
    setlocale(LC_ALL, "");

    game_t game;

    new_game( &game );
    game.player1.wins = 0;
    game.player2.wins = 0;

    ask_names( &game.player1, &game.player2 );

    while (game.play_again) {
        game.play_again = false;
        ask_boats_positions( &game.target_player );
        start_game_countdown();

        while( !game.is_game_over ){
            int x, y;

            if( game.shooting_player->shoots == 0 ) {
                print_board( &game.target_player->board, false );
            }

            ask_shoot_coordinates( &game.shooting_player, &x, &y );
            shoot_and_print_result( &game.shooting_player, &game.target_player, x, y );

            if( game.shooting_player->boats_pieces_sunk >= MAX_BOATS_PER_PLAYER * BOAT_SIZE ){
                printf("%s, you won!\n", game.shooting_player->name);
                ++game.shooting_player->wins;
                game.is_game_over = true;
            } else if( game.shooting_player->shoots >= MAX_SHOOTS_PER_PLAYER ){
                printf("%s, you lost!\n", game.shooting_player->name);
                ++game.target_player->wins;
                game.is_game_over = true;
            }
        }

        printf("----------------------------------------------------\n");
        printf( "Final score:\n" );
        printf( "%s: %d wins\n", game.player1.name, game.player1.wins );
        printf( "%s: %d wins\n", game.player2.name, game.player2.wins );
        printf( "Boats sunk in this round: %d\n", boats_sunk( &game.target_player ) );
        printf( "Correct shots in this round: %d\n", game.shooting_player->boats_pieces_sunk );

        printf("Play again? (y/n) ");
        
        char again;
        fflush(stdin);
        scanf("%c", &again);

        if( again == 'y' || again == 'Y' ){
            new_game( &game );
            game.play_again = true;
            swap_players( &game );
        }
    }
    

}