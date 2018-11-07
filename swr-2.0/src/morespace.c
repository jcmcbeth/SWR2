#include <math.h> 
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

SHIP_PROTOTYPE * first_ship_prototype;
SHIP_PROTOTYPE * last_ship_prototype;

/* 
   Its important that the roomnumbers on this list matches the designs in 
   create_ship_rooms. Very bad things will happen if they don't. 
*/
    
const	struct	model_type	model	[MAX_SHIP_MODEL+1] =
{
/* name, hyperspeed, realspeed, missiles, lasers, tractorbeam, manuever, energy, shield, hull, rooms */
    { "Short Range Fighter", 	0,   255, 10,  4,  0, 255, 2500,  50,   250,   1 },  /* FIGHTER1  */
    { "Shuttle", 		150, 200, 0,   1,  0, 100, 2500,  50,   250,   2 },  /* SHUTTLE1  */
    { "Transport", 		100, 100, 0,   2,  0, 75,  3500,  100,  1000,  4 },  /* TRANSPORT1  */
    { "Long Range Fighter", 	100, 150, 10,  4,  0, 150, 3500,  100,  500,   1 },  /* FIGHTER2  */
    { "Assault Shuttle", 	150, 100, 0,   2,  0, 100, 3500,  100,  500,   3 },  /* SHUTTLE2  */
    { "Assault Transport", 	150, 75,  10,  2,  0, 75,  5000,  200,  1500,  5 },  /* TRANSPORT2  */
    { "Corvette", 		150, 100, 20,  6,  0, 100, 7500,  200,  2500,  8 },  /* CORVETTE  */
    { "Frigate" , 		150, 85,  30,  6,  0, 85,  10000, 250,  5000,  10 },  /* FRIGATE  */
    { "Destroyer", 		150, 70,  50,  8,  0, 70,  15000, 350,  10000, 14 },  /* DESTROYER  */
    { "Cruiser", 		200, 50,  100, 8,  0, 50,  20000, 500,  20000, 15 },  /* CRUISER  */
    { "Battlecruiser", 		200, 35,  150, 10, 0, 35,  30000, 750,  25000, 18 },  /* BATTLESHIP  */
    { "Flagship", 		250, 25,  200, 10, 0, 25,  35000, 1000, 30000, 24 },  /* FLAGSHIP  */
    { "Custom Ship", 		255, 255,  200, 10, 0, 255,  35000, 1000, 30000, MAX_SHIP_ROOMS }  /* CUSTOM_SHIP  */
};

/* local routines */
void	fread_ship_prototype	args( ( SHIP_PROTOTYPE *ship, FILE *fp ) );
bool	load_ship_prototype	args( ( char *shipfile ) );
void	write_prototype_list	args( ( void ) );
void	write_ship_list	args( ( void ) );
void    bridge_rooms( ROOM_INDEX_DATA *rfrom , ROOM_INDEX_DATA *rto , int edir );
void    bridge_elevator( ROOM_INDEX_DATA *rfrom , ROOM_INDEX_DATA *rto , int edir , char * exname );
void 	post_ship_guard( ROOM_INDEX_DATA * pRoomIndex );
void    make_cockpit( ROOM_INDEX_DATA *room , SHIP_DATA *ship );
void    make_turret( ROOM_INDEX_DATA *room , SHIP_DATA *ship );
void    make_bridge( ROOM_INDEX_DATA *room , SHIP_DATA *ship );
void    make_pilot( ROOM_INDEX_DATA *room , SHIP_DATA *ship );
void    make_copilot( ROOM_INDEX_DATA *room , SHIP_DATA *ship );
void    make_engine( ROOM_INDEX_DATA *room , SHIP_DATA *ship );
void    make_medical( ROOM_INDEX_DATA *room , SHIP_DATA *ship );
void    make_lounge( ROOM_INDEX_DATA *room , SHIP_DATA *ship );
void    make_entrance( ROOM_INDEX_DATA *room , SHIP_DATA *ship );
void    make_hanger( ROOM_INDEX_DATA *room , SHIP_DATA *ship );
void    make_garage( ROOM_INDEX_DATA *room , SHIP_DATA *ship );
void    make_elevator( ROOM_INDEX_DATA *room , SHIP_DATA *ship );
long    get_design_value( int hull, int energy, int shield, int speed, int manuever, int lasers, int missiles, int chaff, int smodel );

long int get_prototype_value( SHIP_PROTOTYPE *prototype )
{
     long int price;
          
     if (prototype->class == SPACECRAFT)
        price = 10000;
     else if (prototype->class == AIRCRAFT) 
        price = 5000;
     else if (prototype->class == SUBMARINE) 
        price = 5000;
     else if (prototype->class == SPACE_STATION) 
        price = 100000;
     else 
        price = 2000;
     
     price += ( prototype->tractorbeam * 100 );
     price += ( prototype->realspeed * 10 );
     price += ( 5 * prototype->maxhull );
     price += ( 2 * prototype->maxenergy );
     price += ( 100 * prototype->maxchaff );
               
     if (prototype->maxenergy > 5000 )
          price += ( (prototype->maxenergy-5000)*20 ) ;
     
     if (prototype->maxenergy > 10000 )
          price += ( (prototype->maxenergy-10000)*50 );
     
     if (prototype->maxhull > 1000)
        price += ( (prototype->maxhull-1000)*10 );
     
     if (prototype->maxhull > 10000)
        price += ( (prototype->maxhull-10000)*20 );
        
     if (prototype->maxshield > 200)
          price += ( (prototype->maxshield-200)*50 );
     
     if (prototype->maxshield > 1000)
          price += ( (prototype->maxshield-1000)*100 );
     
     if (prototype->realspeed > 100 )
        price += ( (prototype->realspeed-100)*500 ) ;
        
     if (prototype->lasers > 5 )
        price += ( (prototype->lasers-5)*500 );
      
     if (prototype->maxshield)
     	price += ( 1000 + 10 * prototype->maxshield);
     
     if (prototype->lasers)
     	price += ( 500 + 500 * prototype->lasers );
    
     if (prototype->maxmissiles)
     	price += ( 1000 + 250 * prototype->maxmissiles );
     
     if (prototype->hyperspeed)
        price += ( 1000 + prototype->hyperspeed * 10 );
     
     price *= (int) ( 1 + prototype->model/3 );
     
     return price;
                    
}

long int get_design_value( int hull, int energy, int shield, int speed, int manuever, int lasers, int missiles, int chaff, int smodel )
{
     long int price;
          
     price = 10000;
     
     price += ( speed * 10 );
     price += ( 5 * hull );
     price += ( 2 * energy );
     price += ( 100 * chaff );
               
     if ( energy > 5000 )
          price += ( (energy-5000)*20 ) ;
     
     if (energy > 10000 )
          price += ( (energy-10000)*50 );
     
     if (hull > 1000)
        price += ( (hull-1000)*10 );
     
     if (hull > 10000)
        price += ( (hull-10000)*20 );
        
     if (shield > 200)
          price += ( (shield-200)*50 );
     
     if (shield > 1000)
          price += ( (shield-1000)*100 );
     
     if (speed > 100 )
        price += ( (speed-100)*500 ) ;
        
     if (lasers > 5 )
        price += ( (lasers-5)*500 );
      
     if (shield)
     	price += ( 1000 + 10 * shield);
     
     if (lasers)
     	price += ( 500 + 500 * lasers );
    
     if (missiles)
     	price += ( 1000 + 250 * missiles );
     
     if (model[smodel].hyperspeed)
        price += ( 1000 + model[smodel].hyperspeed * 10 );
     
     price *= (int) ( 1 + smodel/3 );
     
     return price;
                    
}

void write_prototype_list( )
{
    SHIP_PROTOTYPE *prototype;
    FILE *fpout;
    char filename[256];
    
    sprintf( filename, "%s%s", PROTOTYPE_DIR, PROTOTYPE_LIST );
    fpout = fopen( filename, "w" );
    if ( !fpout )
    {
         bug( "FATAL: cannot open protoytpe.lst for writing!\n\r", 0 );
         return;
    }
    for ( prototype = first_ship_prototype; prototype; prototype = prototype->next )
    fprintf( fpout, "%s\n", prototype->filename );
    fprintf( fpout, "$\n" );
    fclose( fpout );
}
                                                                    
SHIP_PROTOTYPE *get_ship_prototype( char *name )
{
    SHIP_PROTOTYPE *prototype;
    
    for ( prototype = first_ship_prototype; prototype; prototype = prototype->next )
       if ( !str_cmp( name, prototype->name ) )
         return prototype;
    
    for ( prototype = first_ship_prototype; prototype; prototype = prototype->next )
       if ( nifty_is_name_prefix( name, prototype->name ) )
         return prototype;
    
    return NULL;
}

void save_ship_prototype( SHIP_PROTOTYPE *prototype )
{
    FILE *fp;
    char filename[256];
    char buf[MAX_STRING_LENGTH];

    if ( !prototype )
    {
	bug( "save_ship_prototype: null prototype pointer!", 0 );
	return;
    }
        
    if ( !prototype->filename || prototype->filename[0] == '\0' )
    {
	sprintf( buf, "save_ship_prototype: %s has no filename", prototype->name );
	bug( buf, 0 );
	return;
    }
 
    sprintf( filename, "%s%s", PROTOTYPE_DIR, prototype->filename );
    
    fclose( fpReserve );
    if ( ( fp = fopen( filename, "w" ) ) == NULL )
    {
    	bug( "save_ship_prototype: fopen", 0 );
    	perror( filename );
    }
    else
    {
	fprintf( fp, "#PROTOTYPE\n" );
	fprintf( fp, "Name         %s~\n",	prototype->name		);
	fprintf( fp, "Filename     %s~\n",	prototype->filename		);
        fprintf( fp, "Description  %s~\n",	prototype->description	);
	fprintf( fp, "Class        %d\n",	prototype->class	);
	fprintf( fp, "Model        %d\n",	prototype->model	);
	fprintf( fp, "Tractorbeam  %d\n",	prototype->tractorbeam	);
	fprintf( fp, "Lasers       %d\n",	prototype->lasers    	);
	fprintf( fp, "Maxmissiles  %d\n",	prototype->maxmissiles	);
	fprintf( fp, "Maxshield    %d\n",	prototype->maxshield		);
	fprintf( fp, "Maxhull      %d\n",	prototype->maxhull		);
	fprintf( fp, "Maxenergy    %d\n",	prototype->maxenergy		);
	fprintf( fp, "Hyperspeed   %d\n",	prototype->hyperspeed	);
	fprintf( fp, "Maxchaff     %d\n",	prototype->maxchaff		);
	fprintf( fp, "Realspeed    %d\n",	prototype->realspeed		);
	fprintf( fp, "Manuever     %d\n",       prototype->manuever          );
	fprintf( fp, "End\n\n"						);
	fprintf( fp, "#END\n"						);
    }
    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}


/*
 * Read in actual prototype data.
 */

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

void fread_ship_prototype( SHIP_PROTOTYPE *prototype, FILE *fp )
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
        
        
        case 'C':
             KEY( "Class",       prototype->class,            fread_number( fp ) );
             break;
                                

	case 'D':
	    KEY( "Description",	prototype->description,	fread_string( fp ) );
	    break;

	case 'E':
	    if ( !str_cmp( word, "End" ) )
	    {
		if (!prototype->name)
		  prototype->name		= STRALLOC( "" );
		if (!prototype->description)
		  prototype->description 	= STRALLOC( "" );
		return;
	    }
	    break;
	    
	case 'F':
	    KEY( "Filename",	prototype->filename,		fread_string_nohash( fp ) );
            break;
        
        case 'H':
            KEY( "Hyperspeed",   prototype->hyperspeed,       fread_number( fp ) );
            break;

        case 'L':
            KEY( "Lasers",   prototype->lasers,      fread_number( fp ) );
            break;

        case 'M':
            KEY( "Model",   prototype->model,      fread_number( fp ) );
            KEY( "Manuever",   prototype->manuever,      fread_number( fp ) );
            KEY( "Maxmissiles",   prototype->maxmissiles,      fread_number( fp ) );
            KEY( "Maxshield",      prototype->maxshield,        fread_number( fp ) );
            KEY( "Maxenergy",      prototype->maxenergy,        fread_number( fp ) );
            KEY( "Maxhull",      prototype->maxhull,        fread_number( fp ) );
            KEY( "Maxchaff",       prototype->maxchaff,      fread_number( fp ) );
             break;

	case 'N':
	    KEY( "Name",	prototype->name,		fread_string( fp ) );
            break;
  
        case 'R':
            KEY( "Realspeed",   prototype->realspeed,       fread_number( fp ) );
            break;
       
	case 'T':
	    KEY( "Tractorbeam", prototype->tractorbeam,      fread_number( fp ) );
	    break;
	}
	
	if ( !fMatch )
	{
	    sprintf( buf, "Fread_ship_prototype: no match: %s", word );
	    bug( buf, 0 );
	}
    }
}

/*
 * Load a prototype file
 */

bool load_ship_prototype( char *prototypefile )
{
    char filename[256];
    SHIP_PROTOTYPE *prototype;
    FILE *fp;
    bool found;
        
    CREATE( prototype, SHIP_PROTOTYPE, 1 );

    found = FALSE;
    sprintf( filename, "%s%s", PROTOTYPE_DIR, prototypefile );

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
		bug( "Load_ship_prototype: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );
	    if ( !str_cmp( word, "PROTOTYPE"	) )
	    {
	    	fread_ship_prototype( prototype, fp );
	    	break;
	    }
	    else
	    if ( !str_cmp( word, "END"	) )
	        break;
	    else
	    {
		char buf[MAX_STRING_LENGTH];

		sprintf( buf, "Load_ship_prototype: bad section: %s.", word );
		bug( buf, 0 );
		break;
	    }
	}
	fclose( fp );
    }
    if ( !(found) )
      DISPOSE( prototype );
    else
    {      
       LINK( prototype, first_ship_prototype, last_ship_prototype, next, prev );
    }
    
    return found;
}

/*
 * Load in all the prototype files.
 */
void load_prototypes( )
{
    FILE *fpList;
    char *filename;
    char prototypelist[256];
    char buf[MAX_STRING_LENGTH];
    
    
    first_ship_prototype	= NULL;
    last_ship_prototype	= NULL;
    
    log_string( "Loading ship prototypes..." );

    sprintf( prototypelist, "%s%s", PROTOTYPE_DIR, PROTOTYPE_LIST );
    fclose( fpReserve );
    if ( ( fpList = fopen( prototypelist, "r" ) ) == NULL )
    {
	perror( prototypelist );
	exit( 1 );
    }

    for ( ; ; )
    {
    
	filename = feof( fpList ) ? "$" : fread_word( fpList );

	if ( filename[0] == '$' )
	  break;
	         
	if ( !load_ship_prototype( filename ) )
	{
	  sprintf( buf, "Cannot load ship prototype file: %s", filename );
	  bug( buf, 0 );
	}

    }
    fclose( fpList );
    log_string(" Done ship prototypes " );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}

void do_setprototype( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    SHIP_PROTOTYPE *prototype;
    
    if ( IS_NPC(ch) || !ch->pcdata )
    	return;

    if ( !ch->desc )
	return;

    switch( ch->substate )
    {
	default:
	  break;
	case SUB_SHIPDESC:
	  prototype = ch->dest_buf;
	  if ( !prototype )
	  {
		bug( "setprototype: sub_shipdesc: NULL ch->dest_buf", 0 );
		stop_editing( ch );
     	        ch->substate = ch->tempnum;
		send_to_char( "&RError: prototype lost.\n\r" , ch);
		return;
	  }
	  STRFREE( prototype->description );
	  prototype->description = copy_buffer( ch );
	  stop_editing( ch );
	  ch->substate = ch->tempnum;
          save_ship_prototype( prototype );
	  return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg1[0] == '\0' )
    {
	send_to_char( "Usage: setprototype <prototype> <field> <values>\n\r", ch );
	send_to_char( "\n\rField being one of:\n\r", ch );
	send_to_char( "filename name description class model\n\r", ch );
	send_to_char( "manuever speed hyperspeed tractorbeam\n\r", ch );
	send_to_char( "lasers missiles shield hull energy chaff\n\r", ch );
	return;
    }

    prototype = get_ship_prototype( arg1 );
    if ( !prototype )
    {
	send_to_char( "No such ship prototype.\n\r", ch );
	return;
    }
    
    if ( !str_cmp( arg2, "name" ) )
    {
	STRFREE( prototype->name );
	prototype->name = STRALLOC( argument );
	send_to_char( "Done.\n\r", ch );
	save_ship_prototype( prototype );
	return;
    }

    if ( !str_cmp( arg2, "filename" ) )
    {
	DISPOSE( prototype->filename );
	prototype->filename = str_dup( argument );
	send_to_char( "Done.\n\r", ch );
	save_ship_prototype( prototype );
	write_prototype_list( );
	return;
    }
 
    if ( !str_cmp( arg2, "description" ) )
    {
        ch->substate = SUB_SHIPDESC;
        ch->dest_buf = prototype;
        start_editing( ch, prototype->description );
	return;
    }

    if ( !str_cmp( arg2, "manuever" ) )
    {   
	prototype->manuever = URANGE( 0, atoi(argument) , 120 );
	send_to_char( "Done.\n\r", ch );
	save_ship_prototype( prototype );
	return;
    }

    if ( !str_cmp( arg2, "lasers" ) )
    {   
	prototype->lasers = URANGE( 0, atoi(argument) , 10 );
	send_to_char( "Done.\n\r", ch );
	save_ship_prototype( prototype );
	return;
    }
    
    if ( !str_cmp( arg2, "class" ) )
    {   
	prototype->class = URANGE( 0, atoi(argument) , 9 );
	send_to_char( "Done.\n\r", ch );
	save_ship_prototype( prototype );
	return;
    }

    if ( !str_cmp( arg2, "model" ) )
    {   
	prototype->model = URANGE( 0, atoi(argument) , 9 );
	send_to_char( "Done.\n\r", ch );
	save_ship_prototype( prototype );
	return;
    }

    if ( !str_cmp( arg2, "missiles" ) )
    {   
	prototype->maxmissiles = URANGE( 0, atoi(argument) , 255 );
	send_to_char( "Done.\n\r", ch );
	save_ship_prototype( prototype );
	return;
    }

    if ( !str_cmp( arg2, "speed" ) )
    {   
	prototype->realspeed = URANGE( 0, atoi(argument) , 150 );
	send_to_char( "Done.\n\r", ch );
	save_ship_prototype( prototype );
	return;
    }
 
    if ( !str_cmp( arg2, "tractorbeam" ) )
    {   
	prototype->tractorbeam = URANGE( 0, atoi(argument) , 255 );
	send_to_char( "Done.\n\r", ch );
	save_ship_prototype( prototype );
	return;
    }
 
    if ( !str_cmp( arg2, "hyperspeed" ) )
    {   
	prototype->hyperspeed = URANGE( 0, atoi(argument) , 255 );
	send_to_char( "Done.\n\r", ch );
	save_ship_prototype( prototype );
	return;
    }
 
    if ( !str_cmp( arg2, "shield" ) )
    {   
	prototype->maxshield = URANGE( 0, atoi(argument) , 1000 );
	send_to_char( "Done.\n\r", ch );
	save_ship_prototype( prototype );
	return;
    }
 
    if ( !str_cmp( arg2, "hull" ) )
    {   
	prototype->maxhull = URANGE( 1, atoi(argument) , 20000 );
	send_to_char( "Done.\n\r", ch );
	save_ship_prototype( prototype );
	return;
    }
 
    if ( !str_cmp( arg2, "energy" ) )
    {   
	prototype->maxenergy = URANGE( 1, atoi(argument) , 30000 );
	send_to_char( "Done.\n\r", ch );
	save_ship_prototype( prototype );
	return;
    }
 
    if ( !str_cmp( arg2, "chaff" ) )
    {   
	prototype->maxchaff = URANGE( 0, atoi(argument) , 25 );
	send_to_char( "Done.\n\r", ch );
	save_ship_prototype( prototype );
	return;
    }
 
    do_setprototype( ch, "" );
    return;
}

void do_showprototype( CHAR_DATA *ch, char *argument )
{   
    SHIP_PROTOTYPE *prototype;

    if ( IS_NPC( ch ) )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Usage: showprototype <prototype>\n\r", ch );
	return;
    }

    prototype = get_ship_prototype( argument );
    if ( !prototype )
    {
	send_to_char( "No such prototype.\n\r", ch );
	return;
    }
    set_char_color( AT_YELLOW, ch );
    ch_printf( ch, "%s : %s\n\rFilename: %s\n\r",
		        prototype->class == SPACECRAFT ? model[prototype->model].name :
		       (prototype->class == SPACE_STATION ? "Space Station" : 
		       (prototype->class == AIRCRAFT ? "Aircraft" : 
		       (prototype->class == BOAT ? "Boat" : 
		       (prototype->class == SUBMARINE ? "Submatine" : 
		       (prototype->class == LAND_VEHICLE ? "land vehicle" : "Unknown" ) ) ) ) ), 
    			prototype->name,
    			prototype->filename);
    ch_printf( ch, "Description: %s\n\r",
    			prototype->description);
    ch_printf( ch, "Tractor Beam: %d  ",
    			prototype->tractorbeam);
    ch_printf( ch, "Lasers: %d     ",
    			prototype->lasers );		
    ch_printf( ch, "Missiles: %d\n\r",
    			prototype->maxmissiles);
    ch_printf( ch, "Hull: %d     ",
    		        prototype->maxhull);	
    ch_printf( ch, "Shields: %d   Energy(fuel): %d   Chaff: %d\n\r",
    		        prototype->maxshield,
    		        prototype->maxenergy,
    		        prototype->maxchaff);
    ch_printf( ch, "Speed: %d    Hyperspeed: %d   Manueverability: %d\n\r",
                        prototype->realspeed, prototype->hyperspeed , prototype->manuever );                    
    return;
}

void do_makeship( CHAR_DATA *ch, char *argument )
{
    SHIP_PROTOTYPE *prototype;
    char arg[MAX_INPUT_LENGTH];
    
    argument = one_argument( argument, arg );
    
    if ( !argument || argument[0] == '\0' )
    {
	send_to_char( "Usage: makeship <filename> <prototype name>\n\r", ch );
	return;
    }

    CREATE( prototype, SHIP_PROTOTYPE, 1 );
    LINK( prototype, first_ship_prototype, last_ship_prototype, next, prev );

    prototype->name		= STRALLOC( argument );
    prototype->description	= STRALLOC( "" );

    prototype->filename = str_dup( arg );
    save_ship_prototype( prototype );
    write_prototype_list( );
	
}

SHIP_DATA * make_ship( SHIP_PROTOTYPE *prototype )
{
    SHIP_DATA *ship;
    int shipreg=0;
    char filename[10];
    char shipname[MAX_STRING_LENGTH];
    int dIndex;
    
    if ( prototype == NULL )
       return NULL;

    for ( ship = first_ship ; ship ; ship = ship->next )
        if ( shipreg < atoi( ship->filename ) )
             shipreg = atoi( ship->filename );

    shipreg++;
    sprintf( filename , "%d" , shipreg );
    sprintf( shipname , "%s %d" , prototype->name, shipreg );
    
    CREATE( ship, SHIP_DATA, 1 );
    LINK( ship, first_ship, last_ship, next, prev );
    
    ship->filename = str_dup( filename ) ;
    
    ship->next_in_starsystem = NULL;
    ship->prev_in_starsystem =NULL;
    ship->next_in_room = NULL;
    ship->prev_in_room = NULL;
    ship->in_room = NULL;
    ship->starsystem = NULL;
    ship->first_hanger = NULL;
    ship->last_hanger = NULL;
    ship->first_turret = NULL;
    ship->last_turret = NULL;
    ship->name = STRALLOC( shipname );
    ship->home = STRALLOC("");
    for ( dIndex = 0 ; dIndex < MAX_SHIP_ROOMS ; dIndex++ );
    	ship->description[dIndex] = NULL;
    if ( prototype->description )
        ship->description[0] = STRALLOC( prototype->description );
    ship->owner = STRALLOC("");
    ship->pilot = STRALLOC("");
    ship->copilot = STRALLOC("");
    ship->dest = NULL;
    ship->type = PLAYER_SHIP;
    ship->class = prototype->class;
    ship->model = prototype->model;
    ship->hyperspeed = prototype->hyperspeed;
    ship->realspeed = prototype->realspeed;
    ship->shipstate = SHIP_DOCKED;
    ship->laserstate = LASER_READY;
    ship->missilestate = MISSILE_READY;
    ship->missiles = prototype->maxmissiles;
    ship->maxmissiles = prototype->maxmissiles;
    ship->lasers = prototype->lasers;
    ship->tractorbeam = prototype->tractorbeam;
    ship->manuever = prototype->manuever;
    ship->hatchopen = FALSE;
    ship->autorecharge = FALSE;
    ship->autotrack = FALSE;
    ship->autospeed = FALSE;
    ship->maxenergy = prototype->maxenergy;
    ship->energy = prototype->maxenergy;
    ship->shield = 0;
    ship->maxshield = prototype->maxshield;
    ship->hull = prototype->maxhull;
    ship->maxhull = prototype->maxhull;
    ship->location = 0;
    ship->lastdoc = 0;
    ship->shipyard = 0;
    ship->collision = 0;
    ship->target = NULL;
    ship->currjump = NULL;
    ship->chaff = prototype->maxchaff;
    ship->maxchaff = prototype->maxchaff;
    ship->chaff_released = FALSE;
    ship->autopilot = FALSE;
    create_ship_rooms ( ship );
    
    save_ship( ship );
    write_ship_list( );
	
    return ship;
}

void do_prototypes( CHAR_DATA *ch, char *argument )
{
  SHIP_PROTOTYPE *prototype;
  int count;

  if ( str_cmp ( argument , "vehicles" ) ) 
  {  
    count = 0;
    send_to_pager( "&Y\n\rThe following ships are currently prototyped:&W\n\r", ch );
    for ( prototype = first_ship_prototype; prototype; prototype = prototype->next )
    {   
        if ( prototype->class > SPACE_STATION )
           continue;
           
        pager_printf( ch, "%-35s    ", prototype->name );
        pager_printf( ch, "%ld to buy.\n\r", get_prototype_value(prototype) ); 
        
        count++;
      }
    
      if ( !count )
      {
        send_to_pager( "There are no ship prototypes currently formed.\n\r", ch );
	return;
      }
 }   

  if ( str_cmp ( argument , "ships" ) ) 
  {  
    count = 0;
    send_to_pager( "&Y\n\rThe following vehicles are currently prototyped:&W\n\r", ch );
        
    for ( prototype = first_ship_prototype; prototype; prototype = prototype->next )
    {   
        if ( prototype->class <= SPACE_STATION )
           continue;

        pager_printf( ch, "%-35s    ", prototype->name );
        pager_printf( ch, "%ld to buy.\n\r", get_prototype_value(prototype) ); 
        
        count++;
      }
    
      if ( !count )
      {
        send_to_pager( "There are no vehicle prototypes currently formed.\n\r", ch );
	return;
      }
 }   

}

void create_ship_rooms( SHIP_DATA * ship )
{
    int roomnum, numrooms;
    ROOM_INDEX_DATA * room[24];
    
    if ( ship->class != SPACECRAFT )
        ship->model = 0;
    
    numrooms = UMIN( model[ship->model].rooms , MAX_SHIP_ROOMS-1 );
       
    for ( roomnum = 0 ; roomnum < numrooms ; roomnum++ )
        room[roomnum] = make_ship_room( ship );

    switch( ship->model )
    {
    default:
         ship->pilotseat = room[0];
         ship->gunseat = room[0];
         ship->viewscreen = room[0];
         ship->entrance = room[0];
         ship->engine = room[0];
         break;
    
    case FIGHTER1:
         ship->pilotseat = room[0];
         ship->gunseat = room[0];
         ship->viewscreen = room[0];
         room[0]->tunnel = 1;
         ship->entrance = room[0];
         ship->engine = room[0];
         break;

    case FIGHTER2:
         ship->pilotseat = room[0];
         ship->gunseat = room[0];
         ship->viewscreen = room[0];
         room[0]->tunnel = 2;
         ship->entrance = room[0];
         ship->engine = room[0];
         break;
         
    case SHUTTLE1:
         bridge_rooms( room[1] , room[0] , DIR_NORTH );
         make_cockpit( room[0] , ship );
         ship->entrance = room[0];
         ship->engine = room[0];
         break;

    case SHUTTLE2:
         bridge_rooms( room[1] , room[0] , DIR_NORTH );
         bridge_rooms( room[1] , room[2] , DIR_SOUTH );
         make_cockpit( room[0], ship);
         make_turret( room[2], ship );
         ship->entrance = room[0];
         ship->engine = room[0];
         break;

    case TRANSPORT1:
         bridge_rooms( room[1] , room[0] , DIR_NORTH );
         bridge_rooms( room[1] , room[2] , DIR_SOUTH );
         bridge_rooms( room[1] , room[3] , DIR_WEST );
         make_cockpit( room[0], ship );
         make_engine( room[2], ship );
         make_entrance( room[1], ship );
         break;

    case TRANSPORT2:
         bridge_rooms( room[1] , room[0] , DIR_NORTH );
         bridge_rooms( room[1] , room[2] , DIR_SOUTH );
         bridge_rooms( room[1] , room[3] , DIR_WEST );
         bridge_rooms( room[3] , room[4] , DIR_DOWN );
         make_cockpit( room[0], ship );
         make_engine( room[2], ship );
         make_entrance( room[1], ship );
         make_turret( room[4], ship );
         break;

    case CORVETTE:
         make_pilot( room[0], ship );
         make_bridge( room[1], ship );
         make_copilot( room[2], ship );
         make_engine( room[5], ship );
         make_entrance( room[3], ship );
         make_turret( room[6], ship );
         make_turret( room[7], ship );
         bridge_rooms( room[0] , room[1] , DIR_EAST );
         bridge_rooms( room[1] , room[2] , DIR_EAST );
         bridge_rooms( room[1] , room[3] , DIR_SOUTH );
         bridge_rooms( room[3] , room[4] , DIR_SOUTH );
         bridge_rooms( room[4] , room[5] , DIR_SOUTH );
         bridge_rooms( room[4] , room[6] , DIR_WEST );
         bridge_rooms( room[4] , room[7] , DIR_EAST );
         post_ship_guard( room[1] );
         break;

    case FRIGATE:
         make_pilot( room[0], ship );
         make_bridge( room[1], ship );
         make_copilot( room[2], ship );
         make_engine( room[9], ship );
         make_entrance( room[6], ship );
         make_turret( room[3], ship );
         make_turret( room[5], ship );
         make_lounge( room[8], ship );
         bridge_rooms( room[0] , room[1] , DIR_EAST );
         bridge_rooms( room[1] , room[2] , DIR_EAST );
         bridge_rooms( room[3] , room[4] , DIR_EAST );
         bridge_rooms( room[4] , room[5] , DIR_EAST );
         bridge_rooms( room[7] , room[8] , DIR_EAST );
         bridge_rooms( room[6] , room[7] , DIR_EAST );
         bridge_rooms( room[1] , room[4] , DIR_SOUTH );
         bridge_rooms( room[4] , room[7] , DIR_SOUTH );
         bridge_rooms( room[7] , room[9] , DIR_SOUTH );
         post_ship_guard( room[1] );
         break;

    case DESTROYER:
         make_pilot( room[0], ship );
         make_bridge( room[1], ship );
         make_copilot( room[2], ship );
         make_engine( room[9], ship );
         make_entrance( room[10], ship );
         make_turret( room[3], ship );
         make_turret( room[5], ship );
         make_garage( room[8], ship );
         make_hanger( room[6], ship );
         make_lounge( room[11], ship );
         bridge_rooms( room[0] , room[1] , DIR_EAST );
         bridge_rooms( room[1] , room[2] , DIR_EAST );
         bridge_rooms( room[3] , room[4] , DIR_EAST );
         bridge_rooms( room[4] , room[5] , DIR_EAST );
         bridge_rooms( room[7] , room[8] , DIR_EAST );
         bridge_rooms( room[6] , room[7] , DIR_EAST );
         bridge_rooms( room[1] , room[4] , DIR_SOUTH );
         bridge_rooms( room[4] , room[7] , DIR_SOUTH );
         bridge_rooms( room[7] , room[9] , DIR_SOUTH );
         bridge_rooms( room[7] , room[10] , DIR_DOWN );
         bridge_rooms( room[7] , room[11] , DIR_UP );
         post_ship_guard( room[1] );
         break;
    
    case CRUISER:
         make_pilot( room[0], ship );
         make_bridge( room[1], ship );
         make_copilot( room[2], ship );
         make_engine( room[7], ship );
         make_entrance( room[5], ship );
         make_turret( room[11], ship );
         make_turret( room[12], ship );
         make_turret( room[13], ship );
         make_turret( room[14], ship );
         make_garage( room[10], ship );
         make_hanger( room[9], ship );
         make_lounge( room[4], ship );
         bridge_rooms( room[0] , room[1] , DIR_EAST );
         bridge_rooms( room[1] , room[2] , DIR_EAST );
         bridge_rooms( room[1] , room[3] , DIR_DOWN );
         bridge_rooms( room[3] , room[4] , DIR_DOWN );
         bridge_rooms( room[3] , room[5] , DIR_NORTH );
         bridge_rooms( room[5] , room[6] , DIR_SOUTHWEST );
         bridge_rooms( room[6] , room[7] , DIR_SOUTHEAST );
         bridge_rooms( room[7] , room[8] , DIR_NORTHEAST );
         bridge_rooms( room[8] , room[5] , DIR_NORTHWEST );
         bridge_rooms( room[6] , room[9] , DIR_WEST );
         bridge_rooms( room[8] , room[10] , DIR_EAST );
         bridge_rooms( room[6] , room[12] , DIR_SOUTHWEST );
         bridge_rooms( room[8] , room[14] , DIR_SOUTHEAST );
         bridge_rooms( room[8] , room[13] , DIR_NORTHEAST );
         bridge_rooms( room[6] , room[11] , DIR_NORTHWEST );
         post_ship_guard( room[1] );
         post_ship_guard( room[1] );
         break;

    case BATTLESHIP:
         make_pilot( room[0], ship );
         make_bridge( room[1], ship );
         make_copilot( room[2], ship );
         make_engine( room[11], ship );
         make_elevator( room[16], ship );
         make_turret( room[3], ship );
         make_turret( room[4], ship );
         make_turret( room[5], ship );
         make_turret( room[6], ship );
         make_turret( room[7], ship );
         make_turret( room[8], ship );
         make_garage( room[12], ship );
         make_hanger( room[13], ship );
         make_hanger( room[15], ship );
         make_lounge( room[9], ship );
         make_medical( room[10], ship );
         bridge_rooms( room[0] , room[1] , DIR_EAST );
         bridge_rooms( room[1] , room[2] , DIR_EAST );
         bridge_rooms( room[17] , room[3] , DIR_NORTHWEST );
         bridge_rooms( room[17] , room[4] , DIR_NORTHEAST );
         bridge_rooms( room[17] , room[5] , DIR_EAST );
         bridge_rooms( room[17] , room[6] , DIR_SOUTHEAST );
         bridge_rooms( room[17] , room[7] , DIR_SOUTHWEST );
         bridge_rooms( room[17] , room[8] , DIR_WEST );
         bridge_rooms( room[13] , room[14] , DIR_EAST );
         bridge_rooms( room[14] , room[15] , DIR_EAST );
         bridge_rooms( room[12] , room[14] , DIR_SOUTH );
         bridge_elevator( room[1] , room[16] , DIR_SOUTH , "Navigation" );
         bridge_elevator( room[17] , room[16] , DIR_SOUTH , "Turrets" );
         bridge_elevator( room[9] , room[16] , DIR_SOUTH , "Lounge" );
         bridge_elevator( room[10] , room[16] , DIR_SOUTH , "Medical" );
         bridge_elevator( room[11] , room[16] , DIR_SOUTH , "Engineering" );
         bridge_elevator( room[14] , room[16] , DIR_SOUTH , "Hangers" );
         post_ship_guard( room[1] );
         post_ship_guard( room[1] );
         break;

    case FLAGSHIP:
         make_pilot( room[0], ship );
         make_bridge( room[1], ship );
         make_copilot( room[2], ship );
         make_engine( room[20], ship );
         make_entrance( room[7], ship );
         make_turret( room[22], ship );
         make_turret( room[23], ship );
         make_turret( room[15], ship );
         make_turret( room[16], ship );
         make_turret( room[17], ship );
         make_turret( room[18], ship );
         make_garage( room[19], ship );
         make_garage( room[21], ship );
         make_hanger( room[8], ship );
         make_hanger( room[14], ship );
         make_lounge( room[5], ship );
         make_medical( room[3], ship );
         bridge_rooms( room[0] , room[1] , DIR_EAST );
         bridge_rooms( room[1] , room[2] , DIR_EAST );
         bridge_rooms( room[3] , room[4] , DIR_EAST );
         bridge_rooms( room[4] , room[5] , DIR_EAST );
         bridge_rooms( room[8] , room[9] , DIR_EAST );
         bridge_rooms( room[9] , room[10] , DIR_EAST );
         bridge_rooms( room[10] , room[11] , DIR_EAST );
         bridge_rooms( room[11] , room[12] , DIR_EAST );
         bridge_rooms( room[12] , room[13] , DIR_EAST );
         bridge_rooms( room[13] , room[14] , DIR_EAST );
         bridge_rooms( room[1] , room[4] , DIR_SOUTH );
         bridge_rooms( room[4] , room[6] , DIR_SOUTH );
         bridge_rooms( room[6] , room[7] , DIR_SOUTH );
         bridge_rooms( room[7] , room[11] , DIR_SOUTH );
         bridge_rooms( room[11] , room[20] , DIR_SOUTH );
         bridge_rooms( room[15] , room[9] , DIR_SOUTH );
         bridge_rooms( room[9] , room[16] , DIR_SOUTH );
         bridge_rooms( room[10] , room[19] , DIR_SOUTH );
         bridge_rooms( room[17] , room[13] , DIR_SOUTH );
         bridge_rooms( room[13] , room[18] , DIR_SOUTH );
         bridge_rooms( room[12] , room[21] , DIR_SOUTH );
         post_ship_guard( room[1] );
         post_ship_guard( room[1] );
         break;
    }

    for ( roomnum = 0 ; roomnum < numrooms ; roomnum++ )
        if ( ship->description[roomnum] && ship->description[roomnum][0] != '\0' )
        {
             STRFREE ( room[roomnum]->description );
             room[roomnum]->description = STRALLOC( ship->description[roomnum] );
        }     

}

void bridge_rooms( ROOM_INDEX_DATA *rfrom , ROOM_INDEX_DATA *rto , int edir )
{
    EXIT_DATA *xit;

                 xit = make_exit( rfrom, rto, edir );
	         xit->keyword		= STRALLOC( "" );
	         xit->description	= STRALLOC( "" );
	         xit->key		= -1;
	         xit->exit_info		= 0;
                 xit = make_exit( rto , rfrom  , rev_dir[edir] );
	         xit->keyword		= STRALLOC( "" );
	         xit->description	= STRALLOC( "" );
	         xit->key		= -1;
	         xit->exit_info		= 0;
}

void bridge_elevator( ROOM_INDEX_DATA *rfrom , ROOM_INDEX_DATA *rto , int edir , char * exname )
{
    EXIT_DATA *xit;

                 xit = make_exit( rfrom, rto, edir );
	         xit->keyword		= STRALLOC( "" );
	         xit->description	= STRALLOC( "" );
	         xit->key		= -1;
	         xit->exit_info		= 0;
                 xit = make_exit( rto , rfrom  , DIR_SOMEWHERE );
	         xit->keyword		= STRALLOC( exname );
	         xit->description	= STRALLOC( "" );
	         xit->key		= -1;
	         xit->exit_info		= 526336;
}

void make_cockpit( ROOM_INDEX_DATA *room , SHIP_DATA *ship )
{
     char buf[MAX_STRING_LENGTH];

     room->tunnel = 4;

     if ( !ship->pilotseat )
       ship->pilotseat = room;
     if ( !ship->gunseat )
       ship->gunseat = room;
     if ( !ship->viewscreen )
       ship->viewscreen = room;

     STRFREE( room->name );
     room->name = STRALLOC( "The Cockpit" );

     strcpy( buf , "This small cockpit houses the pilot controls as well as the ships main\n\r" );
     strcat( buf , "offensive and defensive systems.\n\r" );
     STRFREE ( room->description );
     room->description = STRALLOC(buf);
}

void make_turret( ROOM_INDEX_DATA *room , SHIP_DATA *ship )
{
     char buf[MAX_STRING_LENGTH];
     TURRET_DATA *turret;
     
     room->tunnel = 1;

     STRFREE( room->name );
     room->name = STRALLOC( "A Laser Turret" );

     strcpy( buf , "This turbo laser turret extends from the outter hull of the ship.\n\r" );
     strcat( buf , "It is more powerful than the ships main laser batteries and must\n\r" );
     strcat( buf , "be operated manually by a trained gunner.\n\r" );
     STRFREE ( room->description );
     room->description = STRALLOC(buf);

     CREATE( turret, TURRET_DATA, 1 );
     turret->next = NULL;
     turret->prev = NULL;
     turret->room = room;
     turret->target = NULL;
     turret->laserstate =0;

     LINK( turret, ship->first_turret, ship->last_turret, next, prev );

}

void    make_bridge( ROOM_INDEX_DATA *room , SHIP_DATA *ship )
{
     char buf[MAX_STRING_LENGTH];

     ship->viewscreen = room;

     STRFREE( room->name );
     room->name = STRALLOC( "The Bridge" );

     strcpy( buf , "A large panoramic viewscreen gives you a good view of everything\n\r" );
     strcat( buf , "outside of the ship. Several computer terminals provide information\n\r" );
     strcat( buf , "and status readouts. There are also consoles dedicated to scanning\n\r" );
     strcat( buf , "communications and navigation.\n\r" );
     STRFREE ( room->description );
     room->description = STRALLOC(buf);
}

void    make_pilot( ROOM_INDEX_DATA *room , SHIP_DATA *ship )
{
     char buf[MAX_STRING_LENGTH];

     ship->pilotseat = room;
     room->tunnel = 1;

     STRFREE( room->name );
     room->name = STRALLOC( "The Pilots Chair" );

     strcpy( buf , "The console in front of you contains the spacecrafts primary flight\n\r" );
     strcat( buf , "controls. All of the ships propulsion and directional controls are located\n\r" );
     strcat( buf , "here including the hyperdrive controls.\n\r" );
     STRFREE ( room->description );
     room->description = STRALLOC(buf);
}

void    make_copilot( ROOM_INDEX_DATA *room , SHIP_DATA *ship )
{
     char buf[MAX_STRING_LENGTH];

     ship->gunseat = room;
     room->tunnel = 1;

     STRFREE( room->name );
     room->name = STRALLOC( "The Gunners Chair" );

     strcpy( buf , "This is where the action is. All of the systems main offensive and defensive\n\r" );
     strcat( buf , "controls are located here. There are targetting computers for the main laser\n\r" );
     strcat( buf , "batteries and missile launchers as well as shield controls.\n\r" );
     STRFREE ( room->description );
     room->description = STRALLOC(buf);
}

void    make_engine( ROOM_INDEX_DATA *room , SHIP_DATA *ship )
{
     char buf[MAX_STRING_LENGTH];

     ship->engine = room;

     STRFREE( room->name );
     room->name = STRALLOC( "The Engine Room" );

     strcpy( buf , "Many large mechanical parts litter this room. Throughout the room dozens\n\r" );
     strcat( buf , "of small glowing readouts and computer terminals monitor and provide information\n\r" );
     strcat( buf , "about all of the ships systems. Engineering is the heart of the ship. If\n\r" );
     strcat( buf , "something is damaged fixing it will almost always start here.\n\r" );
     STRFREE ( room->description );
     room->description = STRALLOC(buf);
}

void    make_elevator( ROOM_INDEX_DATA *room , SHIP_DATA *ship )
{
     char buf[MAX_STRING_LENGTH];

     STRFREE( room->name );
     room->name = STRALLOC( "Entrance / Lift" );

     strcpy( buf , "This spacious lift provides access to all of the ships decks as well\n\r" );
     strcpy( buf , "as doubling as the main exit.\n\r\n\r" );
     strcpy( buf , "A sign on the wall lists several lift destinations:\n\r" );
     strcat( buf , "\n\r" );
     strcat( buf , "Navigation\n\r" );
     strcat( buf , "Turrets\n\r" );
     strcat( buf , "Lounge\n\r" );
     strcat( buf , "Medical\n\r" );
     strcat( buf , "Engineering\n\r" );
     strcat( buf , "Hangers\n\r" );
     STRFREE ( room->description );
     room->description = STRALLOC(buf);
}

void    make_medical( ROOM_INDEX_DATA *room , SHIP_DATA *ship )
{
     char buf[MAX_STRING_LENGTH];

     STRFREE( room->name );
     room->name = STRALLOC( "Medical Bay " );

     strcpy( buf , "\n\r" );
     strcat( buf , "This medical bay is out of order.\n\r" );
     strcat( buf , "\n\r" );
     STRFREE ( room->description );
     room->description = STRALLOC(buf);
}

void    make_lounge( ROOM_INDEX_DATA *room , SHIP_DATA *ship )
{
     char buf[MAX_STRING_LENGTH];

     STRFREE( room->name );
     room->name = STRALLOC( "Crew Lounge" );

     strcpy( buf , "\n\r" );
     strcat( buf , "The crew lounge needs to be furnished still.\n\r" );
     strcat( buf , "\n\r" );
     STRFREE ( room->description );
     room->description = STRALLOC(buf);
}

void    make_entrance( ROOM_INDEX_DATA *room , SHIP_DATA *ship )
{
     char buf[MAX_STRING_LENGTH];
     
     ship->entrance = room;

     STRFREE( room->name );
     room->name = STRALLOC( "The Entrance Ramp" );

     strcpy( buf , "\n\r" );
     strcat( buf , "Don't you wish durga would finish his descriptions.\n\r" );
     strcat( buf , "\n\r" );
     STRFREE ( room->description );
     room->description = STRALLOC(buf);
}

void    make_hanger( ROOM_INDEX_DATA *room , SHIP_DATA *ship )
{
     char buf[MAX_STRING_LENGTH];

     STRFREE( room->name );
     room->name = STRALLOC( "A Hanger" );

     strcpy( buf , "\n\r" );
     strcat( buf , "This hanger is out of order.\n\r" );
     strcat( buf , "\n\r" );
     STRFREE ( room->description );
     room->description = STRALLOC(buf);
}

void    make_garage( ROOM_INDEX_DATA *room , SHIP_DATA *ship )
{
     char buf[MAX_STRING_LENGTH];

     STRFREE( room->name );
     room->name = STRALLOC( "Vehicle Garage" );

     strcpy( buf , "\n\r" );
     strcat( buf , "This garage is out of order.\n\r" );
     strcat( buf , "\n\r" );
     STRFREE ( room->description );
     room->description = STRALLOC(buf);
}

void do_designship( CHAR_DATA * ch , char * argument )
{
     int hull, energy, shield, speed, manuever, lasers, missiles, chaff, smodel;
     char arg1[MAX_INPUT_LENGTH];
     char arg2[MAX_INPUT_LENGTH];
     char arg3[MAX_INPUT_LENGTH];
     char arg4[MAX_INPUT_LENGTH];
     char arg5[MAX_INPUT_LENGTH];
     char arg6[MAX_INPUT_LENGTH];
     char arg7[MAX_INPUT_LENGTH];
     char arg8[MAX_INPUT_LENGTH];
     char arg9[MAX_INPUT_LENGTH];
     char arg0[MAX_INPUT_LENGTH];
     char layout[MAX_INPUT_LENGTH];
     long price;
     SHIP_DATA * ship;
     ROOM_INDEX_DATA *location;
     
    if ( IS_NPC(ch) || !ch->pcdata )
    return;

    if ( !ch->desc )
	return;

    switch( ch->substate )
    {
	default:
	  break;
	case SUB_ROOM_DESC:
	  location = ch->dest_buf;
	  if ( !location )
	  {
		bug( "designship decorate: sub_room_desc: NULL ch->dest_buf", 0 );
   	        stop_editing( ch );
	        ch->substate = ch->tempnum;
	        return;
	  }
	  STRFREE( location->description );
	  location->description = copy_buffer( ch );

	  stop_editing( ch );
	  ch->substate = ch->tempnum;

          ship = ( ship_from_room( location ) );
          if ( ship )
              save_ship( ship );

	  return;
    }
  
    if ( ch->pcdata->learned[gsn_shipdesign] <= 0 )
    {
        send_to_char( "You have no idea how to do that..\n\r" , ch);
        return;
    }
        
    if ( argument[0] == '\0' )
    {
        send_to_char( "This skill may be used in one of the following ways:\n\r" , ch);
        send_to_char( "\n\r" , ch);
        send_to_char( "DESIGNSHIP DECORATE\n\r" , ch);
        send_to_char( "DESIGNSHIP TEST      <shipstats> 'model' <prototype_name>\n\r" , ch);
        send_to_char( "DESIGNSHIP PROTOTYPE <shipstats> 'model' <prototype_name>\n\r" , ch);
        send_to_char( "DESIGNSHIP CLANSHIP  <shipstats> 'model' <ship_name>\n\r" , ch);
        send_to_char( "DESIGNSHIP PERSONAL  <shipstats> 'model' <ship_name>\n\r" , ch);
        send_to_char( "DESIGNSHIP CUSTOM    <custom design info>\n\r" , ch);
        send_to_char( "\n\r" , ch);
        send_to_char( "Shipstats are a list of numbers separated by spaces representing:\n\r" , ch);
        send_to_char( "hull energy shield speed manuever lasers missiles chaff\n\r" , ch);
        send_to_char( "\n\r" , ch);
        send_to_char( "Model may be one of the following:\n\r" , ch);
        send_to_char( "'short range fighter'   'shuttle'           'transport'\n\r" , ch);
        send_to_char( "'long range fighter'    'assault shuttle'   'assault transport'\n\r" , ch);
        send_to_char( "'corvette'              'frigate'           'destroyer'\n\r" , ch);
        send_to_char( "'cruiser'               'battlecruiser'     'flagship'\n\r" , ch);
        send_to_char( "\n\r" , ch);
        send_to_char( "For example:\n\r" , ch);
        send_to_char( "designship personal 100 1000 50 100 100 2 0 0 'shuttle' Durgas Shuttle\n\r" , ch);
        send_to_char( "\n\r" , ch);
        send_to_char( "type 'designship custom' for details on custom design.\n\r" , ch);
        return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    argument = one_argument( argument, arg4 );
    argument = one_argument( argument, arg5 );
    argument = one_argument( argument, arg6 );
    argument = one_argument( argument, arg7 );
    argument = one_argument( argument, arg8 );
    argument = one_argument( argument, arg9 );
    argument = one_argument( argument, arg0 );

    if ( !str_cmp( arg1, "decorate" ) )
    {
       if ( !ch->in_room )
          return;
          
       ship = ( ship_from_room( ch->in_room ) );
       if ( !ship )
       {
      	  send_to_char( "&RYou must be inside a ship to do that.\n\r" ,ch );
   	  return;
       }
       if ( !check_pilot( ch , ship ) )     
       {
      	  send_to_char( "&RThis isn't your ship to decorate.\n\r" ,ch );
   	  return;
       }
       ch->substate = SUB_ROOM_DESC;
       ch->dest_buf = ch->in_room;
       start_editing( ch, ch->in_room->description );
       return;
    }

    if ( !ch->in_room || !IS_SET(ch->in_room->room_flags, ROOM_SHIPYARD) )
    {
   	send_to_char( "&RYou must be in a shipyard to do that.\n\r" ,ch );
   	return;
    }

    if ( arg0[0] == '\0' )
    {
      if ( !str_cmp( arg1, "custom" ) )
      {
        send_to_char( "Rules for custom design:\n\r" , ch);
        send_to_char( "\n\r" , ch);
        send_to_char( "DESIGNSHIP CUSTOM <shipstats> <layout> <ship_name>\n\r" , ch);
        send_to_char( "\n\r" , ch);
        send_to_char( "Shipstats are a list of numbers separated by spaces representing:\n\r" , ch);
        send_to_char( "hull energy shield speed manuever lasers missiles chaff\n\r" , ch);
        send_to_char( "\n\r" , ch);
        send_to_char( "Layout is a string of letters that a robot uses as commands to build your\n\r" , ch);
        send_to_char( "ship. The robot starts at the entrance and uses the following commands:\n\r" , ch);
        send_to_char( "\n\r" , ch);
        send_to_char( "n, e, w, s, u, d - move robot that direction and create a new room if needed\n\r" , ch);
        send_to_char( "c - make current room cockpit,          b - make current room bridge\n\r" , ch);
        send_to_char( "p - make current room pilot chair,      2 - make current room copilot chair\n\r" , ch);
        send_to_char( "m - make current room engin room,       + - make current room medical bay\n\r" , ch);
        send_to_char( "l - make current room crew lounge,      t - make current room turret\n\r" , ch);
        send_to_char( "h - make current room hanger,           v - make current room vehicle garage\n\r" , ch);
        send_to_char( "\n\r" , ch);
        send_to_char( "For example:\n\r" , ch);
        send_to_char( "designship custom 100 1000 50 100 100 2 0 0 endtunwncswsdtusw Durgas Shuttle\n\r" , ch);
        send_to_char( "\n\r" , ch);
        send_to_char( "(would make a square shuttle, with a rear entrance, a cockpit at the front\n\rand belly turrets on the sides)\n\r" , ch);
        return;
      }
      else
      {
          do_designship ( ch , "" );
          return;
      }
    }
    
    if ( !str_cmp( arg1, "custom" ) )
       smodel = CUSTOM_SHIP;
    else
    {
        int tm;
        
        smodel = -1;
        
        for ( tm = 0 ; tm < CUSTOM_SHIP ; tm++ )
           if ( !str_cmp(arg0, model[tm].name ) )
              smodel = tm; 
    
        if ( smodel < 0 )
        {
          do_designship ( ch , "" );
          return;
        }    
    }
    
    hull = atoi( arg2 );
    energy = atoi( arg3 );
    shield = atoi( arg4 );
    speed = atoi( arg5 );
    manuever = atoi( arg6 );
    lasers = atoi( arg7 );
    missiles = atoi( arg8 );
    chaff = atoi( arg9 );
    
    if ( hull < 50 || hull > model[smodel].hull )
    {
   	ch_printf( ch, "&R%s hull values must be between 50 and %d.\n\r" ,
   	   model[smodel].name , model[smodel].hull );
   	return;
    }
    if ( energy < 500 || energy > model[smodel].energy )
    {
   	ch_printf( ch, "&R%s energy values must be between 500 and %d.\n\r" ,
   	   model[smodel].name , model[smodel].energy );
   	return;
    }
    if ( shield < 0 || shield > model[smodel].shield )
    {
   	ch_printf( ch, "&R%s shield values must be between 0 and %d.\n\r" ,
   	   model[smodel].name , model[smodel].shield );
   	return;
    }
    if ( speed < 10 || speed > model[smodel].realspeed )
    {
   	ch_printf( ch, "&R%s speed values must be between 10 and %d.\n\r" ,
   	   model[smodel].name , model[smodel].realspeed );
   	return;
    }
    if ( manuever < 10 || manuever > model[smodel].manuever )
    {
   	ch_printf( ch, "&R%s manuever values must be between 10 and %d.\n\r" ,
   	   model[smodel].name , model[smodel].manuever );
   	return;
    }
    if ( lasers < 0 || lasers > model[smodel].lasers )
    {
   	ch_printf( ch, "&R%s lasers must be between 0 and %d.\n\r" ,
   	   model[smodel].name , model[smodel].lasers );
   	return;
    }
    if ( missiles < 0 || missiles > model[smodel].missiles )
    {
   	ch_printf( ch, "&R%s missiles must be between 0 and %d.\n\r" ,
   	   model[smodel].name , model[smodel].missiles );
   	return;
    }
    if ( chaff < 0 || chaff > 20 )
    {
   	ch_printf( ch, "&R%s chaff must be between 0 and 20.\n\r" ,
   	   model[smodel].name );
   	return;
    }

    price = get_design_value( hull, energy, shield, speed, manuever, lasers, missiles, chaff, smodel );

    if ( !str_cmp( arg1, "test" ) )
    {
   	  ch_printf( ch, "&YName:     &W%s\n\r" , argument );
   	  ch_printf( ch, "&YModel:    &W%s\n\r" , model[smodel].name );
   	  ch_printf( ch, "\n\r"  );
   	  ch_printf( ch, "&YHull:     &W%d\n\r" , hull );
   	  ch_printf( ch, "&YEnergy:   &W%d\n\r" , energy );
   	  ch_printf( ch, "&YShield:   &W%d\n\r" , shield );
   	  ch_printf( ch, "&YSpeed:    &W%d\n\r" , speed );
   	  ch_printf( ch, "&YManuever: &W%d\n\r" , manuever );
   	  ch_printf( ch, "&YLasers:   &W%d\n\r" , lasers );
   	  ch_printf( ch, "&YMissiles: &W%d\n\r" , missiles );
   	  ch_printf( ch, "&YChaff:    &W%d\n\r" , chaff );
   	  ch_printf( ch, "\n\r"  );
   	  ch_printf( ch, "&YThis ship would cost &W%d&Y credits.\n\r" ,price );
   	  return;        
    }
       
    if ( !str_cmp( arg1, "clanship" ) || !str_cmp( arg1, "personal" ) )
    {
       int shipreg=0;
       char filename[10];
       char shipname[MAX_STRING_LENGTH];
       CLAN_DATA  * clan;     
       int dIndex;
        
       if ( !str_cmp( arg1, "clanship" ) )
       {
          if ( !ch->pcdata || !ch->pcdata->clan )
          {
   	    ch_printf( ch, "&RYou don't belong to a clan....\n\r" );
   	    return;
          }
          
          clan = ch->pcdata->clan;
          
          if ( price > clan->funds )
          {
   	    ch_printf( ch, "&RThat ship would cost %d. Your organization doesn't have enough.\n\r" ,price );
   	    return;
          }
          
       }
       else
       {
         if ( price > ch->gold )
         {
   	    ch_printf( ch, "&RThat ship would cost %d. You don't have enough.\n\r" ,price );
   	    return;
         }
       }

       for ( ship = first_ship ; ship ; ship = ship->next )
         if ( shipreg < atoi( ship->filename ) )
             shipreg = atoi( ship->filename );

       shipreg++;
       sprintf( filename , "%d" , shipreg );
       sprintf( shipname , "%s %d" , argument , shipreg );
    
       CREATE( ship, SHIP_DATA, 1 );
       LINK( ship, first_ship, last_ship, next, prev );
    
       ship->filename = str_dup( filename ) ;
    
       ship->next_in_starsystem = NULL;
       ship->prev_in_starsystem =NULL;
       ship->next_in_room = NULL;
       ship->prev_in_room = NULL;
       ship->in_room = NULL;
       ship->starsystem = NULL;
       ship->first_hanger = NULL;
       ship->last_hanger = NULL;
       ship->first_turret = NULL;
       ship->last_turret = NULL;
       ship->name = STRALLOC( shipname );
       ship->home = STRALLOC("");
       for ( dIndex = 0 ; dIndex < MAX_SHIP_ROOMS ; dIndex++ );
      	  ship->description[dIndex] = NULL;
       if ( !str_cmp( arg1, "clanship" ) )
          ship->owner = STRALLOC( clan->name );
       else
          ship->owner = STRALLOC( ch->name );
       ship->pilot = STRALLOC("");
       ship->copilot = STRALLOC("");
       ship->dest = NULL;
       ship->type = PLAYER_SHIP;
       ship->class = SPACECRAFT;
       ship->model = smodel;
       ship->hyperspeed = model[smodel].hyperspeed;
       ship->realspeed = speed;
       ship->shipstate = SHIP_DOCKED;
       ship->laserstate = LASER_READY;
       ship->missilestate = MISSILE_READY;
       ship->missiles = missiles;
       ship->maxmissiles = missiles;
       ship->lasers = lasers;
       ship->tractorbeam = 0;
       ship->manuever = manuever;
       ship->hatchopen = FALSE;
       ship->autorecharge = FALSE;
       ship->autotrack = FALSE;
       ship->autospeed = FALSE;
       ship->maxenergy = energy;
       ship->energy = energy;
       ship->shield = 0;
       ship->maxshield = shield;
       ship->hull = hull;
       ship->maxhull = hull;
       ship->location = 0;
       ship->lastdoc = 0;
       ship->shipyard = 0;
       ship->collision = 0;
       ship->target = NULL;
       ship->currjump = NULL;
       ship->chaff = chaff;
       ship->maxchaff = chaff;
       ship->chaff_released = FALSE;
       ship->autopilot = FALSE;
       create_ship_rooms ( ship );

       if ( !str_cmp( arg1, "clanship" ) )
       {     			
           clan->funds -= price;
           ch_printf(ch, "&G%s pays %ld credits for the ship.\n\r", clan->name , price );   
       }
       else
       {     			
           ch->gold -= price;
           ch_printf(ch, "&GYou pay %ld credits to design the ship.\n\r" , price );   
       }
       
       act( AT_PLAIN, "$n walks over to a terminal and makes a credit transaction.",ch,
          NULL, argument , TO_ROOM );

       ship_to_room( ship, ch->in_room->vnum );
       ship->location = ch->in_room->vnum;
       ship->lastdoc = ch->in_room->vnum;
    
       save_ship( ship );
       write_ship_list( );

       learn_from_success( ch , gsn_shipdesign );
       learn_from_success( ch , gsn_shipdesign );
       learn_from_success( ch , gsn_shipdesign );
       learn_from_success( ch , gsn_shipdesign );
       learn_from_success( ch , gsn_shipdesign );
	
       return;
    }
    
    send_to_char( "Unfortunately this skill isn't finished yet....\n\r" , ch);

}

void post_ship_guard( ROOM_INDEX_DATA * pRoomIndex )
{
            MOB_INDEX_DATA * pMobIndex;
            CHAR_DATA * mob;
            char tmpbuf[MAX_STRING_LENGTH];
	    OBJ_DATA *blaster;
            OBJ_INDEX_DATA *pObjIndex;
                        	                
            if ( !(pMobIndex = get_mob_index(MOB_VNUM_SHIP_GUARD)) )
            {
        	   bug( "Reset_all: Missing default guard (%d)", MOB_VNUM_SHIP_GUARD );
        	   return;
      	    }
            
            mob = create_mobile( pMobIndex );
            char_to_room( mob, pRoomIndex );
            mob->top_level = 100;
            mob->hit = 250;
            mob->max_hit = 250;
            mob->armor = 0;
            mob->damroll = 0;
            mob->hitroll = 20;

            if ( ( pObjIndex = get_obj_index( OBJ_VNUM_BLASTER ) ) != NULL )
            {
                 blaster = create_object( pObjIndex, mob->top_level );
                 obj_to_char( blaster, mob );
                 equip_char( mob, blaster, WEAR_WIELD );                        
            } 
            do_setblaster( mob , "full" );

            if ( room_is_dark(pRoomIndex) )
                   SET_BIT(mob->affected_by, AFF_INFRARED);

            sprintf( tmpbuf , "A Security Guard stands alert and ready for trouble.\n\r" );
            STRFREE( mob->long_descr );
            mob->long_descr = STRALLOC( tmpbuf );
            mob->mob_clan  = NULL;

}
