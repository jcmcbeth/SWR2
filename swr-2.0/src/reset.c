#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

ROOM_INDEX_DATA *       room_index_hash         [MAX_KEY_HASH];

SHIP_DATA * make_mob_ship( PLANET_DATA *planet , int model );
void    resetship args( ( SHIP_DATA *ship ) );

void do_reset( CHAR_DATA * ch , char * argument )
{
    reset_all();
}

/*
 * Reset everything.
 */
void reset_all( )
{

   ROOM_INDEX_DATA *pRoomIndex;
   int iHash;
   CHAR_DATA * mob = NULL;
   MOB_INDEX_DATA *pMobIndex = NULL;
   OBJ_INDEX_DATA *pObjIndex = NULL;
   OBJ_DATA * obj = NULL;
   EXIT_DATA *xit;
   bool found = FALSE;
   int vnum, anumber, onum;

   /* natural disasters */
   {
       PLANET_DATA * dPlanet = NULL;
       int pCount = 0;
       int rCount;
       CLAN_DATA * dClan = NULL;
       DESCRIPTOR_DATA *d = NULL;
       
       for ( dPlanet = first_planet ; dPlanet ; dPlanet = dPlanet->next )
           pCount++;
           
       rCount = number_range( 1 , pCount );
       
       pCount = 0;
       
       for ( dPlanet = first_planet ; dPlanet ; dPlanet = dPlanet->next )
           if ( ++pCount == rCount )
               break;
       
       if ( dPlanet && dPlanet->area && dPlanet->governed_by )
           dClan = dPlanet->governed_by;
           
       if ( dClan )
          for ( d = first_descriptor ; d ; d = d->next )
              if ( d->connected == CON_PLAYING && !d->original && 
              d->character && d->character->pcdata && d->character->pcdata->clan 
              && d->character->pcdata->clan == dPlanet->governed_by )
                      break;
                      
       if ( d )
       {
         switch ( number_bits( 2 ) )
         {
         case 0:
         
            for ( pRoomIndex = dPlanet->area->first_room ; pRoomIndex ; pRoomIndex = pRoomIndex->next_in_area )
              if ( pRoomIndex->sector_type == SECT_CITY 
              && !IS_SET( pRoomIndex->room_flags , ROOM_NO_MOB ) )
                 break;
            if( pRoomIndex )
            {
               int mCount;
               char dBuf[MAX_STRING_LENGTH];
               
               if ( (pMobIndex = get_mob_index(MOB_VNUM_ALIEN)) )
               {
                sprintf( dBuf , "(GNET) A Colonist: Help %s is being invaded by alien forces" , dPlanet->name );    
                echo_to_all( AT_LBLUE , dBuf , ECHOTAR_ALL );
                for( mCount = 0 ; mCount < 20 ; mCount++ )
                {
                  mob = create_mobile( pMobIndex );
                  char_to_room( mob, pRoomIndex );
                  mob->hit = 100;
                  mob->max_hit = 100;
                  if ( room_is_dark(pRoomIndex) )
                   SET_BIT(mob->affected_by, AFF_INFRARED);
                  if ( ( pObjIndex = get_obj_index( OBJ_VNUM_BLASTER ) ) != NULL )
                  {
                     obj = create_object( pObjIndex, mob->top_level );
                     obj_to_char( obj, mob );
                     equip_char( mob, obj, WEAR_WIELD );                        
                  } 
                  do_setblaster( mob , "full" );
                }
               }
              
            }
            break;
         
         default:
            break;
         }
         
       }  
   }
      
   for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
   {    
     for ( pRoomIndex  = room_index_hash[iHash]; pRoomIndex; pRoomIndex  = pRoomIndex->next )
     {    
        vnum = 0;
        onum = 0;
        
        for ( xit = pRoomIndex->first_exit ; xit ; xit = xit->next )
             if ( IS_SET(xit->exit_info , EX_ISDOOR ) )
             {
                 SET_BIT( xit->exit_info , EX_CLOSED );
                 if ( xit->key >= 0 )
                     SET_BIT( xit->exit_info , EX_LOCKED );
             }
        
        
        if ( IS_SET(pRoomIndex->room_flags, ROOM_CONTROL ) 
        && pRoomIndex->area && pRoomIndex->area->planet  && pRoomIndex->area->planet->starsystem 
        && pRoomIndex->area->planet->governed_by )
        {
           SPACE_DATA * system =  pRoomIndex->area->planet->starsystem;
           SHIP_DATA * ship;
           int numpatrols = 0;
           int numdestroyers = 0;
           int numbattleships = 0;
           int numcruisers = 0;
           int fleetsize = 0;

           for ( ship = system->first_ship ; ship ; ship = ship->next_in_starsystem )
               if ( !str_cmp( ship->owner , pRoomIndex->area->planet->governed_by->name )
               && ship->type == MOB_SHIP )
               {
                  if ( ship->model == MOB_DESTROYER )
                     numdestroyers++;
                  else if ( ship->model == MOB_CRUISER )
                     numcruisers++;
                  else if ( ship->model == MOB_BATTLESHIP )
                     numbattleships++;
                  else 
                     numpatrols++;
               }
                      
           fleetsize = 100*numbattleships + 25*numcruisers + 5*numdestroyers + numpatrols;
                  
           if ( fleetsize + 100 < pRoomIndex->area->planet->controls )
              make_mob_ship( pRoomIndex->area->planet , MOB_BATTLESHIP );
           else if ( fleetsize + 25 < pRoomIndex->area->planet->controls
           && numcruisers < 5 )
              make_mob_ship( pRoomIndex->area->planet , MOB_CRUISER );
           else if ( fleetsize + 5 < pRoomIndex->area->planet->controls
           && numdestroyers < 5 )
              make_mob_ship( pRoomIndex->area->planet , MOB_DESTROYER );
           else  if ( fleetsize < pRoomIndex->area->planet->controls
           && numpatrols < 5 )
              make_mob_ship( pRoomIndex->area->planet , MOB_PATROL );
        }
        
        if ( IS_SET(pRoomIndex->room_flags, ROOM_TRADE ) )
             vnum = MOB_VNUM_TRADER;
        if ( IS_SET(pRoomIndex->room_flags, ROOM_SUPPLY ) )
             vnum = MOB_VNUM_SUPPLIER;
        if ( IS_SET(pRoomIndex->room_flags, ROOM_PAWN ) )
             vnum = MOB_VNUM_PAWNER;
        if ( IS_SET(pRoomIndex->room_flags, ROOM_RESTAURANT ) )
             vnum = MOB_VNUM_WAITER;
        if ( IS_SET(pRoomIndex->room_flags, ROOM_GARAGE ) )
             vnum = MOB_VNUM_MECHANIC;
        if ( IS_SET(pRoomIndex->room_flags, ROOM_CONTROL ) )
             vnum = MOB_VNUM_CONTROLLER;
        if ( IS_SET(pRoomIndex->room_flags, ROOM_BAR ) )
             vnum = MOB_VNUM_BARTENDER;
        if ( IS_SET(pRoomIndex->room_flags, ROOM_SHIPYARD ) )
             vnum = MOB_VNUM_TECHNICIAN;
        if ( IS_SET(pRoomIndex->room_flags, ROOM_EMPLOYMENT ) )
             vnum = MOB_VNUM_JOB_OFFICER;
        
        if ( vnum > 0 )
        {
            found = FALSE;

	    for ( mob = pRoomIndex->first_person; mob ;mob = mob->next_in_room )
	    {
	        if (  IS_NPC( mob ) && mob->pIndexData && mob->pIndexData->vnum == vnum )
		    found = TRUE;
            }
            
            if ( !found  )
            {
            
                if ( !(pMobIndex = get_mob_index(vnum)) )
                {
        	   bug( "Reset_all: Missing mob (%d)", vnum );
        	   return;
      		}
                mob = create_mobile(pMobIndex);
                SET_BIT ( mob->act , ACT_CITIZEN );           
                if ( room_is_dark(pRoomIndex) )
                   SET_BIT(mob->affected_by, AFF_INFRARED);
                char_to_room(mob, pRoomIndex);
      		if ( pRoomIndex->area && pRoomIndex->area->planet )
                     pRoomIndex->area->planet->population++;
	    	if ( ( IS_SET(pRoomIndex->room_flags, ROOM_NOPEDIT) && vnum == MOB_VNUM_TRADER ) 
	    	||  vnum == MOB_VNUM_SUPPLIER )
	        {
      		   if ( vnum != MOB_VNUM_SUPPLIER || number_bits(1) == 0 ) 
      		   {
      		     if ( !(pObjIndex = get_obj_index(OBJ_VNUM_LIGHT)) )
      			{
        		   bug( "Reset_all: Missing default light (%d)", OBJ_VNUM_LIGHT );
      			   return;
      			}
                     obj = create_object(pObjIndex, 1);
                     SET_BIT(obj->extra_flags, ITEM_INVENTORY);
                     obj = obj_to_char(obj, mob);
                   }
      		   if ( vnum != MOB_VNUM_SUPPLIER || number_bits(1) == 0 ) 
      		   {
      		     if ( !(pObjIndex = get_obj_index(OBJ_VNUM_COMLINK)) )
      			{
        		   bug( "Reset_all: Missing default comlink (%d)", OBJ_VNUM_COMLINK );
      			   return;
      			}
                     obj = create_object(pObjIndex, 1);
                     SET_BIT(obj->extra_flags, ITEM_INVENTORY);
                     obj = obj_to_char(obj, mob);
                   }
                   if ( vnum != MOB_VNUM_SUPPLIER || number_bits(1) == 0 ) 
      		   {
        	     if ( !(pObjIndex = get_obj_index(OBJ_VNUM_CANTEEN)) )
      			{
        		   bug( "Reset_all: Missing default canteen (%d)", OBJ_VNUM_CANTEEN );
      			   return;
      			}
                     obj = create_object(pObjIndex, 1);
                     SET_BIT(obj->extra_flags, ITEM_INVENTORY);
                     obj = obj_to_char(obj, mob);
                   }
      		   if ( vnum != MOB_VNUM_SUPPLIER || number_bits(1) == 0 ) 
      		   {
                     if ( !(pObjIndex = get_obj_index(OBJ_VNUM_SHOVEL)) )
      			{
        		   bug( "Reset_all: Missing default shovel (%d)", OBJ_VNUM_SHOVEL );
      			   return;
      			}
                     obj = create_object(pObjIndex, 1);
                     SET_BIT(obj->extra_flags, ITEM_INVENTORY);
                     obj = obj_to_char(obj, mob);
                   }
                }
                
                if ( vnum == MOB_VNUM_SUPPLIER )   
	        {
      		   if ( number_bits(1) == 0 ) 
      		   {
                     if ( (pObjIndex = get_obj_index(OBJ_VNUM_BATTERY)) )
                     {
                        obj = create_object(pObjIndex, 1);
                        SET_BIT(obj->extra_flags, ITEM_INVENTORY);
                        obj = obj_to_char(obj, mob);
                     }
                   }
      		   if ( number_bits(1) == 0 ) 
      		   {
                     if ( !(pObjIndex = get_obj_index(OBJ_VNUM_BACKPACK)) )
                     {
                       obj = create_object(pObjIndex, 1);
                       SET_BIT(obj->extra_flags, ITEM_INVENTORY);
                       obj = obj_to_char(obj, mob);
                     }
                   }
      		   if ( number_bits(1) == 0 ) 
      		   {
                     if ( (pObjIndex = get_obj_index(OBJ_VNUM_AMMO)) )
                     {
                        obj = create_object(pObjIndex, 1);
                        SET_BIT(obj->extra_flags, ITEM_INVENTORY);
                        obj = obj_to_char(obj, mob);	        
                     }
                   }
      		   if ( number_bits(1) == 0 ) 
      		   {
                     if ( (pObjIndex = get_obj_index(OBJ_VNUM_SCHOOL_DAGGER)) )
                     {
                       obj = create_object(pObjIndex, 1);
                       SET_BIT(obj->extra_flags, ITEM_INVENTORY);
                       obj = obj_to_char(obj, mob);
	             }
	           }
      		   if ( number_bits(1) == 0 ) 
      		   {
                     if ( (pObjIndex = get_obj_index(OBJ_VNUM_BLASTER)) )
                     {
                       obj = create_object(pObjIndex, 1);
                       SET_BIT(obj->extra_flags, ITEM_INVENTORY);
                       obj = obj_to_char(obj, mob);
	             }
	           }
	           
	           onum = number_range( OBJ_VNUM_FIRST_PART ,OBJ_VNUM_LAST_PART  );
                   if ( (pObjIndex = get_obj_index(onum)) )
                   {
                     obj = create_object(pObjIndex, 1);
                     obj = obj_to_char(obj, mob);
                     SET_BIT(obj->extra_flags, ITEM_INVENTORY);
	           }
	        }
	    
	    
	       if ( vnum == MOB_VNUM_WAITER )   
	       {
                    if ( (pObjIndex = get_obj_index(OBJ_VNUM_APPETIZER)) )
                    {
                        obj = create_object(pObjIndex, 1);
                        SET_BIT(obj->extra_flags, ITEM_INVENTORY);
                        obj = obj_to_char(obj, mob);
                    }
                    if ( (pObjIndex = get_obj_index(OBJ_VNUM_SALAD)) )
                    {
                        obj = create_object(pObjIndex, 1);
                        SET_BIT(obj->extra_flags, ITEM_INVENTORY);
                        obj = obj_to_char(obj, mob);
                    }
                    if ( (pObjIndex = get_obj_index(OBJ_VNUM_LUNCH)) )
                    {
                        obj = create_object(pObjIndex, 1);
                        SET_BIT(obj->extra_flags, ITEM_INVENTORY);
                        obj = obj_to_char(obj, mob);
                    }
                    if ( (pObjIndex = get_obj_index(OBJ_VNUM_DINNER)) )
                    {
                        obj = create_object(pObjIndex, 1);
                        SET_BIT(obj->extra_flags, ITEM_INVENTORY);
                        obj = obj_to_char(obj, mob);
                    }
                    if ( (pObjIndex = get_obj_index(OBJ_VNUM_GLASSOFWATER)) )
                    {
                        obj = create_object(pObjIndex, 1);
                        SET_BIT(obj->extra_flags, ITEM_INVENTORY);
                        obj = obj_to_char(obj, mob);
                    }
                    if ( (pObjIndex = get_obj_index(OBJ_VNUM_COFFEE)) )
                    {
                        obj = create_object(pObjIndex, 1);
                        SET_BIT(obj->extra_flags, ITEM_INVENTORY);
                        obj = obj_to_char(obj, mob);
                    }
	       }

	       if ( vnum == MOB_VNUM_BARTENDER )   
	       {
                    if ( (pObjIndex = get_obj_index(OBJ_VNUM_BEER)) )
                    {
                        obj = create_object(pObjIndex, 1);
                        SET_BIT(obj->extra_flags, ITEM_INVENTORY);
                        obj = obj_to_char(obj, mob);
                    }
                    if ( (pObjIndex = get_obj_index(OBJ_VNUM_WHISKEY)) )
                    {
                        obj = create_object(pObjIndex, 1);
                        SET_BIT(obj->extra_flags, ITEM_INVENTORY);
                        obj = obj_to_char(obj, mob);
                    }
                    if ( (pObjIndex = get_obj_index(OBJ_VNUM_GLASSOFWATER)) )
                    {
                        obj = create_object(pObjIndex, 1);
                        SET_BIT(obj->extra_flags, ITEM_INVENTORY);
                        obj = obj_to_char(obj, mob);
                    }
                    if ( (pObjIndex = get_obj_index(OBJ_VNUM_COFFEE)) )
                    {
                        obj = create_object(pObjIndex, 1);
                        SET_BIT(obj->extra_flags, ITEM_INVENTORY);
                        obj = obj_to_char(obj, mob);
                    }
	       }
	    }
	    
	}
        
        if ( IS_SET( pRoomIndex->room_flags, ROOM_MAIL ) )
        {
   	   if ( !(pObjIndex = get_obj_index(OBJ_VNUM_MAIL_TERMINAL)) )
   	   {
             bug( "Reset_all: Missing mail terminal (%d)", OBJ_VNUM_MAIL_TERMINAL );
             return;
   	   }
   	   if ( count_obj_list(pObjIndex, pRoomIndex->first_content) <= 0 )
   	   {
      	     obj = create_object(pObjIndex, 1);
             obj_to_room(obj, pRoomIndex);
   	   }
        }
        
        if ( IS_SET( pRoomIndex->room_flags, ROOM_INFO ) )
        {
   	   if ( !(pObjIndex = get_obj_index(OBJ_VNUM_MESSAGE_TERMINAL)) )
   	   {
             bug( "Reset_all: Missing message terminal (%d)", OBJ_VNUM_MESSAGE_TERMINAL );
             return;
   	   }
   	   if ( count_obj_list(pObjIndex, pRoomIndex->first_content) <= 0 )
   	   {
      	     obj = create_object(pObjIndex, 1);
             obj_to_room(obj, pRoomIndex);
   	   }
        }

        if ( IS_SET( pRoomIndex->room_flags, ROOM_BARRACKS ) 
        && pRoomIndex->area && pRoomIndex->area->planet)
        {
            int guard_count = 0;
            OBJ_DATA * blaster;
            GUARD_DATA * guard;
            char tmpbuf[MAX_STRING_LENGTH];
        
            if ( !(pMobIndex = get_mob_index(MOB_VNUM_PATROL)) )
            {
        	   bug( "Reset_all: Missing default patrol (%d)", vnum );
        	   return;
      	    }
            
            for ( guard = pRoomIndex->area->planet->first_guard ; guard ; guard = guard->next_on_planet )
                guard_count++;
                
            if ( pRoomIndex->area->planet->barracks*5 <= guard_count ) 
                continue;
                
            mob = create_mobile( pMobIndex );
            char_to_room( mob, pRoomIndex );
            mob->top_level = 10;
            mob->hit = 100;
            mob->max_hit = 100;
            mob->armor = 50;
            mob->damroll = 0;
            mob->hitroll = 20;
            if ( ( pObjIndex = get_obj_index( OBJ_VNUM_BLASTER ) ) != NULL )
            {
                 blaster = create_object( pObjIndex, mob->top_level );
                 obj_to_char( blaster, mob );
                 equip_char( mob, blaster, WEAR_WIELD );                        
            } 
            do_setblaster( mob , "full" );

            CREATE( guard , GUARD_DATA , 1 );
                
            guard->planet = pRoomIndex->area->planet;
            LINK( guard , guard->planet->first_guard, guard->planet->last_guard, next_on_planet, prev_on_planet );
            LINK( guard , first_guard, last_guard, next, prev );
            guard->mob = mob;
            guard->reset_loc = pRoomIndex;
            mob->guard_data = guard;           
            if ( room_is_dark(pRoomIndex) )
                   SET_BIT(mob->affected_by, AFF_INFRARED);
            if ( pRoomIndex->area->planet->governed_by )
            {
                sprintf( tmpbuf , "A soldier patrols the area. (%s)\n\r" , pRoomIndex->area->planet->governed_by->name );
                STRFREE( mob->long_descr );
                mob->long_descr = STRALLOC( tmpbuf );
                mob->mob_clan  = pRoomIndex->area->planet->governed_by;
            }
            
            continue; 
        }

        if ( ( IS_SET( pRoomIndex->room_flags, ROOM_SHIPYARD ) 
        || IS_SET( pRoomIndex->room_flags, ROOM_CAN_LAND ) )
        && pRoomIndex->area && pRoomIndex->area->planet)
        {
            char tmpbuf[MAX_STRING_LENGTH];
            CHAR_DATA * rch;
            int numguards = 0;
             
            if ( !(pMobIndex = get_mob_index(MOB_VNUM_GUARD)) )
            {
        	   bug( "Reset_all: Missing default guard (%d)", vnum );
        	   return;
      	    }
            
            for( rch = pRoomIndex->first_person ; rch ; rch = rch->next_in_room )
               if ( IS_NPC(rch) && rch->pIndexData && rch->pIndexData->vnum == MOB_VNUM_GUARD )
                   numguards++;
            
            if ( numguards >= 2 )
               continue;          
            
            mob = create_mobile( pMobIndex );
            char_to_room( mob, pRoomIndex );
            mob->top_level = 100;
            mob->hit = 500;
            mob->max_hit = 500;
            mob->armor = 0;
            mob->damroll = 0;
            mob->hitroll = 20;

            if ( room_is_dark(pRoomIndex) )
                   SET_BIT(mob->affected_by, AFF_INFRARED);
            if ( pRoomIndex->area->planet->governed_by )
            {
                sprintf( tmpbuf , "A Platform Security Guard stands alert and ready for trouble. (%s)\n\r" , pRoomIndex->area->planet->governed_by->name );
                STRFREE( mob->long_descr );
                mob->long_descr = STRALLOC( tmpbuf );
                mob->mob_clan  = pRoomIndex->area->planet->governed_by;
            }
            
        }

/* hidden food & resources */

        if ( !pRoomIndex->area || !pRoomIndex->area->planet )
           continue;

        anumber = number_bits( 2 );
        
        if ( pRoomIndex->sector_type != SECT_CITY &&  pRoomIndex->sector_type != SECT_WATER_NOSWIM  
        &&  pRoomIndex->sector_type != SECT_WATER_SWIM &&  pRoomIndex->sector_type != SECT_UNDERWATER
        &&  pRoomIndex->sector_type != SECT_DUNNO  &&  pRoomIndex->sector_type != SECT_AIR        
        &&  pRoomIndex->sector_type != SECT_INSIDE  &&  pRoomIndex->sector_type != SECT_GLACIAL
        && !pRoomIndex->last_content && number_bits(3) == 0 )
        {
            switch ( pRoomIndex->sector_type )
            {
                default:
                   if ( anumber <=1 )
                      vnum = OBJ_VNUM_PLANT;
                   else
                      vnum = OBJ_VNUM_HEMP;
                   break;

                case SECT_FARMLAND:
                   if ( anumber == 0 )
                      vnum = OBJ_VNUM_FRUIT;
                   else  if ( anumber == 1 )
                      vnum = OBJ_VNUM_ROOT;
                   else  if ( anumber == 2 )
                      vnum = OBJ_VNUM_MUSHROOM;
                   else
                      vnum = OBJ_VNUM_PLANT;
                   break;
                   
                case SECT_FOREST:
                case SECT_BRUSH:
                case SECT_SCRUB:
                   if ( anumber <=1 )
                      vnum = OBJ_VNUM_ROOT;
                   else
                      vnum = OBJ_VNUM_HEMP;
                   break;
                
                case SECT_RAINFOREST:
                case SECT_JUNGLE:
                   if ( anumber <=1 )
                      vnum = OBJ_VNUM_FRUIT;
                   else
                      vnum = OBJ_VNUM_RESIN;
                   break;
                                   
                case SECT_MOUNTAIN:
                case SECT_UNDERGROUND: 
                case SECT_ROCKY:
                case SECT_TUNDRA: 
                case SECT_VOLCANIC:            
                   if ( anumber == 0 )
                      vnum = OBJ_VNUM_MUSHROOM;
                   else
                   if ( anumber == 1 )
                      vnum = OBJ_VNUM_CRYSTAL;
                   else 
                   if ( anumber == 2 )
                      vnum = OBJ_VNUM_METAL;
                   else
                      vnum = OBJ_VNUM_GOLD;
                   break;

                case SECT_SWAMP:
                   if ( anumber <=1 )
                      vnum = OBJ_VNUM_SEAWEED;
                   else
                      vnum = OBJ_VNUM_RESIN;
                   break;
                
                case SECT_OCEANFLOOR: 
                   if ( anumber <=1 )
                      vnum = OBJ_VNUM_SEAWEED;
                   else
                      vnum = OBJ_VNUM_CRYSTAL;
                   break;
            }

            if ( !(pObjIndex = get_obj_index(vnum)) )
            {
        	   bug( "Reset_all: Missing obj (%d)", vnum );
        	   return;
      	    }
            obj = create_object(pObjIndex , 1);
            if ( pRoomIndex->sector_type != SECT_FARMLAND )
            {
               if ( vnum == OBJ_VNUM_ROOT || vnum == OBJ_VNUM_CRYSTAL 
               || vnum == OBJ_VNUM_GOLD || vnum == OBJ_VNUM_METAL ) 
                  SET_BIT ( obj->extra_flags , ITEM_BURRIED );           
               else
                  SET_BIT ( obj->extra_flags , ITEM_HIDDEN );           
            }
            if ( vnum == OBJ_VNUM_GOLD )
            {
               obj->value[0] = number_range(1,10);
               obj->value[1] = obj->value[0];
               obj->cost =  obj->value[0]*10;
            }
            obj_to_room(obj, pRoomIndex);
             
        }
        
/* random mobs start here */        
        

        if ( IS_SET( pRoomIndex->room_flags, ROOM_NO_MOB ) )
           continue;
        
        if ( number_bits(1) != 0 )
           continue;
           
        if ( pRoomIndex->sector_type == SECT_CITY )
        {
           if ( pRoomIndex->area->planet->population >= max_population(pRoomIndex->area->planet) )
              continue;  
           
           if ( number_bits( 5 ) == 0 )
           {
              if ( (pMobIndex = get_mob_index(MOB_VNUM_VENDOR)) )
              {
                  int rep;
                  
                  mob = create_mobile(pMobIndex);
                  SET_BIT ( mob->act , ACT_CITIZEN );           
                  char_to_room(mob, pRoomIndex);
                  pRoomIndex->area->planet->population++;
                  for ( rep = 0 ; rep < 3 ; rep++ )
                    if ( (pObjIndex = get_obj_index( number_range( OBJ_VNUM_FIRST_FABRIC , OBJ_VNUM_LAST_FABRIC  ) ) ) )
                    {
                        obj = create_object(pObjIndex, 1);
                        SET_BIT(obj->extra_flags, ITEM_INVENTORY);
                        obj = obj_to_char(obj, mob);
                    }
                  if ( (pObjIndex = get_obj_index( OBJ_VNUM_SEWKIT ) ) )
                  {
                        obj = create_object(pObjIndex, 1);
                        SET_BIT(obj->extra_flags, ITEM_INVENTORY);
                        obj = obj_to_char(obj, mob);
                  }
                  continue;
              }
           }

           if ( number_bits( 6 ) == 0 )
           {
              if ( (pMobIndex = get_mob_index(MOB_VNUM_DEALER)) )
              {
                  mob = create_mobile(pMobIndex);
                  SET_BIT ( mob->act , ACT_CITIZEN );           
                  char_to_room(mob, pRoomIndex);
                  pRoomIndex->area->planet->population++;
                  if ( (pObjIndex = get_obj_index( OBJ_VNUM_BLACK_POWDER ) ) )
                  {
                        obj = create_object(pObjIndex, 1);
                        SET_BIT(obj->extra_flags, ITEM_INVENTORY);
                        obj = obj_to_char(obj, mob);
                  }
                  continue;
              }
           }
           if ( number_bits( 6 ) == 0 )
           {
              int mnum;
              
              switch ( number_bits( 2 ) )
              {
                   default:
                          mnum = MOB_VNUM_BUM;
                          break;
                   case 0:
                          mnum = MOB_VNUM_THUG;
                          break;
                   case 1:
                          mnum = MOB_VNUM_THIEF;
                          break;
              }
              if ( (pMobIndex = get_mob_index(mnum)) )
              {
                  mob = create_mobile(pMobIndex);
                  SET_BIT ( mob->act , ACT_CITIZEN );           
                  char_to_room(mob, pRoomIndex);
                  pRoomIndex->area->planet->population++;
                  continue;
              }
           }
           
           if ( !(pMobIndex = get_mob_index(MOB_VNUM_CITIZEN)) )
           {
        	   bug( "Reset_all: Missing default colonist (%d)", vnum );
        	   return;
           }
           mob = create_mobile(pMobIndex);
           SET_BIT ( mob->act , ACT_CITIZEN );           
           mob->sex = number_bits( 1 )+1;
           STRFREE( mob->long_descr );
           if ( mob->sex == 2 )
              switch( number_bits( 4 ) )
              {
                default:
                  mob->long_descr = STRALLOC( "A colonist is going about her daily business.\n\r" );
                  break;

                case 0:
                  mob->long_descr = STRALLOC( "A wealthy colonist is dressed in fine silks.\n\r" );
                  mob->gold = number_range( 1 , 100 );
                  break;

                case 1:
                  mob->long_descr = STRALLOC( "A young colonist smiles at you as she walks by.\n\r" );
                  break;

                case 2:
                  mob->long_descr = STRALLOC( "A young schoolgirl is skipping.\n\r" );
                  break;


                case 3:
                  mob->long_descr = STRALLOC( "A colonist is dressed in formal business attire.\n\r" );
                  mob->gold = number_range( 20 , 50 );
                  break;
                  
                case 4:
                  mob->long_descr = STRALLOC( "An elderly colonist strolls by.\n\r" );
                  mob->gold = number_range( 20 , 50 );
                  break;
              }
           else
              switch( number_bits( 3 ) )
              {
                default:
                  mob->long_descr = STRALLOC( "A colonist is going about his daily business.\n\r" );
                  break;

                case 0:
                  mob->long_descr = STRALLOC( "A wealthy colonist is dressed in fine silk robes.\n\r" );
                  mob->gold = number_range( 1 , 100 );
                  break;

                case 1:
                  mob->long_descr = STRALLOC( "A young colonist is just hanging out.\n\r" );
                  break;

                case 2:
                  mob->long_descr = STRALLOC( "A young boy is kicking a small stone around.\n\r" );
                  break;

                case 3:
                  mob->long_descr = STRALLOC( "A businessman looks to be in a hurry.\n\r" );
                  mob->gold = number_range( 20 , 50 );
                  break;

                case 4:
                  mob->long_descr = STRALLOC( "An elderly colonist strolls by.\n\r" );
                  mob->gold = number_range( 20 , 50 );
                  break;
              }
           char_to_room(mob, pRoomIndex);
           pRoomIndex->area->planet->population++;
           continue;
        }

        if ( pRoomIndex->area->planet->wildlife >  pRoomIndex->area->planet->wilderness )
              continue;  

        if ( pRoomIndex->sector_type > SECT_CITY && number_bits( 16 ) == 0 )
        {
              if ( (pMobIndex = get_mob_index(MOB_VNUM_DRAGON)) )
              {
                  OBJ_DATA * nest;
                  
                  mob = create_mobile(pMobIndex);
                  SET_BIT ( mob->act , ACT_CITIZEN );           
                  char_to_room(mob, pRoomIndex);
                  pRoomIndex->area->planet->population++;
                  if ( (pObjIndex = get_obj_index( OBJ_VNUM_DRAGON_NEST ) ) )
                  {
                        nest = create_object(pObjIndex, 1);
                        nest = obj_to_room(nest, pRoomIndex);
                        if ( (pObjIndex = get_obj_index( OBJ_VNUM_DRAGON_CRYSTAL ) ) )
                        {
                            obj = create_object(pObjIndex, 1);
                            obj = obj_to_obj(obj, nest);
                        }
                  }
                  continue;
              }
        }

        anumber = number_bits( 3 );
        
        switch ( pRoomIndex->sector_type )
        {
           default:
              continue;
              break;

           case SECT_WATER_SWIM:
              if ( anumber == 0 )
                vnum = MOB_VNUM_INSECT;
              else if ( anumber == 1 )
                vnum = MOB_VNUM_BIRD;
              else
                vnum = MOB_VNUM_FISH;
              break;
                              
           case SECT_OCEANFLOOR:
           case SECT_UNDERWATER:
              vnum = MOB_VNUM_FISH;
              break;

           case SECT_AIR:
              if ( anumber == 0 )
                vnum = MOB_VNUM_INSECT;
              else
                vnum = MOB_VNUM_BIRD;
              break;

           case SECT_VOLCANIC:
           case SECT_UNDERGROUND: 
           case SECT_DESERT:
              if ( anumber == 0 )
                vnum = MOB_VNUM_INSECT;
              else
                vnum = MOB_VNUM_SCAVENGER;
              break;
                        
           case SECT_MOUNTAIN:
           case SECT_SCRUB:
           case SECT_ROCKY:
              if ( anumber == 0 )
                vnum = MOB_VNUM_INSECT;
              else if ( anumber == 1 )
                vnum = MOB_VNUM_SMALL_ANIMAL;
              else if ( anumber == 2 )
                vnum = MOB_VNUM_SCAVENGER;
              else if ( anumber == 3 )
                vnum = MOB_VNUM_PREDITOR;
              else
                vnum = MOB_VNUM_BIRD;
              break;


           case SECT_FIELD:
           case SECT_FOREST:
           case SECT_HILLS:
           case SECT_SAVANNA:
           case SECT_BRUSH:
           case SECT_STEPPE:
              if ( anumber == 0 )
                vnum = MOB_VNUM_INSECT;
              else if ( anumber == 1 )
                vnum = MOB_VNUM_BIRD;
              else if ( anumber == 2 )
                vnum = MOB_VNUM_SCAVENGER;
              else if ( anumber == 3 )
                vnum = MOB_VNUM_PREDITOR;
              else
                vnum = MOB_VNUM_SMALL_ANIMAL;
              break;
              
           case SECT_TUNDRA:
              if ( anumber == 0 )
                vnum = MOB_VNUM_SMALL_ANIMAL;
              else if ( anumber == 2 )
                vnum = MOB_VNUM_SCAVENGER;
              else if ( anumber == 3 )
                vnum = MOB_VNUM_PREDITOR;
              else
                vnum = MOB_VNUM_BIRD;
              break;

           case SECT_RAINFOREST:
           case SECT_JUNGLE:
           case SECT_SWAMP:
           case SECT_WETLANDS:
              if ( anumber == 0 )
                vnum = MOB_VNUM_SMALL_ANIMAL;
              else if ( anumber == 1 )
                vnum = MOB_VNUM_BIRD;
              else if ( anumber == 2 )
                vnum = MOB_VNUM_SCAVENGER;
              else if ( anumber == 3 )
                vnum = MOB_VNUM_PREDITOR;
              else
                vnum = MOB_VNUM_INSECT;
              break;
  
        }
        
        if ( !(pMobIndex = get_mob_index(vnum)) )
        {
        	   bug( "Reset_all: Missing mob (%d)", vnum );
        	   return;
      	}
        mob = create_mobile(pMobIndex);
        REMOVE_BIT ( mob->act , ACT_CITIZEN );           
        if ( room_is_dark(pRoomIndex) )
                   SET_BIT(mob->affected_by, AFF_INFRARED);
        char_to_room(mob, pRoomIndex);
        pRoomIndex->area->planet->wildlife++;

     }
   }
                        
/*

  for ( pReset = first_reset; pReset; pReset = pReset->next )
  {
    num++;
    switch( pReset->command )
    {
    default:
      bug( "Reset_area: %d bad command %c.", num,  pReset->command );
      break;
    
    case 'M':
      if ( !(pMobIndex = get_mob_index(pReset->arg1)) )
      {
        bug( "Reset_area: %d 'M': bad mob vnum %ld.", num, pReset->arg1 );
        continue;
      }
      if ( !(pRoomIndex = get_room_index(pReset->arg3)) )
      {
        bug( "Reset_area: %d 'M': bad room vnum %ld.", num, pReset->arg3 );
        continue;
      }
      if ( pMobIndex->count >= pReset->arg2 )
      {
        mob = NULL;
        break;
      }
      mob = create_mobile(pMobIndex);
      {
        ROOM_INDEX_DATA *pRoomPrev = get_room_index(pReset->arg3 - 1);
        
        if ( pRoomPrev && IS_SET(pRoomPrev->room_flags, ROOM_PET_SHOP) )
          SET_BIT(mob->act, ACT_PET);
      }
      if ( room_is_dark(pRoomIndex) )
        SET_BIT(mob->affected_by, AFF_INFRARED);
      char_to_room(mob, pRoomIndex);
      level = URANGE(0, mob->top_level - 2, LEVEL_AVATAR);
      if ( pRoomIndex->area->planet )
            pRoomIndex->area->planet->population++;
      break;
    
    case 'G':
    case 'E':
      if ( !(pObjIndex = get_obj_index(pReset->arg1)) )
      {
        bug( "Reset_area: %d 'E' or 'G': bad obj vnum %ld.", num, pReset->arg1 );
        continue;
      }
      if ( !mob )
      {
        lastobj = NULL;
        break;
      }
      if ( mob->pIndexData->pShop )
      {
	int olevel = generate_itemlevel( pObjIndex );
        obj = create_object(pObjIndex, olevel);
        SET_BIT(obj->extra_flags, ITEM_INVENTORY);
      }
      else
        obj = create_object(pObjIndex, number_fuzzy(level));
      obj->level = URANGE(0, obj->level, LEVEL_AVATAR);
      obj = obj_to_char(obj, mob);
      if ( pReset->command == 'E' )
        equip_char(mob, obj, pReset->arg3);
      lastobj = obj;
      break;
    
    case 'O':
      if ( !(pObjIndex = get_obj_index(pReset->arg1)) )
      {
        bug( "Reset_area: %d 'O': bad obj vnum %ld.", num, pReset->arg1 );
        continue;
      }
      if ( !(pRoomIndex = get_room_index(pReset->arg3)) )
      {
        bug( "Reset_area: %d 'O': bad room vnum %ld.", num, pReset->arg3 );
        continue;
      }
      if ( count_obj_list(pObjIndex, pRoomIndex->first_content) > 0 )
      {
        obj = NULL;
        lastobj = NULL;
        break;
      }
      obj = create_object(pObjIndex, number_fuzzy(generate_itemlevel( pObjIndex)));
      obj->level = UMIN(obj->level, LEVEL_AVATAR);
      obj->cost = 0;
      obj_to_room(obj, pRoomIndex);
      lastobj = obj;
      break;
    
    case 'P':
      if ( !(pObjIndex = get_obj_index(pReset->arg1)) )
      {
        bug( "Reset_area: %d 'P': bad obj vnum %ld.", num, pReset->arg1 );
        continue;
      }
      if ( pReset->arg3 > 0 )
      {
        if ( !(pObjToIndex = get_obj_index(pReset->arg3)) )
        {
          bug( "Reset_area: %d 'P': bad objto vnum %ld.", num, pReset->arg3 );
          continue;
        }
        if (
           !(to_obj = get_obj_type(pObjToIndex)) ||
            !to_obj->in_room ||
             count_obj_list(pObjIndex, to_obj->first_content) > 0 )
        {
          obj = NULL;
          break;
        }
        lastobj = to_obj;
      }
      else
      {
        int iNest;
        
        if ( !lastobj )
          break;
        to_obj = lastobj;
        for ( iNest = 0; iNest < pReset->extra; iNest++ )
          if ( !(to_obj = to_obj->last_content) )
          {
            bug( "Reset_area: %d 'P': Invalid nesting obj %d.", num, pReset->arg1 );
            iNest = -1;
            break;
          }
        if ( iNest < 0 )
          continue;
      }
      obj = create_object(pObjIndex, number_fuzzy(UMAX(generate_itemlevel( pObjIndex),to_obj->level)));
      obj->level = UMIN(obj->level, LEVEL_AVATAR);
      obj_to_obj(obj, to_obj);
      break;
    
    case 'H':
      if ( pReset->arg1 > 0 )
      {
        if ( !(pObjToIndex = get_obj_index(pReset->arg1)) )
        {
          bug( "Reset_area: %d 'H': bad objto vnum %ld.", num, pReset->arg1 );
          continue;
        }
        if ( 
           !(to_obj = get_obj_type(pObjToIndex)) ||
            !to_obj->in_room ||
             IS_OBJ_STAT(to_obj, ITEM_HIDDEN) )
          break;
      }
      else
      {
        if ( !lastobj || !obj )
          break;
        to_obj = obj;
      }
      SET_BIT(to_obj->extra_flags, ITEM_HIDDEN);
      break;
    
    case 'B':
      switch(pReset->arg2 & BIT_RESET_TYPE_MASK)
      {
      case BIT_RESET_DOOR:
        {
        int doornum;
        
        if ( !(pRoomIndex = get_room_index(pReset->arg1)) )
        {
          bug( "Reset_area: %d 'B': door: bad room vnum %ld.", num, pReset->arg1 );
          continue;
        }
        doornum = (pReset->arg2 & BIT_RESET_DOOR_MASK)
                >> BIT_RESET_DOOR_THRESHOLD;
        if ( !(pexit = get_exit(pRoomIndex, doornum)) )
          break;
        plc = &pexit->exit_info;
        }
        break;
      case BIT_RESET_ROOM:
        if ( !(pRoomIndex = get_room_index(pReset->arg1)) )
        {
          bug( "Reset_area: %d 'B': room: bad room vnum %ld.", num, pReset->arg1 );
          continue;
        }
        plc = &pRoomIndex->room_flags;
        break;
      case BIT_RESET_OBJECT:
        if ( pReset->arg1 > 0 )
        {
          if ( !(pObjToIndex = get_obj_index(pReset->arg1)) )
          {
            bug( "Reset_area: %d 'B': object: bad objto vnum %ld.", num, pReset->arg1 );
            continue;
          }
          if ( !(to_obj = get_obj_type(pObjToIndex)) ||
                !to_obj->in_room  )
            continue;
        }
        else
        {
          if ( !lastobj || !obj )
            continue;
          to_obj = obj;
        }
        plc = &to_obj->extra_flags;
        break;
      case BIT_RESET_MOBILE:
        if ( !mob )
          continue;
        plc = &mob->affected_by;
        break;
      default:
        bug( "Reset_area: %d 'B': bad options %d.", num, pReset->arg2 );
        continue;
      }
      if ( IS_SET(pReset->arg2, BIT_RESET_SET) )
        SET_BIT(*plc, pReset->arg3);
      else if ( IS_SET(pReset->arg2, BIT_RESET_TOGGLE) )
        TOGGLE_BIT(*plc, pReset->arg3);
      else
        REMOVE_BIT(*plc, pReset->arg3);
      break;
    
    case 'D':
      if ( !(pRoomIndex = get_room_index(pReset->arg1)) )
      {
        bug( "Reset_area: %d 'D': bad room vnum %ld.", num, pReset->arg1 );
        continue;
      }
      if ( !(pexit = get_exit(pRoomIndex, pReset->arg2)) )
        break;
      switch( pReset->arg3 )
      {
      case 0:
        REMOVE_BIT(pexit->exit_info, EX_CLOSED);
        REMOVE_BIT(pexit->exit_info, EX_LOCKED);
        break;
      case 1:
        SET_BIT(   pexit->exit_info, EX_CLOSED);
        REMOVE_BIT(pexit->exit_info, EX_LOCKED);
        if ( IS_SET(pexit->exit_info, EX_xSEARCHABLE) )
          SET_BIT( pexit->exit_info, EX_SECRET);
        break;
      case 2:
        SET_BIT(   pexit->exit_info, EX_CLOSED);
        SET_BIT(   pexit->exit_info, EX_LOCKED);
        if ( IS_SET(pexit->exit_info, EX_xSEARCHABLE) )
          SET_BIT( pexit->exit_info, EX_SECRET);
        break;
      }
      break;
    
    case 'R':
      if ( !(pRoomIndex = get_room_index(pReset->arg1)) )
      {
        bug( "Reset_area: %d 'R': bad room vnum %ld.", num, pReset->arg1 );
        continue;
      }
      randomize_exits(pRoomIndex, pReset->arg2-1);
      break;
    }
  }
  return;

*/

}

SHIP_DATA * make_mob_ship( PLANET_DATA *planet , int model )
{
    SHIP_DATA *ship;
    int shipreg = 0;
    int dIndex = 0;
    char filename[10];
    char shipname[MAX_STRING_LENGTH];
    
    if ( !planet || !planet->governed_by || !planet->starsystem )
       return NULL;
       
    /* mobships are given filenames < 0 and are not saved */   
       
    for ( ship = first_ship ; ship ; ship = ship->next )
        if ( shipreg > atoi( ship->filename ) )
             shipreg = atoi( ship->filename );

    shipreg--;
    sprintf( filename , "%d" , shipreg );
    
    CREATE( ship, SHIP_DATA, 1 );
    LINK( ship, first_ship, last_ship, next, prev );
    
    ship->filename = str_dup( filename ) ;
    
    ship->next_in_starsystem = NULL;
    ship->prev_in_starsystem =NULL;
    ship->next_in_room = NULL;
    ship->prev_in_room = NULL;
    ship->first_turret = NULL;
    ship->last_turret = NULL;
    ship->first_hanger = NULL;
    ship->last_hanger = NULL;
    ship->in_room = NULL;
    ship->starsystem = NULL;
    ship->home = STRALLOC( planet->starsystem->name );
    for ( dIndex = 0 ; dIndex < MAX_SHIP_ROOMS ; dIndex++ )
       ship->description[dIndex] = NULL;
    ship->owner = STRALLOC( planet->governed_by->name );
    ship->pilot = STRALLOC("");
    ship->copilot = STRALLOC("");
    ship->dest = NULL;
    ship->type = MOB_SHIP;
    ship->class = 0;
    ship->model = model;
    ship->hyperspeed = 0;
    ship->laserstate = LASER_READY;
    ship->missilestate = MISSILE_READY;
    ship->tractorbeam = 2;
    ship->hatchopen = FALSE;
    ship->autotrack = FALSE;
    ship->autospeed = FALSE;
    ship->location = 0;
    ship->lastdoc = 0;
    ship->shipyard = 0;
    ship->collision = 0;
    ship->target = NULL;
    ship->currjump = NULL;
    ship->chaff = 0;
    ship->maxchaff = 0;
    ship->chaff_released = FALSE;

    switch( ship->model )
    {
    case MOB_BATTLESHIP:
       ship->realspeed = 25;
       ship->maxmissiles = 50;
       ship->lasers = 10;
       ship->maxenergy = 30000;
       ship->maxshield = 1000;
       ship->maxhull = 30000;
       ship->manuever = 25;
       sprintf( shipname , "Battlecruiser m%d (%s)" , 0-shipreg , planet->governed_by->name );
       break;

    case MOB_CRUISER:
       ship->realspeed = 50;
       ship->maxmissiles = 30;
       ship->lasers = 8;
       ship->maxenergy = 15000;
       ship->maxshield = 350;
       ship->maxhull = 10000;
       ship->manuever = 50;
       sprintf( shipname , "Cruiser m%d (%s)" , 0-shipreg , planet->governed_by->name );
       break;

    case MOB_DESTROYER:
       ship->realspeed = 100;
       ship->maxmissiles = 20;
       ship->lasers = 6;
       ship->maxenergy = 7500;
       ship->maxshield = 200;
       ship->maxhull = 2000;
       ship->manuever = 100;
       ship->hyperspeed = 100;
       sprintf( shipname , "Corvette m%d (%s)" , 0-shipreg , planet->governed_by->name );
       break;

    default:
       ship->realspeed = 255;
       ship->maxmissiles = 0;
       ship->lasers = 2;
       ship->maxenergy = 2500;
       ship->maxshield = 0;
       ship->maxhull = 100;
       ship->manuever = 100;
       sprintf( shipname , "Patrol Starfighter m%d (%s)" , 0-shipreg , planet->governed_by->name );
       break;
    }
    
    ship->name = STRALLOC( shipname );
    ship->hull = ship->maxhull;
    ship->missiles = ship->maxmissiles;
    ship->energy = ship->maxenergy;
    ship->shield = 0;

    ship_to_starsystem(ship, starsystem_from_name(ship->home) );  
    ship->vx = planet->x + number_range( -2000 , 2000 );
    ship->vy = planet->y + number_range( -2000 , 2000 );
    ship->vz = planet->z + number_range( -2000 , 2000 );
    ship->shipstate = SHIP_READY;
    ship->autopilot = TRUE;
    ship->autorecharge = TRUE;
    ship->shield = ship->maxshield;
    
    return ship;
}
