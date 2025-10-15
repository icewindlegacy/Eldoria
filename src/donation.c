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
#include "recycle.h"
#include "lookup.h"
#include "tables.h"

#define DONATION_FILE "donation.dat"
#define TOKENS_FILE "tokens.dat"

void save_donation_pits(void);
void load_donation_pits(void);
void save_tokens(void);
void load_tokens(void);
/* from save.c */
void     fwrite_obj_donation(OBJ_DATA *obj, FILE *fp, int iNest);
OBJ_DATA *fread_obj_donation(FILE *fp);


void save_donation_pits(void)
{
    FILE *fp;
    ROOM_INDEX_DATA *room;
    OBJ_DATA *obj;
    int vnum;

    if ((fp = fopen(DONATION_FILE, "w")) == NULL)
    { bug("save_donation_pits: fopen", 0); return; }

    for (vnum = 0; vnum <= top_vnum_room; vnum++)
    {
        room = get_room_index(vnum);
        if (!room) continue;

#ifdef ROOM_DONATION
        if (IS_SET(room->room_flags, ROOM_DONATION))
        {
            for (obj = room->contents; obj; obj = obj->next_content)
                fwrite_obj_donation(obj, fp, 0);
        }
#endif

#ifdef OBJ_VNUM_PIT
        for (pit = room->contents; pit; pit = pit->next_content)
            if (pit->pIndexData->vnum == OBJ_VNUM_PIT)
                for (obj = pit->contains; obj; obj = obj->next_content)
                    fwrite_obj_donation(obj, fp, 0);
#endif
    }

    fprintf(fp, "#END\n");
    fclose(fp);
}

void load_donation_pits(void)
{
    FILE *fp;
    if ((fp = fopen(DONATION_FILE, "r")) == NULL) return;

    for (;;)
    {
        char letter = fread_letter(fp);
        if (letter == '*') { fread_to_eol(fp); continue; }
        if (letter != '#') break;

        char *word = fread_word(fp);
        if (!str_cmp(word, "O"))
{
    OBJ_DATA *obj = fread_obj_donation(fp);
    if (obj == NULL)
        continue; /* child already linked to parent */

    /* make sure the object is in the global list */
    obj->next = object_list;
    object_list = obj;

#ifdef OBJ_VNUM_PIT
    /* Place only top-level objs into the first pit/room you choose */
    {
        int vnum;
        for (vnum = 0; vnum <= top_vnum_room; vnum++)
        {
            ROOM_INDEX_DATA *room = get_room_index(vnum);
            if (!room) continue;
            OBJ_DATA *pit;
            for (pit = room->contents; pit; pit = pit->next_content)
                if (pit->pIndexData->vnum == OBJ_VNUM_PIT)
                { obj_to_obj(obj, pit); goto placed; }
        }
    }
placed: ;
#endif


#ifdef ROOM_DONATION
            if (obj->in_obj == NULL) /* if not already nested under a container in the room */
            {
                int vnum;
                for (vnum = 0; vnum <= top_vnum_room; vnum++)
                {
                    ROOM_INDEX_DATA *room = get_room_index(vnum);
                    if (room && IS_SET(room->room_flags, ROOM_DONATION))
                    { obj_to_room(obj, room); break; }
                }
            }
#endif
        }
        else if (!str_cmp(word, "END")) break;
    }
    fclose(fp);
}

void save_tokens(void)
{
    FILE *fp;
    ROOM_INDEX_DATA *room;
    OBJ_DATA *obj;
    int vnum;
    int token_count = 0;

    if ((fp = fopen(TOKENS_FILE, "w")) == NULL)
    { bug("save_tokens: fopen", 0); return; }
    
    log_string("save_tokens: Starting to save tokens to tokens.dat");

    for (vnum = 0; vnum <= top_vnum_room; vnum++)
    {
        room = get_room_index(vnum);
        if (!room) continue;

        for (obj = room->contents; obj; obj = obj->next_content)
        {
            if (obj->item_type == ITEM_TOKEN)
            {
                fprintf(fp, "Room %d\n", room->vnum);
                fwrite_obj_donation(obj, fp, 0);
                token_count++;
                log_string("save_tokens: Saved token from room");
            }
        }
    }

    fprintf(fp, "#END\n");
    fclose(fp);
    log_string("save_tokens: Saved tokens total");
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
            
            obj = fread_obj_donation(fp);
            if (obj == NULL)
                continue; /* child already linked to parent */

            /* make sure the object is in the global list */
            obj->next = object_list;
            object_list = obj;

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
