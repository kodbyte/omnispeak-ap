//ap_client.h
#ifndef AP_CLIENT_H
#define AP_CLIENT_H

void ap_client_init(void);
void ap_client_poll(void);
void ap_client_location_check(int location_id);
bool ap_is_checked(int id);
bool ap_has_item(int item_id);
void ap_client_give_item(int item_id);
void ap_apply_level_items(int level, int ep);
void ap_reapply_item(int local_id);
void ap_resync_items();
void ap_datastorage_set_level(int level, int episode);
bool ap_announce_victory(bool keen4done, bool keen5done);
void ap_send_death(const char* cause);

#endif