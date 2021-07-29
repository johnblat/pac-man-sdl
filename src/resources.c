#include <dirent.h>
#include <SDL2/SDL.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "animation.h"
#include "entity.h"
#include "render.h"
#include "resources.h"
#include "tiles.h"
#include "globalData.h"


const int MAX_FILENAME_SIZE = 64;

unsigned int gCurrentLevel = 0;
unsigned int gNumLevels = 0; 

int determine_number_of_levels_from_dirs( ) {
    // int num_levels = 0;
    DIR *res_dir = opendir("res/levels");
    struct dirent *d;
    char *first_five_chars = "level";
    while( ( d = readdir( res_dir ) ) != NULL ) {
        if( strncmp( first_five_chars, d->d_name, 5 ) == 0 ) {
            gNumLevels++;
        }

    } 
    closedir(res_dir);
    return gNumLevels;
}

static void build_resource_file_path(char *fullResourcePath, char *fullLevelDir, char *resourceFileName ) {
    memset( fullResourcePath, '\0', MAX_FILENAME_SIZE );
    strncat( fullResourcePath, fullLevelDir, MAX_FILENAME_SIZE );
    strncat( fullResourcePath, resourceFileName, MAX_FILENAME_SIZE );
}

void createGhostsFromFile( Entities *entities ) {

}

/**
 * Needs to be called on game start.
 * Further alterations to the player on level advancements should be handled elsewhere
 * Player should not be removed and initialized again while game is active
 * 
 * NOTE: load_level_off_disk() should be called before this
 */
void initializePlayersFromFiles( Entities *entities, LevelConfig *levelConfig, unsigned int numPlayers ) {
    // read in file
    SDL_RWops *defaultSpeedReadContext = SDL_RWFromFile("res/default_pac_speed", "r");
    unsigned long size = defaultSpeedReadContext->size(defaultSpeedReadContext);
    char *content = (char *)malloc( size + 1 );
    SDL_RWread( defaultSpeedReadContext, content, size, 1 );
    content[ size ] = '\0';
    gBaseSpeed = strtol(content, NULL , 10);
    free( content );
    SDL_RWclose( defaultSpeedReadContext );

    levelConfig->baseSpeed = gBaseSpeed;

    char *playerSpritesFilename = "res/player_animated_sprites";
    FILE *f = fopen( playerSpritesFilename, "r" );
    if( f == NULL ) {
        fprintf(stderr, "Error opening file %s\n", playerSpritesFilename );
    }

    char line[ 64 ];
    for( int i = 0; i < numPlayers; i++ ) {
        do {
            fgets( line, 64, f );
        } while( line[ 0 ] == '#');

        unsigned int values[ 4 ];

        unsigned int line_idx = 0;

        for( int i = 0; i < 4; i++ ) {
            values[ i ] = strtol( line + line_idx, NULL, 10 );

            while( line[ line_idx ] != ' ' && line[ line_idx ] != '\n' ) {
                line_idx++;
            }
            line_idx++;
        }

        AnimatedSprite *animatedSprite = init_animation( values[ 0 ], values[ 1 ], values[ 2 ], values[ 3 ] );

        createPlayer( entities, levelConfig, animatedSprite );
    }
}

/**
 * Needs to be called on game start.
 * If more ghosts are needed on a certain level this can be run again,
 * however there should be some kind of teardown function if this is added
 * 
 * NOTE: levelConfig should be set by loading_level_off_disk() before calling this 
 */
void initializeGhostsFromFile( Entities *entities, LevelConfig *levelConfig, const char *animatedSpritesFilename  ) {

    char *ghostSpritesFilename = "res/ghost_animated_sprites";
    FILE *f = fopen( ghostSpritesFilename, "r" );
    if( f == NULL ) {
        fprintf(stderr, "Error opening file %s\n", ghostSpritesFilename );
    }

    char line[ 64 ] = {'\0'};
    memset(line, '\0', 64);

    for( int i = 0; i < 4; i++ ) { // 0 thru 4 will also be used for setting the targeting behavior. This will need to change if more ghosts are added with varying targeting behaviors
        do {
            fgets( line, 64, f );
        } while( line[ 0 ] == '#');

        unsigned int values[ 4 ];

        unsigned int line_idx = 0;

        for( int i = 0; i < 4; i++ ) {
            values[ i ] = strtol( line + line_idx, NULL, 10 );

            while( line[ line_idx ] != ' ' && line[ line_idx ] != '\n' ) {
                line_idx++;
            }
            line_idx++;
        }

        AnimatedSprite *animatedSprite = init_animation( values[ 0 ], values[ 1 ], values[ 2 ], values[ 3 ] );

        EntityId ghostId = createGhost( entities, levelConfig, animatedSprite, i );
        gGhostIds[ gNumGhosts ] = ghostId;
        gNumGhosts++;
    }
}

/**
 * NOTE: Must call load_level_off_disk() first, then send levelConfig here
 * Power pellets should be updated every level to get new tiles.
 * This is really for setting the animated sprites
 */
void initializePowerPelletsFromFile( ) {

}

void save_current_level_to_disk( LevelConfig *levelConfig, TileMap *tilemap ) {

    char *res_level_dir = "res/levels/";
    char level_dirname[ MAX_FILENAME_SIZE ];
    memset( level_dirname, '\0', MAX_FILENAME_SIZE );
    snprintf(level_dirname, MAX_FILENAME_SIZE, "level%d/", gCurrentLevel );
    char fullLevelDir[ MAX_FILENAME_SIZE ];
    memset( fullLevelDir, '\0', MAX_FILENAME_SIZE );
    strncat(fullLevelDir, res_level_dir, 11 );
    strncat(fullLevelDir, level_dirname, MAX_FILENAME_SIZE );

    // construct paths for each resource file

    char *ghostPenTileFileName = "ghost_pen_tile";
    char *pacStartingTileFileName = "pac_starting_tile";
    char *dotsFileName = "dots";
    char *powerPelletsFileName = "power_pellets";
    char *slowTilesFileName = "slow_tiles";
    char *tileTextureMapFileName = "tile_texture_map";
    char *wallsFileName = "walls";
    //char *tilesetFileName = "tileset.png";

    char fullResourcePath[ MAX_FILENAME_SIZE ];

    // build resource paths, and save them
    build_resource_file_path( fullResourcePath, fullLevelDir, ghostPenTileFileName );
    save_resource_to_file( &levelConfig->ghostPenTile, fullResourcePath, sizeof( SDL_Point ), 1 );

    build_resource_file_path( fullResourcePath, fullLevelDir, pacStartingTileFileName );
    save_resource_to_file( &levelConfig->pacStartingTile, fullResourcePath, sizeof( SDL_Point ), 1 );

    build_resource_file_path( fullResourcePath, fullLevelDir, dotsFileName );
    save_resource_to_file( tilemap->tm_dots, fullResourcePath, sizeof( char ), TOTAL_NUMBER_OF_TILES );

    build_resource_file_path( fullResourcePath, fullLevelDir, powerPelletsFileName );
    save_resource_to_file( &levelConfig->powerPelletTiles, fullResourcePath, sizeof( SDL_Point ), 4); 

    build_resource_file_path( fullResourcePath, fullLevelDir, slowTilesFileName );
    save_resource_to_file( tilemap->tm_slow_tiles, fullResourcePath, sizeof( SDL_Point ), MAX_SLOW_TILES );

    build_resource_file_path( fullResourcePath, fullLevelDir, tileTextureMapFileName );
    save_resource_to_file( tilemap->tm_texture_atlas_indexes, fullResourcePath, sizeof( TwoDimensionalArrayIndex ), TOTAL_NUMBER_OF_TILES );

    build_resource_file_path( fullResourcePath, fullLevelDir, wallsFileName );
    save_resource_to_file( tilemap->tm_walls, fullResourcePath, sizeof( char ), TOTAL_NUMBER_OF_TILES );

    
}

void load_current_level_off_disk( LevelConfig *levelConfig, TileMap *tilemap, SDL_Renderer *renderer ) { 
    if( gCurrentLevel > gNumLevels ) {
        fprintf( stderr, "Could not load level because requested level number greater than %d. Max is %d\n", gNumLevels, gCurrentLevel );
    }


    // zero out resources
    // initialize textured tiles
    for( int row = 0; row < TILE_ROWS; ++row ) {
        for ( int col = 0; col < TILE_COLS; ++col ) {
            tilemap->tm_texture_atlas_indexes[ row ][ col ] = EMPTY_TILE_TEXTURE_ATLAS_INDEX;
        }
    }

    // initialize dots
    for( int row = 0; row < TILE_ROWS; row++ ){
        for( int col = 0; col < TILE_COLS; col++ ) {
            tilemap->tm_dots[ row ][ col ] = ' ';
        }
    }

    // initialize wall
    for( int row = 0; row < TILE_ROWS; row++ ) {
        for( int col = 0; col < TILE_COLS; col++ ) {
            tilemap->tm_walls[ row ][ col ] = ' ';
        }
    }


    int innerSign = 1;
    int outerSign = 1;
    int top_bound = DOT_PADDING;
    int bottom_bound = TILE_SIZE - DOT_PADDING;
    //int num_dot_positions = TILE_SIZE - DOT_PADDING * 2;
    int startingyPos = top_bound;
    int yPos = top_bound;
    for( int row = 0; row < TILE_ROWS; row++ ) {
        for( int col = 0; col < TILE_COLS; col++ ) {
            tilemap->tm_dot_particles[ row ][ col ].position.x = ( TILE_SIZE * 0.5 );
            tilemap->tm_dot_particles[ row ][ col ].position.y = yPos;
            if( innerSign ) {
                tilemap->tm_dot_particles[ row ][ col ].velocity.y = 1;
                yPos++;
            }
            else {
                tilemap->tm_dot_particles[ row ][ col ].velocity.y = -1;
                yPos--;
            }
            if( yPos >= bottom_bound ) {
                innerSign = 0;
            }
            else if( yPos <= top_bound ) {
                innerSign = 1;
            }

        }

        if( outerSign ) {
            startingyPos++;

        }
        else {
            startingyPos--;
        }
        if( startingyPos >= bottom_bound ) {
            outerSign = 0;
        }
        else if( startingyPos <= top_bound ) {
            outerSign = 1;
        }
        yPos = startingyPos;

        innerSign = outerSign;
    }

    // initialize slow tiles
    for( int i = 0; i < MAX_SLOW_TILES; i++ ) {
        tilemap->tm_slow_tiles[ i ].x = -1;
        tilemap->tm_slow_tiles[ i ].y = -1;
    }

    // initialize screen position
    tilemap->tm_screen_position.x = TILE_SIZE/2;
    tilemap->tm_screen_position.y = TILE_SIZE * 2; 

    char *res_level_dir = "res/levels/";
    char level_dirname[ MAX_FILENAME_SIZE ];
    memset( level_dirname, '\0', MAX_FILENAME_SIZE );
    snprintf(level_dirname, MAX_FILENAME_SIZE, "level%d/", gCurrentLevel );
    char fullLevelDir[ MAX_FILENAME_SIZE ];
    memset( fullLevelDir, '\0', MAX_FILENAME_SIZE );
    strncat(fullLevelDir, res_level_dir, 11 );
    strncat(fullLevelDir, level_dirname, MAX_FILENAME_SIZE );

    // construct paths for each resource file

    char *ghostModeTimesFileName = "ghost_mode_times";
    char *ghostPenTileFileName = "ghost_pen_tile";
    char *pacStartingTileFileName = "pac_starting_tile";
    char *dotsFileName = "dots";
    char *powerPelletsFileName = "power_pellets";
    char *slowTilesFileName = "slow_tiles";
    char *tileTextureMapFileName = "tile_texture_map";
    char *wallsFileName = "walls";
    char *tilesetFileName = "tileset.png";
    char *pickupsFileName = "pickups";

    char fullResourcePath[ MAX_FILENAME_SIZE ];

    build_resource_file_path( fullResourcePath, fullLevelDir, ghostModeTimesFileName );
    load_ghost_mode_times_from_config_file( levelConfig->scatterChasePeriodSeconds, levelConfig->numScatterChasePeriods, fullResourcePath );

    build_resource_file_path( fullResourcePath, fullLevelDir, ghostPenTileFileName );
    try_load_resource_from_file( &levelConfig->ghostPenTile, fullResourcePath, sizeof(SDL_Point), 1 );

    build_resource_file_path( fullResourcePath, fullLevelDir, pacStartingTileFileName );
    try_load_resource_from_file( &levelConfig->pacStartingTile, fullResourcePath, sizeof( SDL_Point ), 1 );

    build_resource_file_path( fullResourcePath, fullLevelDir, dotsFileName );
    try_load_resource_from_file( tilemap->tm_dots, fullResourcePath, sizeof( char ) , TOTAL_NUMBER_OF_TILES );

    build_resource_file_path( fullResourcePath, fullLevelDir, powerPelletsFileName );
    try_load_resource_from_file( levelConfig->powerPelletTiles, fullResourcePath, sizeof( SDL_Point ), 4 );

    build_resource_file_path( fullResourcePath, fullLevelDir, slowTilesFileName );
    try_load_resource_from_file( tilemap->tm_slow_tiles , fullResourcePath, sizeof( SDL_Point ), MAX_SLOW_TILES );

    build_resource_file_path( fullResourcePath, fullLevelDir, tileTextureMapFileName );
    try_load_resource_from_file( tilemap->tm_texture_atlas_indexes, fullResourcePath, sizeof(TwoDimensionalArrayIndex), TOTAL_NUMBER_OF_TILES );

    build_resource_file_path( fullResourcePath, fullLevelDir, wallsFileName );
    try_load_resource_from_file( tilemap->tm_walls, fullResourcePath, sizeof( char ), TOTAL_NUMBER_OF_TILES );

    build_resource_file_path( fullResourcePath, fullLevelDir, pickupsFileName );
    tryLoadPickupsFromConfigFile( levelConfig, fullResourcePath );

    SDL_Surface *surface;

    build_resource_file_path( fullResourcePath, fullLevelDir, tilesetFileName );
    surface = IMG_Load(fullResourcePath);
   
    if( tilemap->tm_texture_atlas != NULL ) {
        SDL_DestroyTexture( tilemap->tm_texture_atlas );
        tilemap->tm_texture_atlas = NULL;
    }
    tilemap->tm_texture_atlas = SDL_CreateTextureFromSurface( renderer, surface );
    SDL_FreeSurface( surface );

    tilemap->one_way_tile.x = levelConfig->ghostPenTile.x;
    tilemap->one_way_tile.y = levelConfig->ghostPenTile.y - 2;


}

void load_global_texture_atlases_from_config_file( SDL_Renderer *renderer ) {
    char *filename_config = "res/texture_atlases";
    FILE *f;
    f = fopen(filename_config, "r");
    if( f == NULL ) {
        fprintf(stderr, "Error opening file %s\n", filename_config );
    }
    
    char filename_texture_atlas[ MAX_FILENAME_SIZE ];

    char textureName[ 16 ];

    int num_sprites_texture_atlas;

    char current_line[ 256 ];
    
    /**
     * Very specific and not general-purpose at all
    */
    while( fgets( current_line, 256, f ) != NULL ) {
        if( current_line[ 0 ] == '#' ){
            continue;
        }
        
        // zero these out
        memset(filename_texture_atlas, '\0', MAX_FILENAME_SIZE );
        memset(textureName, '\0', 16 );
        num_sprites_texture_atlas = 0;

        int beg_value_idx = 0;
        int line_idx = 0;
        int size_bytes_value = 0;
        
        // texture name
        while( current_line[ line_idx ] != ' ' ) {
            line_idx++;
        }

        size_bytes_value = line_idx;
        memcpy( textureName, current_line + beg_value_idx, size_bytes_value );
        line_idx++;
        beg_value_idx = line_idx;

        // texture filename
        while( current_line[ line_idx ] != ' ' ) {
            line_idx++;
        }

        size_bytes_value = line_idx - beg_value_idx;
        memcpy( filename_texture_atlas, current_line + beg_value_idx, size_bytes_value );

        beg_value_idx = line_idx + 1;
        line_idx = beg_value_idx;

        // NUMERIC VALUES
        // num rows
        // num cols
        int numeric_values[ 2 ];
        for( int i = 0; i < 2 ; i++ ) {
            numeric_values[ i ] = strtol( current_line + line_idx , NULL, 10 );
            while( current_line[ line_idx ] != ' ' && current_line[ line_idx ] != '\n') {
                line_idx++;
            }
            line_idx++;
        }

        int num_rows = numeric_values[ 0 ];
        int num_cols = numeric_values[ 1 ];
        addTextureAtlas( renderer, textureName, filename_texture_atlas, num_rows, num_cols );

    }
}

void load_animations_from_config_file( AnimatedSprite **animated_sprites ) { 

    int num_animated_sprites = 0;

    char *filename_config = "res/animated_sprites";
    FILE *f;
    f = fopen(filename_config, "r");
    if( f == NULL ) {
        fprintf(stderr, "Error opening file %s\n", filename_config );
    }
    
    char current_line[ 256 ];

    /**
     * Very specific and not general-purpose at all
    */
    while( fgets( current_line, 256, f ) != NULL ) {
        if( current_line[ 0 ] == '#') { // COMMENTS
            continue;
        }
        // zero these out
        int texture_atlas_id = -1;
        int number_cols = -1;
        int number_rows = -1;
        int fps = -1;

        int values[ 4 ];

        int line_idx = 0;
        
        for( int i = 0; i < 4; i++ ) {
            //char *end_ptr;
            values[ i ] = strtol( current_line + line_idx , NULL, 10 );

            while( current_line[ line_idx ] != ' ' && current_line[ line_idx ] != '\n') {
                line_idx++;
            }
            line_idx++;
        }
        
        texture_atlas_id = values[ 0 ];
        fps = values[ 1 ];
        number_rows = values[ 2 ];
        number_cols = values[ 3 ];

        animated_sprites[ num_animated_sprites ] = init_animation( texture_atlas_id, fps, number_rows, number_cols );
        num_animated_sprites++;
        //assert( *end_ptr == '\n' || *end_ptr == ' ' || *end_ptr == '\0');

    }
}

void load_ghost_mode_times_from_config_file( uint8_t *ghost_mode_times, int num_periods, char *filename_config ) {
    //char *filename_config = "res/ghost_mode_times";
    FILE *f;
    f = fopen( filename_config, "r");
    if( f == NULL ) {
        fprintf(stderr, "Error opening file %s\n", filename_config );
    }

    for( int i = 0; i < num_periods; i++ ) {
        ghost_mode_times[ i ] = 0;
    }

    char current_line[ 256 ];
    memset( current_line, '\0', 256 );
    while( fgets( current_line, 256, f ) != NULL ) {
        if( current_line[ 0 ] == '#') {
            continue;
        }

        //int line_idx = 0;
        char *end_ptr = NULL;
        char *val_ptr = current_line;
        for( int i = 0; i< num_periods; i++ ) {
            ghost_mode_times[ i ] = strtol( val_ptr, &end_ptr, 10 );
            if( end_ptr == NULL ) {
                break;
            }
            val_ptr = end_ptr + 1;
            // while( line[ line_idx ] != ' ' && line[ line_idx ] != '\n' ) {
            //     line_idx++;
            // }
        }
        
    }
    

}

void tryLoadPickupsFromConfigFile( LevelConfig *levelConfig, const char *fullResourcePath ) {
    FILE *f;
    f = fopen(fullResourcePath, "r");
    if( f == NULL ) {
        fprintf(stderr, "Error opening file: %s\n", fullResourcePath );
    }

    char currentLine[ 256 ];
    memset( currentLine, '\0', 256 );

    unsigned int lineIdx = 0;
    unsigned int pickupIdx = 0;

    while( fgets( currentLine, 256, f ) != NULL ) {
        if( currentLine[ 0 ] == '#' ) {
            continue;
        }

        lineIdx = 0;
        // numeric values
        unsigned int values[ 4 ];// textureAtlasId, numDots, ActiveTime, ScoreReward
        unsigned int beginningIdx = lineIdx;
        
        for( int i = 0; i < 4; i++ ) {
            beginningIdx = lineIdx;

            // find end
            while( currentLine[ lineIdx ] != ' ') {
                lineIdx++;
            }

            values[ i ] = strtol( currentLine + beginningIdx, NULL, 10 );

            // go to start of next value
            while( currentLine[ lineIdx ] == ' ' ) {
                lineIdx++;
            }
        }

        levelConfig->pickupConfigs[ pickupIdx ].textureAtlasId = values[ 0 ];
        levelConfig->pickupConfigs[ pickupIdx ].numDots = values[ 1 ];
        levelConfig->pickupConfigs[ pickupIdx ].activeTime = values[ 2 ];
        levelConfig->pickupConfigs[ pickupIdx ].scoreReward = values[ 3 ];

        // string values
        // -> map to enums
        const char *FRUIT_PICKUP_STR = "FRUIT_PICKUP";
        const char *MIRROR_PICKUP_STR = "MIRROR_PICKUP";
        const char *SPEED_BOOST_PICKUP_STR = "SPEED_BOOST_PICKUP";
        const char *POWER_PELLET_PICKUP_STR = "POWER_PELLET_PICKUP";
        const char *STOP_GHOSTS_PICKUP_STR = "STOP_GHOSTS_PICKUP";
        const char *SHIELD_PICKUP_STR = "SHIELD_PICKUP";


        beginningIdx = lineIdx;

        while( currentLine[ lineIdx ] != ' ' && currentLine[ lineIdx ] != '\n' && currentLine[ lineIdx ] != '\0') {
            lineIdx++;
        }
        //lineIdx++;
        
        char pickupTypeStr[ 32 ] = {'\0'};
        size_t size = lineIdx - beginningIdx;
        strncpy( pickupTypeStr, currentLine + beginningIdx, size );
        
        if( strncmp(pickupTypeStr, FRUIT_PICKUP_STR, size ) == 0 ) {
            levelConfig->pickupConfigs[ pickupIdx ].pickupType = FRUIT_PICKUP;
        }
        else if( strncmp(pickupTypeStr, MIRROR_PICKUP_STR, size ) == 0 ) {
            levelConfig->pickupConfigs[ pickupIdx ].pickupType = MIRROR_PICKUP;
        }
        else if( strncmp( pickupTypeStr, SPEED_BOOST_PICKUP_STR, size ) == 0 ){
            levelConfig->pickupConfigs[ pickupIdx ].pickupType = SPEED_BOOST_PICKUP;
        }
        else if( strncmp( pickupTypeStr, POWER_PELLET_PICKUP_STR, size ) == 0 ){
            levelConfig->pickupConfigs[ pickupIdx ].pickupType = POWER_PELLET_PICKUP;
        }
        else if( strncmp( pickupTypeStr, SHIELD_PICKUP_STR, size ) == 0 ){
            levelConfig->pickupConfigs[ pickupIdx ].pickupType = SHIELD_PICKUP;
        }
        else if( strncmp( pickupTypeStr, STOP_GHOSTS_PICKUP_STR, size ) == 0 ){
            levelConfig->pickupConfigs[ pickupIdx ].pickupType = STOP_GHOSTS_PICKUP;
        }

        // new line? new pickup
        pickupIdx++;
    }
    levelConfig->numPickupConfigs = pickupIdx;
}

// void load_render_xx_from_config_file( RenderData **renderDatas ) {

//     int num_render_clips = 0;

//     char *filename_config = "res/render_anims";
//     FILE *f;
//     f = fopen(filename_config, "r");
//     if( f == NULL ) {
//         fprintf(stderr, "Error opening file %s\n", filename_config );
//     }
    
//     char current_line[ 256 ];

//     /**
//      * Very specific and not general-purpose at all
//     */
//     while( fgets( current_line, 256, f ) != NULL ) {
//         if( current_line[ 0 ] == '#') { // COMMENT
//             continue;
//         }
//         //int texture_atlas_id = -1;
//         int animation_id = -1;

//         int values[ 2 ];

//         int line_idx = 0;

//         for( int i = 0; i < 1; i++ ) {
//             values[ i ] = strtol( current_line + line_idx , NULL, 10 );

//             while( current_line[ line_idx ] != ' ' && current_line[ line_idx ] != '\n') {
//                 line_idx++;
//             }
//             line_idx++;
//         }

//         //texture_atlas_id = values[ 0 ];
//         animation_id = values[ 0 ];

//         renderDatas[ num_render_clips ] = renderDataInit( 0, animation_id );
//         num_render_clips++;
//     }
// }
/**
 * FILE I/O FOR SAVING/LOADING
 */

void save_resource_to_file( void* resource_ptr, char *filename, size_t resource_size, int num_resources ) {
    const char *write_binary_mode = "wb";
    SDL_RWops *write_context = SDL_RWFromFile( filename , write_binary_mode );
    if( write_context == NULL ) {
        fprintf(stderr, "%s\n", SDL_GetError() );
        exit( EXIT_FAILURE );
    }
    SDL_RWwrite( write_context, resource_ptr, resource_size, num_resources );
    SDL_RWclose( write_context );
}

void try_load_resource_from_file( void *resource_ptr, char *filename, size_t resource_size, int num_resources ) {
    const char *read_binary_mode = "rb";

    SDL_RWops *read_context = SDL_RWFromFile( filename, read_binary_mode );
    // File does not exist
    if( read_context == NULL ) {
        fprintf(stderr, "%s\n", SDL_GetError() );
        return; // we'll just not do anything
    }

    SDL_RWread( read_context, resource_ptr,  resource_size , num_resources );
    SDL_RWclose( read_context );
}

// void try_load_texture_atlases_from_config_file( char *filename ) {
//     int count = 0;
//     FILE *f;
//     f = fopen(filename, "r");
//     if( f == NULL ) {
//         fprintf(stderr, "Error opening file %s\n", filename );
//     }
//     fgets()
// }