// This #include statement was automatically added by the Particle IDE.
#include <PMS5003_RK_ilab.h>

// Include Particle Device OS APIs
#include "Particle.h"

// Set your 3rd-party SIM APN here
// https://docs.particle.io/reference/firmware/electron/#setcredentials-
//STARTUP(cellular_credentials_set("internet.movistar.mx", "movistar", "movistar", NULL));
//STARTUP(cellular_credentials_set("internet.itelcel.com", "webgprs", "webgprs2002", NULL));
//STARTUP(cellular_credentials_set("mega45g.com", "", "", NULL));

// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(AUTOMATIC);

// Show system, cloud connectivity, and application logs over USB
// View logs with CLI using 'particle serial monitor --follow'
SerialLogHandler logHandler(LOG_LEVEL_INFO);

SYSTEM_THREAD(ENABLED);

PMS5003_RK pms;

unsigned long lastLogMillis = 0;
const std::chrono::milliseconds logPeriod = 5s;

unsigned long lastPublishMillis = 0;
const std::chrono::milliseconds publishPeriod = 15min;
unsigned long lastPublishLastDataMillis = 0;


// This is where your SET signal is connected, in V1.0 design PCB.
// Reset //Module reset signal /TTL level@3.3V, low reset
const pin_t RESET_PIN = D8;
// This is where your SET signal is connected, in V1.0 design PCB.
// Set pin /TTL level@3.3v, high level or suspending is normal working status, while low level is sleeping mode
const pin_t SET_PIN = D7;


void setup() {
    // Set the keep-alive value for 3rd party SIM card here
    // https://docs.particle.io/reference/firmware/electron/#particle-keepalive-
    //Particle.keepAlive(290);  //Movistar
    //Particle.keepAlive(45);     //Telcel
    pinMode(RESET_PIN, OUTPUT); //Reset pin
    pinMode(SET_PIN, OUTPUT); //Set pin 
    digitalWrite(RESET_PIN, HIGH);
    digitalWrite(SET_PIN, HIGH);
    
    waitFor(Serial.isConnected, 10000); delay(2000);

    pms.setup();
}

void loop() {
    if (millis() - lastLogMillis >= logPeriod.count()) {
        lastLogMillis = millis();

        PMS5003_RK::Data lastData;
        unsigned long lastDataMillis;

        pms.getLastData(lastData, lastDataMillis);
        Log.info("pms %s %d", lastData.toString().c_str(), lastDataMillis);

    }

    if (Particle.connected() && (lastPublishMillis == 0 || (millis() - lastPublishMillis >= publishPeriod.count()))) {
        lastPublishMillis = millis();

        PMS5003_RK::Data lastData;
        unsigned long lastDataMillis;

        pms.getLastData(lastData, lastDataMillis);
        if (lastDataMillis != lastPublishLastDataMillis) {
            lastPublishLastDataMillis = lastDataMillis;
            
            char buf[512];
            memset(buf, 0, sizeof(buf));
            JSONBufferWriter writer(buf, sizeof(buf) - 1);

            writer.beginObject();   
            lastData.toJSON(writer);
            writer.endObject();

            Particle.publish("pmsSensor", buf);
            Log.info("published %s", buf);
        }

    }

}