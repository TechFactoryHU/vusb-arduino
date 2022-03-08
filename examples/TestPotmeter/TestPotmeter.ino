#include <TFUsbMidi.h>

void setup() {
    Serial.begin(115200);
    Serial.println("Setup");
   
    Serial.println("VUSB setup onmsg callback");
    VUsbMidi.OnMsg(OnMidiMessage);
    
    Serial.println("VUSB begin");
    VUsbMidi.begin(false);
}

unsigned long ms;
int potmeters[3] = {A0,A1,A2};
int potmeter_last_value[3] = {0,0,0};

void loop() {
  //watch for midi packets
  VUsbMidi.refresh();

  int tmp;
  for (int i=0; i<3; i++) {
     tmp = analogRead(potmeters[i]);
     if (tmp != potmeter_last_value[i]) {
        OnPotmeterValueChanged(potmeters[i], tmp);
        potmeter_last_value[i] = tmp;
        Serial.println("Potmeter ("+String(potmeters[i])+") value changed to "+String(tmp));
     }
  }
}

void OnPotmeterValueChanged(int analog, int value) { 
   //map ADC 0-1024 value to 0-127 
  int velocity = map(value, 0, 1024, 0, 127);

  if (analog == A0) {
      //channel, note, velocity
      VUsbMidi.NoteOn(1, 60, velocity);
  }
  if (analog == A1) {
      //channel, note, velocity
      VUsbMidi.NoteOn(1, 61, velocity);
  }
  if (analog == A2) {
      //channel, note, velocity
      VUsbMidi.NoteOn(1, 62, velocity);
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
