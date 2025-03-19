#include "LoRaMesh/LoRaMesh.h"

char LoRaMesh::targa[7] = {0,0,0,0,0,0,0};
uint8_t LoRaMesh::is_gabbiotto = 0;
LoRaMesh_message_t LoRaMesh::messageToSend = {0};
LoRaMesh_message_t LoRaMesh::messageToRedirect = {0};
CircularQueue<uint16_t> LoRaMesh::queue;
void (*LoRaMesh::userOnReceiveCallBack)(LoRaMesh_message_t) = nullptr;

bool LoRaMesh::init(const char targa[7], uint8_t is_gabbiotto, void (*userOnReceiveCallBack)(LoRaMesh_message_t)) {
    for(int i = 0; i < 7; i++) {
        LoRaMesh::targa[i] = targa[i];

    }
    LoRaMesh::is_gabbiotto = is_gabbiotto;
    LoRaMesh::messageToSend = {0};
    LoRaMesh::messageToRedirect = {0};
    LoRaMesh::userOnReceiveCallBack = userOnReceiveCallBack;
    LoRaMesh::queue = {};
    IoTBoard::init_spi();
    if(!IoTBoard::init_lora()) {
        return 0;
    }
    lora->onReceive(LoRaMesh::onReceive);
    lora->receive();
    return 1;
}

void LoRaMesh::update() {
    lora->receive();
    if (messageToSend.message_id != 0) {
        sendMessagePrivate(messageToSend);
        messageToSend = {0};
    }
    lora->receive();
    if (messageToRedirect.message_id != 0) {
        sendMessagePrivate(messageToRedirect);
        messageToRedirect = {0};
    }
}

void LoRaMesh::onReceive(int packetSize) {
    // Il pacchetto è vuoto
    if(packetSize == 0) {
        return;
    }

    if(packetSize != sizeof(LoRaMesh_message_t)) {
        return;
    }

    // Leggiamo tutto il messaggio e lo inseriamo all'interno di una variabile temporanea message
    LoRaMesh_message_t message;
    lora->readBytes((uint8_t*)&message, sizeof(LoRaMesh_message_t)); 

    if(queue.getIndex(message.message_id) >= 0) {
        return;
    }
    queue.push(message.message_id);

    if(!is_gabbiotto) {
        int rssi = lora->rssi();
        message.cum_RSSI += rssi;
        messageToRedirect = message;
        return;
    }

    userOnReceiveCallBack(message);
}

int LoRaMesh::sendMessage(const char id_gabbiotto[7], LoRaMesh_payload_t payload) {
    // Stiamo già inviando un messaggio. non conviene inviare altri messaggi
    if(messageToSend.message_id != 0) {
        return LORA_MESH_MESSAGE_QUEUE_FULL;
    }

    messageToSend = {
        .message_id = (uint16_t)random(1, 65535),
        .cum_RSSI = 0,
        .payload = payload,
    };
    for(int i = 0; i < 7; i++) {
        messageToSend.id_gabbiotto[i] = id_gabbiotto[i];
        messageToSend.targa[i] = LoRaMesh::targa[i];
    }

    return LORA_MESH_MESSAGE_SENT_SUCCESS;
}

void LoRaMesh::sendMessagePrivate(LoRaMesh_message_t message) {
    lora->beginPacket();
    lora->write((uint8_t*)&message, sizeof(LoRaMesh_message_t));
    lora->endPacket();
}

float LoRaMesh::calculateDistance(int rssi) {
    return rssi * rssi_multiplier;
}
