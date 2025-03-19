#ifndef LORAMESH_H
#define LORAMESH_H

#include <cstdint>
#define INDIRIZZO_BROADCAST 0xff
#define LORA_MESH_MESSAGE_SENT_UNSUCCESS 1
#define LORA_MESH_MESSAGE_SENT_SUCCESS 0
#define LORA_MESH_MESSAGE_QUEUE_FULL -1

#include <iot_board.h>
#include "CircularQueue/CircularQueue.h"
#include "state_t.h"

#define rssi_multiplier -0.5

typedef struct {
    state_t stato;
    uint8_t livello_batteria;
} LoRaMesh_payload_t;

typedef struct {
    char id_gabbiotto[7];
    char targa[7];
    uint16_t message_id;
    int32_t cum_RSSI;
    LoRaMesh_payload_t payload;
} LoRaMesh_message_t;

class LoRaMesh {
public:
    LoRaMesh() = delete;

    static bool init(const char targa[7], uint8_t is_gabbiotto, void (*userOnReceiveCallBack)(LoRaMesh_message_t));
    static int sendMessage(const char id_gabbiotto[7], LoRaMesh_payload_t payload);
    static float calculateDistance(int rssi);
    static void update();
private:
    static CircularQueue<uint16_t> queue;
    static char targa[7]; 
    static uint8_t is_gabbiotto;
    static void onReceive(int packetSize);
    static void (*userOnReceiveCallBack)(LoRaMesh_message_t message);
    static void sendMessagePrivate(LoRaMesh_message_t message);
    static LoRaMesh_message_t messageToSend;
    static LoRaMesh_message_t messageToRedirect;
};

#endif
