/*
 * mqtt_utile.c
 *
 *  Created on: 9 avr. 2026
 *      Author: Nath
 */

 #include "mqtt_utile.h"
  
 #include "esp_log.h"
 #include "mqtt_client.h"
  
 /* ── Privé ───────────────────────────────────────────────────────────────── */
 static const char *TAG = "mqtt_manager";
  
 static esp_mqtt_client_handle_t s_client    = NULL;
 static mqtt_message_cb_t        s_user_cb   = NULL;
 static bool                     s_connected = false;
  
 /* ── Event handler ───────────────────────────────────────────────────────── */
 static void _mqtt_event_handler(void *handler_args, esp_event_base_t base,
                                  int32_t event_id, void *event_data)
 {
     esp_mqtt_event_handle_t event  = (esp_mqtt_event_handle_t)event_data;
     esp_mqtt_client_handle_t client = event->client;
  
     switch ((esp_mqtt_event_id_t)event_id) {
  
     case MQTT_EVENT_CONNECTED:
         s_connected = true;
         ESP_LOGI(TAG, "Connecté au broker");
         esp_mqtt_client_subscribe(client, MQTT_TOPIC_SUB, 0);
         ESP_LOGI(TAG, "Subscribed à: %s", MQTT_TOPIC_SUB);
         break;
  
     case MQTT_EVENT_DISCONNECTED:
         s_connected = false;
         ESP_LOGW(TAG, "Déconnecté du broker");
         break;
  
     case MQTT_EVENT_SUBSCRIBED:
         ESP_LOGI(TAG, "Subscribe confirmé, msg_id=%d", event->msg_id);
         break;
  
     case MQTT_EVENT_PUBLISHED:
         ESP_LOGI(TAG, "Publish confirmé, msg_id=%d", event->msg_id);
         break;
  
     case MQTT_EVENT_DATA:
         ESP_LOGI(TAG, "Message reçu → topic: %.*s", event->topic_len, event->topic);
         ESP_LOGI(TAG, "                data:  %.*s", event->data_len,  event->data);
         /* Appel du callback utilisateur si fourni */
         if (s_user_cb != NULL) {
             s_user_cb(event->topic, event->topic_len,
                       event->data,  event->data_len);
         }
         break;
  
     case MQTT_EVENT_ERROR:
         ESP_LOGE(TAG, "Erreur MQTT");
         if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
             ESP_LOGE(TAG, "Errno transport: %d (%s)",
                      event->error_handle->esp_transport_sock_errno,
                      strerror(event->error_handle->esp_transport_sock_errno));
         } else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
             ESP_LOGE(TAG, "Connexion refusée: 0x%x",
                      event->error_handle->connect_return_code);
         }
         break;
  
     default:
         break;
     }
 }
  
 /* ── API publique ────────────────────────────────────────────────────────── */
 void mqtt_manager_init(mqtt_message_cb_t cb)
 {
     s_user_cb = cb;
  
     const esp_mqtt_client_config_t mqtt_cfg = {
         .broker = {
             .address.uri = MQTT_BROKER_URI,
         },
     };
  
     s_client = esp_mqtt_client_init(&mqtt_cfg);
     esp_mqtt_client_register_event(s_client, ESP_EVENT_ANY_ID,
                                    _mqtt_event_handler, NULL);
     esp_mqtt_client_start(s_client);
  
     ESP_LOGI(TAG, "Client MQTT démarré → %s", MQTT_BROKER_URI);
 }
  
 void mqtt_manager_publish(const char *data, int qos, int retain)
 {
     if (s_client == NULL || !s_connected) {
         ESP_LOGW(TAG, "Publish ignoré — pas connecté");
         return;
     }
     int msg_id = esp_mqtt_client_publish(s_client, MQTT_TOPIC_PUB,
                                           data, 0, qos, retain);
     ESP_LOGI(TAG, "Publish → %s : \"%s\" (msg_id=%d)", MQTT_TOPIC_PUB, data, msg_id);
 }
  
 bool mqtt_manager_is_connected(void)
 {
     return s_connected;
 }


