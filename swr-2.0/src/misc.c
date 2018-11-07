#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

extern int	top_exit;

void do_buyhome( CHAR_DATA *ch, char *argument )
{
     ROOM_INDEX_DATA *room;
     AREA_DATA *pArea;
     
     if ( !ch->in_room )
         return;

     if ( !ch->in_room->area )
     {
         send_to_char( "&RThis is not a safe place to live.\n\r&w", ch);
         return;   
     }

         
     if ( IS_NPC(ch) || !ch->pcdata )
         return;
         
     if ( ch->plr_home != NULL )
     {
         send_to_char( "&RYou already have a home!\n\r&w", ch);
         return;   
     }
     
     room = ch->in_room;
     
     for ( pArea = first_bsort; pArea; pArea = pArea->next_sort )
     {
         if ( room->area == pArea )
         {
             send_to_char( "&RThis area isn't installed yet!\n\r&w", ch);
             return;
         }
     }
     
     if ( !IS_SET( room->room_flags , ROOM_EMPTY_HOME ) )
     {
         send_to_char( "&RThis room isn't for sale!\n\r&w", ch);
         return;   
     }
     
     if ( ch->gold < 100000 )
     {
         send_to_char( "&RThis room costs 100000 credits you don't have enough!\n\r&w", ch);
         return;   
     }
     
     if ( argument[0] == '\0' )
     {
	   send_to_char( "Set the room name.  A very brief single line room description.\n\r", ch );
	   send_to_char( "Usage: Buyhome <Room Name>\n\r", ch );
	   return;
     }
      
     STRFREE( room->name );
     room->name = STRALLOC( argument );

     ch->gold -= 100000;
     
     REMOVE_BIT( room->room_flags , ROOM_EMPTY_HOME );
     SET_BIT( room->room_flags , ROOM_PLR_HOME );
     SET_BIT( room->room_flags , ROOM_HOTEL );
     
     fold_area( room->area, room->area->filename, FALSE );

     ch->plr_home = room;
     do_save( ch , "" );

}

void do_ammo( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *wield;
    OBJ_DATA *obj;
    bool checkammo = FALSE;
    int charge =0;
    
    obj = NULL;
    wield = get_eq_char( ch, WEAR_WIELD );
    if (wield)
    {
       obj = get_eq_char( ch, WEAR_DUAL_WIELD );
       if (!obj) 
          obj = get_eq_char( ch, WEAR_HOLD );
    } 
    else 
    { 
      wield = get_eq_char( ch, WEAR_HOLD );
      obj = NULL;
    } 
    
    if (!wield || wield->item_type != ITEM_WEAPON )
    {
        send_to_char( "&RYou don't seem to be holding a weapon.\n\r&w", ch);
        return; 
    }
    
    if ( wield->value[3] == WEAPON_BLASTER )
    {
    
      if ( obj && obj->item_type != ITEM_AMMO )
      {
        send_to_char( "&RYour hands are too full to reload your blaster.\n\r&w", ch);
        return;
      }
    
      if (obj)
      {
        if ( obj->value[0] > wield->value[5] )
        {
            send_to_char( "That cartridge is too big for your blaster.", ch);
            return;
        }
        unequip_char( ch, obj );
        checkammo = TRUE;
        charge = obj->value[0];
        separate_obj( obj );
        extract_obj( obj );
      }
      else
      {
        for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
        {
           if ( obj->item_type == ITEM_AMMO)
           {
                 if ( obj->value[0] > wield->value[5] )
                 {
                    send_to_char( "That cartridge is too big for your blaster.", ch);
                    continue;
                 }
                 checkammo = TRUE;
                 charge = obj->value[0];
                 separate_obj( obj );
                 extract_obj( obj );
                 break;                         
           }  
        }
      }
    
      if (!checkammo)
      {
        send_to_char( "&RYou don't seem to have any ammo to reload your blaster with.\n\r&w", ch);
        return;
      }
      
      ch_printf( ch, "You replace your ammunition cartridge.\n\rYour blaster is charged with %d shots at high power to %d shots on low.\n\r", charge/5, charge );
      act( AT_PLAIN, "$n replaces the ammunition cell in $p.", ch, wield, NULL, TO_ROOM );
	
    }
    else
    {
    
      if ( obj && obj->item_type != ITEM_BATTERY )
      {
        send_to_char( "&RYour hands are too full to replace the power cell.\n\r&w", ch);
        return;
      }
    
      if (obj)
      {
        unequip_char( ch, obj );
        checkammo = TRUE;
        charge = obj->value[0];
        separate_obj( obj );
        extract_obj( obj );
      }
      else
      {
        for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
        {
           if ( obj->item_type == ITEM_BATTERY)
           {
                 checkammo = TRUE;
                 charge = obj->value[0];
                 separate_obj( obj );
                 extract_obj( obj );
                 break;                         
           }  
        }
      }
    
      if (!checkammo)
      {
        send_to_char( "&RYou don't seem to have a power cell.\n\r&w", ch);
        return;
      }
      
      if (wield->value[3] == WEAPON_LIGHTSABER )
      {
         ch_printf( ch, "You replace your power cell.\n\rYour lightsaber is charged to %d/%d units.\n\r", charge, charge );
         act( AT_PLAIN, "$n replaces the power cell in $p.", ch, wield, NULL, TO_ROOM );
	 act( AT_PLAIN, "$p ignites with a bright glow.", ch, wield, NULL, TO_ROOM );
      }
      else if (wield->value[3] == WEAPON_VIBRO_BLADE )
      {
         ch_printf( ch, "You replace your power cell.\n\rYour vibro-blade is charged to %d/%d units.\n\r", charge, charge );
         act( AT_PLAIN, "$n replaces the power cell in $p.", ch, wield, NULL, TO_ROOM );
      }
      else
      {
         ch_printf( ch, "You feel very foolish.\n\r" );
         act( AT_PLAIN, "$n tries to jam a power cell into $p.", ch, wield, NULL, TO_ROOM );
      }
    }
    
    wield->value[4] = charge;
        
}

void do_setblaster( CHAR_DATA *ch, char *argument )
{
   OBJ_DATA *wield;
   OBJ_DATA *wield2;
 
   wield = get_eq_char( ch, WEAR_WIELD );
   if( wield && !( wield->item_type == ITEM_WEAPON && wield->value[3] == WEAPON_BLASTER ) )
      wield = NULL;
   wield2 = get_eq_char( ch, WEAR_DUAL_WIELD );
   if( wield2 && !( wield2->item_type == ITEM_WEAPON && wield2->value[3] == WEAPON_BLASTER ) )
      wield2 = NULL;
   
   if ( !wield && !wield2 )
   {
      send_to_char( "&RYou don't seem to be wielding a blaster.\n\r&w", ch);
      return;
   }
   
   if ( argument[0] == '\0' )
   {
      send_to_char( "&RUsage: setblaster <full|high|normal|half|low|stun>\n\r&w", ch);
      return;
   }
   
   if ( wield )
     act( AT_PLAIN, "$n adjusts the settings on $p.", ch, wield, NULL, TO_ROOM );
  
   if ( wield2 )
    act( AT_PLAIN, "$n adjusts the settings on $p.", ch, wield2, NULL, TO_ROOM );
          
   if ( !str_cmp( argument, "full" ) )
   {
      if (wield)
      {
        wield->blaster_setting = BLASTER_FULL;
        send_to_char( "&YWielded blaster set to FULL Power\n\r&w", ch);
      }
      if (wield2)
      {
        wield2->blaster_setting = BLASTER_FULL;
        send_to_char( "&YDual wielded blaster set to FULL Power\n\r&w", ch);
      }
      return;
   } 
   if ( !str_cmp( argument, "high" ) )
   {
      if (wield)
      {
        wield->blaster_setting = BLASTER_HIGH;
        send_to_char( "&YWielded blaster set to HIGH Power\n\r&w", ch);
      }
      if (wield2)
      {
        wield2->blaster_setting = BLASTER_HIGH;
        send_to_char( "&YDual wielded blaster set to HIGH Power\n\r&w", ch);
      }
      return;
   } 
   if ( !str_cmp( argument, "normal" ) )
   {
      if (wield)
      {
        wield->blaster_setting = BLASTER_NORMAL;
        send_to_char( "&YWielded blaster set to NORMAL Power\n\r&w", ch);
      }
      if (wield2)
      {
        wield2->blaster_setting = BLASTER_NORMAL;
        send_to_char( "&YDual wielded blaster set to NORMAL Power\n\r&w", ch);
      }
      return;
   } 
   if ( !str_cmp( argument, "half" ) )
   {
      if (wield)
      {
        wield->blaster_setting = BLASTER_HALF;
        send_to_char( "&YWielded blaster set to HALF Power\n\r&w", ch);
      }
      if (wield2)
      {
        wield2->blaster_setting = BLASTER_HALF;
        send_to_char( "&YDual wielded blaster set to HALF Power\n\r&w", ch);
      }
      return;
   } 
   if ( !str_cmp( argument, "low" ) )
   {
      if (wield)
      {
        wield->blaster_setting = BLASTER_LOW;
        send_to_char( "&YWielded blaster set to LOW Power\n\r&w", ch);
      }
      if (wield2)
      {
        wield2->blaster_setting = BLASTER_LOW;
        send_to_char( "&YDual wielded blaster set to LOW Power\n\r&w", ch);
      }
      return;
   } 
   if ( !str_cmp( argument, "stun" ) )
   {
      if (wield)
      {
        wield->blaster_setting = BLASTER_STUN;
        send_to_char( "&YWielded blaster set to STUN\n\r&w", ch);
      }
      if (wield2)
      {
        wield2->blaster_setting = BLASTER_STUN;
        send_to_char( "&YDual wielded blaster set to STUN\n\r&w", ch);
      }
      return;
   } 
   else
     do_setblaster( ch , "" );

}

void do_use( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char argd[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *device;
    OBJ_DATA *obj;
    ch_ret retcode;

    argument = one_argument( argument, argd );
    argument = one_argument( argument, arg );
    
    if ( !str_cmp( arg , "on" ) )
       argument = one_argument( argument, arg );
    
    if ( argd[0] == '\0' )
    {
	send_to_char( "Use what?\n\r", ch );
	return;
    }

    if ( ( device = get_eq_char( ch, WEAR_HOLD ) ) == NULL ||
       !nifty_is_name(argd, device->name) )
    {
	return;
    }
    
    if ( device->item_type != ITEM_DEVICE )
    {
	send_to_char( "You can't figure out what it is your supposed to do with it.\n\r", ch );
	return;
    }
    
    if ( device->value[2] <= 0 )
    {
        send_to_char( "It has no more charge left.", ch);
        return;
    }
    
    obj = NULL;
    if ( arg[0] == '\0' )
    {
	if ( ch->fighting )
	{
	    victim = who_fighting( ch );
	}
	else
	{
	    send_to_char( "Use on whom or what?\n\r", ch );
	    return;
	}
    }
    else
    {
	if ( ( victim = get_char_room ( ch, arg ) ) == NULL
	&&   ( obj    = get_obj_here  ( ch, arg ) ) == NULL )
	{
	    send_to_char( "You can't find your target.\n\r", ch );
	    return;
	}
    }

    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );

    if ( device->value[2] > 0 )
    {
        device->value[2]--;
	if ( victim )
	{
          if ( !oprog_use_trigger( ch, device, victim, NULL, NULL ) )
          {
	    act( AT_MAGIC, "$n uses $p on $N.", ch, device, victim, TO_ROOM );
	    act( AT_MAGIC, "You use $p on $N.", ch, device, victim, TO_CHAR );
          }
	}
	else
	{
          if ( !oprog_use_trigger( ch, device, NULL, obj, NULL ) )
          {
	    act( AT_MAGIC, "$n uses $p on $P.", ch, device, obj, TO_ROOM );
	    act( AT_MAGIC, "You use $p on $P.", ch, device, obj, TO_CHAR );
          }
	}

	retcode = obj_cast_spell( device->value[3], device->value[0], ch, victim, obj );
	if ( retcode == rCHAR_DIED || retcode == rBOTH_DIED )
	{
	   bug( "do_use: char died", 0 );
	   return;
	}
    }


    return;
}

/*
 * Fill a container
 * Many enhancements added by Thoric (ie: filling non-drink containers)
 */
void do_fill( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *source;
    sh_int    dest_item, src_item1, src_item2, src_item3, src_item4;
    int       diff;
    bool      all = FALSE;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    /* munch optional words */
    if ( (!str_cmp( arg2, "from" ) || !str_cmp( arg2, "with" ))
    &&    argument[0] != '\0' )
	argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Fill what?\n\r", ch );
	return;
    }

    if ( ms_find_obj(ch) )
	return;

    if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }
    else
	dest_item = obj->item_type;

    src_item1 = src_item2 = src_item3 = src_item4 = -1;
    switch( dest_item )
    {
	default:
	  act( AT_ACTION, "$n tries to fill $p... (Don't ask me how)", ch, obj, NULL, TO_ROOM );
	  send_to_char( "You cannot fill that.\n\r", ch );
	  return;
	/* place all fillable item types here */
	case ITEM_DRINK_CON:
	  src_item1 = ITEM_FOUNTAIN;
	case ITEM_CONTAINER:
	  src_item1 = ITEM_CONTAINER;	src_item2 = ITEM_CORPSE_NPC;
	  src_item3 = ITEM_CORPSE_PC;	src_item4 = ITEM_CORPSE_NPC;    break;
    }

    if ( dest_item == ITEM_CONTAINER )
    {
	if ( IS_SET(obj->value[1], CONT_CLOSED) )
	{
	    act( AT_PLAIN, "The $d is closed.", ch, NULL, obj->name, TO_CHAR );
	    return;
	}
	if ( get_obj_weight( obj ) / obj->count
	>=   obj->value[0] )
	{
	   send_to_char( "It's already full as it can be.\n\r", ch );
	   return;
	}
    }
    else
    {
	diff = obj->value[0] - obj->value[1];
	if ( diff < 1 || obj->value[1] >= obj->value[0] )
	{
	   send_to_char( "It's already full as it can be.\n\r", ch );
	   return;
	}
    }

    if ( arg2[0] != '\0' )
    {
      if ( dest_item == ITEM_CONTAINER
      && (!str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 )) )
      {
	all = TRUE;
	source = NULL;
      }
      else
      {
	if ( ( source =  get_obj_here( ch, arg2 ) ) == NULL )
	{
	   send_to_char( "You cannot find that item.\n\r", ch );
	   return;
	}
      }
    }
    else
	source = NULL;

    if ( !source )
    {
	bool      found = FALSE;
	OBJ_DATA *src_next;

	found = FALSE;
	separate_obj( obj );
	for ( source = ch->in_room->first_content;
	      source;
	      source = src_next )
	{
	    src_next = source->next_content;
	    if (dest_item == ITEM_CONTAINER)
	    {
		if ( !CAN_WEAR(source, ITEM_TAKE)
		||   (IS_OBJ_STAT( source, ITEM_PROTOTYPE) && !can_take_proto(ch))
		||    ch->carry_weight + get_obj_weight(source) > can_carry_w(ch)
		||   (get_obj_weight(source) + get_obj_weight(obj)/obj->count)
		    > obj->value[0] )
		  continue;
		if ( all && arg2[3] == '.'
		&&  !nifty_is_name( &arg2[4], source->name ) )
		   continue;
		obj_from_room(source);
		if ( source->item_type == ITEM_MONEY )
		{
		   ch->gold += source->value[0];
		   extract_obj( source );
		}
		else
		   obj_to_obj(source, obj);
		found = TRUE;
	    }
	    else
	    if (source->item_type == src_item1
	    ||  source->item_type == src_item2
	    ||  source->item_type == src_item3
	    ||  source->item_type == src_item4 )
	    {
		found = TRUE;
		break;
	    }
	}
	if ( !found )
	{
	    switch( src_item1 )
	    {
		default:
		  send_to_char( "There is nothing appropriate here!\n\r", ch );
		  return;
		case ITEM_FOUNTAIN:
		  send_to_char( "There is no fountain or pool here!\n\r", ch );
		  return;
	    }
	}
	if (dest_item == ITEM_CONTAINER)
	{
	  act( AT_ACTION, "You fill $p.", ch, obj, NULL, TO_CHAR );
	  act( AT_ACTION, "$n fills $p.", ch, obj, NULL, TO_ROOM );
	  return;
	}
    }

    if (dest_item == ITEM_CONTAINER)
    {
	OBJ_DATA *otmp, *otmp_next;
	char name[MAX_INPUT_LENGTH];
	CHAR_DATA *gch;
	char *pd;
	bool found = FALSE;

	if ( source == obj )
	{
	    send_to_char( "You can't fill something with itself!\n\r", ch );
	    return;
	}

	switch( source->item_type )
	{
	    default:	/* put something in container */
		if ( !source->in_room	/* disallow inventory items */
		||   !CAN_WEAR(source, ITEM_TAKE)
		||   (IS_OBJ_STAT( source, ITEM_PROTOTYPE) && !can_take_proto(ch))
		||    ch->carry_weight + get_obj_weight(source) > can_carry_w(ch)
		||   (get_obj_weight(source) + get_obj_weight(obj)/obj->count)
		    > obj->value[0] )
		{
		    send_to_char( "You can't do that.\n\r", ch );
		    return;
		}
		separate_obj( obj );
		act( AT_ACTION, "You take $P and put it inside $p.", ch, obj, source, TO_CHAR );
		act( AT_ACTION, "$n takes $P and puts it inside $p.", ch, obj, source, TO_ROOM );
		obj_from_room(source);
		obj_to_obj(source, obj);
		break;
	    case ITEM_MONEY:
		send_to_char( "You can't do that... yet.\n\r", ch );
		break;
	    case ITEM_CORPSE_PC:
		if ( IS_NPC(ch) )
		{
		    send_to_char( "You can't do that.\n\r", ch );
		    return;
		}
		
		    pd = source->short_descr;
		    pd = one_argument( pd, name );
		    pd = one_argument( pd, name );
		    pd = one_argument( pd, name );
		    pd = one_argument( pd, name );

		    if ( str_cmp( name, ch->name ) && !IS_IMMORTAL(ch) )
		    {
			bool fGroup;

			fGroup = FALSE;
			for ( gch = first_char; gch; gch = gch->next )
			{
			    if ( !IS_NPC(gch)
			    &&   is_same_group( ch, gch )
			    &&   !str_cmp( name, gch->name ) )
			    {
				fGroup = TRUE;
				break;
			    }
			}
			if ( !fGroup )
			{
			    send_to_char( "That's someone else's corpse.\n\r", ch );
			    return;
			}
		     }
		      
	    case ITEM_CONTAINER:
		if ( source->item_type == ITEM_CONTAINER  /* don't remove */
		&&   IS_SET(source->value[1], CONT_CLOSED) )
		{
		    act( AT_PLAIN, "The $d is closed.", ch, NULL, source->name, TO_CHAR );
		    return;
		}
	    case ITEM_DROID_CORPSE:
	    case ITEM_CORPSE_NPC:
		if ( (otmp=source->first_content) == NULL )
		{
		    send_to_char( "It's empty.\n\r", ch );
		    return;
		}
		separate_obj( obj );
		for ( ; otmp; otmp = otmp_next )
		{
		    otmp_next = otmp->next_content;

		    if ( !CAN_WEAR(otmp, ITEM_TAKE)
		    ||   (IS_OBJ_STAT( otmp, ITEM_PROTOTYPE) && !can_take_proto(ch))
		    ||    ch->carry_number + otmp->count > can_carry_n(ch)
		    ||    ch->carry_weight + get_obj_weight(otmp) > can_carry_w(ch)
		    ||   (get_obj_weight(source) + get_obj_weight(obj)/obj->count)
			> obj->value[0] )
			continue;
		    obj_from_obj(otmp);
		    obj_to_obj(otmp, obj);
		    found = TRUE;
		}
		if ( found )
		{
		   act( AT_ACTION, "You fill $p from $P.", ch, obj, source, TO_CHAR );
		   act( AT_ACTION, "$n fills $p from $P.", ch, obj, source, TO_ROOM );
		}
		else
		   send_to_char( "There is nothing appropriate in there.\n\r", ch );
		break;
	}
	return;
    }

    if ( source->value[1] < 1 )
    {
	send_to_char( "There's none left!\n\r", ch );
	return;
    }
    if ( source->count > 1 && source->item_type != ITEM_FOUNTAIN )
      separate_obj( source );
    separate_obj( obj );

    switch( source->item_type )
    {
	default:
	  bug( "do_fill: got bad item type: %d", source->item_type );
	  send_to_char( "Something went wrong...\n\r", ch );
	  return;
	case ITEM_FOUNTAIN:
	  if ( obj->value[1] != 0 && obj->value[2] != 0 )
	  {
	     send_to_char( "There is already another liquid in it.\n\r", ch );
	     return;
	  }
	  obj->value[2] = 0;
	  obj->value[1] = obj->value[0];
	  act( AT_ACTION, "You fill $p from $P.", ch, obj, source, TO_CHAR );
	  act( AT_ACTION, "$n fills $p from $P.", ch, obj, source, TO_ROOM );
	  return;
	case ITEM_DRINK_CON:
	  if ( obj->value[1] != 0 && obj->value[2] != source->value[2] )
	  {
	     send_to_char( "There is already another liquid in it.\n\r", ch );
	     return;
	  }
	  obj->value[2] = source->value[2];
	  if ( source->value[1] < diff )
	    diff = source->value[1];
	  obj->value[1] += diff;
	  source->value[1] -= diff;
	  act( AT_ACTION, "You fill $p from $P.", ch, obj, source, TO_CHAR );
	  act( AT_ACTION, "$n fills $p from $P.", ch, obj, source, TO_ROOM );
	  return;
    }
}

void do_drink( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int amount;
    int liquid;

    argument = one_argument( argument, arg );
    /* munch optional words */
    if ( !str_cmp( arg, "from" ) && argument[0] != '\0' )
	argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	for ( obj = ch->in_room->first_content; obj; obj = obj->next_content )
	    if ( (obj->item_type == ITEM_FOUNTAIN) )
		break;

	if ( !obj )
	{
	    send_to_char( "Drink what?\n\r", ch );
	    return;
	}
    }
    else
    {
	if ( ( obj = get_obj_here( ch, arg ) ) == NULL )
	{
	    send_to_char( "You can't find it.\n\r", ch );
	    return;
	}
    }

    if ( obj->count > 1 && obj->item_type != ITEM_FOUNTAIN )
	separate_obj(obj);

    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 40 )
    {
	send_to_char( "You fail to reach your mouth.  *Hic*\n\r", ch );
	return;
    }

    switch ( obj->item_type )
    {
    default:
	if ( obj->carried_by == ch )
	{
	    act( AT_ACTION, "$n lifts $p up to $s mouth and tries to drink from it...", ch, obj, NULL, TO_ROOM );
	    act( AT_ACTION, "You bring $p up to your mouth and try to drink from it...", ch, obj, NULL, TO_CHAR );
	}
	else
	{
	    act( AT_ACTION, "$n gets down and tries to drink from $p... (Is $e feeling ok?)", ch, obj, NULL, TO_ROOM );
	    act( AT_ACTION, "You get down on the ground and try to drink from $p...", ch, obj, NULL, TO_CHAR );
	}
	break;

    case ITEM_FOUNTAIN:
	if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
	{
	   act( AT_ACTION, "$n drinks from the fountain.", ch, NULL, NULL, TO_ROOM );
	   send_to_char( "You take a long thirst quenching drink.\n\r", ch );
	}

	if ( !IS_NPC(ch) )
	    ch->pcdata->condition[COND_THIRST] = 40;
	break;

    case ITEM_DRINK_CON:
	if ( obj->value[1] <= 0 )
	{
	    send_to_char( "It is already empty.\n\r", ch );
	    return;
	}

	if ( ( liquid = obj->value[2] ) >= LIQ_MAX )
	{
	    bug( "Do_drink: bad liquid number %d.", liquid );
	    liquid = obj->value[2] = 0;
	}

	if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
	{
	   act( AT_ACTION, "$n drinks $T from $p.",
		ch, obj, liq_table[liquid].liq_name, TO_ROOM );
	   act( AT_ACTION, "You drink $T from $p.",
		ch, obj, liq_table[liquid].liq_name, TO_CHAR );
	}

	amount = 1; /* UMIN(amount, obj->value[1]); */
	/* what was this? concentrated drinks?  concentrated water
	   too I suppose... sheesh! */

	gain_condition( ch, COND_DRUNK,
	    amount * liq_table[liquid].liq_affect[COND_DRUNK  ] );
	gain_condition( ch, COND_FULL,
	    amount * liq_table[liquid].liq_affect[COND_FULL   ] );
	gain_condition( ch, COND_THIRST,
	    amount * liq_table[liquid].liq_affect[COND_THIRST ] );

	if ( !IS_NPC(ch) )
	{
	    if ( ch->pcdata->condition[COND_DRUNK]  > 24 )
		send_to_char( "You feel quite sloshed.\n\r", ch );
	    else
	    if ( ch->pcdata->condition[COND_DRUNK]  > 18 )
		send_to_char( "You feel very drunk.\n\r", ch );
	    else
	    if ( ch->pcdata->condition[COND_DRUNK]  > 12 )
		send_to_char( "You feel drunk.\n\r", ch );
	    else
	    if ( ch->pcdata->condition[COND_DRUNK]  > 8 )
		send_to_char( "You feel a little drunk.\n\r", ch );
	    else
	    if ( ch->pcdata->condition[COND_DRUNK]  > 5 )
		send_to_char( "You feel light headed.\n\r", ch );

	    if ( ch->pcdata->condition[COND_FULL]   > 40 )
		send_to_char( "You are full.\n\r", ch );

	    if ( ch->pcdata->condition[COND_THIRST] > 40 )
		send_to_char( "You feel bloated.\n\r", ch );
	    else
	    if ( ch->pcdata->condition[COND_THIRST] > 36 )
		send_to_char( "Your stomach is sloshing around.\n\r", ch );
	    else
	    if ( ch->pcdata->condition[COND_THIRST] > 30 )
		send_to_char( "You do not feel thirsty.\n\r", ch );
	}

	if ( obj->value[3] )
	{
	    /* The drink was poisoned! */
	    AFFECT_DATA af;

	    act( AT_POISON, "$n sputters and gags.", ch, NULL, NULL, TO_ROOM );
	    act( AT_POISON, "You sputter and gag.", ch, NULL, NULL, TO_CHAR );
	    ch->mental_state = URANGE( 20, ch->mental_state + 5, 100 );
	    af.type      = gsn_poison;
	    af.duration  = 3 * obj->value[3];
	    af.location  = APPLY_NONE;
	    af.modifier  = 0;
	    af.bitvector = AFF_POISON;
	    affect_join( ch, &af );
	}

	obj->value[1] -= amount;
	break;
    }
    WAIT_STATE(ch, PULSE_PER_SECOND );
    return;
}

void do_eat( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    int foodcond;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Eat what?\n\r", ch );
	return;
    }

    if ( IS_NPC(ch) || ch->pcdata->condition[COND_FULL] > 5 )
	if ( ms_find_obj(ch) )
	    return;

    if ( (obj = find_obj(ch, argument, TRUE)) == NULL )
	return;

    if ( !IS_IMMORTAL(ch) )
    {
	if ( obj->item_type != ITEM_FOOD  )
	{
	    act( AT_ACTION, "$n starts to nibble on $p... ($e must really be hungry)",  ch, obj, NULL, TO_ROOM );
	    act( AT_ACTION, "You try to nibble on $p...", ch, obj, NULL, TO_CHAR );
	    return;
	}

	if ( !IS_NPC(ch) && ch->pcdata->condition[COND_FULL] > 40 )
	{
	    send_to_char( "You are too full to eat more.\n\r", ch );
	    return;
	}
    }

    /* required due to object grouping */
    separate_obj( obj );
    
    WAIT_STATE( ch, PULSE_PER_SECOND/2 );
    
    if ( obj->in_obj )
    {
	act( AT_PLAIN, "You take $p from $P.", ch, obj, obj->in_obj, TO_CHAR );
	act( AT_PLAIN, "$n takes $p from $P.", ch, obj, obj->in_obj, TO_ROOM );
    }
    if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
    {
      if ( !obj->action_desc || obj->action_desc[0]=='\0' )
      {
        act( AT_ACTION, "$n eats $p.",  ch, obj, NULL, TO_ROOM );
        act( AT_ACTION, "You eat $p.", ch, obj, NULL, TO_CHAR );
      }
      else
        actiondesc( ch, obj, NULL ); 
    }

    switch ( obj->item_type )
    {

    case ITEM_FOOD:
	if ( obj->timer > 0 && obj->value[1] > 0 )
	   foodcond = (obj->timer * 10) / obj->value[1];
	else
	   foodcond = 10;

	if ( !IS_NPC(ch) )
	{
	    int condition;

	    condition = ch->pcdata->condition[COND_FULL];
	    gain_condition( ch, COND_FULL, (obj->value[0] * foodcond) / 10 );
	    if ( condition <= 1 && ch->pcdata->condition[COND_FULL] > 1 )
		send_to_char( "You are no longer hungry.\n\r", ch );
	    else if ( ch->pcdata->condition[COND_FULL] > 40 )
		send_to_char( "You are full.\n\r", ch );
	}

	if (  obj->value[3] != 0
	||   (foodcond < 4 && number_range( 0, foodcond + 1 ) == 0) )
	{
	    /* The food was poisoned! */
	    AFFECT_DATA af;

	    if ( obj->value[3] != 0 )
	    {
		act( AT_POISON, "$n chokes and gags.", ch, NULL, NULL, TO_ROOM );
		act( AT_POISON, "You choke and gag.", ch, NULL, NULL, TO_CHAR );
		ch->mental_state = URANGE( 20, ch->mental_state + 5, 100 );
	    }
	    else
	    {
		act( AT_POISON, "$n gags on $p.", ch, obj, NULL, TO_ROOM );
		act( AT_POISON, "You gag on $p.", ch, obj, NULL, TO_CHAR );
		ch->mental_state = URANGE( 15, ch->mental_state + 5, 100 );
	    }

	    af.type      = gsn_poison;
	    af.duration  = 2 * obj->value[0]
	    		 * (obj->value[3] > 0 ? obj->value[3] : 1);
	    af.location  = APPLY_NONE;
	    af.modifier  = 0;
	    af.bitvector = AFF_POISON;
	    affect_join( ch, &af );
	}
	break;

    }

    if ( obj->serial == cur_obj )
      global_objcode = rOBJ_EATEN;
    extract_obj( obj );
    return;
}

void do_empty( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( !str_cmp( arg2, "into" ) && argument[0] != '\0' )
	argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Empty what?\n\r", ch );
	return;
    }
    if ( ms_find_obj(ch) )
	return;

    if ( (obj = get_obj_carry( ch, arg1 )) == NULL )
    {
	send_to_char( "You aren't carrying that.\n\r", ch );
	return;
    }
    if ( obj->count > 1 )
      separate_obj(obj);

    switch( obj->item_type )
    {
	default:
	  act( AT_ACTION, "You shake $p in an attempt to empty it...", ch, obj, NULL, TO_CHAR );
	  act( AT_ACTION, "$n begins to shake $p in an attempt to empty it...", ch, obj, NULL, TO_ROOM );
	  return;
	case ITEM_DRINK_CON:
	  if ( obj->value[1] < 1 )
	  {
		send_to_char( "It's already empty.\n\r", ch );
		return;
	  }
	  act( AT_ACTION, "You empty $p.", ch, obj, NULL, TO_CHAR );
	  act( AT_ACTION, "$n empties $p.", ch, obj, NULL, TO_ROOM );
	  obj->value[1] = 0;
	  return;
	case ITEM_CONTAINER:
	  if ( IS_SET(obj->value[1], CONT_CLOSED) )
	  {
		act( AT_PLAIN, "The $d is closed.", ch, NULL, obj->name, TO_CHAR );
		return;
	  }
	  if ( !obj->first_content )
	  {
		send_to_char( "It's already empty.\n\r", ch );
		return;
	  }
	  if ( arg2[0] == '\0' )
	  {
		if ( !IS_NPC(ch) &&  IS_SET( ch->act, PLR_LITTERBUG ) )
		{
		       set_char_color( AT_MAGIC, ch );
		       send_to_char( "A magical force stops you!\n\r", ch );
		       set_char_color( AT_TELL, ch );
		       send_to_char( "Someone tells you, 'No littering here!'\n\r", ch );
		       return;
		}
		if ( empty_obj( obj, NULL, ch->in_room ) )
		{
		    act( AT_ACTION, "You empty $p.", ch, obj, NULL, TO_CHAR );
		    act( AT_ACTION, "$n empties $p.", ch, obj, NULL, TO_ROOM );
		    if ( IS_SET( sysdata.save_flags, SV_DROP ) )
			save_char_obj( ch );
		}
		else
		    send_to_char( "Hmmm... didn't work.\n\r", ch );
	  }
	  else
	  {
		OBJ_DATA *dest = get_obj_here( ch, arg2 );

		if ( !dest )
		{
		    send_to_char( "You can't find it.\n\r", ch );
		    return;
		}
		if ( dest == obj )
		{
		    send_to_char( "You can't empty something into itself!\n\r", ch );
		    return;
		}
		if ( dest->item_type != ITEM_CONTAINER )
		{
		    send_to_char( "That's not a container!\n\r", ch );
		    return;
		}
		if ( IS_SET(dest->value[1], CONT_CLOSED) )
		{
		    act( AT_PLAIN, "The $d is closed.", ch, NULL, dest->name, TO_CHAR );
		    return;
		}
		separate_obj( dest );
		if ( empty_obj( obj, dest, NULL ) )
		{
		    act( AT_ACTION, "You empty $p into $P.", ch, obj, dest, TO_CHAR );
		    act( AT_ACTION, "$n empties $p into $P.", ch, obj, dest, TO_ROOM );
		    if ( !dest->carried_by
		    &&    IS_SET( sysdata.save_flags, SV_PUT ) )
			save_char_obj( ch );
		}
		else
		    act( AT_ACTION, "$P is too full.", ch, obj, dest, TO_CHAR );
	  }
	  return;
    }
}
 
void actiondesc( CHAR_DATA *ch, OBJ_DATA *obj, void *vo )
{
    char charbuf[MAX_STRING_LENGTH];
    char roombuf[MAX_STRING_LENGTH];
    char *srcptr = obj->action_desc;
    char *charptr = charbuf;
    char *roomptr = roombuf;
    const char *ichar;
    const char *iroom;

while ( *srcptr != '\0' )
{
  if ( *srcptr == '$' ) 
  {
    srcptr++;
    switch ( *srcptr )
    {
      case 'e':
        ichar = "you";
        iroom = "$e";
        break;

      case 'm':
        ichar = "you";
        iroom = "$m";
        break;

      case 'n':
        ichar = "you";
        iroom = "$n";
        break;

      case 's':
        ichar = "your";
        iroom = "$s";
        break;

      /*case 'q':
        iroom = "s";
        break;*/

      default: 
        srcptr--;
        *charptr++ = *srcptr;
        *roomptr++ = *srcptr;
        break;
    }
  }
  else if ( *srcptr == '%' && *++srcptr == 's' ) 
  {
    ichar = "You";
    iroom = IS_NPC( ch ) ? ch->short_descr : ch->name;
  }
  else
  {
    *charptr++ = *srcptr;
    *roomptr++ = *srcptr;
    srcptr++;
    continue;
  }

  while ( ( *charptr = *ichar ) != '\0' )
  {
    charptr++;
    ichar++;
  }

  while ( ( *roomptr = *iroom ) != '\0' )
  {
    roomptr++;
    iroom++;
  }
  srcptr++;
}

*charptr = '\0';
*roomptr = '\0';

/*
sprintf( buf, "Charbuf: %s", charbuf );
log_string_plus( buf, LOG_HIGH, LEVEL_LESSER ); 
sprintf( buf, "Roombuf: %s", roombuf );
log_string_plus( buf, LOG_HIGH, LEVEL_LESSER ); 
*/

switch( obj->item_type )
{
  case ITEM_FOUNTAIN:
    act( AT_ACTION, charbuf, ch, obj, ch, TO_CHAR );
    act( AT_ACTION, roombuf, ch, obj, ch, TO_ROOM );
    return;

  case ITEM_DRINK_CON:
    act( AT_ACTION, charbuf, ch, obj, liq_table[obj->value[2]].liq_name, TO_CHAR );
    act( AT_ACTION, roombuf, ch, obj, liq_table[obj->value[2]].liq_name, TO_ROOM );
    return;

  case ITEM_ARMOR:
  case ITEM_WEAPON:
  case ITEM_LIGHT:
    act( AT_ACTION, charbuf, ch, obj, ch, TO_CHAR );
    act( AT_ACTION, roombuf, ch, obj, ch, TO_ROOM );
    return;
 
  case ITEM_FOOD:
    act( AT_ACTION, charbuf, ch, obj, ch, TO_CHAR );
    act( AT_ACTION, roombuf, ch, obj, ch, TO_ROOM );
    return;

  default:
    return;
}
return;
}

void do_hail( CHAR_DATA *ch , char *argument )
{
    int vnum;
    ROOM_INDEX_DATA *room;
    
    if ( !ch->in_room )
       return;

    if ( !ch->in_room->area || !ch->in_room->area->planet )
    {
       send_to_char( "There doesn't seem to be any taxis nearby!\n\r", ch );    
       return;
    }
    
    if ( ch->position < POS_FIGHTING )
    {
       send_to_char( "You might want to stop fighting first!\n\r", ch );
       return;
    }
    
    if ( ch->position < POS_STANDING )
    {
       send_to_char( "You might want to stand up first!\n\r", ch );
       return;
    }
    
    if ( IS_SET( ch->in_room->room_flags , ROOM_INDOORS ) )
    {
       send_to_char( "You'll have to go outside to do that!\n\r", ch );
       return;
    }
    
    if ( IS_SET( ch->in_room->room_flags , ROOM_SPACECRAFT ) )
    {
       send_to_char( "You can't do that on spacecraft!\n\r", ch );
       return;
    }

    if ( ch->gold < (ch->top_level-9)  )
    {
       send_to_char( "You don't have enough credits!\n\r", ch );
       return;
    }

    vnum = ch->in_room->vnum;
    
    for ( room = ch->in_room->area->first_room  ;  room  ;  room = room->next_in_area )
    {
             if ( IS_SET(room->room_flags , ROOM_HOTEL ) )
                break;   
    }
    
    if ( room == NULL || !IS_SET(room->room_flags , ROOM_HOTEL ) )
    {
       send_to_char( "There doesn't seem to be any taxis nearby!\n\r", ch );
       return;
    }
    
    ch->gold -= UMAX(ch->top_level-9 , 0);
    
    act( AT_ACTION, "$n hails a speederbike, and drives off to seek shelter.", ch, NULL, NULL,  TO_ROOM );

    char_from_room( ch );
    char_to_room( ch, room );
   
    send_to_char( "A speederbike picks you up and drives you to a safe location.\n\rYou pay the driver 20 credits.\n\r\n\n" , ch );
    act( AT_ACTION, "$n $T", ch, NULL, "arrives on a speederbike, gets off and pays the driver before it leaves.",  TO_ROOM );
                               
    do_look( ch, "auto" );
   
}

void do_suicide( CHAR_DATA *ch, char *argument )
{
        char  logbuf[MAX_STRING_LENGTH];
        
        if ( IS_NPC(ch) || !ch->pcdata )
        {
            send_to_char( "Yeah right!\n\r", ch );
	    return;
        }
        
        if ( argument[0] == '\0' )
        {
            send_to_char( "&RIf you really want to delete this character type suicide and your password.\n\r", ch );
	    return;
        }
        
        if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
	{
	    send_to_char( "Sorry wrong password.\n\r", ch );
	    sprintf( logbuf , "%s attempting to commit suicide... WRONG PASSWORD!" , ch->name );
	    log_string( logbuf );
	    return;
	}

       act( AT_BLOOD, "With a sad determination and trembling hands you slit your own throat!",  ch, NULL, NULL, TO_CHAR    );
       act( AT_BLOOD, "Cold shivers run down your spine as you watch $n slit $s own throat!",  ch, NULL, NULL, TO_ROOM );

       sprintf( logbuf , "%s has committed suicide.." , ch->name );
       log_string( logbuf );

       set_cur_char(ch);
       raw_kill( ch, ch );
            
}

void do_bank( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    long amount = 0;
    
    argument = one_argument( argument , arg1 );
    argument = one_argument( argument , arg2 );
    
    if ( IS_NPC(ch) || !ch->pcdata )
       return;
    
    if (!ch->in_room || !IS_SET(ch->in_room->room_flags, ROOM_BANK) )
    {
       send_to_char( "You must be in a bank to do that!\n\r", ch );
       return;
    }

    if ( arg1[0] == '\0' )
    {
       send_to_char( "Usage: BANK <deposit|withdraw|ballance> [amount]\n\r", ch );
       return;
    }

    if (arg2[0] != '\0' )
        amount = atoi(arg2);

    if ( !str_prefix( arg1 , "deposit" ) )
    {
       if ( amount  <= 0 )
       {
          send_to_char( "You may only deposit amounts greater than zero.\n\r", ch );
          do_bank( ch , "" );
          return;
       }
       
       if ( ch->gold < amount )
       {
          send_to_char( "You don't have that many credits on you.\n\r", ch );
          return;
       }
       
       ch->gold -= amount;
       ch->pcdata->bank += amount;
       
       ch_printf( ch , "You deposit %ld credits into your account.\n\r" ,amount );
       return;
    }
    else if ( !str_prefix( arg1 , "withdrawl" ) )
    {
       if ( amount  <= 0 )
       {
          send_to_char( "You may only withdraw amounts greater than zero.\n\r", ch );
          do_bank( ch , "" );
          return;
       }
     
       if ( ch->pcdata->bank < amount )
       {
          send_to_char( "You don't have that many credits in your account.\n\r", ch );
          return;
       }

       ch->gold += amount;
       ch->pcdata->bank -= amount;
       
       ch_printf( ch , "You withdraw %ld credits from your account.\n\r" ,amount );
       return;

    }
    else if ( !str_prefix( arg1 , "ballance" ) )
    {
        ch_printf( ch , "You have %ld credits in your account.\n\r" , ch->pcdata->bank );
        return; 
    }
    else
    {
      do_bank( ch , "" );
      return;
    }
    
        
}

void do_dig( CHAR_DATA *ch, char *argument )
{
    char arg [MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *startobj;
    bool found, shovel;
    EXIT_DATA *pexit;
   
    switch( ch->substate )
    {
	default:
	  if ( IS_NPC(ch)  && IS_AFFECTED( ch, AFF_CHARM ) )
	  {
	    send_to_char( "You can't concentrate enough for that.\n\r", ch );
	    return;
	  }
          if ( ch->mount )
	  {
	    send_to_char( "You can't do that while mounted.\n\r", ch );
	    return;
	  }
	  one_argument( argument, arg );
	  if ( arg[0] != '\0' )
	  {
	      if ( ( pexit = find_door( ch, arg, TRUE ) ) == NULL
	      &&     get_dir(arg) == -1 )
	      {
		  send_to_char( "What direction is that?\n\r", ch );
		  return;
	      }
	      if ( pexit )
	      {
		  if ( !IS_SET(pexit->exit_info, EX_DIG)
		  &&   !IS_SET(pexit->exit_info, EX_CLOSED) )
		  {
		      send_to_char( "There is no need to dig out that exit.\n\r", ch );
		      return;
		  }
	      }
	  }
	  else
	  {
	      switch( ch->in_room->sector_type )
	      {
		  case SECT_CITY:
		  case SECT_INSIDE:
		    send_to_char( "The floor is too hard to dig through.\n\r", ch );
		    return;
		  case SECT_WATER_SWIM:
		  case SECT_WATER_NOSWIM:
		  case SECT_UNDERWATER:
		    send_to_char( "You cannot dig here.\n\r", ch );
		    return;
		  case SECT_AIR:
		    send_to_char( "What?  In the air?!\n\r", ch );
		    return;
	      }
	  }
	  add_timer( ch, TIMER_DO_FUN, 3, do_dig, 1);
	  ch->dest_buf = str_dup( arg );
	  send_to_char( "You begin digging...\n\r", ch );
 	  act( AT_PLAIN, "$n begins digging...", ch, NULL, NULL, TO_ROOM );
	  return;

	case 1:
	  if ( !ch->dest_buf )
	  {
	      send_to_char( "Your digging was interrupted!\n\r", ch );
	      act( AT_PLAIN, "$n's digging was interrupted!", ch, NULL, NULL, TO_ROOM );
	      bug( "do_dig: dest_buf NULL", 0 );
	      return;
	  }
	  strcpy( arg, ch->dest_buf );  
	  DISPOSE( ch->dest_buf );	
	  break;

	case SUB_TIMER_DO_ABORT:
	  DISPOSE( ch->dest_buf );
	  ch->substate = SUB_NONE;
	  send_to_char( "You stop digging...\n\r", ch );
	  act( AT_PLAIN, "$n stops digging...", ch, NULL, NULL, TO_ROOM );
	  return;
    }

    ch->substate = SUB_NONE;

    if ( number_percent() == 23 )
    {
	send_to_char( "You feel a little bit stronger...\n\r", ch );
        ch->perm_str++;
        ch->perm_str = UMIN( ch->perm_str , 25 );
    }
    
    /* not having a shovel makes it harder to succeed */
    shovel = FALSE;
    for ( obj = ch->first_carrying; obj; obj = obj->next_content )
      if ( obj->item_type == ITEM_SHOVEL )
      {
	  shovel = TRUE;
	  break;
      }

    /* dig out an EX_DIG exit... */
    if ( arg[0] != '\0' )
    {
	if ( ( pexit = find_door( ch, arg, TRUE ) ) != NULL
	&&     IS_SET( pexit->exit_info, EX_DIG )
	&&     IS_SET( pexit->exit_info, EX_CLOSED ) )
	{
	    /* 4 times harder to dig open a passage without a shovel */
	    if ( (number_percent() * (shovel ? 1 : 4)) < 80 ) 
	    {
		REMOVE_BIT( pexit->exit_info, EX_CLOSED );
		send_to_char( "You dig open a passageway!\n\r", ch );
		act( AT_PLAIN, "$n digs open a passageway!", ch, NULL, NULL, TO_ROOM );
		return;
	    }
	}
	send_to_char( "Your dig did not discover any exit...\n\r", ch );
	act( AT_PLAIN, "$n's dig did not discover any exit...", ch, NULL, NULL, TO_ROOM );
	return;
    }

    startobj = ch->in_room->first_content;
    found = FALSE;
    
    for ( obj = startobj; obj; obj = obj->next_content )
    {
	/* twice as hard to find something without a shovel */
	if ( IS_OBJ_STAT( obj, ITEM_BURRIED )
	&&  (number_percent() * (shovel ? 1 : 2)) < 80 )
	{
	  found = TRUE;
	  break;
	}
    }

    if ( !found )
    {
	send_to_char( "Your dig uncovered nothing.\n\r", ch );
	act( AT_PLAIN, "$n's dig uncovered nothing.", ch, NULL, NULL, TO_ROOM );
	return;
    }

    separate_obj(obj);
    REMOVE_BIT( obj->extra_flags, ITEM_BURRIED );
    act( AT_SKILL, "Your dig uncovered $p!", ch, obj, NULL, TO_CHAR );
    act( AT_SKILL, "$n's dig uncovered $p!", ch, obj, NULL, TO_ROOM );
    
    return;
} 

    
void do_search( CHAR_DATA *ch, char *argument )
{
    char arg  [MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *container;
    OBJ_DATA *startobj;
    int percent, door;
    bool found, room;

    door = -1;
    switch( ch->substate )
    {
	default:
	    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
	    {
		send_to_char( "You can't concentrate enough for that.\n\r", ch );
		return;
	    }
	    if ( ch->mount )
	    {
		send_to_char( "You can't do that while mounted.\n\r", ch );
		return;
	    }
	    argument = one_argument( argument, arg );
	    if ( arg[0] != '\0' && (door = get_door( arg )) == -1 )
	    {
		container = get_obj_here( ch, arg );
		if ( !container )
		{
		  send_to_char( "You can't find that here.\n\r", ch );
		  return;
		}
		if ( container->item_type != ITEM_CONTAINER )
		{
		  send_to_char( "You can't search in that!\n\r", ch );
		  return;
		}
		if ( IS_SET(container->value[1], CONT_CLOSED) )
		{
		  send_to_char( "It is closed.\n\r", ch );
		  return;
		}
	    }
	    add_timer( ch, TIMER_DO_FUN, 3,
		       do_search, 1 );
	    send_to_char( "You begin your search...\n\r", ch );
	    ch->dest_buf = str_dup( arg );
	    return;

	case 1:
	    if ( !ch->dest_buf )
	    {
		send_to_char( "Your search was interrupted!\n\r", ch );
		bug( "do_search: dest_buf NULL", 0 );
		return;
	    }
	    strcpy( arg, ch->dest_buf );
	    DISPOSE( ch->dest_buf );
	    break;
	case SUB_TIMER_DO_ABORT:
	    DISPOSE( ch->dest_buf );
	    ch->substate = SUB_NONE;
	    send_to_char( "You stop your search...\n\r", ch );
	    return;
    }
    
    if ( number_percent() == 23 )
    {
	send_to_char( "You feel a little bit wiser...\n\r", ch );
        ch->perm_wis++;
        ch->perm_wis = UMIN( ch->perm_wis , 25 );
    }

    ch->substate = SUB_NONE;
    if ( arg[0] == '\0' )
    {
	room = TRUE;
	startobj = ch->in_room->first_content;
    }
    else
    {
	if ( (door = get_door( arg )) != -1 )
	  startobj = NULL;
	else
	{
	    container = get_obj_here( ch, arg );
	    if ( !container )
	    {
		send_to_char( "You can't find that here.\n\r", ch );
		return;
	    }
	    startobj = container->first_content;
	}
    }

    found = FALSE;

    if ( (!startobj && door == -1) || IS_NPC(ch) )
    {
	send_to_char( "You find nothing.\n\r", ch );
	return;
    }

    percent  = number_percent( );

    if ( door != -1 )
    {
	EXIT_DATA *pexit;
	
	if ( (pexit = get_exit( ch->in_room, door )) != NULL
	&&   IS_SET( pexit->exit_info, EX_SECRET )
	&&   IS_SET( pexit->exit_info, EX_xSEARCHABLE )
	&&   percent < 80 )
	{
	    act( AT_SKILL, "Your search reveals the $d!", ch, NULL, pexit->keyword, TO_CHAR );
	    act( AT_SKILL, "$n finds the $d!", ch, NULL, pexit->keyword, TO_ROOM );
	    REMOVE_BIT( pexit->exit_info, EX_SECRET );
	    return;
	}
    }
    else
    for ( obj = startobj; obj; obj = obj->next_content )
    {
       if ( IS_OBJ_STAT( obj, ITEM_HIDDEN )
       &&   percent < 70 )
       {
	  found = TRUE;
	  break;
       }
    }

    if ( !found )
    {
       send_to_char( "You find nothing.\n\r", ch );
       return;
    }

    separate_obj(obj);
    REMOVE_BIT( obj->extra_flags, ITEM_HIDDEN );
    act( AT_SKILL, "Your search reveals $p!", ch, obj, NULL, TO_CHAR );
    act( AT_SKILL, "$n finds $p!", ch, obj, NULL, TO_ROOM );
    return;
}
