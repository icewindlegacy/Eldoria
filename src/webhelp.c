/***************************************************************************
 * Web Help System for Eldoria MUD
 * Generates dynamic help browser for web interface
 ***************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"

/* Simple HTML color conversion - doesn't need character context */
void web_html_colour(char *buffer, const char *txt)
{
    const char *point;
    const char *color_tag;
    
    buffer[0] = '\0';
    
    for (point = txt; *point; point++)
    {
        if (*point == '{')
        {
            point++;
            if (*point == '\0')
            {
                point--;
                continue;
            }
            
            /* Map MUD color codes to HTML colors */
            color_tag = NULL;
            switch (*point)
            {
                case 'x': color_tag = NULL; strcat(buffer, "</font>"); continue;
                case 'r': color_tag = "<font color='#800000'>"; break;
                case 'R': color_tag = "<font color='#FF0000'>"; break;
                case 'g': color_tag = "<font color='#008000'>"; break;
                case 'G': color_tag = "<font color='#00FF00'>"; break;
                case 'b': color_tag = "<font color='#000080'>"; break;
                case 'B': color_tag = "<font color='#0000FF'>"; break;
                case 'y': color_tag = "<font color='#808000'>"; break;
                case 'Y': color_tag = "<font color='#FFFF00'>"; break;
                case 'c': color_tag = "<font color='#008080'>"; break;
                case 'C': color_tag = "<font color='#00FFFF'>"; break;
                case 'm': color_tag = "<font color='#800080'>"; break;
                case 'M': color_tag = "<font color='#FF00FF'>"; break;
                case 'w': color_tag = "<font color='#C0C0C0'>"; break;
                case 'W': color_tag = "<font color='#FFFFFF'>"; break;
                case 'D': color_tag = "<font color='#808080'>"; break;
                case '{': strcat(buffer, "{"); continue;
                default: continue;
            }
            
            if (color_tag)
                strcat(buffer, color_tag);
            continue;
        }
        
        /* HTML entity escaping */
        if (*point == '<')
            strcat(buffer, "&lt;");
        else if (*point == '>')
            strcat(buffer, "&gt;");
        else if (*point == '&')
            strcat(buffer, "&amp;");
        else if (*point == '"')
            strcat(buffer, "&quot;");
        else if (*point == '\n')
            strcat(buffer, "<br>");
        else
        {
            /* Append single character */
            size_t len = strlen(buffer);
            buffer[len] = *point;
            buffer[len + 1] = '\0';
        }
    }
}

/* Helper function to escape JSON strings */
void json_escape(char *dest, const char *src, size_t max_len)
{
    size_t i = 0, j = 0;
    
    while (src[i] != '\0' && j < max_len - 2)
    {
        switch (src[i])
        {
            case '"':
                if (j < max_len - 3) {
                    dest[j++] = '\\';
                    dest[j++] = '"';
                }
                break;
            case '\\':
                if (j < max_len - 3) {
                    dest[j++] = '\\';
                    dest[j++] = '\\';
                }
                break;
            case '\n':
                if (j < max_len - 3) {
                    dest[j++] = '\\';
                    dest[j++] = 'n';
                }
                break;
            case '\r':
                /* Skip carriage returns */
                break;
            case '\t':
                if (j < max_len - 3) {
                    dest[j++] = '\\';
                    dest[j++] = 't';
                }
                break;
            default:
                if (src[i] >= 32 || src[i] == '\n')
                    dest[j++] = src[i];
                break;
        }
        i++;
    }
    dest[j] = '\0';
}

/* Generate JSON file with all help data */
void help_json_update(void)
{
    FILE *fp;
    HELP_DATA *pHelp;
    char temp_text[MAX_STRING_LENGTH * 4];
    char json_text[MAX_STRING_LENGTH * 4];
    char json_keyword[MAX_STRING_LENGTH];
    int count = 0;
    bool first = TRUE;
    
    if ((fp = fopen("../../public_html/helps.json", "w")) == NULL)
    {
        log_string("help_json_update: Could not open helps.json for writing");
        return;
    }
    
    fprintf(fp, "{\n");
    fprintf(fp, "  \"updated\": \"%ld\",\n", current_time);
    fprintf(fp, "  \"helps\": [\n");
    
    for (pHelp = help_first; pHelp != NULL; pHelp = pHelp->next)
    {
        /* Skip immortal and old high-level helps (temp fix until helps are updated) */
        if (pHelp->level > 50)
            continue;
            
        if (!first)
            fprintf(fp, ",\n");
        first = FALSE;
        
        /* Convert MUD color codes to HTML */
        web_html_colour(temp_text, pHelp->text);
        
        /* Escape for JSON */
        json_escape(json_text, temp_text, sizeof(json_text));
        json_escape(json_keyword, pHelp->keyword, sizeof(json_keyword));
        
        fprintf(fp, "    {\n");
        fprintf(fp, "      \"id\": %d,\n", count);
        fprintf(fp, "      \"level\": %d,\n", pHelp->level);
        fprintf(fp, "      \"keyword\": \"%s\",\n", json_keyword);
        fprintf(fp, "      \"text\": \"%s\"\n", json_text);
        fprintf(fp, "    }");
        
        count++;
    }
    
    fprintf(fp, "\n  ],\n");
    fprintf(fp, "  \"total\": %d\n", count);
    fprintf(fp, "}\n");
    
    fclose(fp);
    
    log_string("help_json_update: Generated help data for web");
}

/* Get spell/skill type based on mana cost and spell function */
const char *get_spell_type(int min_mana, SPELL_FUN *spell_fun)
{
    /* If it has no spell function, it's a passive skill */
    if (spell_fun == NULL)
        return "skill";
    
    /* If it costs mana, it's a spell */
    if (min_mana > 0)
        return "spell";
    
    /* Has a spell function but no mana cost = active skill */
    return "skill";
}

/* Get target type name */
const char *get_target_name(int target)
{
    switch(target)
    {
        case TAR_IGNORE:         return "ignore";
        case TAR_CHAR_OFFENSIVE: return "offensive";
        case TAR_CHAR_DEFENSIVE: return "defensive";
        case TAR_CHAR_SELF:      return "self";
        case TAR_OBJ_INV:        return "object";
        case TAR_OBJ_CHAR_DEF:   return "object_or_char_defensive";
        case TAR_OBJ_CHAR_OFF:   return "object_or_char_offensive";
        default:                 return "unknown";
    }
}

/* Get position name */
const char *get_position_name(int position)
{
    switch(position)
    {
        case POS_DEAD:     return "dead";
        case POS_MORTAL:   return "mortal";
        case POS_INCAP:    return "incap";
        case POS_STUNNED:  return "stunned";
        case POS_SLEEPING: return "sleeping";
        case POS_RESTING:  return "resting";
        case POS_SITTING:  return "sitting";
        case POS_FIGHTING: return "fighting";
        case POS_STANDING: return "standing";
        default:           return "unknown";
    }
}

/* Generate JSON file with all skills and spells data */
void skills_json_update(void)
{
    FILE *fp;
    int sn, i;
    char json_name[MAX_STRING_LENGTH];
    char json_damage[MAX_STRING_LENGTH];
    char json_msgoff[MAX_STRING_LENGTH];
    bool first = TRUE;
    bool first_class;
    int skill_count = 0;
    int spell_count = 0;
    
    if ((fp = fopen("../../public_html/skills.json", "w")) == NULL)
    {
        log_string("skills_json_update: Could not open skills.json for writing");
        return;
    }
    
    fprintf(fp, "{\n");
    fprintf(fp, "  \"updated\": \"%ld\",\n", current_time);
    fprintf(fp, "  \"skills\": [\n");
    
    for (sn = 0; sn < MAX_SKILL; sn++)
    {
        /* Skip empty/undefined skills */
        if (skill_table[sn].name == NULL || skill_table[sn].name[0] == '\0')
            continue;
            
        /* Determine type and count */
        const char *type = get_spell_type(skill_table[sn].min_mana, skill_table[sn].spell_fun);
        if (!strcmp(type, "spell"))
            spell_count++;
        else
            skill_count++;
            
        if (!first)
            fprintf(fp, ",\n");
        first = FALSE;
        
        /* Escape skill name */
        json_escape(json_name, skill_table[sn].name, sizeof(json_name));
        
        fprintf(fp, "    {\n");
        fprintf(fp, "      \"id\": %d,\n", sn);
        fprintf(fp, "      \"name\": \"%s\",\n", json_name);
        fprintf(fp, "      \"type\": \"%s\",\n", type);
        
        /* Class levels */
        fprintf(fp, "      \"levels\": {\n");
        first_class = TRUE;
        for (i = 0; i < MAX_CLASS; i++)
        {
            if (skill_table[sn].skill_level[i] < LEVEL_HERO)
            {
                if (!first_class)
                    fprintf(fp, ",\n");
                first_class = FALSE;
                fprintf(fp, "        \"%s\": %d",
                        class_table[i].name,
                        skill_table[sn].skill_level[i]);
            }
        }
        fprintf(fp, "\n      },\n");
        
        /* Rating (difficulty) by class */
        fprintf(fp, "      \"rating\": {\n");
        first_class = TRUE;
        for (i = 0; i < MAX_CLASS; i++)
        {
            if (skill_table[sn].skill_level[i] < LEVEL_HERO)
            {
                if (!first_class)
                    fprintf(fp, ",\n");
                first_class = FALSE;
                fprintf(fp, "        \"%s\": %d",
                        class_table[i].name,
                        skill_table[sn].rating[i]);
            }
        }
        fprintf(fp, "\n      },\n");
        
        /* Spell-specific info */
        if (skill_table[sn].spell_fun != NULL)
        {
            fprintf(fp, "      \"target\": \"%s\",\n", get_target_name(skill_table[sn].target));
            fprintf(fp, "      \"position\": \"%s\",\n", get_position_name(skill_table[sn].minimum_position));
            fprintf(fp, "      \"mana\": %d,\n", skill_table[sn].min_mana);
            fprintf(fp, "      \"beats\": %d,\n", skill_table[sn].beats);
            
            /* Damage noun */
            if (skill_table[sn].noun_damage && skill_table[sn].noun_damage[0] != '\0')
            {
                json_escape(json_damage, skill_table[sn].noun_damage, sizeof(json_damage));
                fprintf(fp, "      \"damage_noun\": \"%s\",\n", json_damage);
            }
            
            /* Wear-off message */
            if (skill_table[sn].msg_off && skill_table[sn].msg_off[0] != '\0')
            {
                json_escape(json_msgoff, skill_table[sn].msg_off, sizeof(json_msgoff));
                fprintf(fp, "      \"wear_off\": \"%s\",\n", json_msgoff);
            }
        }
        
        /* Slot number (for objects) */
        fprintf(fp, "      \"slot\": %d\n", skill_table[sn].slot);
        
        fprintf(fp, "    }");
    }
    
    fprintf(fp, "\n  ],\n");
    fprintf(fp, "  \"total\": %d,\n", skill_count + spell_count);
    fprintf(fp, "  \"skill_count\": %d,\n", skill_count);
    fprintf(fp, "  \"spell_count\": %d\n", spell_count);
    fprintf(fp, "}\n");
    
    fclose(fp);
    
    logf2("skills_json_update: Generated %d skills, %d spells for web", skill_count, spell_count);
}

