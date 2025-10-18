/**************************************************************************
 *    PNG Loading, Coordinate Based Worldmap System for ROM MUDs, v1.1    *
 * ---------------------------------------------------------------------- *
 * This code has been released as both a standalone snippet and as part   *
 * of the StockMUD+ [ROM] project release.  It was written and developed  *
 * by Joseph Benfield (Hades_Kane / Diablos of End of Time & StockMUD)    *
 * and Matt Fillinger (Grieffels of End of Time & When Worlds Collide)    *
 * for use in When Worlds Collide and StockMUD+.  Thanks go out to those  *
 * who came before and inspired the system to begin with, and whose own   *
 * systems we learned from in our effort to simplify and optimize this    *
 * from-scratch system.  LodePNG was used in place of the libpng library. *
 * ---------------------------------------------------------------------- *
 *         PNG encoding/decoding courtesy of the LodePNG library,         *
 *                Copyright (c) 2005-2024 Lode Vandevenne.                *
 * ---------------------------------------------------------------------- *
 * Conditions of use are that applicable licenses in any codebase this is *
 * being used in are followed, this header is not removed or altered, the *
 * headers for LodePNG are not removed if being used, and that some form  *
 * of credit or acknowledgement is made in a helpfile for (your choice):  *
 * the worldmap system, snippets used / code contributors, or credits.    *
 * ---------------------------------------------------------------------- *
 Worldmap by Joseph Benfield (Hades_Kane/Diablos of End of Time & StockMUD)
    and Matt Fillinger (Grieffels of End of Time & When Worlds Collide)
 **************************************************************************/

#include <stdint.h>

/**********CONFIGURATION DEFINES**********/
//configure defines, comment or uncomment to your preferences
#define WMAP_SECT_MOBS //map mobs will remain in same sector type
#define WMAP_SECT_MOVE //sector based movement speed
#define WMAP_SECT_VIS //sector based visbility distance, affects map radius and scan
#define WMAP_SECT_HIDE //sector based visbility distance, affects seeing other's map icons, and scan
#define WMAP_WTHR_VIS //weather based visbility distance, affects map radius and scan
#define WMAP_NIGHT_VIS //day/night based visbility distance, affects map radius and scan
#define WMAP_MOVE_PTS //movement points are considered on worldmap
//#define WMAP_SHOW_OBJS //objects will show on world map
#define WMAP_NIGHT_FADE16 //colors fade on world map, only enable if NOT using 256 colors
//#define WMAP_NIGHT_FADE256 //colors fade on world map, only enable if using 256 colors

//note that these options will slow down how quick the map draws, specially SECT_DEPTH256:
//#define WMAP_WATER_DEPTH16 //water sectors will have color depth, only enable if NOT using 256 colors
//#define WMAP_WATER_DEPTH256 //water sectors will have color depth, only enable if using 256 colors
//#define WMAP_SECT_DEPTH256 //non water sectors will have color depth, only enable if using 256 colors
#define WMAP_LIGHTS16 //lights will brighten map at night
//#define WMAP_LIGHTS256 //lights will brighten map at night
#define WMAP_OBJ_LIGHTS //glowing obj lights will brighten map at night, must have WMAP_LIGHTS defined
/**********CONFIGURATION DEFINES**********/


/***********MUD SPECIFIC VALUES***********/
#define WMAP_ONE                30000
#define WMAP_TWO                30001
#define WMAP_THREE              30002
#define WMAP_FOUR               30003
#define WMAP_FIVE               30004
#define MAX_WMAP                5

#define MAX_WMAP_SCAN            9

#define STAFF_DISPLAY           20
/***********MUD SPECIFIC VALUES***********/

#define WMAP_NONE               -1
#define WMAP_ANY                0

#define RESET_TYPE_MOB          1
#define RESET_TYPE_OBJ          2
#define RESET_TYPE_OBJ_ON_MOB   3

typedef struct    wmap_type        WMAP_TYPE;
typedef struct    wmap_tile        WMAPTILE_DATA;
typedef struct    wmap_exit        WMAP_EXIT;
typedef struct    wmap_reset       WMAP_RESET_DATA;

extern WMAPTILE_DATA       *first_wmaptile;
extern WMAPTILE_DATA       *last_wmaptile;
extern WMAP_EXIT           *first_wmapexit;
extern WMAP_EXIT           *last_wmapexit;
extern WMAP_TYPE           wmap_table[MAX_WMAP];

struct wmap_tile
{
    WMAPTILE_DATA *next;
    WMAPTILE_DATA *prev;
    char *symb;
    char *name;
    char *desc;
    int wmap_index;
    int x;
    int y;
    int z;
    int vis;
    int terrain;
    int pass; //0 yes, 1 no
};

struct wmap_type
{
    char *name;
    char *filename;
    int vnum;
    int max_x;
    int max_y;
    int display_size;
    bool wrap_wmap;
    uint8_t **grid;
};

struct wmap_exit
{ 
    WMAP_EXIT *next;
    WMAP_EXIT *prev;
    int wmap_index;  // The map this exit belongs to
    int x;
    int y;
    int z;
    int room_vnum;  // Destination room vnum in the area
};

struct wmap_reset
{
    WMAP_RESET_DATA *next;
    int reset_type;
    int vnum;
    int wmap_index;
    int x;
    int y;
    int z;
    int max;
    int mob_vnum;
    bool wear;
};

void load_wmap args ((int wmap_index));
void load_png args((int wmap_index));
bool can_leave args((CHAR_DATA *ch, ROOM_INDEX_DATA *in_room, bool wmap));
bool can_enter args((CHAR_DATA *ch, ROOM_INDEX_DATA *to_room, bool wmap, int new_x, int new_y));
void room_movement args((CHAR_DATA * ch, int door, EXIT_DATA *pexit, ROOM_INDEX_DATA *to_room, bool follow));
bool can_move_points args((CHAR_DATA *ch, int move1, int move2));
bool is_passable args((CHAR_DATA *ch, ROOM_INDEX_DATA *room, int sector));
void wmap_movement args((CHAR_DATA *ch, int dir, bool follow));
WMAP_TYPE* get_wmap_ch args((CHAR_DATA* ch, int* wmap_index));
int save_image args((const char *filename, int wmap_index));
void load_wmap_exits args((void));
void load_wmap_tiles args((void));
void save_wmap_exits args((void));
void save_wmap(int wmap_index);
void save_wmap_tiles args((int wmap_index));
void save_wmap_resets args((int wmap_index));
void load_wmap_resets args((int wmap_index));
void add_wmap_exit(int wmap_index, int x, int y, int z, int room_vnum);
void show_wmap_scale(CHAR_DATA *ch, int width, int height);
WMAPTILE_DATA *find_wmap_tile(int wmap_index, int x, int y, int z);
WMAP_EXIT *find_wmap_exit(int wmap_index, int x, int y, int z);
double wmap_dist args((int x1, int y1, int x2, int y2));
char *strip_newlines args((const char *string));
int wmap_new_x args((CHAR_DATA *ch, int dir));
int wmap_new_y args((CHAR_DATA *ch, int dir));
void do_savemap args((CHAR_DATA *ch, char *argument));
