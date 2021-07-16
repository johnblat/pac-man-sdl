// MAIN
#include <SDL2/SDL.h>
#include <stdio.h>
#include "tiles.h"
#include "resources.h"
#include "constants.h"
#include "comparisons.h"
#include "string.h"
#include "jb_types.h"

typedef struct TileSelectionPanel {
    int num_rows;
    int num_cols;
    int tile_size;
    SDL_Texture *texture_atlas;
} TileSelectionPanel;

typedef enum Mode {
    TILE_MODE,                    // Default ( No Key )
    DOT_MODE,                     // D Key
    WALL_MODE,                    // W Key 
    PAC_PLACEMENT_MODE,           // P Key
    GHOST_PEN_PLACEMENT_MODE,     // G Key
    E_POWER_PELLET_PLACEMENT_MODE,// E Key ( Energizer Power Pellet )
    SLOW_TILE_PLACEMENT_MODE      // V Key ( V has a down shape to it )
} Mode;

Mode current_mode = TILE_MODE;

// power pellet data
SDL_Point poopy_point = { -1, -1 };
int num_power_pellets = 0;
int num_slow_tiles = 0;

SDL_bool showWalls = SDL_FALSE;

/**
 * Returns the values in the num_rows and num_cols pointer parameters
 * It'll be something like 2x18 or something like that because an atlas 
 * can contain many tiles within
 */
void dimensions_of_atlas_surface( int tile_size, int *num_rows, int *num_cols, SDL_Surface *surface ) {
    *num_rows = surface->w / tile_size;
    *num_cols = surface->h / tile_size;
}

/**
 * ADDING OR REMOVING TILES TO MAP
 */
void add_selected_tile_to_position( SDL_Point mouse_position, TwoDimensionalArrayIndex atlas_indexes[ TILE_ROWS ][ TILE_COLS ], SDL_Point tile_map_screen_position, TwoDimensionalArrayIndex selection_panel_atlas_index ) {
    if ( mouse_position.y < tile_map_screen_position.y || mouse_position.y > SCREEN_HEIGHT ) {
        return;
    }

    SDL_Point grid_point = screen_point_to_tile_grid_point( mouse_position,  tile_map_screen_position );

    atlas_indexes[ grid_point.y ][ grid_point.x ] = selection_panel_atlas_index;
}


void remove_tile_from_position( SDL_Point mouse_position, TwoDimensionalArrayIndex atlas_indexes[ TILE_ROWS ][ TILE_COLS ], SDL_Point tile_map_screen_position, TwoDimensionalArrayIndex selection_panel_atlas_index ) {
    if ( mouse_position.y < tile_map_screen_position.y || mouse_position.y > SCREEN_HEIGHT ) {
        return;
    }

    SDL_Point grid_point = screen_point_to_tile_grid_point( mouse_position,  tile_map_screen_position );

    atlas_indexes[ grid_point.y ][ grid_point.x ].r = -1;
    atlas_indexes[ grid_point.y ][ grid_point.x ].c = -1;
}

// ALL THE SAME. COMPRESS
void add_dot_to_position( SDL_Point mouse_position, SDL_Point tile_map_screen_position, char dots[ TILE_ROWS ][ TILE_COLS ] ) {
    if ( mouse_position.y < tile_map_screen_position.y || mouse_position.y > SCREEN_HEIGHT ) {
        return;
    }

    SDL_Point grid_point = screen_point_to_tile_grid_point( mouse_position,  tile_map_screen_position );

    dots[ grid_point.y ][ grid_point.x ] = 'x';
}

void remove_dot_from_position( SDL_Point mouse_position, SDL_Point tile_map_screen_position, char dots[ TILE_ROWS ][ TILE_COLS ] ) {
    if ( mouse_position.y < tile_map_screen_position.y || mouse_position.y > SCREEN_HEIGHT ) {
        return;
    }

    SDL_Point grid_point = screen_point_to_tile_grid_point( mouse_position,  tile_map_screen_position );

    dots[ grid_point.y ][ grid_point.x ] = ' ';
}

void add_wall_to_position( SDL_Point mouse_position, SDL_Point tile_map_screen_position, char walls[ TILE_ROWS ][ TILE_COLS ] ) {
    if ( mouse_position.y < tile_map_screen_position.y || mouse_position.y > SCREEN_HEIGHT ) {
        return;
    }

    SDL_Point grid_point = screen_point_to_tile_grid_point( mouse_position,  tile_map_screen_position );

    walls[ grid_point.y ][ grid_point.x ] = 'x';
}

void remove_wall_from_position( SDL_Point mouse_position, SDL_Point tile_map_screen_position, char walls[ TILE_ROWS ][ TILE_COLS ] ) {
    if ( mouse_position.y < tile_map_screen_position.y || mouse_position.y > SCREEN_HEIGHT ) {
        return;
    }

    SDL_Point grid_point = screen_point_to_tile_grid_point( mouse_position,  tile_map_screen_position );

    walls[ grid_point.y ][ grid_point.x ] = ' ';
}

/**
 * renders a grid in whatever the current draw color is
 */
void render_grid( SDL_Renderer *renderer, int spacing, int x, int y, int w, int h) {
    
    for(int xi = x; xi < w + 1; xi += spacing) {
        SDL_RenderDrawLine( renderer, xi, y, xi, h );
    }

    for(int yi = y; yi < h + 1; yi += spacing) {
        SDL_RenderDrawLine( renderer, x, yi, w, yi );
    }
}


void render_tile_selection_panel( SDL_Renderer *renderer, TileSelectionPanel *panel, TwoDimensionalArrayIndex selected_texture_atlas_index ) {
    // texture atlas
    SDL_Rect render_dst_rect = {0, 0, panel->num_cols * panel->tile_size, panel->num_rows * panel->tile_size };
    SDL_RenderCopy( renderer, panel->texture_atlas, NULL, &render_dst_rect );
 

    // highlight selected
    if( current_mode == TILE_MODE ) {
        SDL_SetRenderDrawColor( renderer, 255,255,120,165);
        SDL_Rect rect_selected_texture_clip = { selected_texture_atlas_index.c * TILE_SIZE, selected_texture_atlas_index.r * TILE_SIZE, TILE_SIZE, TILE_SIZE };
        SDL_RenderFillRect( renderer, &rect_selected_texture_clip );
    }
    else if ( current_mode == DOT_MODE ) {
        SDL_SetRenderDrawColor( renderer, 255,200,0,255);
        int dot_size = 6;
        SDL_Rect dot_rect = { panel->num_cols * TILE_SIZE + TILE_SIZE/2 - dot_size/2, TILE_SIZE/2 - dot_size/2, dot_size,dot_size};
        SDL_RenderFillRect( renderer, &dot_rect );
    }
    else if( current_mode == WALL_MODE ) {
        SDL_SetRenderDrawColor( renderer, 255,200,150,255);
        int wall_size = 40;
        SDL_Rect wall_rect = { panel->num_cols * TILE_SIZE + TILE_SIZE/2 - wall_size/2, TILE_SIZE/2 - wall_size/2, wall_size,wall_size};
        SDL_RenderFillRect( renderer, &wall_rect );
    }
    else if( current_mode == PAC_PLACEMENT_MODE ) {
        SDL_SetRenderDrawColor( renderer, 242, 241, 57, 255 );
        int pac_size = 40;
        SDL_Rect pac_rect = { panel->num_cols * TILE_SIZE + TILE_SIZE/2 - pac_size/2, TILE_SIZE/2 - pac_size/2, pac_size,pac_size};
        SDL_RenderFillRect( renderer, &pac_rect );
    }
    else if( current_mode == E_POWER_PELLET_PLACEMENT_MODE ) {
        SDL_SetRenderDrawColor( renderer, 20,255,20,255);
        int pen_size = 40;
        SDL_Rect pen_rect = { panel->num_cols * TILE_SIZE + TILE_SIZE/2 - pen_size/2, TILE_SIZE/2 - pen_size/2, pen_size, pen_size };
        SDL_RenderFillRect( renderer, &pen_rect );
    }
    else if( current_mode == GHOST_PEN_PLACEMENT_MODE ) {
        SDL_SetRenderDrawColor( renderer, 255,20,20,255);
        int pen_size = 40;
        SDL_Rect pen_rect = { panel->num_cols * TILE_SIZE + TILE_SIZE/2 - pen_size/2, TILE_SIZE/2 - pen_size/2, pen_size, pen_size };
        SDL_RenderFillRect( renderer, &pen_rect );
    }
    // tile grid
    SDL_SetRenderDrawColor( renderer, 250,50,80,255);
    render_grid( renderer, panel->tile_size, 0, 0, render_dst_rect.w, render_dst_rect.h );

}


void update_selected_texture_index_from_screen_based_position( SDL_Point mouse_position, TileSelectionPanel *panel, TwoDimensionalArrayIndex *selection_index ) {
    if ( mouse_position.x > panel->num_cols * panel->tile_size ) {
        return;
    }
    else if ( mouse_position.y > panel->num_rows * panel->tile_size ) {
        return;
    }

    selection_index->r = mouse_position.y / panel->tile_size;
    selection_index->c = mouse_position.x / panel->tile_size;

}


int main( int argc, char *argv[] ) {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Point mouse_point;
    TileMap tilemap;
    TileSelectionPanel tile_selection_panel;
    TwoDimensionalArrayIndex selected_texture_atlas_index = {0, 0};
    unsigned int levelNum = 0;
    LevelConfig levelConfig;

    // INITIALIZEING LEVEL CONFIG
    SDL_Point zero = {0, 0};
    levelConfig.ghostPenTile = zero;
    levelConfig.pacStartingTile = zero;

    // determine what level to edit
    FILE *f = fopen("res/tilemap_editor_level", "r");
    char contents[ 2 ];
    fread( contents, sizeof(char), 2 , f );
    levelNum = strtol( contents, NULL, 10 );
    gCurrentLevel = levelNum;
    
    char *filename = "level";

    // Initializing stuff
    if( SDL_Init( SDL_INIT_VIDEO ) < 0) {
        fprintf( stderr, "Error %s\n", SDL_GetError() );
        exit( EXIT_FAILURE );
    }

    window = SDL_CreateWindow( "JB Pacmonster", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if ( window == NULL ) {
        fprintf( stderr, "Error %s\n ", SDL_GetError() );
        exit( EXIT_FAILURE );
    }

    renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED );
    if (renderer == NULL ) {
        fprintf( stderr, "Error %s\n ", SDL_GetError() );
        exit( EXIT_FAILURE );
    }

    SDL_SetRenderDrawBlendMode( renderer, SDL_BLENDMODE_BLEND );
    
    //tm_init_and_load_texture( renderer, &tilemap );
    // INITIALIZE TILEMAP
    tilemap.tm_texture_atlas = NULL;

    // try_load_resource_from_file( &pac_starting_tile, "res/pac_starting_tile", sizeof( SDL_Point ), 1 );
    // try_load_resource_from_file( &ghost_pen_tile, "res/ghost_pen_tile", sizeof( SDL_Point ), 1 );

    // set power pellets to poppoy tiels
    // for(int i = 0; i < 4; ++i ) {
    //     tm_power_pellet_tiles[ i ] = poopy_point;
    // }
    // try_load_resource_from_file( tm_power_pellet_tiles, "res/power_pellets", sizeof( SDL_Point ), 4 );

    // // set slow tiles to poopy
    // for( int i = 0; i < MAX_SLOW_TILES; ++i ) {
    //     tilemap.tm_slow_tiles[ i ] = poopy_point;
    // }

    // try_load_resource_from_file( tilemap.tm_slow_tiles, "res/slow_tiles", sizeof( SDL_Point ), MAX_SLOW_TILES );

    load_current_level_off_disk( &levelConfig, &tilemap, renderer );
    
    // placeholder
    tile_selection_panel.texture_atlas = tilemap.tm_texture_atlas;
    tile_selection_panel.tile_size = TILE_SIZE;

    // read in num rows and num cols
    
    char *tilesetDimensionsFilename = "tilemap_editor_tileset_dimensions";
    char fullPath[64] = {'\0'};
    snprintf(fullPath, 64, "res/levels/level%d/", levelNum );
    strncat(fullPath, tilesetDimensionsFilename, 64);

    FILE *df = fopen(fullPath, "r");
    char line[64] = {'\0'};
    while( fgets(line, 64, df) && line[0] == '#') {

    }

    tile_selection_panel.num_rows = strtol(line, NULL, 10);

    memset(line, '\0', 16);
    fgets(line, 16, df);

    tile_selection_panel.num_cols = strtol(line, NULL, 10);

    fclose(df);
    
    // end read in dimensions

    SDL_Event event;
    int quit = 0;

    int left_button_pressed = 0;
    int right_button_pressed = 0;

    // delta time - frame rate independent movement
    float max_delta_time = 1 / 60.0;
    float previous_frame_ticks = SDL_GetTicks() / 1000.0;

    while (!quit) {
        // semi-fixed timestep
        float current_frame_ticks = SDL_GetTicks() / 1000.0;
        float delta_time = current_frame_ticks - previous_frame_ticks;
        previous_frame_ticks = current_frame_ticks;
        // adjust for any pauses, debugging breaks, etc
        delta_time = delta_time < max_delta_time ?  delta_time : max_delta_time;

        while (SDL_PollEvent( &event ) != 0 ) {
            if( event.type == SDL_QUIT ) {
                quit = 1;
            }
            if ( event.type == SDL_KEYDOWN ) {
                if ( event.key.keysym.sym == SDLK_SPACE ) {
                    save_resource_to_file( tilemap.tm_texture_atlas_indexes, "res/tile_texture_map", sizeof( TwoDimensionalArrayIndex ), TOTAL_NUMBER_OF_TILES );
                }
            }
            if ( event.type == SDL_MOUSEBUTTONDOWN ) {
                if (  event.button.button == ( SDL_BUTTON_LEFT ) ) {
                    left_button_pressed = 1;
                }
                else if ( event.button.button == ( SDL_BUTTON_RIGHT ) ) {
                    right_button_pressed = 1;
                }
                
            }
            if ( event.type == SDL_MOUSEBUTTONUP ) {
                if (  event.button.button == ( SDL_BUTTON_LEFT ) ) {
                    left_button_pressed = 0;
                }
                else if ( event.button.button == ( SDL_BUTTON_RIGHT ) ) {
                    right_button_pressed = 0;
                }
            }
            // TODO: examine and see if some of these should be key up
            else if (event.type == SDL_KEYDOWN ) {
                if ( event.key.keysym.sym == SDLK_ESCAPE ) {
                    quit = SDL_TRUE;
                }
                else if ( event.key.keysym.sym == SDLK_s ) {
                    save_current_level_to_disk( &levelConfig, &tilemap);

                    SDL_SetRenderDrawColor(renderer, 0,100,0,255);
                    SDL_Rect screen_rect = {0,0, SCREEN_WIDTH, SCREEN_HEIGHT };
                    SDL_RenderFillRect( renderer, &screen_rect);
                    SDL_RenderPresent( renderer );
                    SDL_Delay( 200 );
                    
                }
                else if ( event.key.keysym.sym == SDLK_l ) {
                    try_load_resource_from_file( tilemap.tm_texture_atlas_indexes, "res/maze_file", sizeof( TwoDimensionalArrayIndex ), TOTAL_NUMBER_OF_TILES );
                    try_load_resource_from_file( tilemap.tm_dots, "res/dots", sizeof( char ), TOTAL_NUMBER_OF_TILES );
                    try_load_resource_from_file( tilemap.tm_walls, "res/walls", sizeof( char ), TOTAL_NUMBER_OF_TILES );
                    try_load_resource_from_file( &levelConfig.pacStartingTile, "res/pac_starting_tile", sizeof( SDL_Point ), 1 );
                    try_load_resource_from_file( &levelConfig.powerPelletTiles, "res/power_pellets", sizeof( SDL_Point ), 4); 
                    try_load_resource_from_file( &levelConfig.ghostPenTile, "res/ghost_pen_tile", sizeof( SDL_Point ), 1 );
                    try_load_resource_from_file( tilemap.tm_slow_tiles, "res/slow_tiles", sizeof( SDL_Point ), MAX_SLOW_TILES );

                    SDL_SetRenderDrawColor(renderer, 255,100,100,255);
                    SDL_Rect screen_rect = {0,0, SCREEN_WIDTH, SCREEN_HEIGHT };
                    SDL_RenderFillRect( renderer, &screen_rect);
                    SDL_RenderPresent( renderer );
                    SDL_Delay( 200 );
                }
                else if( event.key.keysym.sym == SDLK_d ) {
                    current_mode = current_mode == DOT_MODE ? TILE_MODE : DOT_MODE;
                }
                else if( event.key.keysym.sym == SDLK_w ) {
                    current_mode = current_mode == WALL_MODE ? TILE_MODE : WALL_MODE;
                }
                else if( event.key.keysym.sym == SDLK_y ) {
                    showWalls = SDL_TRUE;
                }
                else if (event.key.keysym.sym == SDLK_n ) {
                    showWalls = SDL_FALSE;
                }
                else if( event.key.keysym.sym == SDLK_p ) {
                    current_mode = current_mode == PAC_PLACEMENT_MODE ? TILE_MODE : PAC_PLACEMENT_MODE;
                }
                else if( event.key.keysym.sym == SDLK_g ) {
                    current_mode = current_mode == GHOST_PEN_PLACEMENT_MODE ? TILE_MODE : GHOST_PEN_PLACEMENT_MODE;
                }
                else if( event.key.keysym.sym == SDLK_e ) {
                    current_mode = current_mode == E_POWER_PELLET_PLACEMENT_MODE ? TILE_MODE : E_POWER_PELLET_PLACEMENT_MODE;
                }
                else if( event.key.keysym.sym == SDLK_g ) {
                    current_mode = current_mode == GHOST_PEN_PLACEMENT_MODE ? TILE_MODE : GHOST_PEN_PLACEMENT_MODE;
                }
                else if( event.key.keysym.sym == SDLK_t ) {
                    current_mode = TILE_MODE;
                }
                else if( event.key.keysym.sym == SDLK_v ) {
                    current_mode = current_mode == SLOW_TILE_PLACEMENT_MODE ? TILE_MODE : SLOW_TILE_PLACEMENT_MODE;
                }
                
            }
            else if ( event.type == SDL_KEYUP ) {
                if( event.key.keysym.sym == SDLK_PAGEUP ) {
                    // go up a level. Wrap around
                }
                else if( event.key.keysym.sym == SDLK_PAGEDOWN ) {
                    // go down a level. Wrap around
                }
            }
        }

        const Uint8 *current_key_states = SDL_GetKeyboardState( NULL );
        if( current_key_states[ SDL_SCANCODE_DOWN ] ) {
            tilemap.tm_screen_position.y++;
            // increase double speed if shift held down
            if( current_key_states[ SDL_SCANCODE_LSHIFT ] ) {
                tilemap.tm_screen_position.y+=4;
            }
        }
        if( current_key_states[ SDL_SCANCODE_UP ] ) {
            tilemap.tm_screen_position.y--;
            // increase double speed if shift held down

            if( current_key_states[ SDL_SCANCODE_LSHIFT ] ) {
                tilemap.tm_screen_position.y-=4;
            }
        }
        if( current_key_states[ SDL_SCANCODE_RIGHT ] ) {
            tilemap.tm_screen_position.x++;
            // increase double speed if shift held down
            if( current_key_states[ SDL_SCANCODE_LSHIFT ] ) {
                tilemap.tm_screen_position.x+=4;
            }
        }
        if( current_key_states[ SDL_SCANCODE_LEFT ] ) {
            tilemap.tm_screen_position.x--;
            // increase double speed if shift held down
            if( current_key_states[ SDL_SCANCODE_LSHIFT ] ) {
                tilemap.tm_screen_position.x-=4;
            }
        }

        if( current_key_states[ SDL_SCANCODE_W ] && current_key_states[ SDL_SCANCODE_LSHIFT ] ) {
            showWalls = !showWalls;
        }

        if( current_key_states[ SDL_SCANCODE_W ] && !current_key_states[ SDL_SCANCODE_LSHIFT ] ) {
            showWalls = !showWalls;
        }

        if ( left_button_pressed ) {
            SDL_GetMouseState( &mouse_point.x, &mouse_point.y );
            SDL_Point tile_grid_point;
            tile_grid_point = screen_point_to_tile_grid_point( mouse_point, tilemap.tm_screen_position );

            if ( tile_grid_point.x > TILE_COLS - 1) {
                tile_grid_point.x = TILE_COLS - 1;
            }
            if ( tile_grid_point.y > TILE_ROWS - 1) {
                tile_grid_point.y = TILE_ROWS - 1;
            }
            
            //TODO: add selected tile to position
            update_selected_texture_index_from_screen_based_position( mouse_point, &tile_selection_panel, &selected_texture_atlas_index );
            if( current_mode == TILE_MODE ) {
                add_selected_tile_to_position( mouse_point, tilemap.tm_texture_atlas_indexes, tilemap.tm_screen_position, selected_texture_atlas_index );
            }
            else if (current_mode == DOT_MODE ) {
                add_dot_to_position( mouse_point, tilemap.tm_screen_position, tilemap.tm_dots );
            }
            else if( current_mode == WALL_MODE ) {
                add_wall_to_position( mouse_point, tilemap.tm_screen_position, tilemap.tm_walls );
            }
            else if ( current_mode == PAC_PLACEMENT_MODE ) {
                SDL_Point tile = screen_point_to_tile_grid_point( mouse_point, tilemap.tm_screen_position );
                levelConfig.pacStartingTile.x = tile.x;
                levelConfig.pacStartingTile.y = tile.y;
            }
            else if( current_mode == E_POWER_PELLET_PLACEMENT_MODE && num_power_pellets < 4 ) {
                SDL_Point tile = screen_point_to_tile_grid_point( mouse_point, tilemap.tm_screen_position );

                // TODO don't use flag 
                SDL_bool tile_already_exists = SDL_FALSE;
                for( int i = 0; i < 4; ++i ) {
                    // tile already exists
                    if( points_equal( levelConfig.powerPelletTiles[i], tile ) ) tile_already_exists = SDL_TRUE;
                }

                if( !tile_already_exists ) {
                    for( int i = 0; i < 4; ++i ) {
                        if( points_equal( levelConfig.powerPelletTiles[ i ], poopy_point ) ) {
                            levelConfig.powerPelletTiles[ i ] = tile;
                            break;
                        }
                    }
                    num_power_pellets++;
                }
                
                
            }
            else if( current_mode == GHOST_PEN_PLACEMENT_MODE ) {
                SDL_Point tile = screen_point_to_tile_grid_point( mouse_point, tilemap.tm_screen_position );
                levelConfig.ghostPenTile = tile;
            }
            else if( current_mode == SLOW_TILE_PLACEMENT_MODE && num_slow_tiles < MAX_SLOW_TILES) {
                SDL_Point tile = screen_point_to_tile_grid_point( mouse_point, tilemap.tm_screen_position );

                SDL_bool tile_already_exists = SDL_FALSE;
                for( int i = 0; i < MAX_SLOW_TILES; ++i ) {
                    // tile already exists
                    if( points_equal( tilemap.tm_slow_tiles[ i ], tile ) ) tile_already_exists = SDL_TRUE;
                }

                if( !tile_already_exists ) {
                    for( int i = 0; i < MAX_SLOW_TILES; ++i ) {
                        if( points_equal( tilemap.tm_slow_tiles[ i ], poopy_point ) ) {
                            tilemap.tm_slow_tiles[ i ] = tile;
                            break;
                        }
                    }
                    num_slow_tiles++;
                }
            }
            
            //tile_map[ tile_grid_point.y ][ tile_grid_point.x ] = 'x';
        }
        else if ( right_button_pressed ) {
            SDL_GetMouseState( &mouse_point.x, &mouse_point.y );
            SDL_Point tile_grid_point;
            tile_grid_point.x = mouse_point.x / TILE_SIZE;//* 0.015625;
            tile_grid_point.y = mouse_point.y / TILE_SIZE;//* 0.015625;
            if ( tile_grid_point.x > TILE_COLS - 1) {
                tile_grid_point.x = TILE_COLS - 1;
            }
            if ( tile_grid_point.y > TILE_ROWS - 1) {
                tile_grid_point.y = TILE_ROWS - 1;
            }
            //TODO Add selected tile to position
            //tile_map[ tile_grid_point.y ][ tile_grid_point.x ] = '\0';
            if( current_mode == TILE_MODE ) {
                remove_tile_from_position( mouse_point, tilemap.tm_texture_atlas_indexes, tilemap.tm_screen_position, selected_texture_atlas_index );
            }
            else if( current_mode == DOT_MODE ) {
                remove_dot_from_position( mouse_point, tilemap.tm_screen_position, tilemap.tm_dots );
            }
            else if( current_mode == WALL_MODE ) {
                remove_wall_from_position( mouse_point, tilemap.tm_screen_position, tilemap.tm_walls );
            }
            else if( current_mode == E_POWER_PELLET_PLACEMENT_MODE ) {
                SDL_Point tile = screen_point_to_tile_grid_point( mouse_point, tilemap.tm_screen_position );
                for( int i = 0 ; i < 4; ++i ) {
                    if( points_equal( levelConfig.powerPelletTiles[ i ], tile ) ) {
                        levelConfig.powerPelletTiles[ i ].x = -1;
                        levelConfig.powerPelletTiles[ i ].y = -1;
                        num_power_pellets--;
                        break;
                    }
                }
            }
            else if( current_mode == SLOW_TILE_PLACEMENT_MODE ) {
                SDL_Point tile = screen_point_to_tile_grid_point( mouse_point, tilemap.tm_screen_position );
                for( int i = 0 ; i < MAX_SLOW_TILES; ++i ) {
                    if( points_equal( tilemap.tm_slow_tiles[ i ], tile ) ) {
                        tilemap.tm_slow_tiles[ i ].x = -1;
                        tilemap.tm_slow_tiles[ i ].y = -1;
                        num_slow_tiles--;
                        break;
                    }
                }
            }
            
        }

        /**********
         * UPDATE DOTS ANIMATION
         * **********/
        int top_bound = DOT_PADDING;
        int bottom_bound = TILE_SIZE - DOT_PADDING;
        for( int r = 0; r < TILE_ROWS; ++r ) {
            for( int c = 0; c < TILE_COLS; ++c ) {
                
                tilemap.tm_dot_particles[ r ][ c ].position.y += tilemap.tm_dot_particles[ r ][ c ].velocity.y * DOT_SPEED * delta_time ;

                if ( tilemap.tm_dot_particles[ r ][ c ].position.y < top_bound ) {
                    tilemap.tm_dot_particles[ r ][ c ].position.y = top_bound;
                    tilemap.tm_dot_particles[ r ][ c ].velocity.y = 1;
                }
                if ( tilemap.tm_dot_particles[ r ][ c ].position.y > bottom_bound) {
                    tilemap.tm_dot_particles[ r ][ c ].position.y = bottom_bound;
                    
                    tilemap.tm_dot_particles[ r ][ c ].velocity.y = -1;
                }
            }
        }

        SDL_SetRenderDrawColor( renderer, 0,0,0,255);
        SDL_RenderClear( renderer );

        

        tm_render_with_screen_position_offset( renderer, &tilemap );

        SDL_SetRenderDrawColor( renderer, 242, 241, 57, 150 );
        SDL_Point pac_screen_point = tile_grid_point_to_screen_point(levelConfig.pacStartingTile, tilemap.tm_screen_position);
        SDL_Rect pac_rect = { pac_screen_point.x, pac_screen_point.y, TILE_SIZE, TILE_SIZE};
        SDL_RenderFillRect( renderer, &pac_rect );

        // render power pellets
        SDL_SetRenderDrawColor( renderer, 15, 250, 15, 150 );
        for( int i = 0; i < 4; ++i ) {
            if( points_equal( levelConfig.powerPelletTiles[ i ], poopy_point ) ) continue;
            SDL_Point power_pellet_screen_point = tile_grid_point_to_screen_point( levelConfig.powerPelletTiles[ i ], tilemap.tm_screen_position );
            SDL_Rect power_pellet_rect = { power_pellet_screen_point.x, power_pellet_screen_point.y, TILE_SIZE, TILE_SIZE };
            SDL_RenderFillRect( renderer, &power_pellet_rect );
        }

        // render slow tiles
        SDL_SetRenderDrawColor( renderer, 84, 67, 45, 150 );
        for( int i = 0; i < MAX_SLOW_TILES; i++ ) {
            if( points_equal( tilemap.tm_slow_tiles[ i ], poopy_point ) ) continue;
            SDL_Point slow_tile_point = tile_grid_point_to_screen_point( tilemap.tm_slow_tiles[ i ], tilemap.tm_screen_position );
            SDL_Rect slow_rect = { slow_tile_point.x, slow_tile_point.y, TILE_SIZE, TILE_SIZE };
            SDL_RenderFillRect( renderer, &slow_rect );
        }

        // render ghost pen
        SDL_SetRenderDrawColor( renderer, 255,20,20,150);
        SDL_Point pen_screen_point = tile_grid_point_to_screen_point( levelConfig.ghostPenTile, tilemap.tm_screen_position );

        SDL_Rect pen_rect = {pen_screen_point.x, pen_screen_point.y, TILE_SIZE, TILE_SIZE};
        SDL_RenderFillRect( renderer, &pen_rect );

        // render walls - no need to render normally because won't see it during actual game.
        
        if( showWalls || ( current_mode == WALL_MODE) ) {
            SDL_SetRenderDrawColor( renderer, 255, 255, 255, 150 );
            for( int row = 0; row < TILE_ROWS; ++row ) {
                for( int col = 0; col < TILE_COLS; ++col ) {
                    if( tilemap.tm_walls[ row ][ col ] == 'x' ) {

                        int dot_relative_to_tile_y_position = tilemap.tm_dot_particles[ row ][ col ].position.y * (TILE_SIZE);
                        SDL_Rect dot_rect = {
                            tilemap.tm_screen_position.x + ( TILE_SIZE * col ),
                            tilemap.tm_screen_position.y + ( TILE_SIZE * row ),
                            TILE_SIZE,
                            TILE_SIZE
                        };
                        
                        SDL_RenderFillRect( renderer, &dot_rect );
                    }
                }
            } 
        }
            
        
        render_tile_selection_panel( renderer, &tile_selection_panel, selected_texture_atlas_index );
        
        

        SDL_SetRenderDrawColor( renderer, 50,50,50,255);
        render_grid( renderer, TILE_SIZE, tilemap.tm_screen_position.x, tilemap.tm_screen_position.y, SCREEN_WIDTH, SCREEN_HEIGHT );

        

        // draw line down the middle
        SDL_SetRenderDrawColor( renderer, 255,255,255,255);
        int x = SCREEN_WIDTH * 0.5;
        int y = SCREEN_HEIGHT * 0.5 + TILE_SIZE;
        SDL_RenderDrawLine( renderer, x, 0, x, SCREEN_HEIGHT);

        SDL_RenderDrawLine( renderer, 0, y, SCREEN_WIDTH, y );

        SDL_RenderPresent(renderer);
        SDL_Delay(10);
    
    }
    // CLOSE DOWN
    SDL_DestroyRenderer( renderer );
    SDL_DestroyWindow( window );
    SDL_Quit();
}