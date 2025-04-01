#include <cstdint>
#include <iot_board.h>
#include "LoRaMesh/state_t.h"
#include "LoRaMesh/LoRaMesh.h"

state_t stato = st_ormeggio;
const char targa[7] = {'A', 'B', '1', '2', '3', 'X', 'Y'};
const char targaGabbiotto[7] = {'A', 'B', '1', '2', '3', 'X', 'Y'};
const bool isGabbiotto = true;
LoRaMesh_payload_t payload;
/*const uint8_t idBarca = 0x01;*/
String s = "";
int i = 0;
int counterBatteria = 0;
bool printDisplay = false;
int livelloBatteria;

void simulaBarca();
void onReceive(LoRaMesh_message_t message);

void setup() {
    IoTBoard::init_serial();
    IoTBoard::init_buttons();
    IoTBoard::init_display();

    if(!LoRaMesh::init(targa, onReceive)) {
        Serial.println("Errore nell'avvio di LoRa");
        ESP.restart();
    }
    if(isGabbiotto) {
        Serial.println("Dispositivo avviato come gabbiotto");
        display->println("Dispositivo avviato come gabbiotto");
    }
    else {
        Serial.println("Dispositivo avviato come barca");
        display->println("Dispositivo avviato come barca");
    }
    Serial.print("Targa di questo dispositivo: ");
    for(int i = 0; i < 7; i++) {
        Serial.print(targa[i]);
    }
    livelloBatteria = random(50, 100);
    display->println("Livello batteria corrente: " + String(livelloBatteria) + "%");
    display->display();

}

void loop() {
    if(printDisplay) {
        printDisplay = false;
        display->display();
    }
    if(!isGabbiotto) {
        simulaBarca();
        switch(stato) {
            payload.stato = stato;
            payload.livello_batteria = livelloBatteria;
            case st_ormeggio: {
                int ret = LoRaMesh::sendMessage(targaGabbiotto, payload);
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
                LoRaMesh::sendMessage(targaGabbiotto, payload);
                Serial.println("Mi hanno rubato");
                break;
            default:
                break;
        }
    }
    LoRaMesh::update();
    delay(1000);
}

void onReceive(LoRaMesh_message_t message) {
    display->clearDisplay();
    display->setCursor(0, 0);
    display->printf("Targa mittente: ");
    for(int i = 0; i < 7; i++) {
        display->printf("%c", message.targa_mittente[i]);
    }
    display->printf("\n");
    display->printf("Id Messaggio: %d\n", (message.message_id));
    display->printf("Livello batteria: %d\n", payload.livello_batteria);
    display->printf("Stato barca: %s\n", payload.stato == st_ormeggio ? "parcheggiata" : "rubata");
    printDisplay = true;
}

void simulaBarca() {
    counterBatteria++;
    if(counterBatteria % 30 == 0) {
        livelloBatteria--;
        if(livelloBatteria == 0) {
            ESP.restart();
        }
    }
    int rand = random(100);
    if(rand == 5) {
        stato = st_movimento;
    }
}
