/***************************************************************************
 *  File: string.c                                                         *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/


#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "tables.h"
#include "olc.h"
#include "const.h"

char *string_linedel( char *, int );
char *string_lineadd( char *, char *, int );
char *numlineas( char * );

/*****************************************************************************
 Name:		string_edit
 Purpose:	Clears string and puts player into editing mode.
 Called by:	none
 ****************************************************************************/
void string_edit( CHAR_DATA *ch, char **pString )
{
    send_to_char( "-========- Entering EDIT Mode -=========-\n\r", ch );
    send_to_char( "    Type .h on a new line for help\n\r", ch );
    send_to_char( " Terminate with a ~ or @ on a blank line.\n\r", ch );
    send_to_char( "-=======================================-\n\r", ch );

    if ( *pString == NULL )
    {
        *pString = str_dup( "" );
    }
    else
    {
        **pString = '\0';
    }

    ch->desc->pString = pString;

    return;
}



/*****************************************************************************
 Name:		string_append
 Purpose:	Puts player into append mode for given string.
 Called by:	(many)olc_act.c
 ****************************************************************************/
void string_append( CHAR_DATA *ch, char **pString )
{
    send_to_char( "{b-=======- {WEntering APPEND Mode {b-========-{x\n\r", ch );
    send_to_char( "    {wType {C.h{w on a new line for help{x\n\r", ch );
    send_to_char( " {wTerminate with a {C~{w or {C@{w on a blank line.{x\n\r", ch );
    send_to_char( "{b-=======================================-{x\n\r", ch );

    if ( *pString == NULL )
    {
        *pString = str_dup( "" );
    }
    send_to_char( numlineas(*pString), ch );

/* numlineas entrega el string con \n\r */
/*  if ( *(*pString + strlen( *pString ) - 1) != '\r' )
	send_to_char( "\n\r", ch ); */

    ch->desc->pString = pString;

    return;
}



/*****************************************************************************
 Name:		string_replace
 Purpose:	Substitutes one string for another.
 Called by:	string_add(string.c) (aedit_builder)olc_act.c.
 ****************************************************************************/
char * string_replace( char * orig, char * old, char * knew )
{
    char xbuf[MAX_STRING_LENGTH];
    int i;

    xbuf[0] = '\0';
    strcpy( xbuf, orig );
    if ( strstr( orig, old ) != NULL )
    {
        i = strlen( orig ) - strlen( strstr( orig, old ) );
        xbuf[i] = '\0';
        strcat( xbuf, knew );
        strcat( xbuf, &orig[i+strlen( old )] );
        free_string( orig );
    }

    return str_dup( xbuf );
}



/*****************************************************************************
 Name:		string_add
 Purpose:	Interpreter for string editing.
 Called by:	game_loop_xxxx(comm.c).
 ****************************************************************************/
extern const char *szFinishPrompt;

void string_add( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    /*
     * Thanks to James Seng
     */
    smash_tilde( argument );

    if ( *argument == '.' )
    {
        char arg1 [MAX_INPUT_LENGTH];
        char arg2 [MAX_INPUT_LENGTH];
        char arg3 [MAX_INPUT_LENGTH];
        char tmparg3 [MAX_INPUT_LENGTH];

        argument = one_argument( argument, arg1 );
        argument = first_arg( argument, arg2, FALSE );
	strcpy( tmparg3, argument );
        argument = first_arg( argument, arg3, FALSE );

        if ( !str_cmp( arg1, ".c" ) )
        {
            send_to_char( "String cleared.\n\r", ch );
	    free_string(*ch->desc->pString);
	    *ch->desc->pString = str_dup( "" );
            return;
        }

        if ( !str_cmp( arg1, ".s" ) )
        {
            send_to_char( "String so far:\n\r", ch );
            send_to_char( numlineas(*ch->desc->pString), ch );
            return;
        }

        if ( !str_cmp( arg1, ".r" ) )
        {
            if ( arg2[0] == '\0' )
            {
                send_to_char(
                    "usage:  .r \"old string\" \"new string\"\n\r", ch );
                return;
            }

            *ch->desc->pString =
                string_replace( *ch->desc->pString, arg2, arg3 );
            sprintf( buf, "'%s' replaced with '%s'.\n\r", arg2, arg3 );
            send_to_char( buf, ch );
            return;
        }

        if ( !str_cmp( arg1, ".f" ) )
        {
            *ch->desc->pString = format_string( *ch->desc->pString );
            send_to_char( "String formatted.\n\r", ch );
            return;
        }
        
	if ( !str_cmp( arg1, ".ld" ) )
	{
		*ch->desc->pString = string_linedel( *ch->desc->pString, atoi(arg2) );
		send_to_char( "Line deleted.\n\r", ch );
		return;
	}

	if ( !str_cmp( arg1, ".li" ) )
	{
		*ch->desc->pString = string_lineadd( *ch->desc->pString, tmparg3, atoi(arg2) );
		send_to_char( "Line inserted.\n\r", ch );
		return;
	}

	if ( !str_cmp( arg1, ".lr" ) )
	{
		*ch->desc->pString = string_linedel( *ch->desc->pString, atoi(arg2) );
		*ch->desc->pString = string_lineadd( *ch->desc->pString, tmparg3, atoi(arg2) );
		send_to_char( "Linea replaced.\n\r", ch );
		return;
	}

        if ( !str_cmp( arg1, ".h" ) )
        {
            send_to_char( "Sedit help (commands on blank line):   \n\r", ch );
            send_to_char( ".r 'old' 'new'   - replace a substring \n\r", ch );
            send_to_char( "                   (requires '', \"\") \n\r", ch );
            send_to_char( ".h               - get help (this info)\n\r", ch );
            send_to_char( ".s               - show string so far  \n\r", ch );
            send_to_char( ".f               - (word wrap) string  \n\r", ch );
            send_to_char( ".c               - clear string so far \n\r", ch );
            send_to_char( ".ld <num>        - delete line number <num>\n\r", ch );
            send_to_char( ".li <num> <str>  - line <str> insert line <num>\n\r", ch );
	    send_to_char( ".lr <num> <str>  - replace line <number>  <str>\n\r", ch );
            send_to_char( "@                - end string          \n\r", ch );
            return;
        }

        send_to_char( "SEdit:  Invalid dot command.\n\r", ch );
        return;
    }

    if ( *argument == '~' || *argument == '@' )
    {
	if ( ch->desc->editor == ED_MPCODE ) /* para los mobprogs */
	{
		MOB_INDEX_DATA *mob;
		int hash;
		PROG_LIST *mpl;
		PROG_CODE *mpc;

		EDIT_MPCODE(ch, mpc);

		if ( mpc != NULL )
			for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
				for ( mob = mob_index_hash[hash]; mob; mob = mob->next )
					for ( mpl = mob->mprogs; mpl; mpl = mpl->next )
						if ( mpl->vnum == mpc->vnum )
						{
							sprintf( buf, "Arreglando mob %d.\n\r", mob->vnum );
							send_to_char( buf, ch );
							mpl->code = mpc->code;
						}
	}

       	if ( ch->desc->editor == ED_OPCODE ) /* for the objprogs */
	{
		OBJ_INDEX_DATA *obj;
		int hash;
		PROG_LIST *opl;
		PROG_CODE *opc;

		EDIT_OPCODE(ch, opc);

		if ( opc != NULL )
			for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
				for ( obj = obj_index_hash[hash]; obj; obj = obj->next )
					for ( opl = obj->oprogs; opl; opl = opl->next )
						if ( opl->vnum == opc->vnum )
						{
							sprintf( buf, "Fixing object %d.\n\r", obj->vnum );
							send_to_char( buf, ch );
							opl->code = opc->code;
						}
	}

	if ( ch->desc->editor == ED_RPCODE ) /* for the roomprogs */
	{
		ROOM_INDEX_DATA *room;
		int hash;
		PROG_LIST *rpl;
		PROG_CODE *rpc;

		EDIT_RPCODE(ch, rpc);

		if ( rpc != NULL )
			for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
				for ( room = room_index_hash[hash]; room; room = room->next )
					for ( rpl = room->rprogs; rpl; rpl = rpl->next )
						if ( rpl->vnum == rpc->vnum )
						{
							sprintf( buf, "Fixing room %d.\n\r", room->vnum );
							send_to_char( buf, ch );
							rpl->code = rpc->code;
						}
	}

        ch->desc->pString = NULL;
        if (!IS_NPC(ch) && ch->pcdata->in_progress)     /* This a note we're editing? */
        {
            send_to_desc("\n\r\n\r", ch->desc);
            send_to_desc(szFinishPrompt, ch->desc);
            send_to_desc("\n\r", ch->desc);
        }
        return;
    }

    strcpy( buf, *ch->desc->pString );

    /*
     * Truncate strings to MAX_STRING_LENGTH.
     * --------------------------------------
     */
    if ( strlen( buf ) + strlen( argument ) >= ( MAX_STRING_LENGTH - 4 ) )
    {
        send_to_char( "String too long, last line skipped.\n\r", ch );

	/* Force character out of editing mode. */
        ch->desc->pString = NULL;
        return;
    }

    /*
     * Ensure no tilde's inside string.
     * --------------------------------
     */
    smash_tilde( argument );

    strcat( buf, argument );
    strcat( buf, "\n\r" );
    free_string( *ch->desc->pString );
    *ch->desc->pString = str_dup( buf );
    return;
}



/*
 * Thanks to Kalgen for the new procedure (no more bug!)
 * Original wordwrap() written by Surreality.
 */
/*****************************************************************************
 Name:		format_string
 Purpose:	Special string formating and word-wrapping.
 Called by:	string_add(string.c) (many)olc_act.c
 ****************************************************************************/
char *format_string( char *oldstring /*, bool fSpace */)
{
  char xbuf[MAX_STRING_LENGTH];
  char xbuf2[MAX_STRING_LENGTH];
  char *rdesc;
  int i=0;
  bool cap=TRUE;
  
  xbuf[0]=xbuf2[0]=0;
  
  i=0;
  
  for (rdesc = oldstring; *rdesc; rdesc++)
  {
    if (*rdesc=='\n')
    {
      if (xbuf[i-1] != ' ')
      {
        xbuf[i]=' ';
        i++;
      }
    }
    else if (*rdesc=='\r') ;
    else if (*rdesc==' ')
    {
      if (xbuf[i-1] != ' ')
      {
        xbuf[i]=' ';
        i++;
      }
    }
    else if (*rdesc==')')
    {
      if (xbuf[i-1]==' ' && xbuf[i-2]==' ' && 
          (xbuf[i-3]=='.' || xbuf[i-3]=='?' || xbuf[i-3]=='!'))
      {
        xbuf[i-2]=*rdesc;
        xbuf[i-1]=' ';
        xbuf[i]=' ';
        i++;
      }
      else
      {
        xbuf[i]=*rdesc;
        i++;
      }
    }
    else if (*rdesc=='.' || *rdesc=='?' || *rdesc=='!') {
      if (xbuf[i-1]==' ' && xbuf[i-2]==' ' && 
          (xbuf[i-3]=='.' || xbuf[i-3]=='?' || xbuf[i-3]=='!')) {
        xbuf[i-2]=*rdesc;
        if (*(rdesc+1) != '\"')
        {
          xbuf[i-1]=' ';
          xbuf[i]=' ';
          i++;
        }
        else
        {
          xbuf[i-1]='\"';
          xbuf[i]=' ';
          xbuf[i+1]=' ';
          i+=2;
          rdesc++;
        }
      }
      else
      {
        xbuf[i]=*rdesc;
        if (*(rdesc+1) != '\"')
        {
          xbuf[i+1]=' ';
          xbuf[i+2]=' ';
          i += 3;
        }
        else
        {
          xbuf[i+1]='\"';
          xbuf[i+2]=' ';
          xbuf[i+3]=' ';
          i += 4;
          rdesc++;
        }
      }
      cap = TRUE;
    }
    else
    {
      xbuf[i]=*rdesc;
      if ( cap )
        {
          cap = FALSE;
          xbuf[i] = UPPER( xbuf[i] );
        }
      i++;
    }
  }
  xbuf[i]=0;
  strcpy(xbuf2,xbuf);
  
  rdesc=xbuf2;
  
  xbuf[0]=0;
  
  for ( ; ; )
  {
    for (i=0; i<77; i++)
    {
      if (!*(rdesc+i)) break;
    }
    if (i<77)
    {
      break;
    }
    for (i=(xbuf[0]?76:73) ; i ; i--)
    {
      if (*(rdesc+i)==' ') break;
    }
    if (i)
    {
      *(rdesc+i)=0;
      strcat(xbuf,rdesc);
      strcat(xbuf,"\n\r");
      rdesc += i+1;
      while (*rdesc == ' ') rdesc++;
    }
    else
    {
      bug ("No spaces", 0);
      *(rdesc+75)=0;
      strcat(xbuf,rdesc);
      strcat(xbuf,"-\n\r");
      rdesc += 76;
    }
  }
  while (*(rdesc+i) && (*(rdesc+i)==' '||
                        *(rdesc+i)=='\n'||
                        *(rdesc+i)=='\r'))
    i--;
  *(rdesc+i+1)=0;
  strcat(xbuf,rdesc);
  if (xbuf[strlen(xbuf)-2] != '\n')
    strcat(xbuf,"\n\r");

  free_string(oldstring);
  return(str_dup(xbuf));
}



/*
 * Used above in string_add.  Because this function does not
 * modify case if fCase is FALSE and because it understands
 * parenthesis, it would probably make a nice replacement
 * for one_argument.
 */
/*****************************************************************************
 Name:		first_arg
 Purpose:	Pick off one argument from a string and return the rest.
 		Understands quates, parenthesis (barring ) ('s) and
 		percentages.
 Called by:	string_add(string.c)
 ****************************************************************************/
char *first_arg( char *argument, char *arg_first, bool fCase )
{
    char cEnd;

    while ( *argument == ' ' )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"'
      || *argument == '%'  || *argument == '(' )
    {
        if ( *argument == '(' )
        {
            cEnd = ')';
            argument++;
        }
        else cEnd = *argument++;
    }

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
    if ( fCase ) *arg_first = LOWER(*argument);
            else *arg_first = *argument;
	arg_first++;
	argument++;
    }
    *arg_first = '\0';

    while ( *argument == ' ' )
	argument++;

    return argument;
}




/*
 * Used in olc_act.c for aedit_builders.
 */
char * string_unpad( char * argument )
{
    char buf[MAX_STRING_LENGTH];
    char *s;

    s = argument;

    while ( *s == ' ' )
        s++;

    strcpy( buf, s );
    s = buf;

    if ( *s != '\0' )
    {
        while ( *s != '\0' )
            s++;
        s--;

        while( *s == ' ' )
            s--;
        s++;
        *s = '\0';
    }

    free_string( argument );
    return str_dup( buf );
}



/*
 * Same as capitalize but changes the pointer's data.
 * Used in olc_act.c in aedit_builder.
 */
char * string_proper( char * argument )
{
    char *s;

    s = argument;

    while ( *s != '\0' )
    {
        if ( *s != ' ' )
        {
            *s = UPPER(*s);
            while ( *s != ' ' && *s != '\0' )
                s++;
        }
        else
        {
            s++;
        }
    }

    return argument;
}

char *string_linedel( char *string, int line )
{
	char *strtmp = string;
	char buf[MAX_STRING_LENGTH];
	int cnt = 1, tmp = 0;

	buf[0] = '\0';

	for ( ; *strtmp != '\0'; strtmp++ )
	{
		if ( cnt != line )
			buf[tmp++] = *strtmp;

		if ( *strtmp == '\n' )
		{
			if ( *(strtmp + 1) == '\r' )
			{
				if ( cnt != line )
					buf[tmp++] = *(++strtmp);
				else
					++strtmp;
			}

			cnt++;
		}
	}

	buf[tmp] = '\0';

	free_string(string);
	return str_dup(buf);
}

char *string_lineadd( char *string, char *newstr, int line )
{
	char *strtmp = string;
	int cnt = 1, tmp = 0;
	bool done = FALSE;
	char buf[MAX_STRING_LENGTH];

	buf[0] = '\0';

	for ( ; *strtmp != '\0' || (!done && cnt == line); strtmp++ )
	{
		if ( cnt == line && !done )
		{
			strcat( buf, newstr );
			strcat( buf, "\n\r" );
			tmp += strlen(newstr) + 2;
			cnt++;
			done = TRUE;
		}

		buf[tmp++] = *strtmp;

		if ( done && *strtmp == '\0' )
			break;

		if ( *strtmp == '\n' )
		{
			if ( *(strtmp + 1) == '\r' )
				buf[tmp++] = *(++strtmp);

			cnt++;
		}

		buf[tmp] = '\0';
	}

	free_string(string);
	return str_dup(buf);
}

/* buf queda con la linea sin \n\r */
char *getaline( char *str, char *buf )
{
	int tmp = 0;
	bool found = FALSE;

	while ( *str )
	{
		if ( *str == '\n' )
		{
			found = TRUE;
			break;
		}

		buf[tmp++] = *(str++);
	}

	if ( found )
	{
		if ( *(str + 1) == '\r' )
			str += 2;
		else
			str += 1;
	} /* para que quedemos en el inicio de la prox linea */

	buf[tmp] = '\0';

	return str;
}

char *numlineas( char *string )
{
	int cnt = 1;
	static char buf[MAX_STRING_LENGTH*2];
	char buf2[MAX_STRING_LENGTH], tmpb[MAX_STRING_LENGTH];

	buf[0] = '\0';

	while ( *string )
	{
		string = getaline( string, tmpb );
		sprintf( buf2, "%2d. %s\n\r", cnt++, tmpb );
		strcat( buf, buf2 );
	}

	return buf;
}

char *itos ( int num )
{
     char                buf[256];
     sprintf( buf, "%d", num );
     return str_dup ( buf );
}

size_t strlcat(char *dst, const char *src, size_t siz)
{
     register char *d = dst;
     register const char *s = src;
     register size_t n = siz;
     size_t dlen;
     /* Find the end of dst and adjust bytes left but don't go past end */
     while (n-- != 0 && *d != '\0')
          d++;
     dlen = d - dst;
     n = siz - dlen;
     if (n == 0)
          return(dlen + strlen(s));
     while (*s != '\0')
     {
          if (n != 1)
          {
               *d++ = *s;
               n--;
          }
          s++;
     }
     *d = '\0';
     return(dlen + (s - src));       /* count does not include NUL */
}
//worldmap.c
char *fade_color16(const char *string)
{
    if (!string)
    {
        return NULL;
    }

    size_t len = strlen(string);
    char *result = (char *)malloc(len + 1);

    if (!result)
    {
        return NULL;
    }

    size_t i, j = 0;
    for (i = 0; i < len; i++)
    {
        if (string[i] == '{' && isupper(string[i + 1]) && string[i + 1] != 'D')
        {
            result[j++] = '{';
            result[j++] = tolower(string[i + 1]);
            i++; 
        }
        else
        {
            result[j++] = string[i];
        }
    }

    result[j] = '\0';
    return result;
}

char *fade_color(const char *string, int fade)
{
    char buf[MAX_STRING_LENGTH];
    char *newstr;
    int count = 0;
    char temp;
    int r = 0, g = 0, b = 0;
    bool bg = FALSE;

    if(fade > 5)
        fade = 5;

    newstr = buf;
    while (*string && count < (MAX_STRING_LENGTH - 1))
    {
        temp = *string++;
        if (temp == '{')
        {
            temp = *string++;
            if (temp == '[')
            {
                temp = *string++;
                if (temp == 'F' || temp == 'f' || temp == 'B' || temp == 'b')
                {
                    if(temp == 'B' || temp == 'b')
                        bg = TRUE;
                    else
                        bg = FALSE;
                    r = 0;
                    g = 0;
                    b = 0;
                    temp = *string++;
                    if (temp == 'G' || temp == 'g')
                    {
                        buf[count++] = '{';
                        buf[count++] = '[';
                        buf[count++] = 'F';
                        buf[count++] = 'G';

                        temp = *string++;;
                        if(temp == '0')
                            g = 0;
                        else if(temp == '1')
                            g = 1;
                        else
                            g = 2;

                        temp = *string++;;
                        if(temp == '0')
                            b = 0;
                        else if(temp == '1')
                            b = 1;
                        else if(temp == '2')
                            b = 2;
                        else if(temp == '3')
                            b = 3;
                        else if(temp == '4')
                            b = 4;
                        else if(temp == '5')
                            b = 5;
                        else if(temp == '6')
                            b = 6;
                        else if(temp == '7')
                            b = 7;
                        else if(temp == '8')
                            b = 8;
                        else
                            b = 9;

                        if(fade == 5)
                        {
                            buf[count++] = '0';
                            buf[count++] = '2';
                        }
                        else
                        {
                            int sum = g * 10 + b;

                            if(fade == 1)
                            {
                                sum = sum * 83 / 100;

                                g = (sum / 10) % 10;
                                b = sum  % 10;
                            }
                            else if(fade == 2)
                            {
                                sum = sum * 66 / 100;

                                g = (sum / 10) % 10;
                                b = sum  % 10;
                            }
                            else if(fade == 3)
                            {
                                sum = sum * 49 / 100;

                                g = (sum / 10) % 10;
                                b = sum  % 10;
                            }
                            else if(fade == 4)
                            {
                                sum = sum * 32 / 100;

                                g = (sum / 10) % 10;
                                b = sum  % 10;
                            }


                            if(g == 2)
                                buf[count++] = '2';
                            else if(g == 1)
                                buf[count++] = '1';
                            else if(g == 0)
                                buf[count++] = '0';

                            if(b == 9)
                                buf[count++] = '9';
                            else if(b == 8)
                                buf[count++] = '8';
                            else if(b == 7)
                                buf[count++] = '7';
                            else if(b == 6)
                                buf[count++] = '6';
                            else if(b == 5)
                                buf[count++] = '5';
                            else if(b == 4)
                                buf[count++] = '4';
                            else if(b == 3)
                                buf[count++] = '3';
                            else if(b == 2)
                                buf[count++] = '2';
                            else if(b == 1)
                                buf[count++] = '1';
                            else if(b == 0)
                            {
                                if(g == 0)
                                    buf[count++] = '1';
                                else
                                    buf[count++] = '0';
                            }
                        }
                    }
                    else
                    {
                        buf[count++] = '{';
                        buf[count++] = '[';
                        if(!bg)
                            buf[count++] = 'F';
                        else
                            buf[count++] = 'B';

                        if(temp == '0')
                            r = 0;
                        else if(temp == '1')
                            r = 1;
                        else if(temp == '2')
                            r = 2;
                        else if(temp == '3')
                            r = 3;
                        else if(temp == '4')
                            r = 4;
                        else
                            r = 5;

                        temp = *string++;;
                        if(temp == '0')
                            g = 0;
                        else if(temp == '1')
                            g = 1;
                        else if(temp == '2')
                            g = 2;
                        else if(temp == '3')
                            g = 3;
                        else if(temp == '4')
                            g = 4;
                        else
                            g = 5;

                        temp = *string++;;
                        if(temp == '0')
                            b = 0;
                        else if(temp == '1')
                            b = 1;
                        else if(temp == '2')
                            b = 2;
                        else if(temp == '3')
                            b = 3;
                        else if(temp == '4')
                            b = 4;
                        else
                            b = 5;

                        if(fade == 5)
                        {
                            buf[count++] = 'G';
                            buf[count++] = '0';
                            buf[count++] = '2';
                        }
                        else
                        {
                            if(fade == 1)
                            {
                                if(r > 2)
                                    r -= 1;
                                if(g > 2)
                                    g -= 1;
                                if(b > 2)
                                    b -= 1;
                            }
                            else if(fade == 2)
                            {
                                if(r == 0)
                                    r = 0;
                                else if(r == 1)
                                    r = 1;
                                else if(r == 2)
                                    r -= 1;
                                else
                                    r -= fade;

                                if(g == 0)
                                    g = 0;
                                else if(g == 1)
                                    g = 1;
                                else if(g == 2)
                                    g -= 1;
                                else
                                    g -= fade;

                                if(b == 0)
                                    b = 0;
                                else if(b == 1)
                                    b = 1;
                                else if(b == 2)
                                    b -= 1;
                                else
                                    b -= fade;
                            }
                            else if(fade == 3)
                            {
                                if(r == 0)
                                    r = 0;
                                else if(r == 1)
                                    r = 1;
                                else if(r == 2)
                                    r -= 1;
                                else if(r == 3)
                                    r -= 2;
                                else
                                    r -= fade;

                                if(g == 0)
                                    g = 0;
                                else if(g == 1)
                                    g = 1;
                                else if(g == 2)
                                    g -= 1;
                                else if(g == 3)
                                    g -= 2;
                                else
                                    g -= fade;

                                if(b == 0)
                                    b = 0;
                                else if(b == 1)
                                    b = 1;
                                else if(b == 2)
                                    b -= 1;
                                else if(b == 3)
                                    b -= 2;
                                else
                                    b -= fade;
                            }
                            else if(fade == 4)
                            {
                                if(r > 0)
                                    r = 1;
                                if(g > 0)
                                    g = 1;
                                if(b > 0)
                                    b = 1;
                            }

                            if(r == 5)
                                buf[count++] = '5';
                            else if(r == 4)
                                buf[count++] = '4';
                            else if(r == 3)
                                buf[count++] = '3';
                            else if(r == 2)
                                buf[count++] = '2';
                            else if(r == 1)
                                buf[count++] = '1';
                            else if(r == 0)
                                buf[count++] = '0';

                            if(g == 5)
                                buf[count++] = '5';
                            else if(g == 4)
                                buf[count++] = '4';
                            else if(g == 3)
                                buf[count++] = '3';
                            else if(g == 2)
                                buf[count++] = '2';
                            else if(g == 1)
                                buf[count++] = '1';
                            else if(g == 0)
                                buf[count++] = '0';

                            if(b == 5)
                                buf[count++] = '5';
                            else if(b == 4)
                                buf[count++] = '4';
                            else if(b == 3)
                                buf[count++] = '3';
                            else if(b == 2)
                                buf[count++] = '2';
                            else if(b == 1)
                                buf[count++] = '1';
                            else if(b == 0)
                                buf[count++] = '0';
                        }

                    }
                    //buf[count++] = ']';
                }
            }
            else if (temp == '{')
            {
                buf[count++] = '{';
                buf[count++] = temp;
            }
            else
            {
                buf[count++] = '{';

                if (temp == 'r' || temp == 'R' || temp == 'g' || temp == 'G' || temp == 'y' || temp == 'Y' || temp == 'b' || temp == 'B'
                ||  temp == 'm' || temp == 'M' || temp == 'c' || temp == 'C' || temp == 'w' || temp == 'W' || temp == 'a' || temp == 'A'
                ||  temp == 'j' || temp == 'J' || temp == 'l' || temp == 'L' || temp == 'o' || temp == 'O' || temp == 'p' || temp == 'P'
                ||  temp == 't' || temp == 'T' || temp == 'v' || temp == 'V' || temp == 'D')
                {
                    r = 0;
                    g = 0;
                    b = 0;
                    buf[count++] = '[';
                    buf[count++] = 'F';

                    if(fade == 5)
                    {
                        buf[count++] = 'G';
                        buf[count++] = '0';
                        buf[count++] = '2';
                        buf[count++] = ']';
                    }
                    else
                    {
                        switch ( temp )
                        {
                            case 'r': // dark red F200
                                r = 2; g = 0; b = 0;
                                break;
                            case 'R': // light red F500
                                r = 5; g = 0; b = 0;
                                break;
                            case 'g': // dark green F020
                                r = 0; g = 2; b = 0;
                                break;
                            case 'G': // light green F050
                                r = 0; g = 5; b = 0;
                                break;
                            case 'y': // dark yellow F220
                                r = 2; g = 2; b = 0;
                                break;
                            case 'Y': // light yellow F550
                                r = 5; g = 5; b = 0;
                                break;
                            case 'b': // dark blue F002
                                r = 0; g = 0; b = 2;
                                break;
                            case 'B': // light blue F005
                                r = 0; g = 0; b = 5;
                                break;
                            case 'm': // dark magenta F202
                                r = 2; g = 0; b = 2;
                                break;
                            case 'M': // light magenta F505
                                r = 5; g = 0; b = 5;
                                break;
                            case 'c': // dark cyan F022
                                r = 0; g = 2; b = 2;
                                break;
                            case 'C': // light cyan F055
                                r = 0; g = 5; b = 5;
                                break;
                            case 'w': // dark white F333
                                r = 3; g = 3; b = 3;
                                break;
                            case 'W': // light white F555
                                r = 5; g = 5; b = 5;
                                break;
                            case 'a': // dark azure F014
                                r = 0; g = 1; b = 4;
                                break;
                            case 'A': // light azure F025
                                r = 0; g = 2; b = 5;
                                break;
                            case 'j': // dark jade F031
                                r = 0; g = 3; b = 1;
                                break;
                            case 'J': // light jade F052
                                r = 0; g = 5; b = 2;
                                break;
                            case 'l': // dark lime F140
                                r = 1; g = 4; b = 0;
                                break;
                            case 'L': // light lime F250
                                r = 2; g = 5; b = 0;
                                break;
                            case 'o': // dark orange F310
                                r = 3; g = 1; b = 0;
                                break;
                            case 'O': // light orange F520
                                r = 5; g = 2; b = 0;
                                break;
                            case 'p': // dark pink F301
                                r = 3; g = 0; b = 1;
                                break;
                            case 'P': // light pink F502
                                r = 5; g = 0; b = 2;
                                break;
                            case 't': // dark tan F210
                                r = 2; g = 1; b = 0;
                                break;
                            case 'T': // light tan F321
                                r = 3; g = 2; b = 1;
                                break;
                            case 'v': // dark violet F104
                                r = 1; g = 0; b = 4;
                                break;
                            case 'V': // light violet F205
                                r = 2; g = 0; b = 5;
                                break;
                        }

                        if(temp == 'D')
                        {
                            buf[count++] = 'G';
                            if(fade == 1)
                            {
                                buf[count++] = '1';
                                buf[count++] = '0';
                            }
                            else if(fade == 2)
                            {
                                buf[count++] = '0';
                                buf[count++] = '8';
                            }
                            else if(fade == 3)
                            {
                                buf[count++] = '0';
                                buf[count++] = '6';
                            }
                            else
                            {
                                buf[count++] = '0';
                                buf[count++] = '4';
                            }
                        }
                        else
                        {
                            if(fade == 1)
                            {
                                if(r > 2)
                                    r -= 1;
                                if(g > 2)
                                    g -= 1;
                                if(b > 2)
                                    b -= 1;
                            }
                            else if(fade == 2)
                            {
                                if(r == 0)
                                    r = 0;
                                else if(r == 1)
                                    r = 1;
                                else if(r == 2)
                                    r -= 1;
                                else
                                    r -= fade;

                                if(g == 0)
                                    g = 0;
                                else if(g == 1)
                                    g = 1;
                                else if(g == 2)
                                    g -= 1;
                                else
                                    g -= fade;

                                if(b == 0)
                                    b = 0;
                                else if(b == 1)
                                    b = 1;
                                else if(b == 2)
                                    b -= 1;
                                else
                                    b -= fade;
                            }
                            else if(fade == 3)
                            {
                                if(r == 0)
                                    r = 0;
                                else if(r == 1)
                                    r = 1;
                                else if(r == 2)
                                    r -= 1;
                                else if(r == 3)
                                    r -= 2;
                                else
                                    r -= fade;

                                if(g == 0)
                                    g = 0;
                                else if(g == 1)
                                    g = 1;
                                else if(g == 2)
                                    g -= 1;
                                else if(g == 3)
                                    g -= 2;
                                else
                                    g -= fade;

                                if(b == 0)
                                    b = 0;
                                else if(b == 1)
                                    b = 1;
                                else if(b == 2)
                                    b -= 1;
                                else if(b == 3)
                                    b -= 2;
                                else
                                    b -= fade;
                            }
                            else if(fade == 4)
                            {
                                if(r > 0)
                                    r = 1;
                                if(g > 0)
                                    g = 1;
                                if(b > 0)
                                    b = 1;
                            }

                            if(r == 5)
                                buf[count++] = '5';
                            else if(r == 4)
                                buf[count++] = '4';
                            else if(r == 3)
                                buf[count++] = '3';
                            else if(r == 2)
                                buf[count++] = '2';
                            else if(r == 1)
                                buf[count++] = '1';
                            else if(r == 0)
                                buf[count++] = '0';

                            if(g == 5)
                                buf[count++] = '5';
                            else if(g == 4)
                                buf[count++] = '4';
                            else if(g == 3)
                                buf[count++] = '3';
                            else if(g == 2)
                                buf[count++] = '2';
                            else if(g == 1)
                                buf[count++] = '1';
                            else if(g == 0)
                                buf[count++] = '0';

                            if(b == 5)
                                buf[count++] = '5';
                            else if(b == 4)
                                buf[count++] = '4';
                            else if(b == 3)
                                buf[count++] = '3';
                            else if(b == 2)
                                buf[count++] = '2';
                            else if(b == 1)
                                buf[count++] = '1';
                            else if(b == 0)
                                buf[count++] = '0';
                        }

                        buf[count++] = ']';
                    }
                }
                else
                    buf[count++] = temp;
            }
            continue;
        }
        buf[count++] = temp;
    }
    buf[count] = '\0';
    return newstr;
}


//worldmap.c
char *strip_newlines(const char *string)
{
    if (!string)
        return NULL;

    size_t len = strlen(string);
    char *result = (char *)malloc(len + 1);

    if (!result)
        return NULL;

    size_t i, j = 0;
    for (i = 0; i < len; i++)
    {
        if (string[i] == '\n' || string[i] == '\r')
        {
            result[j++] = ' ';
            
            if (string[i + 1] == '\n' || string[i + 1] == '\r')
                i++;
        }
        else
            result[j++] = string[i];
    }

    result[j] = '\0';
    return result;
}

char *strip_recolor(const char *string)
{
    if (!string)
        return NULL;

    size_t len = strlen(string);
    char *result = (char *)malloc(len * 2 + 1);

    if (!result)
        return NULL;

    size_t i, j = 0;
    for (i = 0; i < len; i++)
    {
        if (string[i] == '{')
        {
            i++;
            if (string[i] == '\0')
                break;
            continue;
        }
        result[j++] = string[i];
    }

    result[j] = '\0';
    return result;
}
