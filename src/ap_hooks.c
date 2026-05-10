#include <stdio.h>
#include "ap_hooks.h"
#include "ap_defs.h"
#include "ap_client.h"
#include "id_sd.h"
#include "id_ca.h"
#include "id_us.h"
#include "id_vh.h"
#include "id_in.h"
#include "ck_def.h"
#include "ck_play.h"

int ap_current_level = -1;
int ap_current_episode = -1;
int ap_starting_points = 0;
int ap_points_gained = 0;
bool ap_has_pogo = 0;
bool ap_has_stunner = 0;
bool ap_force_abort = 0;

bool ap_death_link_enabled = false;
bool ap_pending_death = false;
bool ap_suppress_death_send = false;

void ap_on_level_complete(void)
{
	int location_id = LOC_LEVEL_COMPLETE(ap_current_episode, ap_current_level);
	ap_client_location_check(location_id);
	
	bool keen4done = ap_is_checked(LOC_LEVEL_COMPLETE(AP_EPISODE_CK4, AP_LEVEL_BEAN_WITH_BACON_MEGAROCKET));
	bool keen5done = ap_is_checked(LOC_LEVEL_COMPLETE(AP_EPISODE_CK5, AP_LEVEL_QUANTUM_EXPLOSION_DYNAMO));
	ap_announce_victory(keen4done, keen5done);
}

void ap_on_keygem_get(int item)
{
	int location_id = LOC_KEYGEM(ap_current_episode, ap_current_level, item);
	ap_client_location_check(location_id);
}

void ap_on_security_card_get(void)
{
	int location_id = LOC_SECURITY_KEYCARD(ap_current_episode, ap_current_level);
	ap_client_location_check(location_id);

}

void ap_on_score_increase(int score)
{
	ap_points_gained = score - ap_starting_points;
}

static const char *ap_death_msgs_generic[] = {
	"Commander Keen was hit",
	"Commander Keen forgot to dodge",
	"Commander Keen got Slugged",
	"Commander Keen's helmet wasn't on tight enough",
	"Commander Keen took a Wormouth to the face",
	"Commander Keen ran out of lifewater",
	"Commander Keen's mom is going to be mad",
	"Billy Blaze got grounded for a week",
	"Commander Keen learned not to touch the spiky things",
	"Commander Keen's eight-year-old genius failed him",
	"Commander Keen got a little too curious",
	"Commander Keen needed more pickles",
};

static const char *ap_death_msgs_eaten[] = {
	"Commander Keen got Dopefish'd",
	"Commander Keen was today's special",
	"Commander Keen learned why Dopefish is so happy",
	"Commander Keen swam too close to the buffet",
	"Commander Keen forgot fish gotta eat too",
};

static const char *ap_death_msgs_fell[] = {
	"Commander Keen took the express elevator down",
	"Commander Keen discovered gravity, the hard way",
	"Commander Keen failed his orbital re-entry",
	"Commander Keen jumped one time too many",
	"Commander Keen forgot to pack a parachute",
};

const char* ap_random_death_message(int kind)
{
	const char **arr;
	int n;
	switch (kind)
	{
		case AP_DEATH_EATEN:
			arr = ap_death_msgs_eaten;
			n = (int)(sizeof(ap_death_msgs_eaten) / sizeof(ap_death_msgs_eaten[0]));
			break;
		case AP_DEATH_FELL:
			arr = ap_death_msgs_fell;
			n = (int)(sizeof(ap_death_msgs_fell) / sizeof(ap_death_msgs_fell[0]));
			break;
		default:
			arr = ap_death_msgs_generic;
			n = (int)(sizeof(ap_death_msgs_generic) / sizeof(ap_death_msgs_generic[0]));
			break;
	}
	return arr[US_RndT() % n];
}

void ap_on_death(const char* cause)
{
	if (!ap_death_link_enabled)
		return;
	if (ap_suppress_death_send)
	{
		// This kill was triggered by an incoming DeathLink — don't echo it back.
		ap_suppress_death_send = false;
		return;
	}
	ap_send_death(cause ? cause : "Commander Keen died");
}

void ap_apply_pending_death(void)
{
	if (!ap_pending_death)
		return;
	// Drop pending if we can't apply it right now. Holding it across the
	// death animation would cause a second death immediately after respawn.
	if (ck_gameState.levelState != LS_Playing
	    || ck_invincibilityTimer != 0
	    || ck_godMode)
	{
		ap_pending_death = false;
		return;
	}

	ap_pending_death = false;
	ap_suppress_death_send = true;
	CK_KillKeen();
}

bool ap_has_level(int level, int ep)
{
	int ap_level_to_item_ck4[]{
		0,
		AP_ITEM_BV,
		AP_ITEM_SV,
		AP_ITEM_PP,
		AP_ITEM_COTD,
		AP_ITEM_COC,
		AP_ITEM_CRYS,
		AP_ITEM_HI,
		AP_ITEM_SY,
		AP_ITEM_MIR,
		AP_ITEM_LO,
		AP_ITEM_POTM,
		AP_ITEM_POS,
		AP_ITEM_POTGA,
		AP_ITEM_POTF,
		AP_ITEM_IOT,
		AP_ITEM_IOF,
		AP_ITEM_WOW,
		AP_ITEM_BWBMR
	};

	int ap_level_to_item_ck5[]{
		0,
		AP_ITEM_IVS,
		AP_ITEM_SC,
		AP_ITEM_DTV,
		AP_ITEM_EFS,
		AP_ITEM_DTB,
		AP_ITEM_RCC,
		AP_ITEM_DTS,
		AP_ITEM_NBI,
		AP_ITEM_DTT,
		AP_ITEM_BMI,
		AP_ITEM_GDH,
		AP_ITEM_QED
	};

	if (ep == 1)
		if (level <= 17)
		{
			return ap_has_item(ap_level_to_item_ck4[level]);
		}
		else
		{
			//bwbm should require all other levels at least reachable
			for (int i = 1; i <= 18; i++)
			{
				if (i == 14) continue; //skip pyramid of the forbidden
				if (!ap_has_item(ap_level_to_item_ck4[i]))
					return false;
			}
			return true;
		}

	if (ep == 2)
		return ap_has_item(ap_level_to_item_ck5[level]);

	return false;

}

void ap_open_blocks(void)
{
    int x, y;
    uint16_t *pw;
    int flags;

    pw = CA_TilePtrAtPos(0, 0, 2); // info layer

    for (y = 0; y < CA_GetMapHeight(); y++)
    {
        for (x = 0; x < CA_GetMapWidth(); x++, pw++)
        {
            flags = (*pw) >> 8;

            if (flags == 0xD0)
            {
                // remove blocking foreground tile
                CA_SetTileAtPos(x, y, 1, 0);
            }
        }
    }
}

void ap_show_message(const char* msg)
{
	US_CenterWindow(20, 3);
	US_PrintCentered(msg);
	VH_UpdateScreen();
	IN_WaitButton();
}