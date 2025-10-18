/******************************************************************************
*       *****   **    **           **             ***** ***        *******    *         
*    ******  ***** *****        *****          ******  * **      *       ***  *         
*   **   *  *  ***** *****     *  ***         **   *  *  **     *         **  *         
*  *    *  *   * **  * **         ***        *    *  *   **     **        *   *         
*      *  *    *     *           *  **           *  *    *       ***          *         
*     ** **    *     *           *  **          ** **   *       ** ***        *         
*     ** **    *     *          *    **         ** **  *         *** ***      *         
*     ** **    *     *          *    **         ** ****            *** ***    *         
*     ** **    *     *         *      **        ** **  ***           *** ***  *         
*     ** **    *     **        *********        ** **    **            ** *** *         
*     *  **    *     **       *        **       *  **    **             ** ** *         
*        *     *      **      *        **          *     **              * *  *         
*    ****      *      **     *****      **     ****      ***   ***        *   *         
*   *  *****           **   *   ****    ** *  *  ****    **   *  *********    *         
*  *     **                *     **      **  *    **     *   *     *****      *         
*  *                       *                 *               *                *         
*    **                      **                **              **             *         
*                                                                             *
*  MULTI                    ADVENTURER         ROLEPLAYING     SYSTEM         *
*  MARS 0.1b is copyright 2015-2024 Ro Black mars@wr3tch.org                  *
******************************************************************************/
/******************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,           *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.      *
 *                                                                            *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael             *
 *  Chastain, Michael Quan, and Mitchell Tse.                                 *
 *                                                                            *
 *  In order to use any part of this Merc Diku Mud, you must comply with      *
 *  both the original Diku license in 'license.doc' as well the Merc          *
 *  license in 'license.txt'.  In particular, you may not remove either of    *
 *  these copyright notices.                                                  *
 *                                                                            *
 *  Much time and thought has gone into this software and you are             *
 *  benefitting.  We hope that you share your changes too.  What goes         *
 *  around, comes around.                                                     *
 *****************************************************************************/

/******************************************************************************
*	ROM 2.4 is copyright 1993-1998 Russ Taylor			                      *
*	ROM has been brought to you by the ROM consortium		                  *
*	    Russ Taylor (rtaylor@hypercube.org)				                      *
*	    Gabrielle Taylor (gtaylor@hypercube.org)			                  *
*	    Brian Moore (zump@rom.org)					                          *
*	By using this code, you have agreed to follow the terms of the	          *
*	ROM license, in the file Rom24/doc/rom.license			                  *
******************************************************************************/
/* dwatch egate artifice fdmud code copyright 2002-2008 Bob Kircher	  */


#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <malloc.h>
#include "merc.h"
#include "const.h"
#include "recycle.h"
#include "lookup.h"
#include "tables.h"

#define DONATION_FILE "../area/donation.dat"
#define PITS_FILE "../area/pits.dat"
#define TOKENS_FILE "../area/tokens.dat"

void save_donation_pits(void);
void load_donation_pits(void);
void save_pits(void);
void load_pits(void);
void save_tokens(void);
void load_tokens(void);
/* from save.c and handler.c - use the standard working functions */
void    fwrite_obj  args( ( CHAR_DATA *ch,  OBJ_DATA  *obj, FILE *fp, int iNest ) );
void    fread_obj   args( ( CHAR_DATA *ch,  FILE *fp ) );
void    obj_from_room args( ( OBJ_DATA *obj ) );
void    obj_to_obj  args( ( OBJ_DATA *obj, OBJ_DATA *obj_to ) );


void save_donation_pits(void)
{
    FILE *fp;
    OBJ_DATA *obj;
    int obj_count = 0;
    char buf[MAX_STRING_LENGTH];

    if ((fp = fopen(DONATION_FILE, "w")) == NULL)
    { bug("save_donation_pits: fopen", 0); return; }

    log_string("save_donation_pits: Saving donation room objects");

    /* Use object_list approach like pits and tokens */
    for (obj = object_list; obj != NULL; obj = obj->next)
    {
        /* Save objects in donation rooms, but NOT tokens (saved separately) */
        if (obj->in_room != NULL && 
            IS_SET(obj->in_room->room_flags, ROOM_DONATION) &&
            obj->item_type != ITEM_TOKEN)
        {
            sprintf(buf, "save_donation_pits: Saving object vnum %d from donation room %d", 
                    obj->pIndexData->vnum, obj->in_room->vnum);
            log_string(buf);
            
            /* Temporarily break next_content to prevent recursion */
            OBJ_DATA *saved_next = obj->next_content;
            obj->next_content = NULL;
            
            fwrite_obj(NULL, obj, fp, 0);
            
            /* Restore */
            obj->next_content = saved_next;
            
            obj_count++;
        }
    }

    fprintf(fp, "#END\n");
    fclose(fp);
    
    sprintf(buf, "save_donation_pits: Saved %d objects from donation rooms", obj_count);
    log_string(buf);
}

void save_pits(void)
{
    FILE *fp;
    OBJ_DATA *pit;
    int pit_count = 0;
    int obj_count = 0;
    char buf[MAX_STRING_LENGTH];

    if (OBJ_VNUM_PIT <= 0)
        return; /* Pits not configured */

    if ((fp = fopen(PITS_FILE, "w")) == NULL)
    { bug("save_pits: fopen", 0); return; }

    log_string("save_pits: Saving pit contents");

    /* Iterate through global object list to find pits */
    for (pit = object_list; pit != NULL; pit = pit->next)
    {
        /* Only save contents of pits that are in rooms */
        if (pit->pIndexData->vnum != OBJ_VNUM_PIT)
            continue;
        if (pit->in_room == NULL)
            continue;
        
        /* Save each object IN the pit, with room info */
        OBJ_DATA *obj;
                for (obj = pit->contains; obj; obj = obj->next_content)
        {
            OBJ_DATA *obj_next;
            pit_count++;
            sprintf(buf, "save_pits: Saving object vnum %d from pit in room %d", 
                    obj->pIndexData->vnum, pit->in_room->vnum);
            log_string(buf);
            
            /* Temporarily set in_room so fread_obj can place it */
            obj_next = obj->next_content;
            obj->next_content = NULL; /* Don't recurse - we're looping ourselves */
            
            /* Save which room's pit this came from */
            fprintf(fp, "PitRoom %d\n", pit->in_room->vnum);
            
            /* Temporarily set in_room for save */
            ROOM_INDEX_DATA *saved_in_room = obj->in_room;
            OBJ_DATA *saved_in_obj = obj->in_obj;
            obj->in_room = pit->in_room; /* Set room so fread_obj can load it */
            obj->in_obj = NULL;
            
            fwrite_obj(NULL, obj, fp, 0);
            
            /* Restore */
            obj->in_room = saved_in_room;
            obj->in_obj = saved_in_obj;
            obj->next_content = obj_next;
            
            obj_count++;
        }
    }

    fprintf(fp, "#END\n");
    fclose(fp);
    
    sprintf(buf, "save_pits: Saved %d objects from pits",
            obj_count);
    log_string(buf);
}

void load_donation_pits(void)
{
    FILE *fp;
    char *word;
    char letter;
    char buf[MAX_STRING_LENGTH];
    
    if ((fp = fopen(DONATION_FILE, "r")) == NULL) 
    {
        log_string("load_donation_pits: No donation.dat file found");
        return;
    }

    log_string("load_donation_pits: Loading donation room objects");

    /* Use exact same logic as load_copyover_obj */
    for (;;)
    {
        letter = fread_letter(fp);
        if (letter == '*')
        {
            fread_to_eol(fp);
            continue;
        }
        if (letter != '#')
        {
            bug("load_donation_pits: # not found.", 0);
            break;
        }
        
        word = fread_word(fp);
        if (!str_cmp(word, "OBJECT")) 
            fread_obj(NULL, fp);
        else if (!str_cmp(word, "O"))
            fread_obj(NULL, fp);
        else if (!str_cmp(word, "END"))
            break;
        else
        {
            bug("load_donation_pits: bad section.", 0);
            break;
        }
    }
    fclose(fp);
    
    log_string("load_donation_pits: Load complete");
}

void load_pits(void)
{
    FILE *fp;
    char *word;
    char letter;
    int room_vnum = 0;
    int obj_count = 0;
    char buf[MAX_STRING_LENGTH];
    
    if (OBJ_VNUM_PIT <= 0)
        return; /* Pits not configured */
    
    if ((fp = fopen(PITS_FILE, "r")) == NULL) 
    {
        log_string("load_pits: No pits.dat file found");
        return;
    }

    log_string("load_pits: Loading pit contents");

    for (;;)
    {
        letter = fread_letter(fp);
        if (letter == '*')
        {
            fread_to_eol(fp);
            continue;
        }
        
        /* Handle PitRoom lines (not starting with #) */
        if (letter != '#')
        {
            ungetc(letter, fp);
            word = fread_word(fp);
            if (!str_cmp(word, "PitRoom"))
            {
                room_vnum = fread_number(fp);
                sprintf(buf, "load_pits: Next object goes to pit in room %d", room_vnum);
                log_string(buf);
                continue;
            }
            break; /* Unknown keyword */
        }
        
        /* Handle #O or #END */
        word = fread_word(fp);
        if (!str_cmp(word, "OBJECT") || !str_cmp(word, "O"))
        {
            fread_obj(NULL, fp);
            /* Object was loaded by fread_obj and placed in room_vnum */
            /* We need to move it from the room INTO the pit */
            if (room_vnum > 0)
            {
                ROOM_INDEX_DATA *room = get_room_index(room_vnum);
                if (room)
                {
                    /* Find the pit in this room */
                    OBJ_DATA *pit;
                    for (pit = room->contents; pit; pit = pit->next_content)
                    {
                        if (pit->pIndexData->vnum == OBJ_VNUM_PIT)
                        {
                            /* Find the loaded object in this room (not the pit itself) */
                            OBJ_DATA *obj, *obj_next;
                            for (obj = room->contents; obj; obj = obj_next)
                            {
                                obj_next = obj->next_content;
                                if (obj->pIndexData->vnum != OBJ_VNUM_PIT)
                                {
                                    /* This is an item that should be in the pit */
                                    obj_from_room(obj);
                                    obj_to_obj(obj, pit);
                                    obj_count++;
                                    sprintf(buf, "load_pits: Moved object vnum %d into pit (room %d)", 
                                            obj->pIndexData->vnum, room_vnum);
                                    log_string(buf);
                                    break; /* Only move one object per #O read */
                                }
                            }
                            break;
                        }
                    }
                }
            }
        }
        else if (!str_cmp(word, "END"))
            break;
        else
        {
            bug("load_pits: bad section.", 0);
            break;
        }
    }
    fclose(fp);
    
    sprintf(buf, "load_pits: Loaded %d objects into pits", obj_count);
    log_string(buf);
}

void save_tokens(void)
{
    FILE *fp;
    OBJ_DATA *obj;
    int token_count = 0;
    char buf[MAX_STRING_LENGTH];

    if ((fp = fopen(TOKENS_FILE, "w")) == NULL)
    { bug("save_tokens: fopen", 0); return; }
    
    log_string("save_tokens: Starting to save tokens to tokens.dat");

    /* Iterate through global object_list to avoid recursion issues */
    for (obj = object_list; obj != NULL; obj = obj->next)
    {
        if (obj->item_type == ITEM_TOKEN && obj->in_room != NULL)
        {
            sprintf(buf, "save_tokens: Found token %p vnum %d in room %d", 
                    obj, obj->pIndexData->vnum, obj->in_room->vnum);
            log_string(buf);
            
            fprintf(fp, "Room %d\n", obj->in_room->vnum);
            /* Temporarily break the next_content link to prevent recursion */
            OBJ_DATA *saved_next = obj->next_content;
            obj->next_content = NULL;
            
            fwrite_obj(NULL, obj, fp, 0);
            
            /* Restore the link */
            obj->next_content = saved_next;
            
                token_count++;
            sprintf(buf, "save_tokens: Saved token from room %d", obj->in_room->vnum);
            log_string(buf);
        }
    }

    fprintf(fp, "#END\n");
    fclose(fp);
    sprintf(buf, "save_tokens: Saved %d tokens", token_count);
    log_string(buf);
}

void load_tokens(void)
{
    FILE *fp;
    ROOM_INDEX_DATA *room = NULL;
    OBJ_DATA *obj;
    int room_vnum;
    char *word;
    int token_count = 0;

    if ((fp = fopen(TOKENS_FILE, "r")) == NULL) 
    {
        log_string("load_tokens: tokens.dat file not found");
        return;
    }
    
    log_string("load_tokens: Starting to load tokens from tokens.dat");

    for (;;)
    {
        char letter = fread_letter(fp);
        if (letter == '*') { fread_to_eol(fp); continue; }
        if (letter == EOF) break;

        /* Handle lines that don't start with # (like "Room 499") */
        if (letter != '#')
        {
            ungetc(letter, fp);
            word = fread_word(fp);
            if (!str_cmp(word, "Room"))
            {
                room_vnum = fread_number(fp);
                room = get_room_index(room_vnum);
                if (!room) 
                {
                    log_string("load_tokens: Room not found, skipping");
                    fread_to_eol(fp);
                    continue;
                }
                log_string("load_tokens: Found room for tokens");
            }
            else
            {
                fread_to_eol(fp);
            }
            continue;
        }

        /* Handle lines that start with # */
        word = fread_word(fp);
        if (!str_cmp(word, "O"))
        {
            if (room == NULL)
            {
                /* Skip this object if we don't have a valid room */
                fread_to_eol(fp);
                continue;
            }
            
            fread_obj(NULL, fp);
            /* fread_obj automatically adds to global list and handles nesting */
            /* We need to find the object that was just loaded to place it */
            OBJ_DATA *obj;
            for (obj = object_list; obj != NULL; obj = obj->next)
            {
                if (obj->item_type == ITEM_TOKEN && obj->in_room == NULL && obj->carried_by == NULL)
                    break; /* Found the newly loaded token */
            }
            if (obj == NULL)
                continue;

            /* Place the token in the room */
            obj_to_room(obj, room);
            token_count++;
            log_string("load_tokens: Loaded token into room");
        }
        else if (!str_cmp(word, "END")) break;
        else
        {
            fread_to_eol(fp);
        }
    }
    fclose(fp);
    log_string("load_tokens: Loaded tokens total");
}
