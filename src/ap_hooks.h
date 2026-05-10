//ap_hooks.h
#ifndef AP_HOOKS_H
#define AP_HOOKS_H

extern int ap_current_level;
extern int ap_current_episode;
extern int ap_starting_points;
extern int ap_points_gained;
extern bool ap_has_pogo;
extern bool ap_has_stunner;
extern bool ap_force_abort;

extern bool ap_death_link_enabled;
extern bool ap_pending_death;
extern bool ap_suppress_death_send;

void ap_on_level_complete(void);
void ap_on_keygem_get(int item);
void ap_on_security_card_get(void);
void ap_on_wetsuit_get(void);
void ap_on_score_increase(int points);
void ap_on_death(const char* cause);
void ap_apply_pending_death(void);

// Death message categories for ap_random_death_message().
#define AP_DEATH_GENERIC 0
#define AP_DEATH_EATEN   1
#define AP_DEATH_FELL    2
const char* ap_random_death_message(int kind);
bool ap_has_level(int level, int episode);
void ap_open_blocks(void);
void ap_show_message(const char* msg);

#endif