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
 
#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include "merc.h"
#include "tables.h"
#include "interp.h"
#include "lodepng.h"
#include "olc.h"
#include "worldmap.h"

#define WMAP_EXIT_FILE WMAP_DIR "wmap_exits.dat"
#define WMAP_RESET_FILE WMAP_DIR "%s_resets.dat"

const char *wmap_symbol (CHAR_DATA *ch, int wmap_index, int x, int y, int dx, int dy);
int wrap_coordinate(int coord, int max_value);

extern char *strip_recolor args((const char *string));
extern char *fade_color args((const char *string, int fade));
extern char *fade_color16 args((const char *string));
extern char *bright_color args((const char *string, int fade));
extern char *bright_color16 args((const char *string));
extern ROOM_INDEX_DATA *find_location args ((CHAR_DATA * ch, char *arg));
extern int sector_lookup args( (const char *name) );

WMAPTILE_DATA       *first_wmaptile;
WMAPTILE_DATA       *last_wmaptile;
WMAP_EXIT           *first_wmapexit;
WMAP_EXIT           *last_wmapexit;
WMAP_RESET_DATA     *wmap_reset_list[MAX_WMAP] = {NULL};

WMAP_TYPE wmap_table[MAX_WMAP] = {
//   MAP NAME       MAP PNG         MAP VNUM        X       Y       SIZE    WRAPPING
    {"mapone",      "mapone.png",   WMAP_ONE,       500,    500,    16,     FALSE    },
    {"maptwo",      "maptwo.png",   WMAP_TWO,       500,    500,    16,     TRUE     },
    {"mapthree",    "mapthree.png", WMAP_THREE,     500,    500,    16,     TRUE     }
};

const struct sec_type sector_flags[SECT_MAX] = {
    //name                screenread desc     bit                 mv  r     g     b     3x3wal  3x3flr  mapsymb passmsg
    {"inside",            "interior region",  SECT_INSIDE,        1,  1,    1,    1,    "{y+",  "{w.",  " ",    NULL},
    {"city",              "city",             SECT_CITY,          2,  255,  128,  64,   "{y+",  "{D.",  "{W#",  NULL},
    {"field",             "grassy field",     SECT_FIELD,         2,  0,    132,  0,    "{g+",  "{G:",  "{G\"", NULL},
    {"forest",            "forest",           SECT_FOREST,        3,  0,    99,   0,    "{g@",  "{G.",  "{g@",  NULL},
    {"hills",             "hills",            SECT_HILLS,         4,  107,  140,  33,   "{g+",  "{G^",  "{G^",  NULL},
    {"mountain",          "mountains",        SECT_MOUNTAIN,      6,  140,  66,   16,   "{D^",  "{y:",  "{y^",  NULL},
    {"swim",              "shallow water",    SECT_WATER_SWIM,    4,  0,    175,  255,  "{c~",  "{C~",  "{C~",  NULL},
    {"noswim",            "deep water",       SECT_WATER_NOSWIM,  1,  0,    0,    255,  "{b~",  "{B~",  "{B~",  "The water is too deep to swim in."},
    {"unused",            "uncharted terrain",SECT_UNUSED,        6,  10,   10,   10,   "{y+",  "{D.",  "{DX",  "Invalid exit."},
    {"air",               "open air",         SECT_AIR,           10, 5,    5,    5,    "{w+",  "{w.",  "{C%",  "You can't fly."},
    {"desert",            "desert",           SECT_DESERT,        6,  255,  239,  156,  "{y+",  "{Y.",  "{Y=",  NULL},
    {"rock_mountain",     "rocky mountains",  SECT_ROCK_MOUNTAIN, 7,  173,  173,  173,  "{D^",  "{y:",  "{D^",  "The mountains are too steep to climb."},
    {"snow_mountain",     "snowy mountains",  SECT_SNOW_MOUNTAIN, 8,  230,  230,  230,  "{w^",  "{w:",  "{W^",  "The mountains are too steep to climb."},
    {"beach",             "beach",            SECT_BEACH,         3,  214,  206,  16,   "{y+",  "{Y.",  "{Y=",  NULL},
    {"deep_water",        "deep water",       SECT_DEEP_WATER,    1,  0,    0,    125,  "{b~",  "{b~",  "{b~",  "The water is too deep to swim in."},
    {"rough_water",       "rough deep water", SECT_ROUGH_WATER,   1,  0,    0,    100,  "{b~",  "{b~",  "{b~",  "The water is too deep to swim in."},
    {"trail",             "dirt trail",       SECT_TRAIL,         2,  100,  80,   0,    "{D+",  "{y.",  "{y+",  NULL},
    {"road",              "paved road",       SECT_ROAD,          1,  45,   60,   80,   "{D+",  "{c.",  "{c+",  NULL},
    {"swamp",             "swamp",            SECT_SWAMP,         3,  82,   107,  49,   "{D&",  "{G.",  "{D&",  NULL},
};

//command to display sectors and their associated values according tot he sector table
void do_show_sectors(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    buf[0] = '\0';

    send_to_char("{DSector Name          Symbol{x\r\n", ch);
    
    for (int sector = 0; sector < SECT_MAX; sector++)
    {
        sprintf(buf, "{x  %-20s %s{x\r\n", sector_flags[sector].name, sector_flags[sector].wmap_symb);
        send_to_char(buf, ch);
    }
}

//saves the map tiles and PNG of current map
void do_savemap(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    int wmap_index = -1;
    WMAP_TYPE* current_wmap = get_wmap_ch(ch, &wmap_index);

    save_wmap_tiles(wmap_index);

    if (save_image(current_wmap->filename, wmap_index)) 
    {
        sprintf(buf,"The map '%s' has been saved!\r\n", current_wmap->name);
        send_to_char(buf,ch);
    }
    else
        send_to_char("Failed to save the map.\r\n",ch);
    return;
}

//lists the current maps as per the map table
void do_wmaptable(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
        return;
    
    send_to_char("{WIndex {D| {WName       {D| {WFilename   {D| {WVnum    {D| {WX    {D| {WY    {D| {WSize {D| {WWrap{x\r\n", ch);
    send_to_char("{D-----------------------------------------------------------------------\r\n", ch);

    for (int i = 0; i < MAX_WMAP; i++)
    {
        snprintf(buf, sizeof(buf), "{w%-5d {D| {w%-10s {D| {w%-10s {D| {w%-7d {D| {w%-4d {D| {w%-4d {D| {w%-4d {D| {w%-5s{x\n",
                 i,
                 wmap_table[i].name, wmap_table[i].filename, wmap_table[i].vnum, 
                 wmap_table[i].max_x, wmap_table[i].max_y,
                 wmap_table[i].display_size, wmap_table[i].wrap_wmap ? "TRUE" : "FALSE");

        send_to_char(buf, ch);
    }
}

//prints a view of the map according to supplied parameters
void do_wmap_show(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    int width, height;
    int x = ch->wmap[1];
    int y = ch->wmap[2];

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0' || arg2[0] == '\0')
    {
        send_to_char("Syntax: wmapshow <width> <height>\r\n", ch);
        return;
    }

    width = atoi(arg1);
    height = atoi(arg2);

    if (width < 1 || height < 1)
    {
        send_to_char("Invalid map dimensions.\r\n", ch);
        return;
    }

    int active_wmap_width = wmap_table[ch->wmap[0]].max_x;
    int active_wmap_height = wmap_table[ch->wmap[0]].max_y;

    bool is_wrapping_enabled = wmap_table[ch->wmap[0]].wrap_wmap;
    int row_count = 0;

    buf[0] = '\0';  


    for (int dy = -height / 2; dy <= height / 2; dy++)
    {
        int row_width = 0;
        char line[MAX_STRING_LENGTH];  
        line[0] = '\0';  

    
        for (int dx = -width / 2; dx <= width / 2; dx++)
        {
            int wrapped_x = x + dx;
            int wrapped_y = y + dy;
            bool out_of_bounds = false;


            if (is_wrapping_enabled)
            {
                wrapped_x = wrap_coordinate(wrapped_x, active_wmap_width);
                wrapped_y = wrap_coordinate(wrapped_y, active_wmap_height);
            }
            else
            {
                if (wrapped_x < 0 || wrapped_x >= active_wmap_width || wrapped_y < 0 || wrapped_y >= active_wmap_height)
                    out_of_bounds = true;
                else
                {
                
                    wrapped_x = (wrapped_x < 0) ? 0 : (wrapped_x >= active_wmap_width ? active_wmap_width - 1 : wrapped_x);
                    wrapped_y = (wrapped_y < 0) ? 0 : (wrapped_y >= active_wmap_height ? active_wmap_height - 1 : wrapped_y);
                }
            }

            if (out_of_bounds)
                strcat(line, " ");  
            else
                strcat(line, wmap_symbol(ch, ch->wmap[0], wrapped_x, wrapped_y, dx, dy));

            row_width++;
        }

        strcat(buf, line);
        strcat(buf, "\r\n"); 

        row_count++;

        if (row_count % 5 == 0)
        {
            send_to_char(strip_recolor(buf), ch);  
            buf[0] = '\0';  
        }
    }
 
    if (buf[0] != '\0')
        send_to_char(strip_recolor(buf), ch);
}

//string copying utility
char * stpcpy (char *dst, const char *src)
{
  const size_t len = strlen (src);
  return (char *) memcpy (dst, src, len + 1) + len;
}

//pulls the vnum of the wmap of a character or object
int wmap_vnum(CHAR_DATA *ch, OBJ_DATA *obj)
{
    if(ch == NULL && obj == NULL)
    {
        bug("wmap_vnum: ch & obj NULL",0);
        return FALSE;
    }

    if(ch && is_wmap_vnum(ch->in_room->vnum))
        return ch->in_room->vnum;

    if(obj)
    {
        ROOM_INDEX_DATA *room = NULL;

        if(obj->in_room != NULL)
            room = obj->in_room;
        else if(obj->carried_by != NULL)
            room = obj->carried_by->in_room;
        else if(obj->in_obj != NULL)
        {
            if(obj->in_obj->in_room != NULL)
                room = obj->in_obj->in_room;
            else if(obj->in_obj->carried_by != NULL)
                room = obj->in_obj->carried_by->in_room;
        }

        if (obj && room != NULL
        && is_wmap_vnum(room->vnum))
            return room->vnum;
    }

    return -1;
}

//get character's or object's X value, more intuitive than keeping track of what the wmap[] values represent
int wmap_x(CHAR_DATA *ch, OBJ_DATA *obj)
{
    if(ch == NULL && obj == NULL)
    {
        bug("wmap_x: ch & obj NULL",0);
        return FALSE;
    }

    if(obj && wmap_vnum(NULL,obj) == WMAP_NONE)
        return -1;

    if(ch && ch->wmap[1] >= 0)
        return ch->wmap[1];

    if(obj)
    {
        if(obj->in_room != NULL)
            return obj->wmap[1];
        else if(obj->carried_by != NULL)
            return obj->carried_by->wmap[1];
        else if(obj->in_obj != NULL)
        {
            if(obj->in_obj->in_room != NULL)
                return obj->in_obj->wmap[1];
            else if(obj->in_obj->carried_by != NULL)
                return obj->in_obj->carried_by->wmap[1];
        }
    }

    return -1;
}

//get character's or object's Y value, more intuitive than keeping track of what the wmap[] values represent
int wmap_y(CHAR_DATA *ch, OBJ_DATA *obj)
{
    if(ch == NULL && obj == NULL)
    {
        bug("wmap_y: ch & obj NULL",0);
        return FALSE;
    }

    if(obj && wmap_vnum(NULL,obj) == WMAP_NONE)
        return -1;

    if(ch && ch->wmap[2] >= 0)
        return ch->wmap[2];

    if(obj)
    {
        if(obj->in_room != NULL)
            return obj->wmap[2];
        else if(obj->carried_by != NULL)
            return obj->carried_by->wmap[2];
        else if(obj->in_obj != NULL)
        {
            if(obj->in_obj->in_room != NULL)
                return obj->in_obj->wmap[2];
            else if(obj->in_obj->carried_by != NULL)
                return obj->in_obj->carried_by->wmap[2];
        }
    }

    return -1;
}

//get character's or object's Z value, more intuitive than keeping track of what the wmap[] values represent
int wmap_z(CHAR_DATA *ch, OBJ_DATA *obj)
{
    if(ch == NULL && obj == NULL)
    {
        bug("wmap_z: ch & obj NULL",0);
        return FALSE;
    }

    if(ch && ch->wmap[3] >= 0)
        return ch->wmap[3];

    if(obj)
    {
        if(obj->in_room != NULL)
            return obj->wmap[3];
        else if(obj->carried_by != NULL)
            return obj->carried_by->wmap[3];
        else if(obj->in_obj != NULL)
        {
            if(obj->in_obj->in_room != NULL)
                return obj->in_obj->wmap[3];
            else if(obj->in_obj->carried_by != NULL)
                return obj->in_obj->carried_by->wmap[3];
        }
    }

    return -1;
}

//get a character's or object's world map number, more intuitive than keeping track of what the wmap[] values represent
int wmap_num(CHAR_DATA *ch, OBJ_DATA *obj)
{
    if(ch)
        return (ch->wmap[0] >= 0 && ch->wmap[0] < MAX_WMAP) ? ch->wmap[0] : -1;
    if (obj)
    {
        if (obj->in_room)
        {
            if(wmap_vnum(NULL,obj) == WMAP_NONE)
                return -1;
            return obj->wmap[0];
        }
        if (obj->carried_by)
        {
            if(wmap_vnum(obj->carried_by,NULL) == WMAP_NONE)
                return -1;
            return obj->carried_by->wmap[0];
        }

        OBJ_DATA *container = obj->in_obj;
        if (container)
        {
            if (container->in_room)
            {
                if(wmap_vnum(NULL,container) == WMAP_NONE)
                    return -1;
                return container->wmap[0];
            }
            if (container->carried_by)
            {
                if(wmap_vnum(container->carried_by,NULL) == WMAP_NONE)
                    return -1;
                return container->carried_by->wmap[0];
            }
        }
    }
    return -1;
}

//get a WMAP value from a vnum
int wmap_vnum_index(int vnum)
{
    for (int i = 0; i < MAX_WMAP; i++)
    {
        if (wmap_table[i].vnum == vnum)
            return i;
    }
    return -1;
}

//returns TRUE if a character or object is currently on a map
bool is_wmap(CHAR_DATA *ch, OBJ_DATA *obj)
{
    if (!ch && !obj)
    {
        bug("is_wmap: ch & obj NULL", 0);
        return FALSE;
    }

    if (ch)
    {
        int vnum = wmap_vnum(ch, NULL);
        int x = wmap_x(ch, NULL);
        int y = wmap_y(ch, NULL);
        int num = wmap_num(ch, NULL);

        if (vnum != WMAP_NONE && x >= 0 && y >= 0)
        {
            if (num == -1)
            {
                bug("is_wmap: ch on map with bad wmap_num", 0);
                return FALSE;
            }
            return TRUE;
        }
    }

    if (obj)
    {
        int vnum = wmap_vnum(NULL, obj);
        int x = wmap_x(NULL, obj);
        int y = wmap_y(NULL, obj);
        int num = wmap_num(NULL, obj);

        if (vnum != WMAP_NONE && x >= 0 && y >= 0)
        {
            if (num == -1)
            {
                bug("is_wmap: obj on map with bad wmap_num", 0);
                return FALSE;
            }
            return TRUE;
        }
    }
    return FALSE;
}

//is the supplied vnum a wmap vnum?
bool is_wmap_vnum(int vnum)
{
    for (int i = 0; i < MAX_WMAP; i++)
        if (vnum == wmap_table[i].vnum)
            return TRUE;
    return FALSE;
}

//returns max X value of the map a character is on
int wmax_x(CHAR_DATA *ch)
{
    int wmap_index = wmap_num(ch,NULL);
    return (wmap_index != -1) ? wmap_table[wmap_index].max_x : 0;
}

//returns max Y value of the map a character is on
int wmax_y(CHAR_DATA *ch)
{
    int wmap_index = wmap_num(ch,NULL);
    return (wmap_index != -1) ? wmap_table[wmap_index].max_y : 0;
}

//name of world map that a character or object is on
char *wmap_name(CHAR_DATA *ch, OBJ_DATA *obj)
{
    int vnum = 0;
    if(ch && obj)
    {
        bug("wmap_name: ch and obj present",0);
        return "NULL";
    }

    if(!ch && !obj)
    {
        bug("wmap_name: ch and obj NULL",0);
        return "NULL";
    }

    if(ch)
    {
        if(!is_wmap(ch,NULL))
            return "NULL";
        vnum = ch->in_room->vnum;
    }
    if(obj)
    {
        if(!is_wmap(NULL,obj) || obj->in_room == NULL)
            return "NULL";
        vnum = obj->in_room->vnum;
    }

    for (int i = 0; i < MAX_WMAP; i++)
    {
        if (wmap_table[i].vnum == vnum)
            return wmap_table[i].name;
    }
    return "NULL";
}

//helper function that handles followers
void follow_me(CHAR_DATA *ch, ROOM_INDEX_DATA *room, int dir, int old_x, int old_y)
{
    CHAR_DATA *fch, *fch_next;

    for (fch = room->people; fch != NULL; fch = fch_next)
    {
        fch_next = fch->next_in_room;

        if(fch == ch) continue;
        if(wmap_x(fch,NULL) != old_x || wmap_y(fch,NULL) != old_y) continue;

        if (fch->master == ch && IS_AFFECTED (fch, AFF_CHARM)
            && fch->position < POS_STANDING)
            do_function (fch, &do_stand, "");

        if (fch->master == ch && fch->position == POS_STANDING)
        {
            //act ("You follow $N.", fch, NULL, ch, TO_CHAR);
            move_char (fch, dir, TRUE);
        }
    }
    return;
}

//returns X value of a wmap tile that is 1 value DIR of character
int wmap_new_x(CHAR_DATA *ch, int dir)
{
    int new_x = wmap_x(ch, NULL);
    WMAP_TYPE* current_wmap = &wmap_table[wmap_num(ch,NULL)];
    bool wrapping = current_wmap && current_wmap->wrap_wmap;

    // Directional movement
    if (dir == DIR_WEST || dir == DIR_NORTHWEST || dir == DIR_SOUTHWEST)
        new_x -= 1;
    else if (dir == DIR_EAST || dir == DIR_NORTHEAST || dir == DIR_SOUTHEAST)
        new_x += 1;

    if (wrapping)
        new_x = (new_x + wmax_x(ch)) % wmax_x(ch);
    else
        new_x = (new_x < 0) ? 0 : (new_x >= wmax_x(ch) ? wmax_x(ch) - 1 : new_x);

    return new_x;
}

//returns Y value of a wmap tile that is 1 value DIR of character
int wmap_new_y(CHAR_DATA *ch, int dir)
{
    int new_y = wmap_y(ch, NULL);
    WMAP_TYPE* current_wmap = &wmap_table[wmap_num(ch,NULL)];
    bool wrapping = current_wmap && current_wmap->wrap_wmap;

    // Directional movement
    if (dir == DIR_NORTH || dir == DIR_NORTHEAST || dir == DIR_NORTHWEST)
        new_y -= 1;
    else if (dir == DIR_SOUTH || dir == DIR_SOUTHEAST || dir == DIR_SOUTHWEST)
        new_y += 1;

    if (wrapping)
        new_y = (new_y + wmax_y(ch)) % wmax_y(ch);
    else
        new_y = (new_y < 0) ? 0 : (new_y >= wmax_y(ch) ? wmax_y(ch) - 1 : new_y);

    return new_y;
}

//handles movement on the world map, called by move_char
void wmap_movement(CHAR_DATA *ch, int dir, bool follow)
{
    char buf[MSL];
    int wmap_index = wmap_num(ch,NULL);

    if (!is_wmap(ch, NULL))
    {
        sprintf(buf, "wmap_movement: character not on map %s", IS_NPC(ch) ? ch->short_descr : ch->name);
        bug(buf, 0);
        return;
    }

    if (wmap_index == -1)
    {
        send_to_char("No map found for this location.\r\n", ch);
        return;
    }

    int new_x = wmap_new_x(ch,dir);
    int new_y = wmap_new_y(ch,dir);

    if(!can_enter(ch, NULL, TRUE, new_x, new_y))
        return;

    if(dir >= DIR_NORTHEAST
    && !can_enter(ch, NULL, TRUE, wmap_x(ch, NULL), new_y) && !can_enter(ch, NULL, TRUE, new_x, wmap_y(ch, NULL)))
        return;

    #if defined (WMAP_MOVE_PTS)
    int sec1 = get_sector(ch,0,0,0);
    int sec2 = get_sector(NULL,wmap_index,new_x,new_y);
    if(!IS_HOLYLIGHT(ch) && !can_move_points(ch, sector_flags[sec1].move, sector_flags[sec2].move))
        return;
    #endif

    #if defined (WMAP_SECT_MOVE)
    int wait = sector_flags[get_sector(ch,0,0,0)].move / 3;
    wait *= PULSE_PER_SECOND;
    WAIT_STATE(ch,UMAX(0,wait));
    #endif

    CHAR_DATA *fch;
    CHAR_DATA *fch_next;
    int old_x = wmap_x(ch, NULL);
    int old_y = wmap_y(ch, NULL);

    for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
    {
        fch_next = fch->next_in_room;
        if(fch == ch) continue;
        if(!same_room(ch,fch,NULL)) continue;
        printf_to_char(fch,"%s leaves %s.\r\n",PERS(ch,fch),dir_name[dir]);
    }

    // Move the character
    set_coords(ch,NULL,new_x,new_y);

    for (fch = ch->in_room->people; fch != NULL; fch = fch_next)
    {
        fch_next = fch->next_in_room;
        if(fch == ch) continue;
        if(!same_room(ch,fch,NULL)) continue;
        printf_to_char(fch,"%s has arrived.\r\n",PERS(ch,fch));
    }

    WMAP_EXIT *me = find_wmap_exit(wmap_index,new_x,new_y,wmap_z(ch,NULL));

    // Display the updated map after moving
    if (me == NULL)
    {
        bool screenread = FALSE;
        if(screenread)
            do_function(ch, &do_look, "screenread");
        else
            do_function(ch, &do_look, "auto");
        follow_me(ch,ch->in_room,dir,old_x,old_y);

        if(!IS_NPC(ch) && ch->desc && ch->desc->editor == ED_WMAP && ch->pcdata->wmap_sec >= 0)
            wmap_table[wmap_index].grid[new_x][new_y] = ch->pcdata->wmap_sec; //worldmap.c fix
    }

    if (me != NULL)
    {
        if(!get_room_index(me->room_vnum))
        {
            bug("wmap_movement: invalid room vnum for wmap exit",0);
            return;
        }
        ROOM_INDEX_DATA *prev_room = ch->in_room;
        char_from_room(ch);
        char_to_room(ch, get_room_index(me->room_vnum));
        if (!IS_AFFECTED (ch, AFF_SNEAK) && ch->invis_level < LEVEL_HERO)
            act ("$n has arrived.", ch, NULL, NULL, TO_ROOM);
        do_function(ch, &do_look, "auto");
        follow_me(ch,prev_room,dir,old_x,old_y);
        return;
    }
}

//helper function to determine if a player can leave the current room/tile
bool can_leave(CHAR_DATA *ch, ROOM_INDEX_DATA *in_room, bool wmap)
{
    if(ch == NULL)
    {
        bug("can_leave: no valid ch",0);
        return FALSE;
    }

    if(in_room == NULL && wmap == FALSE)
    {
        bug("can_leave: in_room can't be NULL and map FALSE",0);
        return FALSE;
    }

    if (IS_AFFECTED (ch, AFF_CHARM)
        && ch->master != NULL && in_room == ch->master->in_room)
    {
        send_to_char ("What?  And leave your beloved master?\r\n", ch);
        return FALSE;
    }

    if(wmap)
    {
        int wmap_index = wmap_num(ch,NULL);

        if (wmap_index == -1)
        {
            send_to_char("No map found for this location.\r\n", ch);
            return FALSE;
        }

        if(IS_HOLYLIGHT(ch))
            return TRUE;

        int sec = get_sector(ch,0,0,0);
        if (!is_passable(ch,NULL,sec))
        {
            printf_to_char(ch,"%s\r\n",sector_flags[sec].pass);
            return FALSE;
        }

        WMAPTILE_DATA *tile = find_wmap_tile(wmap_index,wmap_x(ch,NULL),wmap_y(ch,NULL),wmap_z(ch,NULL));
        if(tile && tile->pass == 1)
        {
            send_to_char("Invalid exit.\r\n",ch);
            return FALSE;
        }
    }
    else
    {
        if(!is_passable(ch,in_room,-1))
        {
            printf_to_char(ch,"%s\r\n",sector_flags[in_room->sector_type].pass);
            return FALSE;
        }
    }
    return TRUE;
}

//helper function to determine if a player can enter the next room/tile
bool can_enter(CHAR_DATA *ch, ROOM_INDEX_DATA *to_room, bool wmap, int new_x, int new_y)
{
    if(ch == NULL)
    {
        bug("can_enter: no valid ch",0);
        return FALSE;
    }

    if(to_room == NULL && wmap == FALSE)
    {
        bug("can_enter: to_room can't be NULL and map FALSE",0);
        return FALSE;
    }

    if(wmap == TRUE && (new_x < 0 || new_y < 0))
    {
        bug("can_enter: new_x / new_y can't be < 0 and map TRUE",0);
        return FALSE;
    }

    if(!wmap)
    {
        if (!is_room_owner (ch, to_room) && room_is_private (to_room))
        {
            send_to_char ("That room is private right now.\r\n", ch);
            return FALSE;
        }

        if (!IS_NPC (ch))
        {
            int iClass, iGuild;

            for (iClass = 0; iClass < MAX_CLASS; iClass++)
            {
                for (iGuild = 0; iGuild < MAX_GUILD; iGuild++)
                {
                    if (iClass != ch->klass[0] //worldmap.c fix
                        && to_room->vnum == class_table[iClass].guild[iGuild])
                    {
                        send_to_char ("You aren't allowed in there.\r\n", ch);
                        return FALSE;
                    }
                }
            }
        }
        if(!is_passable(ch,to_room,-1))
        {
            printf_to_char(ch,"%s\r\n",sector_flags[to_room->sector_type].pass);
            return FALSE;
        }
    }
    else
    {
        int wmap_index = wmap_num(ch,NULL);

        if (wmap_index == -1)
        {
            send_to_char("No map found for this location.\r\n", ch);
            return FALSE;
        }

        if(IS_HOLYLIGHT(ch))
            return TRUE;

        int sec = get_sector(NULL,wmap_index,new_x,new_y);
        if (!is_passable(ch,NULL,sec))
        {
            printf_to_char(ch,"%s\r\n",sector_flags[sec].pass);
            return FALSE;
        }

        WMAPTILE_DATA *tile = find_wmap_tile(wmap_index,new_x,new_y,wmap_z(ch,NULL));
        if(tile && tile->pass == 1)
        {
            send_to_char("Invalid exit.\r\n",ch);
            return FALSE;
        }
    }

    return TRUE;
}

//pulls wmap exit data based on vnum, called by room_movement
WMAP_EXIT *match_exit_vnum(int vnum)
{
    WMAP_EXIT *exit;

    for( exit = first_wmapexit; exit != NULL; exit = exit->next )
    {
        if(!exit->room_vnum || exit->room_vnum < 1) continue;
        if(exit->room_vnum == vnum)
            return exit;
    }

    return NULL;
}

//handles movement in a regular room, called by move_char
void room_movement(CHAR_DATA * ch, int door, EXIT_DATA *pexit, ROOM_INDEX_DATA *to_room, bool follow)
{
    CHAR_DATA *fch;
    CHAR_DATA *fch_next;

    ROOM_INDEX_DATA *in_room;

    if(!IS_NPC(ch) && IS_SET(ch->act, PLR_MODERN) && ch->pcdata->doorbump[0] >= 0 && ch->pcdata->doorbump[0] != door)
    {
        ch->pcdata->doorbump[0] = -1;
        ch->pcdata->doorbump[1] = 0;
    }

    in_room = ch->in_room;

    if(!IS_NPC(ch) && IS_SET(ch->act, PLR_MODERN) && ch->pcdata->doorbump[0] >= 0 && ch->pcdata->doorbump[0] != door)
    {
        ch->pcdata->doorbump[0] = -1;
        ch->pcdata->doorbump[1] = 0;
    }

    if (IS_SET(pexit->exit_info, EX_CLOSED)
            &&  (!IS_AFFECTED(ch, AFF_PASS_DOOR) 
                 || IS_SET(pexit->exit_info,EX_NOPASS))
            &&   !IS_HOLYLIGHT(ch))
    {
        if(!IS_NPC(ch) && IS_SET(ch->act, PLR_MODERN))
        {
            if(!IS_NPC(ch) && ch->pcdata->doorbump[0] == door && ch->pcdata->doorbump[1] > 0)
            {
                if(ch->pcdata->doorbump[1] == 1 && IS_SET(pexit->exit_info, EX_LOCKED))
                {
                    if(has_key( ch, pexit->key))
                    {
                        do_function(ch, &do_unlock, dir_name[door] );
                        ch->pcdata->doorbump[1] = 2;
                        return;
                    }
                }

                if(ch->pcdata->doorbump[1] >= 1 && !IS_SET(pexit->exit_info, EX_LOCKED))
                {
                    do_function(ch, &do_open, dir_name[door] );
                    ch->pcdata->doorbump[0] = -1;
                    ch->pcdata->doorbump[1] = 0;
                    return;
                }
            }

            printf_to_char(ch,"The %s to the %s is closed%s.\r\n",strlen(pexit->keyword) < 1 ? "door" : pexit->keyword,dir_name[door],
                IS_SET(pexit->exit_info, EX_LOCKED) ? " and locked" : "");

            if(IS_SET(pexit->exit_info, EX_LOCKED) && !has_key( ch, pexit->key))
                send_to_char("  {WYou lack the key.{w\r\n",ch);
            else if(IS_SET(pexit->exit_info, EX_LOCKED) &&  has_key( ch, pexit->key))
                send_to_char("  {WYou have the key.{w\r\n",ch);

            if(!IS_NPC(ch) && ch->pcdata->doorbump[1] == 0
            && ((IS_SET(pexit->exit_info, EX_LOCKED) && has_key( ch, pexit->key) )
            || !IS_SET(pexit->exit_info, EX_LOCKED)))
            {
                ch->pcdata->doorbump[0] = door;
                ch->pcdata->doorbump[1] = 1;
            }
            return;
        }
        else
        {
            act ("The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR);
            return;
        }
    }

    if(!can_move_points(ch, sector_flags[in_room->sector_type].move, sector_flags[to_room->sector_type].move))
        return;

    if (!IS_AFFECTED (ch, AFF_SNEAK) && ch->invis_level < LEVEL_HERO)
        act ("$n leaves $T.", ch, NULL, dir_name[door], TO_ROOM);

    if (is_wmap_vnum(to_room->vnum))
    {
        WMAP_EXIT *exit = NULL;

        if((exit = match_exit_vnum(ch->in_room->vnum)) == NULL)
        {
            bug("room_movement: exit to map, null wmap_exit data, room:",ch->in_room->vnum);
            return;
        }

        char_from_room (ch);

        set_wmap(ch,NULL,exit->wmap_index);
        set_coords(ch, NULL,
            door == DIR_WEST  ? exit->x-1 : door == DIR_EAST  ? exit->x+1 : exit->x,
            door == DIR_NORTH ? exit->y-1 : door == DIR_SOUTH ? exit->y+1 : exit->y);
        set_z(ch,NULL,exit->z);

        //moved this lower for the auto wedit/redit switch 3/31/2025
        char_to_room (ch, to_room);
    }
    else
    {
        char_from_room (ch);
        char_to_room (ch, to_room);
    }

    if (!IS_AFFECTED (ch, AFF_SNEAK) && ch->invis_level < LEVEL_HERO)
        act ("$n has arrived.", ch, NULL, NULL, TO_ROOM);

    do_function (ch, &do_look, "auto");

    if (in_room == to_room)        // no circular follows
        return;

    for (fch = in_room->people; fch != NULL; fch = fch_next)
    {
        fch_next = fch->next_in_room;

        if (fch->master == ch && IS_AFFECTED (fch, AFF_CHARM)
            && fch->position < POS_STANDING)
            do_function (fch, &do_stand, "");

        if (fch->master == ch && fch->position == POS_STANDING
            && can_see_room (fch, to_room))
        {
            if (IS_SET (ch->in_room->room_flags, ROOM_LAW)
                && (IS_NPC (fch) && IS_SET (fch->act, ACT_AGGRESSIVE)))
            {
                act ("You can't bring $N into the city.",
                     ch, NULL, fch, TO_CHAR);
                act ("You aren't allowed in the city.",
                     fch, NULL, NULL, TO_CHAR);
                continue;
            }

            act ("You follow $N.", fch, NULL, ch, TO_CHAR);
            move_char (fch, door, TRUE);
        }
    }
    return;
}

//helper function for if a character has enough movement points to move from current room, into next, called by wmap_movement and room_movement
bool can_move_points(CHAR_DATA *ch, int move1, int move2)
{
    if(IS_NPC(ch) || IS_HOLYLIGHT(ch))
        return TRUE;

    int move;

    move = move1 + move2;
    move /= 2;                // i.e. the average

    // conditional effects
    if (IS_AFFECTED (ch, AFF_FLYING) || IS_AFFECTED (ch, AFF_HASTE))
        move /= 2;

    if (IS_AFFECTED (ch, AFF_SLOW))
        move *= 2;

    if (ch->move < move)
    {
        send_to_char ("You are too exhausted.\r\n", ch);
        return FALSE;
    }
    
    ch->move -= move;
    return TRUE;
}

//pulls the map data and wmap number for character
WMAP_TYPE* get_wmap_ch(CHAR_DATA* ch, int* wmap_index)
{
    if (!ch || !wmap_index)
        return NULL;

    *wmap_index = ch->wmap[0];

    if (*wmap_index < 0 || *wmap_index >= MAX_WMAP)
        return NULL;

    return &wmap_table[*wmap_index];
}

//helper function to return sector type in a room or tile of a character
int get_sector(CHAR_DATA *ch, int wmap_index, int x, int y)
{
    if(ch && is_wmap(ch, NULL))
        return wmap_table[wmap_num(ch,NULL)].grid[wmap_x(ch,NULL)][wmap_y(ch,NULL)]; //worldmap.c fix
    else if(ch == NULL && wmap_index >= 0 && x >= 0 && y >= 0)
        return wmap_table[wmap_index].grid[x][y]; //worldmap.c fix
    else
        return ch->in_room->sector_type;
}

//helper function for if a sector type is water
bool IS_WATER(int sector)
{
    if(sector == SECT_WATER_SWIM || sector == SECT_WATER_NOSWIM
    || sector == SECT_DEEP_WATER || sector == SECT_ROUGH_WATER)
        return TRUE;

    return FALSE;
}

/*
using number_bits instead of number_range for increased efficiency for this specific function
number_bits(3) < 1   // Equivalent to number_range(1,5) == 1
number_bits(3) < 2   // Equivalent to number_range(1,5) < 3
number_bits(2) == 0  // Equivalent to number_range(1,4) == 1
number_bits(1) + 1   // Equivalent to number_range(1,2)
*/
//note that raw values, such as wmap[], are used here (unlike in other places) to minimize performance issues by going through helper functions
//returns the symbol of a particular coordinate on a map
const char *wmap_symbol (CHAR_DATA *ch, int wmap_index, int x, int y, int dx, int dy)
{
    if (dx == 0 && dy == 0) 
        return "{R*"; // Player position

    CHAR_DATA *och, *och_next;
    #if defined (WMAP_LIGHTS16) || defined (WMAP_LIGHTS256) || defined (WMAP_SHOW_OBJS)
    OBJ_DATA *obj;
    #endif
    #if defined (WMAP_SHOW_OBJS) || defined (WMAP_OBJ_LIGHTS)
    OBJ_DATA *obj_next;
    #if defined (WMAP_SHOW_OBJS)
    bool fObj = FALSE;
    #endif
    #endif
    bool fPlayer = FALSE;
    bool fCombat = FALSE;
    bool found = FALSE;
    int depth = MAX_WMAP_SCAN;
    int mod = 0;
    int dst = 0;
    #if defined (WMAP_SECT_VIS)
    int sec = 0;
    #endif
    #if defined (WMAP_LIGHTS16) || defined (WMAP_LIGHTS256)
    bool maplight = FALSE;
    #if defined (WMAP_LIGHTS256)
    int fade = 0;
    #endif
    #endif

    for (och = ch->in_room->people; och != NULL; och = och_next)
    {
        och_next = och->next_in_room;
        if(och->wmap[0] != wmap_index) continue;
        if(!can_see(ch,och)) continue;

        #if defined (WMAP_LIGHTS16) || defined (WMAP_LIGHTS256)
        if( (x - och->wmap[1]) * (x - och->wmap[1]) + ((y - och->wmap[2]) * (y - och->wmap[2])) * 2 <= 12
        &&  (weather_info.sunlight == SUN_SET || weather_info.sunlight == SUN_DARK)
        &&  (obj = get_eq_char (och, WEAR_LIGHT)) != NULL
        &&  obj->item_type == ITEM_LIGHT && obj->value[2] != 0)
        {
            maplight = TRUE;
            #if defined (WMAP_LIGHTS256)
            if(fade == 0 || (fade > UMIN(abs(x - och->wmap[1]) + abs(y - och->wmap[2]), 4)))
                fade = UMIN(abs(x - och->wmap[1]) + abs(y - och->wmap[2]), 4);
            #endif
        }
        #endif

        if (och != ch && och->wmap[1] == x && och->wmap[2] == y)
        {
            #if defined (WMAP_SECT_VIS)
            if(!IS_HOLYLIGHT(ch))
            {
                if((sec = sector_flags[wmap_table[wmap_index].grid[ch->wmap[1]][ch->wmap[2]]].move) > 2)
                    mod += sec * 100 / 150;
            }
            #endif
            #if defined (WMAP_SECT_HIDE)
            int och_sector = wmap_table[wmap_index].grid[och->wmap[1]][och->wmap[2]]; //worldmap.c fix
            mod = sector_flags[och_sector].move / 2;
            dst = wmap_dist(ch->wmap[1],ch->wmap[2],och->wmap[1],och->wmap[2]);
            #endif
            if(dst > depth - mod && !IS_HOLYLIGHT(ch))
                continue;

            found = TRUE;
            if (!IS_NPC(och))
                fPlayer = TRUE;
            if (och->fighting != NULL)
                fCombat = TRUE;
        }
    }

    #if defined (WMAP_SHOW_OBJS) || defined (WMAP_OBJ_LIGHTS)
    if(!found)
    {
        for (obj = ch->in_room->contents; obj != NULL; obj = obj_next)
        {
            obj_next = obj->next_content;
            if(obj->wmap[0] != wmap_index) continue;
            if(!can_see_obj(ch,obj)) continue;

            #if defined (WMAP_OBJ_LIGHTS)
            #if defined (WMAP_LIGHTS16) || defined (WMAP_LIGHTS256)
            if( (x - obj->wmap[1]) * (x - obj->wmap[1]) + ((y - obj->wmap[2]) * (y - obj->wmap[2])) * 2 <= 12
            &&  (weather_info.sunlight == SUN_SET || weather_info.sunlight == SUN_DARK)
            &&  obj->item_type == ITEM_LIGHT && obj->value[2] != 0 && IS_OBJ_STAT(obj, ITEM_GLOW))
            {
                maplight = TRUE;
                #if defined (WMAP_LIGHTS256)
                if(fade == 0 || (fade > UMIN(abs(x - obj->wmap[1]) + abs(y - obj->wmap[2]), 4)))
                    fade = UMIN(abs(x - obj->wmap[1]) + abs(y - obj->wmap[2]), 4);
                #endif
            }
            #endif
            #endif

            #if defined (WMAP_SHOW_OBJS)
            if (obj->wmap[1] == x && obj->wmap[2] == y)
            {
                #if defined (WMAP_SECT_VIS)
                if(!IS_HOLYLIGHT(ch))
                {
                    if((sec = sector_flags[wmap_table[wmap_index].grid[ch->wmap[1]][ch->wmap[2]]].move) > 2)
                        mod += sec * 100 / 150;
                }
                #endif
                #if defined (WMAP_SECT_HIDE)
                mod = sector_flags[wmap_table[wmap_index].grid[obj->wmap[1]][obj->wmap[2]]].move / 2;
                dst = wmap_dist(ch->wmap[1],ch->wmap[2],obj->wmap[1],obj->wmap[2]);
                #endif
                if(dst > depth - mod && !IS_HOLYLIGHT(ch))
                    continue;

                fObj = TRUE;
                break;
            }
            #endif
        }
    }
    #endif

    if (fCombat)
        return "{M*"; // Combat ongoing
    else if (fPlayer)
        return "{C*"; // Another player is here
    else if (found)
        return "{B*"; // NPC
    #if defined (WMAP_SHOW_OBJS)
    else if (fObj)
        return "{wo";
    #endif

    char *symb = NULL;

    //WMAPTILE_DATA *tile = wmap_table[wmap_index].grid[x][y]; //worldmap.c fix - grid is uint8_t not pointer
    WMAPTILE_DATA *tile2 = find_wmap_tile(wmap_index,x,y,0);

    if (tile2 && tile2->symb && tile2->symb[0] != '\0')
        symb = tile2->symb;
    else
        symb = sector_flags[wmap_table[wmap_index].grid[x][y]].wmap_symb; //worldmap.c fix

    uint8_t terrain = wmap_table[wmap_index].grid[x][y]; //worldmap.c fix
    if (terrain >= 0 && terrain < SECT_MAX && symb)
    {
        #if defined (WMAP_NIGHT_FADE16)
        if(weather_info.sunlight == SUN_SET || weather_info.sunlight == SUN_DARK)
        {
            #if defined (WMAP_WATER_DEPTH16)
            if(IS_WATER(terrain) && number_bits(3) < 1)
                return symb;
            #endif
            if(!maplight)
                return fade_color16(symb);
            #if defined (WMAP_LIGHTS16) || defined (WMAP_LIGHTS256)
            else 
            {
                #if defined (WMAP_LIGHTS256)
                if(fade > 0)
                    return fade_color(symb,fade);
                else
                #endif
                return symb;
            }
            #endif
        }
        #if defined (WMAP_WATER_DEPTH16)
        else
        {
            if(terrain == SECT_WATER_SWIM && number_bits(3) < 2)
                return fade_color16(symb);
        }
        #endif
        #endif

        #if defined (WMAP_NIGHT_FADE256)
        if(weather_info.sunlight == SUN_SET || weather_info.sunlight == SUN_DARK)
        {
            #if defined (WMAP_WATER_DEPTH256)
            if(IS_WATER(terrain) && number_bits(3) < 1)
                return symb;
            #endif
            #if defined (WMAP_SECT_DEPTH256)
            if(!IS_WATER(terrain) && number_bits(2) == 0)
                return fade_color(symb,3);
            #endif
            if(!maplight)
                return fade_color(symb,4);
            #if defined (WMAP_LIGHTS16) || defined (WMAP_LIGHTS256)
            else 
            {
                #if defined (WMAP_LIGHTS256)
                if(fade > 0)
                    return fade_color(symb,fade);
                else
                #endif
                return symb;
            }
            #endif
        }
        #if defined (WMAP_WATER_DEPTH256)
        else
        {
            if(terrain == SECT_WATER_SWIM && number_bits(3) < 2)
                return fade_color(symb,3);
        }
        #endif
        #endif

        #if defined (WMAP_WATER_DEPTH16)
        if(terrain == SECT_WATER_NOSWIM)
        {
            if(number_bits(3) < 2)
                return fade_color16(symb);
            else
                return symb;
        }
        if(IS_WATER(terrain) && number_bits(3) < 1)
            return bright_color16(symb);
        #endif

        #if defined (WMAP_WATER_DEPTH256)
        if(IS_WATER(terrain) && number_bits(3) < 1)
            return bright_color(symb,2);
        #endif

        #if defined (WMAP_SECT_DEPTH256)
        if(!maplight && !IS_WATER(terrain) && number_bits(2) == 0)
            return fade_color(symb,number_bits(1) + 1);
        #endif
        return symb;
    }

    return "{x?"; // Unknown tile
}

//returns new coordinate of a wmap when dealing with wrapping, called by do_wmap_show and draw_wmap
int wrap_coordinate(int coord, int max_value)
{
    return (coord + max_value) % max_value;
}

//helps with the rounded wmap drawing
bool is_within_circle(int dx, int dy, int view_radius)
{
    float aspect_ratio = 1.8f; 
    float scaled_dy = dy * aspect_ratio;
    return (dx * dx + scaled_dy * scaled_dy) <= (view_radius * view_radius);
}

//determines if the character sees the rounded version, or if it fills the maximum space
bool is_full_view(CHAR_DATA *ch)
{
    return IS_HOLYLIGHT(ch);
}

//returns how large the wmap drawing circle should be
int get_view_radius(CHAR_DATA *ch) 
{
    int size = wmap_table[ch->wmap[0]].display_size;
    int mod = 0;

    if (is_full_view(ch)) 
        return STAFF_DISPLAY;

    #if defined (WMAP_SECT_VIS)
    mod += UMAX(0,sector_flags[get_sector(ch,0,0,0)].move - 2);
    #endif

    #if defined (WMAP_WTHR_VIS)
    if(weather_info.sky == SKY_RAINING || weather_info.sky == SKY_LIGHTNING)
        mod += size / 4;
    #endif

    #if defined (WMAP_NIGHT_VIS)
    if(weather_info.sunlight == SUN_SET || weather_info.sunlight == SUN_DARK)
        mod += size / 3;
    #endif

    return UMAX(7,size - mod);
}

//handles number of lines when adding a description to the map, called by add_border_row, draw_wmap
int visible_length(const char *str) 
{
    int length = 0;
    bool in_color = false;

    for (const char *p = str; *p; p++) 
    {
        if (*p == '{')  
            in_color = true;
        else if (in_color)
            in_color = false;  
        else
            length++;  
    }
    return length;
}

//helper function for border around the wmap view
void add_border_row(char *buf, int width, bool is_top, CHAR_DATA *ch)
{
    buf[0] = '\0';

    if (is_top)
    {
        char *buf_ptr = buf;
        buf_ptr += sprintf(buf_ptr, "\r\n{D.");
        
        int dashes_length = width;  
        for (int i = 0; i < dashes_length; i++) 
            *buf_ptr++ = '-';
        
        buf_ptr += sprintf(buf_ptr, ".");

        int wmap_index = ch->wmap[0];
        int x = ch->wmap[1];
        int y = ch->wmap[2];
        int z = ch->wmap[3];

        WMAPTILE_DATA *tile = find_wmap_tile(wmap_index,x,y,z);

        if (tile && tile->name && tile->name[0] != '\0') 
            buf_ptr += sprintf(buf_ptr, "  {x%s", tile->name);  

        buf_ptr += sprintf(buf_ptr, "\r\n");
        *buf_ptr = '\0';
    }
    else
    {
        char coord_buf[MAX_STRING_LENGTH];
        sprintf(coord_buf, "{D[X{w%-4d{D] [Y{w%-4d{D]{D", ch->wmap[1], ch->wmap[2]);

        int text_len = visible_length(coord_buf);
        int total_dashes = width;
        int left_padding = (total_dashes - text_len) / 2;
        int right_padding = total_dashes - text_len - left_padding;

        char *buf_ptr = buf;
        *buf_ptr++ = '{'; *buf_ptr++ = 'D'; *buf_ptr++ = '\''; 

        for (int i = 0; i < left_padding; i++) 
            *buf_ptr++ = '-';

        buf_ptr = stpcpy(buf_ptr, coord_buf);

        for (int i = 0; i < right_padding; i++) 
            *buf_ptr++ = '-';

        *buf_ptr++ = '\''; 
        *buf_ptr++ = '\r'; 
        *buf_ptr++ = '\n'; 
        *buf_ptr++ = '{'; *buf_ptr++ = 'x'; 

        *buf_ptr = '\0'; 
    }
}

//note that raw values, such as wmap[], are used here (unlike in other places) to minimize performance issues by going through helper functions
//the way strings are handled here are also for best performance
//function that handles assembling and displaying the world map view, and any tile based descriptions
void draw_wmap(CHAR_DATA *ch, int wmap_index, int view_radius, bool full_view)
{
    char buf[MAX_STRING_LENGTH];
    int x = ch->wmap[1];
    int y = ch->wmap[2];

    int active_wmap_width = wmap_table[wmap_index].max_x;
    int active_wmap_height = wmap_table[wmap_index].max_y;

    int max_row_width = 0;
    int adjusted_height = (view_radius * 1.8) / 2;
    bool is_wrapping_enabled = wmap_table[wmap_index].wrap_wmap;

    char wmap_rows[view_radius * 2 + 20][MAX_STRING_LENGTH];
    int row_count = 0;

    // Declare desc_lines as a VLA and initialize manually
    char *desc_lines[view_radius * 2 + 20];
    for (int i = 0; i < view_radius * 2 + 20; i++)
        desc_lines[i] = NULL;
    int desc_line_count = 0;

    // Move MAX_LINE_WIDTH up and adjust dynamically
    const int MAX_LINE_WIDTH = 77 - (view_radius * 2 + 2); // Adjust based on map width
    int full_width = MAX_LINE_WIDTH;

    // Fetch tile description and wrap it with color persistence
    WMAPTILE_DATA *tile = find_wmap_tile(wmap_index,x,y,0);

    if (tile && tile->desc && tile->desc[0] != '\0')
    {
        char *desc_copy = strdup(tile->desc);
        char *ptr = desc_copy;
        char line_buf[MAX_STRING_LENGTH];
        char color_buf[10] = ""; // Buffer for current color code (e.g., "{R")
        int line_pos = 0;        // Total chars including color codes
        int visible_pos = 0;     // Visible chars only

        while (*ptr && desc_line_count < view_radius * 2 + 20)
        {
            // Adjust full_width for first line offset
            if (desc_line_count == 0)
                full_width = MAX_LINE_WIDTH - 2;
            else
                full_width = MAX_LINE_WIDTH;

            // Check for color code
            if (*ptr == '{' && *(ptr + 1) != '\0' && (*(ptr - 1) == '\0' || *(ptr - 1) != '{'))
            {
                if(*(ptr + 1) == '{')
                {
                    if (line_pos + 1 < sizeof(line_buf) - 1) // Ensure buffer space
                        line_buf[line_pos++] = *ptr++;
                    else
                        ptr += 1; // Skip color code if no room
                }
                else if(*(ptr + 1) == '[')
                {
                    if (line_pos + 7 < sizeof(line_buf) - 1) // Ensure buffer space
                    {
                        line_buf[line_pos++] = *ptr++;
                        line_buf[line_pos++] = *ptr++;
                        line_buf[line_pos++] = *ptr++;
                        line_buf[line_pos++] = *ptr++;
                        line_buf[line_pos++] = *ptr++;
                        line_buf[line_pos++] = *ptr++;
                        line_buf[line_pos++] = *ptr++;
                        strncpy(color_buf, line_buf + line_pos - 7, 7);
                        color_buf[7] = '\0';
                    }
                    else
                        ptr += 7; // Skip color code if no room
                }
                else
                {
                    if (line_pos + 2 < sizeof(line_buf) - 1) // Ensure buffer space
                    {
                        line_buf[line_pos++] = *ptr++;
                        line_buf[line_pos++] = *ptr++;
                        strncpy(color_buf, line_buf + line_pos - 2, 2);
                        color_buf[2] = '\0';
                        // Don�t increment visible_pos for color codes
                    }
                    else
                        ptr += 2; // Skip color code if no room
                }
                continue;
            }

            // Handle explicit newlines
            if (*ptr == '\r' || *ptr == '\n')
            {
                if (visible_pos < full_width && line_pos < MAX_STRING_LENGTH - 1)
                {
                    line_buf[line_pos++] = ' ';
                    visible_pos++;
                }
                ptr++;
                if (*ptr == '\r' || *ptr == '\n') ptr++;
                continue;
            }

            // Add character to current line
            if (visible_pos < full_width)
            {
                line_buf[line_pos++] = *ptr++;
                visible_pos++; // Only increment for visible chars
            }
            else
            {
                // Find last space to break at
                int break_pos = line_pos - 1;
                while (break_pos >= 0 && line_buf[break_pos] != ' ')
                    break_pos--;
                
                if (break_pos > 0) // Break at space
                {
                    line_buf[break_pos] = '\0';
                    desc_lines[desc_line_count++] = strdup(line_buf);
                    ptr -= (line_pos - break_pos - 1); // Move back to after the space
                    line_pos = 0;
                    visible_pos = 0;
                    if (color_buf[0]) // Carry color forward
                    {
                        strcpy(line_buf, color_buf);
                        line_pos = strlen(color_buf);
                        // visible_pos remains 0
                    }
                }
                else // No space found, hard break
                {
                    line_buf[line_pos] = '\0';
                    desc_lines[desc_line_count++] = strdup(line_buf);
                    line_pos = 0;
                    visible_pos = 0;
                    if (color_buf[0]) // Carry color forward
                    {
                        strcpy(line_buf, color_buf);
                        line_pos = strlen(color_buf);
                        // visible_pos remains 0
                    }
                    ptr++;
                }
            }
        }

        // Add any remaining text as the last line
        if (line_pos > 0 && desc_line_count < view_radius * 2 + 20)
        {
            line_buf[line_pos] = '\0';
            desc_lines[desc_line_count++] = strdup(line_buf);
        }

        free(desc_copy);
    }

    int ht = adjusted_height;
    bool dht = FALSE;
    if(adjusted_height < desc_line_count)
    {
        ht = desc_line_count + 2;
        dht = TRUE;
    }

    for (int dy = -adjusted_height / 2; dy <= ht / 2; dy++)
    {
        char *buf_ptr = buf;
        if(dy <= adjusted_height / 2)
        {
            *buf_ptr++ = '{'; *buf_ptr++ = 'D'; *buf_ptr++ = '|';
        }
        else if(dy > (adjusted_height / 2) + 1)
        {
             *buf_ptr++ = ' '; *buf_ptr++ = ' ';
        }

        int row_width = 0;

        for (int dx = -view_radius; dx <= view_radius; dx++)
        {
            if (!full_view && !is_within_circle(dx, dy, view_radius))
            {
                if(dy <= adjusted_height / 2)
                {
                    *buf_ptr++ = '{';
                    *buf_ptr++ = 'x';
                }
                if(dy != (adjusted_height / 2) + 1)
                    *buf_ptr++ = ' ';  
                continue;
            }

            int wrapped_x = x + dx;
            int wrapped_y = y + dy;
            bool out_of_bounds = false;

            if (is_wrapping_enabled)
            {
                wrapped_x = wrap_coordinate(wrapped_x, active_wmap_width);
                wrapped_y = wrap_coordinate(wrapped_y, active_wmap_height);
            }
            else
            {
                if (wrapped_x < 0 || wrapped_x >= active_wmap_width || wrapped_y < 0 || wrapped_y >= active_wmap_height)
                    out_of_bounds = true;
                else
                {
                    wrapped_x = (wrapped_x < 0) ? 0 : (wrapped_x >= active_wmap_width ? active_wmap_width - 1 : wrapped_x);
                    wrapped_y = (wrapped_y < 0) ? 0 : (wrapped_y >= active_wmap_height ? active_wmap_height - 1 : wrapped_y);
                }
            }

            if (out_of_bounds || (dht && dy > adjusted_height / 2))
            {
                if(dy != (adjusted_height / 2) + 1)
                    *buf_ptr++ = ' ';
            }
            else
            {
                const char *symbol = wmap_symbol(ch, wmap_index, wrapped_x, wrapped_y, dx, dy);
                if (symbol)
                    buf_ptr = stpcpy(buf_ptr, symbol);
                else
                    *buf_ptr++ = ' '; 
            }

            row_width++;
        }

        if(dht && dy == (adjusted_height / 2) + 1)
        {
            char coord_buf[MAX_STRING_LENGTH];
            sprintf(coord_buf, "{D[X{w%-4d{D] [Y{w%-4d{D]{D", ch->wmap[1], ch->wmap[2]);

            int text_len = visible_length(coord_buf);
            int total_dashes = max_row_width;
            int left_padding = (total_dashes - text_len) / 2;
            int right_padding = total_dashes - text_len - left_padding;


            *buf_ptr++ = '{'; *buf_ptr++ = 'D'; *buf_ptr++ = '\''; 

            for (int i = 0; i < left_padding; i++) 
                *buf_ptr++ = '-';

            buf_ptr = stpcpy(buf_ptr, coord_buf);

            for (int i = 0; i < right_padding; i++) 
                *buf_ptr++ = '-';

            *buf_ptr++ = '{'; *buf_ptr++ = 'x'; 
        }


        if(dy <= adjusted_height / 2)
        {
            *buf_ptr++ = '{'; *buf_ptr++ = 'D'; *buf_ptr++ = '|'; *buf_ptr++ = '{'; *buf_ptr++ = 'x';
        }
        if(dy == (adjusted_height / 2) + 1)
        {
            *buf_ptr++ = '{'; *buf_ptr++ = 'D'; *buf_ptr++ = '\''; *buf_ptr++ = '{'; *buf_ptr++ = 'x'; *buf_ptr++ = ' ';
        }
        else
            *buf_ptr++ = ' ';

        // Append description line if available
        if (row_count < desc_line_count)
        {
            int padding = 2; // Space between border and desc
            for (int i = 0; i < padding; i++)
                *buf_ptr++ = ' ';
            buf_ptr += sprintf(buf_ptr, "%s%s", row_count == 0 ? "  " : "", desc_lines[row_count]);
        }

        *buf_ptr++ = '\r'; *buf_ptr++ = '\n';
        *buf_ptr = '\0';  
        strcpy(wmap_rows[row_count++], buf);  

        if (row_width > max_row_width)
            max_row_width = row_width;
    }

    // Display top border
    add_border_row(buf, max_row_width, TRUE, ch);
    send_to_char(buf, ch);

    int tcount = row_count;

    // Display map rows with descriptions
    for (int i = 0; i < tcount; i++)
        send_to_char(strip_recolor(wmap_rows[i]), ch);

    // Display bottom border
    if(!dht)
    {
        add_border_row(buf, max_row_width, FALSE, ch);
        send_to_char(buf, ch);
    }

    for (int i = row_count; i < desc_line_count; i++)
    {
        char desc_buf[MAX_STRING_LENGTH];
        int padding = max_row_width + 5;
        snprintf(desc_buf, sizeof(desc_buf), "%*s%s\r\n", padding, " ", desc_lines[i]);
        send_to_char(desc_buf, ch);
    }
    // Free allocated description lines
    for (int i = 0; i < desc_line_count; i++)
        if (desc_lines[i])
            free(desc_lines[i]);
}

//screen reader version of the world map
void screenread_wmap(CHAR_DATA *ch, bool alook)
{
    int wmap = wmap_num(ch,NULL);
    int x = wmap_x(ch,NULL);
    int y = wmap_y(ch,NULL);
    WMAPTILE_DATA *tile = find_wmap_tile(wmap,x,y,0);

    if(tile && tile->name && tile->name[0] != '\0')
        printf_to_char(ch,"%s X %d Y %d\r\n",tile->name,x,y);
    if (tile && tile->desc && tile->desc[0] != '\0')
    {
        printf_to_char(ch,"  %s\r\n",tile->desc, ch );
        return;
    }

    if(!alook)
    {
        printf_to_char(ch,"X %d Y %d, %s here.\r\n",x,y,sector_flags[get_sector(ch,0,0,0)].desc);
        do_function(ch, &do_scan, "screenread");
    }
    else if(!IS_NPC(ch) && IS_SET(ch->comm, COMM_BRIEF))
    {
        printf_to_char(ch,"%s here.\r\n",sector_flags[get_sector(ch,0,0,0)].desc);
    }
    else
    {
        int no1 = get_sector( NULL, wmap, x,   y-1 );
        int no2 = get_sector( NULL, wmap, x,   y-2 );
        int ne1 = get_sector( NULL, wmap, x-1, y-1 );
        int ne2 = get_sector( NULL, wmap, x-2, y-2 );
        int ea1 = get_sector( NULL, wmap, x+1, y   );
        int ea2 = get_sector( NULL, wmap, x+2, y   );
        int se1 = get_sector( NULL, wmap, x+1, y+1 );
        int se2 = get_sector( NULL, wmap, x+2, y+2 );
        int so1 = get_sector( NULL, wmap, x,   y+1 );
        int so2 = get_sector( NULL, wmap, x,   y+2 );
        int sw1 = get_sector( NULL, wmap, x-1, y-1 );
        int sw2 = get_sector( NULL, wmap, x-2, y-2 );
        int we1 = get_sector( NULL, wmap, x-1, y   );
        int we2 = get_sector( NULL, wmap, x-2, y   );
        int nw1 = get_sector( NULL, wmap, x-1, y-1 );
        int nw2 = get_sector( NULL, wmap, x-2, y-2 );
        char buf[MAX_STRING_LENGTH];

        printf_to_char(ch,"X %d Y %d, Terrain: ",x,y);
        sprintf(buf,"%s here.\r\n",sector_flags[get_sector(ch,0,0,0)].desc);

        for( int i = 0; i < SECT_MAX; i++ )
        {
            if (i == no1 && i == no2 && i == ne1 && i == ne2 && i == ea1 && i == ea2
            &&  i == se1 && i == se2 && i == so1 && i == so2 && i == sw1 && i == sw2
            &&  i == we1 && i == we2 && i == nw1 && i == nw2)
            {
                strcat(buf,sector_flags[i].desc);
                strcat(buf," in all directions.\r\n");
            }
            else if (i == no1 && i == ne1 && i == ea1 && i == se1 && i == so1 && i == sw1 && i == we1 && i == nw1 )
            {
                strcat(buf,sector_flags[i].desc);
                strcat(buf," in all nearby directions");

                if (i == no2 || i == ne2 || i == ea2 || i == se2 || i == so2 || i == sw2 || i == we2 || i == nw2)
                {
                    strcat(buf," and further");

                    if(i == no2)
                        strcat(buf,", north");
                    if(i == ne2)
                        strcat(buf,", northeast");
                    if(i == ea2)
                        strcat(buf,", east");
                    if(i == se2)
                        strcat(buf,", southeast");
                    if(i == so2)
                        strcat(buf,", south");
                    if(i == sw2)
                        strcat(buf,", southwest");
                    if(i == we2)
                        strcat(buf,", west");
                    if(i == nw2)
                        strcat(buf,", northwest");
                    strcat(buf,".\r\n");
                }
                else
                    strcat(buf,".\r\n");
            }
            else if (i == no1 || i == no2 || i == ne1 || i == ne2 || i == ea1 || i == ea2
            ||  i == se1 || i == se2 || i == so1 || i == so2 || i == sw1 || i == sw2
            ||  i == we1 || i == we2 || i == nw1 || i == nw2)
            {
                strcat(buf,sector_flags[i].desc);

                if (i == no1)
                    strcat(buf,", north");
                if(i == ne1)
                    strcat(buf,", northeast");
                if(i == ea1)
                    strcat(buf,", east");
                if(i == se1)
                    strcat(buf,", southeast");
                if(i == so1)
                    strcat(buf,", south");
                if(i == sw1)
                    strcat(buf,", southwest");
                if(i == we1)
                    strcat(buf,", west");
                if(i == nw1)
                    strcat(buf,", northwest");

                if (i == no2 || i == ne2 || i == ea2 || i == se2 || i == so2 || i == sw2 || i == we2 || i == nw2)
                {
                    strcat(buf," and further");

                    if(i == no2 && no1 != no2)
                        strcat(buf,", north");
                    if(i == ne2 && ne1 != ne2)
                        strcat(buf,", northeast");
                    if(i == ea2 && ea1 != ea2)
                        strcat(buf,", east");
                    if(i == se2 && se1 != se2)
                        strcat(buf,", southeast");
                    if(i == so2 && so1 != so2)
                        strcat(buf,", south");
                    if(i == sw2 && sw1 != sw2)
                        strcat(buf,", southwest");
                    if(i == we2 && we1 != we2)
                        strcat(buf,", west");
                    if(i == nw2 && nw1 != nw2)
                        strcat(buf,", northwest");
                    strcat(buf,".\r\n");
                }
                else
                    strcat(buf,".\r\n");
            }
        }
        send_to_char(buf,ch);
        do_function(ch, &do_scan, "screenread");
    }
}

//function that determines which version of the wmap to send to the player, called by do_look
void display_wmap(CHAR_DATA *ch, bool alook)
{
    if (ch->wmap[0] == -1)
    {
        send_to_char("No map found for this location.\r\n", ch);
        return;
    }

    bool screenread = FALSE;
    if(screenread)
    {
        screenread_wmap(ch,alook);
        return;
    }

    bool full_view = is_full_view(ch);
    int view_radius = get_view_radius(ch);
    draw_wmap(ch, ch->wmap[0], view_radius, full_view);
}

//assigns X value to character or object, more intuitive than keeping track of what the wmap[] values represent
void set_x(CHAR_DATA *ch, OBJ_DATA *obj, int value)
{
    if(ch && obj)
    {
        bug("set_x: ch and obj present",0);
        return;
    }

    if(!ch && !obj)
    {
        bug("set_x: ch and obj NULL",0);
        return;
    }

    if(ch)
        ch->wmap[1] = value;
    if(obj)
        obj->wmap[1] = value;
    return;
}

//assigns Y value to character or object, more intuitive than keeping track of what the wmap[] values represent
void set_y(CHAR_DATA *ch, OBJ_DATA *obj, int value)
{
    if(ch && obj)
    {
        bug("set_y: ch and obj present",0);
        return;
    }

    if(!ch && !obj)
    {
        bug("set_y: ch and obj NULL",0);
        return;
    }

    if(ch)
        ch->wmap[2] = value;
    if(obj)
        obj->wmap[2] = value;
    return;
}

//assigns Z value to character or object, more intuitive than keeping track of what the wmap[] values represent
void set_z(CHAR_DATA *ch, OBJ_DATA *obj, int value)
{
    if(ch && obj)
    {
        bug("set_z: ch and obj present",0);
        return;
    }

    if(!ch && !obj)
    {
        bug("set_z: ch and obj NULL",0);
        return;
    }

    if(ch)
        ch->wmap[3] = value;
    if(obj)
        obj->wmap[3] = value;
    return;
}

//assigns world map number value to character or object, more intuitive than keeping track of what the wmap[] values represent
void set_wmap(CHAR_DATA *ch, OBJ_DATA *obj, int value)
{
    if(ch && obj)
    {
        bug("set_wmap: ch and obj present",0);
        return;
    }

    if(!ch && !obj)
    {
        bug("set_wmap: ch and obj NULL",0);
        return;
    }

    if(value > MAX_WMAP)
    {
        bug("set_wmap: value over MAX_WMAP",0);
        return;
    }

    if(ch)
        ch->wmap[0] = value;
    if(obj)
        obj->wmap[0] = value;
    return;
}

//assigns X & Y values to character or object, more intuitive than keeping track of what the wmap[] values represent
void set_coords(CHAR_DATA *ch, OBJ_DATA *obj, int x, int y)
{
    if(ch && obj)
    {
        bug("set_coords: ch and obj present",0);
        return;
    }

    if(!ch && !obj)
    {
        bug("set_coords: ch and obj NULL",0);
        return;
    }

    if(ch)
    {
        set_x(ch,NULL,x);
        set_y(ch,NULL,y);
    }
    if(obj)
    {
        set_x(NULL,obj,x);
        set_y(NULL,obj,y);
    }
    return;
}

//tch/tobj = target, sch/sobj = source
//copies X/Y value from/to character or object
void clone_coords(CHAR_DATA *tch, OBJ_DATA *tobj, CHAR_DATA *sch, OBJ_DATA *sobj)
{
    if((tch && tobj) || (sch && sobj))
    {
        bug("clone_coords: ch and obj present",0);
        return;
    }

    if(tch)
    {
        if(sch)
        {
            if(!is_wmap(sch,NULL))
                return;

            set_x(tch,NULL,wmap_x(sch,NULL));
            set_y(tch,NULL,wmap_y(sch,NULL));
            set_z(tch,NULL,wmap_z(sch,NULL));
            set_wmap(tch,NULL,wmap_num(sch,NULL));
        }
        if(sobj)
        {
            if(!is_wmap(NULL,sobj))
                return;

            set_x(tch,NULL,wmap_x(NULL,sobj));
            set_y(tch,NULL,wmap_y(NULL,sobj));
            set_z(tch,NULL,wmap_z(NULL,sobj));
            set_wmap(tch,NULL,wmap_num(NULL,sobj));
        }
        return;
    }

    if(tobj)
    {
        if(sch)
        {
            if(!is_wmap(sch,NULL))
                return;

            set_x(NULL,tobj,wmap_x(sch,NULL));
            set_y(NULL,tobj,wmap_y(sch,NULL));
            set_z(NULL,tobj,wmap_z(sch,NULL));
            set_wmap(NULL,tobj,wmap_num(sch,NULL));
        }
        if(sobj)
        {
            if(!is_wmap(NULL,sobj))
                return;

            set_x(NULL,tobj,wmap_x(NULL,sobj));
            set_y(NULL,tobj,wmap_y(NULL,sobj));
            set_z(NULL,tobj,wmap_z(NULL,sobj));
            set_wmap(NULL,tobj,wmap_num(NULL,sobj));
        }
        return;
    }
}

//check if character or object is in the same room/tile as another
bool same_room(CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj)
{
    if(ch == NULL)
    {
        bug("same_room: no valid ch",0);
        return FALSE;
    }

    if(ch->in_room == NULL)
    {
        bug("same_room: ch->in_room == NULL",0);
        return FALSE;
    }

    if(victim == NULL && obj == NULL)
    {
        bug("same_room: NULL victim & obj",0);
        return FALSE;
    }

    if(victim)
    {
        if(ch == victim)
            return TRUE;

        if(victim->in_room == NULL)
        {
            bug("same_room: victim->in_room == NULL",0);
            return FALSE;
        }

        if(ch->in_room->vnum != victim->in_room->vnum || wmap_z(ch,NULL) != wmap_z(victim,NULL)
        || wmap_x(ch,NULL) != wmap_x(victim,NULL) || wmap_y(ch,NULL) != wmap_y(victim,NULL))
            return FALSE;

        return TRUE;
    }
    
    if(obj)
    {
        ROOM_INDEX_DATA *room = NULL;

        if(obj->in_room != NULL)
            room = obj->in_room;
        else if(obj->carried_by != NULL)
            room = obj->carried_by->in_room;
        else if(obj->in_obj != NULL)
        {
            if(obj->in_obj->in_room != NULL)
                room = obj->in_obj->in_room;
            else if(obj->in_obj->carried_by != NULL)
                room = obj->in_obj->carried_by->in_room;
        }
        if(room == NULL)
        {
            bug("same_room: obj room == NULL",0);
            return FALSE;
        }
        if(ch->in_room->vnum != room->vnum || wmap_z(ch,NULL) != wmap_z(NULL,obj)
        || wmap_x(ch,NULL) != wmap_x(NULL,obj) || wmap_y(ch,NULL) != wmap_y(NULL,obj))
            return FALSE;

        return TRUE;
    }
    return FALSE;
}

//pulls the tile data from a particular map/coord
WMAPTILE_DATA *find_wmap_tile(int wmap_index, int x, int y, int z)
{
    WMAPTILE_DATA *tile;

    for( tile = first_wmaptile; tile != NULL; tile = tile->next )
        if(tile->wmap_index == wmap_index && tile->x == x && tile->y == y && tile->z == z)
            return tile;

    return NULL;
}

//pulls the exit data from a particular map/coord
WMAP_EXIT *find_wmap_exit(int wmap_index, int x, int y, int z)
{
    WMAP_EXIT *exit;

    for( exit = first_wmapexit; exit != NULL; exit = exit->next )
        if(exit->wmap_index == wmap_index && exit->x == x && exit->y == y && exit->z == z)
            return exit;

    return NULL;
}

//handles adding an exit from the map to an area
void add_wmap_exit(int wmap_index, int x, int y, int z, int room_vnum)
{
    if (x < 0 || x >= wmap_table[wmap_index].max_x || y < 0 || y >= wmap_table[wmap_index].max_y)
    {
        bug("add_wmap_exit: Coordinates out of bounds", 0);
        return;
    }

    WMAP_EXIT *exit = find_wmap_exit(wmap_index,x,y,z);

    CREATE( exit, WMAP_EXIT, 1 );
    LINK( exit, first_wmapexit, last_wmapexit, next, prev );
    exit->wmap_index = wmap_index;
    exit->x = x;
    exit->y = y;
    exit->z = z;
    exit->room_vnum = room_vnum;
}

//saves ALL wmap exits
void save_wmap_exits()
{
    //char buf[MAX_STRING_LENGTH];
    WMAP_EXIT *exit;

    FILE *fp = fopen(WMAP_EXIT_FILE, "w");
    if (!fp)
    {
        perror("save_wmap_exits: Failed to open file");
        bug("save_wmap_exits: Could not open " WMAP_EXIT_FILE " for writing.", 0);
        return;
    }

    fprintf(fp, "#MAPEXIT\n");

    int saved_count = 0;
    for( exit = first_wmapexit; exit != NULL; exit = exit->next )
    {
        fprintf(fp, "%d %d %d %d %d\n", exit->wmap_index, exit->x, exit->y, exit->z, exit->room_vnum);
        saved_count++;

        //sprintf(buf, "Saved exit: MapIndex: %d, X: %d, Y: %d, Z: %d, RoomVnum: %d", exit->wmap_index, exit->x, exit->y, exit->z, exit->room_vnum);
        //bug(buf,0);
    }

    fprintf(fp, "#END\n");
    fflush(fp);  // Ensure buffer writes to disk
    fclose(fp);

    if (saved_count == 0)
        bug("save_wmap_exits: No exits were saved!", 0);
    else
    {
        char buf[MAX_STRING_LENGTH];
        snprintf(buf, sizeof(buf), "save_wmap_exits: Successfully saved %d map exits.", saved_count);
        log_string(buf);
    }
}

//loads ALL wmap exits
void load_wmap_exits()
{
    WMAP_EXIT *exit;
    first_wmapexit = NULL;
    last_wmapexit = NULL;

    log_string ("Loading wmap exits...");

    FILE *fp = fopen(WMAP_EXIT_FILE, "r");
    if (!fp)
    {
        bug("load_wmap_exits: Could not open " WMAP_EXIT_FILE " for reading.", 0);
        return;
    }

    char line[MAX_STRING_LENGTH];
    while (fgets(line, sizeof(line), fp))
    {
        if (line[0] == '#')
            continue; 

        int wmap_index, x, y, z, room_vnum;
        if (sscanf(line, "%d %d %d %d %d", &wmap_index, &x, &y, &z, &room_vnum) == 5)
        {
            if (wmap_index >= 0 && wmap_index < MAX_WMAP
            && x >= 0 && x < wmap_table[wmap_index].max_x && y >= 0 && y < wmap_table[wmap_index].max_y)
            {
                CREATE(exit, WMAP_EXIT, 1);
                exit->wmap_index = wmap_index;
                exit->x = x;
                exit->y = y;
                exit->z = z;
                exit->room_vnum = room_vnum;
                LINK(exit, first_wmapexit, last_wmapexit, next, prev);
            }
        }
    }

    fclose(fp);
}

//saves tile data for specific wmap
void save_wmap_tiles(int wmap_index)
{
    bool has_tiles = FALSE;
    WMAPTILE_DATA *tile;
    if (wmap_index < 0 || wmap_index >= MAX_WMAP)
        return;

    if (!wmap_table[wmap_index].filename || wmap_table[wmap_index].filename[0] == '\0')
        return;

    char filepath[MAX_STRING_LENGTH];

    snprintf(filepath, sizeof(filepath), "%s%s.dat", WMAP_DIR, wmap_table[wmap_index].filename);

    FILE *fp = fopen(filepath, "w");
    if (!fp)
    {
        bugf("save_wmap_tile: Could not open %s for writing.", filepath);
        return;
    }

    for( tile = first_wmaptile; tile != NULL; tile = tile->next )
    {
        if(tile->wmap_index != wmap_index) continue;
        if( (!tile->name || strlen(tile->name) < 1) && (!tile->desc || strlen(tile->desc) < 1) && (!tile->symb || strlen(tile->symb) < 1) ) continue;
        fprintf(fp, "Coords %d %d %d %d %d\n", tile->x, tile->y, tile->z, tile->vis, tile->pass);
        if(tile->symb && strlen(tile->symb) > 0) fprintf(fp, "Symb %s~\n", tile->symb);
        if(tile->name && strlen(tile->name) > 0) fprintf(fp, "Name %s~\n", tile->name);
        if(tile->desc && strlen(tile->desc) > 0) fprintf(fp, "Desc %s~\n", strip_newlines(tile->desc));
        has_tiles = TRUE;
    }
    if (!has_tiles)
    {
        fclose(fp);
        remove(filepath);
        return;
    }
    fprintf(fp, "END\n");
    fclose(fp);
}


#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )                    \
                if ( !str_cmp( word, literal ) )        \
                {                                       \
                    field  = value;                     \
                    fMatch = TRUE;                      \
                    break;                              \
                }

//loads ALL wmap tile data
void load_wmap_tiles()
{
    char filepath[MAX_STRING_LENGTH];
    WMAPTILE_DATA *tile;
    const char * word;
    bool fMatch, fEnd = FALSE;
    first_wmaptile = NULL;
    last_wmaptile = NULL;
    //char buf[MSL];

    log_string ("Loading wmap tiles...");

    for (int wmap_index = 0; wmap_index < MAX_WMAP; wmap_index++)
    {
        if (!wmap_table[wmap_index].filename || wmap_table[wmap_index].filename[0] == '\0')
            continue;

        tile = NULL;

        snprintf(filepath, sizeof(filepath), "%s%s.dat", WMAP_DIR, wmap_table[wmap_index].filename);

        FILE *fp = fopen(filepath, "r");
        if (!fp)
        {
            bugf("load_wmap_tiles: Could not open %s for reading.", filepath);
            continue;
        }
        //else
        //    bugf("load_wmap_tiles: loading %s for reading.", filepath);

        fMatch = FALSE;
        fEnd = FALSE;
        for ( ; ; )
        {
            word   = feof (fp) ? "END" : fread_word (fp);

            switch (UPPER (word[0]))
            {
                case '*':
                    fMatch = TRUE;
                    fread_to_eol (fp);
                    break;

                case 'C':
                    if (!str_cmp(word, "Coords"))
                    {
                        CREATE(tile, WMAPTILE_DATA, 1);
                        tile->wmap_index = wmap_index;
                        tile->x = fread_number(fp);
                        tile->y = fread_number(fp);
                        tile->z = fread_number(fp);
                        tile->vis = fread_number(fp);
                        tile->pass = fread_number(fp);
                        LINK(tile, first_wmaptile, last_wmaptile, next, prev);
                        fMatch = TRUE;
                        break;
                    }
                    break;

                case 'D':
                    KEY("Desc",tile->desc,fread_string(fp));
                    break;

                case 'E':
                    if (!str_cmp (word, "END"))
                    {
                        fMatch = TRUE;
                        fEnd = TRUE;
                        break;
                    }
                    break;

                case 'N':
                    KEY("Name",tile->name,fread_string(fp));
                    break;

                case 'S':
                    KEY("Symb", tile->symb, fread_string(fp));
                    break;
            }

            if (!fMatch)
            {
                bug ("load_wmap_tiles: no match!", 0);
                //sprintf(buf, "load_wmap_tiles: no match: %s", word );
                //bug(buf,0);
                fread_to_eol(fp);
            }
            if (fEnd)
                break;
        }
        fclose(fp);
    }
}

//check if a sector type in a room or wmap can be passed onto
bool is_passable(CHAR_DATA *ch, ROOM_INDEX_DATA *room, int sector)
{
    if(ch == NULL && room == NULL && sector < 0)
    {
        bug("is_passable: bad ch, room, and sector",0);
        return FALSE;
    }

    if(ch && IS_HOLYLIGHT(ch))
        return TRUE;

    if(room)
        sector = sector_flags[room->sector_type].bit;

    //passable message is NULL, sector is passable
    if(!sector_flags[sector].pass)
        return TRUE;

    if(sector == SECT_AIR && !IS_AFFECTED (ch, AFF_FLYING))
        return FALSE;

    if(sector == SECT_WATER_NOSWIM && !IS_AFFECTED (ch, AFF_FLYING))
    {
        OBJ_DATA *obj;

        for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
        {
            if (obj->item_type == ITEM_BOAT)
                return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}

//saves wmap resets for specified wmap
void save_wmap_resets(int wmap_index)
{
    if (wmap_index < 0 || wmap_index >= MAX_WMAP)
        return;

    char filepath[MAX_STRING_LENGTH];
    snprintf(filepath, sizeof(filepath), WMAP_RESET_FILE, wmap_table[wmap_index].name);

    FILE *fp = fopen(filepath, "w");
    if (!fp)
    {
        bugf("save_wmap_resets: Could not open %s for writing.", filepath);
        return;
    }

    fprintf(fp, "#WMAPRESETS\n");

    WMAP_RESET_DATA *reset;
    int count = 0;

    for (reset = wmap_reset_list[wmap_index]; reset != NULL; reset = reset->next)
    {
        fprintf(fp, "%d %d %d %d %d %d %d %d %d\n",
                reset->reset_type,
                reset->vnum,
                reset->wmap_index,
                reset->x,
                reset->y,
                reset->z,
                reset->max,
                reset->mob_vnum,
                reset->wear ? 1 : 0);
        count++;
    }

    fprintf(fp, "#END\n");
    fclose(fp);

    if (count > 0)
    {
        char buf[MAX_STRING_LENGTH];
        snprintf(buf, sizeof(buf), "Saved %d resets for map %s", count, wmap_table[wmap_index].name);
        log_string(buf);
    }
}

//loads wmap resets for a specific map
void load_wmap_resets(int wmap_index)
{
    if (wmap_index < 0 || wmap_index >= MAX_WMAP)
        return;

    char filepath[MAX_STRING_LENGTH];
    snprintf(filepath, sizeof(filepath), WMAP_RESET_FILE, wmap_table[wmap_index].name);

    FILE *fp = fopen(filepath, "r");
    if (!fp)
        return;

    char line[MAX_STRING_LENGTH];
    WMAP_RESET_DATA *last = NULL;

    while (fgets(line, sizeof(line), fp))
    {
        if (line[0] == '#' || line[0] == '\n')
            continue;

        int reset_type, vnum, map_index, x, y, z, max, mob_vnum, wear;
        if (sscanf(line, "%d %d %d %d %d %d %d %d %d", &reset_type, &vnum, &map_index, &x, &y, &z, &max, &mob_vnum, &wear) == 9)
        {
            WMAP_RESET_DATA *reset = (WMAP_RESET_DATA *)malloc(sizeof(WMAP_RESET_DATA));
            if (!reset)
            {
                bug("load_wmap_resets: Memory allocation failed!", 0);
                continue;
            }

            reset->reset_type = reset_type;
            reset->vnum = vnum;
            reset->wmap_index = map_index;
            reset->x = x;
            reset->y = y;
            reset->z = z;
            reset->max = max;
            reset->mob_vnum = mob_vnum;
            reset->wear = wear;
            reset->next = NULL;

            if (last)
                last->next = reset;
            else
                wmap_reset_list[wmap_index] = reset;
            last = reset;
        }
    }

    fclose(fp);
}

//helper function to determine the wear location of an object, called by resets
int get_wear_location(CHAR_DATA *ch, OBJ_INDEX_DATA *obj)
{
    if (ch == NULL || obj == NULL)
        return WEAR_NONE;

    if (!IS_SET(obj->wear_flags, ITEM_TAKE))
        return WEAR_NONE;

    if (IS_SET(obj->wear_flags, ITEM_WEAR_HEAD))
        return WEAR_HEAD;
    if (IS_SET(obj->wear_flags, ITEM_WEAR_NECK))
    {
        if (!get_eq_char(ch, WEAR_NECK_1))
            return WEAR_NECK_1;
        if (!get_eq_char(ch, WEAR_NECK_2))
            return WEAR_NECK_2;
        return WEAR_HOLD; 
    }
    if (IS_SET(obj->wear_flags, ITEM_WEAR_BODY))
        return WEAR_BODY;
    if (IS_SET(obj->wear_flags, ITEM_WEAR_ARMS))
        return WEAR_ARMS;
    if (IS_SET(obj->wear_flags, ITEM_WEAR_LEGS))
        return WEAR_LEGS;
    if (IS_SET(obj->wear_flags, ITEM_WEAR_FEET))
        return WEAR_FEET;
    if (IS_SET(obj->wear_flags, ITEM_WEAR_HANDS))
        return WEAR_HANDS;
    if (IS_SET(obj->wear_flags, ITEM_WEAR_FINGER))
    {
        if (!get_eq_char(ch, WEAR_FINGER_L))
            return WEAR_FINGER_L;
        if (!get_eq_char(ch, WEAR_FINGER_R))
            return WEAR_FINGER_R;
        return WEAR_HOLD; 
    }
    if (IS_SET(obj->wear_flags, ITEM_WEAR_SHIELD))
        return WEAR_SHIELD;
    if (IS_SET(obj->wear_flags, ITEM_WEAR_ABOUT))
        return WEAR_ABOUT;
    if (IS_SET(obj->wear_flags, ITEM_WEAR_WAIST))
        return WEAR_WAIST;
    if (IS_SET(obj->wear_flags, ITEM_WEAR_WRIST))
    {
        if (!get_eq_char(ch, WEAR_WRIST_L))
            return WEAR_WRIST_L;
        if (!get_eq_char(ch, WEAR_WRIST_R))
            return WEAR_WRIST_R;
        return WEAR_HOLD; 
    }
    if (IS_SET(obj->wear_flags, ITEM_WIELD))
        return WEAR_WIELD;
    if (IS_SET(obj->wear_flags, ITEM_HOLD))
        return WEAR_HOLD;

    if (IS_SET(obj->wear_flags, ITEM_TAKE))
        return WEAR_HOLD;

    return WEAR_NONE;
}

//handles loading wmap resets onto the map
void process_wmap_resets()
{
    for (int i = 0; i < MAX_WMAP; i++)
    {
        WMAP_RESET_DATA *reset;
        ROOM_INDEX_DATA *room = get_room_index(wmap_table[i].vnum);
        if (!room)
            continue;

        for (reset = wmap_reset_list[i]; reset != NULL; reset = reset->next)
        {
            if (reset->reset_type != RESET_TYPE_MOB)
                continue;

            int current_count = 0;
            CHAR_DATA *ch;
            for (ch = room->people; ch != NULL; ch = ch->next_in_room)
            {
                if (IS_NPC(ch) && ch->pIndexData->vnum == reset->vnum && ch->reset_wmap[0] == reset->wmap_index
                && ch->reset_wmap[1] == reset->x && ch->reset_wmap[2] == reset->y  && ch->reset_wmap[3] == reset->z)
                    current_count++;
            }

            if (current_count < reset->max)
            {
                MOB_INDEX_DATA *pMobIndex = get_mob_index(reset->vnum);
                if (pMobIndex)
                {
                    CHAR_DATA *mob = create_mobile(pMobIndex);
                    //set coords for mob to load into
                    set_wmap(mob, NULL, reset->wmap_index);
                    set_z(mob, NULL, reset->z);
                    char_to_room(mob, room);
                    set_coords(mob, NULL, reset->x, reset->y);
                    //set coords where mob/reset originated from
                    mob->reset_wmap[0] = reset->wmap_index;
                    mob->reset_wmap[1] = reset->x;
                    mob->reset_wmap[2] = reset->y;
                    mob->reset_wmap[3] = reset->z;
                }
            }
        }

        for (reset = wmap_reset_list[i]; reset != NULL; reset = reset->next)
        {
            int current_count = 0;

            if (reset->reset_type == RESET_TYPE_OBJ)
            {
                OBJ_DATA *obj;
                for (obj = room->contents; obj != NULL; obj = obj->next_content)
                {
                    if (obj->pIndexData->vnum == reset->vnum && obj->reset_wmap[0] == reset->wmap_index
                    && obj->reset_wmap[1] == reset->x && obj->reset_wmap[2] == reset->y  && obj->reset_wmap[3] == reset->z)
                        current_count++;
                }

                if (current_count < reset->max)
                {
                    OBJ_INDEX_DATA *pObjIndex = get_obj_index(reset->vnum);
                    if (pObjIndex)
                    {
                        OBJ_DATA *obj = create_object(pObjIndex, pObjIndex->level);
                        obj_to_room(obj, room);
                        //set coords for obj to load into
                        set_wmap(NULL, obj, reset->wmap_index);
                        set_z(NULL, obj, reset->z);
                        set_coords(NULL, obj, reset->x, reset->y);
                        //set coords where obj/reset originated from
                        obj->reset_wmap[0] = reset->wmap_index;
                        obj->reset_wmap[1] = reset->x;
                        obj->reset_wmap[2] = reset->y;
                        obj->reset_wmap[3] = reset->z;
                    }
                }
            }
            else if (reset->reset_type == RESET_TYPE_OBJ_ON_MOB)
            {
                CHAR_DATA *mob = NULL;
                for (CHAR_DATA *ch = room->people; ch != NULL; ch = ch->next_in_room)
                {
                    if (IS_NPC(ch) && ch->pIndexData->vnum == reset->mob_vnum && ch->reset_wmap[0] == reset->wmap_index
                    && ch->reset_wmap[1] == reset->x && ch->reset_wmap[2] == reset->y  && ch->reset_wmap[3] == reset->z)
                    {
                        mob = ch;
                        break;
                    }
                }

                if (!mob)
                    continue;

                OBJ_INDEX_DATA *pObjIndex = get_obj_index(reset->vnum);
                if (!pObjIndex)
                    continue;

                int wear_loc = reset->wear ? get_wear_location(mob, pObjIndex) : WEAR_HOLD;
                if (wear_loc != WEAR_NONE)
                {
                    OBJ_DATA *equipped = get_eq_char(mob, wear_loc);
                    if (equipped && equipped->pIndexData->vnum == reset->vnum)
                        current_count++;
                }

                for (OBJ_DATA *obj = mob->carrying; obj != NULL; obj = obj->next_content)
                {
                    if (obj->pIndexData->vnum == reset->vnum && obj->wear_loc == WEAR_NONE)
                        current_count++;
                }

                if (current_count < reset->max)
                {
                    OBJ_DATA *new_obj = create_object(pObjIndex, pObjIndex->level);
                    obj_to_char(new_obj, mob);
                    if (reset->wear)
                    {
                        wear_loc = get_wear_location(mob, pObjIndex);
                        if (wear_loc != WEAR_NONE)
                            equip_char(mob, new_obj, wear_loc);
                        else
                            equip_char(mob, new_obj, WEAR_HOLD); 
                    }
                    else
                    {
                        equip_char(mob, new_obj, WEAR_HOLD);
                    }
                }
            }
        }
    }
}

//begin flood fill floodfill flood_fill
// Structure to hold a coordinate
typedef struct
{
    int x, y;
} Point;

// Stack implementation for flood fill
typedef struct
{
    Point *data;
    int size;
    int capacity;
} Stack;

// Function to initialize a stack
void init_stack(Stack *stack, int capacity)
{
    stack->data = (Point *)malloc(capacity * sizeof(Point));
    stack->size = 0;
    stack->capacity = capacity;
}

// Function to push a point onto the stack
void push(Stack *stack, int x, int y)
{
    if (stack->size >= stack->capacity)
    {
        stack->capacity *= 2;
        stack->data = (Point *)realloc(stack->data, stack->capacity * sizeof(Point));
    }
    stack->data[stack->size++] = (Point){x, y};
}

// Function to pop a point from the stack
Point pop(Stack *stack)
{
    return stack->data[--stack->size];
}

// Function to check if the stack is empty
int is_empty(Stack *stack)
{
    return stack->size == 0;
}

// Function to free the stack
void free_stack(Stack *stack)
{
    free(stack->data);
}

// Iterative flood fill function for grid
void flood_fill(CHAR_DATA *ch, int wmap_index, int x, int y, int replacement)
{
    int wmaps = MAX_WMAP;
    int height = wmax_x(ch);
    int width = wmax_y(ch);
    int target = get_sector(NULL,wmap_index,x,y);

    if (wmap_index < 0 || wmap_index >= wmaps || x < 0 || y < 0 || x >= width || y >= height || get_sector(NULL,wmap_index,x,y) != target || get_sector(NULL,wmap_index,x,y) == replacement)
    {
        send_to_char("flood_fill: out of bounds\r\n",ch);
        return;
    }

    Stack stack;
    init_stack(&stack, width * height);
    push(&stack, x, y);

    while (!is_empty(&stack))
    {
        Point p = pop(&stack);
        int px = p.x, py = p.y;

        if (px < 0 || py < 0 || px >= width || py >= height || get_sector(NULL,wmap_index,px,py) != target)
            continue;

        wmap_table[wmap_index].grid[px][py] = replacement;

        push(&stack, px + 1, py);
        push(&stack, px - 1, py);
        push(&stack, px, py + 1);
        push(&stack, px, py - 1);
    }

    free_stack(&stack);
}

//handles drawing a fractal of specified sector, decay makes the outer edges more sparse
void fractal_fill(CHAR_DATA *ch, int wmap_index, int x, int y, int distance, int sector, bool use_decay)
{
    int height = wmax_x(ch);
    int width = wmax_y(ch);

    // Base case: If the distance is 0, stop recursion
    if (distance < 1)
    {
        return;
    }

    // Declare the center threshold based on the grid dimensions (20% of the width and height)
    int center_threshold_x = width / 5;  // Define the threshold as 20% of the width
    int center_threshold_y = height / 5; // Define the threshold as 20% of the height

    // Apply the fractal to the center point
    wmap_table[wmap_index].grid[x][y] = sector;

    // Apply the fractal to neighboring points
    for (int dx = -distance; dx <= distance; dx++)
    {
        for (int dy = -distance; dy <= distance; dy++)
        {
            int new_x = x + dx;
            int new_y = y + dy;

            // Ensure the point is within bounds
            if (new_x >= 0 && new_x < width && new_y >= 0 && new_y < height)
            {
                // Calculate the Euclidean distance from the center for the neighboring point
                float center_distance = sqrt((new_x - width / 2) * (new_x - width / 2) + (new_y - height / 2) * (new_y - height / 2));

                // Apply decay based on the radial distance from the center
                if (use_decay)
                {
                    float decay_factor = 0.0f;

                    // Apply stronger effect in the center area
                    if (center_distance < (center_threshold_x + center_threshold_y) / 2)
                    {
                        decay_factor = 55.0f;  // 55% chance for the center area to be filled
                    }
                    else
                    {
                        // Apply decay based on the radial distance
                        float distance_factor = center_distance / (width + height);

                        // For the outermost 35%, reduce the chance significantly (2-3% chance at max distance)
                        if (distance_factor > 0.65f)  // Farthest 35% of the distance
                        {
                            decay_factor = 1.0f + (rand() % 2);  // 2 to 3% chance for the outermost 20%
                        }
                        else
                        {
                            // Apply exponential decay effect for a smooth transition
                            decay_factor = 60.0f / (1 + center_distance * center_distance / (width * height));  // Radial exponential decay
                        }
                    }

                    // Apply the sector value based on decay factor
                    if (rand() % 100 < decay_factor)
                    {
                        wmap_table[wmap_index].grid[new_x][new_y] = sector;
                    }
                }
                else
                {
                    // No decay; random application
                    if (rand() % 2 == 0)
                    {
                        wmap_table[wmap_index].grid[new_x][new_y] = sector;
                    }
                }
            }
        }
    }

    // Recurse further, decreasing the distance
    fractal_fill(ch, wmap_index, x, y, distance / 2, sector, use_decay);
}

//draws a square of specified sector, font will adjust for aspect ratio to make square more visibly squared
void square_fill(CHAR_DATA *ch, int wmap_index, int x, int y, int distance, int sector, bool font)
{
    int height = wmax_x(ch);
    int width = wmax_y(ch);

    float aspect_ratio = font ? 0.75f : 1.0f; // Reduce height when font is true
    float adjusted_distance_y = distance * aspect_ratio;
    float adjusted_distance_x = distance * (font ? 1.3f : 1.0f); // Slightly extend width

    for (int dx = -adjusted_distance_x; dx <= adjusted_distance_x; dx++)
    {
        for (int dy = -adjusted_distance_y; dy <= adjusted_distance_y; dy++)
        {
            int new_x = x + dx;
            int new_y = y + dy;

            if (new_x >= 0 && new_x < width && new_y >= 0 && new_y < height)
            {
                wmap_table[wmap_index].grid[new_x][new_y] = sector;
            }
        }
    }
}

//draws a circle of specified sector, font will adjust for aspect ratio to make circle more visibly round
void circle_fill(CHAR_DATA *ch, int wmap_index, int x, int y, int distance, int sector, bool font)
{
    int height = wmax_x(ch);
    int width = wmax_y(ch);
    
    float aspect_ratio = font ? 0.75f : 1.0f; // Reduce height more when font is true
    float adjusted_distance_y = distance * aspect_ratio;
    float adjusted_distance_x = distance * (font ? 1.3f : 1.0f); // Extend width a bit more

    for (int dx = -adjusted_distance_x; dx <= adjusted_distance_x; dx++)
    {
        for (int dy = -adjusted_distance_y; dy <= adjusted_distance_y; dy++)
        {
            int new_x = x + dx;
            int new_y = y + dy;

            if (new_x >= 0 && new_x < width && new_y >= 0 && new_y < height)
            {
                float distance_check = (dx * dx) / (adjusted_distance_x * adjusted_distance_x) + (dy * dy) / (adjusted_distance_y * adjusted_distance_y);
                if (distance_check <= 1.0f)
                {
                    wmap_table[wmap_index].grid[new_x][new_y] = sector;
                }
            }
        }
    }
}

int active_wmap_width = 0;
int active_wmap_height = 0;

//loads the PNG into the wmap array memory
void load_png(int wmap_index)
{
    char buf[MAX_STRING_LENGTH];

    if (wmap_index < 0 || wmap_index >= MAX_WMAP || wmap_table[wmap_index].name == NULL)
    {
        bug("Invalid map index.", 0);
        return;
    }

    const struct wmap_type *wmap = &wmap_table[wmap_index];
    char filepath[MAX_STRING_LENGTH];
    snprintf(filepath, sizeof(filepath), "%s%s", WMAP_DIR, wmap->filename);

    unsigned char *image = NULL;
    unsigned width, height;

    unsigned error = lodepng_decode24_file(&image, &width, &height, filepath);
    if (error)
    {
        sprintf(buf, "Error loading PNG: %s", lodepng_error_text(error));
        bug(buf, 0);
        return;
    }

    if (width != wmap->max_x || height != wmap->max_y)
    {
        sprintf(buf, "Image size (%dx%d) doesn't match expected map size (%dx%d)",
                width, height, wmap->max_x, wmap->max_y);
        bug(buf, 0);
        free(image);
        return;
    }

    // Allocate grid if not already allocated
    if (wmap_table[wmap_index].grid == NULL)
    {
        // Allocate memory for the grid (2D array of uint8_t)
        wmap_table[wmap_index].grid = (uint8_t **)malloc(wmap->max_x * sizeof(uint8_t *));
        if (!wmap_table[wmap_index].grid)
        {
            bug("Memory allocation failed for grid rows.\r\n", 0);
            free(image);
            return;
        }

        for (int x = 0; x < wmap->max_x; x++)
        {
            wmap_table[wmap_index].grid[x] = (uint8_t *)malloc(wmap->max_y * sizeof(uint8_t));  // Now uint8_t
            if (!wmap_table[wmap_index].grid[x])
            {
                bug("Memory allocation failed for grid columns.\r\n", 0);
                // Free previously allocated rows
                for (int i = 0; i < x; i++)
                    free(wmap_table[wmap_index].grid[i]);
                free(wmap_table[wmap_index].grid);
                wmap_table[wmap_index].grid = NULL;
                free(image);
                return;
            }
            for (int y = 0; y < wmap->max_y; y++)
            {
                wmap_table[wmap_index].grid[x][y] = 0;  // Initialize grid to 0 (or another default value)
            }
        }
    }

    // Populate the grid with terrain data
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int index = (y * width + x) * 3;
            unsigned char r = image[index];
            unsigned char g = image[index + 1];
            unsigned char b = image[index + 2];

            // Determine the sector based on RGB values
            int sec;
            for (sec = 0; sec < SECT_MAX; sec++)
            {
                if (sector_flags[sec].red == r && sector_flags[sec].green == g && sector_flags[sec].blue == b)
                {
                    wmap_table[wmap_index].grid[x][y] = sec;  // Store sector index as uint8_t
                    break;
                }
            }

            if (sec == SECT_MAX)
            {
                sprintf(buf, "No matching sector found for pixel at (%d, %d) with color (R=%d, G=%d, B=%d)\n", x, y, r, g, b);
                bug(buf, 0);
            }
        }
    }

    free(image);  // Don't forget to free the loaded PNG data
}

//reloads the map from PNG, called by wedit_reload
void load_wmap(int wmap_index)
{
    char buf[MAX_STRING_LENGTH];
    if (wmap_index < 0 || wmap_index >= MAX_WMAP || wmap_table[wmap_index].name == NULL)
    {
        bug("Invalid map index",0);
        return;
    }

    active_wmap_width = wmap_table[wmap_index].max_x;
    active_wmap_height = wmap_table[wmap_index].max_y;

    if (active_wmap_width > wmap_table[wmap_index].max_x || active_wmap_height > wmap_table[wmap_index].max_y)
    {
        sprintf(buf, "Loaded map size exceeds max allowed dimensions.");
        bug(buf,0);
        return;
    }

    load_png(wmap_index);
}

// Function to save the PNG using LodePNG (for 24-bit images)
int save_image(const char *filename, int wmap_index)
{
    char buf[MAX_STRING_LENGTH];
    unsigned width = wmap_table[wmap_index].max_x;
    unsigned height = wmap_table[wmap_index].max_y;

    unsigned char *image = (unsigned char *)malloc(3 * width * height); // Allocate memory for RGB data (3 bytes per pixel)

    // Convert grid data to RGB image data
    for (unsigned y = 0; y < height; y++)
    {
        for (unsigned x = 0; x < width; x++)
        {
            uint8_t sector_index = wmap_table[wmap_index].grid[x][y];  // Directly access the terrain value (sector index)

            // Get RGB values from sector_flags using the sector index
            int r = sector_flags[sector_index].red; 
            int g = sector_flags[sector_index].green;
            int b = sector_flags[sector_index].blue;

            unsigned char *pixel = &image[3 * (y * width + x)];
            pixel[0] = r;     
            pixel[1] = g; 
            pixel[2] = b; 
        }
    }

    // Define directory and construct full paths
    const char *directory = "../maps/";  // Directory where images are saved
    const char *temp_filename = "temp_image.png";
    char temp_filepath[256];
    char final_filepath[256];

    // Construct full file paths
    snprintf(temp_filepath, sizeof(temp_filepath), "%s%s", directory, temp_filename);
    snprintf(final_filepath, sizeof(final_filepath), "%s%s", directory, filename);

    // Save the image as a temporary file
    unsigned error = lodepng_encode24_file(temp_filepath, image, width, height);
    free(image);  // Free the image buffer

    if (error)
    {
        sprintf(buf, "Error %u: %s", error, lodepng_error_text(error));
        bug(buf, 0);
        return 0;  // Indicating failure to save
    }

    // Check if the temporary file was saved correctly
    FILE *temp_file = fopen(temp_filepath, "rb");
    if (!temp_file)
    {
        sprintf(buf, "Failed to open temporary file for verification.");
        bug(buf, 0);
        return 0;  // Failed to verify the file
    }
    fclose(temp_file);  // Close the temporary file after verification

    // Rename the temporary file to overwrite the original file
    if (rename(temp_filepath, final_filepath) != 0)
    {
        sprintf(buf, "Error: Failed to rename temporary file to original file.");
        bug(buf, 0);
        return 0;  // Renaming failed
    }

    sprintf(buf, "Image saved successfully to %s", final_filepath);
    bug(buf, 0);
    return 1;  // Success
}

//Suggested by Hi-Potion, aka TheDude, presents a smaller, "zoomed out" version of the map
void do_zoomwmap(CHAR_DATA *ch, char *argument)
{
    int zoom;
    if (!is_number(argument)) return;

    if ((zoom = atoi(argument)) < 1) return;

    int wmap_index = ch->wmap[0];
    int center_x = ch->wmap[1];
    int center_y = ch->wmap[2];
    int view_radius = get_view_radius(ch);
    int view_diameter = view_radius * 2 + 1;
    int adjusted_height = (view_radius * 1.8) / 2;

    const char ***zoomed_grid = (const char ***)calloc(view_diameter, sizeof(char**));
    for (int i = 0; i < view_diameter; i++)
    {
        zoomed_grid[i] = (const char **)calloc(view_diameter, sizeof(char*));
    }

    // Resize the map with zoom
    for (int dy = -adjusted_height / 2; dy <= adjusted_height / 2; dy++)
    {
        for (int dx = -view_radius; dx <= view_radius; dx++) // Fixed loop variable
        {
            int start_x = center_x + dx * zoom;
            int start_y = center_y + dy * zoom;
            int end_x = start_x + zoom;
            int end_y = start_y + zoom;

            int terrain_count[SECT_MAX] = {0};
            int max_count = 0;
            int mode_terrain = 0;

            for (int y = start_y; y < end_y; y++)
            {
                for (int x = start_x; x < end_x; x++)
                {
                    int wrapped_x = (x + wmap_table[wmap_index].max_x) % wmap_table[wmap_index].max_x;
                    int wrapped_y = (y + wmap_table[wmap_index].max_y) % wmap_table[wmap_index].max_y;
                    
                    // Access sector index directly (no need for WMAPTILE_DATA)
                    uint8_t sector_index = wmap_table[wmap_index].grid[wrapped_x][wrapped_y];

                    if (sector_index < SECT_MAX)
                    {
                        terrain_count[sector_index]++;
                        if (terrain_count[sector_index] > max_count)
                        {
                            max_count = terrain_count[sector_index];
                            mode_terrain = sector_index;
                        }
                    }
                }
            }

            // Store the most frequent terrain symbol in zoomed grid
            zoomed_grid[dy + adjusted_height / 2][dx + view_radius] = sector_flags[mode_terrain].wmap_symb;
        }
    }

    // Display the zoomed map with borders
    char buf[MAX_STRING_LENGTH];

    int fade = 0;
    #if defined(WMAP_NIGHT_FADE16)
    if (weather_info.sunlight == SUN_SET || weather_info.sunlight == SUN_DARK)
        fade = 16;
    #endif
    #if defined(WMAP_NIGHT_FADE256)
    if (weather_info.sunlight == SUN_SET || weather_info.sunlight == SUN_DARK)
        fade = 256;
    #endif

    // Top border
    send_to_char("{D+", ch);
    for (int i = 0; i < view_diameter; i++)
        send_to_char("-", ch);
    send_to_char("+{x\n", ch);

    // Map content
    for (int dy = -adjusted_height / 2; dy <= adjusted_height / 2; dy++)
    {
        char *buf_ptr = buf;

        // Left border
        *buf_ptr++ = '{';
        *buf_ptr++ = 'D';
        *buf_ptr++ = '|';

        // Map content
        for (int dx = -view_radius; dx <= view_radius; dx++)
        {
            // Check if we're at the center of the map
            if (dx == 0 && dy == 0)
            {
                // Display "{R*" at the center
                buf_ptr = stpcpy(buf_ptr, "{R*");
            }
            else
            {
                const char *symbol = zoomed_grid[dy + adjusted_height / 2][dx + view_radius];
                if (symbol)
                {
                    if (fade == 16)
                        buf_ptr = stpcpy(buf_ptr, fade_color16(symbol));
                    else if (fade == 256)
                        buf_ptr = stpcpy(buf_ptr, fade_color(symbol, 2));
                    else
                        buf_ptr = stpcpy(buf_ptr, symbol);
                }
                else
                    *buf_ptr++ = ' ';
            }
        }

        // Right border
        *buf_ptr++ = '{';
        *buf_ptr++ = 'D';
        *buf_ptr++ = '|';
        *buf_ptr++ = '{';
        *buf_ptr++ = 'x';
        *buf_ptr++ = '\r';
        *buf_ptr++ = '\n';
        *buf_ptr = '\0';

        send_to_char(strip_recolor(buf), ch);
    }

    // Bottom border
    send_to_char("{D+", ch);
    for (int i = 0; i < view_diameter; i++)
        send_to_char("-", ch);
    send_to_char("+{x\n", ch);

    // Free allocated memory for zoomed_grid
    for (int i = 0; i < view_diameter; i++)
    {
        free(zoomed_grid[i]);
    }
    free(zoomed_grid);
}

//worldmap.c - helper function for calculating distance on the worldmap
double wmap_dist(int x1, int y1, int x2, int y2)
{
    int dx = x2 - x1;
    int dy = y2 - y1;
    return sqrt(dx * dx + dy * dy);
}

//worldmap.c - Diagonal movement commands
void do_northeast (CHAR_DATA * ch, char *argument)
{
    move_char (ch, DIR_NORTHEAST, FALSE);
    return;
}

void do_northwest (CHAR_DATA * ch, char *argument)
{
    move_char (ch, DIR_NORTHWEST, FALSE);
    return;
}

void do_southeast (CHAR_DATA * ch, char *argument)
{
    move_char (ch, DIR_SOUTHEAST, FALSE);
    return;
}

void do_southwest (CHAR_DATA * ch, char *argument)
{
    move_char (ch, DIR_SOUTHWEST, FALSE);
    return;
}
