#ifndef __tfusbmidi_h_
#define __tfusbmidi_h_

#include <arduino.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <string.h>



#include "vusb/usbdrv.h"
#include <util/delay.h>     /* for _delay_ms() */



/*
*	Buffer implementation from
*	https://github.com/mimuz/mimuz-avr-core/blob/master/arduino/libraries/VUSBMidiATtiny/queue.c
*/

#define VUSBMIDI_RINGBUFFER_SIZE 8
#define VUSBMIDI_RINGBUFFER_ESIZE 4

/*
*	Midi types from
*	https://github.com/FortySevenEffects/arduino_midi_library
*/

enum TFMidiType {
	InvalidType           = 0x00,    ///< For notifying errors
    NoteOff               = 0x80,    ///< Note Off
    NoteOn                = 0x90,    ///< Note On
    AfterTouchPoly        = 0xA0,    ///< Polyphonic AfterTouch
    ControlChange         = 0xB0,    ///< Control Change / Channel Mode
    ProgramChange         = 0xC0,    ///< Program Change
    AfterTouchChannel     = 0xD0,    ///< Channel (monophonic) AfterTouch
    PitchBend             = 0xE0,    ///< Pitch Bend
    SystemExclusive       = 0xF0,    ///< System Exclusive
    TimeCodeQuarterFrame  = 0xF1,    ///< System Common - MIDI Time Code Quarter Frame
    SongPosition          = 0xF2,    ///< System Common - Song Position Pointer
    SongSelect            = 0xF3,    ///< System Common - Song Select
    TuneRequest           = 0xF6,    ///< System Common - Tune Request
    Clock                 = 0xF8,    ///< System Real Time - Timing Clock
    Start                 = 0xFA,    ///< System Real Time - Start
    Continue              = 0xFB,    ///< System Real Time - Continue
    Stop                  = 0xFC,    ///< System Real Time - Stop
    ActiveSensing         = 0xFE,    ///< System Real Time - Active Sensing
    SystemReset           = 0xFF,    ///< System Real Time - System Reset
};

struct TFMidiMessage {
	TFMidiType type;
	byte channel;
	byte data1;
	byte data2;
};

class TFUsbMidi {
	private:
		uint8_t buffer[VUSBMIDI_RINGBUFFER_SIZE*VUSBMIDI_RINGBUFFER_ESIZE];
		uint8_t _bufftop;
		uint8_t _bufflast;
		uint8_t _buffsize;
		
		uint8_t buffNext(uint8_t value);
		uint8_t buffPush(uint8_t *a);
		uint8_t* buffPop(void);
		
		void (*_onMsgCallback)(TFMidiMessage);
		
		void processMessage(void);
		TFMidiType getMessageType(byte *p);
		
	public:
		TFUsbMidi (void);
		void OnMsg(void (*onMsgCallback)(TFMidiMessage));
		
		void begin();
		void refresh();
		void read(uchar *data, uchar len);
		
		void NoteOn(byte ch, byte note, byte velocity);
		void NoteOff(byte ch, byte note);
		void ControlChange(byte ch, byte num, byte value);
		
		
		void write(TFMidiMessage msg);
		void write(byte *buffer, byte size);
};

extern TFUsbMidi VUsbMidi;

#endif // __tfusbmidi_h_
