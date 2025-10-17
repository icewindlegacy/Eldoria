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
        /* Skip high-level immortal helps */
        if (pHelp->level > LEVEL_HERO)
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

