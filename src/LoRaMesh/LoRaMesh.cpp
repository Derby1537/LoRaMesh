#include "LoRaMesh/LoRaMesh.h"
#include <cstring>

char LoRaMesh::targa[7] = {0,0,0,0,0,0,0};
LoRaMesh_message_t LoRaMesh::messageToSend = {0};
LoRaMesh_message_t LoRaMesh::messageToRedirect = {0};
CircularQueue<uint16_t> LoRaMesh::queue;
void (*LoRaMesh::userOnReceiveCallBack)(LoRaMesh_message_t) = nullptr;

bool LoRaMesh::init(char targa[7], void (*userOnReceiveCallBack)(LoRaMesh_message_t)) {
    for(int i = 0; i < 7; i++) {
        LoRaMesh::targa[i] = targa[i];

    }
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

    // Il messaggio non è per noi, lo reinviamo
    if(memcmp(message.targa_destinatario, LoRaMesh::targa, 7) != 0) {
        messageToRedirect = message;
        return;
    }

    // Altrimenti lo diamo al main
    userOnReceiveCallBack(message);
}

int LoRaMesh::sendMessage(char targa_destinatario[7], LoRaMesh_payload_t payload) {
    // Stiamo già inviando un messaggio. non conviene inviare altri messaggi
    if(messageToSend.message_id != 0) {
        return LORA_MESH_MESSAGE_QUEUE_FULL;
    }

    messageToSend = {
        .message_id = (uint16_t)random(1, 65535),
        .payload = payload,
    };
    for(int i = 0; i < 7; i++) {
        messageToSend.targa_destinatario[i] =  targa_destinatario[i];
        messageToSend.targa_mittente[i] = LoRaMesh::targa[i];
    }

    return LORA_MESH_MESSAGE_SENT_SUCCESS;
}

void LoRaMesh::sendMessagePrivate(LoRaMesh_message_t message) {
    lora->beginPacket();
    lora->write((uint8_t*)&message, sizeof(LoRaMesh_message_t));
    lora->endPacket();
}

