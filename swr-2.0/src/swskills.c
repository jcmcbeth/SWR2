#include <math.h> 
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

void    add_reinforcements  args( ( CHAR_DATA *ch ) );
ch_ret  one_hit             args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
int     xp_compute                ( CHAR_DATA *ch , CHAR_DATA *victim );
int ris_save( CHAR_DATA *ch, int chance, int ris );
CHAR_DATA *get_char_room_mp( CHAR_DATA *ch, char *argument );
void  clear_roomtype( ROOM_INDEX_DATA * room );

extern int      top_affect;
extern int top_r_vnum;

const	char *	sector_name	[SECT_MAX]	=
{
    "inside", "city", "field", "forest", "hills", "mountain", "water swim", "water noswim", 
    "underwater", "air", "desert", "unknown", "ocean floor", "underground",
    "scrub", "rocky", "savanna", "tundra", "glacial", "rainforest", "jungle", 
    "swamp", "wetlands", "brush", "steppe", "farmland", "volcanic"
};

void do_makeblade( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int level, chance, charge;
    bool checktool, checkdura, checkbatt, checkoven; 
    OBJ_DATA *obj;
    OBJ_INDEX_DATA *pObjIndex;
    int vnum;
    AFFECT_DATA *paf;
    AFFECT_DATA *paf2;
            
    strcpy( arg , argument );
    
    switch( ch->substate )
    { 
    	default:
    	        
    	        if ( arg[0] == '\0' )
                {
                  send_to_char( "&RUsage: Makeblade <name>\n\r&w", ch);
                  return;   
                }
 
    	        checktool = FALSE;
                checkdura = FALSE;
                checkbatt = FALSE;
                checkoven = FALSE;
        
                for ( obj = ch->last_carrying; obj; obj = obj->prev_content )     
                {
                  if (obj->item_type == ITEM_TOOLKIT)
                    checktool = TRUE;
                  if (obj->item_type == ITEM_METAL)
          	    checkdura = TRUE;
                  if (obj->item_type == ITEM_BATTERY)
                  checkbatt = TRUE;

                  if (obj->item_type == ITEM_OVEN)
                  checkoven = TRUE;                  
                }
                
                if ( !checktool )
                {
                   send_to_char( "&RYou need toolkit to make a vibro-blade.\n\r", ch);
                   return;
                }
 
                if ( !checkdura )
                {
                   send_to_char( "&RYou need something to make it out of.\n\r", ch);
                   return;
                }

                if ( !checkbatt )
                {
                   send_to_char( "&RYou need a power source for your blade.\n\r", ch);
                   return;
                }
                
                if ( !checkoven )
                {
                   send_to_char( "&RYou need a small furnace to heat the metal.\n\r", ch);
                   return;
                }
 
    	        chance = IS_NPC(ch) ? ch->top_level
	                 : (int) (ch->pcdata->learned[gsn_makeblade]);
                if ( number_percent( ) < chance )
    		{
    		   send_to_char( "&GYou begin the long process of crafting a vibroblade.\n\r", ch);
    		   act( AT_PLAIN, "$n takes $s tools and a small oven and begins to work on something.", ch,
		        NULL, argument , TO_ROOM );
		   add_timer ( ch , TIMER_DO_FUN , 10 , do_makeblade , 1 );
    		   ch->dest_buf = str_dup(arg);
    		   return;
	        }
	        send_to_char("&RYou can't figure out how to fit the parts together.\n\r",ch);
	        learn_from_failure( ch, gsn_makeblade );
    	   	return;	
    	
    	case 1: 
    		if ( !ch->dest_buf )
    		     return;
    		strcpy(arg, ch->dest_buf);
    		DISPOSE( ch->dest_buf);
    		break;
    		
    	case SUB_TIMER_DO_ABORT:
    		DISPOSE( ch->dest_buf );
    		ch->substate = SUB_NONE;    		                                   
    	        send_to_char("&RYou are interupted and fail to finish your work.\n\r", ch);
    	        return;
    }
    
    ch->substate = SUB_NONE;

    level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->learned[gsn_makeblade]);
    vnum = OBJ_VNUM_PROTO_BLADE;
    
    if ( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
    {
         send_to_char( "&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r", ch );
         return;
    }

    checktool = FALSE;
    checkdura = FALSE;
    checkbatt = FALSE;
    checkoven = FALSE;
    
    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )     
    {
       if (obj->item_type == ITEM_TOOLKIT)
          checktool = TRUE;
       if (obj->item_type == ITEM_OVEN)
          checkoven = TRUE;
       if (obj->item_type == ITEM_METAL && checkdura == FALSE)
       {
          checkdura = TRUE;
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
       }
       if (obj->item_type == ITEM_BATTERY && checkbatt == FALSE )
       {
          charge = UMAX( 5, obj->value[0] ); 
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
          checkbatt = TRUE;
       }
    }                            
    
    if ( ( !checktool ) || ( !checkdura ) || ( !checkbatt ) || ( !checkoven ) )
    {
       send_to_char( "&RYou activate your newly created vibroblade.\n\r", ch);
       send_to_char( "&RIt hums softly for a few seconds then begins to shake violently.\n\r", ch);
       send_to_char( "&RIt finally shatters breaking apart into a dozen pieces.\n\r", ch);
       learn_from_failure( ch, gsn_makeblade );
       return;
    }

    obj = create_object( pObjIndex, level );
    
    obj->item_type = ITEM_WEAPON;
    SET_BIT( obj->wear_flags, ITEM_WIELD );
    SET_BIT( obj->wear_flags, ITEM_TAKE );
    obj->level = level;
    obj->weight = 3;
    STRFREE( obj->name );
    strcpy( buf, arg );
    strcat( buf, " vibro-blade blade" );
    obj->name = STRALLOC( buf );
    strcpy( buf, arg );
    STRFREE( obj->short_descr );
    obj->short_descr = STRALLOC( buf );        
    STRFREE( obj->description );
    strcat( buf, " was left here." );
    obj->description = STRALLOC( buf );
    CREATE( paf, AFFECT_DATA, 1 );
    paf->type               = -1;
    paf->duration           = -1;
    paf->location           = get_atype( "backstab" );
    paf->modifier           = level/3;
    paf->bitvector          = 0;
    paf->next               = NULL;
    LINK( paf, obj->first_affect, obj->last_affect, next, prev );
    ++top_affect;
    CREATE( paf2, AFFECT_DATA, 1 );
    paf2->type               = -1;
    paf2->duration           = -1;
    paf2->location           = get_atype( "hitroll" );
    paf2->modifier           = -2;
    paf2->bitvector          = 0;
    paf2->next               = NULL;
    LINK( paf2, obj->first_affect, obj->last_affect, next, prev );
    ++top_affect;
    obj->value[0] = INIT_WEAPON_CONDITION;      
    obj->value[1] = (int) (level/20+10);      /* min dmg  */
    obj->value[2] = (int) (level/10+20);      /* max dmg */
    obj->value[3] = WEAPON_VIBRO_BLADE;
    obj->value[4] = charge;
    obj->value[5] = charge;
    obj->cost = obj->value[2]*10;
                                                                    
    obj = obj_to_char( obj, ch );
                                                            
    send_to_char( "&GYou finish your work and hold up your newly created blade.&w\n\r", ch);
    act( AT_PLAIN, "$n finishes crafting a vibro-blade.", ch,
         NULL, argument , TO_ROOM );
        
    learn_from_success( ch, gsn_makeblade );
    learn_from_success( ch, gsn_makeblade );
    learn_from_success( ch, gsn_makeblade );
}

void do_makeblaster( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int level, chance;
    bool checktool, checkdura, checkbatt, checkoven, checkcond, checkcirc, checkammo;
    OBJ_DATA *obj;
    OBJ_INDEX_DATA *pObjIndex;
    int vnum, power, scope, ammo;
    AFFECT_DATA *paf;
    AFFECT_DATA *paf2;
    
    strcpy( arg , argument );
    
    switch( ch->substate )
    { 
    	default:
    	        if ( arg[0] == '\0' )
                {
                  send_to_char( "&RUsage: Makeblaster <name>\n\r&w", ch);
                  return;   
                }

    	        checktool = FALSE;
                checkdura = FALSE;
                checkbatt = FALSE;
                checkoven = FALSE;
                checkcond = FALSE;
                checkcirc = FALSE;

                for ( obj = ch->last_carrying; obj; obj = obj->prev_content )     
                {
                  if (obj->item_type == ITEM_TOOLKIT)
                    checktool = TRUE;
                  if (obj->item_type == ITEM_PLASTIC)
          	    checkdura = TRUE;
                  if (obj->item_type == ITEM_BATTERY)
                    checkbatt = TRUE;
                  if (obj->item_type == ITEM_OVEN)
                    checkoven = TRUE;
                  if (obj->item_type == ITEM_CIRCUIT)
                    checkcirc = TRUE;
                  if (obj->item_type == ITEM_SUPERCONDUCTOR)
                    checkcond = TRUE;                  
                }
                
                if ( !checktool )
                {
                   send_to_char( "&RYou need toolkit to make a blaster.\n\r", ch);
                   return;
                }
 
                if ( !checkdura )
                {
                   send_to_char( "&RYou need something to make it out of.\n\r", ch);
                   return;
                }

                if ( !checkbatt )
                {
                   send_to_char( "&RYou need a power source for your blaster.\n\r", ch);
                   return;
                }
                
                if ( !checkoven )
                {
                   send_to_char( "&RYou need a small furnace to heat the plastics.\n\r", ch);
                   return;
                }
                
                if ( !checkcirc )
                {
                   send_to_char( "&RYou need a small circuit board to control the firing mechanism.\n\r", ch);
                   return;
                }
                
                if ( !checkcond )
                {
                   send_to_char( "&RYou still need a small superconductor.\n\r", ch);
                   return;
                }
 
    	        chance = IS_NPC(ch) ? ch->top_level
	                 : (int) (ch->pcdata->learned[gsn_makeblaster]);
                if ( number_percent( ) < chance )
    		{
    		   send_to_char( "&GYou begin the long process of making a blaster.\n\r", ch);
    		   act( AT_PLAIN, "$n takes $s tools and a small oven and begins to work on something.", ch,
		        NULL, argument , TO_ROOM );
		   add_timer ( ch , TIMER_DO_FUN , 10 , do_makeblaster , 1 );
    		   ch->dest_buf   = str_dup(arg);
    		   return;
	        }
	        send_to_char("&RYou can't figure out how to fit the parts together.\n\r",ch);
	        learn_from_failure( ch, gsn_makeblaster );
    	   	return;	
    	
    	case 1: 
    		if ( !ch->dest_buf )
    		     return;
    		strcpy(arg, ch->dest_buf);
    		DISPOSE( ch->dest_buf);
    		break;
    		
    	case SUB_TIMER_DO_ABORT:
    		DISPOSE( ch->dest_buf );
    		ch->substate = SUB_NONE;    		                                   
    	        send_to_char("&RYou are interupted and fail to finish your work.\n\r", ch);
    	        return;
    }
    
    ch->substate = SUB_NONE;
    
    level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->learned[gsn_makeblaster]);
    vnum = OBJ_VNUM_PROTO_BLASTER;
    
    if ( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
    {
         send_to_char( "&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r", ch );
         return;
    }

    checkammo= FALSE;
    checktool = FALSE;
    checkdura = FALSE;
    checkbatt = FALSE;
    checkoven = FALSE;
    checkcond = FALSE;
    checkcirc = FALSE;
    power     = 0;
    scope     = 0;
    ammo = 0;
    
    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )     
    {
       if (obj->item_type == ITEM_TOOLKIT)
          checktool = TRUE;
       if (obj->item_type == ITEM_OVEN)
          checkoven = TRUE;
       if (obj->item_type == ITEM_PLASTIC && checkdura == FALSE)
       {
          checkdura = TRUE;
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
       }
       if (obj->item_type == ITEM_AMMO && checkammo == FALSE)
       {
          ammo = obj->value[0];
          checkammo = TRUE;
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
       }
       if (obj->item_type == ITEM_BATTERY && checkbatt == FALSE)
       {
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
          checkbatt = TRUE;
       }
       if (obj->item_type == ITEM_LENS && scope == 0)
       {
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
          scope++;
       }
       if (obj->item_type == ITEM_SUPERCONDUCTOR && power<2)
       {
          power++;
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
          checkcond = TRUE;
       }
       if (obj->item_type == ITEM_CIRCUIT && checkcirc == FALSE)
       {
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
          checkcirc = TRUE;
       }
    }                            
    
    if ( ( !checktool ) || ( !checkdura ) || ( !checkbatt ) || ( !checkoven )  || ( !checkcond ) || ( !checkcirc) )
    {
       send_to_char( "&RYou hold up your new blaster and aim at a leftover piece of plastic.\n\r", ch);
       send_to_char( "&RYou slowly squeeze the trigger hoping for the best...\n\r", ch);
       send_to_char( "&RYour blaster backfires destroying your weapon and burning your hand.\n\r", ch);
       learn_from_failure( ch, gsn_makeblaster );
       return;
    }

    obj = create_object( pObjIndex, level );
    
    obj->item_type = ITEM_WEAPON;
    SET_BIT( obj->wear_flags, ITEM_WIELD );
    SET_BIT( obj->wear_flags, ITEM_TAKE );
    obj->level = level;
    obj->weight = 2+level/10;
    STRFREE( obj->name );
    strcpy( buf , arg );
    strcat( buf , " blaster");
    obj->name = STRALLOC( buf );
    strcpy( buf, arg );
    STRFREE( obj->short_descr );
    obj->short_descr = STRALLOC( buf );        
    STRFREE( obj->description );
    strcat( buf, " was carelessly misplaced here." );
    obj->description = STRALLOC( buf );
    CREATE( paf, AFFECT_DATA, 1 );
    paf->type               = -1;
    paf->duration           = -1;
    paf->location           = get_atype( "hitroll" );
    paf->modifier           = URANGE( 0, 1+scope, level/30 );
    paf->bitvector          = 0;
    paf->next               = NULL;
    LINK( paf, obj->first_affect, obj->last_affect, next, prev );
    ++top_affect;
    CREATE( paf2, AFFECT_DATA, 1 );
    paf2->type               = -1;
    paf2->duration           = -1;
    paf2->location           = get_atype( "damroll" );
    paf2->modifier           = URANGE( 0, power, level/30);
    paf2->bitvector          = 0;
    paf2->next               = NULL;
    LINK( paf2, obj->first_affect, obj->last_affect, next, prev );
    ++top_affect;
    obj->value[0] = INIT_WEAPON_CONDITION;       /* condition  */
    obj->value[1] = (int) (level/10+15);      /* min dmg  */
    obj->value[2] = (int) (level/5+25);      /* max dmg  */
    obj->value[3] = WEAPON_BLASTER;
    obj->value[4] = ammo;
    obj->value[5] = 2000;
    obj->cost = obj->value[2]*50;
                                                                   
    obj = obj_to_char( obj, ch );
                                                            
    send_to_char( "&GYou finish your work and hold up your newly created blaster.&w\n\r", ch);
    act( AT_PLAIN, "$n finishes making $s new blaster.", ch,
         NULL, argument , TO_ROOM );
    
    learn_from_success( ch, gsn_makeblaster );
    learn_from_success( ch, gsn_makeblaster );
    learn_from_success( ch, gsn_makeblaster );
}

void do_makelightsaber( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int chance;
    bool checktool, checkdura, checkbatt, 
         checkoven, checkcond, checkcirc, checklens, checkgems, checkmirr;
    OBJ_DATA *obj;
    OBJ_INDEX_DATA *pObjIndex;
    int vnum, level, gems, charge, gemtype;
    AFFECT_DATA *paf;
    AFFECT_DATA *paf2;
    
    strcpy( arg, argument );    
    
    switch( ch->substate )
    { 
    	default:
    	        if ( arg[0] == '\0' )
                {
                  send_to_char( "&RUsage: Makelightsaber <name>\n\r&w", ch);
                  return;   
                }

    	        checktool = FALSE;
                checkdura = FALSE;
                checkbatt = FALSE;
                checkoven = FALSE;
                checkcond = FALSE;
                checkcirc = FALSE;
                checklens = FALSE;
                checkgems = FALSE;
                checkmirr = FALSE;

                if ( !IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
                {
                   send_to_char( "&RYou need to be in a quiet peaceful place to craft a lightsaber.\n\r", ch);
                   return;
                }
                
                for ( obj = ch->last_carrying; obj; obj = obj->prev_content )     
                {
                  if (obj->item_type == ITEM_TOOLKIT)
                    checktool = TRUE;
                  if (obj->item_type == ITEM_LENS)
                    checklens = TRUE;
                  if (obj->item_type == ITEM_CRYSTAL)
                    checkgems = TRUE;                    
                  if (obj->item_type == ITEM_MIRROR)
                    checkmirr = TRUE;
                  if (obj->item_type == ITEM_PLASTIC || obj->item_type == ITEM_METAL )
          	    checkdura = TRUE;
                  if (obj->item_type == ITEM_BATTERY)
                    checkbatt = TRUE;
                  if (obj->item_type == ITEM_OVEN)
                    checkoven = TRUE;
                  if (obj->item_type == ITEM_CIRCUIT)
                    checkcirc = TRUE;
                  if (obj->item_type == ITEM_SUPERCONDUCTOR)
                    checkcond = TRUE;                  
                }
                
                if ( !checktool )
                {
                   send_to_char( "&RYou need toolkit to make a lightsaber.\n\r", ch);
                   return;
                }
 
                if ( !checkdura )
                {
                   send_to_char( "&RYou need something to make it out of.\n\r", ch);
                   return;
                }

                if ( !checkbatt )
                {
                   send_to_char( "&RYou need a power source for your lightsaber.\n\r", ch);
                   return;
                }
                
                if ( !checkoven )
                {
                   send_to_char( "&RYou need a small furnace to heat and shape the components.\n\r", ch);
                   return;
                }
                
                if ( !checkcirc )
                {
                   send_to_char( "&RYou need a small circuit board.\n\r", ch);
                   return;
                }
                
                if ( !checkcond )
                {
                   send_to_char( "&RYou still need a small superconductor for your lightsaber.\n\r", ch);
                   return;
                }
                
                if ( !checklens )
                {
                   send_to_char( "&RYou still need a lens to focus the beam.\n\r", ch);
                   return;
                }
                
                if ( !checkgems )
                {
                   send_to_char( "&RLightsabers require 1 to 3 gems to work properly.\n\r", ch);
                   return;
                }
                
                if ( !checkmirr )
                {
                   send_to_char( "&RYou need a high intesity reflective cup to create a lightsaber.\n\r", ch);
                   return;
                }
 
    	        chance = IS_NPC(ch) ? ch->top_level
	                 : (int) (ch->pcdata->learned[gsn_lightsaber_crafting]);
                if ( number_percent( ) < chance )
    		{
    		   send_to_char( "&GYou begin the long process of crafting a lightsaber.\n\r", ch);
    		   act( AT_PLAIN, "$n takes $s tools and a small oven and begins to work on something.", ch,
		        NULL, argument , TO_ROOM );
		   add_timer ( ch , TIMER_DO_FUN , 10 , do_makelightsaber , 1 );
    		   ch->dest_buf = str_dup(arg);
    		   return;
	        }
	        send_to_char("&RYou can't figure out how to fit the parts together.\n\r",ch);
	        learn_from_failure( ch, gsn_lightsaber_crafting );
    	   	return;	
    	
    	case 1: 
    		if ( !ch->dest_buf )
    		     return;
    		strcpy(arg, ch->dest_buf);
    		DISPOSE( ch->dest_buf);
    		break;
    		
    	case SUB_TIMER_DO_ABORT:
    		DISPOSE( ch->dest_buf );
    		ch->substate = SUB_NONE;    		                                   
    	        send_to_char("&RYou are interupted and fail to finish your work.\n\r", ch);
    	        return;
    }
    
    ch->substate = SUB_NONE;
    
    level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->learned[gsn_lightsaber_crafting]);
    vnum = OBJ_VNUM_PROTO_LIGHTSABER;
    
    if ( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
    {
         send_to_char( "&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r", ch );
         return;
    }

    checktool = FALSE;
    checkdura = FALSE;
    checkbatt = FALSE;
    checkoven = FALSE;
    checkcond = FALSE;
    checkcirc = FALSE;
    checklens = FALSE;
    checkgems = FALSE;
    checkmirr = FALSE;
    gems = 0;
    charge = 0;
    gemtype =0;

    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )     
    {
       if (obj->item_type == ITEM_TOOLKIT)
          checktool = TRUE;
       if (obj->item_type == ITEM_OVEN)
          checkoven = TRUE;
       if ( (obj->item_type == ITEM_PLASTIC || obj->item_type == ITEM_METAL) && checkdura == FALSE)
       {
          checkdura = TRUE;
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
       }
       if (obj->item_type == ITEM_METAL && checkdura == FALSE)
       {
          checkdura = TRUE;
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
       }
       if (obj->item_type == ITEM_BATTERY && checkbatt == FALSE)
       {
          charge = UMIN(obj->value[1], 10);
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
          checkbatt = TRUE;
       }
       if (obj->item_type == ITEM_SUPERCONDUCTOR && checkcond == FALSE)
       {
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
          checkcond = TRUE;
       }
       if (obj->item_type == ITEM_CIRCUIT && checkcirc == FALSE)
       {
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
          checkcirc = TRUE;
       }
       if (obj->item_type == ITEM_LENS && checklens == FALSE)
       {
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
          checklens = TRUE;
       }
       if (obj->item_type == ITEM_MIRROR && checkmirr == FALSE)
       {
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
          checkmirr = TRUE;
       }
       if (obj->item_type == ITEM_CRYSTAL && gems < 3)
       {
          gems++;
          if ( gemtype < obj->value[0] )
             gemtype = obj->value[0];
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
          checkgems = TRUE;
       }
    }                            
    
                
    if ( ( !checktool ) || ( !checkdura ) || ( !checkbatt ) || ( !checkoven ) 
                                       || ( !checkmirr ) || ( !checklens ) || ( !checkgems ) || ( !checkcond ) || ( !checkcirc) )
    
    {
       send_to_char( "&RYou hold up your new lightsaber and press the switch hoping for the best.\n\r", ch);
       send_to_char( "&RInstead of a blade of light, smoke starts pouring from the handle.\n\r", ch);
       send_to_char( "&RYou drop the hot handle and watch as it melts on away on the floor.\n\r", ch);
       learn_from_failure( ch, gsn_lightsaber_crafting );
       return;
    }

    obj = create_object( pObjIndex, level );
    
    obj->item_type = ITEM_WEAPON;
    SET_BIT( obj->wear_flags, ITEM_WIELD );
    SET_BIT( obj->wear_flags, ITEM_TAKE );
    obj->level = level;
    obj->weight = 5;
    STRFREE( obj->name );
    obj->name = STRALLOC( "lightsaber saber" );
    strcpy( buf, arg );
    STRFREE( obj->short_descr );
    obj->short_descr = STRALLOC( buf );        
    STRFREE( obj->description );
    strcat( buf, " was carelessly misplaced here." );
    obj->description = STRALLOC( buf );
    STRFREE( obj->action_desc );
    strcpy( buf, arg );
    strcat( buf, " ignites with a hum and a soft glow." );
    obj->action_desc = STRALLOC( buf );
    CREATE( paf, AFFECT_DATA, 1 );
    paf->type               = -1;
    paf->duration           = -1;
    paf->location           = get_atype( "hitroll" );
    paf->modifier           = URANGE( 0, gems, level/30 );
    paf->bitvector          = 0;
    paf->next               = NULL;
    LINK( paf, obj->first_affect, obj->last_affect, next, prev );
    ++top_affect;
    CREATE( paf2, AFFECT_DATA, 1 );
    paf2->type               = -1;
    paf2->duration           = -1;
    paf2->location           = get_atype( "parry" );
    paf2->modifier           = ( level/3 );
    paf2->bitvector          = 0;
    paf2->next               = NULL;
    LINK( paf2, obj->first_affect, obj->last_affect, next, prev );
    ++top_affect;
    obj->value[0] = INIT_WEAPON_CONDITION;       /* condition  */
    obj->value[1] = (int) (level/10+gemtype*2);      /* min dmg  */
    obj->value[2] = (int) (level/5+gemtype*6);      /* max dmg */
    obj->value[3] = WEAPON_LIGHTSABER;
    obj->value[4] = charge;
    obj->value[5] = charge;
    obj->cost = obj->value[2]*75;
                                                                    
    obj = obj_to_char( obj, ch );
                                                            
    send_to_char( "&GYou finish your work and hold up your newly created lightsaber.&w\n\r", ch);
    act( AT_PLAIN, "$n finishes making $s new lightsaber.", ch,
         NULL, argument , TO_ROOM );
    
        learn_from_success( ch, gsn_lightsaber_crafting );
        learn_from_success( ch, gsn_lightsaber_crafting );
        learn_from_success( ch, gsn_lightsaber_crafting );
}



void do_makejewelry( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int level, chance;
    bool checktool, checkoven, checkmetal; 
    OBJ_DATA *obj;
    OBJ_DATA *metal;
    int value, cost;
            
    argument = one_argument( argument, arg );
    strcpy ( arg2, argument);
    
    
    switch( ch->substate )
    { 
    	default:
    	        
    		if ( str_cmp( arg, "finger" )
    		&& str_cmp( arg, "wrist" )
    		&& str_cmp( arg, "neck" )
    		&& str_cmp( arg, "ears" ) )
    		{
        		send_to_char( "&RYou cannot make jewelry for that body part.\n\r&w", ch);
        		send_to_char( "&RTry .\n\r&w", ch);
        		return;
    		}
    		
    	        if ( arg2[0] == '\0' )
                {
                  send_to_char( "&RUsage: Makejewelry <wearloc> <name>\n\r&w", ch);
                  return;   
                }
 
    	        checktool = FALSE;
                checkoven = FALSE;
                checkmetal = FALSE;
        
                for ( obj = ch->last_carrying; obj; obj = obj->prev_content )     
                {
                  if (obj->item_type == ITEM_TOOLKIT)
                    checktool = TRUE;
                  if (obj->item_type == ITEM_OVEN)
          	    checkoven = TRUE;
                  if (obj->item_type == ITEM_RARE_METAL)
          	    checkmetal = TRUE;
                }
                
                if ( !checktool )
                {
                   send_to_char( "&RYou need a toolkit.\n\r", ch);
                   return;
                }
 
                if ( !checkoven )
                {
                   send_to_char( "&RYou need an oven.\n\r", ch);
                   return;
                }
                
                if ( !checkmetal )
                {
                   send_to_char( "&RYou need some precious metal.\n\r", ch);
                   return;
                }

    	        chance = IS_NPC(ch) ? ch->top_level
	                 : (int) (ch->pcdata->learned[gsn_makejewelry]);
                if ( number_percent( ) < chance )
    		{
    		   send_to_char( "&GYou begin the long process of creating some jewelry.\n\r", ch);
    		   act( AT_PLAIN, "$n takes $s toolkit and some metal and begins to work.", ch,
		        NULL, argument , TO_ROOM );
		   add_timer ( ch , TIMER_DO_FUN , 10 , do_makejewelry , 1 );
    		   ch->dest_buf = str_dup(arg);
    		   ch->dest_buf_2 = str_dup(arg2);
    		   return;
	        }
	        send_to_char("&RYou can't figure out what to do.\n\r",ch);
	        learn_from_failure( ch, gsn_makejewelry );
    	   	return;	
    	
    	case 1: 
    		if ( !ch->dest_buf )
    		     return;
    		if ( !ch->dest_buf_2 )
    		     return;
    		strcpy(arg, ch->dest_buf);
    		DISPOSE( ch->dest_buf);
    		strcpy(arg2, ch->dest_buf_2);
    		DISPOSE( ch->dest_buf_2);
    		break;
    		
    	case SUB_TIMER_DO_ABORT:
    		DISPOSE( ch->dest_buf );
    		DISPOSE( ch->dest_buf_2 );
    		ch->substate = SUB_NONE;    		                                   
    	        send_to_char("&RYou are interupted and fail to finish your work.\n\r", ch);
    	        return;
    }
    
    ch->substate = SUB_NONE;
    
    level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->learned[gsn_makejewelry]);
    
    checkmetal = FALSE;
    checkoven = FALSE;
    checktool = FALSE;
    value=0;
    cost=0;
    
    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )     
    {
       if (obj->item_type == ITEM_TOOLKIT)
          checktool = TRUE;
       if (obj->item_type == ITEM_OVEN)
          checkoven = TRUE;
       if (obj->item_type == ITEM_RARE_METAL && checkmetal == FALSE)
       {
          checkmetal = TRUE;
          separate_obj( obj );
          obj_from_char( obj );
          metal = obj;
       }
       if (obj->item_type == ITEM_CRYSTAL)
       {
          cost += obj->cost;
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
       }
    }                            
    
    if ( ( !checkoven ) || ( !checktool ) || ( !checkmetal ) )
    {
       send_to_char( "&RYou hold up your newly created jewelry.\n\r", ch);
       send_to_char( "&RIt suddenly dawns upon you that you have created the most useless\n\r", ch);
       send_to_char( "&Rpiece of junk you've ever seen. You quickly hide your mistake...\n\r", ch);
       learn_from_failure( ch, gsn_makejewelry );
       return;
    }

    obj = metal; 

    obj->item_type = ITEM_ARMOR;
    SET_BIT( obj->wear_flags, ITEM_TAKE );
    value = get_wflag( arg );
    if ( value < 0 || value > 31 )
        SET_BIT( obj->wear_flags, ITEM_WEAR_NECK );                    
    else
        SET_BIT( obj->wear_flags, 1 << value );
    obj->level = level;
    STRFREE( obj->name );
    strcpy( buf, arg2 );
    obj->name = STRALLOC( buf );
    strcpy( buf, arg2 );
    STRFREE( obj->short_descr );
    obj->short_descr = STRALLOC( buf );        
    STRFREE( obj->description );
    strcat( buf, " was dropped here." );
    obj->description = STRALLOC( buf );
    obj->value[0] = obj->value[1];      
    obj->cost *= 10;
    obj->cost += cost;
                                                                    
    obj = obj_to_char( obj, ch );
                                                            
    send_to_char( "&GYou finish your work and hold up your newly created jewelry.&w\n\r", ch);
    act( AT_PLAIN, "$n finishes sewing some new jewelry.", ch,
         NULL, argument , TO_ROOM );
    
        learn_from_success( ch, gsn_makejewelry );
        learn_from_success( ch, gsn_makejewelry );
     
}

void do_makearmor( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int level, chance;
    bool checksew, checkfab; 
    OBJ_DATA *obj;
    OBJ_DATA *material;
    int value;
            
    argument = one_argument( argument, arg );
    strcpy ( arg2, argument);
    
    
    switch( ch->substate )
    { 
    	default:
    	        
		if ( str_cmp( arg, "body" )
    		&& str_cmp( arg, "head" )
    		&& str_cmp( arg, "legs" )
    		&& str_cmp( arg, "feet" )
    		&& str_cmp( arg, "hands" )
    		&& str_cmp( arg, "arms" )
    		&& str_cmp( arg, "about" )
    		&& str_cmp( arg, "waist" ) )
    		{
        		send_to_char( "&RYou cannot make clothing for that body part.\n\r&w", ch);
        		send_to_char( "&RTry: body, head, legs, feet, hands, arms, about, or waist.\n\r&w", ch);
        		return;
    		}
    	        
    	        if ( arg2[0] == '\0' )
                {
                  send_to_char( "&RUsage: Makearmor <wearloc> <name>\n\r&w", ch);
                  return;   
                }
 
    	        checksew = FALSE;
                checkfab = FALSE;
        
                for ( obj = ch->last_carrying; obj; obj = obj->prev_content )     
                {
                  if (obj->item_type == ITEM_FABRIC)
                    checkfab = TRUE;
                  if (obj->item_type == ITEM_THREAD)
          	    checksew = TRUE;
                }
                
                if ( !checkfab )
                {
                   send_to_char( "&RYou need some sort of fabric or material.\n\r", ch);
                   return;
                }
 
                if ( !checksew )
                {
                   send_to_char( "&RYou need a needle and some thread.\n\r", ch);
                   return;
                }

    	        chance = IS_NPC(ch) ? ch->top_level
	                 : (int) (ch->pcdata->learned[gsn_makearmor]);
                if ( number_percent( ) < chance )
    		{
    		   send_to_char( "&GYou begin the long process of creating some armor.\n\r", ch);
    		   act( AT_PLAIN, "$n takes $s sewing kit and some material and begins to work.", ch,
		        NULL, argument , TO_ROOM );
		   add_timer ( ch , TIMER_DO_FUN , 10 , do_makearmor , 1 );
    		   ch->dest_buf = str_dup(arg);
    		   ch->dest_buf_2 = str_dup(arg2);
    		   return;
	        }
	        send_to_char("&RYou can't figure out what to do.\n\r",ch);
	        learn_from_failure( ch, gsn_makearmor );
    	   	return;	
    	
    	case 1: 
    		if ( !ch->dest_buf )
    		     return;
    		if ( !ch->dest_buf_2 )
    		     return;
    		strcpy(arg, ch->dest_buf);
    		DISPOSE( ch->dest_buf);
    		strcpy(arg2, ch->dest_buf_2);
    		DISPOSE( ch->dest_buf_2);
    		break;
    		
    	case SUB_TIMER_DO_ABORT:
    		DISPOSE( ch->dest_buf );
    		DISPOSE( ch->dest_buf_2 );
    		ch->substate = SUB_NONE;    		                                   
    	        send_to_char("&RYou are interupted and fail to finish your work.\n\r", ch);
    	        return;
    }
    
    ch->substate = SUB_NONE;
    
    level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->learned[gsn_makearmor]);
    
    checksew = FALSE;
    checkfab = FALSE;
    
    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )     
    {
       if (obj->item_type == ITEM_THREAD)
          checksew = TRUE;
       if (obj->item_type == ITEM_FABRIC && checkfab == FALSE)
       {
          checkfab = TRUE;
          separate_obj( obj );
          obj_from_char( obj );
          material = obj;
       }
    }                            
    
    if ( ( !checkfab ) || ( !checksew ) )
    {
       send_to_char( "&RYou hold up your newly created armor.\n\r", ch);
       send_to_char( "&RIt suddenly dawns upon you that you have created the most useless\n\r", ch);
       send_to_char( "&Rgarment you've ever seen. You quickly hide your mistake...\n\r", ch);
       learn_from_failure( ch, gsn_makearmor );
       return;
    }

    obj = material; 

    obj->item_type = ITEM_ARMOR;
    SET_BIT( obj->wear_flags, ITEM_TAKE );
    value = get_wflag( arg );
    if ( value < 0 || value > 31 )
        SET_BIT( obj->wear_flags, ITEM_WEAR_BODY );                    
    else
        SET_BIT( obj->wear_flags, 1 << value );
    obj->level = level;
    STRFREE( obj->name );
    strcpy( buf, arg2 );
    obj->name = STRALLOC( buf );
    strcpy( buf, arg2 );
    STRFREE( obj->short_descr );
    obj->short_descr = STRALLOC( buf );        
    STRFREE( obj->description );
    strcat( buf, " was dropped here." );
    obj->description = STRALLOC( buf );
    obj->value[0] = obj->value[1];      
    obj->cost *= 10;
                                                                    
    obj = obj_to_char( obj, ch );
                                                            
    send_to_char( "&GYou finish your work and hold up your newly created garment.&w\n\r", ch);
    act( AT_PLAIN, "$n finishes sewing some new armor.", ch,
         NULL, argument , TO_ROOM );
    
        learn_from_success( ch, gsn_makearmor );
        learn_from_success( ch, gsn_makearmor );
        learn_from_success( ch, gsn_makearmor );
}


void do_makeshield( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int chance;
    bool checktool, checkbatt, checkcond, checkcirc, checkgems;
    OBJ_DATA *obj;
    OBJ_INDEX_DATA *pObjIndex;
    int vnum, level, charge, gemtype;
    
    strcpy( arg, argument );    
    
    switch( ch->substate )
    { 
    	default:
    	        if ( arg[0] == '\0' )
                {
                  send_to_char( "&RUsage: Makeshield <name>\n\r&w", ch);
                  return;   
                }

    	        checktool = FALSE;
                checkbatt = FALSE;
                checkcond = FALSE;
                checkcirc = FALSE;
                checkgems = FALSE;

                for ( obj = ch->last_carrying; obj; obj = obj->prev_content )     
                {
                  if (obj->item_type == ITEM_TOOLKIT)
                    checktool = TRUE;
                  if (obj->item_type == ITEM_CRYSTAL)
                    checkgems = TRUE;                    
                  if (obj->item_type == ITEM_BATTERY)
                    checkbatt = TRUE;
                  if (obj->item_type == ITEM_CIRCUIT)
                    checkcirc = TRUE;
                  if (obj->item_type == ITEM_SUPERCONDUCTOR)
                    checkcond = TRUE;                  
                }
                
                if ( !checktool )
                {
                   send_to_char( "&RYou need toolkit to make an energy shield.\n\r", ch);
                   return;
                }
 
                if ( !checkbatt )
                {
                   send_to_char( "&RYou need a power source for your energy shield.\n\r", ch);
                   return;
                }
                
                if ( !checkcirc )
                {
                   send_to_char( "&RYou need a small circuit board.\n\r", ch);
                   return;
                }
                
                if ( !checkcond )
                {
                   send_to_char( "&RYou still need a small superconductor for your energy shield.\n\r", ch);
                   return;
                }
                
                if ( !checkgems )
                {
                   send_to_char( "&RYou need a small crystal.\n\r", ch);
                   return;
                }
                
    	        chance = IS_NPC(ch) ? ch->top_level
	                 : (int) (ch->pcdata->learned[gsn_makeshield]);
                if ( number_percent( ) < chance )
    		{
    		   send_to_char( "&GYou begin the long process of crafting an energy shield.\n\r", ch);
    		   act( AT_PLAIN, "$n takes $s tools and begins to work on something.", ch,
		        NULL, argument , TO_ROOM );
		   add_timer ( ch , TIMER_DO_FUN , 10 , do_makeshield , 1 );
    		   ch->dest_buf = str_dup(arg);
    		   return;
	        }
	        send_to_char("&RYou can't figure out how to fit the parts together.\n\r",ch);
	        learn_from_failure( ch, gsn_makeshield );
    	   	return;	
    	
    	case 1: 
    		if ( !ch->dest_buf )
    		     return;
    		strcpy(arg, ch->dest_buf);
    		DISPOSE( ch->dest_buf);
    		break;
    		
    	case SUB_TIMER_DO_ABORT:
    		DISPOSE( ch->dest_buf );
    		ch->substate = SUB_NONE;    		                                   
    	        send_to_char("&RYou are interupted and fail to finish your work.\n\r", ch);
    	        return;
    }
    
    ch->substate = SUB_NONE;
    
    level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->learned[gsn_makeshield]);
    vnum = OBJ_VNUM_PROTO_SHIELD;
    
    if ( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
    {
         send_to_char( "&RThe item you are trying to create is missing from the database.\n\rPlease inform the administration of this error.\n\r", ch );
         return;
    }

    checktool = FALSE;
    checkbatt = FALSE;
    checkcond = FALSE;
    checkcirc = FALSE;
    checkgems = FALSE;
    charge = 0;

    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )     
    {
       if (obj->item_type == ITEM_TOOLKIT)
          checktool = TRUE;

       if (obj->item_type == ITEM_BATTERY && checkbatt == FALSE)
       {
          charge = UMIN(obj->value[1], 10);
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
          checkbatt = TRUE;
       }
       if (obj->item_type == ITEM_SUPERCONDUCTOR && checkcond == FALSE)
       {
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
          checkcond = TRUE;
       }
       if (obj->item_type == ITEM_CIRCUIT && checkcirc == FALSE)
       {
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
          checkcirc = TRUE;
       }
       if (obj->item_type == ITEM_CRYSTAL && checkgems == FALSE)
       {
          gemtype = obj->value[0];
          separate_obj( obj );
          obj_from_char( obj );
          extract_obj( obj );
          checkgems = TRUE;
       }
    }                            
    
    if ( ( !checktool ) || ( !checkbatt )
                                       || ( !checkgems ) || ( !checkcond ) || ( !checkcirc) )
    
    {
       send_to_char( "&RYou hold up your new energy shield and press the switch hoping for the best.\n\r", ch);
       send_to_char( "&RInstead of a field of energy being created, smoke starts pouring from the device.\n\r", ch);
       send_to_char( "&RYou drop the hot device and watch as it melts on away on the floor.\n\r", ch);
       learn_from_failure( ch, gsn_makeshield );
       return;
    }

    obj = create_object( pObjIndex, level );
    
    obj->item_type = ITEM_ARMOR;
    SET_BIT( obj->wear_flags, ITEM_WIELD );
    SET_BIT( obj->wear_flags, ITEM_WEAR_SHIELD );
    obj->level = level;
    obj->weight = 2;
    STRFREE( obj->name );
    obj->name = STRALLOC( "energy shield" );
    strcpy( buf, arg );
    STRFREE( obj->short_descr );
    obj->short_descr = STRALLOC( buf );        
    STRFREE( obj->description );
    strcat( buf, " was carelessly misplaced here." );
    obj->description = STRALLOC( buf );
    obj->value[0] = (int) (level/10+gemtype*2);      /* condition */
    obj->value[1] = (int) (level/10+gemtype*2);      /* armor */
    obj->value[4] = charge;
    obj->value[5] = charge;
    obj->cost = obj->value[2]*100;
                                                                    
    obj = obj_to_char( obj, ch );
                                                            
    send_to_char( "&GYou finish your work and hold up your newly created energy shield.&w\n\r", ch);
    act( AT_PLAIN, "$n finishes making $s new energy shield.", ch,
         NULL, argument , TO_ROOM );
    
        learn_from_success( ch, gsn_makeshield );
        learn_from_success( ch, gsn_makeshield );
        learn_from_success( ch, gsn_makeshield );

}

void do_makecontainer( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int level, chance;
    bool checksew, checkfab; 
    OBJ_DATA *obj;
    OBJ_DATA *material;
    int value;
            
    argument = one_argument( argument, arg );
    strcpy( arg2 , argument );
    
    
    switch( ch->substate )
    { 
    	default:
    	        
    		if ( str_cmp( arg, "body" )
    		&& str_cmp( arg, "legs" )
    		&& str_cmp( arg, "about" )
    		&& str_cmp( arg, "waist" )
    		&& str_cmp( arg, "hold" ) )
    		{
        		send_to_char( "&RYou cannot make a container for that body part.\n\r&w", ch);
        		send_to_char( "&RTry body, legs, about, waist or hold.\n\r&w", ch);
        		return;
    		}
    		
    	        if ( arg2[0] == '\0' )
                {
                  send_to_char( "&RUsage: Makecontainer <wearloc> <name>\n\r&w", ch);
                  return;   
                }
 
    	        checksew = FALSE;
                checkfab = FALSE;
        
                for ( obj = ch->last_carrying; obj; obj = obj->prev_content )     
                {
                  if (obj->item_type == ITEM_FABRIC)
                    checkfab = TRUE;
                  if (obj->item_type == ITEM_THREAD)
          	    checksew = TRUE;
                }
                
                if ( !checkfab )
                {
                   send_to_char( "&RYou need some sort of fabric or material.\n\r", ch);
                   return;
                }
 
                if ( !checksew )
                {
                   send_to_char( "&RYou need a needle and some thread.\n\r", ch);
                   return;
                }

    	        chance = IS_NPC(ch) ? ch->top_level
	                 : (int) (ch->pcdata->learned[gsn_makecontainer]);
                if ( number_percent( ) < chance )
    		{
    		   send_to_char( "&GYou begin the long process of creating a bag.\n\r", ch);
    		   act( AT_PLAIN, "$n takes $s sewing kit and some material and begins to work.", ch,
		        NULL, argument , TO_ROOM );
		   add_timer ( ch , TIMER_DO_FUN , 10 , do_makecontainer , 1 );
    		   ch->dest_buf = str_dup(arg);
    		   ch->dest_buf_2 = str_dup(arg2);
    		   return;
	        }
	        send_to_char("&RYou can't figure out what to do.\n\r",ch);
	        learn_from_failure( ch, gsn_makecontainer );
    	   	return;	
    	
    	case 1: 
    		if ( !ch->dest_buf )
    		     return;
    		if ( !ch->dest_buf_2 )
    		     return;
    		strcpy(arg, ch->dest_buf);
    		DISPOSE( ch->dest_buf);
    		strcpy(arg2, ch->dest_buf_2);
    		DISPOSE( ch->dest_buf_2);
    		break;
    		
    	case SUB_TIMER_DO_ABORT:
    		DISPOSE( ch->dest_buf );
    		DISPOSE( ch->dest_buf_2 );
    		ch->substate = SUB_NONE;    		                                   
    	        send_to_char("&RYou are interupted and fail to finish your work.\n\r", ch);
    	        return;
    }
    
    ch->substate = SUB_NONE;
    
    level = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->learned[gsn_makecontainer]);
    
    checksew = FALSE;
    checkfab = FALSE;
    
    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )     
    {
       if (obj->item_type == ITEM_THREAD)
          checksew = TRUE;
       if (obj->item_type == ITEM_FABRIC && checkfab == FALSE)
       {
          checkfab = TRUE;
          separate_obj( obj );
          obj_from_char( obj );
          material = obj;
       }
    }                            
    
    if ( ( !checkfab ) || ( !checksew ) )
    {
       send_to_char( "&RYou hold up your newly created container.\n\r", ch);
       send_to_char( "&RIt suddenly dawns upon you that you have created the most useless\n\r", ch);
       send_to_char( "&Rcontainer you've ever seen. You quickly hide your mistake...\n\r", ch);
       learn_from_failure( ch, gsn_makecontainer );
       return;
    }

    obj = material; 

    obj->item_type = ITEM_CONTAINER;
    SET_BIT( obj->wear_flags, ITEM_TAKE );
    value = get_wflag( arg );
    if ( value < 0 || value > 31 )
        SET_BIT( obj->wear_flags, ITEM_HOLD );                    
    else
        SET_BIT( obj->wear_flags, 1 << value );
    obj->level = level;
    STRFREE( obj->name );
    strcpy( buf, arg2 );
    obj->name = STRALLOC( buf );
    strcpy( buf, arg2 );
    STRFREE( obj->short_descr );
    obj->short_descr = STRALLOC( buf );        
    STRFREE( obj->description );
    strcat( buf, " was dropped here." );
    obj->description = STRALLOC( buf );
    obj->value[0] = level;
    obj->value[1] = 0;
    obj->value[2] = 0;      
    obj->value[3] = 10;      
    obj->cost *= 2;
                                                                    
    obj = obj_to_char( obj, ch );
                                                            
    send_to_char( "&GYou finish your work and hold up your newly created container.&w\n\r", ch);
    act( AT_PLAIN, "$n finishes sewing a new container.", ch,
         NULL, argument , TO_ROOM );
    
        learn_from_success( ch, gsn_makecontainer );
        learn_from_success( ch, gsn_makecontainer );
        learn_from_success( ch, gsn_makecontainer );
}

void do_reinforcements( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int chance, credits;
    
    if ( IS_NPC( ch ) || !ch->pcdata )
    	return;
    	
    strcpy( arg, argument );    
    
    switch( ch->substate )
    { 
    	default:
    	        if ( ch->backup_wait )
    	        {
    	            send_to_char( "&RYour reinforcements are already on the way.\n\r", ch );
    	            return;
    	        }
    	        
    	        if ( !ch->pcdata->clan )
    	        {
    	            send_to_char( "&RYou need to be a member of an organization before you can call for reinforcements.\n\r", ch );
    	            return;
    	        }    
    	        
    	        if ( ch->gold < 5000 )
    	        {
    	            ch_printf( ch, "&RYou dont have enough credits to send for reinforcements.\n\r" );
    	            return;
    	        }    
    	        
    	        chance = (int) (ch->pcdata->learned[gsn_reinforcements]);
                if ( number_percent( ) < chance )
    		{
    		   send_to_char( "&GYou begin making the call for reinforcements.\n\r", ch);
    		   act( AT_PLAIN, "$n begins issuing orders int $s comlink.", ch,
		        NULL, argument , TO_ROOM );
		   add_timer ( ch , TIMER_DO_FUN , 1 , do_reinforcements , 1 );
    		   ch->dest_buf = str_dup(arg);
    		   return;
	        }
	        send_to_char("&RYou call for reinforcements but nobody answers.\n\r",ch);
	        learn_from_failure( ch, gsn_reinforcements );
    	   	return;	
    	
    	case 1:
    		if ( !ch->dest_buf )
    		   return;
    		strcpy(arg, ch->dest_buf);
    		DISPOSE( ch->dest_buf);
    		break;
    		
    	case SUB_TIMER_DO_ABORT:
    		DISPOSE( ch->dest_buf );
    		ch->substate = SUB_NONE;    		                                   
    	        send_to_char("&RYou are interupted before you can finish your call.\n\r", ch);
    	        return;
    }
    
    ch->substate = SUB_NONE;
    
    send_to_char( "&GYour reinforcements are on the way.\n\r", ch);
    credits = 5000;
    ch_printf( ch, "It cost you %d credits.\n\r", credits);
    ch->gold -= UMIN( credits , ch->gold );
             
    learn_from_success( ch, gsn_reinforcements );
    learn_from_success( ch, gsn_reinforcements );
    
    ch->backup_mob = MOB_VNUM_SOLDIER;

    ch->backup_wait = 1;
    
}

void do_postguard( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int chance, credits;
    
    if ( IS_NPC( ch ) || !ch->pcdata )
    	return;
    	
    strcpy( arg, argument );    
    
    switch( ch->substate )
    { 
    	default:
    	        if ( ch->backup_wait )
    	        {
    	            send_to_char( "&RYou already have backup coming.\n\r", ch );
    	            return;
    	        }
    	        
    	        if ( !ch->pcdata->clan )
    	        {
    	            send_to_char( "&RYou need to be a member of an organization before you can call for a guard.\n\r", ch );
    	            return;
    	        }    
                
                if ( !ch->in_room || !ch->in_room->area || 
                ( ch->in_room->area->planet && ch->in_room->area->planet->governed_by != ch->pcdata->clan ) )
    	        {
    	            send_to_char( "&RYou cannot post guards on enemy planets. Try calling for reinforcements instead.\n\r", ch );
    	            return;
    	        }    
                    	        
    	        if ( ch->gold < 5000 )
    	        {
    	            ch_printf( ch, "&RYou dont have enough credits.\n\r", ch );
    	            return;
    	        }    
    	        
    	        chance = (int) (ch->pcdata->learned[gsn_postguard]);
                if ( number_percent( ) < chance )
    		{
    		   send_to_char( "&GYou begin making the call for reinforcements.\n\r", ch);
    		   act( AT_PLAIN, "$n begins issuing orders int $s comlink.", ch,
		        NULL, argument , TO_ROOM );
		   add_timer ( ch , TIMER_DO_FUN , 1 , do_postguard , 1 );
    		   ch->dest_buf = str_dup(arg);
    		   return;
	        }
	        send_to_char("&RYou call for a guard but nobody answers.\n\r",ch);
	        learn_from_failure( ch, gsn_postguard );
    	   	return;	
    	
    	case 1:
    		if ( !ch->dest_buf )
    		   return;
    		strcpy(arg, ch->dest_buf);
    		DISPOSE( ch->dest_buf);
    		break;
    		
    	case SUB_TIMER_DO_ABORT:
    		DISPOSE( ch->dest_buf );
    		ch->substate = SUB_NONE;    		                                   
    	        send_to_char("&RYou are interupted before you can finish your call.\n\r", ch);
    	        return;
    }
    
    ch->substate = SUB_NONE;
    
    send_to_char( "&GYour guard is on the way.\n\r", ch);
    
    credits = 5000;
    ch_printf( ch, "It cost you %d credits.\n\r", credits);
    ch->gold -= UMIN( credits , ch->gold );

    learn_from_success( ch, gsn_postguard );
    learn_from_success( ch, gsn_postguard );
    
    ch->backup_mob = MOB_VNUM_GUARD;

    ch->backup_wait = 1;

}

void add_reinforcements( CHAR_DATA *ch )
{
     MOB_INDEX_DATA  * pMobIndex;
     OBJ_DATA        * blaster;
     OBJ_INDEX_DATA  * pObjIndex;
       
     
     if ( !ch->in_room )
        return;
     
     if ( ( pMobIndex = get_mob_index( ch->backup_mob ) ) == NULL )
        return;         

     if ( ch->backup_mob == MOB_VNUM_SOLDIER       )  
     {
        CHAR_DATA * mob[3];
        int         mob_cnt;
        
        send_to_char( "Your reinforcements have arrived.\n\r", ch );
        for ( mob_cnt = 0 ; mob_cnt < 3 ; mob_cnt++ )
        {
            mob[mob_cnt] = create_mobile( pMobIndex );
            if ( !mob[mob_cnt] )
                return;
            char_to_room( mob[mob_cnt], ch->in_room );
            act( AT_IMMORT, "$N has arrived.", ch, NULL, mob[mob_cnt], TO_ROOM );
            mob[mob_cnt]->top_level = 50;
            mob[mob_cnt]->hit = 100;
            mob[mob_cnt]->max_hit = 100;
            mob[mob_cnt]->armor = 50;
            mob[mob_cnt]->damroll = 0;
            mob[mob_cnt]->hitroll = 10;
            if ( ( pObjIndex = get_obj_index( OBJ_VNUM_BLASTER ) ) != NULL )
            {
                 blaster = create_object( pObjIndex, mob[mob_cnt]->top_level );
                 obj_to_char( blaster, mob[mob_cnt] );
                 equip_char( mob[mob_cnt], blaster, WEAR_WIELD );                        
            } 
            
            if ( mob[mob_cnt]->master )
	       stop_follower( mob[mob_cnt] );
	    add_follower( mob[mob_cnt], ch );
            SET_BIT( mob[mob_cnt]->affected_by, AFF_CHARM );
            do_setblaster( mob[mob_cnt] , "full" );
            if ( ch->pcdata && ch->pcdata->clan )   
               mob[mob_cnt]->mob_clan = ch->pcdata->clan;
        }
     }
     else
     {
        CHAR_DATA *mob;
        
        mob = create_mobile( pMobIndex );
        char_to_room( mob, ch->in_room );
        if ( ch->pcdata && ch->pcdata->clan )
        {
          char tmpbuf[MAX_STRING_LENGTH];
        
          sprintf( tmpbuf , "A guard stands at attention. (%s)\n\r" , ch->pcdata->clan->name );
          STRFREE( mob->long_descr );
          mob->long_descr = STRALLOC( tmpbuf );
        }
        act( AT_IMMORT, "$N has arrived.", ch, NULL, mob, TO_ROOM );
        send_to_char( "Your guard has arrived.\n\r", ch );
        mob->top_level = 75;
        mob->hit = 200;
        mob->max_hit = 200;
        mob->armor = 0;
        mob->damroll = 5;
        mob->hitroll = 20;
        if ( ( pObjIndex = get_obj_index( OBJ_VNUM_BLASTER ) ) != NULL )
        {
            blaster = create_object( pObjIndex, mob->top_level );
            obj_to_char( blaster, mob );
            equip_char( mob, blaster, WEAR_WIELD );                        
        }
        do_setblaster( mob , "full" );
        if ( ch->pcdata && ch->pcdata->clan )   
               mob->mob_clan = ch->pcdata->clan;
     }                    
}

void do_torture( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance, dam;
    bool fail;
    
    if ( !IS_NPC(ch)
    &&  ch->pcdata->learned[gsn_torture] <= 0  )
    {
	send_to_char(
	    "Your mind races as you realize you have no idea how to do that.\n\r", ch );
	return;
    }

    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char( "You can't do that right now.\n\r", ch );
	return;
    }

    one_argument( argument, arg );

    if ( ch->mount )
    {
	send_to_char( "You can't get close enough while mounted.\n\r", ch );
	return;
    }

    if ( arg[0] == '\0' )
    {
	send_to_char( "Torture whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Are you masacistic or what...\n\r", ch );
	return;
    }
    
    if ( !IS_AWAKE(victim) )
    {
	send_to_char( "You need to wake them first.\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim ) )
      return;

    if ( victim->fighting )
    {
	send_to_char( "You can't torture someone whos in combat.\n\r", ch );
	return;
    }
    
    ch->alignment = ch->alignment -= 100;
    ch->alignment = URANGE( -1000, ch->alignment, 1000 );
    
    WAIT_STATE( ch, skill_table[gsn_torture]->beats );
    
    fail = FALSE;
    chance = ris_save( victim, IS_NPC(ch) ? ch->top_level : ch->pcdata->learned[gsn_torture], RIS_PARALYSIS );
    if ( chance == 1000 )
      fail = TRUE;
    else
      fail = saves_para_petri( chance, victim );

    chance = 5;
    if ( !fail
    && (  IS_NPC(ch)
    || (number_percent( ) + chance) < ch->pcdata->learned[gsn_torture] ) )
    {
	learn_from_success( ch, gsn_torture );
	WAIT_STATE( ch,     2 * PULSE_VIOLENCE );
	WAIT_STATE( victim, PULSE_VIOLENCE );
	act( AT_SKILL, "$N slowly tortures you. The pain is excruciating.", victim, NULL, ch, TO_CHAR );
	act( AT_SKILL, "You torture $N, leaving $M screaming in pain.", ch, NULL, victim, TO_CHAR );
	act( AT_SKILL, "$n tortures $N, leaving $M screaming in agony!", ch, NULL, victim, TO_NOTVICT );
        
        dam = dice( (IS_NPC(ch) ? ch->top_level : ch->pcdata->learned[gsn_torture])/10 , 4 );
        dam = URANGE( 0, victim->max_hit-10, dam ); 
        victim->hit -= dam;
        victim->max_hit -= dam;
        
        ch_printf( victim, "You lose %d permanent hit points." ,dam);
        ch_printf( ch, "They lose %d permanent hit points." , dam);
         
    }
    else
    {
	act( AT_SKILL, "$N tries to cut off your finger!", victim, NULL, ch, TO_CHAR );
	act( AT_SKILL, "You mess up big time.", ch, NULL, victim, TO_CHAR );
	act( AT_SKILL, "$n tries to painfully torture $N.", ch, NULL, victim, TO_NOTVICT );
	WAIT_STATE( ch,     2 * PULSE_VIOLENCE );
        global_retcode = multi_hit( victim, ch, TYPE_UNDEFINED );
    }
    return;
    
}

void do_disguise( CHAR_DATA *ch, char *argument )
{
    int chance;

    if ( IS_NPC(ch) )
	return;

    if ( IS_SET( ch->pcdata->flags, PCFLAG_NOTITLE ))
    {
        send_to_char( "You try but the Force resists you.\n\r", ch );
        return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Change your title to what?\n\r", ch );
	return;
    }
    
    chance = (int) (ch->pcdata->learned[gsn_disguise]);
    
    if ( number_percent( ) > chance )
    {
        send_to_char( "You try to disguise yourself but fail.\n\r", ch );
        return;
    }
                
    if ( strlen(argument) > 50 )
	argument[50] = '\0';
    
    smash_tilde( argument );
    set_title( ch, argument );
    send_to_char( "Ok.\n\r", ch );
}

void do_first_aid( CHAR_DATA *ch, char *argument )
{
   OBJ_DATA   *medpac;
   CHAR_DATA  *victim;
   int         heal;
   char        buf[MAX_STRING_LENGTH];

   if ( ch->position == POS_FIGHTING )
   {
         send_to_char( "You can't do that while fighting!\n\r",ch );
         return;
   }
   
   medpac = get_eq_char( ch, WEAR_HOLD );
   if ( !medpac || medpac->item_type != ITEM_MEDPAC )
   {
         send_to_char( "You need to be holding a medpac.\n\r",ch );
         return;
   }  

   if ( medpac->value[0] <= 0 )
   {
         send_to_char( "Your medpac seems to be empty.\n\r",ch );
         return;         
   }
   
   if ( argument[0] == '\0' )
      victim = ch;
   else
      victim = get_char_room( ch, argument );
            
   if ( !victim )
   {
       ch_printf( ch, "I don't see any %s here...\n\r" , argument );     
       return;
   }
   
   heal = number_range( 1, 150 );
   
   if ( heal > ch->pcdata->learned[gsn_first_aid]*2 )
   {
       ch_printf( ch, "You fail in your attempt at first aid.\n\r");     
       learn_from_failure( ch , gsn_first_aid );
       return;
   } 
      
   if ( victim == ch )
   {
       ch_printf( ch, "You tend to your wounds.\n\r");
       sprintf( buf , "$n uses %s to help heal $s wounds." , medpac->short_descr );        
       act( AT_ACTION, buf, ch, NULL, victim, TO_ROOM );  
   }
   else
   {
       sprintf( buf , "You tend to $N's wounds." );        
       act( AT_ACTION, buf, ch, NULL, victim, TO_CHAR );  
       sprintf( buf , "$n uses %s to help heal $N's wounds." , medpac->short_descr );        
       act( AT_ACTION, buf, ch, NULL, victim, TO_NOTVICT );  
       sprintf( buf , "$n uses %s to help heal your wounds." , medpac->short_descr );        
       act( AT_ACTION, buf, ch, NULL, victim, TO_VICT );  
   }

   --medpac->value[0];
   victim->hit += URANGE ( 0, heal , victim->max_hit - victim->hit );

   learn_from_success( ch , gsn_first_aid );
}

void do_snipe( CHAR_DATA *ch, char *argument )
{
   OBJ_DATA        * wield;
   char              arg[MAX_INPUT_LENGTH];  
   char              arg2[MAX_INPUT_LENGTH];
   sh_int            dir, dist;
   sh_int            max_dist = 3;
   EXIT_DATA       * pexit;
   ROOM_INDEX_DATA * was_in_room;
   ROOM_INDEX_DATA * to_room;
   CHAR_DATA       * victim;
   char              buf[MAX_STRING_LENGTH];
   bool              pfound = FALSE;
   
   if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
    {
	set_char_color( AT_MAGIC, ch );
	send_to_char( "You'll have to do that elswhere.\n\r", ch );
	return;
    }
     
   if ( get_eq_char( ch, WEAR_DUAL_WIELD ) != NULL )
   {
         send_to_char( "You can't do that while wielding two weapons.",ch );
         return;
   }
    
   wield = get_eq_char( ch, WEAR_WIELD );
   if ( !wield || wield->item_type != ITEM_WEAPON || wield->value[3] != WEAPON_BLASTER )
   {
         send_to_char( "You don't seem to be holding a blaster",ch );
         return;
   }  

   argument = one_argument( argument, arg );
   argument = one_argument( argument, arg2 );
   
   if ( ( dir = get_door( arg ) ) == -1 || arg2[0] == '\0' )
   {
     send_to_char( "Usage: snipe <dir> <target>\n\r", ch );
     return;
   }
 
   if ( ( pexit = get_exit( ch->in_room, dir ) ) == NULL )
   {
     send_to_char( "Are you expecting to fire through a wall!?\n\r", ch );
     return;
   }

   if ( IS_SET( pexit->exit_info, EX_CLOSED ) )
   {
     send_to_char( "Are you expecting to fire through a door!?\n\r", ch );
     return;
   }

   was_in_room = ch->in_room;
   
   for ( dist = 0; dist <= max_dist; dist++ )   
   {
     if ( IS_SET( pexit->exit_info, EX_CLOSED ) )
        break; 
     
     if ( !pexit->to_room )
        break;
     
       to_room = pexit->to_room;
    
     char_from_room( ch );
     char_to_room( ch, to_room );    
     

     if ( IS_NPC(ch) && ( victim = get_char_room_mp( ch, arg2 ) ) != NULL )
     {
        pfound = TRUE;
        break;
     }
     else if ( !IS_NPC(ch) && ( victim = get_char_room( ch, arg2 ) ) != NULL )
     {
        pfound = TRUE;
        break;
     }


     if ( ( pexit = get_exit( ch->in_room, dir ) ) == NULL )
        break;
            
   }
   
   char_from_room( ch );
   char_to_room( ch, was_in_room );    
       
   if ( !pfound )
   {
       ch_printf( ch, "You don't see that person to the %s!\n\r", dir_name[dir] );
       char_from_room( ch );
       char_to_room( ch, was_in_room );    
       return;
   }
   
    if ( victim == ch )
    {
	send_to_char( "Shoot yourself ... really?\n\r", ch );
	return;
    }
    
    if ( IS_SET( victim->in_room->room_flags, ROOM_SAFE ) )
    {
	set_char_color( AT_MAGIC, ch );
	send_to_char( "You can't shoot them there.\n\r", ch );
	return;
    }
 
    if ( is_safe( ch, victim ) )
	return;
    
    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
        act( AT_PLAIN, "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( ch->position == POS_FIGHTING )
    {
	send_to_char( "You do the best you can!\n\r", ch );
	return;
    }
    
    if ( !IS_NPC( victim ) && IS_SET( ch->act, PLR_NICE ) )
    {
      send_to_char( "You feel too nice to do that!\n\r", ch );
      return;
    }
    
    switch ( dir )
    {
        case 0:
        case 1:
           dir += 2;
           break;
        case 2:
        case 3:
           dir -= 2;
           break;
        case 4:
        case 7:
           dir += 1;
           break;
        case 5:
        case 8:
           dir -= 1;
           break;
        case 6:
           dir += 3;
           break;
        case 9:
           dir -=3;
           break;
    }
    
    char_from_room( ch );
    char_to_room( ch, victim->in_room );    
                
       sprintf( buf , "A blaster shot fires at you from the %s." , dir_name[dir] );
       act( AT_ACTION, buf , victim, NULL, ch, TO_CHAR );      
       act( AT_ACTION, "You fire at $N.", ch, NULL, victim, TO_CHAR );         
       sprintf( buf, "A blaster shot fires at $N from the %s." , dir_name[dir] );
       act( AT_ACTION, buf, ch, NULL, victim, TO_NOTVICT );  
                                                   
       one_hit( ch, victim, TYPE_UNDEFINED );  
       
       if ( char_died(ch) ) 
          return;
          
       stop_fighting( ch , TRUE );
       
            
    char_from_room( ch );
    char_to_room( ch, was_in_room );    
     
   WAIT_STATE( ch, 1 * PULSE_VIOLENCE );

   if ( IS_NPC( victim ) && !char_died(victim) )
   {
      if ( IS_SET( victim->act , ACT_SENTINEL ) )
      {
         victim->was_sentinel = victim->in_room;
         REMOVE_BIT( victim->act, ACT_SENTINEL );
      }
      
      start_hating( victim , ch );
      start_hunting( victim, ch );
     
   } 
   
}

/* syntax throw <obj> [direction] [target] */

void do_throw( CHAR_DATA *ch, char *argument )
{
   OBJ_DATA        * obj;
   OBJ_DATA        * tmpobj;
   char              arg[MAX_INPUT_LENGTH];  
   char              arg2[MAX_INPUT_LENGTH];
   char              arg3[MAX_INPUT_LENGTH];
   sh_int            dir;
   EXIT_DATA       * pexit;
   ROOM_INDEX_DATA * was_in_room;
   ROOM_INDEX_DATA * to_room;
   CHAR_DATA       * victim;
   char              buf[MAX_STRING_LENGTH];


   argument = one_argument( argument, arg );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );
   
   was_in_room = ch->in_room;
      
   if ( arg[0] == '\0' )
   {
     send_to_char( "Usage: throw <object> [direction] [target]\n\r", ch );
     return;
   }

     
   obj = get_eq_char( ch, WEAR_MISSILE_WIELD );
   if ( !obj || !nifty_is_name( arg, obj->name ) )
      obj = get_eq_char( ch, WEAR_HOLD );
      if ( !obj || !nifty_is_name( arg, obj->name ) )
          obj = get_eq_char( ch, WEAR_WIELD );
          if ( !obj || !nifty_is_name( arg, obj->name ) )
              obj = get_eq_char( ch, WEAR_DUAL_WIELD );
              if ( !obj || !nifty_is_name( arg, obj->name ) )
   if ( !obj || !nifty_is_name_prefix( arg, obj->name ) )
      obj = get_eq_char( ch, WEAR_HOLD );
      if ( !obj || !nifty_is_name_prefix( arg, obj->name ) )
          obj = get_eq_char( ch, WEAR_WIELD );
          if ( !obj || !nifty_is_name_prefix( arg, obj->name ) )
              obj = get_eq_char( ch, WEAR_DUAL_WIELD );
              if ( !obj || !nifty_is_name_prefix( arg, obj->name ) )
   {
         ch_printf( ch, "You don't seem to be holding or wielding %s.\n\r", arg );
         return;
   }  

    if ( IS_OBJ_STAT(obj, ITEM_NOREMOVE) )
    {
	act( AT_PLAIN, "You can't throw $p.", ch, obj, NULL, TO_CHAR );
	return;
    }

   if ( ch->position == POS_FIGHTING )
   {
       victim = who_fighting( ch );
       if ( char_died ( victim ) )
           return;
       act( AT_ACTION, "You throw $p at $N.", ch, obj, victim, TO_CHAR );
       act( AT_ACTION, "$n throws $p at $N.", ch, obj, victim, TO_NOTVICT );
       act( AT_ACTION, "$n throw $p at you.", ch, obj, victim, TO_VICT );        
   }
   else if ( arg2[0] == '\0' )
   {
       sprintf( buf, "$n throws %s at the floor." , obj->short_descr );
       act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );  
       ch_printf( ch, "You throw %s at the floor.\n\r", obj->short_descr );
       
       victim = NULL;
   }
   else  if ( ( dir = get_door( arg2 ) ) != -1 )
   {
      if ( ( pexit = get_exit( ch->in_room, dir ) ) == NULL )
      {
         send_to_char( "Are you expecting to throw it through a wall!?\n\r", ch );
         return;
      }

      
      if ( IS_SET( pexit->exit_info, EX_CLOSED ) )
      {
          send_to_char( "Are you expecting to throw it  through a door!?\n\r", ch );
          return;
      }
      
      
      switch ( dir )
      {
        case 0:
        case 1:
           dir += 2;
           break;
        case 2:
        case 3:
           dir -= 2;
           break;
        case 4:
        case 7:
           dir += 1;
           break;
        case 5:
        case 8:
           dir -= 1;
           break;
        case 6:
           dir += 3;
           break;
        case 9:
           dir -=3;
           break;
      }

       to_room = pexit->to_room;
    

      char_from_room( ch );
      char_to_room( ch, to_room );    
     
      victim = get_char_room( ch, arg3 );

      if ( victim )
      { 
        if ( is_safe( ch, victim ) )
	return;
    
        if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
        {
        act( AT_PLAIN, "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
	return;
        }
    
        if ( !IS_NPC( victim ) && IS_SET( ch->act, PLR_NICE ) )
        {
        send_to_char( "You feel too nice to do that!\n\r", ch );
        return;
        }
    
        char_from_room( ch );
        char_to_room( ch, was_in_room );    

      
        if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
        {
	set_char_color( AT_MAGIC, ch );
	send_to_char( "You'll have to do that elswhere.\n\r", ch );
	return;
        }
       
           to_room = pexit->to_room;
    
       
        char_from_room( ch );
        char_to_room( ch, to_room );    
        
        sprintf( buf , "Someone throws %s at you from the %s." , obj->short_descr , dir_name[dir] );
        act( AT_ACTION, buf , victim, NULL, ch, TO_CHAR );      
        act( AT_ACTION, "You throw %p at $N.", ch, obj, victim, TO_CHAR );         
        sprintf( buf, "%s is thrown at $N from the %s." , obj->short_descr , dir_name[dir] );
        act( AT_ACTION, buf, ch, NULL, victim, TO_NOTVICT );  


      }
      else
      {   
         ch_printf( ch, "You throw %s %s.\n\r", obj->short_descr , dir_name[get_dir( arg2 )] );
         sprintf( buf, "%s is thrown from the %s." , obj->short_descr , dir_name[dir] );
         act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );  

      }
   }
   else if ( ( victim = get_char_room( ch, arg2 ) ) != NULL )
   {
        if ( is_safe( ch, victim ) )
	return;
    
        if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
        {
        act( AT_PLAIN, "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
	return;
        }
    
        if ( !IS_NPC( victim ) && IS_SET( ch->act, PLR_NICE ) )
        {
        send_to_char( "You feel too nice to do that!\n\r", ch );
        return;
        }
    
   }
   else
   {
       ch_printf( ch, "They don't seem to be here!\n\r");
       return;         
   }

   
   if ( obj == get_eq_char( ch, WEAR_WIELD )
   && ( tmpobj = get_eq_char( ch, WEAR_DUAL_WIELD)) != NULL )
       tmpobj->wear_loc = WEAR_WIELD;

   unequip_char( ch, obj );
   separate_obj( obj );
   obj_from_char( obj );
   obj = obj_to_room( obj, ch->in_room );
   
   damage_obj ( obj );
   
/* NOT NEEDED UNLESS REFERING TO OBJECT AGAIN 

   if( obj_extracted(obj) )
      return;
*/
   if ( ch->in_room !=  was_in_room )
   {
     char_from_room( ch );
     char_to_room( ch, was_in_room );    
   }
   
   if ( !victim || char_died( victim ) )
       learn_from_failure( ch, gsn_throw );
   else
   {
       
       WAIT_STATE( ch, skill_table[gsn_throw]->beats );
       if ( IS_NPC(ch) || number_percent( ) < ch->pcdata->learned[gsn_throw] )
       {
	 learn_from_success( ch, gsn_throw );
	 global_retcode = damage( ch, victim, number_range( obj->weight*2 , (obj->weight*2 + ch->perm_str) ), TYPE_HIT );
       }
       else
       {
	 learn_from_failure( ch, gsn_throw );
	 global_retcode = damage( ch, victim, 0, TYPE_HIT );
       }
    
       if ( IS_NPC( victim ) && !char_died ( victim) )
       {
          if ( IS_SET( victim->act , ACT_SENTINEL ) )
          {
             victim->was_sentinel = victim->in_room;
             REMOVE_BIT( victim->act, ACT_SENTINEL );
          }
      
          start_hating( victim , ch );
          start_hunting( victim, ch );
     
       } 

   }
   
   return;                                   
  
}

void do_pickshiplock( CHAR_DATA *ch, char *argument )
{
   do_pick( ch, argument );
}

void do_hijack( CHAR_DATA *ch, char *argument )
{
    int chance; 
    SHIP_DATA *ship;
    char buf[MAX_STRING_LENGTH];
    bool uhoh = FALSE;
    CHAR_DATA *guard;
    ROOM_INDEX_DATA *room;
            
    	        if ( (ship = ship_from_cockpit(ch->in_room)) == NULL )  
    	        {
    	            send_to_char("&RYou must be in the cockpit of a ship to do that!\n\r",ch);
    	            return;
    	        }
    	        
    	        if ( ship->class > SPACE_STATION )
    	        {
    	            send_to_char("&RThis isn't a spacecraft!\n\r",ch);
    	            return;
    	        }
    	        
    	        if ( check_pilot( ch , ship ) )
    	        {
    	            send_to_char("&RWhat would be the point of that!\n\r",ch);
    	            return;
    	        }
    	        
    	        if ( ship->type == MOB_SHIP && get_trust(ch) < 102 )
    	        {
    	            send_to_char("&RThis ship isn't pilotable by mortals at this point in time...\n\r",ch);
    	            return;
    	        }
    	        
                if  ( ship->class == SPACE_STATION )
                {
                   send_to_char( "You can't do that here.\n\r" , ch );
                   return;
                }   
    
    	        if ( ship->lastdoc != ship->location )
                {
                     send_to_char("&rYou don't seem to be docked right now.\n\r",ch);
                     return;
                }
    
    	        if ( ship->shipstate != SHIP_DOCKED && ship->shipstate != SHIP_DISABLED )
    	        {
    	            send_to_char("The ship is not docked right now.\n\r",ch);
    	            return;
    	        }
                
                if ( ship->shipstate == SHIP_DISABLED )
    	        {
    	            send_to_char("The ships drive is disabled .\n\r",ch);
    	            return;
    	        }
                
                for ( room = ship->first_room ; room ; room = room->next_in_ship )
		{
		   for ( guard = room->first_person; guard ; guard = guard->next_in_room )
		      if ( IS_NPC(guard) && guard->pIndexData && guard->pIndexData->vnum == MOB_VNUM_SHIP_GUARD 
		      && guard->position > POS_SLEEPING && !guard->fighting )
                      {
                         start_hating( guard, ch );
                         start_hunting( guard , ch );
                         uhoh = TRUE;
                      }   
		}
		
                if ( uhoh )
    	        {
    	            send_to_char("Uh oh....\n\r",ch);
    	            return;
    	        }
		                
                chance = IS_NPC(ch) ? 0
	                 : (int)  (ch->pcdata->learned[gsn_hijack]) ;
                if ( number_percent( ) > chance )
    		{  
    		    send_to_char("You fail to figure out the correct launch code.\n\r",ch);
    	            return;
                }
                
                chance = IS_NPC(ch) ? 0
	                 : (int)  (ch->pcdata->learned[gsn_spacecraft]) ;
                if ( number_percent( ) < chance )
    		{  
                
    		   if (ship->hatchopen)
    		   {
    		     ship->hatchopen = FALSE;
    		     sprintf( buf , "The hatch on %s closes." , ship->name);  
       	             echo_to_room( AT_YELLOW , get_room_index(ship->location) , buf );
       	             echo_to_room( AT_YELLOW , ship->entrance , "The hatch slides shut." );
       	           }
    		   set_char_color( AT_GREEN, ch );
    		   send_to_char( "Launch sequence initiated.\n\r", ch);
    		   act( AT_PLAIN, "$n starts up the ship and begins the launch sequence.", ch,
		        NULL, argument , TO_ROOM );
		   echo_to_ship( AT_YELLOW , ship , "The ship hums as it lifts off the ground.");
    		   sprintf( buf, "%s begins to launch.", ship->name );
    		   echo_to_room( AT_YELLOW , get_room_index(ship->location) , buf );
    		   ship->shipstate = SHIP_LAUNCH;
    		   ship->currspeed = ship->realspeed;
                   learn_from_success( ch, gsn_spacecraft );
                   learn_from_success( ch, gsn_hijack );
                   sprintf( buf, "%s has been hijacked!", ship->name );
    		   echo_to_all( AT_RED , buf, 0 );
    		   
                   return;   	   	
                }
                set_char_color( AT_RED, ch );
	        send_to_char("You fail to work the controls properly!\n\r",ch);
    	   	return;	
    	
}


void do_propeganda ( CHAR_DATA *ch , char *argument )
{
    char buf  [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    PLANET_DATA *planet;
    CLAN_DATA   *clan;
    int percent;
    
   if ( IS_NPC(ch) || !ch->pcdata || !ch->pcdata->clan || !ch->in_room->area || !ch->in_room->area->planet )
   {
       send_to_char( "What would be the point of that.\n\r", ch );
       return;
   }
    
    argument = one_argument( argument, arg1 );

    if ( ch->mount )
    {
	send_to_char( "You can't do that while mounted.\n\r", ch );
	return;
    }

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Spread propeganda to who?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "That's pointless.\n\r", ch );
	return;
    }

    if ( !IS_SET( victim->act , ACT_CITIZEN ) )
    {
        send_to_char( "I don't think diplomacy will work on them...\n\r" , ch );
        return;    
    }
    
    if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
    {
	set_char_color( AT_MAGIC, ch );
	send_to_char( "This isn't a good place to do that.\n\r", ch );
	return;
    }

    if ( ch->position == POS_FIGHTING )
    {
        send_to_char( "Interesting combat technique.\n\r" , ch );
        return;
    }
    
    if ( victim->position == POS_FIGHTING )
    {
        send_to_char( "They're a little busy right now.\n\r" , ch );
        return;
    }
    

    if ( ch->position <= POS_SLEEPING )
    {
        send_to_char( "In your dreams or what?\n\r" , ch );
        return;
    }
    
    if ( victim->position <= POS_SLEEPING )
    {
        send_to_char( "You might want to wake them first...\n\r" , ch );
        return;
    }

    clan = ch->pcdata->clan;
       
    planet = ch->in_room->area->planet;
        
    sprintf( buf, ", and the evils of %s" , planet->governed_by ? planet->governed_by->name : "their current leaders" );
    ch_printf( ch, "You speak to them about the benifits of the %s%s.\n\r", ch->pcdata->clan->name,
        planet->governed_by == clan ? "" : buf );
    act( AT_ACTION, "$n speaks about his organization.\n\r", ch, NULL, victim, TO_VICT    );
    act( AT_ACTION, "$n tells $N about their organization.\n\r",  ch, NULL, victim, TO_NOTVICT );

    WAIT_STATE( ch, skill_table[gsn_propeganda]->beats );

    if ( percent - get_curr_cha(ch) + victim->top_level > ch->pcdata->learned[gsn_propeganda]  ) 
    {

        if ( planet->governed_by != clan )
	{
	  sprintf( buf, "%s is a traitor!" , ch->name);
	  do_yell( victim, buf );
          global_retcode = multi_hit( victim, ch, TYPE_UNDEFINED );
	}
	
	return;
    }
    
    if ( planet->governed_by == clan )
    { 
       planet->pop_support += 1;
       send_to_char( "Popular support for your organization increases.\n\r", ch );
    }     
    else
    {
       planet->pop_support -= .1;
       send_to_char( "Popular support for the current government decreases.\n\r", ch );
    }
    
    if ( number_percent() == 23 )
    {
	send_to_char( "You feel a bit more charming than you used to...\n\r", ch );
        ch->perm_cha++;
        ch->perm_cha = UMIN( ch->perm_cha , 25 );
    }

    learn_from_success( ch, gsn_propeganda );
        
    if ( planet->pop_support > 100 )
        planet->pop_support = 100;
    if ( planet->pop_support < -100 )
        planet->pop_support = -100;

}

void  clear_roomtype( ROOM_INDEX_DATA * location )
{
      if ( location->area && location->area->planet )
      {
          if ( location->sector_type <= SECT_CITY )
              location->area->planet->citysize--;
          else if ( location->sector_type == SECT_FARMLAND )
              location->area->planet->farmland--;
          else if ( location->sector_type != SECT_DUNNO )
              location->area->planet->wilderness--;

          if ( IS_SET( location->room_flags , ROOM_BARRACKS ) )
              location->area->planet->barracks--;
          if ( IS_SET( location->room_flags , ROOM_CONTROL ) )
              location->area->planet->controls--;
              
      }

      REMOVE_BIT( location->room_flags , ROOM_NO_MOB );
      REMOVE_BIT( location->room_flags , ROOM_HOTEL );
      REMOVE_BIT( location->room_flags , ROOM_SAFE );
      REMOVE_BIT( location->room_flags , ROOM_CAN_LAND );
      REMOVE_BIT( location->room_flags , ROOM_SHIPYARD );
      REMOVE_BIT( location->room_flags , ROOM_EMPTY_HOME );
      REMOVE_BIT( location->room_flags , ROOM_DARK );
      REMOVE_BIT( location->room_flags , ROOM_INFO );
      REMOVE_BIT( location->room_flags , ROOM_MAIL );
      REMOVE_BIT( location->room_flags , ROOM_TRADE );
      REMOVE_BIT( location->room_flags , ROOM_SUPPLY );
      REMOVE_BIT( location->room_flags , ROOM_PAWN );
      REMOVE_BIT( location->room_flags , ROOM_RESTAURANT );
      REMOVE_BIT( location->room_flags , ROOM_BAR );
      REMOVE_BIT( location->room_flags , ROOM_CONTROL );
      REMOVE_BIT( location->room_flags , ROOM_BARRACKS );
      REMOVE_BIT( location->room_flags , ROOM_GARAGE );
      REMOVE_BIT( location->room_flags , ROOM_BANK );
      REMOVE_BIT( location->room_flags , ROOM_EMPLOYMENT );
}

void do_landscape ( CHAR_DATA *ch , char *argument )
{
    CLAN_DATA * clan;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    ROOM_INDEX_DATA * location;
    int chance;
    char arg[MAX_INPUT_LENGTH];
    
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
		bug( "landscape: sub_room_desc: NULL ch->dest_buf", 0 );
		location = ch->in_room;
	  }
	  STRFREE( location->description );
	  location->description = copy_buffer( ch );
	  stop_editing( ch );
	  ch->substate = ch->tempnum;
          if ( strlen( location->description ) > 150 )
   	     learn_from_success( ch , gsn_landscape );
          else
          {
  	     send_to_char( "That rooms description is too short.\n\r", ch );
  	     send_to_char( "You skill level deminishes with your lazyness.\n\r", ch );
             if ( ch->pcdata->learned[gsn_landscape] > 50 )
                ch->pcdata->learned[gsn_landscape] -= 5;
          }   
          SET_BIT( location->area->flags , AFLAG_MODIFIED );
	  for ( obj = ch->in_room->first_content; obj; obj = obj_next )
	  { 
	    obj_next = obj->next_content;
	    extract_obj( obj );
	  }
          echo_to_room( AT_WHITE, location , "The construction crew finishes its work." );
	  return;
   }
     
   location = ch->in_room;
   clan = ch->pcdata->clan;

   if ( !clan )
   {
	send_to_char( "You need to be part of an organization before you can do that!\n\r", ch );
	return;         
   }

   if ( (ch->pcdata && ch->pcdata->bestowments
   &&    is_name("build", ch->pcdata->bestowments))
   || nifty_is_name( ch->name, clan->leaders  ) )
	;
   else
   {
	send_to_char( "Your organization hasn't given you permission to edit their lands!\n\r", ch );
	return;
   }

   if ( !location->area || !location->area->planet ||
   clan != location->area->planet->governed_by  )
   {
	send_to_char( "You may only landscape areas on planets that your organization controls!\n\r", ch );
	return;   
   }
  
   if ( IS_SET( location->room_flags , ROOM_NOPEDIT ) )
   {
	send_to_char( "Sorry, But you may not edit this room.\n\r", ch );
	return;   
   }
  
   argument = one_argument( argument, arg );

   if ( argument[0] == '\0' )
   {
	send_to_char( "Usage: LANDSCAPE  <Room Type>  <New Room Name>\n\r", ch );
	send_to_char( "<Room Type> may be one of the following:\n\r\n\r", ch );
	
	send_to_char( "wilderness   - the planets default terrain\n\r", ch );
	send_to_char( "farmland     - cleared farmland\n\r", ch );
	send_to_char( "city         - a city street\n\r", ch );	
	send_to_char( "platform     - ships land here\n\r", ch );
	send_to_char( "shipyard     - ships are built here\n\r", ch );
	send_to_char( "inside       - inside a building\n\r", ch );
	send_to_char( "house        - may be used as a players home\n\r", ch );
	send_to_char( "cave         - a mine or dug out tunnel\n\r", ch );
	send_to_char( "info         - message and information room\n\r", ch );
	send_to_char( "mail         - post office\n\r", ch );
	send_to_char( "hotel        - players can enter/leave game here\n\r", ch );
	send_to_char( "trade        - players can sell resources here\n\r", ch );
	send_to_char( "supply       - a supply store\n\r", ch );
	send_to_char( "pawn         - will trade useful items\n\r", ch );
	send_to_char( "restaurant   - food is bought here\n\r", ch );
	send_to_char( "bar          - liquor is sold here\n\r", ch );
	send_to_char( "control      - control tower for patrol ships\n\r", ch );
	send_to_char( "barracks     - houses military patrols\n\r", ch );
	send_to_char( "garage       - vehicles are built here\n\r", ch );
	send_to_char( "bank         - room is a bank\n\r", ch );
	send_to_char( "employment   - room is an employment office\n\r", ch );
	return;   
   }


   chance = (int) (ch->pcdata->learned[gsn_landscape]);
   if ( number_percent( ) > chance )
   {
	send_to_char( "You can't quite get the desired affect.\n\r", ch );
	return;   
   }
   
   clear_roomtype( location );
   
   if ( IS_SET( location->room_flags , ROOM_PLR_HOME ))
   {
      location->area->planet->citysize++;
      SET_BIT( location->room_flags , ROOM_NO_MOB );
      SET_BIT( location->room_flags , ROOM_HOTEL );
      SET_BIT( location->room_flags , ROOM_SAFE );
   }      
   else if ( !str_cmp( arg, "city" ) )
   {
      location->area->planet->citysize++;
      location->sector_type = SECT_CITY;
   }
   else if ( !str_cmp( arg, "wilderness" ) )
   {
      location->area->planet->wilderness++;
      location->sector_type = location->area->planet->sector;
   }
   else if ( !str_cmp( arg, "inside" ) )
   {
      location->area->planet->citysize++;
      location->sector_type = SECT_INSIDE;   
   }
   else if ( !str_cmp( arg, "farmland" ) )
   {
      location->area->planet->farmland++;
      location->sector_type = SECT_FARMLAND;
   }
   else if ( !str_cmp( arg, "platform" ) )
   {
      location->area->planet->citysize++;
      location->sector_type = SECT_CITY;
      SET_BIT( location->room_flags , ROOM_CAN_LAND );
   }
   else if ( !str_cmp( arg, "shipyard" ) )
   {
      location->area->planet->citysize++;
      location->sector_type = SECT_CITY;
      SET_BIT( location->room_flags , ROOM_SHIPYARD );
   }
   else if ( !str_cmp( arg, "house" ) )
   {
      location->area->planet->citysize++;
      location->sector_type = SECT_INSIDE;
      SET_BIT( location->room_flags , ROOM_EMPTY_HOME );
      SET_BIT( location->room_flags , ROOM_NO_MOB );
   }
   else if ( !str_cmp( arg, "cave" ) )
   {
      location->area->planet->wilderness++;
      location->sector_type = SECT_UNDERGROUND;
      SET_BIT( location->room_flags , ROOM_DARK );
   }
   else if ( !str_cmp( arg, "info" ) )
   {
      location->area->planet->citysize++;
      location->sector_type = SECT_INSIDE;
      SET_BIT( location->room_flags , ROOM_INFO );
      SET_BIT( location->room_flags , ROOM_NO_MOB );
   }
   else if ( !str_cmp( arg, "mail" ) )
   {
      location->area->planet->citysize++;
      location->sector_type = SECT_INSIDE;
      SET_BIT( location->room_flags , ROOM_MAIL );
      SET_BIT( location->room_flags , ROOM_NO_MOB );
   }
   else if ( !str_cmp( arg, "bank" ) )
   {
      location->area->planet->citysize++;
      location->sector_type = SECT_INSIDE;
      SET_BIT( location->room_flags , ROOM_BANK );
   }
   else if ( !str_cmp( arg, "hotel" ) )
   {
      location->area->planet->citysize++;
      location->sector_type = SECT_INSIDE;
      SET_BIT( location->room_flags , ROOM_HOTEL );
      SET_BIT( location->room_flags , ROOM_SAFE );
      SET_BIT( location->room_flags , ROOM_NO_MOB );
   }
   else if ( !str_cmp( arg, "trade" ) )
   {
      location->area->planet->citysize++;
      location->sector_type = SECT_INSIDE;
      SET_BIT( location->room_flags , ROOM_SAFE );
      SET_BIT( location->room_flags , ROOM_TRADE );
   }
   else if ( !str_cmp( arg, "supply" ) )
   {
      location->area->planet->citysize++;
      location->sector_type = SECT_INSIDE;
      SET_BIT( location->room_flags , ROOM_SAFE );
      SET_BIT( location->room_flags , ROOM_SUPPLY );
   }
   else if ( !str_cmp( arg, "pawn" ) )
   {
      location->area->planet->citysize++;
      location->sector_type = SECT_INSIDE;
      SET_BIT( location->room_flags , ROOM_SAFE );
      SET_BIT( location->room_flags , ROOM_PAWN );
   }
   else if ( !str_cmp( arg, "restaurant" ) )
   {
      location->area->planet->citysize++;
      location->sector_type = SECT_INSIDE;
      SET_BIT( location->room_flags , ROOM_RESTAURANT );
   }
   else if ( !str_cmp( arg, "bar" ) ) 
   {
      location->area->planet->citysize++;
      location->sector_type = SECT_INSIDE;
      SET_BIT( location->room_flags , ROOM_BAR );
   }
   else if ( !str_cmp( arg, "control" ) )
   {
      location->area->planet->controls++;
      location->area->planet->citysize++;
      location->sector_type = SECT_INSIDE;
      SET_BIT( location->room_flags , ROOM_CONTROL );
   }
   else if ( !str_cmp( arg, "barracks" ) )
   {
      location->area->planet->citysize++;
      location->area->planet->barracks++;
      location->sector_type = SECT_INSIDE;
      SET_BIT( location->room_flags , ROOM_BARRACKS );
   }
   else if ( !str_cmp( arg, "garage" ) )
   {
      location->area->planet->citysize++;
      location->sector_type = SECT_INSIDE;
      SET_BIT( location->room_flags , ROOM_GARAGE );
   }
   else if ( !str_cmp( arg, "employment" ) )
   {
      location->area->planet->citysize++;
      location->sector_type = SECT_INSIDE;
      SET_BIT( location->room_flags , ROOM_EMPLOYMENT );
   }
   else
   {
    	do_landscape( ch, "" );
	return;   
    }

    echo_to_room( AT_WHITE, location, "A construction crew enters the room and begins to work." );
    
    STRFREE ( location->name );
    location->name = STRALLOC( argument );
    
    ch->substate = SUB_ROOM_DESC;
    ch->dest_buf = location;
    start_editing( ch, location->description );
    return;
   
}

void do_construction ( CHAR_DATA *ch , char *argument )
{
    CLAN_DATA * clan;
    int chance, ll;
    EXIT_DATA * xit;
    EXIT_DATA * xit2;
    int edir;
    ROOM_INDEX_DATA *nRoom;
    char buf[MAX_STRING_LENGTH];
             
    if ( IS_NPC(ch) || !ch->pcdata || !ch->in_room )
    	return;
    
    clan = ch->pcdata->clan;

    if ( !clan )
    {
	send_to_char( "You need to be part of an organization before you can do that!\n\r", ch );
	return;         
    }

    if ( (ch->pcdata && ch->pcdata->bestowments
    &&    is_name("build", ch->pcdata->bestowments))
    || nifty_is_name( ch->name, clan->leaders  ) )
	;
    else
    {
	send_to_char( "Your organization hasn't given you permission to build on their lands!\n\r", ch );
	return;
    }

   if ( !ch->in_room->area || !ch->in_room->area->planet ||
   clan != ch->in_room->area->planet->governed_by      )
   {
	send_to_char( "You may only build on planets that your organization controls!\n\r", ch );
	return;   
   }

   if ( ch->in_room->area->planet->size >= 2000 )
   {
	send_to_char( "This planet is big enough. Go build somewhere else...\n\r", ch );
	return;   
   }

   if ( IS_SET( ch->in_room->room_flags , ROOM_NOPEDIT ) )
   {
	   send_to_char( "Sorry, But you may not edit this room.\n\r", ch );
	   return;   
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Begin construction in what direction?\n\r", ch );
	return;         
    }

   if ( ch->gold < 500 )
   {
	send_to_char( "You do not have enough money. It will cost you 500 credits to do that.\n\r", ch );
	return;   
   }

   edir = get_dir(argument);
   xit = get_exit(ch->in_room, edir);
   if ( xit )
   {
	send_to_char( "There is already a room in that direction.\n\r", ch );
	return;   
   }
   	
   chance = (int) (ch->pcdata->learned[gsn_construction]);
   if ( number_percent( ) > chance )
   {
	send_to_char( "You can't quite get the desired affect.\n\r", ch );
        ch->gold -= 5;
	return;   
   }

   ch->gold -= 500;

   nRoom = make_room( ++top_r_vnum );
   nRoom->area = ch->in_room->area;
   LINK( nRoom , ch->in_room->area->first_room , ch->in_room->area->last_room , next_in_area , prev_in_area );
   STRFREE( nRoom->name );
   STRFREE( nRoom->description );
   nRoom->name = STRALLOC( "Construction Site" );
   nRoom->description = STRALLOC ( "\n\rThis area is under construction.\n\rIt still needs some landscaping.\n\r\n\r" );
   nRoom->sector_type = SECT_DUNNO;
   SET_BIT( nRoom->room_flags , ROOM_NO_MOB );
   
   xit = make_exit( ch->in_room, nRoom, edir );
   xit->keyword		= STRALLOC( "" );
   xit->description	= STRALLOC( "" );
   xit->key		= -1;
   xit->exit_info	= 0;

   xit2 = make_exit( nRoom , ch->in_room  , rev_dir[edir] );
   xit2->keyword		= STRALLOC( "" );
   xit2->description	= STRALLOC( "" );
   xit2->key		= -1;
   xit2->exit_info	= 0;

   ch->in_room->area->planet->size++;

   for ( ll = 1 ; ll <= 20 ; ll++ )
       learn_from_success( ch , gsn_construction );
    
   SET_BIT( ch->in_room->area->flags , AFLAG_MODIFIED );
   
   sprintf( buf , "A construction crew begins working on a new area to the %s." , dir_name[edir] );
   echo_to_room( AT_WHITE, ch->in_room, buf );
   
}

void do_bridge ( CHAR_DATA *ch , char *argument )
{
    CLAN_DATA * clan;
    int chance, ll;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    EXIT_DATA   *xit, *texit;
    int   evnum, edir, ekey;
    ROOM_INDEX_DATA *toroom;
    char buf[MAX_STRING_LENGTH];
                        
    if ( IS_NPC(ch) || !ch->pcdata || !ch->in_room )
    	return;
    
    clan = ch->pcdata->clan;

    if ( !clan )
    {
	send_to_char( "You need to be part of an organization before you can do that!\n\r", ch );
	return;         
    }

    if ( (ch->pcdata && ch->pcdata->bestowments
    &&    is_name("build", ch->pcdata->bestowments))
    || nifty_is_name( ch->name, clan->leaders  ) )
	;
    else
    {
	send_to_char( "Your organization hasn't given you permission to build on their lands!\n\r", ch );
	return;
    }

   if ( !ch->in_room->area || !ch->in_room->area->planet ||
   clan != ch->in_room->area->planet->governed_by      )
   {
	send_to_char( "You may only build on planets that your organization controls!\n\r", ch );
	return;   
   }

   if ( IS_SET( ch->in_room->room_flags , ROOM_NOPEDIT ) )
   {
	   send_to_char( "Sorry, But you may not edit this room.\n\r", ch );
	   return;   
    }

   if ( ch->gold < 500 )
   {
	send_to_char( "You do not have enough money. It will cost you 500 credits to do that.\n\r", ch );
	return;   
   }
   	
   argument = one_argument( argument , arg1 );
   if ( argument[0] == '\0' )
   {
	send_to_char( "USAGE: bridge <direction> <action> <argument>\n\r", ch );
	send_to_char( "\n\rAction being one of the following:\n\r", ch );
	send_to_char( "connect, door, keycode\n\r", ch );
	return;      
   }
   argument = one_argument( argument , arg2 );

   chance = (int) (ch->pcdata->learned[gsn_bridge]);
   if ( number_percent( ) > chance )
   {
	send_to_char( "You can't quite get the desired affect.\n\r", ch );
        ch->gold -= 10;
	return;   
   }

   edir = get_dir(arg1);
   xit = get_exit(ch->in_room, edir);
   
   if ( !str_cmp( arg2 , "connect" ) )
   {
       if ( xit )
       {
 	  send_to_char( "There's already an exit in that direction.\n\r", ch );
	  return;      
       }            
       evnum = atoi( argument );
       if ( (toroom = get_room_index( evnum )) == NULL )
       {
            ch_printf( ch, "Non-existant room: %d\n\r", evnum );
            return;
       }
       if ( ch->in_room->area != toroom->area )
       {
            ch_printf( ch, "Nice try. Room %d isn't even on the same planet!\n\r" , evnum );
            return;
       }
       if ( IS_SET(toroom->room_flags, ROOM_NOPEDIT ) )
       {
            ch_printf( ch, "Room %d isn't editable by players!\n\r" , evnum );
            return;
       }
       texit = get_exit( toroom, rev_dir[edir] );
       if ( texit )
       {
            ch_printf( ch, "Room %d already has an entrance from that direction!\n\r" , evnum );
            return;
       }
       
      xit = make_exit( ch->in_room, toroom, edir );
      xit->keyword		= STRALLOC( "" );
      xit->description	= STRALLOC( "" );
      xit->key		= -1;
      xit->exit_info	= 0;
      texit = make_exit( toroom , ch->in_room  , rev_dir[edir] );
      texit->keyword		= STRALLOC( "" );
      texit->description	= STRALLOC( "" );
      texit->key		= -1;
      texit->exit_info	= 0;
      
      sprintf( buf , "A construction crew opens up a passage to the %s." , dir_name[edir] );
      echo_to_room( AT_WHITE, ch->in_room, buf );
      sprintf( buf , "A construction crew opens up a passage from the %s." , dir_name[rev_dir[edir]] );
      echo_to_room( AT_WHITE, toroom , buf );                                                       
   }
   else if ( !str_cmp( arg2 , "keycode" ) )
   {
       if ( !xit )
       {
 	  send_to_char( "There's no exit in that direction.\n\r", ch );
	  return;      
       }            

       if ( !IS_SET( xit->exit_info , EX_ISDOOR ) )
       {
 	  send_to_char( "There's no door in that direction.\n\r", ch );
	  return;      
       }            
       
       ekey = atoi( argument );
       ch_printf( ch , "Ok the lock code is now: %d" , ekey );
       xit->key = ekey;
   
   }
   else if ( !str_cmp( arg2 , "door" ) )
   {
       if ( !xit )
       {
 	  send_to_char( "There's no exit in that direction.\n\r", ch );
	  return;      
       }            

       if ( !IS_SET( xit->exit_info , EX_ISDOOR ) )
       {
          sprintf( buf , "A construction crew builds a door to the %s." , dir_name[edir] );
          echo_to_room( AT_WHITE, ch->in_room, buf );
          SET_BIT(  xit->exit_info , EX_ISDOOR );       
          texit = get_exit_to( xit->to_room, rev_dir[edir], ch->in_room->vnum );
          if ( texit )
          {
             sprintf( buf , "A construction crew builds a door to the %s." , dir_name[rev_dir[edir]] );
             echo_to_room( AT_WHITE, xit->to_room, buf );
             SET_BIT(  texit->exit_info , EX_ISDOOR );
          }
       }
       else
       {
          sprintf( buf , "A construction crew removes the door to the %s." , dir_name[edir] );
          echo_to_room( AT_WHITE, ch->in_room, buf );
          REMOVE_BIT(  xit->exit_info , EX_ISDOOR );       
          texit = get_exit_to( xit->to_room, rev_dir[edir], ch->in_room->vnum );
          if ( texit )
          {
             sprintf( buf , "A construction crew removes the door to the %s." , dir_name[rev_dir[edir]] );
             echo_to_room( AT_WHITE, xit->to_room, buf );
             REMOVE_BIT(  texit->exit_info , EX_ISDOOR );
          }
       }
       
   }
   else
   {
        do_bridge( ch , "" );
        return;
   }
   
   ch->gold -= 500;

   for ( ll = 1 ; ll <= 20 ; ll++ )
       learn_from_success( ch , gsn_bridge );
   
   SET_BIT( ch->in_room->area->flags , AFLAG_MODIFIED );
   
}



void do_survey ( CHAR_DATA *ch , char *argument )
{
    ROOM_INDEX_DATA * room;
    int chance;
    
    if ( IS_NPC(ch) || !ch->pcdata || !ch->in_room )
    	return;
    
    room = ch->in_room;
    
   	chance = (int) (ch->pcdata->learned[gsn_survey]);
   	if ( number_percent( ) > chance )
   	{
		send_to_char( "You have a hard time surveying this region.\n\r", ch );
		return;   
   	}

       ch_printf( ch, "&Y%s\n\r\n\r", room->name );
       ch_printf( ch, "&WIndex:&Y %d\n\r", room->vnum );
       if ( room->area && room->area->planet )
            ch_printf( ch, "&WPlanet:&Y %s\n\r", room->area->planet->name );
       ch_printf( ch, "&WSize:&Y %d\n\r", room->tunnel );
       ch_printf( ch, "&WSector:&Y %s\n\r", sector_name[room->sector_type] );
       send_to_char( "&WInfo:\n\r", ch );
       
       if ( IS_SET( room->room_flags , ROOM_DARK) )
       	   ch_printf( ch, "&Y   Room is always dark.\n\r" );
       
       if ( IS_SET( room->room_flags , ROOM_INDOORS) )
       	   ch_printf( ch, "&Y   Room is indoors.\n\r" );
       
       if ( IS_SET( room->room_flags , ROOM_SHIPYARD) )
       	   ch_printf( ch, "   &YSpacecraft can be built or purchased here.\n\r" );

       if ( IS_SET( room->room_flags , ROOM_CAN_LAND) )
       	   ch_printf( ch, "&Y   Spacecraft can land here.\n\r" );
       
       if ( IS_SET( room->room_flags , ROOM_EMPLOYMENT) )
       	   ch_printf( ch, "&Y   You can find temporary employment here.\n\r" );

       if ( IS_SET( room->room_flags , ROOM_BANK) )
       	   ch_printf( ch, "&Y   This room may be used as a bank.\n\r" );
       
       if ( IS_SET( room->room_flags , ROOM_SAFE) )
       	   ch_printf( ch, "&Y   Combat cannot take place in this room.\n\r" );
       
       if ( IS_SET( room->room_flags , ROOM_HOTEL) )
       	   ch_printf( ch, "&Y   Players may quit and enter the game from here.\n\r" );
       
       if ( IS_SET( room->room_flags , ROOM_EMPTY_HOME ) )
       	   ch_printf( ch, "&Y   This room may be purchased for use as a players home / storage locker.\n\r" );

       if ( IS_SET( room->room_flags , ROOM_PLR_HOME ) )
       	   ch_printf( ch, "&Y   This room is a players private home.\n\r" );

       if ( IS_SET( room->room_flags , ROOM_MAIL ) )
       	   ch_printf( ch, "&Y   This is a post office.\n\r" );

       if ( IS_SET( room->room_flags , ROOM_INFO ) )
       	   ch_printf( ch, "&Y   A message and information terminal may be installed here.\n\r" );

       if ( IS_SET( room->room_flags , ROOM_TRADE ) )
       	   ch_printf( ch, "&Y   This room is used for resource trade.\n\r" );

       if ( IS_SET( room->room_flags , ROOM_SUPPLY ) )
       	   ch_printf( ch, "&Y   This is a supply store.\n\r" );

       if ( IS_SET( room->room_flags , ROOM_PAWN ) )
       	   ch_printf( ch, "&Y   You can buy and sell useful items here.\n\r" );

       if ( IS_SET( room->room_flags , ROOM_RESTAURANT ) )
       	   ch_printf( ch, "&Y   This room is a restaurant.\n\r" );

       if ( IS_SET( room->room_flags , ROOM_BARRACKS ) )
       	   ch_printf( ch, "&Y   This is a military barracks.\n\r" );

       if ( IS_SET( room->room_flags , ROOM_GARAGE ) )
       	   ch_printf( ch, "&Y   Vehicles are built and sold here.\n\r" );

       if ( IS_SET( room->room_flags , ROOM_CONTROL ) )
       	   ch_printf( ch, "&Y   This is a control tower for patrol spacecraft.\n\r" );

       if ( IS_SET( room->room_flags , ROOM_BAR ) )
       	   ch_printf( ch, "&Y   This is bar.\n\r" );

       if ( IS_SET( room->room_flags , ROOM_NOPEDIT ) )
       	   ch_printf( ch, "&WThis room is NOT player editable.\n\r" );
       else	  
       	   ch_printf( ch, "&WThis room IS editable by players.\n\r" );
       
       learn_from_success( ch , gsn_survey );
       return;   
    
}

 
void do_quicktalk ( CHAR_DATA *ch , char *argument )
{
    CHAR_DATA *rch;
    int chance;

    if ( ch->position != POS_FIGHTING )
    {
         send_to_char( "You can't talk your way out of a fight that hasn't started yet...\n\r",ch );
         return;
    }
    
    act( AT_ACTION, "$n attempts to stop the fight.", ch, NULL, NULL, TO_ROOM );

    chance = (int) (ch->pcdata->learned[gsn_quicktalk]);
    chance = UMIN( chance , 95 );
    if ( number_percent( ) > chance )
    {
	send_to_char( "You fail to calm your attackers.\n\r", ch );
	return;   
    }

    for ( rch = ch->in_room->first_person ; rch; rch = rch->next_in_room )
    {
	if ( rch->fighting )
	    stop_fighting( rch, TRUE );
       
        stop_hating( rch );
        stop_hunting( rch );
        stop_fearing( rch );
    }

    learn_from_success( ch , gsn_quicktalk );

    send_to_char( "You successfully talk your way out of a sticky situation.\n\r", ch );
    return;

}