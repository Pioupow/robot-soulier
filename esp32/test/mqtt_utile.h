/*
 * mqtt_utile.h
 *
 *  Created on: 9 avr. 2026
 *      Author: Nath
 */

#ifndef MAIN_MQTT_UTILE_H_
#define MAIN_MQTT_UTILE_H_

#include <stdint.h>
#include <stdbool.h>
 
/* ── Config broker ───────────────────────────────────────────────────────── */
#define MQTT_BROKER_URI     "mqtt://172.17.15.103:1883"
 
/* ── Topics ──────────────────────────────────────────────────────────────── */
#define MQTT_TOPIC_SUB      "nate/commande"   // ESP32 écoute ce topic
#define MQTT_TOPIC_PUB      "nate/status"     // ESP32 publie sur ce topic
 
/* ── Callback reçu quand un message arrive sur MQTT_TOPIC_SUB ────────────── */
typedef void (*mqtt_message_cb_t)(const char *topic,   int topic_len,
                                  const char *data,    int data_len);
 
/**
 * @brief Initialise et démarre le client MQTT.
 *        À appeler après wifi_manager_init().
 * @param cb  Fonction appelée à chaque message reçu (peut être NULL)
 */
void mqtt_manager_init(mqtt_message_cb_t cb);
 
/**
 * @brief Publie un message sur MQTT_TOPIC_PUB
 * @param data     Payload (string)
 * @param qos      0, 1 ou 2
 * @param retain   0 ou 1
 */
void mqtt_manager_publish(const char *data, int qos, int retain);
 
/**
 * @brief Retourne true si le client est connecté au broker
 */
bool mqtt_manager_is_connected(void);



#endif /* MAIN_MQTT_UTILE_H_ */
