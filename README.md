# V-USB Arduino port (20121206)

Allows an Arduino to act as midi device through 2nd (firmware-only) usb port.

# V-USB

[V-USB](http://www.obdev.at/products/vusb/index.html) by Objective Development Software GmbH
	http://www.obdev.at/products/vusb/index.html
	
# Circuit
![Arduino & V-USB](https://github.com/TechFactoryHU/vusb-arduino/circuits/arduino_vusb_dev.png)
![Arduino & V-USB](https://github.com/TechFactoryHU/vusb-arduino/circuits/arduino_vusb_dev_schematic.png)

Default config:
	D- = Pin 7
	D+ = Pin 2

You can change default values in src/usbconfig.h file. For more information please visit [Obdev's site] (https://www.obdev.at/products/vusb/index.html)
	

# Midi sample
```C++
#include <TFUsbMidi.h>
unsigned long ms = 0;

void setup() {
	Serial.begin(115200);
	//setup midi msg callback
	VUsbMidi.OnMsg(OnMidiMessage);
    VUsbMidi.begin();
}

void loop() {
	//watch for new midi messages
	VUsbMidi.refresh();
  
	//send some midi messages at every 5sec
	if (ms < millis()) {
		Serial.println("SendNoteOn");
		//NoteOn message: channel, note, velocity
		VUsbMidi.NoteOn(1, 60, 100);
		delay(200);
		//NoteOff message: channel, note
		VUsbMidi.NoteOff(1, 60);
		
		//ControlChange message: channel, ctrlid, value
		VUsbMidi.ControlChange(2, 10, 54);
		
		//Send raw message v1:
		TFMidiMessage midimsg;
		midimsg.type = TFMidiType.NoteOn;
		midimsg.channel = 1;
		midimsg.data1  = 60; //note
		midimsg.data2  = 50; //velocity
		VUsbMidi.write(midimsg);
		
		//send raw message v2
		byte buffer[4];
		buffer[0] = 0x09;
		buffer[1] = 0x90 | 1; // message type | channel
		buffer[2] = 0x7f & 60; //note
		buffer[3] = 0x7f & 50; //velocity
		VUsbMidi.write(buffer,4);
		
		ms = millis() + 5000; 
	}
}

//this function will be called on every midi message
void OnMidiMessage(TFMidiMessage msg) {
    Serial.print(msg.type);
    Serial.print("\t");
    Serial.print(msg.channel);
    Serial.print("\t");
    Serial.print(msg.data1);
    Serial.print("\t");
    Serial.println(msg.data2);
}
```

	
# TFMidiUsb wrapper

Wrapper class based on:

- V-USB-MIDI
	http://cryptomys.de/horo/V-USB-MIDI/
	
- vusb-for-arduino 
	https://code.google.com/p/vusb-for-arduino/downloads/list

- Arduino MIDI Library
	https://github.com/FortySevenEffects/arduino_midi_library

- mimuz-avr-core
	https://github.com/mimuz/mimuz-avr-core/blob/master/arduino/libraries/VUSBMidiATtiny


