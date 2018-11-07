#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "mud.h"

extern int	top_affect;
extern int	top_ed;
extern bool	fBootDb;

extern OBJ_INDEX_DATA *obj_index_hash[MAX_KEY_HASH];
extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
extern ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];

char *  const   weapon_table    [13] =
{
    "none",
    "w1",  "vibro-blade",  "lightsaber", "w4", "w5",
    "blaster",  "w7", "w8", "w9", "w10",
    "w11", "w12"
};

char *  const   ex_flags [] = 
{ 
"isdoor", "closed", "locked", "secret", "swim", "pickproof", "fly", "climb",
"dig", "r1", "nopassdoor", "hidden", "passage", "portal", "r2", "r3",
"can_climb", "can_enter", "can_leave", "auto", "r4", "searchable", 
"bashed", "bashproof", "nomob", "window", "can_look" };

char *	const	r_flags	[] =
{
"dark", "reserved", "nomob", "indoors", "can_land", "garage", "control",
"trade", "bank", "r9", "safe", "mail", "information", "r13",
"shipyard", "barracks", "r16", "no_edit", "r18", "restaurant",
"plr_home", "empty_home", "r22", "hotel", "nofloor", "pawn", "supply",
"bar", "employment", "spacecraft", "r30", "auction"
};

char *	const	o_flags	[] =
{
"glow", "hum", "dark", "hutt_size", "i4", "invis", "magic", "nodrop", "bless",
"i9", "i10", "i11", "noremove", "inventory",
"i14", "i15", "i16", "i17", "small_size", "large_size",
"donation", "clanobject", "i22", "i23", "i24", 
"hidden", "poisoned", "covering", "deathrot", "burried", "prototype", "human_size"
};

char *	const	mag_flags	[] =
{
"returning", "backstabber", "bane", "loyal", "haste", "drain", 
"lightning_blade" 
};

char *	const	w_flags	[] =
{
"take", "finger", "neck", "body", "head", "legs", "feet", "hands", "arms",
"shield", "about", "waist", "wrist", "wield", "hold", "_dual_", "ears", "eyes",
"_missile_", "r1","r2","r3","r4","r5","r6",
"r7","r8","r9","r10","r11","r12","r13"
};

char *	const	area_flags	[] =
{
"nopkill", "modified", "r2", "r3", "r4", "r5", "r6", "r7", "r8",
"r9", "r10", "r11", "r12", "r13", "r14", "r15", "r16", "r17",
"r18", "r19","r20","r21","r22","r23","r24",
"r25","r26","r27","r28","r29","r30","r31"
};

char *	const	o_types	[] =
{
"none", "light", "weapon", "armor", "furniture", 
"trash", "container", "paper", "drinkcon", "food", 
"money", "pen", "corpse", "corpse_pc", "fountain", 
"scraps", "lockpick", "ammo", "shovel", "lens", 
"crystal", "plastic", "battery", "toolkit", "metal", 
"oven", "mirror", "circuit", "superconductor", "comlink", 
"medpac", "fabric", "rare_metal", "magnet",  "thread", 
"device", "droid_corpse", "resource"
};

char *	const	a_types	[] =
{
"none", "strength", "dexterity", "intelligence", "wisdom", "constitution",
"sex", "null", "level", "age", "height", "weight", "force", "hit", "move",
"credits", "experience", "armor", "hitroll", "damroll", "save_poison", "save_rod",
"save_para", "save_breath", "save_spell", "charisma", "affected", "resistant",
"immune", "susceptible", "weaponspell", "luck", "backstab", "pick", "track",
"steal", "sneak", "hide", "palm", "detrap", "dodge", "peek", "scan", "gouge",
"search", "mount", "disarm", "kick", "parry", "bash", "stun", "punch", "climb",
"grip", "scribe", "brew", "wearspell", "removespell", "mentalstate", "emotion",
"stripsn", "remove", "dig", "full", "thirst", "drunk", "blood"
};

char *	const	a_flags [] =
{
"blind", "invisible", "detect_evil", "detect_invis", "detect_magic",
"detect_hidden", "weaken", "sanctuary", "faerie_fire", "infrared", "curse",
"_flaming", "poison", "protect", "paralysis", "sneak", "hide", "sleep",
"charm", "flying", "pass_door", "floating", "truesight", "detect_traps",
"scrying", "fireshield", "shockshield", "r1", "iceshield", "possess", 
"berserk", "aqua_breath" };

char *	const	act_flags [] =
{
"npc", "sentinel", "scavenger", "r3", "r4", "aggressive", "stayarea",
"wimpy", "pet", "train", "practice", "immortal", "deadly", "polyself",
"meta_aggr", "guardian", "running", "nowander", "mountable", "mounted", "citizen",
"secretive", "polymorphed", "mobinvis", "noassist", "nokill", "droid", "nocorpse", 
"r28", "r29", "prototype", "r31" };

char *	const	pc_flags [] =
{
"r1", "deadly", "unauthed", "norecall", "nointro", "gag", "retired", "guest",
"nosummon", "pageron", "notitled", "room", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r13",
"r14", "r15", "r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23", "r24",
"r25"
};

char *	const	plr_flags [] =
{
"npc", "boughtpet", "shovedrag", "autoexits", "autoloot", "autosac", "blank", 
"outcast", "brief", "combine", "prompt", "telnet_ga", "holylight", 
"wizinvis", "roomvnum","silence", "noemote", "attacker", "notell", "log", 
"deny", "freeze", "killer","pf_3", "litterbug", "ansi", "rip", "nice", 
"flee" ,"autocred", "automap", "afk"
};

char *	const	trap_flags [] =
{
"room", "obj", "enter", "leave", "open", "close", "get", "put", "pick",
"unlock", "north", "south", "east", "r1", "west", "up", "down", "examine",
"r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r13",
"r14", "r15" 
};

char *	const	wear_locs [] =
{
"light", "finger1", "finger2", "neck1", "neck2", "body", "head", "legs",
"feet", "hands", "arms", "shield", "about", "waist", "wrist1", "wrist2",
"wield", "hold", "dual_wield", "ears", "eyes", "missile_wield"
};

char *	const	ris_flags [] =
{
"fire", "cold", "electricity", "energy", "blunt", "pierce", "slash", "acid",
"poison", "drain", "sleep", "charm", "hold", "nonmagic", "plus1", "plus2",
"plus3", "plus4", "plus5", "plus6", "magic", "paralysis", "r1", "r2", "r3",
"r4", "r5", "r6", "r7", "r8", "r9", "r10"
};

char *	const	trig_flags [] =
{
"up", "unlock", "lock", "d_north", "d_south", "d_east", "d_west", "d_up",
"d_down", "door", "container", "open", "close", "passage", "oload", "mload",
"teleport", "teleportall", "teleportplus", "death", "cast", "fakeblade",
"rand4", "rand6", "trapdoor", "anotherroom", "usedial", "absolutevnum",
"showroomdesc", "autoreturn", "r2", "r3"
};

char *	const	part_flags [] =
{
"head", "arms", "legs", "heart", "brains", "guts", "hands", "feet", "fingers",
"ear", "eye", "long_tongue", "eyestalks", "tentacles", "fins", "wings",
"tail", "scales", "claws", "fangs", "horns", "tusks", "tailattack",
"sharpscales", "beak", "haunches", "hooves", "paws", "forelegs", "feathers",
"r1", "r2"
};

char *	const	attack_flags [] =
{
"bite", "claws", "tail", "sting", "punch", "kick", 
"r6", "r7", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "r16", "r17", 
"r18", "r19", "r20", "r21", "r22", "r23", "r24", "r25", "r26", "r27", "r28", "r29",
"r30", "r31"
};

char *	const	defense_flags [] =
{
"parry", "dodge", "r2", "r3", "r4" ,"r5",
"r6", "r7", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "r16", "r17", 
"r18", "r19", "r20", "r21", "r22", "r23", "r24", "r25", "r26", "r27", "r28", "r29",
"r30", "r31"
};

/*
 * Note: I put them all in one big set of flags since almost all of these
 * can be shared between mobs, objs and rooms for the exception of
 * bribe and hitprcnt, which will probably only be used on mobs.
 * ie: drop -- for an object, it would be triggered when that object is
 * dropped; -- for a room, it would be triggered when anything is dropped
 *          -- for a mob, it would be triggered when anything is dropped
 *
 * Something to consider: some of these triggers can be grouped together,
 * and differentiated by different arguments... for example:
 *  hour and time, rand and randiw, speech and speechiw
 * 
 */
char *	const	mprog_flags [] =
{
"act", "speech", "rand", "fight", "death", "hitprcnt", "entry", "greet",
"allgreet", "give", "bribe", "hour", "time", "wear", "remove", "sac",
"look", "exa", "zap", "get", "drop", "damage", "repair", "randiw",
"speechiw", "pull", "push", "sleep", "rest", "leave", "script", "use"
};


char *flag_string( int bitvector, char * const flagarray[] )
{
    static char buf[MAX_STRING_LENGTH];
    int x;

    buf[0] = '\0';
    for ( x = 0; x < 32 ; x++ )
      if ( IS_SET( bitvector, 1 << x ) )
      {
	strcat( buf, flagarray[x] );
	strcat( buf, " " );
      }
    if ( (x=strlen( buf )) > 0 )
      buf[--x] = '\0';
    
    return buf;
}


bool can_rmodify( CHAR_DATA *ch, ROOM_INDEX_DATA *room )
{
	if ( IS_NPC( ch ) )
	  return FALSE;
	if ( IS_IMMORTAL( ch ) )
	  return TRUE;

	return FALSE;
}

bool can_omodify( CHAR_DATA *ch, OBJ_DATA *obj )
{
	if ( IS_NPC( ch ) )
	  return FALSE;
	if ( IS_IMMORTAL( ch )  )
	  return TRUE;
	return FALSE;
}

bool can_oedit( CHAR_DATA *ch, OBJ_INDEX_DATA *obj )
{
	if ( IS_NPC( ch ) )
	  return FALSE;
	if ( IS_IMMORTAL( ch ) )
	  return TRUE;
	return FALSE;
}


bool can_mmodify( CHAR_DATA *ch, CHAR_DATA *mob )
{
	if ( mob == ch )
	  return TRUE;

	if ( !IS_NPC( mob ) )
	{
	   if ( IS_IMMORTAL( ch ) && get_trust(ch) > 
		get_trust( mob ) )
	     return TRUE;
	   else
	     send_to_char( "You can't do that.\n\r", ch );
	     return FALSE;
	}

	if ( IS_NPC( ch ) )
	  return FALSE;
	if ( IS_IMMORTAL( ch ) )
	  return TRUE;
	return FALSE;
}

bool can_medit( CHAR_DATA *ch, MOB_INDEX_DATA *mob )
{
	if ( IS_NPC( ch ) )
	  return FALSE;
	if ( IS_IMMORTAL( ch ) )
	  return TRUE;
	return FALSE;
}

int get_otype( char *type )
{
    int x;

    for ( x = 0; x < (sizeof(o_types) / sizeof(o_types[0]) ); x++ )
      if ( !str_cmp( type, o_types[x] ) )
        return x;
    return -1;
}

int get_aflag( char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
      if ( !str_cmp( flag, a_flags[x] ) )
        return x;
    return -1;
}

int get_trapflag( char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
      if ( !str_cmp( flag, trap_flags[x] ) )
        return x;
    return -1;
}

int get_atype( char *type )
{
    int x;

    for ( x = 0; x < MAX_APPLY_TYPE; x++ )
      if ( !str_cmp( type, a_types[x] ) )
        return x;
    return -1;
}

int get_wearloc( char *type )
{
    int x;
    
    for ( x = 0; x < MAX_WEAR; x++ )
      if ( !str_cmp( type, wear_locs[x] ) )
        return x;
    return -1;
}

int get_exflag( char *flag )
{
    int x;
    
    for ( x = 0; x <= MAX_EXFLAG; x++ )
      if ( !str_cmp( flag, ex_flags[x] ) )
        return x;
    return -1;
}

int get_rflag( char *flag )
{
    int x;
    
    for ( x = 0; x < 32; x++ )
      if ( !str_cmp( flag, r_flags[x] ) )
        return x;
    return -1;
}

int get_mpflag( char *flag )
{
    int x;
    
    for ( x = 0; x < 32; x++ )
      if ( !str_cmp( flag, mprog_flags[x] ) )
        return x;
    return -1;
}

int get_oflag( char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
      if ( !str_cmp( flag, o_flags[x] ) )
        return x;
    return -1;
}

int get_areaflag( char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
      if ( !str_cmp( flag, area_flags[x] ) )
        return x;
    return -1;
}

int get_wflag( char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
      if ( !str_cmp( flag, w_flags[x] ) )
        return x;
    return -1;
}

int get_actflag( char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
      if ( !str_cmp( flag, act_flags[x] ) )
        return x;
    return -1;
}

int get_pcflag( char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
      if ( !str_cmp( flag, pc_flags[x] ) )
        return x;
    return -1;
}
int get_plrflag( char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
      if ( !str_cmp( flag, plr_flags[x] ) )
        return x;
    return -1;
}

int get_risflag( char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
      if ( !str_cmp( flag, ris_flags[x] ) )
        return x;
    return -1;
}

int get_trigflag( char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
      if ( !str_cmp( flag, trig_flags[x] ) )
        return x;
    return -1;
}

int get_partflag( char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
      if ( !str_cmp( flag, part_flags[x] ) )
        return x;
    return -1;
}

int get_attackflag( char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
      if ( !str_cmp( flag, attack_flags[x] ) )
        return x;
    return -1;
}

int get_defenseflag( char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
      if ( !str_cmp( flag, defense_flags[x] ) )
        return x;
    return -1;
}

/*
 * Remove carriage returns from a line
 */
char *strip_cr( char *str )
{
    static char newstr[MAX_STRING_LENGTH];
    int i, j;

    for ( i=j=0; str[i] != '\0'; i++ )
	if ( str[i] != '\r' )
	{
	  newstr[j++] = str[i];	
	}
    newstr[j] = '\0';
    return newstr;
}


/*
 * Removes the tildes from a line, except if it's the last character.
 */
void smush_tilde( char *str )
{
    int len;
    char last;
    char *strptr;
    
    strptr = str;
    
    len  = strlen( str );
    if ( len )
      last = strptr[len-1];
    else
      last = '\0';

    for ( ; *str != '\0'; str++ )
    {
	if ( *str == '~' )
	    *str = '-';
    }
    if ( len )
      strptr[len-1] = last;

    return;
}


void start_editing( CHAR_DATA *ch, char *data )
{
	EDITOR_DATA *edit;
	sh_int lines, size, lpos;
	char c;
	
	if ( !ch->desc )
	{
	   bug( "Fatal: start_editing: no desc", 0 );
	   return;
	}
	if ( ch->substate == SUB_RESTRICTED )
	   bug( "NOT GOOD: start_editing: ch->substate == SUB_RESTRICTED", 0 );

	set_char_color( AT_GREEN, ch );
	send_to_char( "Begin entering your text (/? =help /s =save /c =clear /l =list /f =format)\n\r", ch );
	send_to_char( "--------------------------------------------------------------------------\n\r> ", ch );
	if ( ch->editor )
	  stop_editing( ch );
	
	CREATE( edit, EDITOR_DATA, 1 );
	edit->numlines = 0;
	edit->on_line  = 0;
	edit->size     = 0;
	size = 0;  lpos = 0;  lines = 0;
	if ( !data )
	    bug("editor: data is NULL!\n\r",0);
	else
	for ( ;; )
	{  
	   c = data[size++];
	   if ( c == '\0' )
	   {
		edit->line[lines][lpos] = '\0';
		break;
	   }
	   else
	   if ( c == '\r' );
	   else
	   if ( c == '\n' || lpos > 78)
	   {
		edit->line[lines][lpos] = '\0';
		lines++;
		lpos = 0;
	   }
	   else
	     edit->line[lines][lpos++] = c;
	   if ( lines >= 49 || size > 4096 )
	   {
		edit->line[lines][lpos] = '\0';
		break;
	   }	   
	}
	edit->numlines = lines;
	edit->size = size;
	edit->on_line = lines;
	ch->editor = edit;
	ch->desc->connected = CON_EDITING;
}

char *copy_buffer( CHAR_DATA *ch )
{
   char buf[MAX_STRING_LENGTH];
   char tmp[100];
   sh_int x, len;

   if ( !ch )
   {
	bug( "copy_buffer: null ch", 0 );
	return STRALLOC( "" );
   } 

   if ( !ch->editor )
   {
	bug( "copy_buffer: null editor", 0 );
	return STRALLOC( "" );
   }

   buf[0] = '\0';
   for ( x = 0; x < ch->editor->numlines; x++ )
   {
      strcpy( tmp, ch->editor->line[x] );
      smush_tilde( tmp );
      len = strlen(tmp);
      if ( tmp && tmp[len-1] == '~' )
        tmp[len-1] = '\0';
      else
        strcat( tmp, "\n\r" );
      strcat( buf, tmp );
   }
   return STRALLOC( buf );
}

void stop_editing( CHAR_DATA *ch )
{
    set_char_color( AT_PLAIN, ch );
    DISPOSE( ch->editor );
    ch->editor = NULL;
    send_to_char( "Done.\n\r", ch );
    ch->dest_buf  = NULL;
    ch->spare_ptr = NULL;
    ch->substate  = SUB_NONE;
    if ( !ch->desc )
    {
	bug( "Fatal: stop_editing: no desc", 0 );
	return;
    }
    ch->desc->connected = CON_PLAYING;
}

void do_goto( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    CHAR_DATA *fch;
    CHAR_DATA *fch_next;
    ROOM_INDEX_DATA *in_room;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Goto where?\n\r", ch );
	return;
    }

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {

	send_to_char( "You cannot find that...\n\r", ch );
	return;
    
    }
    
    in_room = ch->in_room;
    if ( ch->fighting )
	stop_fighting( ch, TRUE );

    if ( !IS_SET(ch->act, PLR_WIZINVIS) )
         if (ch->pcdata && ch->pcdata->bamfout[0] != '\0')
               act( AT_IMMORT, "$T", ch, NULL, ch->pcdata->bamfout ,  TO_ROOM );
          else  
               act( AT_IMMORT, "$n $T", ch, NULL, "leaves in a swirl of the force.",  TO_ROOM );
                                  
                                  
    ch->regoto = ch->in_room->vnum;
    char_from_room( ch );
    if ( ch->mount )
    {
	char_from_room( ch->mount );
	char_to_room( ch->mount, location );
    }
    char_to_room( ch, location );

   if ( !IS_SET(ch->act, PLR_WIZINVIS) )
         if (ch->pcdata && ch->pcdata->bamfin[0] != '\0')
             act( AT_IMMORT, "$T", ch, NULL, ch->pcdata->bamfin ,  TO_ROOM );
         else  
             act( AT_IMMORT, "$n $T", ch, NULL, "enters in a swirl of the Force.",  TO_ROOM );
                               
                               
    do_look( ch, "auto" );

    if ( ch->in_room == in_room )
      return;
    for ( fch = in_room->first_person; fch; fch = fch_next )
    {
	fch_next = fch->next_in_room;
	if ( fch->master == ch && IS_IMMORTAL(fch) )
	{
	    act( AT_ACTION, "You follow $N.", fch, NULL, ch, TO_CHAR );
	    do_goto( fch, argument );
	}
    }
    return;
}

void do_mset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    char buf  [MAX_STRING_LENGTH];
    char outbuf[MAX_STRING_LENGTH];
    int  num,size,plus;
    char char1,char2;
    CHAR_DATA *victim;
    int value;
    int minattr, maxattr;
    bool lockvictim;
    char *origarg = argument;

    if ( IS_NPC( ch ) )
    {
	send_to_char( "Mob's can't mset\n\r", ch );
	return;    
    }

    if ( !ch->desc )
    {
	send_to_char( "You have no descriptor\n\r", ch );
	return;
    }

    switch( ch->substate )
    {
	default:
	  break;
	case SUB_MOB_DESC:
	  if ( !ch->dest_buf )
	  {
		send_to_char( "Fatal error: report to Thoric.\n\r", ch );
		bug( "do_mset: sub_mob_desc: NULL ch->dest_buf", 0 );
		ch->substate = SUB_NONE;
		return;
	  }
	  victim = ch->dest_buf;
	  if ( char_died(victim) )
	  {
		send_to_char( "Your victim died!\n\r", ch );
		stop_editing( ch );
		return;
	  }
	  STRFREE( victim->description );
	  victim->description = copy_buffer( ch );
	  if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
	  {
		STRFREE( victim->pIndexData->description );
		victim->pIndexData->description = QUICKLINK( victim->description );
	  }
	  stop_editing( ch );
	  ch->substate = ch->tempnum;
	  return;
    }

    victim = NULL;
    lockvictim = FALSE;
    smash_tilde( argument );

    if ( ch->substate == SUB_REPEATCMD )
    {
	victim = ch->dest_buf;
	if ( char_died(victim) )
	{
	    send_to_char( "Your victim died!\n\r", ch );
	    victim = NULL;
	    argument = "done";
	}
	if ( argument[0] == '\0' || !str_cmp( argument, " " )
	||   !str_cmp( argument, "stat" ) )
	{
	    if ( victim )
		do_mstat( ch, victim->name );
	    else
	        send_to_char( "No victim selected.  Type '?' for help.\n\r", ch );
	    return;
	}
	if ( !str_cmp( argument, "done" ) || !str_cmp( argument, "off" ) )
	{
	    send_to_char( "Mset mode off.\n\r", ch );
	    ch->substate = SUB_NONE;
	    ch->dest_buf = NULL;
	    if ( ch->pcdata && ch->pcdata->subprompt )
		STRFREE( ch->pcdata->subprompt );
	    return;
	}
    }
    if ( victim )
    {
	lockvictim = TRUE;
	strcpy( arg1, victim->name );
	argument = one_argument( argument, arg2 );
	strcpy( arg3, argument );
    }
    else
    {
	lockvictim = FALSE;
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	strcpy( arg3, argument );
    }
/*
    if ( !str_cmp( arg1, "on" ) )
    {
	send_to_char( "Syntax: mset <victim|vnum> on.\n\r", ch );
	return;
    }
*/
    if ( arg1[0] == '\0' || (arg2[0] == '\0' && ch->substate != SUB_REPEATCMD)
    ||   !str_cmp( arg1, "?" ) )
    {
	if ( ch->substate == SUB_REPEATCMD )
	{
	    if ( victim )
		send_to_char( "Syntax: <field>  <value>\n\r",		ch );
	    else
		send_to_char( "Syntax: <victim> <field>  <value>\n\r",	ch );
	}
	else
	    send_to_char( "Syntax: mset <victim> <field>  <value>\n\r",	ch );
	send_to_char( "\n\r",						ch );
	send_to_char( "Field being one of:\n\r",			ch );
	send_to_char( "  str int wis dex con cha lck frc sex\n\r",	ch );
	send_to_char( "  credits hp force move align\n\r",ch );
	send_to_char( "  hitroll damroll armor affected level\n\r",	ch );
	send_to_char( "  thirst drunk full blood flags\n\r",		ch );
	send_to_char( "  pos defpos part (see BODYPARTS)\n\r",		ch );
	send_to_char( "  sav1 sav2 sav4 sav4 sav5 (see SAVINGTHROWS)\n\r", ch );
	send_to_char( "  resistant immune susceptible (see RIS)\n\r",	ch );
	send_to_char( "  attack defense numattacks\n\r",		ch );
	send_to_char( "  name short long description title spec spec2\n\r", ch );
	send_to_char( "  clan vip wanted\n\r",                  ch );
	send_to_char( "\n\r",						ch );
	send_to_char( "For editing index/prototype mobiles:\n\r",	ch );
	send_to_char( "  hitnumdie hitsizedie hitplus (hit points)\n\r",ch );
	send_to_char( "  damnumdie damsizedie damplus (damage roll)\n\r",ch );
	send_to_char( "To toggle area flag: aloaded\n\r",ch);
	return;
    }

    if ( !victim )
    {
	if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
	{
	    send_to_char( "No one like that in all the realms.\n\r", ch );
	    return;
	}
    }

    if ( get_trust( ch ) < get_trust( victim ) && !IS_NPC( victim ) )
    {
	send_to_char( "You can't do that!\n\r", ch );
	ch->dest_buf = NULL;
	return;
    }
    if ( lockvictim )
      ch->dest_buf = victim;

    if ( IS_NPC(victim) )
    {
	minattr = 1;
	maxattr = 25;
    }
    else
    {
	minattr = 3;
	maxattr = 18;
    }

/*
    if ( !str_cmp( arg2, "on" ) )
    {
	CHECK_SUBRESTRICTED( ch );
	ch_printf( ch, "Mset mode on. (Editing %s).\n\r",
		victim->name );
	ch->substate = SUB_REPEATCMD;
	ch->dest_buf = victim;
	if ( ch->pcdata )
	{
	   if ( ch->pcdata->subprompt )
		STRFREE( ch->pcdata->subprompt );
	   if ( IS_NPC(victim) )
		sprintf( buf, "<&CMset &W#%ld&w> %%i", victim->pIndexData->vnum );
	   else
		sprintf( buf, "<&CMset &W%s&w> %%i", victim->name );
	   ch->pcdata->subprompt = STRALLOC( buf );
	}
	return;
    }
 */
 
    value = is_number( arg3 ) ? atoi( arg3 ) : -1;

    if ( atoi(arg3) < -1 && value == -1 )
      value = atoi(arg3);

    if ( !str_cmp( arg2, "str" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	  return;
	if ( value < minattr || value > maxattr )
	{
	    ch_printf( ch, "Strength range is %d to %d.\n\r", minattr, maxattr );
	    return;
	}
	victim->perm_str = value;
	if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
	  victim->pIndexData->perm_str = value;
	return;
    }

    if ( !str_cmp( arg2, "int" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	  return;
	if ( value < minattr || value > maxattr )
	{
	    ch_printf( ch, "Intelligence range is %d to %d.\n\r", minattr, maxattr );
	    return;
	}
	victim->perm_int = value;
	if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
	  victim->pIndexData->perm_int = value;
	return;
    }

    if ( !str_cmp( arg2, "wis" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	  return;
	if ( value < minattr || value > maxattr )
	{
	    ch_printf( ch, "Wisdom range is %d to %d.\n\r", minattr, maxattr );
	    return;
	}
	victim->perm_wis = value;
	if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
	  victim->pIndexData->perm_wis = value;
	return;
    }

    if ( !str_cmp( arg2, "dex" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	  return;
	if ( value < minattr || value > maxattr )
	{
	    ch_printf( ch, "Dexterity range is %d to %d.\n\r", minattr, maxattr );
	    return;
	}
	victim->perm_dex = value;
	if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
	  victim->pIndexData->perm_dex = value;
	return;
    }

    if ( !str_cmp( arg2, "con" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	  return;
	if ( value < minattr || value > maxattr )
	{
	    ch_printf( ch, "Constitution range is %d to %d.\n\r", minattr, maxattr );
	    return;
	}
	victim->perm_con = value;
	if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
	  victim->pIndexData->perm_con = value;
	return;
    }

    if ( !str_cmp( arg2, "cha" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	  return;
	if ( value < minattr || value > maxattr )
	{
	    ch_printf( ch, "Charisma range is %d to %d.\n\r", minattr, maxattr );
	    return;
	}
	victim->perm_cha = value;
	if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
	  victim->pIndexData->perm_cha = value;
	return;
    }

    if ( !str_cmp( arg2, "lck" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	  return;
	if ( value < minattr || value > maxattr )
	{
	    ch_printf( ch, "Luck range is %d to %d.\n\r", minattr, maxattr );
	    return;
	}
	victim->perm_lck = value;
	if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
	  victim->pIndexData->perm_lck = value;
	return;
    }
    
    if ( !str_cmp( arg2, "frc" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	  return;
	if ( value < 0 || value > 20 )
	{
	    ch_printf( ch, "Frc range is %d to %d.\n\r", minattr, maxattr );
	    return;
	}
	victim->perm_frc = value;
	if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
	  victim->pIndexData->perm_frc = value;
	return;
    }

    if ( !str_cmp( arg2, "sav1" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	  return;
	if ( value < -30 || value > 30 )
	{
	    send_to_char( "Saving throw range vs poison is -30 to 30.\n\r", ch );
	    return;
	}
	victim->saving_poison_death = value;
	if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
	  victim->pIndexData->saving_poison_death = value;
	return;
    }

    if ( !str_cmp( arg2, "sav2" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	  return;
	if ( value < -30 || value > 30 )
	{
	    send_to_char( "Saving throw range vs wands is -30 to 30.\n\r", ch );
	    return;
	}
	victim->saving_wand = value;
	if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
	  victim->pIndexData->saving_wand = value;
	return;
    }

    if ( !str_cmp( arg2, "sav3" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	  return;
	if ( value < -30 || value > 30 )
	{
	    send_to_char( "Saving throw range vs para is -30 to 30.\n\r", ch );
	    return;
	}
	victim->saving_para_petri = value;
	if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
	  victim->pIndexData->saving_para_petri = value;
	return;
    }

    if ( !str_cmp( arg2, "sav4" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	  return;
	if ( value < -30 || value > 30 )
	{
	    send_to_char( "Saving throw range vs bad breath is -30 to 30.\n\r", ch );
	    return;
	}
	victim->saving_breath = value;
	if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
	  victim->pIndexData->saving_breath = value;
	return;
    }

    if ( !str_cmp( arg2, "sav5" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	  return;
	if ( value < -30 || value > 30 )
	{
	    send_to_char( "Saving throw range vs force powers is -30 to 30.\n\r", ch );
	    return;
	}
	victim->saving_spell_staff = value;
	if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
	  victim->pIndexData->saving_spell_staff = value;
	return;
    }

    if ( !str_cmp( arg2, "sex" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	  return;
	if ( value < 0 || value > 2 )
	{
	    send_to_char( "Sex range is 0 to 2.\n\r", ch );
	    return;
	}
	victim->sex = value;
	if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
	  victim->pIndexData->sex = value;
	return;
    }

    if ( !str_cmp( arg2, "armor" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	  return;
	if ( value < -300 || value > 300 )
	{
	    send_to_char( "AC range is -300 to 300.\n\r", ch );
	    return;
	}
	victim->armor = value;
	if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
	  victim->pIndexData->ac = value;
	return;
    }

    if ( !str_cmp( arg2, "numattacks" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	  return;
	if ( !IS_NPC(victim) )
	{
	    send_to_char( "Not on PC's.\n\r", ch );
	    return;
	}

	if ( value < 0 || value > 20 )
	{
	    send_to_char( "Attacks range is 0 to 20.\n\r", ch );
	    return;
	}
	victim->numattacks = value;
	if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
	  victim->pIndexData->numattacks = value;
	return;
    }

    if ( !str_cmp( arg2, "credits" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	  return;
	victim->gold = value;
	if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
	  victim->pIndexData->gold = value;
	return;
    }

    if ( !str_cmp( arg2, "hitroll" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	  return;
	victim->hitroll = URANGE(0, value, 85);
	if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
	  victim->pIndexData->hitroll = victim->hitroll;
	return;
    }

    if ( !str_cmp( arg2, "damroll" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	  return;
	victim->damroll = URANGE(0, value, 65);
	if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
	  victim->pIndexData->damroll = victim->damroll;
	return;
    }

    if ( !str_cmp( arg2, "hp" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	  return;
	if ( value < 1 || value > 32700 )
	{
	    send_to_char( "Hp range is 1 to 32,700 hit points.\n\r", ch );
	    return;
	}
	victim->max_hit = value;
	return;
    }

    if ( !str_cmp( arg2, "force" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	  return;
	if ( value < 0 || value > 30000 )
	{
	    send_to_char( "Force range is 0 to 30,000 force points.\n\r", ch );
	    return;
	}
	victim->max_mana = value;
	return;
    }

    if ( !str_cmp( arg2, "move" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	  return;
	if ( value < 0 || value > 30000 )
	{
	    send_to_char( "Move range is 0 to 30,000 move points.\n\r", ch );
	    return;
	}
	victim->max_move = value;
	return;
    }

    if ( !str_cmp( arg2, "align" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	  return;
	if ( value < -1000 || value > 1000 )
	{
	    send_to_char( "Alignment range is -1000 to 1000.\n\r", ch );
	    return;
	}
	victim->alignment = value;
	if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
	  victim->pIndexData->alignment = value;
	return;
    }

    if ( !str_cmp( arg2, "password" ) )
    {
      char *pwdnew;
      char *p;

      if ( IS_NPC( victim ) )
      {
	send_to_char( "Mobs don't have passwords.\n\r", ch );
	return;
      }

      if ( strlen(arg3) < 5 )
      {
	send_to_char(
	    "New password must be at least five characters long.\n\r", ch );
	return;
      }

      /*
       * No tilde allowed because of player file format.
       */
      pwdnew = crypt( arg3, ch->name );
      for ( p = pwdnew; *p != '\0'; p++ )
      {
	if ( *p == '~' )
	{
	    send_to_char(
		"New password not acceptable, try again.\n\r", ch );
	    return;
	}
      }

      DISPOSE( victim->pcdata->pwd );
      victim->pcdata->pwd = str_dup( pwdnew );
      if ( IS_SET(sysdata.save_flags, SV_PASSCHG) )
 	save_char_obj( victim );
      send_to_char( "Ok.\n\r", ch );
      ch_printf( victim, "Your password has been changed by %s.\n\r", ch->name );
      return;
    }

    if ( !str_cmp( arg2, "quest" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < 0 || value > 500 )
	{
	    send_to_char( "The current quest range is 0 to 500.\n\r", ch );
	    return;
	}

	victim->pcdata->quest_number = value;
	return;
    }

    if ( !str_cmp( arg2, "qpa" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	victim->pcdata->quest_accum = value;
	return;
    }

    if ( !str_cmp( arg2, "qp" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < 0 || value > 5000 )
	{
	    send_to_char( "The current quest point range is 0 to 5000.\n\r", ch );
	    return;
	}

	victim->pcdata->quest_curr = value;
	return;
    }

    if ( !str_cmp( arg2, "mentalstate" ) )
    {
	if ( value < -100 || value > 100 )
	{
	    send_to_char( "Value must be in range -100 to +100.\n\r", ch );
	    return;
	}
	victim->mental_state = value;
	return;
    }

    if ( !str_cmp( arg2, "emotion" ) )
    {
	if ( value < -100 || value > 100 )
	{
	    send_to_char( "Value must be in range -100 to +100.\n\r", ch );
	    return;
	}
	victim->emotional_state = value;
	return;
    }

    if ( !str_cmp( arg2, "thirst" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < 0 || value > 100 )
	{
	    send_to_char( "Thirst range is 0 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_THIRST] = value;
	return;
    }

    if ( !str_cmp( arg2, "drunk" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < 0 || value > 100 )
	{
	    send_to_char( "Drunk range is 0 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_DRUNK] = value;
	return;
    }

    if ( !str_cmp( arg2, "full" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < 0 || value > 100 )
	{
	    send_to_char( "Full range is 0 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_FULL] = value;
	return;
    }

    if ( !str_cmp( arg2, "name" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	  return;

	STRFREE( victim->name );
	victim->name = STRALLOC( arg3 );
	if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
	{
	   STRFREE( victim->pIndexData->player_name );
	   victim->pIndexData->player_name = QUICKLINK( victim->name );
	}
	return;
    }

    if ( !str_cmp( arg2, "minsnoop" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}
	if ( victim->pcdata )
	{
	    victim->pcdata->min_snoop = value;
	    return;
	}
    }

    if ( !str_cmp( arg2, "clan" ) )
    {
	CLAN_DATA *clan;

	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( !arg3 || arg3[0] == '\0' )
	{
	    STRFREE( victim->pcdata->clan_name );
	    victim->pcdata->clan_name	= STRALLOC( "" );
	    victim->pcdata->clan	= NULL;
	    send_to_char( "Removed from clan.\n\rPlease make sure you adjust that clan's members accordingly.\n\rAlso be sure to remove any bestowments they have been given.\n\r", ch );
	    return;
	}
	clan = get_clan( arg3 );
	if ( !clan )
	{
	   send_to_char( "No such clan.\n\r", ch );
	   return;
	}
	STRFREE( victim->pcdata->clan_name );
	victim->pcdata->clan_name = QUICKLINK( clan->name );
	victim->pcdata->clan = clan;
	send_to_char( "Done.\n\rPlease make sure you adjust that clan's members accordingly.\n\r", ch );
	return;
    }

    
    if ( !str_cmp( arg2, "short" ) )
    {
	STRFREE( victim->short_descr );
	victim->short_descr = STRALLOC( arg3 );
	if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
	{
	   STRFREE( victim->pIndexData->short_descr );
	   victim->pIndexData->short_descr = QUICKLINK( victim->short_descr );
	}
	return;
    }

    if ( !str_cmp( arg2, "long" ) )
    {
	STRFREE( victim->long_descr );
	strcpy( buf, arg3 );
	strcat( buf, "\n\r" );
	victim->long_descr = STRALLOC( buf );
	if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
	{
	   STRFREE( victim->pIndexData->long_descr );
	   victim->pIndexData->long_descr = QUICKLINK( victim->long_descr );
	}
	return;
    }

    if ( !str_cmp( arg2, "description" ) )
    {
	if ( arg3[0] )
	{
	   STRFREE( victim->description );
	   victim->description = STRALLOC( arg3 );
	   if ( IS_NPC( victim ) && IS_SET( victim->act, ACT_PROTOTYPE ) )
	   {
	      STRFREE(victim->pIndexData->description );
	      victim->pIndexData->description = QUICKLINK( victim->description );
	   }
	   return;
	}
	CHECK_SUBRESTRICTED( ch );
	if ( ch->substate == SUB_REPEATCMD )
	  ch->tempnum = SUB_REPEATCMD;
	else
	  ch->tempnum = SUB_NONE;
	ch->substate = SUB_MOB_DESC;
	ch->dest_buf = victim;
	start_editing( ch, victim->description );
	return;
    }

    if ( !str_cmp( arg2, "title" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	set_title( victim, arg3 );
	return;
    }

    if ( !str_cmp( arg2, "spec" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	  return;
	if ( !IS_NPC(victim) )
	{
	    send_to_char( "Not on PC's.\n\r", ch );
	    return;
	}

        if ( !str_cmp( arg3, "none" ) )
        {
          victim->spec_fun = NULL;
	  send_to_char( "Special function removed.\n\r", ch );
	  if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
	    victim->pIndexData->spec_fun = victim->spec_fun;
	  return;
        }

	if ( ( victim->spec_fun = spec_lookup( arg3 ) ) == 0 )
	{
	    send_to_char( "No such spec fun.\n\r", ch );
	    return;
	}
	if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
	  victim->pIndexData->spec_fun = victim->spec_fun;
	return;
    }

    if ( !str_cmp( arg2, "spec2" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	  return;
	if ( !IS_NPC(victim) )
	{
	    send_to_char( "Not on PC's.\n\r", ch );
	    return;
	}

        if ( !str_cmp( arg3, "none" ) )
        {
          victim->spec_2 = NULL;
	  send_to_char( "Special function removed.\n\r", ch );
	  if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
	    victim->pIndexData->spec_2 = victim->spec_2;
	  return;
        }

	if ( ( victim->spec_2 = spec_lookup( arg3 ) ) == 0 )
	{
	    send_to_char( "No such spec fun.\n\r", ch );
	    return;
	}
	if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
	  victim->pIndexData->spec_2 = victim->spec_2;
	return;
    }

    if ( !str_cmp( arg2, "flags" ) )
    {
        bool pcflag;

	if ( !can_mmodify( ch, victim ) )
	  return;
	if ( !argument || argument[0] == '\0' )
	{
	   send_to_char( "Usage: mset <victim> flags <flag> [flag]...\n\r", ch );
	   send_to_char( "sentinal, scavenger, aggressive, stayarea, wimpy, practice, immortal,\n\r", ch );
	   send_to_char( "deadly, mountable, guardian, nokill, scholar, noassist, droid, nocorpse,\n\r", ch );
	   return;
	}
	while ( argument[0] != '\0' )
	{
           pcflag = FALSE;
	   argument = one_argument( argument, arg3 );
	   value = IS_NPC( victim) ? get_actflag( arg3 ) : get_plrflag( arg3 );

	   if ( !IS_NPC( victim ) && ( value < 0 || value > 31 ) )
           {
             pcflag = TRUE;
	     value = get_pcflag( arg3 );
           }
	   if ( value < 0 || value > 31 )
	     ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
	   else
	   {
	     if ( IS_NPC(victim) && 1 << value == ACT_IS_NPC )
	       send_to_char( "If that could be changed, it would cause many problems.\n\r", ch );
	     else
	     if ( IS_NPC(victim) && 1 << value == ACT_POLYMORPHED )
	       send_to_char( "Changing that would be a _bad_ thing.\n\r", ch);
	     else
	     {
		if ( pcflag )
		  TOGGLE_BIT( victim->pcdata->flags, 1 << value );
		else
		{ 
		  TOGGLE_BIT( victim->act, 1 << value );
		  /* NPC check added by Gorog */
		  if ( IS_NPC(victim) && (1 << value == ACT_PROTOTYPE) )
		    victim->pIndexData->act = victim->act;
		}
	     }
	   }
	}
	if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
	  victim->pIndexData->act = victim->act; 
	return;
    }

    if ( !str_cmp( arg2, "affected" ) )
    {

	if ( !can_mmodify( ch, victim ) )
	  return;
	if ( !argument || argument[0] == '\0' )
	{
	   send_to_char( "Usage: mset <victim> affected <flag> [flag]...\n\r", ch);
	   return;
	}
	while ( argument[0] != '\0' )
	{
	   argument = one_argument( argument, arg3 );
	   value = get_aflag( arg3 );
	   if ( value < 0 || value > 31 )
	     ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
	   else
	     TOGGLE_BIT( victim->affected_by, 1 << value );
	}
	if ( IS_NPC( victim ) && IS_SET( victim->act, ACT_PROTOTYPE ) )
	  victim->pIndexData->affected_by = victim->affected_by; 
	return;
    }

    /*
     * save some more finger-leather for setting RIS stuff
     */
    if ( !str_cmp( arg2, "r" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	    return;

	sprintf(outbuf,"%s resistant %s",arg1, arg3);
        do_mset( ch, outbuf );
        return;
    }
    if ( !str_cmp( arg2, "i" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	    return;


	sprintf(outbuf,"%s immune %s",arg1, arg3);
        do_mset( ch, outbuf );
        return;
    }
    if ( !str_cmp( arg2, "s" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	    return;

	sprintf(outbuf,"%s susceptible %s",arg1, arg3);
        do_mset( ch, outbuf );
        return;
    }
    if ( !str_cmp( arg2, "ri" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	    return;

	sprintf(outbuf,"%s resistant %s",arg1, arg3);
        do_mset( ch, outbuf );
	sprintf(outbuf,"%s immune %s",arg1, arg3);
        do_mset( ch, outbuf );
        return;
    }

    if ( !str_cmp( arg2, "rs" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	    return;

	sprintf(outbuf,"%s resistant %s",arg1, arg3);
        do_mset( ch, outbuf );
	sprintf(outbuf,"%s susceptible %s",arg1, arg3);
        do_mset( ch, outbuf );
        return;
    }
    if ( !str_cmp( arg2, "is" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	    return;

	sprintf(outbuf,"%s immune %s",arg1, arg3);
        do_mset( ch, outbuf );
	sprintf(outbuf,"%s susceptible %s",arg1, arg3);
        do_mset( ch, outbuf );
        return;
    }
    if ( !str_cmp( arg2, "ris" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	    return;

	sprintf(outbuf,"%s resistant %s",arg1, arg3);
        do_mset( ch, outbuf );
	sprintf(outbuf,"%s immune %s",arg1, arg3);
        do_mset( ch, outbuf );
	sprintf(outbuf,"%s susceptible %s",arg1, arg3);
        do_mset( ch, outbuf );
        return;
    }

    if ( !str_cmp( arg2, "resistant" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	    return;
	if ( !argument || argument[0] == '\0' )
	{
	   send_to_char( "Usage: mset <victim> resistant <flag> [flag]...\n\r", ch );
	   return;
	}
	while ( argument[0] != '\0' )
	{
	   argument = one_argument( argument, arg3 );
	   value = get_risflag( arg3 );
	   if ( value < 0 || value > 31 )
		ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
	   else
		TOGGLE_BIT( victim->resistant, 1 << value );
	}
	if ( IS_NPC( victim ) && IS_SET( victim->act, ACT_PROTOTYPE ) )
	   victim->pIndexData->resistant = victim->resistant; 
	return;
    }

    if ( !str_cmp( arg2, "immune" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	    return;
	if ( !argument || argument[0] == '\0' )
	{
	    send_to_char( "Usage: mset <victim> immune <flag> [flag]...\n\r", ch );
	    return;
	}
	while ( argument[0] != '\0' )
	{
	    argument = one_argument( argument, arg3 );
	    value = get_risflag( arg3 );
	    if ( value < 0 || value > 31 )
		ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
	    else
		TOGGLE_BIT( victim->immune, 1 << value );
	}
	if ( IS_NPC( victim ) && IS_SET( victim->act, ACT_PROTOTYPE ) )
	    victim->pIndexData->immune = victim->immune; 
	return;
    }

    if ( !str_cmp( arg2, "susceptible" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	    return;
	if ( !argument || argument[0] == '\0' )
	{
	    send_to_char( "Usage: mset <victim> susceptible <flag> [flag]...\n\r", ch );
	    return;
	}
	while ( argument[0] != '\0' )
	{
	    argument = one_argument( argument, arg3 );
	    value = get_risflag( arg3 );
	    if ( value < 0 || value > 31 )
		ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
	    else
		TOGGLE_BIT( victim->susceptible, 1 << value );
	}
	if ( IS_NPC( victim ) && IS_SET( victim->act, ACT_PROTOTYPE ) )
	    victim->pIndexData->susceptible = victim->susceptible; 
	return;
    }

    if ( !str_cmp( arg2, "part" ) )
    {
	if ( !can_mmodify( ch, victim ) )
	    return;
	if ( !argument || argument[0] == '\0' )
	{
	    send_to_char( "Usage: mset <victim> part <flag> [flag]...\n\r", ch );
	    return;
	}
	while ( argument[0] != '\0' )
	{
	    argument = one_argument( argument, arg3 );
	    value = get_partflag( arg3 );
	    if ( value < 0 || value > 31 )
		ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
	    else
		TOGGLE_BIT( victim->xflags, 1 << value );
	}
	if ( IS_NPC( victim ) && IS_SET( victim->act, ACT_PROTOTYPE ) )
	    victim->pIndexData->xflags = victim->xflags; 
	return;
    }

    if ( !str_cmp( arg2, "attack" ) )
    {
	if ( !IS_NPC( victim ) )
	{
	    send_to_char( "You can only modify a mobile's attacks.\n\r", ch );
	    return;
	}
	if ( !can_mmodify( ch, victim ) )
	    return;
	if ( !argument || argument[0] == '\0' )
	{
	    send_to_char( "Usage: mset <victim> attack <flag> [flag]...\n\r", ch );
	    send_to_char( "bite          claws        tail        sting      punch        kick\n\r", ch );
	    send_to_char( "trip          bash         stun        gouge      backstab\n\r", ch );
	    return;
	}
	while ( argument[0] != '\0' )
	{
	    argument = one_argument( argument, arg3 );
	    value = get_attackflag( arg3 );
	    if ( value < 0 || value > 31 )
		ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
	    else
		TOGGLE_BIT( victim->attacks, 1 << value );
	}
	if ( IS_NPC( victim ) && IS_SET( victim->act, ACT_PROTOTYPE ) )
	    victim->pIndexData->attacks = victim->attacks; 
	return;
    }

    if ( !str_cmp( arg2, "defense" ) )
    {
	if ( !IS_NPC( victim ) )
	{
	    send_to_char( "You can only modify a mobile's defenses.\n\r", ch );
	    return;
	}
	if ( !can_mmodify( ch, victim ) )
	    return;
	if ( !argument || argument[0] == '\0' )
	{
	    send_to_char( "Usage: mset <victim> defense <flag> [flag]...\n\r", ch );
	    send_to_char( "parry        dodge\n\r",ch );
	    return;
	}
	while ( argument[0] != '\0' )
	{
	    argument = one_argument( argument, arg3 );
	    value = get_defenseflag( arg3 );
	    if ( value < 0 || value > 31 )
		ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
	    else
		TOGGLE_BIT( victim->defenses, 1 << value );
	}
	if ( IS_NPC( victim ) && IS_SET( victim->act, ACT_PROTOTYPE ) )
	    victim->pIndexData->defenses = victim->defenses; 
	return;
    }

    if ( !str_cmp( arg2, "pos" ) )
    {
	if ( !IS_NPC(victim) )
	{
	    send_to_char( "Mobiles only.\n\r", ch );
	    return;
	}
	if ( !can_mmodify( ch, victim ) )
	    return;
	if ( value < 0 || value > POS_STANDING )
	{
	    ch_printf( ch, "Position range is 0 to %d.\n\r", POS_STANDING );
	    return;
	}
	victim->position = value;
	if ( IS_NPC( victim ) && IS_SET( victim->act, ACT_PROTOTYPE ) )
	    victim->pIndexData->position = victim->position; 
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "defpos" ) )
    {
	if ( !IS_NPC(victim) )
	{
	    send_to_char( "Mobiles only.\n\r", ch );
	    return;
	}
	if ( !can_mmodify( ch, victim ) )
	    return;
	if ( value < 0 || value > POS_STANDING )
	{
	    ch_printf( ch, "Position range is 0 to %d.\n\r", POS_STANDING );
	    return;
	}
	victim->defposition = value;
	if ( IS_NPC( victim ) && IS_SET( victim->act, ACT_PROTOTYPE ) )
	    victim->pIndexData->defposition = victim->defposition; 
	send_to_char( "Done.\n\r", ch );
	return;
    }

    /*
     * save some finger-leather
     */
    if ( !str_cmp( arg2, "hitdie" ) )
    {
	if ( !IS_NPC(victim) )
	{
	    send_to_char( "Mobiles only.\n\r", ch );
	    return;
	}
	if ( !can_mmodify( ch, victim ) )
	  return;

        sscanf(arg3,"%d %c %d %c %d",&num,&char1,&size,&char2,&plus);
	sprintf(outbuf,"%s hitnumdie %d",arg1, num);
        do_mset( ch, outbuf );

	sprintf(outbuf,"%s hitsizedie %d",arg1, size);
        do_mset( ch, outbuf );

	sprintf(outbuf,"%s hitplus %d",arg1, plus);
        do_mset( ch, outbuf );
        return;
    }
    /*
     * save some more finger-leather
     */
    if ( !str_cmp( arg2, "damdie" ) )
    {
	if ( !IS_NPC(victim) )
	{
	    send_to_char( "Mobiles only.\n\r", ch );
	    return;
	}
	if ( !can_mmodify( ch, victim ) )
	  return;

        sscanf(arg3,"%d %c %d %c %d",&num,&char1,&size,&char2,&plus);
	sprintf(outbuf,"%s damnumdie %d",arg1, num);
        do_mset( ch, outbuf );
	sprintf(outbuf,"%s damsizedie %d",arg1, size);
        do_mset( ch, outbuf );
	sprintf(outbuf,"%s damplus %d",arg1, plus);
        do_mset( ch, outbuf );
        return;
    }

    if ( !str_cmp( arg2, "hitnumdie" ) )
    {
	if ( !IS_NPC(victim) )
	{
	    send_to_char( "Mobiles only.\n\r", ch );
	    return;
	}
	if ( !can_mmodify( ch, victim ) )
	  return;
	if ( value < 0 || value > 32767 )
	{
	    send_to_char( "Number of hitpoint dice range is 0 to 30000.\n\r", ch );
	    return;
	}
	if ( IS_NPC( victim ) && IS_SET( victim->act, ACT_PROTOTYPE ) )
	  victim->pIndexData->hitnodice = value;
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "hitsizedie" ) )
    {
	if ( !IS_NPC(victim) )
	{
	    send_to_char( "Mobiles only.\n\r", ch );
	    return;
	}
	if ( !can_mmodify( ch, victim ) )
	  return;
	if ( value < 0 || value > 32767 )
	{
	    send_to_char( "Hitpoint dice size range is 0 to 30000.\n\r", ch );
	    return;
	}
	if ( IS_NPC( victim ) && IS_SET( victim->act, ACT_PROTOTYPE ) )
	  victim->pIndexData->hitsizedice = value;
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "hitplus" ) )
    {
	if ( !IS_NPC(victim) )
	{
	    send_to_char( "Mobiles only.\n\r", ch );
	    return;
	}
	if ( !can_mmodify( ch, victim ) )
	  return;
	if ( value < 0 || value > 32767 )
	{
	    send_to_char( "Hitpoint bonus range is 0 to 30000.\n\r", ch );
	    return;
	}
	if ( IS_NPC( victim ) && IS_SET( victim->act, ACT_PROTOTYPE ) )
	  victim->pIndexData->hitplus = value;
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "damnumdie" ) )
    {
	if ( !IS_NPC(victim) )
	{
	    send_to_char( "Mobiles only.\n\r", ch );
	    return;
	}
	if ( !can_mmodify( ch, victim ) )
	  return;
	if ( value < 0 || value > 100 )
	{
	    send_to_char( "Number of damage dice range is 0 to 100.\n\r", ch );
	    return;
	}
	if ( IS_NPC( victim ) && IS_SET( victim->act, ACT_PROTOTYPE ) )
	  victim->pIndexData->damnodice = value;
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "damsizedie" ) )
    {
	if ( !IS_NPC(victim) )
	{
	    send_to_char( "Mobiles only.\n\r", ch );
	    return;
	}
	if ( !can_mmodify( ch, victim ) )
	    return;
	if ( value < 0 || value > 100 )
	{
	    send_to_char( "Damage dice size range is 0 to 100.\n\r", ch );
	    return;
	}
	if ( IS_NPC( victim ) && IS_SET( victim->act, ACT_PROTOTYPE ) )
	  victim->pIndexData->damsizedice = value;
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "damplus" ) )
    {
	if ( !IS_NPC(victim) )
	{
	    send_to_char( "Mobiles only.\n\r", ch );
	    return;
	}
	if ( !can_mmodify( ch, victim ) )
	  return;
	if ( value < 0 || value > 1000 )
	{
	    send_to_char( "Damage bonus range is 0 to 1000.\n\r", ch );
	    return;
	}

	if ( IS_NPC( victim ) && IS_SET( victim->act, ACT_PROTOTYPE ) )
	  victim->pIndexData->damplus = value;
	send_to_char( "Done.\n\r", ch );
	return;

    }


    /*
     * Generate usage message.
     */
    if ( ch->substate == SUB_REPEATCMD )
    {
	ch->substate = SUB_RESTRICTED;
	interpret( ch, origarg );
	ch->substate = SUB_REPEATCMD;
	ch->last_cmd = do_mset;
    }
    else
	do_mset( ch, "" );
    return;
}


void do_oset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    char buf  [MAX_STRING_LENGTH];
    char outbuf  [MAX_STRING_LENGTH];
    OBJ_DATA *obj, *tmpobj;
    EXTRA_DESCR_DATA *ed;
    bool lockobj;
    char *origarg = argument;

    int value, tmp;

    if ( IS_NPC( ch ) )
    {
	send_to_char( "Mob's can't oset\n\r", ch );
	return;    
    }

    if ( !ch->desc )
    {
	send_to_char( "You have no descriptor\n\r", ch );
	return;
    }

    switch( ch->substate )
    {
	default:
	  break;

	case SUB_OBJ_EXTRA:
	  if ( !ch->dest_buf )
	  {
		send_to_char( "Fatal error: report to Thoric.\n\r", ch );
		bug( "do_oset: sub_obj_extra: NULL ch->dest_buf", 0 );
		ch->substate = SUB_NONE;
		return;
	  }
	  /*
	   * hopefully the object didn't get extracted...
	   * if you're REALLY paranoid, you could always go through
	   * the object and index-object lists, searching through the
	   * extra_descr lists for a matching pointer...
	   */
	  ed  = ch->dest_buf;
	  STRFREE( ed->description );
	  ed->description = copy_buffer( ch );
	  tmpobj = ch->spare_ptr;
	  stop_editing( ch );
	  ch->dest_buf = tmpobj;
	  ch->substate = ch->tempnum;
	  return;
	
	case SUB_OBJ_LONG:
	  if ( !ch->dest_buf )
	  {
		send_to_char( "Fatal error: report to Thoric.\n\r", ch );
		bug( "do_oset: sub_obj_long: NULL ch->dest_buf", 0 );
		ch->substate = SUB_NONE;
		return;
	  }
	  obj = ch->dest_buf;
	  if ( obj && obj_extracted(obj) )
	  {
		send_to_char( "Your object was extracted!\n\r", ch );
		stop_editing( ch );
		return;
	  }
	  STRFREE( obj->description );
	  obj->description = copy_buffer( ch );
	  if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
	  {
		STRFREE( obj->pIndexData->description );
		obj->pIndexData->description = QUICKLINK( obj->description );
	  }
	  tmpobj = ch->spare_ptr;
	  stop_editing( ch );
	  ch->substate = ch->tempnum;
	  ch->dest_buf = tmpobj;
	  return;
    }

    obj = NULL;
    smash_tilde( argument );

    if ( ch->substate == SUB_REPEATCMD )
    {
	obj = ch->dest_buf;
	if ( obj && obj_extracted(obj) )
	{
	    send_to_char( "Your object was extracted!\n\r", ch );
	    obj = NULL;
	    argument = "done";
	}
	if ( argument[0] == '\0' || !str_cmp( argument, " " )
	||   !str_cmp( argument, "stat" ) )
	{
	    if ( obj )
		do_ostat( ch, obj->name );
	    else
	        send_to_char( "No object selected.  Type '?' for help.\n\r", ch );
	    return;
	}
	if ( !str_cmp( argument, "done" ) || !str_cmp( argument, "off" ) )
	{
	    send_to_char( "Oset mode off.\n\r", ch );
	    ch->substate = SUB_NONE;
	    ch->dest_buf = NULL;
	    if ( ch->pcdata && ch->pcdata->subprompt )
		STRFREE( ch->pcdata->subprompt );
	    return;
	}
    }
    if ( obj )
    {
	lockobj = TRUE;
	strcpy( arg1, obj->name );
	argument = one_argument( argument, arg2 );
	strcpy( arg3, argument );
    }
    else
    {
	lockobj = FALSE;
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	strcpy( arg3, argument );
    }

/*
    if ( !str_cmp( arg1, "on" ) )
    {
	send_to_char( "Syntax: oset <object|vnum> on.\n\r", ch );
	return;
    }
*/

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !str_cmp( arg1, "?" ) )
    {
	if ( ch->substate == SUB_REPEATCMD )
	{
	    if ( obj )
		send_to_char( "Syntax: <field>  <value>\n\r",		ch );
	    else
		send_to_char( "Syntax: <object> <field>  <value>\n\r",	ch );
	}
	else
	    send_to_char( "Syntax: oset <object> <field>  <value>\n\r",	ch );
	send_to_char( "\n\r",						ch );
	send_to_char( "Field being one of:\n\r",			ch );
	send_to_char( "  flags wear level weight cost timer\n\r",	ch );
	send_to_char( "  name short long desc ed rmed actiondesc\n\r",	ch );
	send_to_char( "  type value0 value1 value2 value3 value4 value5\n\r",	ch );
	send_to_char( "  affect rmaffect layers\n\r",				ch );
	send_to_char( "For weapons:             For armor:\n\r",	ch );
	send_to_char( "  weapontype condition     ac condition\n\r",	ch );
	send_to_char( "  numdamdie sizedamdie                  \n\r",   ch );
	send_to_char( "  charges   maxcharges                  \n\r",   ch );
	send_to_char( "For devices:\n\r",			ch );
	send_to_char( "  slevel spell maxcharges charges\n\r",		ch );
	send_to_char( "For containers:          For levers and switches:\n\r", ch );
	send_to_char( "  cflags key capacity      tflags\n\r",		ch );
	return;
    }

    if ( !obj )
    {
	if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
	{
	   send_to_char( "There is nothing like that in all the realms.\n\r", ch );
	   return;
 	}
    }
    if ( lockobj )
	ch->dest_buf = obj;
    else
	ch->dest_buf = NULL;

    separate_obj( obj );
    value = atoi( arg3 );

/*
    if ( !str_cmp( arg2, "on" ) )
    {
	ch_printf( ch, "Oset mode on. (Editing '%s' vnum %ld).\n\r",
		obj->name, obj->pIndexData->vnum );
	ch->substate = SUB_REPEATCMD;
	ch->dest_buf = obj;
	if ( ch->pcdata )
	{
	   if ( ch->pcdata->subprompt )
		STRFREE( ch->pcdata->subprompt );
	   sprintf( buf, "<&COset &W#%d&w> %%i", obj->pIndexData->vnum );
	   ch->pcdata->subprompt = STRALLOC( buf );
	}
	return;
    }
*/

    if ( !str_cmp( arg2, "value0" ) || !str_cmp( arg2, "v0" ) )
    {
	if ( !can_omodify( ch, obj ) )
	  return;
	obj->value[0] = value;
	if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
	  obj->pIndexData->value[0] = value;
	return;
    }

    if ( !str_cmp( arg2, "value1" ) || !str_cmp( arg2, "v1" ) )
    {
	if ( !can_omodify( ch, obj ) )
	  return;
	obj->value[1] = value;
	if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
	  obj->pIndexData->value[1] = value;
	return;
    }

    if ( !str_cmp( arg2, "value2" ) || !str_cmp( arg2, "v2" ) )
    {
	if ( !can_omodify( ch, obj ) )
	  return;
	obj->value[2] = value;
	if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
	{
	  obj->pIndexData->value[2] = value;
	  if ( obj->item_type == ITEM_WEAPON && value != 0 )
	    obj->value[2] = obj->pIndexData->value[1] * obj->pIndexData->value[2];
	}
	return;
    }

    if ( !str_cmp( arg2, "value3" ) || !str_cmp( arg2, "v3" ) )
    {
	if ( !can_omodify( ch, obj ) )
	  return;
	obj->value[3] = value;
	if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
	  obj->pIndexData->value[3] = value;
	return;
    }

    if ( !str_cmp( arg2, "value4" ) || !str_cmp( arg2, "v4" ) )
    {
	if ( !can_omodify( ch, obj ) )
	  return;
	obj->value[4] = value;
	if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
	  obj->pIndexData->value[4] = value;
	return;
    }

    if ( !str_cmp( arg2, "value5" ) || !str_cmp( arg2, "v5" ) )
    {
	if ( !can_omodify( ch, obj ) )
	  return;
	obj->value[5] = value;
	if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
	  obj->pIndexData->value[5] = value;
	return;
    }

    if ( !str_cmp( arg2, "type" ) )
    {
	if ( !can_omodify( ch, obj ) )
	  return;
	if ( !argument || argument[0] == '\0' )
	{
	   send_to_char( "Usage: oset <object> type <type>\n\r", ch );
	   send_to_char( "Possible Types:\n\r", ch );
           send_to_char( "None        Light\n\r", ch );
           send_to_char( "Armor       Comlink    Fabric\n\r", ch );
           send_to_char( "Furniture   Trash      Container   Drink_con\n\r", ch );
           send_to_char( "Food        Money\n\r", ch );
           send_to_char( "Fountain    Weapon     Medpac\n\r", ch );
           send_to_char( "Superconductor         Rare_metal\n\r", ch );
           send_to_char( "Paper       Magnet\n\r", ch );
           send_to_char( "Lockpick    Shovel     Thread     Ammo\n\r", ch );
           send_to_char( "Lens        Crystal    Plastic    Battery\n\r", ch );	   
           send_to_char( "Toolkit     Metal      Oven       Mirror      Circuit\n\r", ch );	   
           send_to_char( "Device\n\r", ch );	   
           return;
	}
	value = get_otype( argument );
	if ( value < 1 )
	{
	     ch_printf( ch, "Unknown type: %s\n\r", arg3 );
	     return;	
	}
	obj->item_type = (sh_int) value;
	if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
	  obj->pIndexData->item_type = obj->item_type; 
	return;	
    }

    if ( !str_cmp( arg2, "flags" ) )
    {
	if ( !can_omodify( ch, obj ) )
	  return;
	if ( !argument || argument[0] == '\0' )
	{
	   send_to_char( "Usage: oset <object> flags <flag> [flag]...\n\r", ch );
	   send_to_char( "glow, dark, magic, bless, noremove,\n\r", ch );
           send_to_char( "donation, covering, hum, invis, nodrop\n\r", ch );
           send_to_char( "inventory, clanobject\n\r", ch );
	   send_to_char( "small_size, human_size, large_size, hutt_size\n\r", ch );
	   return;
	}
	while ( argument[0] != '\0' )
	{
	   argument = one_argument( argument, arg3 );
	   value = get_oflag( arg3 );
	   if ( value < 0 || value > 31 )
	     ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
	   else
	   {
	       TOGGLE_BIT(obj->extra_flags, 1 << value);
	       if ( 1 << value == ITEM_PROTOTYPE )
	         obj->pIndexData->extra_flags = obj->extra_flags;
	   }
	}
	if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
	  obj->pIndexData->extra_flags = obj->extra_flags; 
	return;
    }

    if ( !str_cmp( arg2, "wear" ) )
    {
	if ( !can_omodify( ch, obj ) )
	  return;
	if ( !argument || argument[0] == '\0' )
	{
	   send_to_char( "Usage: oset <object> wear <flag> [flag]...\n\r", ch );
           send_to_char( "Possible locations:\n\r", ch );
           send_to_char( "take   finger   neck    body    head   legs\n\r", ch );
           send_to_char( "feet   hands    arms    shield  about  waist\n\r", ch );
           send_to_char( "wrist  wield    hold    ears    eyes\n\r", ch );
	   return;
	}
	while ( argument[0] != '\0' )
	{
	   argument = one_argument( argument, arg3 );
	   value = get_wflag( arg3 );
	   if ( value < 0 || value > 31 )
	     ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
	   else
	     TOGGLE_BIT( obj->wear_flags, 1 << value );
	}

	if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
	  obj->pIndexData->wear_flags = obj->wear_flags;
	return;
    }

    if ( !str_cmp( arg2, "level" ) )
    {
	if ( !can_omodify( ch, obj ) )
	  return;
	obj->level = value;
	return;
    }

    if ( !str_cmp( arg2, "weight" ) )
    {
	if ( !can_omodify( ch, obj ) )
	  return;
	obj->weight = value;
	if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
	  obj->pIndexData->weight = value;
	return;
    }

    if ( !str_cmp( arg2, "cost" ) )
    {
	if ( !can_omodify( ch, obj ) )
	  return;
	obj->cost = value;
	if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
	  obj->pIndexData->cost = value;
	return;
    }

    if ( !str_cmp( arg2, "layers" ) )
    {
	if ( !can_omodify( ch, obj ) )
	  return;
	if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
	  obj->pIndexData->layers = value;
	else
	  send_to_char( "Item must have prototype flag to set this value.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "timer" ) )
    {
	if ( !can_omodify( ch, obj ) )
	  return;
	obj->timer = value;
	return;
    }
	
    if ( !str_cmp( arg2, "name" ) )
    {
	if ( !can_omodify( ch, obj ) )
	  return;
	STRFREE( obj->name );
	obj->name = STRALLOC( arg3 );
	if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
	{
	   STRFREE(obj->pIndexData->name );
	   obj->pIndexData->name = QUICKLINK( obj->name );
	}
	return;
    }

    if ( !str_cmp( arg2, "short" ) )
    {
	STRFREE( obj->short_descr );
	obj->short_descr = STRALLOC( arg3 );
	if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
	{
	   STRFREE(obj->pIndexData->short_descr );
	   obj->pIndexData->short_descr = QUICKLINK( obj->short_descr );
	}
        else
        /* Feature added by Narn, Apr/96 
	 * If the item is not proto, add the word 'rename' to the keywords
	 * if it is not already there.
	 */
        {
          if ( str_infix( "rename", obj->name ) )
          {
            sprintf( buf, "%s %s", obj->name, "rename" );
	    STRFREE( obj->name );
	    obj->name = STRALLOC( buf );
          }
        }
	return;
    }

    if ( !str_cmp( arg2, "actiondesc" ) )
    {
	if ( strstr( arg3, "%n" )
	||   strstr( arg3, "%d" )
	||   strstr( arg3, "%l" ) )
	{
	   send_to_char( "Illegal characters!\n\r", ch );
	   return;
	}
	STRFREE( obj->action_desc );
	obj->action_desc = STRALLOC( arg3 );
	if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
	{
	   STRFREE(obj->pIndexData->action_desc );
	   obj->pIndexData->action_desc = QUICKLINK( obj->action_desc );
	}
	return;
    }

    if ( !str_cmp( arg2, "long" ) )
    {
	if ( arg3[0] )
	{
	   STRFREE( obj->description );
	   obj->description = STRALLOC( arg3 );
	   if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
	   {
	      STRFREE(obj->pIndexData->description );
	      obj->pIndexData->description = QUICKLINK( obj->description );
	   }
	   return;
	}
	CHECK_SUBRESTRICTED( ch );
	if ( ch->substate == SUB_REPEATCMD )
	  ch->tempnum = SUB_REPEATCMD;
	else
	  ch->tempnum = SUB_NONE;
	if ( lockobj )
	  ch->spare_ptr = obj;
	else
	  ch->spare_ptr = NULL;
	ch->substate = SUB_OBJ_LONG;
	ch->dest_buf = obj;
	start_editing( ch, obj->description );
	return;
    }

    if ( !str_cmp( arg2, "affect" ) )
    {
	AFFECT_DATA *paf;
	sh_int loc;
	int bitv;

	argument = one_argument( argument, arg2 );
	if ( !arg2 || arg2[0] == '\0' || !argument || argument[0] == 0 )
	{
	   send_to_char( "Usage: oset <object> affect <field> <value>\n\r", ch );
           send_to_char( "Affect Fields:\n\r", ch );
           send_to_char( "none        strength    dexterity   intelligence  wisdom       constitution\n\r", ch );
           send_to_char( "sex         level       age         height        weight       force\n\r", ch );
           send_to_char( "hit         move        credits     experience    armor        hitroll\n\r", ch );
           send_to_char( "damroll     save_para   save_rod    save_poison   save_breath  save_power\n\r", ch );
           send_to_char( "charisma    resistant   immune      susceptible   affected     luck\n\r", ch );
           send_to_char( "backstab    pick        track       steal         sneak        hide\n\r", ch );
           send_to_char( "detrap      dodge       peek        scan          gouge        search\n\r", ch );
           send_to_char( "mount       disarm      kick        parry         bash         stun\n\r", ch );
           send_to_char( "punch       climb       grip        scribe        brew\n\r", ch );
	   return;
	}
	loc = get_atype( arg2 );
	if ( loc < 1 )
	{
	     ch_printf( ch, "Unknown field: %s\n\r", arg2 );
	     return;	
	}
	if ( loc >= APPLY_AFFECT && loc < APPLY_WEAPONSPELL )
	{
	   bitv = 0;
	   while ( argument[0] != '\0' )
	   {
		argument = one_argument( argument, arg3 );
		if ( loc == APPLY_AFFECT )
		  value = get_aflag( arg3 );
		else
		  value = get_risflag( arg3 );
		if ( value < 0 || value > 31 )
		  ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
		else
		  SET_BIT( bitv, 1 << value );
	   }
	   if ( !bitv )
	     return;
	   value = bitv;
	}
	else
	{
	   argument = one_argument( argument, arg3 );
	   value = atoi( arg3 );
	}
	CREATE( paf, AFFECT_DATA, 1 );
	paf->type		= -1;
	paf->duration		= -1;
	paf->location		= loc;
	paf->modifier		= value;
	paf->bitvector		= 0;
	paf->next		= NULL;
	if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
	  LINK( paf, obj->pIndexData->first_affect,
		     obj->pIndexData->last_affect, next, prev );
	else
	  LINK( paf, obj->first_affect, obj->last_affect, next, prev );
	++top_affect;
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "rmaffect" ) )
    {
	AFFECT_DATA *paf;
	sh_int loc, count;
	
	if ( !argument || argument[0] == '\0' )
	{
	   send_to_char( "Usage: oset <object> rmaffect <affect#>\n\r", ch );
	   return;
	}
	loc = atoi( argument );
	if ( loc < 1 )
	{
	     send_to_char( "Invalid number.\n\r", ch );
	     return;	
	}

	count = 0;
	
	if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
	{
	  OBJ_INDEX_DATA *pObjIndex;
	  
	  pObjIndex = obj->pIndexData;
	  for ( paf = pObjIndex->first_affect; paf; paf = paf->next )
	  {
		if ( ++count == loc )
		{
		   UNLINK( paf, pObjIndex->first_affect, pObjIndex->last_affect, next, prev );
		   DISPOSE( paf );
		   send_to_char( "Removed.\n\r", ch );
		   --top_affect;
		   return;
		}
	  }
	  send_to_char( "Not found.\n\r", ch );
	  return;
	}
	else
	{
	  for ( paf = obj->first_affect; paf; paf = paf->next )
	  {
		if ( ++count == loc )
		{
		   UNLINK( paf, obj->first_affect, obj->last_affect, next, prev );
		   DISPOSE( paf );
		   send_to_char( "Removed.\n\r", ch );
		   --top_affect;
		   return;
		}
	  }
	  send_to_char( "Not found.\n\r", ch );
	  return;
	}
    }

    if ( !str_cmp( arg2, "ed" ) )
    {
	if ( !arg3 || arg3[0] == '\0' )
	{
	    send_to_char( "Syntax: oset <object> ed <keywords>\n\r",
		ch );
	    return;
	}
	CHECK_SUBRESTRICTED( ch );
	if ( obj->timer )
	{
	   send_to_char("It's not safe to edit an extra description on an object with a timer.\n\rTurn it off first.\n\r", ch );
	   return;
	}
	if ( obj->item_type == ITEM_PAPER )
	{
	   send_to_char("You can not add an extra description to a note paper at the moment.\n\r", ch);
	   return;
	}
	if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
	  ed = SetOExtraProto( obj->pIndexData, arg3 );
	else
	  ed = SetOExtra( obj, arg3 );
	if ( ch->substate == SUB_REPEATCMD )
	  ch->tempnum = SUB_REPEATCMD;
	else
	  ch->tempnum = SUB_NONE;
	if ( lockobj )
	  ch->spare_ptr = obj;
	else
	  ch->spare_ptr = NULL;
	ch->substate = SUB_OBJ_EXTRA;
	ch->dest_buf = ed;
	start_editing( ch, ed->description );
	return;
    }

    if ( !str_cmp( arg2, "desc" ) )
    {
	CHECK_SUBRESTRICTED( ch );
	if ( obj->timer )
	{
	   send_to_char("It's not safe to edit a description on an object with a timer.\n\rTurn it off first.\n\r", ch );
	   return;
	}
	if ( obj->item_type == ITEM_PAPER )
	{
	   send_to_char("You can not add a description to a note paper at the moment.\n\r", ch);
	   return;
	}
	if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
	  ed = SetOExtraProto( obj->pIndexData, obj->name );
	else
	  ed = SetOExtra( obj, obj->name );
	if ( ch->substate == SUB_REPEATCMD )
	  ch->tempnum = SUB_REPEATCMD;
	else
	  ch->tempnum = SUB_NONE;
	if ( lockobj )
	  ch->spare_ptr = obj;
	else
	  ch->spare_ptr = NULL;
	ch->substate = SUB_OBJ_EXTRA;
	ch->dest_buf = ed;
	start_editing( ch, ed->description );
	return;
    }

    
    
    if ( !str_cmp( arg2, "rmed" ) )
    {
	if ( !arg3 || arg3[0] == '\0' )
	{
	   send_to_char( "Syntax: oset <object> rmed <keywords>\n\r", ch );
	   return;
	}
	if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
	{
	    if ( DelOExtraProto( obj->pIndexData, arg3 ) )
		send_to_char( "Deleted.\n\r", ch );
	    else
		send_to_char( "Not found.\n\r", ch );
	    return;
	}
	if ( DelOExtra( obj, arg3 ) )
	  send_to_char( "Deleted.\n\r", ch );
	else
	  send_to_char( "Not found.\n\r", ch );
	return;
    }
    /*
     * save some finger-leather
     */
    if ( !str_cmp( arg2, "ris" ) )
    {
	sprintf(outbuf, "%s affect resistant %s", arg1, arg3);
        do_oset( ch, outbuf );
	sprintf(outbuf, "%s affect immune %s", arg1, arg3);
        do_oset( ch, outbuf );
	sprintf(outbuf, "%s affect susceptible %s", arg1, arg3);
        do_oset( ch, outbuf );
        return;
    }

    if ( !str_cmp( arg2, "r" ) )
    {
	sprintf(outbuf, "%s affect resistant %s", arg1, arg3);
        do_oset( ch, outbuf );
        return;
    }

    if ( !str_cmp( arg2, "i" ) )
    {
	sprintf(outbuf, "%s affect immune %s", arg1, arg3);
        do_oset( ch, outbuf );
        return;
    }
    if ( !str_cmp( arg2, "s" ) )
    {
	sprintf(outbuf, "%s affect susceptible %s", arg1, arg3);
        do_oset( ch, outbuf );
        return;
    }

    if ( !str_cmp( arg2, "ri" ) )
    {
	sprintf(outbuf, "%s affect resistant %s", arg1, arg3);
        do_oset( ch, outbuf );
	sprintf(outbuf, "%s affect immune %s", arg1, arg3);
        do_oset( ch, outbuf );
        return;
    }

    if ( !str_cmp( arg2, "rs" ) )
    {
	sprintf(outbuf, "%s affect resistant %s", arg1, arg3);
        do_oset( ch, outbuf );
	sprintf(outbuf, "%s affect susceptible %s", arg1, arg3);
        do_oset( ch, outbuf );
        return;
    }

    if ( !str_cmp( arg2, "is" ) )
    {
	sprintf(outbuf, "%s affect immune %s", arg1, arg3);
        do_oset( ch, outbuf );
	sprintf(outbuf, "%s affect susceptible %s", arg1, arg3);
        do_oset( ch, outbuf );
        return;
    }

    /*
     * Make it easier to set special object values by name than number
     * 						-Thoric
     */
    tmp = -1;
    switch( obj->item_type )
    {
	case ITEM_WEAPON:
	    if ( !str_cmp( arg2, "weapontype" ) )
	    {
		int x;

		value = -1;
		for ( x = 0; x < sizeof( weapon_table ) / sizeof( weapon_table[0] ); x++ )
		  if ( !str_cmp( arg3, weapon_table[x] ) )
		    value = x;
		if ( value < 0 )
		{
		    send_to_char( "Unknown weapon type.\n\r", ch );
		    send_to_char( "\n\rChoices:\n\r", ch );
		    send_to_char( "   none, lightsaber, vibro-blade, blaster\n\r", ch );
		    return;
		}
		tmp = 3;
		break;
	    }
	    if ( !str_cmp( arg2, "condition" ) )	tmp = 0;
	    if ( !str_cmp( arg2, "numdamdie" ) )        tmp = 1;
	    if ( !str_cmp( arg2, "sizedamdie" ) )       tmp = 2;
	    if ( !str_cmp( arg2, "charges" ) )          tmp = 4;
	    if ( !str_cmp( arg2, "maxcharges" ) )       tmp = 5;
	    if ( !str_cmp( arg2, "charge" ) )          tmp = 4;
	    if ( !str_cmp( arg2, "maxcharge" ) )       tmp = 5;
	    break;
	case ITEM_AMMO:
	    if ( !str_cmp( arg2, "charges" ) )          tmp = 0;
	    if ( !str_cmp( arg2, "charge" ) )          tmp = 0;
	    break;
	case ITEM_BATTERY:
	    if ( !str_cmp( arg2, "charges" ) )          tmp = 0;
	    if ( !str_cmp( arg2, "charge" ) )          tmp = 0;
	    break;
	case ITEM_ARMOR:
	    if ( !str_cmp( arg2, "condition" ) )	tmp = 0;
	    if ( !str_cmp( arg2, "ac" )	)		tmp = 1;
	    break;
	case ITEM_DEVICE:
	    if ( !str_cmp( arg2, "slevel" ) )		tmp = 0;
	    if ( !str_cmp( arg2, "spell" ) )
	    {
	    	tmp = 3;
		value = skill_lookup(arg3);
	    }
	    if ( !str_cmp( arg2, "maxcharges" )	)	tmp = 1;
	    if ( !str_cmp( arg2, "charges" ) )		tmp = 2;
	    break;
	case ITEM_CONTAINER:
	    if ( !str_cmp( arg2, "capacity" ) )		tmp = 0;
	    if ( !str_cmp( arg2, "cflags" ) )		tmp = 1;
	    if ( !str_cmp( arg2, "key" ) )		tmp = 2;
	    break;
    }
    if ( tmp >= 0 && tmp <= 5 )
    {
	if ( !can_omodify( ch, obj ) )
	  return;
	obj->value[tmp] = value;
	if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
	  obj->pIndexData->value[tmp] = value;
	return;
    }

    /*
     * Generate usage message.
     */
    if ( ch->substate == SUB_REPEATCMD )
    {
	ch->substate = SUB_RESTRICTED;
	interpret( ch, origarg );
	ch->substate = SUB_REPEATCMD;
	ch->last_cmd = do_oset;
    }
    else
	do_oset( ch, "" );
    return;
}


/*
 * Obsolete Merc room editing routine
 */
void do_rset( CHAR_DATA *ch, char *argument )
{
    return;
}

/*
 * Returns value 0 - 9 based on directional text.
 */
int get_dir( char *txt )
{
    int edir;
    char c1,c2;

    if ( !str_cmp( txt, "northeast" ) )
      return DIR_NORTHEAST;
    if ( !str_cmp( txt, "northwest" ) )
      return DIR_NORTHWEST;
    if ( !str_cmp( txt, "southeast" ) )
      return DIR_SOUTHEAST;
    if ( !str_cmp( txt, "southwest" ) )
      return DIR_SOUTHWEST;
    if ( !str_cmp( txt, "somewhere" ) )
      return 10;

    c1 = txt[0];
    if ( c1 == '\0' )
      return 0;
    c2 = txt[1];
    edir = 0;
    switch ( c1 )
    {
	  case 'n':
	  switch ( c2 )
	  {
		    default:   edir = 0; break;	/* north */
		    case 'e':  edir = 6; break; /* ne	 */
		    case 'w':  edir = 7; break; /* nw	 */
	  }
	  break;    case '0':  edir = 0; break; /* north */
	  case 'e': case '1':  edir = 1; break; /* east  */
	  case 's':
	  switch ( c2 )
	  {
		    default:   edir = 2; break; /* south */
		    case 'e':  edir = 8; break; /* se	 */
		    case 'w':  edir = 9; break; /* sw	 */
	  }
	  break;    case '2':  edir = 2; break; /* south */
	  case 'w': case '3':  edir = 3; break; /* west	 */
	  case 'u': case '4':  edir = 4; break; /* up	 */
	  case 'd': case '5':  edir = 5; break; /* down	 */
		    case '6':  edir = 6; break; /* ne	 */
		    case '7':  edir = 7; break; /* nw	 */
		    case '8':  edir = 8; break; /* se	 */
		    case '9':  edir = 9; break; /* sw	 */
		    case '?':  edir = 10;break; /* somewhere */
    }
    return edir;
}

void do_redit( CHAR_DATA *ch, char *argument )
{
    char arg [MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char buf [MAX_STRING_LENGTH];
    ROOM_INDEX_DATA	*location, *tmp;
    EXTRA_DESCR_DATA	*ed;
    char		dir;
    EXIT_DATA		*xit, *texit;
    int			value;
    int			edir, ekey;
    long		 evnum;
    char		*origarg = argument;

    if ( !ch->desc )
    {
	send_to_char( "You have no descriptor.\n\r", ch );
	return;
    }

    switch( ch->substate )
    {
	default:
	  break;
	case SUB_ROOM_DESC:
	  location = ch->dest_buf;
	  if ( !location )
	  {
		bug( "redit: sub_room_desc: NULL ch->dest_buf", 0 );
		location = ch->in_room;
	  }
	  STRFREE( location->description );
	  location->description = copy_buffer( ch );
	  stop_editing( ch );
	  ch->substate = ch->tempnum;
	  return;
	case SUB_ROOM_EXTRA:
	  ed = ch->dest_buf;
	  if ( !ed )
	  {
		bug( "redit: sub_room_extra: NULL ch->dest_buf", 0 );
		stop_editing( ch );
		return;
	  }
	  STRFREE( ed->description );
	  ed->description = copy_buffer( ch );
	  stop_editing( ch );
	  ch->substate = ch->tempnum;
	  return;
    }

    location = ch->in_room;

    smash_tilde( argument );
    argument = one_argument( argument, arg );
    if ( ch->substate == SUB_REPEATCMD )
    {
	if ( arg[0] == '\0' )
	{
	    do_rstat( ch, "" );
	    return;
	}
	if ( !str_cmp( arg, "done" ) || !str_cmp( arg, "off" ) )
	{
	    send_to_char( "Redit mode off.\n\r", ch );
	    if ( ch->pcdata && ch->pcdata->subprompt )
		STRFREE( ch->pcdata->subprompt );
	    ch->substate = SUB_NONE;
	    return;
	}
    }
    if ( arg[0] == '\0' || !str_cmp( arg, "?" ) )
    {
	if ( ch->substate == SUB_REPEATCMD )
	  send_to_char( "Syntax: <field> value\n\r",			ch );
	else
	  send_to_char( "Syntax: redit <field> value\n\r",		ch );
	send_to_char( "\n\r",						ch );
	send_to_char( "Field being one of:\n\r",			ch );
	send_to_char( "  name desc ed rmed\n\r",			ch );
	send_to_char( "  exit bexit exdesc exflags exname exkey\n\r",	ch );
	send_to_char( "  flags sector teledelay televnum tunnel\n\r",	ch );
	send_to_char( "  exdistance\n\r",				ch );
	return;
    }

    if ( !can_rmodify( ch, location ) )
      return;

/*
    if ( !str_cmp( arg, "on" ) )
    {
	send_to_char( "Redit mode on.\n\r", ch );
	ch->substate = SUB_REPEATCMD;
	if ( ch->pcdata )
	{
	   if ( ch->pcdata->subprompt )
		STRFREE( ch->pcdata->subprompt );
	   ch->pcdata->subprompt = STRALLOC( "<&CRedit &W#%r&w> %i" );
	}
	return;
    }
*/

    if ( !str_cmp( arg, "substate" ) )
    {
	  argument = one_argument( argument, arg2);
          if( !str_cmp( arg2, "north" )  )
	  {
               ch->inter_substate = SUB_NORTH; 
	       return;
	  }
          if( !str_cmp( arg2, "east" )  )
	  {
               ch->inter_substate = SUB_EAST; 
	       return;
	  }
          if( !str_cmp( arg2, "south" )  )
	  {
               ch->inter_substate = SUB_SOUTH; 
	       return;
	  }
          if( !str_cmp( arg2, "west" )  )
	  {
               ch->inter_substate = SUB_WEST; 
	       return;
	  }
          if( !str_cmp( arg2, "up" )  )
	  {
               ch->inter_substate = SUB_UP; 
	       return;
	  }
          if( !str_cmp( arg2, "down" )  )
	  {
               ch->inter_substate = SUB_DOWN; 
	       return;
	  }
          send_to_char( " unrecognized substate in redit\n\r", ch);
	  return;
    }


    if ( !str_cmp( arg, "name" ) )
    {
	if ( argument[0] == '\0' )
	{
	   send_to_char( "Set the room name.  A very brief single line room description.\n\r", ch );
	   send_to_char( "Usage: redit name <Room summary>\n\r", ch );
	   return;
	}
	STRFREE( location->name );
	location->name = STRALLOC( argument );
	return;
    }

    if ( !str_cmp( arg, "desc" ) )
    {
	if ( ch->substate == SUB_REPEATCMD )
	  ch->tempnum = SUB_REPEATCMD;
	else
	  ch->tempnum = SUB_NONE;
	ch->substate = SUB_ROOM_DESC;
	ch->dest_buf = location;
	start_editing( ch, location->description );
	return;
    }

    if ( !str_cmp( arg, "tunnel" ) )
    {
	if ( !argument || argument[0] == '\0' )
	{
	   send_to_char( "Set the maximum characters allowed in the room at one time. (0 = unlimited).\n\r", ch );
	   send_to_char( "Usage: redit tunnel <value>\n\r", ch );
	   return;
	}
	location->tunnel = URANGE( 0, atoi(argument), 1000 );
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "ed" ) )
    {
	if ( !argument || argument[0] == '\0' )
	{
	   send_to_char( "Create an extra description.\n\r", ch );
	   send_to_char( "You must supply keyword(s).\n\r", ch );
	   return;
	}
	CHECK_SUBRESTRICTED( ch );
	ed = SetRExtra( location, argument );
	if ( ch->substate == SUB_REPEATCMD )
	  ch->tempnum = SUB_REPEATCMD;
	else
	  ch->tempnum = SUB_NONE;
	ch->substate = SUB_ROOM_EXTRA;
	ch->dest_buf = ed;
	start_editing( ch, ed->description );
	return;
    }

    if ( !str_cmp( arg, "rmed" ) )
    {
	if ( !argument || argument[0] == '\0' )
	{
	   send_to_char( "Remove an extra description.\n\r", ch );
	   send_to_char( "You must supply keyword(s).\n\r", ch );
	   return;
	}
	if ( DelRExtra( location, argument ) )
	  send_to_char( "Deleted.\n\r", ch );
	else
	  send_to_char( "Not found.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "flags" ) )
    {
	if ( !argument || argument[0] == '\0' )
	{
	   send_to_char( "Toggle the room flags.\n\r", ch );
	   send_to_char( "Usage: redit flags <flag> [flag]...\n\r", ch );
           send_to_char( "\n\rPossible Flags: \n\r", ch );
           send_to_char( "dark, nomob, indoors, nomagic, bank,\n\r", ch );
           send_to_char( "private, safe, petshop, norecall, donation, nodropall, silence,\n\r", ch );
           send_to_char( "logspeach, nodrop, clanstoreroom, plr_home, empty_home, teleport\n\r", ch );
           send_to_char( "nofloor\n\r", ch );
           send_to_char( "spacecraft, auction, no_drive, can_land, can_fly, hotel\n\r", ch );                                               
           return;
	}
	while ( argument[0] != '\0' )
	{
	   argument = one_argument( argument, arg2 );
	   value = get_rflag( arg2 );
	   if ( value < 0 || value > 31 )
	     ch_printf( ch, "Unknown flag: %s\n\r", arg2 );
	   else
	   {
     	       if ( 1 << value == ROOM_BARRACKS || 1 << value == ROOM_CONTROL  )
 	          send_to_char( "&RSetting Barracks or Control flags can mess up a clans production and revenue.\n\r", ch );

	       TOGGLE_BIT( location->room_flags, 1 << value );
	   }
	}
	return;
    }

    if ( !str_cmp( arg, "teledelay" ) )
    {
	if ( !argument || argument[0] == '\0' )
	{
	   send_to_char( "Set the delay of the teleport. (0 = off).\n\r", ch );
	   send_to_char( "Usage: redit teledelay <value>\n\r", ch );
	   return;
	}
	location->tele_delay = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "televnum" ) )
    {
	if ( !argument || argument[0] == '\0' )
	{
	   send_to_char( "Set the vnum of the room to teleport to.\n\r", ch );
	   send_to_char( "Usage: redit televnum <vnum>\n\r", ch );
	   return;
	}
	location->tele_vnum = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "sector" ) )
    {
	if ( !argument || argument[0] == '\0' )
	{
	   send_to_char( "Set the sector type.\n\r", ch );
	   send_to_char( "Usage: redit sector <value>\n\r", ch );
           send_to_char( "\n\rSector Values:\n\r", ch );
           send_to_char( "0:dark, 1:city, 2:field, 3:forest, 4:hills, 5:mountain, 6:water_swim\n\r", ch );
           send_to_char( "7:water_noswim, 8:underwater, 9:air, 10:desert, 11:unkown, 12:oceanfloor, 13:underground\n\r", ch );
           send_to_char( "14:scrub, 15:rocky, 16:savanna, 17:tundra, 18:glacial, 19:rainforest, 20:jungle\n\r", ch );
           send_to_char( "21:swamp, 22:wetlands, 23:brush, 24:steppe, 25:farmland, 26:volcano\n\r", ch );
                                 
	   return;
	}
	if ( location->sector_type <= SECT_CITY && location->area && location->area->planet )
	     location->area->planet->citysize--;
	else if ( location->sector_type == SECT_FARMLAND && location->area && location->area->planet )
	     location->area->planet->farmland--;
	else if ( location->sector_type != SECT_DUNNO && location->area && location->area->planet )
	     location->area->planet->wilderness--;
	      
	location->sector_type = atoi( argument );
	if ( location->sector_type < 0 || location->sector_type >= SECT_MAX )
	{
	  location->sector_type = 1;
	  send_to_char( "Out of range\n\r.", ch ); 
	}
	else
	  send_to_char( "Done.\n\r", ch );
	if ( location->sector_type <= SECT_CITY && location->area && location->area->planet )
	     location->area->planet->citysize++;
	else if ( location->sector_type == SECT_FARMLAND && location->area && location->area->planet )
	     location->area->planet->farmland++;
	else if ( location->sector_type != SECT_DUNNO && location->area && location->area->planet )
	     location->area->planet->wilderness++;
	return;
    }

    if ( !str_cmp( arg, "exkey" ) )
    {
	argument = one_argument( argument, arg2 );
	argument = one_argument( argument, arg3 );
	if ( arg2[0] == '\0' || arg3[0] == '\0' )
	{
	   send_to_char( "Usage: redit exkey <dir> <keycode>\n\r", ch );
	   return;
	}
	if ( arg2[0] == '#' )
	{
	   edir = atoi( arg2+1 );
	   xit = get_exit_num( location, edir );
	}
	else
	{
	   edir = get_dir( arg2 );
	   xit = get_exit( location, edir );
	}
	value = atoi( arg3 );
	if ( !xit )
	{
	   send_to_char( "No exit in that direction.  Use 'redit exit ...' first.\n\r", ch );
	   return;
	}
	xit->key = value;
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "exname" ) )
    {
	argument = one_argument( argument, arg2 );
	if ( arg2[0] == '\0' )
	{
	   send_to_char( "Change or clear exit keywords.\n\r", ch );
	   send_to_char( "Usage: redit exname <dir> [keywords]\n\r", ch );
	   return;
	}
	if ( arg2[0] == '#' )
	{
	   edir = atoi( arg2+1 );
	   xit = get_exit_num( location, edir );
	}
	else
	{
	   edir = get_dir( arg2 );
	   xit = get_exit( location, edir );
	}
	if ( !xit )
	{
	   send_to_char( "No exit in that direction.  Use 'redit exit ...' first.\n\r", ch );
	   return;
	}
	STRFREE( xit->keyword );
	xit->keyword = STRALLOC( argument );
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "exflags" ) )
    {
	if ( !argument || argument[0] == '\0' )
	{
	   send_to_char( "Toggle or display exit flags.\n\r", ch );
	   send_to_char( "Usage: redit exflags <dir> <flag> [flag]...\n\r", ch );
           send_to_char( "\n\rExit flags:\n\r", ch );
           send_to_char( "isdoor, closed, locked, can_look, searchable, can_leave, can_climb,\n\r", ch );
           send_to_char( "nopassdoor, secret, pickproof, fly, climb, dig, window, auto, can_enter\n\r", ch );
           send_to_char( "hidden, no_mob, bashproof, bashed\n\r", ch );
                                            
	   return;
	}
	argument = one_argument( argument, arg2 );
	if ( arg2[0] == '#' )
	{
	   edir = atoi( arg2+1 );
	   xit = get_exit_num( location, edir );
	}
	else
	{
	   edir = get_dir( arg2 );
	   xit = get_exit( location, edir );
	}
	if ( !xit )
	{
	   send_to_char( "No exit in that direction.  Use 'redit exit ...' first.\n\r", ch );
	   return;
	}
	if ( argument[0] == '\0' )
	{
	   sprintf( buf, "Flags for exit direction: %d  Keywords: %s  Key: %d\n\r[ ",
	   	xit->vdir, xit->keyword, xit->key );
	   for ( value = 0; value <= MAX_EXFLAG; value++ )
	   {
		if ( IS_SET( xit->exit_info, 1 << value ) )
		{
		    strcat( buf, ex_flags[value] );
		    strcat( buf, " " );
		}
	   }
	   strcat( buf, "]\n\r" );
	   send_to_char( buf, ch );
	   return;
	}
	while ( argument[0] != '\0' )
	{
	   argument = one_argument( argument, arg2 );
	   value = get_exflag( arg2 );
	   if ( value < 0 || value > MAX_EXFLAG )
	     ch_printf( ch, "Unknown flag: %s\n\r", arg2 );
	   else
	     TOGGLE_BIT( xit->exit_info, 1 << value );
	}
	return;
    }



    if ( !str_cmp( arg, "ex_flags" ) )
    {
	argument = one_argument( argument, arg2 );
        switch(ch->inter_substate)
	{
           case SUB_EAST : dir = 'e'; edir = 1; break;
           case SUB_WEST : dir = 'w'; edir = 3; break;
           case SUB_SOUTH: dir = 's'; edir = 2; break;
           case SUB_UP   : dir = 'u'; edir = 4; break;
           case SUB_DOWN : dir = 'd'; edir = 5; break;
	   default:
           case SUB_NORTH: dir = 'n'; edir = 0; break;
	}

	value = get_exflag(arg2);
        if ( value < 0 )
	{
           send_to_char("Bad exit flag. \n\r", ch);
	   return;
	}
	if ( (xit = get_exit(location,edir)) == NULL )
	{ 
	   sprintf(buf,"exit %c 1",dir);
	   do_redit(ch,buf);
	   xit = get_exit(location,edir);
	}     
	TOGGLE_BIT( xit->exit_info, value );
	return;
    }


    if ( !str_cmp( arg, "ex_to_room" ) )
    {
	argument = one_argument( argument, arg2 );
        switch(ch->inter_substate)
	{
           case SUB_EAST : dir = 'e'; edir = 1; break;
           case SUB_WEST : dir = 'w'; edir = 3; break;
           case SUB_SOUTH: dir = 's'; edir = 2; break;
           case SUB_UP   : dir = 'u'; edir = 4; break;
           case SUB_DOWN : dir = 'd'; edir = 5; break;
	   default:
           case SUB_NORTH: dir = 'n'; edir = 0; break;
	}
	evnum = atoi(arg2);
	if ( evnum < 1 || evnum > 32766 )
	{
	    send_to_char( "Invalid room number.\n\r", ch );
	    return;
	}
	if ( (tmp = get_room_index( evnum )) == NULL )
	{
	    send_to_char( "Non-existant room.\n\r", ch );
	    return;
	}
	if ( (xit = get_exit(location,edir)) == NULL )
	{ 
	   sprintf(buf,"exit %c 1",dir);
	   do_redit(ch,buf);
	   xit = get_exit(location,edir);
	}     
	xit->vnum = evnum;
	return;
    }

    if ( !str_cmp( arg, "ex_key" ) )
    {
	argument = one_argument( argument, arg2 );
        switch(ch->inter_substate)
	{
           case SUB_EAST : dir = 'e'; edir = 1; break;
           case SUB_WEST : dir = 'w'; edir = 3; break;
           case SUB_SOUTH: dir = 's'; edir = 2; break;
           case SUB_UP   : dir = 'u'; edir = 4; break;
           case SUB_DOWN : dir = 'd'; edir = 5; break;
	   default:
           case SUB_NORTH: dir = 'n'; edir = 0; break;
	}
	if ( (xit = get_exit(location,edir)) == NULL )
	{ 
	   sprintf(buf,"exit %c 1",dir);
	   do_redit(ch,buf);
	   xit = get_exit(location,edir);
	}     
	xit->key = atoi( arg2 );
	return;
    }

    if ( !str_cmp( arg, "ex_exdesc" ) )  
    {
        switch(ch->inter_substate)
	{
           case SUB_EAST : dir = 'e'; edir = 1; break;
           case SUB_WEST : dir = 'w'; edir = 3; break;
           case SUB_SOUTH: dir = 's'; edir = 2; break;
           case SUB_UP   : dir = 'u'; edir = 4; break;
           case SUB_DOWN : dir = 'd'; edir = 5; break;
	   default:
           case SUB_NORTH: dir = 'n'; edir = 0; break;
	}
	if ( (xit = get_exit(location, edir)) == NULL )
	{ 
	   sprintf(buf,"exit %c 1",dir);
	   do_redit(ch,buf);
	}     
	sprintf(buf,"exdesc %c %s",dir,argument);
	do_redit(ch,buf);
	return;
    }

    if ( !str_cmp( arg, "ex_keywords" ) )  /* not called yet */
    {
        switch(ch->inter_substate)
	{
           case SUB_EAST : dir = 'e'; edir = 1; break;
           case SUB_WEST : dir = 'w'; edir = 3; break;
           case SUB_SOUTH: dir = 's'; edir = 2; break;
           case SUB_UP   : dir = 'u'; edir = 4; break;
           case SUB_DOWN : dir = 'd'; edir = 5; break;
	   default:
           case SUB_NORTH: dir = 'n'; edir = 0; break;
	}
	if ( (xit = get_exit(location, edir)) == NULL )
	{ 
	   sprintf(buf, "exit %c 1", dir);
	   do_redit(ch,buf);
	   if ( (xit = get_exit(location, edir)) == NULL )
	     return;
	}     
	sprintf( buf, "%s %s", xit->keyword, argument );
	STRFREE( xit->keyword );
	xit->keyword = STRALLOC( buf );
	return;
    }

    if ( !str_cmp( arg, "exit" ) )
    {
	bool addexit, numnotdir;

	argument = one_argument( argument, arg2 );
	argument = one_argument( argument, arg3 );
	if ( !arg2 || arg2[0] == '\0' )
	{
	    send_to_char( "Create, change or remove an exit.\n\r", ch );
	    send_to_char( "Usage: redit exit <dir> [room] [flags] [key] [keywords]\n\r", ch );
	    return;
	}
	addexit = numnotdir = FALSE;
	switch( arg2[0] )
	{
	    default:	edir = get_dir(arg2);			  break;
	    case '+':	edir = get_dir(arg2+1);	addexit = TRUE;	  break;
	    case '#':	edir = atoi(arg2+1);	numnotdir = TRUE; break;  
	}
	if ( !arg3 || arg3[0] == '\0' )
	    evnum = 0;
	else
	    evnum = atoi( arg3 );
	if ( numnotdir )
	{
	    if ( (xit = get_exit_num(location, edir)) != NULL )
	      edir = xit->vdir;
	}
	else
	    xit = get_exit(location, edir);
	if ( !evnum )
	{
	    if ( xit )
	    {
		extract_exit(location, xit);
		send_to_char( "Exit removed.\n\r", ch );
		return;
	    }
	    send_to_char( "No exit in that direction.\n\r", ch );
	    return;
	}
	if ( evnum < 1 || evnum > 32766 )
	{
	    send_to_char( "Invalid room number.\n\r", ch );
	    return;
	}
	if ( (tmp = get_room_index( evnum )) == NULL )
	{
	    send_to_char( "Non-existant room.\n\r", ch );
	    return;
	}
	if ( addexit || !xit )
	{
	    if ( numnotdir )
	    {
		send_to_char( "Cannot add an exit by number, sorry.\n\r", ch );
		return;
	    }
	    if ( addexit && xit && get_exit_to(location, edir, tmp->vnum) )
	    {
		send_to_char( "There is already an exit in that direction leading to that location.\n\r", ch );
		return;
	    }
	    xit = make_exit( location, tmp, edir );
	    xit->keyword		= STRALLOC( "" );
	    xit->description		= STRALLOC( "" );
	    xit->key			= -1;
	    xit->exit_info		= 0;
	    act( AT_IMMORT, "$n reveals a hidden passage!", ch, NULL, NULL, TO_ROOM );
	}
	else
	    act( AT_IMMORT, "Something is different...", ch, NULL, NULL, TO_ROOM );
	if ( xit->to_room != tmp )
	{
	    xit->to_room = tmp;
	    xit->vnum = evnum;
	    texit = get_exit_to( xit->to_room, rev_dir[edir], location->vnum );
	    if ( texit )
	    {
		texit->rexit = xit;
		xit->rexit = texit;
	    }
	}
	argument = one_argument( argument, arg3 );
	if ( arg3 && arg3[0] != '\0' )
	    xit->exit_info = atoi( arg3 );
	if ( argument && argument[0] != '\0' )
	{
	    one_argument( argument, arg3 );
	    ekey = atoi( arg3 );
	    if ( ekey != 0 || arg3[0] == '0' )
	    {
		argument = one_argument( argument, arg3 );
		xit->key = ekey;
	    }
	    if ( argument && argument[0] != '\0' )
	    {
		STRFREE( xit->keyword );
		xit->keyword = STRALLOC( argument );
	    }
	}
	send_to_char( "Done.\n\r", ch );
	return;
    }

    /*
     * Twisted and evil, but works				-Thoric
     * Makes an exit, and the reverse in one shot.
     */
    if ( !str_cmp( arg, "bexit" ) )
    {
	EXIT_DATA *xit, *rxit;
	char tmpcmd[MAX_INPUT_LENGTH];
	ROOM_INDEX_DATA *tmploc;
	long vnum;
	int exnum;
	char rvnum[MAX_INPUT_LENGTH];
	bool numnotdir;

	argument = one_argument( argument, arg2 );
	argument = one_argument( argument, arg3 );
	if ( !arg2 || arg2[0] == '\0' )
	{
	    send_to_char( "Create, change or remove a two-way exit.\n\r", ch );
	    send_to_char( "Usage: redit bexit <dir> [room] [flags] [key] [keywords]\n\r", ch );
	    return;
	}
	numnotdir = FALSE;
	switch( arg2[0] )
	{
	    default:
		edir = get_dir( arg2 );
		break;
	    case '#':
		numnotdir = TRUE;
		edir = atoi( arg2+1 );
		break;
	    case '+':
		edir = get_dir( arg2+1 );
		break;
	}
	tmploc = location;
	exnum = edir;
	if ( numnotdir )
	{
	    if ( (xit = get_exit_num(tmploc, edir)) != NULL )
	      edir = xit->vdir;
	}
	else
	    xit = get_exit(tmploc, edir);
	rxit = NULL;
	vnum = 0;
	rvnum[0] = '\0';
	if ( xit )
	{
	    vnum = xit->vnum;
	    if ( arg3[0] != '\0' )
	      sprintf( rvnum, "%ld", tmploc->vnum );
	    if ( xit->to_room )
	      rxit = get_exit(xit->to_room, rev_dir[edir]);
	    else
	      rxit = NULL;
	}
	sprintf( tmpcmd, "exit %s %s %s", arg2, arg3, argument );
	do_redit( ch, tmpcmd );
	if ( numnotdir )
	  xit = get_exit_num(tmploc, exnum);
	else
	  xit = get_exit(tmploc, edir);
	if ( !rxit && xit )
	{
	    vnum = xit->vnum;
	    if ( arg3[0] != '\0' )
	      sprintf( rvnum, "%ld", tmploc->vnum );
	    if ( xit->to_room )
	      rxit = get_exit(xit->to_room, rev_dir[edir]);
	    else
	      rxit = NULL;
	}
	if ( vnum )
	{
	    sprintf( tmpcmd, "%ld redit exit %d %s %s",
				vnum,
				rev_dir[edir],
				rvnum,
				argument );
	    do_at( ch, tmpcmd );
	}
	return;
    }

    if ( !str_cmp( arg, "exdistance" ) )
    {
	argument = one_argument( argument, arg2 );
	if ( !arg2 || arg2[0] == '\0' )
	{
	   send_to_char( "Set the distance (in rooms) between this room, and the destination room.\n\r", ch );
	   send_to_char( "Usage: redit exdistance <dir> [distance]\n\r", ch );
	   return;
	}
	if ( arg2[0] == '#' )
	{
	   edir = atoi( arg2+1 );
	   xit = get_exit_num( location, edir );
	}
	else
	{
	   edir = get_dir( arg2 );
	   xit = get_exit( location, edir );
	}
	if ( xit )
	{
	   xit->distance = URANGE( 1, atoi(argument), 50 );
	   send_to_char( "Done.\n\r", ch );
	   return;
	}
	send_to_char( "No exit in that direction.  Use 'redit exit ...' first.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "exdesc" ) )
    {
	argument = one_argument( argument, arg2 );
	if ( !arg2 || arg2[0] == '\0' )
	{
	   send_to_char( "Create or clear a description for an exit.\n\r", ch );
	   send_to_char( "Usage: redit exdesc <dir> [description]\n\r", ch );
	   return;
	}
	if ( arg2[0] == '#' )
	{
	   edir = atoi( arg2+1 );
	   xit = get_exit_num( location, edir );
	}
	else
	{
	   edir = get_dir( arg2 );
	   xit = get_exit( location, edir );
	}
	if ( xit )
	{
	   STRFREE( xit->description );
	   if ( !argument || argument[0] == '\0' )
	     xit->description = STRALLOC( "" );
	   else
	   {
	     sprintf( buf, "%s\n\r", argument );
	     xit->description = STRALLOC( buf );
	   }
	   send_to_char( "Done.\n\r", ch );
	   return;
	}
	send_to_char( "No exit in that direction.  Use 'redit exit ...' first.\n\r", ch );
	return;
    }

    /*
     * Generate usage message.
     */
    if ( ch->substate == SUB_REPEATCMD )
    {
	ch->substate = SUB_RESTRICTED;
	interpret( ch, origarg );
	ch->substate = SUB_REPEATCMD;
	ch->last_cmd = do_redit;
    }
    else
	do_redit( ch, "" );
    return;
}

void do_ocreate( CHAR_DATA *ch, char *argument )
{
    char arg [MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA	*pObjIndex;
    OBJ_DATA		*obj;
    long			 vnum, cvnum;

    if ( IS_NPC(ch) )
    {
	send_to_char( "Mobiles cannot create.\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg );

    vnum = is_number( arg ) ? atoi( arg ) : -1;

    if ( vnum == -1 || !argument || argument[0] == '\0' )
    {
	send_to_char( "Usage: ocreate <vnum> [copy vnum] <item name>\n\r", ch );
	return;
    }

    if ( vnum < 1 || vnum > 32767 )
    {
	send_to_char( "Bad number.\n\r", ch );
	return;
    }

    one_argument( argument, arg2 );
    cvnum = atoi( arg2 );
    if ( cvnum != 0 )
      argument = one_argument( argument, arg2 );
    if ( cvnum < 1 )
      cvnum = 0;

    if ( get_obj_index( vnum ) )
    {
	send_to_char( "An object with that number already exists.\n\r", ch );
	return;
    }    

    if ( IS_NPC( ch ) )
      return;

    pObjIndex = make_object( vnum, cvnum, argument );
    if ( !pObjIndex )
    {
	send_to_char( "Error.\n\r", ch );
	log_string( "do_ocreate: make_object failed." );
	return;
    }
    obj = create_object( pObjIndex, get_trust(ch) );
    obj_to_char( obj, ch );
    act( AT_IMMORT, "$n makes some ancient arcane gestures, and opens $s hands to reveal $p!", ch, obj, NULL, TO_ROOM );
    act( AT_IMMORT, "You make some ancient arcane gestures, and open your hands to reveal $p!", ch, obj, NULL, TO_CHAR );
}

void do_mcreate( CHAR_DATA *ch, char *argument )
{
    char arg [MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA	*pMobIndex;
    CHAR_DATA		*mob;
    long			 vnum, cvnum;

    if ( IS_NPC(ch) )
    {
	send_to_char( "Mobiles cannot create.\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg );
    
    vnum = is_number( arg ) ? atoi( arg ) : -1;

    if ( vnum == -1 || !argument || argument[0] == '\0' )
    {
	send_to_char( "Usage: mcreate <vnum> [cvnum] <mobile name>\n\r", ch );
	return;
    }

    if ( vnum < 1 || vnum > 32767 )
    {
	send_to_char( "Bad number.\n\r", ch );
	return;
    }

    one_argument( argument, arg2 );
    cvnum = atoi( arg2 );
    if ( cvnum != 0 )
      argument = one_argument( argument, arg2 );
    if ( cvnum < 1 )
      cvnum = 0;

    if ( get_mob_index( vnum ) )
    {
	send_to_char( "A mobile with that number already exists.\n\r", ch );
	return;
    }    

    if ( IS_NPC( ch ) )
      return;

    pMobIndex = make_mobile( vnum, cvnum, argument );
    if ( !pMobIndex )
    {
	send_to_char( "Error.\n\r", ch );
	log_string( "do_mcreate: make_mobile failed." );
	return;
    }
    mob = create_mobile( pMobIndex );
    char_to_room( mob, ch->in_room );
    act( AT_IMMORT, "$n waves $s arms about, and $N appears at $s command!", ch, NULL, mob, TO_ROOM );
    act( AT_IMMORT, "You wave your arms about, and $N appears at your command!", ch, NULL, mob, TO_CHAR );
}


/*
 * Simple but nice and handle line editor.			-Thoric
 */
void edit_buffer( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    EDITOR_DATA *edit;
    char cmd[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH+1];
    sh_int x, line, max_buf_lines;
    bool save;

    if ( (d = ch->desc) == NULL )
    {
	send_to_char( "You have no descriptor.\n\r", ch );
	return;
    }

   if ( d->connected != CON_EDITING )
   {
	send_to_char( "You can't do that!\n\r", ch );
	bug( "Edit_buffer: d->connected != CON_EDITING", 0 );
	return;
   }
    
   if ( ch->substate <= SUB_PAUSE )
   {
	send_to_char( "You can't do that!\n\r", ch );
	bug( "Edit_buffer: illegal ch->substate (%d)", ch->substate );
	d->connected = CON_PLAYING;
	return;
   }
   
   if ( !ch->editor )
   {
	send_to_char( "You can't do that!\n\r", ch );
	bug( "Edit_buffer: null editor", 0 );
	d->connected = CON_PLAYING;
	return;
   }
   
   edit = ch->editor;
   save = FALSE;
   max_buf_lines = 24;

   if ( ch->substate == SUB_MPROG_EDIT || ch->substate == SUB_HELP_EDIT )
     max_buf_lines = 48;

   if ( argument[0] == '/' || argument[0] == '\\' )
   {
	one_argument( argument, cmd );
	if ( !str_cmp( cmd+1, "?" ) )
	{
	    send_to_char( "Editing commands\n\r---------------------------------\n\r", ch );
	    send_to_char( "/l              list buffer\n\r",	ch );
	    send_to_char( "/c              clear buffer\n\r",	ch );
	    send_to_char( "/d [line]       delete line\n\r",	ch );
	    send_to_char( "/g <line>       goto line\n\r",	ch );
	    send_to_char( "/i <line>       insert line\n\r",	ch );
	    send_to_char( "/r <old> <new>  global replace\n\r",	ch );
	    send_to_char( "/a              abort editing\n\r",	ch );
	    send_to_char( "/f              format text ( to fit screen )\n\r",  ch );
	      send_to_char( "/! <command>    execute command (do not use another editing command)\n\r",  ch );
	    send_to_char( "/s              save buffer\n\r\n\r> ",ch );
	    return;
	}
	
	if ( !str_cmp( cmd+1, "c" ) )
	{
	    
	    memset( edit, '\0', sizeof(EDITOR_DATA) );
	    edit->numlines = 0;
	    edit->on_line   = 0;
	    send_to_char( "Buffer cleared.\n\r> ", ch );
	    return;
	}
	if ( !str_cmp( cmd+1, "r" ) )
	{
	    char word1[MAX_INPUT_LENGTH];
	    char word2[MAX_INPUT_LENGTH];
	    char *sptr, *wptr, *lwptr;
	    int x, count, wordln, word2ln, lineln;

	    sptr = one_argument( argument, word1 );
	    sptr = one_argument( sptr, word1 );
	    sptr = one_argument( sptr, word2 );
	    if ( word1[0] == '\0' || word2[0] == '\0' )
	    {
		send_to_char( "Need word to replace, and replacement.\n\r> ", ch );
		return;
	    }
	    if ( strcmp( word1, word2 ) == 0 )
	    {
		send_to_char( "Done.\n\r> ", ch );
		return;
	    }
	    count = 0;  wordln = strlen(word1);  word2ln = strlen(word2);
	    ch_printf( ch, "Replacing all occurrences of %s with %s...\n\r", word1, word2 );
	    for ( x = edit->on_line; x < edit->numlines; x++ )
	    {
		lwptr = edit->line[x];
		while ( (wptr = strstr( lwptr, word1 )) != NULL )
		{
		    sptr = lwptr;
		    lwptr = wptr + wordln;
		    sprintf( buf, "%s%s", word2, wptr + wordln );
		    lineln = wptr - edit->line[x] - wordln;
		    ++count;
		    if ( strlen(buf) + lineln > 79 )
		    {
			lineln = UMAX(0, (79 - strlen(buf)));
			buf[lineln] = '\0';
			break;
		    }
		    else
			lineln = strlen(buf);
		    buf[lineln] = '\0';
		    strcpy( wptr, buf );
		}
	    }
	    ch_printf( ch, "Found and replaced %d occurrence(s).\n\r> ", count );
	    return;
	}
        
        if ( !str_cmp( cmd+1, "f" ) )
	{
	    char   temp_buf[MAX_STRING_LENGTH+max_buf_lines];
	    int    x, ep, old_p, end_mark; 
	    int    p = 0;
	    
	    for ( x = 0; x < edit->numlines; x++ )
	    {
	       strcpy ( temp_buf+p , edit->line[x] );
	       p += strlen( edit->line[x] );
	       temp_buf[p] = ' ';
	       p++;
	    }
	    
	    temp_buf[p] = '\0';
	    end_mark = p;
	    p = 75;
	    old_p =0;
	    edit->on_line =0;
	    edit->numlines =0;
	    
	    while ( old_p < end_mark )
	    {
	        while ( temp_buf[p] != ' ' && p > old_p )
	          p--;
	          
	        if ( p == old_p )
	          p +=75;
	        
	        if ( p > end_mark )
	          p = end_mark;
	        
	        ep =0;
	        for ( x= old_p ; x < p ; x++ )  
	        {
	           edit->line[edit->on_line][ep] = temp_buf[x];
                   ep++;
                }
                edit->line[edit->on_line][ep] = '\0';

	        edit->on_line++;
	        edit->numlines++;
	        
	        old_p = p+1 ;
	        p += 75;
	        
	    }
	    
	    send_to_char( "OK.\n\r> ", ch );
	    return;
	}
	
	if ( !str_cmp( cmd+1, "i" ) )
	{
	    if ( edit->numlines >= max_buf_lines )
		send_to_char( "Buffer is full.\n\r> ", ch );
	    else
	    {
		if ( argument[2] == ' ' )
		  line = atoi( argument + 2 ) - 1;
		else
		  line = edit->on_line;
		if ( line < 0 )
		  line = edit->on_line;
		if ( line < 0 || line > edit->numlines )
		  send_to_char( "Out of range.\n\r> ", ch );
		else
		{
		  for ( x = ++edit->numlines; x > line; x-- )
			strcpy( edit->line[x], edit->line[x-1] );
		  strcpy( edit->line[line], "" );
		  send_to_char( "Line inserted.\n\r> ", ch );
		}
 	    }
	    return;
	}
	if ( !str_cmp( cmd+1, "d" ) )
	{
	    if ( edit->numlines == 0 )
		send_to_char( "Buffer is empty.\n\r> ", ch );
	    else
	    {
		if ( argument[2] == ' ' )
		  line = atoi( argument + 2 ) - 1;
		else
		  line = edit->on_line;
		if ( line < 0 )
		  line = edit->on_line;
		if ( line < 0 || line > edit->numlines )
		  send_to_char( "Out of range.\n\r> ", ch );
		else
		{
		  if ( line == 0 && edit->numlines == 1 )
		  {
			memset( edit, '\0', sizeof(EDITOR_DATA) );
			edit->numlines = 0;
			edit->on_line   = 0;
			send_to_char( "Line deleted.\n\r> ", ch );
			return;
		  }
		  for ( x = line; x < (edit->numlines - 1); x++ )
			strcpy( edit->line[x], edit->line[x+1] );
		  strcpy( edit->line[edit->numlines--], "" );
		  if ( edit->on_line > edit->numlines )
		    edit->on_line = edit->numlines;
		  send_to_char( "Line deleted.\n\r> ", ch );
		}
 	    }
	    return;
	}
	if ( !str_cmp( cmd+1, "g" ) )
	{
	    if ( edit->numlines == 0 )
		send_to_char( "Buffer is empty.\n\r> ", ch );
	    else
	    {
		if ( argument[2] == ' ' )
		  line = atoi( argument + 2 ) - 1;
		else
		{
		    send_to_char( "Goto what line?\n\r> ", ch );
		    return;
		}
		if ( line < 0 )
		  line = edit->on_line;
		if ( line < 0 || line > edit->numlines )
		  send_to_char( "Out of range.\n\r> ", ch );
		else
		{
		  edit->on_line = line;
		  ch_printf( ch, "(On line %d)\n\r> ", line+1 );
		}
 	    }
	    return;
	}
	if ( !str_cmp( cmd+1, "l" ) )
	{
	    if ( edit->numlines == 0 )
	      send_to_char( "Buffer is empty.\n\r> ", ch );
	    else
	    {
	      send_to_char( "------------------\n\r", ch );
	      for ( x = 0; x < edit->numlines; x++ )
		 ch_printf( ch, "%2d> %s\n\r", x+1, edit->line[x] );
	      send_to_char( "------------------\n\r> ", ch );
	    }
	    return;
	}
	if ( !str_cmp( cmd+1, "a" ) )
	{
	    send_to_char( "\n\rAborting... ", ch );
	    stop_editing( ch );
	    return;
	}
	if ( !str_cmp( cmd+1, "!" ) )
	{
	    DO_FUN *last_cmd;
	    int substate = ch->substate;

	    last_cmd = ch->last_cmd;
	    ch->substate = SUB_RESTRICTED;
	    interpret(ch, argument+3);
	    ch->substate = substate;
	    ch->last_cmd = last_cmd;
	    set_char_color( AT_GREEN, ch );
	    send_to_char( "\n\r> ", ch );
	    return;
	}
	if ( !str_cmp( cmd+1, "s" ) )
	{
	    d->connected = CON_PLAYING;
	    if ( !ch->last_cmd )
	      return;
	    (*ch->last_cmd) ( ch, "" );
	    return;
	}
   }

   if ( edit->size + strlen(argument) + 1 >= MAX_STRING_LENGTH - 1 )
	send_to_char( "You buffer is full.\n\r", ch );
   else
   {
        int b_end;
        int bm = 75;
        int bp = 0;
        int ep =0;

	strcpy( buf, argument );
	strcat( buf, " " );
	
	b_end = strlen(buf);
        
	while ( bp < b_end )
	{
	    while ( buf[bm] != ' ' && bm > bp )
	       bm--;
	       
	    if ( bm == bp ) 
	       bm += 75;
	    
	    if ( bm > b_end )
	       bm = b_end;
	    
	    ep=0;
	    while ( bp < bm )
	    {
	        edit->line[edit->on_line][ep] = buf[bp];
	        bp++;
	        ep++;      
            }
            
            bm = bp+75;
            bp ++;
            
            edit->line[edit->on_line][ep] = '\0';
            edit->on_line++;
            
            if ( edit->on_line > edit->numlines )
	       edit->numlines++;
	    if ( edit->numlines > max_buf_lines )
	    {
	        edit->numlines = max_buf_lines;
	        send_to_char( "Buffer full.\n\r", ch );
	        save = TRUE;
	        break;
	    }
	}
   }

   if ( save )
   {
      d->connected = CON_PLAYING;
      if ( !ch->last_cmd )
        return;
      (*ch->last_cmd) ( ch, "" );
      return;
   }
   send_to_char( "> ", ch );
}

void free_area( AREA_DATA *are )
{
    DISPOSE( are->name );
    DISPOSE( are->filename );
    DISPOSE( are );
    are = NULL;
}

void assign_area( CHAR_DATA *ch )
{
}

void do_aassign( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	AREA_DATA *tarea, *tmp;

	if ( IS_NPC( ch ) )
	  return;

	if ( argument[0] == '\0' )
	{
	    send_to_char( "Syntax: aassign <filename.are>\n\r", ch );
	    return;
	}

	if ( !str_cmp( "none", argument )
	||   !str_cmp( "null", argument )
	||   !str_cmp( "clear", argument ) )
	{
	    ch->pcdata->area = NULL;
	    assign_area( ch );
	    if ( !ch->pcdata->area )
	      send_to_char( "Area pointer cleared.\n\r", ch );
	    else
	      send_to_char( "Originally assigned area restored.\n\r", ch );
	    return;
	} 

	sprintf( buf, "%s", argument );
        tarea = NULL;


	  for ( tmp = first_area; tmp; tmp = tmp->next )
	    if ( !str_cmp( buf, tmp->filename ) )
	    {  
		tarea = tmp;
		break;
	    }

	 
	if ( !tarea )
	{
	    send_to_char( "No such area.  Use 'zones'.\n\r", ch );
	    return;
	}
	ch->pcdata->area = tarea;
	ch_printf( ch, "Assigning you: %s\n\r", tarea->name );
	return;
}


EXTRA_DESCR_DATA *SetRExtra( ROOM_INDEX_DATA *room, char *keywords )
{
    EXTRA_DESCR_DATA *ed;
    
    for ( ed = room->first_extradesc; ed; ed = ed->next )
    {
	  if ( is_name( keywords, ed->keyword ) )
	    break;
    }
    if ( !ed )
    {
	CREATE( ed, EXTRA_DESCR_DATA, 1 );
	LINK( ed, room->first_extradesc, room->last_extradesc, next, prev );
	ed->keyword	= STRALLOC( keywords );
	ed->description	= STRALLOC( "" );
	top_ed++;
    }
    return ed;
}

bool DelRExtra( ROOM_INDEX_DATA *room, char *keywords )
{
    EXTRA_DESCR_DATA *rmed;
    
    for ( rmed = room->first_extradesc; rmed; rmed = rmed->next )
    {
	  if ( is_name( keywords, rmed->keyword ) )
	    break;
    }
    if ( !rmed )
      return FALSE;
    UNLINK( rmed, room->first_extradesc, room->last_extradesc, next, prev );
    STRFREE( rmed->keyword );
    STRFREE( rmed->description );
    DISPOSE( rmed );
    top_ed--;
    return TRUE;
}

EXTRA_DESCR_DATA *SetOExtra( OBJ_DATA *obj, char *keywords )
{
    EXTRA_DESCR_DATA *ed;
    
    for ( ed = obj->first_extradesc; ed; ed = ed->next )
    {
	  if ( is_name( keywords, ed->keyword ) )
	    break;
    }
    if ( !ed )
    {
	CREATE( ed, EXTRA_DESCR_DATA, 1 );
	LINK( ed, obj->first_extradesc, obj->last_extradesc, next, prev );
	ed->keyword	= STRALLOC( keywords );
	ed->description	= STRALLOC( "" );
	top_ed++;
    }
    return ed;
}

bool DelOExtra( OBJ_DATA *obj, char *keywords )
{
    EXTRA_DESCR_DATA *rmed;
    
    for ( rmed = obj->first_extradesc; rmed; rmed = rmed->next )
    {
	  if ( is_name( keywords, rmed->keyword ) )
	    break;
    }
    if ( !rmed )
      return FALSE;
    UNLINK( rmed, obj->first_extradesc, obj->last_extradesc, next, prev );
    STRFREE( rmed->keyword );
    STRFREE( rmed->description );
    DISPOSE( rmed );
    top_ed--;
    return TRUE;
}

EXTRA_DESCR_DATA *SetOExtraProto( OBJ_INDEX_DATA *obj, char *keywords )
{
    EXTRA_DESCR_DATA *ed;
    
    for ( ed = obj->first_extradesc; ed; ed = ed->next )
    {
	  if ( is_name( keywords, ed->keyword ) )
	    break;
    }
    if ( !ed )
    {
	CREATE( ed, EXTRA_DESCR_DATA, 1 );
	LINK( ed, obj->first_extradesc, obj->last_extradesc, next, prev );
	ed->keyword	= STRALLOC( keywords );
	ed->description	= STRALLOC( "" );
	top_ed++;
    }
    return ed;
}

bool DelOExtraProto( OBJ_INDEX_DATA *obj, char *keywords )
{
    EXTRA_DESCR_DATA *rmed;
    
    for ( rmed = obj->first_extradesc; rmed; rmed = rmed->next )
    {
	  if ( is_name( keywords, rmed->keyword ) )
	    break;
    }
    if ( !rmed )
      return FALSE;
    UNLINK( rmed, obj->first_extradesc, obj->last_extradesc, next, prev );
    STRFREE( rmed->keyword );
    STRFREE( rmed->description );
    DISPOSE( rmed );
    top_ed--;
    return TRUE;
}

void fold_area( AREA_DATA *tarea, char *filename, bool install )
{
    ROOM_INDEX_DATA	*room;
    MPROG_DATA		*mprog;
    EXIT_DATA		*xit;
    EXTRA_DESCR_DATA	*ed;
    char		 buf[MAX_STRING_LENGTH];
    FILE		*fpout;
    long			 vnum;

    sprintf( buf, "Saving %s...", tarea->filename );
    log_string_plus( buf, LOG_NORMAL );

    sprintf( buf, "%s.bak", filename );
    rename( filename, buf );
    fclose( fpReserve );
    if ( ( fpout = fopen( filename, "w" ) ) == NULL )
    {
	bug( "fold_area: fopen", 0 );
	perror( filename );
	fpReserve = fopen( NULL_FILE, "r" );
	return;
    }
    
    fprintf( fpout, "#AREA   %s~\n\n\n\n", tarea->name );

    /* save rooms   */
    fprintf( fpout, "#ROOMS\n" );
    for ( room = tarea->first_room ; room ; room = room->next_in_area )
    {
	vnum = room->vnum;
	if ( install )
	{
	    CHAR_DATA *victim, *vnext;
	    OBJ_DATA  *obj, *obj_next;

	    /* purge room of (prototyped) mobiles */
	    for ( victim = room->first_person; victim; victim = vnext )
	    {
		vnext = victim->next_in_room;
		if ( IS_NPC(victim) )
		  extract_char( victim, TRUE );
	    }
	    /* purge room of (prototyped) objects */
	    for ( obj = room->first_content; obj; obj = obj_next )
	    {
		obj_next = obj->next_content;
		extract_obj( obj );
	    }
	}
	fprintf( fpout, "#%ld\n",	vnum				);
	fprintf( fpout, "%s~\n",	room->name			);
	fprintf( fpout, "%s~\n",	strip_cr( room->description )	);
	if ( (room->tele_delay > 0 && room->tele_vnum > 0) || room->tunnel > 0 )
	  fprintf( fpout, "0 %d %d %d %ld %d\n",	room->room_flags,
						room->sector_type,
						room->tele_delay,
						room->tele_vnum,
						room->tunnel		);
	else
	  fprintf( fpout, "0 %d %d\n",	room->room_flags,
					room->sector_type	);
	for ( xit = room->first_exit; xit; xit = xit->next )
	{
	   if ( IS_SET(xit->exit_info, EX_PORTAL) ) /* don't fold portals */
		continue;
	   fprintf( fpout, "D%d\n",		xit->vdir );
	   fprintf( fpout, "%s~\n",		strip_cr( xit->description ) );
	   fprintf( fpout, "%s~\n",		strip_cr( xit->keyword ) );
	   if ( xit->distance > 1 )
	     fprintf( fpout, "%d %d %ld %d\n",	xit->exit_info & ~EX_BASHED,
	   					xit->key,
	   					xit->vnum,
	   					xit->distance );
	   else
	     fprintf( fpout, "%d %d %ld\n",	xit->exit_info & ~EX_BASHED,
	   					xit->key,
	   					xit->vnum );
	}	
	for ( ed = room->first_extradesc; ed; ed = ed->next )
	   fprintf( fpout, "E\n%s~\n%s~\n",
			ed->keyword, strip_cr( ed->description ));

	if ( room->map )   /* maps */
	{
#ifdef OLDMAPS
	   fprintf( fpout, "M\n" );
	   fprintf( fpout, "%s~\n", strip_cr( room->map )	);
#endif
	   fprintf( fpout, "M %ld %d %d %c\n",	room->map->vnum
					      , room->map->x
					      , room->map->y
					      , room->map->entry );
	}
	if ( room->mudprogs )
	{
	  for ( mprog = room->mudprogs; mprog; mprog = mprog->next )
		fprintf( fpout, "> %s %s~\n%s~\n",
				mprog_type_to_name( mprog->type ),
				mprog->arglist, strip_cr(mprog->comlist) );
	  fprintf( fpout, "|\n" );	  
	}
	fprintf( fpout, "S\n" );
    }
    fprintf( fpout, "#0\n\n\n" );

    /* END */
    fprintf( fpout, "#$\n" );
    fclose( fpout );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}

void do_savearea( CHAR_DATA *ch, char *argument )
{
}

void do_loadarea( CHAR_DATA *ch, char *argument )
{
}

/*
 * Dangerous command.  Can be used to install an area that was either:
 *   (a) already installed but removed from area.lst
 *   (b) designed offline
 * The mud will likely crash if:
 *   (a) this area is already loaded
 *   (b) it contains vnums that exist
 *   (c) the area has errors
 *
 * NOTE: Use of this command is not recommended.		-Thoric
 */
void do_unfoldarea( CHAR_DATA *ch, char *argument )
{

    if ( !argument || argument[0] == '\0' )
    {
	send_to_char( "Unfold what?\n\r", ch );
	return;
    }

    fBootDb = TRUE;
    load_area_file( last_area, argument );
    fBootDb = FALSE;
    return;
}


void do_foldarea( CHAR_DATA *ch, char *argument )
{
    AREA_DATA	*tarea;
    char         arg[MAX_INPUT_LENGTH];
       
    argument = one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Usage: foldarea <filename> [remproto]\n\r", ch );
	return;
    }

    for ( tarea = first_area; tarea; tarea = tarea->next )
    {
	if ( !str_cmp( tarea->filename, arg ) )
	{
	  send_to_char( "Folding...\n\r", ch );
	  if (!strcmp( argument, "remproto") )
	     fold_area( tarea, tarea->filename, TRUE );
	  else
	     fold_area( tarea, tarea->filename, FALSE );
	  send_to_char( "Done.\n\r", ch );
	  return;
	}
    }
    send_to_char( "No such area exists.\n\r", ch );
    return;
}

extern int top_area;

void write_area_list( )
{
    AREA_DATA *tarea;
    FILE *fpout;

    fpout = fopen( AREA_LIST, "w" );
    if ( !fpout )
    {
	bug( "FATAL: cannot open area.lst for writing!\n\r", 0 );
 	return;
    }	  
    fprintf( fpout, "help.are\n" );
    for ( tarea = first_area; tarea; tarea = tarea->next )
	fprintf( fpout, "%s\n", tarea->filename );
    fprintf( fpout, "mobiles\n" );
    fprintf( fpout, "objects\n" );
    fprintf( fpout, "$\n" );
    fclose( fpout );
}

void do_astat( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *tarea;
    bool proto, found;
 
    found = FALSE; proto = FALSE;
    for ( tarea = first_area; tarea; tarea = tarea->next )
	if ( !str_cmp( tarea->filename, argument ) )
	{
	  found = TRUE;
	  break;
	}

    if ( !found )
    {
      if ( argument && argument[0] != '\0' ) 
      {
 	send_to_char( "Area not found.  Check 'zones'.\n\r", ch );
	return;
      }
      else
      {
        tarea = ch->in_room->area;
      }
    }

    ch_printf( ch, "Name: %s\n\rFilename: %-20s",
			tarea->name,
			tarea->filename);
    ch_printf( ch, "Area flags: %s\n\r", flag_string(tarea->flags, area_flags) );
    ch_printf( ch, "Planet: %s\n\r", tarea->planet ?  tarea->planet->name : "" );

}


void do_aset( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *tarea;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    bool proto, found;
    long  vnum;
    int   value;
    
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    vnum = atoi( argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Usage: aset <area filename> <field> <value>\n\r", ch );
	send_to_char( "\n\rField being one of:\n\r", ch );
	send_to_char( "  name filename\n\r", ch );
	send_to_char( "  flags planet\n\r", ch );
	return;
    }

    found = FALSE; proto = FALSE;
    for ( tarea = first_area; tarea; tarea = tarea->next )
	if ( !str_cmp( tarea->filename, arg1 ) )
	{
	  found = TRUE;
	  break;
	}

    if ( !found )
      for ( tarea = first_build; tarea; tarea = tarea->next )
	if ( !str_cmp( tarea->filename, arg1 ) )
	{
	  found = TRUE;
	  proto = TRUE;
	  break;
	}

    if ( !found )
    {
	send_to_char( "Area not found.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "name" ) )
    {
	DISPOSE( tarea->name );
	tarea->name = str_dup( argument );
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "no_edit" ) )
    {
        ROOM_INDEX_DATA * room;
        
        if ( str_cmp (argument , "wholearea" ) )
	{
	    send_to_char( "If you are sure you want the entire area not editable by players\n\rthen you must type, 'aset no_edit wholearea'.\n\r", ch );
	    return;
	}
       
        for  ( room = tarea->first_room ; room ; room = room->next_in_area )
            SET_BIT( room->room_flags , ROOM_NOPEDIT );
            
        send_to_char( "OK. Entire area set no_edit for players.\n\r", ch );
	send_to_char( "You might want to make a few rooms editable so they can expand it.\n\r", ch );
	send_to_char( "Don't forget to save....\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "planet" ) )
    {
        PLANET_DATA *planet;
        planet = get_planet(argument);
     
        if ( tarea->planet )
        {
             tarea->planet->area = NULL;
             tarea->planet->size = 0;
             tarea->planet->citysize = 0;
             tarea->planet->wilderness = 0;
             tarea->planet->farmland = 0;
             tarea->planet->barracks = 0;
             tarea->planet->controls = 0;
        }
     
        if (planet)
        {
           ROOM_INDEX_DATA *room;
           
           planet->size = 0;
           planet->citysize = 0;
           planet->wilderness = 0;
           planet->farmland = 0;
           planet->barracks = 0;
           planet->controls = 0;
           tarea->planet=planet;
           planet->area = tarea;
           for ( room = tarea->first_room ; room ; room = room->next_in_area )
           {
               planet->size++;
               if ( room->sector_type <= SECT_CITY )
                  planet->citysize++;
               else if ( room->sector_type == SECT_FARMLAND )
                  planet->farmland++;
               else if ( room->sector_type != SECT_DUNNO )
                  planet->wilderness++;
                  
               if ( IS_SET( room->room_flags, ROOM_BARRACKS ) )
                  planet->barracks++;
               if ( IS_SET( room->room_flags, ROOM_CONTROL ) )
                  planet->controls++;
           }
           save_planet(planet);
        }
	return;
    }

    if ( !str_cmp( arg2, "filename" ) )
    {
	DISPOSE( tarea->filename );
	tarea->filename = str_dup( argument );
	write_area_list( );
	fold_area( tarea, tarea->filename, TRUE );
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "flags" ) )
    {
	if ( !argument || argument[0] == '\0' )
	{
	   send_to_char( "Usage: aset <filename> flags <flag> [flag]...\n\r", ch );
	   return;
	}
	while ( argument[0] != '\0' )
	{
	   argument = one_argument( argument, arg3 );
	   value = get_areaflag( arg3 );
	   if ( value < 0 || value > 31 )
	     ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
	   else
	   {
           if ( IS_SET( tarea->flags, 1 << value ) )
	         REMOVE_BIT( tarea->flags, 1 << value );
	       else
	         SET_BIT( tarea->flags, 1 << value );
	   }
        }
        return;
    }

    do_aset( ch, "" );
    return;
}


void do_rlist( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA	*room;
    AREA_DATA		*tarea;
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Rlist which area?\n\r", ch );
	return;
    }

    for ( tarea = first_area; tarea; tarea = tarea->next )
    {
	if ( !str_cmp( tarea->name, arg ) )
	{
          for ( room = tarea->first_room; room ; room = room->next_in_area )
             pager_printf( ch, "%5d) %s\n\r", room->vnum,
					 room->name);
	  return;
	}
    }
    send_to_char( "No such area exists... type zones for a list.\n\r", ch );
    return;
    
}

void do_olist( CHAR_DATA *ch, char *argument )
{
    OBJ_INDEX_DATA	*obj;
    long			 vnum;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int lrange;
    int trange;
    int hash;
    
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    lrange = ( is_number( arg1 ) ? atoi( arg1 ) : 1 );
    trange = ( is_number( arg2 ) ? atoi( arg2 ) : 1 );

    if ( arg1[0] == '\0' )
    {
        for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
          for ( obj = obj_index_hash[hash]; obj ; obj = obj->next )
             pager_printf( ch, "%5d) %-20s '%s'\n\r", obj->vnum,
					 obj->name,
					 obj->short_descr );
        return;
    }

    for ( vnum = lrange; vnum <= trange; vnum++ )
    {
	if ( (obj = get_obj_index( vnum )) == NULL )
	  continue;
	pager_printf( ch, "%5d) %-20s (%s)\n\r", vnum,
					     obj->name,
					     obj->short_descr );
    }
    return;
}

void do_mlist( CHAR_DATA *ch, char *argument )
{
    MOB_INDEX_DATA	*mob;
    long			 vnum;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int lrange;
    int trange;
    int hash;
    
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    
    if ( arg1[0] == '\0' )
    {
        for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
          for ( mob = mob_index_hash[hash]; mob ; mob = mob->next )
             pager_printf( ch, "%5d) %-20s '%s'\n\r", mob->vnum,
					 mob->player_name,
					 mob->short_descr );
        return;
    }
    
    lrange = ( is_number( arg1 ) ? atoi( arg1 ) : 1 );
    trange = ( is_number( arg2 ) ? atoi( arg2 ) : 1 );
    
    for ( vnum = lrange; vnum <= trange; vnum++ )
    {
	  if ( (mob = get_mob_index( vnum )) == NULL )
	    continue;
	  pager_printf( ch, "%5d) %-20s '%s'\n\r", vnum,
					 mob->player_name,
					 mob->short_descr );
    }
}

void mpedit( CHAR_DATA *ch, MPROG_DATA *mprg, int mptype, char *argument )
{
	if ( mptype != -1 )
	{
	  mprg->type = 1 << mptype;
	  if ( mprg->arglist )
	    STRFREE( mprg->arglist );
	  mprg->arglist = STRALLOC( argument );
	}
	ch->substate = SUB_MPROG_EDIT;
	ch->dest_buf = mprg;
	if ( !mprg->comlist )
	   mprg->comlist = STRALLOC( "" );
	start_editing( ch, mprg->comlist );
	return;
}

/*
 * Mobprogram editing - cumbersome				-Thoric
 */
void do_mpedit( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    char arg4 [MAX_INPUT_LENGTH];
    CHAR_DATA  *victim;
    MPROG_DATA *mprog, *mprg, *mprg_next;
    int value, mptype, cnt;

    if ( IS_NPC( ch ) )
    {
	send_to_char( "Mob's can't mpedit\n\r", ch );
	return;    
    }

    if ( !ch->desc )
    {
	send_to_char( "You have no descriptor\n\r", ch );
	return;
    }

    switch( ch->substate )
    {
	default:
	  break;
	case SUB_MPROG_EDIT:
	  if ( !ch->dest_buf )
	  {
		send_to_char( "Fatal error: report to Thoric.\n\r", ch );
		bug( "do_mpedit: sub_mprog_edit: NULL ch->dest_buf", 0 );
		ch->substate = SUB_NONE;
		return;
	  }
	  mprog	 = ch->dest_buf;
	  if ( mprog->comlist )
	    STRFREE( mprog->comlist );
	  mprog->comlist = copy_buffer( ch );
	  stop_editing( ch );
	  return;
    }

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    value = atoi( arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Syntax: mpedit <victim> <command> [number] <program> <value>\n\r", ch );
	send_to_char( "\n\r",						ch );
	send_to_char( "Command being one of:\n\r",			ch );
	send_to_char( "  add delete insert edit list\n\r",		ch );
	send_to_char( "Program being one of:\n\r",			ch );
	send_to_char( "  act speech rand fight hitprcnt greet allgreet\n\r", ch );
	send_to_char( "  entry give bribe death time hour script\n\r",	ch );
	return;
    }

    if ( !IS_IMMORTAL( ch ) )
    {
      if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
      {
	send_to_char( "They aren't here.\n\r", ch );
	return;
      }
    }
    else
    {
      if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
      {
	send_to_char( "No one like that in all the realms.\n\r", ch );
	return;
      }
    }

    if ( get_trust( ch ) < get_trust( victim ) || !IS_NPC(victim) )
    {
	send_to_char( "You can't do that!\n\r", ch );
	return;
    }

    if ( !can_mmodify( ch, victim ) )
	return;

    if ( !IS_SET( victim->act, ACT_PROTOTYPE ) )
    {
	send_to_char( "A mobile must have a prototype flag to be mpset.\n\r", ch );
	return;
    }

    mprog = victim->pIndexData->mudprogs;

    set_char_color( AT_GREEN, ch );

    if ( !str_cmp( arg2, "list" ) )
    {
	cnt = 0;
	if ( !mprog )
	{
	   send_to_char( "That mobile has no mob programs.\n\r", ch );
	   return;
	}
	for ( mprg = mprog; mprg; mprg = mprg->next )
	  ch_printf( ch, "%d>%s %s\n\r%s\n\r",
	      ++cnt,
	      mprog_type_to_name( mprg->type ),
	      mprg->arglist,
	      mprg->comlist );
	return;
    }

    if ( !str_cmp( arg2, "edit" ) )
    {
	if ( !mprog )
	{
	   send_to_char( "That mobile has no mob programs.\n\r", ch );
	   return;
	}
	argument = one_argument( argument, arg4 );
	if ( arg4[0] != '\0' )
	{
	  mptype = get_mpflag( arg4 );
	  if ( mptype == -1 )
	  {
	    send_to_char( "Unknown program type.\n\r", ch );
	    return;
	  }
	}
	else
	  mptype = -1;
	if ( value < 1 )
	{
	   send_to_char( "Program not found.\n\r", ch );
	   return;
	}
	cnt = 0;
	for ( mprg = mprog; mprg; mprg = mprg->next )
	{
	   if ( ++cnt == value )
	   {
		mpedit( ch, mprg, mptype, argument );
		victim->pIndexData->progtypes = 0;
		for ( mprg = mprog; mprg; mprg = mprg->next )
		   victim->pIndexData->progtypes |= mprg->type;
		return;
	   }
	}
	send_to_char( "Program not found.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "delete" ) )
    {
	int num;
	bool found;

	if ( !mprog )
	{
	   send_to_char( "That mobile has no mob programs.\n\r", ch );
	   return;
	}
	argument = one_argument( argument, arg4 );
	if ( value < 1 )
	{
	   send_to_char( "Program not found.\n\r", ch );
	   return;
	}
	cnt = 0; found = FALSE;
	for ( mprg = mprog; mprg; mprg = mprg->next )
	{
	   if ( ++cnt == value )
	   {
		mptype = mprg->type;
		found = TRUE;
		break;
	   }
	}
	if ( !found )
	{
	   send_to_char( "Program not found.\n\r", ch );
	   return;
	}
	cnt = num = 0;
	for ( mprg = mprog; mprg; mprg = mprg->next )
	   if ( IS_SET( mprg->type, mptype ) )
	     num++;
	if ( value == 1 )
	{
	   mprg_next = victim->pIndexData->mudprogs;
	   victim->pIndexData->mudprogs = mprg_next->next;
	}
	else
	for ( mprg = mprog; mprg; mprg = mprg_next )
	{
	   mprg_next = mprg->next;
	   if ( ++cnt == (value - 1) )
	   {
		mprg->next = mprg_next->next;
		break;
	   }
	}
	STRFREE( mprg_next->arglist );
	STRFREE( mprg_next->comlist );
	DISPOSE( mprg_next );
	if ( num <= 1 )
	  REMOVE_BIT( victim->pIndexData->progtypes, mptype );
	send_to_char( "Program removed.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "insert" ) )
    {
	if ( !mprog )
	{
	   send_to_char( "That mobile has no mob programs.\n\r", ch );
	   return;
	}
	argument = one_argument( argument, arg4 );
	mptype = get_mpflag( arg4 );
	if ( mptype == -1 )
	{
	   send_to_char( "Unknown program type.\n\r", ch );
	   return;
	}
	if ( value < 1 )
	{
	   send_to_char( "Program not found.\n\r", ch );
	   return;
	}
	if ( value == 1 )
	{
	   CREATE( mprg, MPROG_DATA, 1 );
	   victim->pIndexData->progtypes |= ( 1 << mptype );
	   mpedit( ch, mprg, mptype, argument );
	   mprg->next = mprog;
	   victim->pIndexData->mudprogs = mprg;
	   return;
	}
	cnt = 1;
	for ( mprg = mprog; mprg; mprg = mprg->next )
	{
	   if ( ++cnt == value && mprg->next )
	   {
		CREATE( mprg_next, MPROG_DATA, 1 );
		victim->pIndexData->progtypes |= ( 1 << mptype );
		mpedit( ch, mprg_next, mptype, argument );
		mprg_next->next = mprg->next;
		mprg->next	= mprg_next;
		return;
	   }
	}
	send_to_char( "Program not found.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "add" ) )
    {
	mptype = get_mpflag( arg3 );
	if ( mptype == -1 )
	{
	   send_to_char( "Unknown program type.\n\r", ch );
	   return;
	}
	if ( mprog != NULL )
	  for ( ; mprog->next; mprog = mprog->next );
	CREATE( mprg, MPROG_DATA, 1 );
	if ( mprog )
	  mprog->next			= mprg;
	else
	  victim->pIndexData->mudprogs	= mprg;
	victim->pIndexData->progtypes	|= ( 1 << mptype );
	mpedit( ch, mprg, mptype, argument );
	mprg->next = NULL;
	return;
    }

    do_mpedit( ch, "" );
}

void do_opedit( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    char arg4 [MAX_INPUT_LENGTH];
    OBJ_DATA   *obj;
    MPROG_DATA *mprog, *mprg, *mprg_next;
    int value, mptype, cnt;

    if ( IS_NPC( ch ) )
    {
	send_to_char( "Mob's can't opedit\n\r", ch );
	return;    
    }

    if ( !ch->desc )
    {
	send_to_char( "You have no descriptor\n\r", ch );
	return;
    }

    switch( ch->substate )
    {
	default:
	  break;
	case SUB_MPROG_EDIT:
	  if ( !ch->dest_buf )
	  {
		send_to_char( "Fatal error: report to Thoric.\n\r", ch );
		bug( "do_opedit: sub_oprog_edit: NULL ch->dest_buf", 0 );
		ch->substate = SUB_NONE;
		return;
	  }
	  mprog	 = ch->dest_buf;
	  if ( mprog->comlist )
	    STRFREE( mprog->comlist );
	  mprog->comlist = copy_buffer( ch );
	  stop_editing( ch );
	  return;
    }

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    value = atoi( arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Syntax: opedit <object> <command> [number] <program> <value>\n\r", ch );
	send_to_char( "\n\r",						ch );
	send_to_char( "Command being one of:\n\r",			ch );
	send_to_char( "  add delete insert edit list\n\r",		ch );
	send_to_char( "Program being one of:\n\r",			ch );
	send_to_char( "  act speech rand wear remove sac zap get\n\r",  ch );
	send_to_char( "  drop damage repair greet exa use\n\r",ch );
	send_to_char( "  pull push (for levers,pullchains,buttons)\n\r",ch );
	send_to_char( "\n\r", ch);
	send_to_char( "Object should be in your inventory to edit.\n\r",ch);
	return;
    }

    if ( !IS_IMMORTAL( ch ) )
    {
      if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
      {
	send_to_char( "You aren't carrying that.\n\r", ch );
	return;
      }
    }
    else
    {
      if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
      {
	send_to_char( "Nothing like that in all the realms.\n\r", ch );
	return;
      }
    }

    if ( !can_omodify( ch, obj ) )
	return;

    if ( !IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
    {
	send_to_char( "An object must have a prototype flag to be opset.\n\r", ch );
	return;
    }

    mprog = obj->pIndexData->mudprogs;

    set_char_color( AT_GREEN, ch );

    if ( !str_cmp( arg2, "list" ) )
    {
	cnt = 0;
	if ( !mprog )
	{
	   send_to_char( "That object has no obj programs.\n\r", ch );
	   return;
	}
	for ( mprg = mprog; mprg; mprg = mprg->next )
	  ch_printf( ch, "%d>%s %s\n\r%s\n\r",
	      ++cnt,
	      mprog_type_to_name( mprg->type ),
	      mprg->arglist,
	      mprg->comlist );
	return;
    }

    if ( !str_cmp( arg2, "edit" ) )
    {
	if ( !mprog )
	{
	   send_to_char( "That object has no obj programs.\n\r", ch );
	   return;
	}
	argument = one_argument( argument, arg4 );
	if ( arg4[0] != '\0' )
	{
	  mptype = get_mpflag( arg4 );    
	  if ( mptype == -1 )
	  {
	    send_to_char( "Unknown program type.\n\r", ch );
	    return;
	  }
	}
	else
	  mptype = -1;
	if ( value < 1 )
	{
	   send_to_char( "Program not found.\n\r", ch );
	   return;
	}
	cnt = 0;
	for ( mprg = mprog; mprg; mprg = mprg->next )
	{
	   if ( ++cnt == value )
	   {
		mpedit( ch, mprg, mptype, argument );
		obj->pIndexData->progtypes = 0;
		for ( mprg = mprog; mprg; mprg = mprg->next )
		   obj->pIndexData->progtypes |= mprg->type;
		return;
	   }
	}
	send_to_char( "Program not found.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "delete" ) )
    {
	int num;
	bool found;

	if ( !mprog )
	{
	   send_to_char( "That object has no obj programs.\n\r", ch );
	   return;
	}
	argument = one_argument( argument, arg4 );
	if ( value < 1 )
	{
	   send_to_char( "Program not found.\n\r", ch );
	   return;
	}
	cnt = 0; found = FALSE;
	for ( mprg = mprog; mprg; mprg = mprg->next )
	{
	   if ( ++cnt == value )
	   {
		mptype = mprg->type;
		found = TRUE;
		break;
	   }
	}
	if ( !found )
	{
	   send_to_char( "Program not found.\n\r", ch );
	   return;
	}
	cnt = num = 0;
	for ( mprg = mprog; mprg; mprg = mprg->next )
	   if ( IS_SET( mprg->type, mptype ) )
	     num++;
	if ( value == 1 )
	{
	   mprg_next = obj->pIndexData->mudprogs;
	   obj->pIndexData->mudprogs = mprg_next->next;
	}
	else
	for ( mprg = mprog; mprg; mprg = mprg_next )
	{
	   mprg_next = mprg->next;
	   if ( ++cnt == (value - 1) )
	   {
		mprg->next = mprg_next->next;
		break;
	   }
	}
	STRFREE( mprg_next->arglist );
	STRFREE( mprg_next->comlist );
	DISPOSE( mprg_next );
	if ( num <= 1 )
	  REMOVE_BIT( obj->pIndexData->progtypes, mptype );
	send_to_char( "Program removed.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "insert" ) )
    {
	if ( !mprog )
	{
	   send_to_char( "That object has no obj programs.\n\r", ch );
	   return;
	}
	argument = one_argument( argument, arg4 );
	mptype = get_mpflag( arg4 );
	if ( mptype == -1 )
	{
	    send_to_char( "Unknown program type.\n\r", ch );
	    return;
	}
	if ( value < 1 )
	{
	   send_to_char( "Program not found.\n\r", ch );
	   return;
	}
	if ( value == 1 )
	{
	   CREATE( mprg, MPROG_DATA, 1 );
	   obj->pIndexData->progtypes	|= ( 1 << mptype );
	   mpedit( ch, mprg, mptype, argument );
	   mprg->next = mprog;
	   obj->pIndexData->mudprogs = mprg;
	   return;
	}
	cnt = 1;
	for ( mprg = mprog; mprg; mprg = mprg->next )
	{
	   if ( ++cnt == value && mprg->next )
	   {
		CREATE( mprg_next, MPROG_DATA, 1 );
		obj->pIndexData->progtypes |= ( 1 << mptype );
		mpedit( ch, mprg_next, mptype, argument );
		mprg_next->next = mprg->next;
		mprg->next	= mprg_next;
		return;
	   }
	}
	send_to_char( "Program not found.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "add" ) )
    {
	mptype = get_mpflag( arg3 );
	if ( mptype == -1 )
	{
	   send_to_char( "Unknown program type.\n\r", ch );
	   return;
	}
	if ( mprog != NULL )
	for ( ; mprog->next; mprog = mprog->next );
	CREATE( mprg, MPROG_DATA, 1 );
	if ( mprog )
	  mprog->next			 = mprg;
	else
	  obj->pIndexData->mudprogs	 = mprg;
	obj->pIndexData->progtypes	|= ( 1 << mptype );
	mpedit( ch, mprg, mptype, argument );
	mprg->next = NULL;
	return;
    }

    do_opedit( ch, "" );
}



/*
 * RoomProg Support
 */
void rpedit( CHAR_DATA *ch, MPROG_DATA *mprg, int mptype, char *argument )
{
	if ( mptype != -1 )
	{
	  mprg->type = 1 << mptype;
	  if ( mprg->arglist )
	    STRFREE( mprg->arglist );
	  mprg->arglist = STRALLOC( argument );
	}
	ch->substate = SUB_MPROG_EDIT;
	ch->dest_buf = mprg;
	if(!mprg->comlist)
          mprg->comlist = STRALLOC("");
	start_editing( ch, mprg->comlist );
	return;
}

void do_rpedit( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    MPROG_DATA *mprog, *mprg, *mprg_next;
    int value, mptype, cnt;

    if ( IS_NPC( ch ) )
    {
	send_to_char( "Mob's can't rpedit\n\r", ch );
	return;    
    }

    if ( !ch->desc )
    {
	send_to_char( "You have no descriptor\n\r", ch );
	return;
    }

    switch( ch->substate )
    {
	default:
	  break;
	case SUB_MPROG_EDIT:
	  if ( !ch->dest_buf )
	  {
		send_to_char( "Fatal error: report to Thoric.\n\r", ch );
		bug( "do_opedit: sub_oprog_edit: NULL ch->dest_buf", 0 );
		ch->substate = SUB_NONE;
		return;
	  }
	  mprog	 = ch->dest_buf;
	  if ( mprog->comlist )
	    STRFREE( mprog->comlist );
	  mprog->comlist = copy_buffer( ch );
	  stop_editing( ch );
	  return;
    }

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    value = atoi( arg2 );
    /* argument = one_argument( argument, arg3 ); */

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Syntax: rpedit <command> [number] <program> <value>\n\r", ch );
	send_to_char( "\n\r",						ch );
	send_to_char( "Command being one of:\n\r",			ch );
	send_to_char( "  add delete insert edit list\n\r",		ch );
	send_to_char( "Program being one of:\n\r",			ch );
	send_to_char( "  act speech rand sleep rest rfight enter\n\r",  ch );
	send_to_char( "  leave death\n\r",                              ch );
	send_to_char( "\n\r",						ch );
	send_to_char( "You should be standing in room you wish to edit.\n\r",ch);
	return;
    }

    if ( !can_rmodify( ch, ch->in_room ) )
	return;

    mprog = ch->in_room->mudprogs;

    set_char_color( AT_GREEN, ch );

    if ( !str_cmp( arg1, "list" ) )
    {
	cnt = 0;
	if ( !mprog )
	{
	   send_to_char( "This room has no room programs.\n\r", ch );
	   return;
	}
	for ( mprg = mprog; mprg; mprg = mprg->next )
	  ch_printf( ch, "%d>%s %s\n\r%s\n\r",
	      ++cnt,
	      mprog_type_to_name( mprg->type ),
	      mprg->arglist,
	      mprg->comlist );
	return;
    }

    if ( !str_cmp( arg1, "edit" ) )
    {
	if ( !mprog )
	{
	   send_to_char( "This room has no room programs.\n\r", ch );
	   return;
	}
	argument = one_argument( argument, arg3 );
	if ( arg3[0] != '\0' )
	{
	  mptype = get_mpflag( arg3 );    
	  if ( mptype == -1 )
	  {
	    send_to_char( "Unknown program type.\n\r", ch );
	    return;
	  }
	}
	else
	  mptype = -1;
	if ( value < 1 )
	{
	   send_to_char( "Program not found.\n\r", ch );
	   return;
	}
	cnt = 0;
	for ( mprg = mprog; mprg; mprg = mprg->next )
	{
	   if ( ++cnt == value )
	   {
		mpedit( ch, mprg, mptype, argument );
		ch->in_room->progtypes = 0;
		for ( mprg = mprog; mprg; mprg = mprg->next )
		   ch->in_room->progtypes |= mprg->type;
		return;
	   }
	}
	send_to_char( "Program not found.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg1, "delete" ) )
    {
	int num;
	bool found;

	if ( !mprog )
	{
	   send_to_char( "That room has no room programs.\n\r", ch );
	   return;
	}
	argument = one_argument( argument, arg3 );
	if ( value < 1 )
	{
	   send_to_char( "Program not found.\n\r", ch );
	   return;
	}
	cnt = 0; found = FALSE;
	for ( mprg = mprog; mprg; mprg = mprg->next )
	{
	   if ( ++cnt == value )
	   {
		mptype = mprg->type;
		found = TRUE;
		break;
	   }
	}
	if ( !found )
	{
	   send_to_char( "Program not found.\n\r", ch );
	   return;
	}
	cnt = num = 0;
	for ( mprg = mprog; mprg; mprg = mprg->next )
	   if ( IS_SET( mprg->type, mptype ) )
	     num++;
	if ( value == 1 )
	{
	   mprg_next = ch->in_room->mudprogs;
	   ch->in_room->mudprogs = mprg_next->next;
	}
	else
	for ( mprg = mprog; mprg; mprg = mprg_next )
	{
	   mprg_next = mprg->next;
	   if ( ++cnt == (value - 1) )
	   {
		mprg->next = mprg_next->next;
		break;
	   }
	}
	STRFREE( mprg_next->arglist );
	STRFREE( mprg_next->comlist );
	DISPOSE( mprg_next );
	if ( num <= 1 )
	  REMOVE_BIT( ch->in_room->progtypes, mptype );
	send_to_char( "Program removed.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "insert" ) )
    {
	if ( !mprog )
	{
	   send_to_char( "That room has no room programs.\n\r", ch );
	   return;
	}
	argument = one_argument( argument, arg3 );
	mptype = get_mpflag( arg2 );
	if ( mptype == -1 )
	{
	   send_to_char( "Unknown program type.\n\r", ch );
	   return;
	}
	if ( value < 1 )
	{
	   send_to_char( "Program not found.\n\r", ch );
	   return;
	}
	if ( value == 1 )
	{
	   CREATE( mprg, MPROG_DATA, 1 );
	   ch->in_room->progtypes |= ( 1 << mptype );
	   mpedit( ch, mprg, mptype, argument );
	   mprg->next = mprog;
	   ch->in_room->mudprogs = mprg;
	   return;
	}
	cnt = 1;
	for ( mprg = mprog; mprg; mprg = mprg->next )
	{
	   if ( ++cnt == value && mprg->next )
	   {
		CREATE( mprg_next, MPROG_DATA, 1 );
		ch->in_room->progtypes |= ( 1 << mptype );
		mpedit( ch, mprg_next, mptype, argument );
		mprg_next->next = mprg->next;
		mprg->next	= mprg_next;
		return;
	   }
	}
	send_to_char( "Program not found.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg1, "add" ) )
    {
	mptype = get_mpflag( arg2 );
	if ( mptype == -1 )
	{
	   send_to_char( "Unknown program type.\n\r", ch );
	   return;
	}
	if ( mprog )
	  for ( ; mprog->next; mprog = mprog->next );
	CREATE( mprg, MPROG_DATA, 1 );
	if ( mprog )
	  mprog->next		= mprg;
	else
	  ch->in_room->mudprogs	= mprg;
	ch->in_room->progtypes |= ( 1 << mptype );
	mpedit( ch, mprg, mptype, argument );
	mprg->next = NULL;
	return;
    }

    do_rpedit( ch, "" );
}

void do_allsave( CHAR_DATA *ch , char * argument )
{
     save_mobs();
     save_objects();
     write_area_list();
}

void save_mobs()
{
    MOB_INDEX_DATA	*pMobIndex;
    MPROG_DATA		*mprog;
    SHOP_DATA		*pShop;
    REPAIR_DATA		*pRepair;
    char		 buf[MAX_STRING_LENGTH];
    FILE		*fpout;
    long			 vnum;
    bool		 complexmob;
    int hash;
    
    sprintf( buf, "Saving Mobiles...." );
    log_string_plus( buf, LOG_NORMAL );

    rename( "mobiles" , "mobiles.bak" );
    fclose( fpReserve );
    if ( ( fpout = fopen( "mobiles", "w" ) ) == NULL )
    {
	bug( "save_mobiles: fopen", 0 );
	perror( "mobiles" );
	fpReserve = fopen( NULL_FILE, "r" );
	return;
    }
    
    fprintf( fpout, "#MOBILES\n" );
    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
     for ( pMobIndex = mob_index_hash[hash];
	      pMobIndex;
	      pMobIndex = pMobIndex->next )
    {
	vnum = pMobIndex->vnum;
	if ( pMobIndex->perm_str != 13	||   pMobIndex->perm_int   != 13
	||   pMobIndex->perm_wis != 13	||   pMobIndex->perm_dex   != 13
	||   pMobIndex->perm_con != 13	||   pMobIndex->perm_cha   != 13
	||   pMobIndex->perm_lck != 13
	||   pMobIndex->hitroll  != 0	||   pMobIndex->damroll    != 0
	||   pMobIndex->attacks	 != 0	||   pMobIndex->defenses   != 0
	||   pMobIndex->height	 != 0	||   pMobIndex->weight	   != 0
	||   pMobIndex->xflags	 != 0   ||   pMobIndex->numattacks != 0 )
	  complexmob = TRUE;
	else
	  complexmob = FALSE;
	fprintf( fpout, "#%ld\n",	vnum				);
	fprintf( fpout, "%s~\n",	pMobIndex->player_name		);
	fprintf( fpout,	"%s~\n",	pMobIndex->short_descr		);
	fprintf( fpout,	"%s~\n",	strip_cr(pMobIndex->long_descr)	);
	fprintf( fpout, "%s~\n",	strip_cr(pMobIndex->description));
	fprintf( fpout, "%d %d %d %c\n",pMobIndex->act,
					pMobIndex->affected_by,
					pMobIndex->alignment,
					complexmob ? 'Z' : 'S'		);
					
	fprintf( fpout, "%d %d %d ",	pMobIndex->level,
					pMobIndex->mobthac0,
					pMobIndex->ac			);
	fprintf( fpout, "%dd%d+%d ",	pMobIndex->hitnodice,
					pMobIndex->hitsizedice,
					pMobIndex->hitplus		);
	fprintf( fpout, "%dd%d+%d\n",	pMobIndex->damnodice,
					pMobIndex->damsizedice,
					pMobIndex->damplus		);
	fprintf( fpout, "%d 0\n",	pMobIndex->gold			);
	fprintf( fpout, "%d %d %d\n",	pMobIndex->position,
					pMobIndex->defposition,
					pMobIndex->sex			);
	if ( complexmob )
	{
	  fprintf( fpout, "%d %d %d %d %d %d %d\n",
					pMobIndex->perm_str,
					pMobIndex->perm_int,
					pMobIndex->perm_wis,
					pMobIndex->perm_dex,
					pMobIndex->perm_con,
					pMobIndex->perm_cha,
					pMobIndex->perm_lck );
	  fprintf( fpout, "%d %d %d %d %d\n",
					pMobIndex->saving_poison_death,
					pMobIndex->saving_wand,
					pMobIndex->saving_para_petri,
					pMobIndex->saving_breath,
					pMobIndex->saving_spell_staff );
	  fprintf( fpout, "0 0 %d %d 0 0 %d\n",
					pMobIndex->height,
					pMobIndex->weight,
					pMobIndex->numattacks );
	  fprintf( fpout, "%d %d %d %d %d %d %d %d\n",
					pMobIndex->hitroll,
					pMobIndex->damroll,
					pMobIndex->xflags,
					pMobIndex->resistant,
					pMobIndex->immune,
					pMobIndex->susceptible,
					pMobIndex->attacks,
					pMobIndex->defenses );
	fprintf( fpout, "0 0 0 0 0 0 0 0\n" );
	}
	if ( pMobIndex->mudprogs )
	{
	  for ( mprog = pMobIndex->mudprogs; mprog; mprog = mprog->next )
		fprintf( fpout, "> %s %s~\n%s~\n",
				mprog_type_to_name( mprog->type ),
				mprog->arglist, strip_cr(mprog->comlist) );
	  fprintf( fpout, "|\n" );	  
	}
    }
    fprintf( fpout, "#0\n\n\n" );

    fprintf( fpout, "#SHOPS\n" );
    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
     for ( pMobIndex = mob_index_hash[hash];
	      pMobIndex;
	      pMobIndex = pMobIndex->next )
    {
	vnum = pMobIndex->vnum;
	if ( (pShop = pMobIndex->pShop) == NULL )
	  continue;
	fprintf( fpout, " %d   %2d %2d %2d %2d %2d   %3d %3d",
		 pShop->keeper,
		 pShop->buy_type[0],
		 pShop->buy_type[1],
		 pShop->buy_type[2],
		 pShop->buy_type[3],
		 pShop->buy_type[4],
		 pShop->profit_buy,
		 pShop->profit_sell );
	fprintf( fpout, "        %2d %2d    ; %s\n",
		 pShop->open_hour,
		 pShop->close_hour,
		 pMobIndex->short_descr );
    }
    fprintf( fpout, "0\n\n\n" );

    /* save repair shops */

    fprintf( fpout, "#REPAIRS\n" );
    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
     for ( pMobIndex = mob_index_hash[hash];
	      pMobIndex;
	      pMobIndex = pMobIndex->next )
    {
	vnum = pMobIndex->vnum;
	if ( (pRepair = pMobIndex->rShop) == NULL )
	  continue;
	fprintf( fpout, " %d   %2d %2d %2d         %3d %3d",
		 pRepair->keeper,
		 pRepair->fix_type[0],
		 pRepair->fix_type[1],
		 pRepair->fix_type[2],
		 pRepair->profit_fix,
		 pRepair->shop_type );
	fprintf( fpout, "        %2d %2d    ; %s\n",
		 pRepair->open_hour,
		 pRepair->close_hour,
		 pMobIndex->short_descr );
    }
    fprintf( fpout, "0\n\n\n" );

    /* save specials */

    fprintf( fpout, "#SPECIALS\n" );
    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
     for ( pMobIndex = mob_index_hash[hash];
	      pMobIndex;
	      pMobIndex = pMobIndex->next )
    {
	vnum = pMobIndex->vnum;
	if ( pMobIndex->spec_fun )
	   fprintf( fpout, "M  %ld %s\n",	pMobIndex->vnum,
					lookup_spec( pMobIndex->spec_fun ) );
        if ( pMobIndex->spec_2 )
	   fprintf( fpout, "M  %ld %s\n",	pMobIndex->vnum,
					lookup_spec( pMobIndex->spec_2 ) );
    }
    fprintf( fpout, "S\n\n\n" );

    /* END */
    fprintf( fpout, "#$\n" );
    fclose( fpout );
    fpReserve = fopen( NULL_FILE, "r" );
    return;

}

void save_objects()
{
    OBJ_INDEX_DATA	*pObjIndex;
    EXTRA_DESCR_DATA	*ed;
    char		 buf[MAX_STRING_LENGTH];
    FILE		*fpout;
    long			 vnum;
    int 		hash, val0, val1, val2, val3, val4, val5;
    AFFECT_DATA         *paf;
    MPROG_DATA		*mprog;
    
    sprintf( buf, "Saving Objects..." );
    log_string_plus( buf, LOG_NORMAL );

    rename( "objects", "objects.bak" );
    fclose( fpReserve );
    if ( ( fpout = fopen( "objects", "w" ) ) == NULL )
    {
	bug( "fold_area: fopen", 0 );
	perror( "objects" );
	fpReserve = fopen( NULL_FILE, "r" );
	return;
    }
    

    fprintf( fpout, "#OBJECTS\n" );
    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
     for ( pObjIndex = obj_index_hash[hash];
	      pObjIndex;
	      pObjIndex = pObjIndex->next )
    {
	vnum = pObjIndex->vnum;
	fprintf( fpout, "#%ld\n",	vnum				);
	fprintf( fpout, "%s~\n",	pObjIndex->name			);
	fprintf( fpout, "%s~\n",	pObjIndex->short_descr		);
	fprintf( fpout, "%s~\n",	pObjIndex->description		);
	fprintf( fpout, "%s~\n",	pObjIndex->action_desc		);
	if ( pObjIndex->layers )
	  fprintf( fpout, "%d %d %d %d\n",	pObjIndex->item_type,
						pObjIndex->extra_flags,
						pObjIndex->wear_flags,
						pObjIndex->layers	);
	else
	  fprintf( fpout, "%d %d %d\n",	pObjIndex->item_type,
					pObjIndex->extra_flags,
					pObjIndex->wear_flags		);

	val0 = pObjIndex->value[0];
	val1 = pObjIndex->value[1];
	val2 = pObjIndex->value[2];
	val3 = pObjIndex->value[3];
	val4 = pObjIndex->value[4];
	val5 = pObjIndex->value[5];
	switch ( pObjIndex->item_type )
	{
	case ITEM_DEVICE:
	    if ( IS_VALID_SN(val3) ) val3 = skill_table[val3]->slot;
	    break;
	}
	if ( val4 || val5 )
	  fprintf( fpout, "%d %d %d %d %d %d\n",val0, 
						val1,
						val2,
						val3,
						val4,
						val5 );
	else
	  fprintf( fpout, "%d %d %d %d\n",	val0, 
						val1,
						val2,
						val3 );

	fprintf( fpout, "%d %d 0\n",	pObjIndex->weight,
					pObjIndex->cost );

	for ( ed = pObjIndex->first_extradesc; ed; ed = ed->next )
	   fprintf( fpout, "E\n%s~\n%s~\n",
			ed->keyword, strip_cr( ed->description )	);

	for ( paf = pObjIndex->first_affect; paf; paf = paf->next )
	   fprintf( fpout, "A\n%d %d\n", paf->location,
	     ((paf->location == APPLY_WEAPONSPELL
	    || paf->location == APPLY_WEARSPELL
	    || paf->location == APPLY_REMOVESPELL
	    || paf->location == APPLY_STRIPSN)
	    && IS_VALID_SN(paf->modifier))
	    ? skill_table[paf->modifier]->slot : paf->modifier		);

	if ( pObjIndex->mudprogs )
	{
	  for ( mprog = pObjIndex->mudprogs; mprog; mprog = mprog->next )
		fprintf( fpout, "> %s %s~\n%s~\n",
				mprog_type_to_name( mprog->type ),
				mprog->arglist, strip_cr(mprog->comlist) );
	  fprintf( fpout, "|\n" );	  
	}
    }
    fprintf( fpout, "#0\n\n\n" );

    /* END */
    fprintf( fpout, "#$\n" );
    fclose( fpout );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}

void save_some_areas( )
{
     AREA_DATA * area;
     
     for ( area = first_area ; area ; area = area->next )
     {
         if ( IS_SET( area->flags , AFLAG_MODIFIED ) )
         {
              REMOVE_BIT( area->flags , AFLAG_MODIFIED );
              fold_area( area , area->filename, FALSE );
         }
     }     

}