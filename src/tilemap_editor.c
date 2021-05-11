// MAIN
#include <SDL2/SDL.h>
#include <stdio.h>
#include "tiles.h"
#include "resources.h"
#include "constants.h"
#include "jb_types.h"

typedef struct TileSelectionPanel {
    int num_rows;
    int num_cols;
    int tile_size;
    SDL_Texture *texture_atlas;
} TileSelectionPanel;

typedef enum Mode {
    TILE_MODE,
    DOT_MODE
} Mode;

Mode current_mode = TILE_MODE;
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
    // tile grid
    SDL_SetRenderDrawColor( renderer, 250,50,80,255);
    render_grid( renderer, panel->tile_size, 0, 0, render_dst_rect.w, render_dst_rect.h );

}

// void save_tile_map_atlas_indexes_to_file( TileMap *tile_map ) {
//     SDL_RWops *write_context = SDL_RWFromFile("maze_file", "wb" );
//     SDL_RWwrite( write_context, tile_map->tm_texture_atlas_indexes, sizeof( TwoDimensionalArrayIndex ), TILE_ROWS * TILE_COLS );
//     SDL_RWclose( write_context );
// }

// void load_tile_map_atlas_indexes_from_file( TileMap *tile_map ){
//     SDL_RWops *read_context = SDL_RWFromFile( "maze_file", "rb" );
//     size_t file_size_bytes = SDL_RWsize( read_context );
//     size_t num_bytes_read = SDL_RWread( read_context, tile_map->tm_texture_atlas_indexes, sizeof( TwoDimensionalArrayIndex ), TILE_ROWS * TILE_COLS );

// }


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
    
    tm_init_and_load_texture( renderer, &tilemap, "maze_file" );
    // placeholder
    tile_selection_panel.texture_atlas = tilemap.tm_texture_atlas;
    tile_selection_panel.tile_size = TILE_SIZE;
    tile_selection_panel.num_cols = 21;
    tile_selection_panel.num_rows = 2;
    
    SDL_Event event;
    int quit = 0;

    int left_button_pressed = 0;
    int right_button_pressed = 0;

    while (!quit) {
        while (SDL_PollEvent( &event ) != 0 ) {
            if( event.type == SDL_QUIT ) {
                quit = 1;
            }
            if ( event.type == SDL_KEYDOWN ) {
                if ( event.key.keysym.sym == SDLK_SPACE ) {
                    save_tilemap_texture_atlas_indexes_to_file( tilemap.tm_texture_atlas_indexes );
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
            else if (event.type == SDL_KEYDOWN ) {
                if ( event.key.keysym.sym == SDLK_ESCAPE ) {
                    quit = SDL_TRUE;
                }
                else if ( event.key.keysym.sym == SDLK_s ) {
                    save_tilemap_texture_atlas_indexes_to_file( tilemap.tm_texture_atlas_indexes );
                    save_dots_to_file( tilemap.tm_dots );
                    SDL_SetRenderDrawColor(renderer, 0,100,0,255);
                    SDL_Rect screen_rect = {0,0, SCREEN_WIDTH, SCREEN_HEIGHT };
                    SDL_RenderFillRect( renderer, &screen_rect);
                    SDL_RenderPresent( renderer );
                    SDL_Delay( 200 );
                    
                }
                else if ( event.key.keysym.sym == SDLK_l ) {
                    try_load_tilemap_texture_atlas_indexes_from_file( tilemap.tm_texture_atlas_indexes );
                    try_load_dots_from_file( tilemap.tm_dots );
                    SDL_SetRenderDrawColor(renderer, 255,100,100,255);
                    SDL_Rect screen_rect = {0,0, SCREEN_WIDTH, SCREEN_HEIGHT };
                    SDL_RenderFillRect( renderer, &screen_rect);
                    SDL_RenderPresent( renderer );
                    SDL_Delay( 200 );
                }
                else if( event.key.keysym.sym == SDLK_d ) {
                    current_mode = current_mode == DOT_MODE ? TILE_MODE : DOT_MODE;
                }
            }
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
        }

        SDL_SetRenderDrawColor( renderer, 0,0,0,255);
        SDL_RenderClear( renderer );

        tm_render_with_screen_position_offset( renderer, &tilemap );
        render_tile_selection_panel( renderer, &tile_selection_panel, selected_texture_atlas_index );
        
        SDL_SetRenderDrawColor( renderer, 50,50,50,255);
        render_grid( renderer, TILE_SIZE, 0, TILE_SIZE * 2, SCREEN_WIDTH, SCREEN_HEIGHT );

        SDL_RenderPresent(renderer);
        SDL_Delay(50);
    
    }
    // CLOSE DOWN
    SDL_DestroyRenderer( renderer );
    SDL_DestroyWindow( window );
    SDL_Quit();
}