/* Web Room Explorer for Eldoria MUD
 * Creates HTML files for virtual room exploration via web browser
 * Features: Compass rose navigation, room descriptions, exits
 */

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "merc.h"

/* HTML color conversion functions from who.c */
extern int html_colour(char type, char *string);
extern void html_colourconv(char *buffer, const char *txt, CHAR_DATA *ch);

void do_webroom(CHAR_DATA *ch, char *argument)
{
    FILE *fp;
    ROOM_INDEX_DATA *location;
    EXIT_DATA *pExit;
    sh_int door;
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char desc_buf[MAX_STRING_LENGTH * 2];
    int vnum;
    int end_vnum;
    int count = 0;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0')
    {
        send_to_char("Syntax:  webroom <starting vnum> [ending vnum]\n\r", ch);
        send_to_char("         webroom all - generates all rooms in current area\n\r", ch);
        return;
    }

    /* Handle "all" keyword for current area */
    if (!str_cmp(arg1, "all"))
    {
        if (!ch->in_room || !ch->in_room->area)
        {
            send_to_char("You must be in a valid area.\n\r", ch);
            return;
        }
        vnum = ch->in_room->area->min_vnum;
        end_vnum = ch->in_room->area->max_vnum;
        printf_to_char(ch, "Generating web rooms for area: %s [%d-%d]\n\r",
            ch->in_room->area->name, vnum, end_vnum);
    }
    else
    {
        vnum = atoi(arg1);
        
        if (arg2[0] == '\0')
            end_vnum = vnum;
        else
        {
            end_vnum = atoi(arg2);
            if (end_vnum < vnum || end_vnum - vnum > 999)
            {
                send_to_char("You may only write 1000 rooms at a time.\n\r", ch);
                return;
            }
        }
    }

    /* Create the explore directory if needed */
    sprintf(buf, "mkdir -p ../../public_html/explore");
    system(buf);

    for (; vnum <= end_vnum; vnum++)
    {
        sprintf(buf, "%d", vnum);
        if ((location = get_room_index(vnum)) == NULL)
            continue;

        count++;
        sprintf(buf, "../../public_html/explore/%d.html", vnum);
        
        if ((fp = file_open(buf, "w")) == NULL)
        {
            bug("webroom: could not open file for writing", 0);
            continue;
        }

        /* HTML Header */
        fprintf(fp, "<!DOCTYPE html>\n<html>\n<head>\n");
        fprintf(fp, "<meta charset=\"UTF-8\">\n");
        fprintf(fp, "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n");
        fprintf(fp, "<title>%s - Eldoria MUD</title>\n", location->name);
        
        /* CSS Styling */
        fprintf(fp, "<style>\n");
        fprintf(fp, "body {\n");
        fprintf(fp, "    background: #000;\n");
        fprintf(fp, "    color: #0f0;\n");
        fprintf(fp, "    font-family: 'Courier New', monospace;\n");
        fprintf(fp, "    margin: 0;\n");
        fprintf(fp, "    padding: 20px;\n");
        fprintf(fp, "    font-size: 14px;\n");
        fprintf(fp, "}\n");
        fprintf(fp, ".container {\n");
        fprintf(fp, "    max-width: 900px;\n");
        fprintf(fp, "    margin: 0 auto;\n");
        fprintf(fp, "    background: rgba(0, 20, 0, 0.8);\n");
        fprintf(fp, "    border: 2px solid #0f0;\n");
        fprintf(fp, "    padding: 20px;\n");
        fprintf(fp, "    box-shadow: 0 0 20px #0f0;\n");
        fprintf(fp, "}\n");
        fprintf(fp, ".room-name {\n");
        fprintf(fp, "    color: #00ffff;\n");
        fprintf(fp, "    font-size: 24px;\n");
        fprintf(fp, "    font-weight: bold;\n");
        fprintf(fp, "    text-align: center;\n");
        fprintf(fp, "    margin-bottom: 20px;\n");
        fprintf(fp, "    text-shadow: 0 0 10px #00ffff;\n");
        fprintf(fp, "}\n");
        fprintf(fp, ".room-desc {\n");
        fprintf(fp, "    color: #fff;\n");
        fprintf(fp, "    line-height: 1.6;\n");
        fprintf(fp, "    margin: 20px 0;\n");
        fprintf(fp, "    padding: 15px;\n");
        fprintf(fp, "    background: rgba(0, 0, 0, 0.5);\n");
        fprintf(fp, "    border-left: 3px solid #00ffff;\n");
        fprintf(fp, "}\n");
        fprintf(fp, ".compass {\n");
        fprintf(fp, "    display: grid;\n");
        fprintf(fp, "    grid-template-columns: 80px 80px 80px;\n");
        fprintf(fp, "    grid-template-rows: 80px 80px 80px;\n");
        fprintf(fp, "    gap: 5px;\n");
        fprintf(fp, "    margin: 30px auto;\n");
        fprintf(fp, "    width: 255px;\n");
        fprintf(fp, "}\n");
        fprintf(fp, ".exit-btn {\n");
        fprintf(fp, "    background: linear-gradient(145deg, #003300, #006600);\n");
        fprintf(fp, "    border: 2px solid #0f0;\n");
        fprintf(fp, "    color: #0f0;\n");
        fprintf(fp, "    text-decoration: none;\n");
        fprintf(fp, "    display: flex;\n");
        fprintf(fp, "    align-items: center;\n");
        fprintf(fp, "    justify-content: center;\n");
        fprintf(fp, "    font-weight: bold;\n");
        fprintf(fp, "    cursor: pointer;\n");
        fprintf(fp, "    transition: all 0.3s;\n");
        fprintf(fp, "    font-size: 12px;\n");
        fprintf(fp, "    text-shadow: 0 0 5px #0f0;\n");
        fprintf(fp, "}\n");
        fprintf(fp, ".exit-btn:hover {\n");
        fprintf(fp, "    background: linear-gradient(145deg, #006600, #009900);\n");
        fprintf(fp, "    box-shadow: 0 0 15px #0f0;\n");
        fprintf(fp, "    transform: scale(1.05);\n");
        fprintf(fp, "}\n");
        fprintf(fp, ".exit-btn.disabled {\n");
        fprintf(fp, "    background: #111;\n");
        fprintf(fp, "    border-color: #333;\n");
        fprintf(fp, "    color: #333;\n");
        fprintf(fp, "    cursor: not-allowed;\n");
        fprintf(fp, "    text-shadow: none;\n");
        fprintf(fp, "}\n");
        fprintf(fp, ".center-cell {\n");
        fprintf(fp, "    background: radial-gradient(circle, #003366, #001133);\n");
        fprintf(fp, "    border: 2px solid #00ffff;\n");
        fprintf(fp, "    color: #00ffff;\n");
        fprintf(fp, "    display: flex;\n");
        fprintf(fp, "    flex-direction: column;\n");
        fprintf(fp, "    align-items: center;\n");
        fprintf(fp, "    justify-content: center;\n");
        fprintf(fp, "    font-size: 10px;\n");
        fprintf(fp, "}\n");
        fprintf(fp, ".info-bar {\n");
        fprintf(fp, "    color: #ffff00;\n");
        fprintf(fp, "    text-align: center;\n");
        fprintf(fp, "    margin: 10px 0;\n");
        fprintf(fp, "    font-size: 12px;\n");
        fprintf(fp, "}\n");
        fprintf(fp, ".back-link {\n");
        fprintf(fp, "    text-align: center;\n");
        fprintf(fp, "    margin-top: 20px;\n");
        fprintf(fp, "}\n");
        fprintf(fp, ".back-link a {\n");
        fprintf(fp, "    color: #00ffff;\n");
        fprintf(fp, "    text-decoration: none;\n");
        fprintf(fp, "    padding: 10px 20px;\n");
        fprintf(fp, "    border: 1px solid #00ffff;\n");
        fprintf(fp, "    display: inline-block;\n");
        fprintf(fp, "}\n");
        fprintf(fp, ".back-link a:hover {\n");
        fprintf(fp, "    background: rgba(0, 255, 255, 0.1);\n");
        fprintf(fp, "    box-shadow: 0 0 10px #00ffff;\n");
        fprintf(fp, "}\n");
        fprintf(fp, "</style>\n");
        fprintf(fp, "</head>\n<body>\n");
        
        fprintf(fp, "<div class=\"container\">\n");
        
        /* Room name */
        fprintf(fp, "<div class=\"room-name\">%s</div>\n", location->name);
        
        /* Room info bar */
        fprintf(fp, "<div class=\"info-bar\">");
        fprintf(fp, "Area: %s | Vnum: [%d] | Sector: %s",
            location->area ? location->area->name : "Unknown",
            location->vnum,
            location->sector_type >= 0 && location->sector_type < SECT_MAX 
                ? sector_flags[location->sector_type].name 
                : "unknown");
        fprintf(fp, "</div>\n");
        
        /* Room description */
        fprintf(fp, "<div class=\"room-desc\">\n");
        html_colourconv(desc_buf, location->description, ch);
        fprintf(fp, "%s", desc_buf);
        fprintf(fp, "</div>\n");
        
        /* Compass Rose Navigation */
        fprintf(fp, "<div class=\"compass\">\n");
        
        /* Row 1: NW, N, NE (or UP) */
        for (door = 0; door < 3; door++)
        {
            int actual_door = (door == 0) ? DIR_NORTHWEST : 
                             (door == 1) ? DIR_NORTH : 
                             DIR_UP;
            
            if (actual_door == DIR_NORTHWEST || actual_door == DIR_NORTHEAST)
                continue; /* Skip diagonals for standard ROM */
                
            pExit = location->exit[actual_door];
            
            if (door == 0) /* NW position - show UP if available */
            {
                pExit = location->exit[DIR_UP];
                if (pExit && pExit->u1.to_room)
                    fprintf(fp, "<a href=\"%d.html\" class=\"exit-btn\">⬆️<br>UP</a>\n",
                        pExit->u1.to_room->vnum);
                else
                    fprintf(fp, "<div class=\"exit-btn disabled\">⬆️<br>UP</div>\n");
            }
            else if (door == 1) /* N position */
            {
                pExit = location->exit[DIR_NORTH];
                if (pExit && pExit->u1.to_room)
                    fprintf(fp, "<a href=\"%d.html\" class=\"exit-btn\">⬆️<br>NORTH</a>\n",
                        pExit->u1.to_room->vnum);
                else
                    fprintf(fp, "<div class=\"exit-btn disabled\">⬆️<br>NORTH</div>\n");
            }
            else /* NE position - empty */
            {
                fprintf(fp, "<div class=\"exit-btn disabled\"></div>\n");
            }
        }
        
        /* Row 2: W, CENTER, E */
        pExit = location->exit[DIR_WEST];
        if (pExit && pExit->u1.to_room)
            fprintf(fp, "<a href=\"%d.html\" class=\"exit-btn\">⬅️<br>WEST</a>\n",
                pExit->u1.to_room->vnum);
        else
            fprintf(fp, "<div class=\"exit-btn disabled\">⬅️<br>WEST</div>\n");
        
        /* Center - current room indicator */
        fprintf(fp, "<div class=\"center-cell\"><div>📍</div><div>YOU ARE<br>HERE</div></div>\n");
        
        pExit = location->exit[DIR_EAST];
        if (pExit && pExit->u1.to_room)
            fprintf(fp, "<a href=\"%d.html\" class=\"exit-btn\">➡️<br>EAST</a>\n",
                pExit->u1.to_room->vnum);
        else
            fprintf(fp, "<div class=\"exit-btn disabled\">➡️<br>EAST</div>\n");
        
        /* Row 3: SW (or DOWN), S, SE */
        pExit = location->exit[DIR_DOWN];
        if (pExit && pExit->u1.to_room)
            fprintf(fp, "<a href=\"%d.html\" class=\"exit-btn\">⬇️<br>DOWN</a>\n",
                pExit->u1.to_room->vnum);
        else
            fprintf(fp, "<div class=\"exit-btn disabled\">⬇️<br>DOWN</div>\n");
        
        pExit = location->exit[DIR_SOUTH];
        if (pExit && pExit->u1.to_room)
            fprintf(fp, "<a href=\"%d.html\" class=\"exit-btn\">⬇️<br>SOUTH</a>\n",
                pExit->u1.to_room->vnum);
        else
            fprintf(fp, "<div class=\"exit-btn disabled\">⬇️<br>SOUTH</div>\n");
        
        /* SE position - empty */
        fprintf(fp, "<div class=\"exit-btn disabled\"></div>\n");
        
        fprintf(fp, "</div>\n"); /* End compass */
        
        /* Exit list in text form */
        fprintf(fp, "<div style=\"text-align: center; margin: 20px 0; color: #ffff00;\">\n");
        fprintf(fp, "<strong>Obvious Exits:</strong> ");
        
        bool found_exit = FALSE;
        for (door = 0; door < MAX_DIR; door++)
        {
            if ((pExit = location->exit[door]) != NULL && pExit->u1.to_room != NULL)
            {
                if (found_exit)
                    fprintf(fp, ", ");
                fprintf(fp, "<a href=\"%d.html\" style=\"color: #00ff00;\">%s</a>",
                    pExit->u1.to_room->vnum, 
                    capitalize(dir_name[door]));
                found_exit = TRUE;
            }
        }
        
        if (!found_exit)
            fprintf(fp, "<span style=\"color: #888;\">None</span>");
        
        fprintf(fp, "</div>\n");
        
        /* Back to main page link */
        fprintf(fp, "<div class=\"back-link\">\n");
        fprintf(fp, "<a href=\"../index.html#explore\">🏠 Return to Main Page</a>\n");
        fprintf(fp, "</div>\n");
        
        fprintf(fp, "</div>\n"); /* End container */
        fprintf(fp, "</body>\n</html>\n");
        
        file_close(fp);
    }

    printf_to_char(ch, "Generated %d web room%s.\n\r", 
        count, count == 1 ? "" : "s");
    return;
}

