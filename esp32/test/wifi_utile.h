/*
 * wifi_utile.h
 *
 *  Created on: 9 avr. 2026
 *      Author: Nath
 */

#ifndef MAIN_WIFI_UTILE_H_
#define MAIN_WIFI_UTILE_H_


 
#include <stdbool.h>
 
/**
 * @brief Initialise et connecte le WiFi en mode station.
 *        Bloque jusqu'à l'obtention d'une IP ou l'échec max de retries.
 * @return true si connecté, false si échec
 */
bool wifi_manager_init(void);
 
/**
 * @brief Retourne true si le WiFi est actuellement connecté
 */
bool wifi_manager_is_connected(void);


#endif /* MAIN_WIFI_UTILE_H_ */
