#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
/* #include <stdlib.h> */
#include <time.h>
#include "mud.h"

extern int top_area;
extern int top_r_vnum;
void write_area_list();
void write_starsystem_list();
extern const   char *  sector_name     [SECT_MAX];

PLANET_DATA * first_planet;
PLANET_DATA * last_planet;

GUARD_DATA * first_guard;
GUARD_DATA * last_guard;

/* local routines */
void	fread_planet	args( ( PLANET_DATA *planet, FILE *fp ) );
bool	load_planet_file	args( ( char *planetfile ) );
void	write_planet_list	args( ( void ) );

PLANET_DATA *get_planet( char *name )
{
    PLANET_DATA *planet;
    
    if ( name[0] == '\0' )
       return NULL;
    
    for ( planet = first_planet; planet; planet = planet->next )
       if ( !str_cmp( name, planet->name ) )
         return planet;
    
    for ( planet = first_planet; planet; planet = planet->next )
       if ( nifty_is_name( name, planet->name ) )
         return planet;
    
    for ( planet = first_planet; planet; planet = planet->next )
       if ( !str_prefix( name, planet->name ) )
         return planet;
    
    for ( planet = first_planet; planet; planet = planet->next )
       if ( nifty_is_name_prefix( name, planet->name ) )
         return planet;
    
    return NULL;
}

void write_planet_list( )
{
    PLANET_DATA *tplanet;
    FILE *fpout;
    char filename[256];

    sprintf( filename, "%s%s", PLANET_DIR, PLANET_LIST );
    fpout = fopen( filename, "w" );
    if ( !fpout )
    {
	bug( "FATAL: cannot open planet.lst for writing!\n\r", 0 );
 	return;
    }	  
    for ( tplanet = first_planet; tplanet; tplanet = tplanet->next )
	fprintf( fpout, "%s\n", tplanet->filename );
    fprintf( fpout, "$\n" );
    fclose( fpout );
}

void save_planet( PLANET_DATA *planet )
{
    FILE *fp;
    char filename[256];
    char buf[MAX_STRING_LENGTH];

    if ( !planet )
    {
	bug( "save_planet: null planet pointer!", 0 );
	return;
    }
        
    if ( !planet->filename || planet->filename[0] == '\0' )
    {
	sprintf( buf, "save_planet: %s has no filename", planet->name );
	bug( buf, 0 );
	return;
    }
 
    sprintf( filename, "%s%s", PLANET_DIR, planet->filename );
    
    fclose( fpReserve );
    if ( ( fp = fopen( filename, "w" ) ) == NULL )
    {
    	bug( "save_planet: fopen", 0 );
    	perror( filename );
    }
    else
    {
        AREA_DATA *pArea;
        
	fprintf( fp, "#PLANET\n" );
	fprintf( fp, "Name         %s~\n",	planet->name		);
	fprintf( fp, "Filename     %s~\n",	planet->filename        );
	fprintf( fp, "X            %d\n",	planet->x               );
	fprintf( fp, "Y            %d\n",	planet->y               );
	fprintf( fp, "Z            %d\n",	planet->z               );
	fprintf( fp, "Sector       %d\n",	planet->sector          );
	fprintf( fp, "PopSupport   %d\n",	(int) (planet->pop_support)      );
	if ( planet->starsystem && planet->starsystem->name )
        	fprintf( fp, "Starsystem   %s~\n",	planet->starsystem->name);
	if ( planet->governed_by && planet->governed_by->name )
        	fprintf( fp, "GovernedBy   %s~\n",	planet->governed_by->name);
	pArea = planet->area;
	if (pArea->filename)
         	fprintf( fp, "Area         %s~\n",	pArea->filename  );
	fprintf( fp, "End\n\n"						);
	fprintf( fp, "#END\n"						);
    }
    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}


void fread_planet( PLANET_DATA *planet, FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    char *word;
    bool fMatch;

    for ( ; ; )
    {
	word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'A':
	    if ( !str_cmp( word, "Area" ) )
	    {
	        char aName[MAX_STRING_LENGTH];
                AREA_DATA *pArea;
                	        
	     	sprintf (aName, fread_string(fp));
		for( pArea = first_area ; pArea ; pArea = pArea->next )
	          if (pArea->filename && !str_cmp(pArea->filename , aName ) )
	          {
	             ROOM_INDEX_DATA *room;
	             
	             planet->size = 0;
	             planet->citysize = 0;
	             planet->wilderness = 0;
	             planet->farmland = 0;
	             planet->barracks = 0;
	             planet->controls = 0;
	             pArea->planet = planet; 
	             planet->area = pArea;
	             for( room = pArea->first_room ; room ; room = room->next_in_area )
                     {  	       
                       	  planet->size++;
                          if ( room->sector_type <= SECT_CITY )
                             planet->citysize++;    
                          else if ( room->sector_type == SECT_FARMLAND )
                             planet->farmland++;    
                          else if ( room->sector_type != SECT_DUNNO )
                             planet->wilderness++;
                             
                          if ( IS_SET( room->room_flags , ROOM_CONTROL ))
                             planet->controls++;    
                          if ( IS_SET( room->room_flags , ROOM_BARRACKS ))
                             planet->barracks++;    
                     } 	       
	          }      
                fMatch = TRUE;
	    }
	    break;

	case 'E':
	    if ( !str_cmp( word, "End" ) )
	    {
		if (!planet->name)
		  planet->name		= STRALLOC( "" );
		return;
	    }
	    break;

	case 'F':
	    KEY( "Filename",	planet->filename,		fread_string_nohash( fp ) );
	    break;
	
	case 'G':
	    if ( !str_cmp( word, "GovernedBy" ) )
	    {
	     	planet->governed_by = get_clan ( fread_string(fp) );
                fMatch = TRUE;
	    }
	    break;
	
	case 'N':
	    KEY( "Name",	planet->name,		fread_string( fp ) );
	    break;
	
	case 'P':
	    KEY( "PopSupport",	planet->pop_support,		fread_number( fp ) );
	    break;

	case 'S':
	    KEY( "Sector",	planet->sector,		fread_number( fp ) );
	    if ( !str_cmp( word, "Starsystem" ) )
	    {
	     	planet->starsystem = starsystem_from_name ( fread_string(fp) );
                if (planet->starsystem)
                {
                     SPACE_DATA *starsystem = planet->starsystem;
                     
                     LINK( planet , starsystem->first_planet, starsystem->last_planet, next_in_system, prev_in_system );
                }
                fMatch = TRUE;
	    }
	    break;

	case 'X':
	    KEY( "X",	planet->x,		fread_number( fp ) );
	    break;

	case 'Y':
	    KEY( "Y",	planet->y,		fread_number( fp ) );
	    break;

	case 'Z':
	    KEY( "Z",	planet->z,		fread_number( fp ) );
	    break;
    
	}
	
	if ( !fMatch )
	{
	    sprintf( buf, "Fread_planet: no match: %s", word );
	    bug( buf, 0 );
	}
	
    }
}

bool load_planet_file( char *planetfile )
{
    char filename[256];
    PLANET_DATA *planet;
    FILE *fp;
    bool found;

    CREATE( planet, PLANET_DATA, 1 );
    
    planet->governed_by = NULL;
    planet->next_in_system = NULL;
    planet->prev_in_system = NULL;
    planet->starsystem = NULL ;
    planet->area = NULL;
    planet->first_guard = NULL;
    planet->last_guard = NULL;
    
    found = FALSE;
    sprintf( filename, "%s%s", PLANET_DIR, planetfile );

    if ( ( fp = fopen( filename, "r" ) ) != NULL )
    {

	found = TRUE;
	for ( ; ; )
	{
	    char letter;
	    char *word;

	    letter = fread_letter( fp );
	    if ( letter == '*' )
	    {
		fread_to_eol( fp );
		continue;
	    }

	    if ( letter != '#' )
	    {
		bug( "Load_planet_file: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );
	    if ( !str_cmp( word, "PLANET"	) )
	    {
	    	fread_planet( planet, fp );
	    	break;
	    }
	    else
	    if ( !str_cmp( word, "END"	) )
	        break;
	    else
	    {
		char buf[MAX_STRING_LENGTH];

		sprintf( buf, "Load_planet_file: bad section: %s.", word );
		bug( buf, 0 );
		break;
	    }
	}
	fclose( fp );
    }

    if ( !found )
      DISPOSE( planet );
    else
      LINK( planet, first_planet, last_planet, next, prev );

    return found;
}

void load_planets( )
{
    FILE *fpList;
    char *filename;
    char planetlist[256];
    char buf[MAX_STRING_LENGTH];
    
    first_planet	= NULL;
    last_planet	= NULL;

    log_string( "Loading planets..." );

    sprintf( planetlist, "%s%s", PLANET_DIR, PLANET_LIST );
    fclose( fpReserve );
    if ( ( fpList = fopen( planetlist, "r" ) ) == NULL )
    {
	perror( planetlist );
	exit( 1 );
    }

    for ( ; ; )
    {
	filename = feof( fpList ) ? "$" : fread_word( fpList );
	log_string( filename );
	if ( filename[0] == '$' )
	  break;

	if ( !load_planet_file( filename ) )
	{
	  sprintf( buf, "Cannot load planet file: %s", filename );
	  bug( buf, 0 );
	}
    }
    fclose( fpList );
    log_string(" Done planets " );  
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}

void do_setplanet( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    PLANET_DATA *planet;

    if ( IS_NPC( ch ) )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Usage: setplanet <planet> <field> [value]\n\r", ch );
	send_to_char( "\n\rField being one of:\n\r", ch );
	send_to_char( " name filename starsystem governed_by\n\r", ch );
	return;
    }

    planet = get_planet( arg1 );
    if ( !planet )
    {
	send_to_char( "No such planet.\n\r", ch );
	return;
    }


    if ( !strcmp( arg2, "name" ) )
    {
	STRFREE( planet->name );
	planet->name = STRALLOC( argument );
	send_to_char( "Done.\n\r", ch );
	save_planet( planet );
	return;
    }

    if ( !strcmp( arg2, "sector" ) )
    {
	planet->sector = atoi(argument);
	send_to_char( "Done.\n\r", ch );
	save_planet( planet );
	return;
    }

    if ( !strcmp( arg2, "governed_by" ) )
    {
        CLAN_DATA *clan;
        clan = get_clan( argument );
        if ( clan )
        { 
           planet->governed_by = clan;
           send_to_char( "Done.\n\r", ch ); 
       	   save_planet( planet );
        }
        else
           send_to_char( "No such clan.\n\r", ch ); 
	return;
    }

    if ( !strcmp( arg2, "starsystem" ) )
    {
        SPACE_DATA *starsystem;
        
        if ((starsystem=planet->starsystem) != NULL)
          UNLINK(planet, starsystem->first_planet, starsystem->last_planet, next_in_system, prev_in_system);
	if ( (planet->starsystem = starsystem_from_name(argument)) )
        {
           starsystem = planet->starsystem;
           LINK(planet, starsystem->first_planet, starsystem->last_planet, next_in_system, prev_in_system);	
           send_to_char( "Done.\n\r", ch );
	}
	else 
	       	send_to_char( "No such starsystem.\n\r", ch );
	save_planet( planet );
	return;
    }

    if ( !strcmp( arg2, "filename" ) )
    {
	DISPOSE( planet->filename );
	planet->filename = str_dup( argument );
	send_to_char( "Done.\n\r", ch );
	save_planet( planet );
	write_planet_list( );
	return;
    }


    do_setplanet( ch, "" );
    return;
}

void do_showplanet( CHAR_DATA *ch, char *argument )
{   
    GUARD_DATA * guard;
    PLANET_DATA *planet;
    int num_guards = 0;
    int pf = 0;
    int pc = 0;
    int pw = 0;
    
    if ( IS_NPC( ch ) )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Usage: showplanet <planet>\n\r", ch );
	return;
    }

    planet = get_planet( argument );
    if ( !planet )
    {
	send_to_char( "No such planet.\n\r", ch );
	return;
    }

    for ( guard = planet->first_guard ; guard ; guard = guard->next_on_planet )
        num_guards++;

    if ( planet->size > 0 )
    {
       float tempf;
       
       tempf = planet->citysize;
       pc = tempf / planet->size *  100;

       tempf = planet->wilderness;
       pw = tempf / planet->size *  100;

       tempf = planet->farmland;
       pf = tempf / planet->size *  100;    
    }
    
    ch_printf( ch, "&W%s\n\r", planet->name);
    if ( IS_IMMORTAL(ch) )
          ch_printf( ch, "&WFilename: &G%s\n\r", planet->filename);
        
    ch_printf( ch, "&WTerrain: &G%s\n\r", 
                   sector_name[planet->sector]  );
    ch_printf( ch, "&WGoverned by: &G%s\n\r", 
                   planet->governed_by ? planet->governed_by->name : "" );
    ch_printf( ch, "&WPlanet Size: &G%d\n\r", 
                   planet->size );
    ch_printf( ch, "&WPercent Civilized: &G%d\n\r", pc ) ;
    ch_printf( ch, "&WPercent Wilderness: &G%d\n\r", pw ) ;
    ch_printf( ch, "&WPercent Farmland: &G%d\n\r", pf ) ;
    ch_printf( ch, "&WBarracks: &G%d\n\r", planet->barracks );
    ch_printf( ch, "&WControl Towers: &G%d\n\r", planet->controls );
    ch_printf( ch, "&WPatrols: &G%d&W/%d\n\r", num_guards , planet->barracks*5 );
    ch_printf( ch, "&WPopulation: &G%d&W/%d\n\r", planet->population , max_population( planet ) );
    ch_printf( ch, "&WPopular Support: &G%.2f\n\r", 
                   planet->pop_support );
    ch_printf( ch, "&WCurrent Monthly Revenue: &G%ld\n\r", 
                   get_taxes( planet) );
    if ( IS_IMMORTAL(ch) && !planet->area )
    {
          ch_printf( ch, "&RWarning - this planet is not attached to an area!&G");
          ch_printf( ch, "\n\r" );
    }         
    
    return;
}


void do_makeplanet( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    PLANET_DATA *planet;
    char arg1[MAX_STRING_LENGTH];
    char arg3[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char filename[MAX_STRING_LENGTH];
    char pname[MAX_STRING_LENGTH];
    char * description = NULL;
    bool destok = TRUE;
    int rnum, sector;
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *troom;
    EXIT_DATA * xit;
    SPACE_DATA *starsystem;
            
    switch( ch->substate )
    {
	default:
	  break;
	case SUB_ROOM_DESC:
	  pArea = (AREA_DATA *) ch->dest_buf;
	  if ( !pArea )
	  {
		bug( "makep: sub_room_desc: NULL ch->dest_buf", 0 );
		destok = FALSE;
	  }
	  planet = (PLANET_DATA *) ch->dest_buf_2;
	  if ( !planet )
	  {
		bug( "make: sub_room_desc: NULL ch->dest_buf2", 0 );
		destok = FALSE;
	  }
	  description = copy_buffer( ch );
	  stop_editing( ch );
	  ch->substate = ch->tempnum;
	  if ( !destok )
	     return;
         
         for ( rnum = 1 ; rnum <= 25 ; rnum ++ )
         {
      	     location = make_room( ++top_r_vnum );
      	     planet->size++;

	     if ( !location )
	     {
	        bug( "makep: make_room failed", 0 );
	        return;
	     }

	     location->area = pArea;
	     STRFREE( location->description );
	     STRFREE( location->name );
	     
	     if ( rnum == 12 )
	     {
	        location->name = STRALLOC( "Supply Shop" );
                strcpy( buf , "This visible part of this shop consists of a long desk with a couple\n\r" );
                strcat( buf , "of computer terminals located along its length. A large set of sliding\n\r" );
                strcat( buf , "doors conceals the supply room behind. In front of the main desk a\n\r" );
                strcat( buf , "smaller circular desk houses several mail terminals as the shop also\n\r" );
                strcat( buf , "doubles as a post office. This shop stocks an assortment of basic\n\r" );
                strcat( buf , "supplies useful to both travellers and settlers. The shopkeeper will\n\r" );
                strcat( buf , "also purchase some items that might have some later resale or trade\n\r" );
                strcat( buf , "value.\n\r" );
	        location->description = STRALLOC(buf);
             	location->sector_type = SECT_INSIDE;
		SET_BIT( location->room_flags , ROOM_INDOORS );
		SET_BIT( location->room_flags , ROOM_NO_MOB );
		SET_BIT( location->room_flags , ROOM_SAFE );
		SET_BIT( location->room_flags , ROOM_NOPEDIT );
		SET_BIT( location->room_flags , ROOM_MAIL );
		SET_BIT( location->room_flags , ROOM_TRADE );
		SET_BIT( location->room_flags , ROOM_BANK );
	        planet->citysize++;
	     }
	     else if ( rnum == 13 )
	     {
	        strcpy( buf , planet->name );
	        strcat( buf , ": Colonization Center" );
	        location->name = STRALLOC( buf );
                strcpy( buf , "You stand in the main foyer of the colonization center. This is one of\n\r" );
                strcat( buf , "many similar buildings scattered on planets throughout the galaxy. It and\n\r" );
	        strcat( buf , "the others like it serve two main purposes. The first is as an initial\n\r" );
	        strcat( buf , "living and working space for early settlers to the planet. It provides\n\r" );
	        strcat( buf , "a center of operations during the early stages of a colony while the\n\r" );
	        strcat( buf , "surrounding area is being developed. Its second purpose after the\n\r" );
	        strcat( buf , "initial community has been settled is to provide an information center\n\r" );
	        strcat( buf , "for new citizens and for tourists. Food, transportation, shelter, \n\r" );
	        strcat( buf , "supplies, and information are all contained in one area making it easy\n\r" );
	        strcat( buf , "for those unfamiliar with the planet to adjust. This also makes it a\n\r" );
	        strcat( buf , "very crowded place at times.\n\r" );
	        location->description = STRALLOC(buf);
             	location->sector_type = SECT_INSIDE;
		SET_BIT( location->room_flags , ROOM_INDOORS );
		SET_BIT( location->room_flags , ROOM_NO_MOB );
		SET_BIT( location->room_flags , ROOM_SAFE );
		SET_BIT( location->room_flags , ROOM_NOPEDIT );
		SET_BIT( location->room_flags , ROOM_INFO );
	        planet->citysize++;
	     }
	     else if ( rnum == 14 ) 
	     {
	        location->name = STRALLOC( "Community Shuttle Platform" );
                strcpy( buf , "This platform is large enough for several spacecraft to land and take off\n\r" );
                strcat( buf , "from. Its surface is a hard glossy substance that is mostly smooth except\n\r" );
                strcat( buf , "a few ripples and impressions that suggest its liquid origin. Power boxes\n\r" );
                strcat( buf , "are scattered about the platform strung together by long strands of thick\n\r" );
                strcat( buf , "power cables and fuel hoses. Glowing strips divide the platform into\n\r" );
                strcat( buf , "multiple landing areas. Hard rubber pathways mark pathways for pilots and\n\r" );
                strcat( buf , "passengers, leading from the various landing areas to the Colonization\n\r" );
                strcat( buf , "Center.\n\r" );
	        location->description = STRALLOC(buf);
             	location->sector_type = SECT_CITY;
		SET_BIT( location->room_flags , ROOM_SHIPYARD );
		SET_BIT( location->room_flags , ROOM_CAN_LAND );
		SET_BIT( location->room_flags , ROOM_NO_MOB );
		SET_BIT( location->room_flags , ROOM_NOPEDIT );
	        planet->citysize++;
	     }
	     else if ( rnum == 18 )
	     {
	        strcpy( buf , planet->name );
	        strcat( buf , ": Center Hotel" );
	        location->name = STRALLOC( buf );
                strcpy( buf , "This part of the center serves as a temporary home for new settlers\n\r" );
                strcat( buf , "until a more permanent residence is found. It is also used as a hotel\n\r" );
	        strcat( buf , "for tourists and visitors. the shape of the hotel is circular with rooms\n\r" );
	        strcat( buf , "located around the perimeter extending several floors above ground level.\n\r" );
	        strcat( buf , "\n\rThis is a good place to rest. You may safely leave and reenter the\n\r" );
	        strcat( buf , "game from here.\n\r" );
	        location->description = STRALLOC(buf);
             	location->sector_type = SECT_INSIDE;
		SET_BIT( location->room_flags , ROOM_INDOORS );
		SET_BIT( location->room_flags , ROOM_HOTEL );
		SET_BIT( location->room_flags , ROOM_SAFE );
		SET_BIT( location->room_flags , ROOM_NO_MOB );
		SET_BIT( location->room_flags , ROOM_NOPEDIT );
	        planet->citysize++;
	     }
	     else
	     { 
	     	location->description = STRALLOC(description);
	     	location->name = STRALLOC( planet->name );
             	location->sector_type = planet->sector;
             	planet->wilderness++;
             }
             
	     LINK( location , pArea->first_room , pArea->last_room , next_in_area , prev_in_area );
             
             if ( rnum > 5 && rnum != 23 && rnum != 17 && rnum != 19
                  && rnum != 12 && rnum != 14 ) 
             {
                 troom = get_room_index( top_r_vnum - 5 );
                 xit = make_exit( location, troom, 0 );
	         xit->keyword		= STRALLOC( "" );
	         xit->description		= STRALLOC( "" );
	         xit->key			= -1;
	         xit->exit_info		= 0;
                 xit = make_exit( troom, location, 2 );
	         xit->keyword		= STRALLOC( "" );
	         xit->description	= STRALLOC( "" );
	         xit->key		= -1;
	         xit->exit_info		= 0;
	     }
             if ( rnum != 1 && rnum != 6 && rnum != 11 && rnum != 16
                  && rnum != 21 && rnum != 12 && rnum != 15 
                  && rnum != 18 && rnum != 19 ) 
             {
                 troom = get_room_index( top_r_vnum - 1 );
                 xit = make_exit( location, troom, 3 );
	         xit->keyword		= STRALLOC( "" );
	         xit->description		= STRALLOC( "" );
	         xit->key			= -1;
	         xit->exit_info		= 0;
                 xit = make_exit( troom, location, 1 );
	         xit->keyword		= STRALLOC( "" );
	         xit->description	= STRALLOC( "" );
	         xit->key		= -1;
	         xit->exit_info		= 0;
	     }
         }   
                  
         save_planet( planet );
         fold_area( pArea , pArea->filename , FALSE );
         write_area_list();
         write_planet_list();
         sprintf( buf , "%d" , top_r_vnum - 17 );
	 do_goto( ch , buf );
	 reset_all();

	 return;
    }

    if ( IS_NPC(ch) || !ch->pcdata )
       return;

    if (!ch->in_room || !IS_SET( ch->in_room->room_flags, ROOM_SHIPYARD ) )
    {
    	send_to_char( "Exploration probes can only be launched from a shipyard.\n\r", ch );
	return;
    }    

    if ( ch->gold < 100000 )
    {
    	send_to_char( "It costs 100000 credits to launch an exploration probe.\n\r", ch );
	return;
    }    

    if ( !argument || argument[0] == '\0' )
    {
	send_to_char( "Would you like to explore an existing star system or a new one?\n\r\n\r", ch );
	send_to_char( "Usage: explore <starsystem> <planet name>\n\r", ch );
	send_to_char( " Note: The first word in the planets name MUST be original.\n\r", ch );
	return;
    }

    argument = one_argument( argument , arg1 );

    starsystem = starsystem_from_name(arg1);
    
    if ( starsystem )
    {
         PLANET_DATA *tp;

         if ( starsystem == starsystem_from_name( NEWBIE_STARSYSTEM ) )
         {
        	ch_printf( ch, "You cannot explore in that system.\n\r", tp->governed_by->name );
        	return;
         }            
         
         for ( tp = starsystem->first_planet ; tp ; tp = tp->next_in_system )
           if ( tp->governed_by && 
           ( !ch->pcdata->clan || ch->pcdata->clan != tp->governed_by ) )
           {
        	ch_printf( ch, "You cannot explore in that system without permission from %s.\n\r", tp->governed_by->name );
        	return;
           } 

    }

    if ( !argument || argument[0] == '\0' )
    {
	send_to_char( "What would you call the new planet if you found it?\n\r\n\r", ch );
	send_to_char( "Usage: explore <starsystem> <planet name>\n\r", ch );
	send_to_char( " Note: The first word in the planets name MUST be original.\n\r", ch );
	return;
    }
    
    sector = 0;
    while ( sector == 0 || sector == SECT_WATER_SWIM
    || sector == SECT_WATER_NOSWIM || sector == SECT_UNDERWATER 
    || sector == SECT_FARMLAND )     
        sector = number_range( SECT_FIELD , SECT_MAX-1 );
        
    strcpy( pname , argument ); 
    argument = one_argument( argument , arg3 );

    for ( pArea = first_area; pArea; pArea = pArea->next )
    {
	if ( !str_cmp( pArea->filename, capitalize(arg3) ) )
	{  
	  send_to_char( "Sorry, the first word in the planets name must be original.\n\r", ch );	
	  return;
        }
    }

    strcpy ( buf , strlower(arg3) );
    strcat ( buf , ".planet" );
    
    for ( planet = first_planet ; planet; planet = planet->next )
    {
	if ( !str_cmp( planet->filename, buf ) )
	{  
	  send_to_char( "A planet with that filename already exists.\n\r", ch );	
	  send_to_char( "The first word in the planets name must be original.\n\r", ch );	
	  return;
        }
    }
    
    
    ch->gold -= 100000;
	  
    send_to_char( "You spend 100000 credits to launch an explorer probe.\n\r", ch );	
    echo_to_room( AT_WHITE , ch->in_room, "A small probe lifts off into space." );

    if (  number_percent() < 20 )
	return;

    sector = 0;
    while ( sector == 0 || sector == SECT_WATER_SWIM
    || sector == SECT_WATER_NOSWIM || sector == SECT_UNDERWATER 
    || sector == SECT_FARMLAND )     
        sector = number_range( SECT_FIELD , SECT_MAX-1 );
    
    pArea = NULL;
    planet = NULL;
    
    CREATE( planet, PLANET_DATA, 1 );
    LINK( planet, first_planet, last_planet, next, prev );
    planet->governed_by = NULL;
    planet->next_in_system = NULL;
    planet->prev_in_system = NULL;
    planet->first_guard = NULL;
    planet->last_guard = NULL;
    planet->name		= STRALLOC( capitalize(pname) );
    planet->sector = sector;
    planet->size = 0;
    planet->citysize = 0;
    planet->wilderness = 0;
    planet->farmland = 0;
    planet->barracks = 0;
    planet->controls = 0;
    planet->x      = number_range ( -10000 , 10000 );
    planet->y      = number_range ( -10000 , 10000 );
    planet->z      = number_range ( -10000 , 10000 );

    if ( starsystem )
    {
           planet->starsystem = starsystem;
           starsystem = planet->starsystem;
           LINK(planet, starsystem->first_planet, starsystem->last_planet, next_in_system, prev_in_system);
    }
    else
    {
        CREATE( starsystem, SPACE_DATA, 1 );
        LINK( starsystem, first_starsystem, last_starsystem, next, prev );
        starsystem->name		= STRALLOC( capitalize(arg1) );
        starsystem->star1            = STRALLOC( arg1 );  
        starsystem->star2            = STRALLOC( "" );
        starsystem->first_planet = planet;
        starsystem->last_planet = planet;
        starsystem->first_ship = NULL;
        starsystem->last_ship = NULL;
        starsystem->first_missile = NULL;
        starsystem->last_missile = NULL; 
        sprintf( filename, "%s.system" , strlower(arg1) );
        starsystem->filename = str_dup( filename );
        save_starsystem( starsystem );
        write_starsystem_list();
        planet->starsystem = starsystem;
    }
    
    CREATE( pArea, AREA_DATA, 1 );
    pArea->first_room	= NULL;
    pArea->last_room	= NULL;
    pArea->planet       = NULL;
    pArea->planet       = planet;
    pArea->name	        = STRALLOC( capitalize(pname) );

    planet->area = pArea;

    LINK( pArea, first_area, last_area, next, prev );
    top_area++;

    pArea->filename	= str_dup( capitalize(arg3) );
    sprintf( filename, "%s.planet" , strlower(arg3) );
    planet->filename = str_dup( filename );

    send_to_char( "\n\r&YYour probe has discovered a new planet.\n\r", ch );
    send_to_char( "The terrain apears to be mostly &W", ch );
    send_to_char( sector_name[sector], ch );
    send_to_char( "&Y.\n\r", ch );
    send_to_char( "\n\rPlease enter a description of your planet.\n\r", ch );
    send_to_char( "It should be a short paragraph of 5 or more lines.\n\r", ch );
    send_to_char( "It will be used as the planet's default room descriptions.\n\r\n\r", ch );
       
    description = STRALLOC( "" );

/* save them now just in case... */

    save_planet( planet );
    fold_area( pArea , pArea->filename , FALSE );
    write_area_list();
    write_planet_list();

	if ( ch->substate == SUB_REPEATCMD )
	  ch->tempnum = SUB_REPEATCMD;
	else
	  ch->tempnum = SUB_NONE;
	ch->substate = SUB_ROOM_DESC;
	ch->dest_buf = (void *) pArea;
	ch->dest_buf_2 = (void *) planet; 
	start_editing( ch, description );
	return;

}

void do_planets( CHAR_DATA *ch, char *argument )
{
    PLANET_DATA *planet;
    int count = 0;
    SPACE_DATA *starsystem;
    
    set_char_color( AT_WHITE, ch );
    send_to_char( "Planet          Starsystem    Governed By                  Popular Support\n\r" , ch );
    
    for ( starsystem = first_starsystem ; starsystem; starsystem = starsystem->next )    
     for ( planet = starsystem->first_planet; planet; planet = planet->next_in_system )
     {
        ch_printf( ch, "&G%-15s %-12s  %-25s    ", 
                   planet->name , starsystem->name , 
                   planet->governed_by ? planet->governed_by->name : "" );
        ch_printf( ch, "%.1f\n\r", planet->pop_support );
        if ( IS_IMMORTAL(ch) && !planet->area )
        {
          ch_printf( ch, "&RWarning - this planet is not attached to an area!&G");
          ch_printf( ch, "\n\r" );
        }         
        
        count++;
     }
            
    for ( planet = first_planet; planet; planet = planet->next )
    {
        if ( planet->starsystem )
           continue;
           
        ch_printf( ch, "&G%-15s %-12s  %-25s    ", 
                   planet->name , "", 
                   planet->governed_by ? planet->governed_by->name : "" );
        ch_printf( ch, "%.1f\n\r", planet->pop_support );
        if ( IS_IMMORTAL(ch) && !planet->area )
        {
          ch_printf( ch, "&RWarning - this planet is not attached to an area!&G");
          ch_printf( ch, "\n\r" );
        }         
        
        count++;
    }

    if ( !count )
    {
	set_char_color( AT_BLOOD, ch);
        send_to_char( "There are no planets currently formed.\n\r", ch );
    }
    send_to_char( "&WUse SHOWPLANET for more information.\n\r", ch );
    
}

void do_capture ( CHAR_DATA *ch , char *argument )
{
   CLAN_DATA *clan;
   PLANET_DATA *planet;
   PLANET_DATA *cPlanet;
   float support = 0.0;
   int pCount = 0;   
   char buf[MAX_STRING_LENGTH];
   
   if ( !ch->in_room || !ch->in_room->area)
      return;   

   if ( IS_NPC(ch) || !ch->pcdata )
   {
       send_to_char ( "huh?\n\r" , ch );
       return;
   }
   
   if ( !ch->pcdata->clan )
   {
       send_to_char ( "You need to be a member of an organization to do that!\n\r" , ch );
       return;
   }
   
   clan = ch->pcdata->clan;
      
   if ( ( planet = ch->in_room->area->planet ) == NULL )
   {
       send_to_char ( "You must be on a planet to capture it.\n\r" , ch );
       return;
   }   
   
   if ( clan == planet->governed_by )
   {
       send_to_char ( "Your organization already controls this planet.\n\r" , ch );
       return;
   }
   
   if ( planet->starsystem )
   {
       SHIP_DATA *ship;
       CLAN_DATA *sClan;
              
       for ( ship = planet->starsystem->first_ship ; ship ; ship = ship->next_in_starsystem )
       {
          sClan = get_clan(ship->owner);
          if ( !sClan ) 
             continue;
          if ( sClan == planet->governed_by )
          {
             send_to_char ( "A planet cannot be captured while protected by orbiting spacecraft.\n\r" , ch );
             return;
          }
       }
   }
   
   if ( planet->first_guard )
   {
       send_to_char ( "This planet is protected by soldiers.\n\r" , ch );
       send_to_char ( "You will have to eliminate all enemy forces before you can capture it.\n\r" , ch );
       return;
   }
      
   if ( planet->pop_support > 0 )
   {
       send_to_char ( "The population is not in favour of changing leaders right now.\n\r" , ch );
       return;
   }
   
   for ( cPlanet = first_planet ; cPlanet ; cPlanet = cPlanet->next )
        if ( clan == cPlanet->governed_by )
        {
            pCount++;
            support += cPlanet->pop_support;
        }
   
   if ( support < 0 )
   {
       send_to_char ( "There is not enough popular support for your organization!\n\rTry improving loyalty on the planets that you already control.\n\r" , ch );
       return;
   }
   
   planet->governed_by = clan;
   planet->pop_support = 50;
   
   sprintf( buf , "%s has been captured by %s!", planet->name, clan->name );   
   echo_to_all( AT_RED , buf , 0 );
   
   save_planet( planet );
      
   return; 
}

long get_taxes( PLANET_DATA *planet )
{
      long gain;
      int resource;
      int bigships;
            
      resource = planet->wilderness;
      resource = UMIN( resource , planet->population );
      
      gain = 500*planet->population;
      gain += 10000*resource;
      gain += planet->pop_support*100; 
      
      gain -= 5000 * planet->barracks;
      gain -= 10000 * planet->controls;
      
      bigships = planet->controls/5;  /* 100k for destroyers, 1 mil for battleships */
      gain -= 50000 * bigships;
      
      return gain;
}

int max_population( PLANET_DATA *planet )
{
     int support;
     int pmax;
     int rmax;
     
     support = (planet->pop_support + 200) / 3;
      
     pmax = planet->citysize;
     rmax = planet->wilderness/5 + 5*planet->farmland;
     
     pmax = pmax * support / 100;
     
     return UMIN( rmax , pmax );
     
}
