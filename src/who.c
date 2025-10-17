#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <malloc.h>
#include "merc.h"
#include "recycle.h"
#include "lookup.h"
#include "tables.h"
 
#if !defined(macintosh)
extern  int     _filbuf         args( (FILE *) );
#endif

int html_colour args (( char type, char *string ));
void html_colourconv args (( char *buffer, const char *txt, CHAR_DATA *ch ));

void who_html_update (void)
{
  FILE *fp; 
  DESCRIPTOR_DATA *d;
  char buf[2*MAX_INPUT_LENGTH]; 
  char buf2[2*MAX_INPUT_LENGTH];
  int imm_count = 0;
  int mort_count = 0;
  int total_count = 0;
  
  buf[0] = '\0';
  buf2[0] = '\0';
  
  if ( (fp = file_open("../../public_html/online.html", "w") ) == NULL)
  {
      logf2( "%s",  "Online html doesn't exist.\n\r");
      return;
  }

  /* Count players first */
  for ( d = descriptor_list; d != NULL ; d = d->next )
  {
    CHAR_DATA *wch;
    if ( d->connected != CON_PLAYING)
        continue;
    wch = ( d->original != NULL ) ? d->original : d->character;
    if (wch->invis_level > LEVEL_HERO)
        continue;
    total_count++;
    if (wch->level >= LEVEL_IMMORTAL)
        imm_count++;
    else
        mort_count++;
  }

  /* Write HTML header */
  fprintf(fp, "<!DOCTYPE html>\n<html>\n<head>\n");
  fprintf(fp, "<meta charset=\"UTF-8\">\n");
  fprintf(fp, "<title>Who's Online - Eldoria MUD</title>\n");
  fprintf(fp, "<style>\n");
  fprintf(fp, "    body {\n");
  fprintf(fp, "        background: #000;\n");
  fprintf(fp, "        color: #0f0;\n");
  fprintf(fp, "        font-family: 'Courier New', 'Lucida Console', monospace;\n");
  fprintf(fp, "        margin: 20px;\n");
  fprintf(fp, "        padding: 0;\n");
  fprintf(fp, "        font-size: 14px;\n");
  fprintf(fp, "        line-height: 1.4;\n");
  fprintf(fp, "    }\n");
  fprintf(fp, "    .container {\n");
  fprintf(fp, "        max-width: 900px;\n");
  fprintf(fp, "        margin: 0 auto;\n");
  fprintf(fp, "    }\n");
  fprintf(fp, "    .logo {\n");
  fprintf(fp, "        color: #00ffff;\n");
  fprintf(fp, "        text-align: center;\n");
  fprintf(fp, "        font-weight: bold;\n");
  fprintf(fp, "        margin-bottom: 20px;\n");
  fprintf(fp, "        text-shadow: 0 0 10px #00ffff;\n");
  fprintf(fp, "        white-space: pre;\n");
  fprintf(fp, "        font-size: 12px;\n");
  fprintf(fp, "    }\n");
  fprintf(fp, "    .section-header {\n");
  fprintf(fp, "        color: #ffff00;\n");
  fprintf(fp, "        font-weight: bold;\n");
  fprintf(fp, "        margin: 20px 0 10px 0;\n");
  fprintf(fp, "        text-shadow: 0 0 5px #ffff00;\n");
  fprintf(fp, "    }\n");
  fprintf(fp, "    .player-line {\n");
  fprintf(fp, "        margin: 5px 0;\n");
  fprintf(fp, "        padding: 5px;\n");
  fprintf(fp, "        border-left: 2px solid #00ff00;\n");
  fprintf(fp, "        padding-left: 10px;\n");
  fprintf(fp, "    }\n");
  fprintf(fp, "    .player-line:hover {\n");
  fprintf(fp, "        background: rgba(0, 255, 0, 0.1);\n");
  fprintf(fp, "    }\n");
  fprintf(fp, "    .bracket {\n");
  fprintf(fp, "        color: #ff00ff;\n");
  fprintf(fp, "    }\n");
  fprintf(fp, "    .level {\n");
  fprintf(fp, "        color: #fff;\n");
  fprintf(fp, "        font-weight: bold;\n");
  fprintf(fp, "    }\n");
  fprintf(fp, "    .race {\n");
  fprintf(fp, "        color: #00aaff;\n");
  fprintf(fp, "    }\n");
  fprintf(fp, "    .class {\n");
  fprintf(fp, "        color: #00ffff;\n");
  fprintf(fp, "    }\n");
  fprintf(fp, "    .clan {\n");
  fprintf(fp, "        color: #ffaa00;\n");
  fprintf(fp, "    }\n");
  fprintf(fp, "    .name {\n");
  fprintf(fp, "        color: #fff;\n");
  fprintf(fp, "        font-weight: bold;\n");
  fprintf(fp, "    }\n");
  fprintf(fp, "    .stats {\n");
  fprintf(fp, "        color: #00ffff;\n");
  fprintf(fp, "        text-align: center;\n");
  fprintf(fp, "        margin-top: 20px;\n");
  fprintf(fp, "        padding: 10px;\n");
  fprintf(fp, "        border: 1px solid #00ffff;\n");
  fprintf(fp, "        border-radius: 5px;\n");
  fprintf(fp, "    }\n");
  fprintf(fp, "    .update-time {\n");
  fprintf(fp, "        color: #888;\n");
  fprintf(fp, "        text-align: center;\n");
  fprintf(fp, "        margin-top: 10px;\n");
  fprintf(fp, "        font-size: 12px;\n");
  fprintf(fp, "    }\n");
  fprintf(fp, "    .imm-title {\n");
  fprintf(fp, "        color: #ff00ff;\n");
  fprintf(fp, "        font-weight: bold;\n");
  fprintf(fp, "    }\n");
  fprintf(fp, "</style>\n");
  fprintf(fp, "</head>\n<body>\n");
  fprintf(fp, "<div class=\"container\">\n");

  /* ASCII Art Logo */
  fprintf(fp, "<div class=\"logo\">\n");
  fprintf(fp, "___________.__       .___           .__         \n");
  fprintf(fp, "\\_   _____/|  |    __| _/___________|__|____  \n");
  fprintf(fp, " |    __)_ |  |   / __ |/  _ \\_  __ \\  \\__  \\  \n");
  fprintf(fp, " |        \\|  |__/ /_/ (  &lt;_&gt; )  | \\/  |/ __ \\_ \n");
  fprintf(fp, "/_______  /|____/\\____ |\\____/|__|  |__(____  / \n");
  fprintf(fp, "        \\/            \\/                    \\/  \n");
  fprintf(fp, "                   MUD\n");
  fprintf(fp, "</div>\n");

  /* Immortals Section */
  if (imm_count > 0)
  {
    fprintf(fp, "<div class=\"section-header\">Immortals Online:</div>\n");
    
    for ( d = descriptor_list; d != NULL ; d = d->next )
    {
      CHAR_DATA *wch;
      const char *klass;
      
      if ( d->connected != CON_PLAYING)
          continue;
      wch = ( d->original != NULL ) ? d->original : d->character;
      
      if (wch->invis_level > LEVEL_HERO || wch->level < LEVEL_IMMORTAL)
          continue;
      
      /* Determine immortal rank */
      klass = class_who(wch);
      switch(wch->level)
      {
          case MAX_LEVEL - 0 : klass = "IMP"; break;
          case MAX_LEVEL - 1 : klass = "CRE"; break;
          case MAX_LEVEL - 2 : klass = "SUP"; break;
          case MAX_LEVEL - 3 : klass = "DEI"; break;
          case MAX_LEVEL - 4 : klass = "GOD"; break;
          case MAX_LEVEL - 5 : klass = "IMM"; break;
          case MAX_LEVEL - 6 : klass = "DEM"; break;
          case MAX_LEVEL - 7 : klass = "ANG"; break;
          case MAX_LEVEL - 8 : klass = "AVA"; break;
      }
      
      fprintf(fp, "<div class=\"player-line\">");
      fprintf(fp, "<span class=\"bracket\">[</span>");
      
      if (wch->level >= LEVEL_IMMORTAL && wch->pcdata->immtitle != NULL && wch->pcdata->immtitle[0] != '\0')
      {
          buf2[0] = '\0';
          html_colourconv( buf, wch->pcdata->immtitle, wch );
          fprintf(fp, "<span class=\"imm-title\">%s</span>", buf);
      }
      else
      {
          fprintf(fp, "<span class=\"level\">%3d</span> ", wch->level);
          fprintf(fp, "<span class=\"race\">%6s</span> ", 
                  wch->race < MAX_PC_RACE ? pc_race_table[wch->race].who_name : "      ");
          fprintf(fp, "<span class=\"class\">%s</span>", klass);
      }
      
      fprintf(fp, "<span class=\"bracket\">]</span> ");
      
      if (clan_table[wch->clan].who_name[0] != '\0')
          fprintf(fp, "<span class=\"clan\">%s</span> ", clan_table[wch->clan].who_name);
      
      if (IS_SET(wch->comm, COMM_AFK))
          fprintf(fp, "<span style=\"color:#ffff00\">[AFK]</span> ");
      
      fprintf(fp, "<span class=\"name\">%s</span>", wch->name);
      
      buf2[0] = '\0';
      sprintf(buf2, "%s", (IS_NPC(wch) ? "" : wch->pcdata->title));
      html_colourconv( buf, buf2, wch );
      fprintf(fp, "%s", buf);
      
      fprintf(fp, "</div>\n");
    }
  }

  /* Mortals Section */
  if (mort_count > 0)
  {
    fprintf(fp, "\n<div class=\"section-header\">Mortals Online:</div>\n");
    
    for ( d = descriptor_list; d != NULL ; d = d->next )
    {
      CHAR_DATA *wch;
      const char *klass;
      
      if ( d->connected != CON_PLAYING)
          continue;
      wch = ( d->original != NULL ) ? d->original : d->character;
      
      if (wch->invis_level > LEVEL_HERO || wch->level >= LEVEL_IMMORTAL)
          continue;
      
      klass = class_who(wch);
      
      fprintf(fp, "<div class=\"player-line\">");
      fprintf(fp, "<span class=\"bracket\">[</span>");
      fprintf(fp, "<span class=\"level\">%3d</span> ", wch->level);
      fprintf(fp, "<span class=\"race\">%6s</span> ", 
              wch->race < MAX_PC_RACE ? pc_race_table[wch->race].who_name : "      ");
      fprintf(fp, "<span class=\"class\">%s</span>", klass);
      fprintf(fp, "<span class=\"bracket\">]</span> ");
      
      if (clan_table[wch->clan].who_name[0] != '\0')
          fprintf(fp, "<span class=\"clan\">%s</span> ", clan_table[wch->clan].who_name);
      
      if (IS_SET(wch->comm, COMM_AFK))
          fprintf(fp, "<span style=\"color:#ffff00\">[AFK]</span> ");
      
      fprintf(fp, "<span class=\"name\">%s</span>", wch->name);
      
      buf2[0] = '\0';
      sprintf(buf2, "%s", (IS_NPC(wch) ? "" : wch->pcdata->title));
      html_colourconv( buf, buf2, wch );
      fprintf(fp, "%s", buf);
      
      fprintf(fp, "</div>\n");
    }
  }

  /* Stats footer */
  fprintf(fp, "\n<div class=\"stats\">\n");
  fprintf(fp, "Players found: <span style=\"color:#fff\">%d</span>", total_count);
  if (imm_count > 0)
      fprintf(fp, " &nbsp;|&nbsp; Immortals: <span style=\"color:#ff00ff\">%d</span>", imm_count);
  if (mort_count > 0)
      fprintf(fp, " &nbsp;|&nbsp; Mortals: <span style=\"color:#00ff00\">%d</span>", mort_count);
  fprintf(fp, "\n</div>\n");

  /* Update time */
  sprintf(buf, "%s", ctime( &current_time ));
  buf[strlen(buf)-1] = '\0'; /* Remove newline */
  fprintf(fp, "<div class=\"update-time\">Last updated: %s</div>\n", buf);

  fprintf(fp, "</div>\n</body>\n</html>\n");
  file_close( fp ); 
  
  return;
}
/* end function */

int html_colour( char type, char *string )
{
    char	code[ 25 ];
    char	*p = NULL;

        
    switch( type )
    {
	default:
	case '\0':
	    code[0] = '\0';
	    break;
	case ' ':
	    sprintf( code, " " );
	    break;
	case 'x':
	    sprintf( code, "<font color=""#006400"">" );
	    break;
	case 'b':
	    sprintf( code, "<font color=""#00008B"">" );
	    break;
	case 'c':
	    sprintf( code, "<font color=""#008B8B"">" );
	    break;
	case 'g':
	    sprintf( code, "<font color=""#006400"">" );
	    break;
	case 'm':
	    sprintf( code, "<font color=""#8B008B"">" );
	    break;
	case 'r':
	    sprintf( code, "<font color=""#8B0000"">" );
	    break;
	case 'w':
	    sprintf( code, "<font color=""#808080"">" );
	    break;
	case 'y':
	    sprintf( code, "<font color=""#808000"">" );
	    break;
	case 'B':
	    sprintf( code, "<font color=""#0000FF"">" );
	    break;
	case 'C':
	    sprintf( code, "<font color=""#OOFFFF"">" );
	    break;
	case 'G':
	    sprintf( code, "<font color=""#00FF00"">" );
	    break;
	case 'M':
	    sprintf( code, "<font color=""#FF00FF"">" );
	    break;
	case 'R':
	    sprintf( code, "<font color=""#FF0000"">" );
	    break;
	case 'W':
	    sprintf( code, "<font color=""#FFFFFF"">" );
	    break;
	case 'Y':
	    sprintf( code, "<font color=""#FFFF00"">" );
	    break;
	case 'D':
	    sprintf( code, "<font color=""#636363"">" );
	    break;
	case '{':
	    sprintf( code, "{" );
	    break;
    }

    p = code;
    while( *p != '\0' )
    {
	*string = *p++;
	*++string = '\0';
    }

    return( strlen( code ) );
}

void html_colourconv( char *buffer, const char *txt, CHAR_DATA *ch )
{
    const	char	*point;
		int	skip = 0;

    for( point = txt ; *point ; point++ )
    {
	if( *point == '{' )
	{
	    point++;
	    if( *point == '\0' )
		point--;
	    else
	      skip = html_colour( *point, buffer );
	    while( skip-- > 0 )
		++buffer;
	    continue;
	}
	/* Following is put in to prevent adding HTML links to titles,
	   except for IMMS who know what they're doing and can be
	   punished if they screw it up! */
	if( (*point == '<') && (!IS_IMMORTAL(ch)) )
	{
	    *buffer = '[';
	    *++buffer = '\0';
	    continue;
	}
	if( (*point == '>') && (!IS_IMMORTAL(ch)) )
	{
	    *buffer = ']';
	    *++buffer = '\0';
	    continue;
	}
	*buffer = *point;
	*++buffer = '\0';
    }			
    *buffer = '\0';
    return;
}


                                       
