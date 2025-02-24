#include <cstdint>
#include <iot_board.h>
#include "state_t.h"
#include "LoRaMesh/LoRaMesh.h"

state_t stato = st_ormeggio;
const uint8_t idBarca = (uint8_t)random(1, 255);
/*const uint8_t idBarca = 0x01;*/
const uint8_t idGabbiotto = 0xff;
String s = "";
int i = 0;
bool printDisplay = false;

void simulaBarca();
void onReceive(LoRaMesh_message_t message);

void setup() {
    IoTBoard::init_serial();
    IoTBoard::init_buttons();
    IoTBoard::init_display();

    if(!LoRaMesh::init(idBarca, onReceive)) {
        Serial.println("Errore nell'avvio di LoRa");
        ESP.restart();
    }
    Serial.println("Dispositivo avviato");
    display->println("Dispositivo avviato");
    display->display();
    Serial.println("Indirizzo LoRa di questa barca: " + String(idBarca));
    display->println(idBarca);


}

void loop() {
    if(printDisplay) {
        printDisplay = false;
        display->display();
    }
    simulaBarca();
    switch(stato) {
        case st_ormeggio: {
            int ret = LoRaMesh::sendMessage(idGabbiotto, stato);
            if(ret == LORA_MESH_MESSAGE_QUEUE_FULL) {
                Serial.println("La coda e' piena");
            }
            else if (ret == LORA_MESH_MESSAGE_SENT_SUCCESS) {
                Serial.println("Il messaggio e' stato ricevuto con successo");
            }
            Serial.println("Io sto bene");
            break;
        }
        case st_movimento:
            LoRaMesh::sendMessage(idGabbiotto, stato);
            Serial.println("Mi hanno rubato");
            break;
    }
    LoRaMesh::update();
    delay(1000);
}

void onReceive(LoRaMesh_message_t message) {
    display->clearDisplay();
    display->setCursor(0, 0);
    display->printf("Destinatario: %d\n", (message.destinatario));
    display->printf("Mittente: %d\n", (message.mittente));
    display->printf("Id Messaggio: %d\n", (message.message_id));
    display->printf("PayLoad: %d\n", (message.payload));
    printDisplay = true;
}

void simulaBarca() {
    int rand = random(100);
    if(rand == 5) {
        stato = st_movimento;
    }
}
