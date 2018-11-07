#include <sys/stat.h>

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

VOTE_DATA * first_poll;
VOTE_DATA * last_poll;

/* local routines */
void	fread_poll	args( ( VOTE_DATA *poll, FILE *fp ) );
bool	load_poll_file	args( ( char *pollfile ) );
void	write_poll_list	args( ( void ) );


void write_poll_list( )
{
    VOTE_DATA *tpoll;
    FILE *fpout;
    char player[256];

    sprintf( player, "%s%s", VOTE_DIR, VOTE_LIST );
    fpout = fopen( player, "w" );
    if ( !fpout )
    {
	bug( "FATAL: cannot open vote.lst for writing!\n\r", 0 );
 	return;
    }	  
    for ( tpoll = first_poll; tpoll; tpoll = tpoll->next )
	fprintf( fpout, "%s\n", tpoll->player );
    fprintf( fpout, "$\n" );
    fclose( fpout );
}

/*
 * Save a poll's data to its data file
 */
void save_poll( VOTE_DATA *poll )
{
    FILE *fp;
    char player[256];
    char buf[MAX_STRING_LENGTH];

    if ( !poll )
    {
	bug( "save_poll: null poll pointer!", 0 );
	return;
    }
        
    if ( !poll->player || poll->player[0] == '\0' )
	return;
 
    sprintf( player, "%s%s", VOTE_DIR, poll->player );
    
    fclose( fpReserve );
    if ( ( fp = fopen( player, "w" ) ) == NULL )
    {
    	bug( "save_poll: fopen", 0 );
    	perror( player );
    }
    else
    {
	fprintf( fp, "#VOTE\n" );
	fprintf( fp, "Player       %s~\n",	poll->player		);
	fprintf( fp, "Type         %d\n",	poll->type		);
	fprintf( fp, "Infavour     %d\n",	poll->in_favour		);
	fprintf( fp, "Against      %d\n",	poll->against		);
	fprintf( fp, "Timeofvote   %s~\n",	poll->time_of_vote	);
	fprintf( fp, "Iplist       %s~\n",	poll->ip_list		);
	fprintf( fp, "Namelist     %s~\n",	poll->name_list		);
	fprintf( fp, "End\n\n"						);
	fprintf( fp, "#END\n"						);
    }
    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}

/*
 * Read in actual poll data.
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

void fread_poll( VOTE_DATA *poll, FILE *fp )
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
	    KEY( "Against",	poll->against,	fread_number( fp ) );
	    break;

	case 'E':
	    if ( !str_cmp( word, "End" ) )
	    {
		if (!poll->time_of_vote)
		  poll->time_of_vote	= STRALLOC( "" );
		if (!poll->ip_list)
		  poll->ip_list		= STRALLOC( "" );
		if (!poll->name_list)
		  poll->name_list 	= STRALLOC( "" );
		return;
	    }
	    break;
	    
	case 'I':
	    KEY( "Infavour",	poll->in_favour,	fread_number( fp ) );
	    KEY( "Iplist",	poll->ip_list,		fread_string( fp ) );
	    break;

	case 'N':
	    KEY( "Namelist",	poll->name_list,	fread_string( fp ) );
	    break;

	case 'P':
	    KEY( "Player",	poll->player,		fread_string( fp ) );
	    break;

	case 'T':
	    KEY( "Timeofvote",	poll->time_of_vote,	fread_string( fp ) );
	    KEY( "Type",	poll->type,		fread_number( fp ) );
	    break;

	}
	
	if ( !fMatch )
	{
	    sprintf( buf, "Fread_poll: no match: %s", word );
	    bug( buf, 0 );
	}
	
    }
}

/*
 * Load a poll file
 */

bool load_poll_file( char *pollfile )
{
    char player[256];
    VOTE_DATA *poll;
    FILE *fp;
    bool found;

    CREATE( poll, VOTE_DATA, 1 );
    
    found = FALSE;
    sprintf( player, "%s%s", VOTE_DIR, pollfile );

    if ( ( fp = fopen( player, "r" ) ) != NULL )
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
		bug( "Load_poll_file: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );
	    if ( !str_cmp( word, "VOTE"	) )
	    {
	    	fread_poll( poll, fp );
	    	break;
	    }
	    else
	    if ( !str_cmp( word, "END"	) )
	        break;
	    else
	    {
		char buf[MAX_STRING_LENGTH];

		sprintf( buf, "Load_poll_file: bad section: %s.", word );
		bug( buf, 0 );
		break;
	    }
	}
	fclose( fp );
    }

    if ( found )
    {
	LINK( poll, first_poll, last_poll, next, prev );
        return found;	
    }
    else
      DISPOSE( poll );

    return found;
}


/*
 * Load in all the poll files.
 */
void load_polls( )
{
    FILE *fpList;
    char *player;
    char polllist[256];
    char buf[MAX_STRING_LENGTH];
    
    first_poll	= NULL;
    last_poll	= NULL;

    log_string( "Loading polls..." );

    sprintf( polllist, "%s%s", VOTE_DIR, VOTE_LIST );
    fclose( fpReserve );
    if ( ( fpList = fopen( polllist, "r" ) ) == NULL )
    {
	perror( polllist );
	exit( 1 );
    }

    for ( ; ; )
    {
	player = feof( fpList ) ? "$" : fread_word( fpList );
	log_string( player );
	if ( player[0] == '$' )
	  break;

	if ( !load_poll_file( player ) )
	{
	  sprintf( buf, "Cannot load poll file: %s", player );
	  bug( buf, 0 );
	}
    }
    fclose( fpList );
    log_string(" Done polls\n\r" );
    fpReserve = fopen( NULL_FILE, "r" );
    
    return;
}

