#include <TFUsbMidi.h>

void setup() {
    Serial.begin(115200);
    Serial.println("Setup");

    Serial.println("VUSB setup onmsg callback");
    VUsbMidi.OnMsg(OnMidiMessage);
    
    Serial.println("VUSB begin");
    VUsbMidi.begin();
    
}

unsigned long ms;

void loop() {
  //watch for midi packets
  VUsbMidi.refresh();
  
  //send some midi messages on every 5sec
  if (ms < millis()) {
    //send note on:  channel, note, velocity
    VUsbMidi.NoteOn(1, 60, 100);
    delay(200);
    //send note off: channel, note
    VUsbMidi.NoteOff(1, 60);
    //send control change message: channel, ctrlid, value
    VUsbMidi.ControlChange(2, 10, 54);
    
    //Send raw message v1:
    TFMidiMessage midimsg;
    midimsg.type = TFMidiType::NoteOn;
    midimsg.channel = 1;
    midimsg.data1  = 60; //note
    midimsg.data2  = 50; //velocity
    VUsbMidi.write(midimsg);
    
    //send raw message v2
    byte buffer[4];
    buffer[0] = 0x09;
    buffer[1] = 0x90 | 1; // message type | channel
    buffer[2] = 0x7f & 30; //note
    buffer[3] = 0x7f & 50; //velocity
    VUsbMidi.write(buffer,4);

    ms = millis() + 5000; 
  }
}


void OnMidiMessage(TFMidiMessage msg) {
    Serial.print(msg.type);
    Serial.print("\t");
    Serial.print(msg.channel);
    Serial.print("\t");
    Serial.print(msg.data1);
    Serial.print("\t");
    Serial.println(msg.data2);
}

