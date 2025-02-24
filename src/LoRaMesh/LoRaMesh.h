#ifndef LORAMESH_H
#define LORAMESH_H

#include <cstdint>
#define INDIRIZZO_BROADCAST 0xff
#define LORA_MESH_MESSAGE_SENT_UNSUCCESS 1
#define LORA_MESH_MESSAGE_SENT_SUCCESS 0
#define LORA_MESH_MESSAGE_QUEUE_FULL -1

#include <iot_board.h>
#include "CircularQueue/CircularQueue.h"

typedef struct {
    uint8_t destinatario;
    uint8_t mittente;
    uint8_t message_id;
    uint8_t payload;
} LoRaMesh_message_t;

class LoRaMesh {
public:
    LoRaMesh() = delete;

    static bool init(int id, void (*userOnReceiveCallBack)(LoRaMesh_message_t));
    static int sendMessage(uint8_t destination, uint8_t payload);
    static void update();
private:
    static CircularQueue<uint8_t> queue;
    static uint8_t id; 
    static void onReceive(int packetSize);
    static void (*userOnReceiveCallBack)(LoRaMesh_message_t message);
    static void sendMessagePrivate(LoRaMesh_message_t message);
    static LoRaMesh_message_t messageToSend;
    static LoRaMesh_message_t messageToRedirect;
};

#endif
