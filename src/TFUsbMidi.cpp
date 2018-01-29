#include <arduino.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h> 
#include <util/delay.h>    
#include <avr/eeprom.h>
#include <avr/pgmspace.h>

#include "vusb/usbdrv.h"
#include "TFUsbMidi.h"


/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */
static uchar    currentAddress;
static uchar    bytesRemaining;
#ifdef __cplusplus
extern "C"{
#endif 

static const PROGMEM char deviceDescrMIDI[] = {	/* USB device descriptor */
	18,			/* sizeof(usbDescriptorDevice): length of descriptor in bytes */
	USBDESCR_DEVICE,	/* descriptor type */
	0x10, 0x01,		/* USB version supported */
	0,			/* device class: defined at interface level */
	0,			/* subclass */
	0,			/* protocol */
	8,			/* max packet size */
	USB_CFG_VENDOR_ID,	/* 2 bytes */
	USB_CFG_DEVICE_ID,	/* 2 bytes */
	USB_CFG_DEVICE_VERSION,	/* 2 bytes */
	1,			/* manufacturer string index */
	2,			/* product string index */
	0,			/* serial number string index */
	1,			/* number of configurations */
};

// B.2 Configuration Descriptor
static const PROGMEM char configDescrMIDI[] = {	/* USB configuration descriptor */
	9,			/* sizeof(usbDescrConfig): length of descriptor in bytes */
	USBDESCR_CONFIG,	/* descriptor type */
	101, 0,			/* total length of data returned (including inlined descriptors) */
	2,			/* number of interfaces in this configuration */
	1,			/* index of this configuration */
	0,			/* configuration name string index */
	0,
	//USBATTR_BUSPOWER,
	USB_CFG_MAX_BUS_POWER / 2,	/* max USB current in 2mA units */

// B.3 AudioControl Interface Descriptors
// The AudioControl interface describes the device structure (audio function topology)
// and is used to manipulate the Audio Controls. This device has no audio function
// incorporated. However, the AudioControl interface is mandatory and therefore both
// the standard AC interface descriptor and the classspecific AC interface descriptor
// must be present. The class-specific AC interface descriptor only contains the header
// descriptor.

// B.3.1 Standard AC Interface Descriptor
// The AudioControl interface has no dedicated endpoints associated with it. It uses the
// default pipe (endpoint 0) for all communication purposes. Class-specific AudioControl
// Requests are sent using the default pipe. There is no Status Interrupt endpoint provided.
	/* AC interface descriptor follows inline: */
	9,			/* sizeof(usbDescrInterface): length of descriptor in bytes */
	USBDESCR_INTERFACE,	/* descriptor type */
	0,			/* index of this interface */
	0,			/* alternate setting for this interface */
	0,			/* endpoints excl 0: number of endpoint descriptors to follow */
	1,			/* */
	1,			/* */
	0,			/* */
	0,			/* string index for interface */

// B.3.2 Class-specific AC Interface Descriptor
// The Class-specific AC interface descriptor is always headed by a Header descriptor
// that contains general information about the AudioControl interface. It contains all
// the pointers needed to describe the Audio Interface Collection, associated with the
// described audio function. Only the Header descriptor is present in this device
// because it does not contain any audio functionality as such.
	/* AC Class-Specific descriptor */
	9,			/* sizeof(usbDescrCDC_HeaderFn): length of descriptor in bytes */
	36,			/* descriptor type */
	1,			/* header functional descriptor */
	0x0, 0x01,		/* bcdADC */
	9, 0,			/* wTotalLength */
	1,			/* */
	1,			/* */

// B.4 MIDIStreaming Interface Descriptors

// B.4.1 Standard MS Interface Descriptor
	/* interface descriptor follows inline: */
	9,			/* length of descriptor in bytes */
	USBDESCR_INTERFACE,	/* descriptor type */
	1,			/* index of this interface */
	0,			/* alternate setting for this interface */
	2,			/* endpoints excl 0: number of endpoint descriptors to follow */
	1,			/* AUDIO */
	3,			/* MS */
	0,			/* unused */
	0,			/* string index for interface */

// B.4.2 Class-specific MS Interface Descriptor
	/* MS Class-Specific descriptor */
	7,			/* length of descriptor in bytes */
	36,			/* descriptor type */
	1,			/* header functional descriptor */
	0x0, 0x01,		/* bcdADC */
	65, 0,			/* wTotalLength */

// B.4.3 MIDI IN Jack Descriptor
	6,			/* bLength */
	36,			/* descriptor type */
	2,			/* MIDI_IN_JACK desc subtype */
	1,			/* EMBEDDED bJackType */
	1,			/* bJackID */
	0,			/* iJack */

	6,			/* bLength */
	36,			/* descriptor type */
	2,			/* MIDI_IN_JACK desc subtype */
	2,			/* EXTERNAL bJackType */
	2,			/* bJackID */
	0,			/* iJack */

//B.4.4 MIDI OUT Jack Descriptor
	9,			/* length of descriptor in bytes */
	36,			/* descriptor type */
	3,			/* MIDI_OUT_JACK descriptor */
	1,			/* EMBEDDED bJackType */
	3,			/* bJackID */
	1,			/* No of input pins */
	2,			/* BaSourceID */
	1,			/* BaSourcePin */
	0,			/* iJack */

	9,			/* bLength of descriptor in bytes */
	36,			/* bDescriptorType */
	3,			/* MIDI_OUT_JACK bDescriptorSubtype */
	2,			/* EXTERNAL bJackType */
	4,			/* bJackID */
	1,			/* bNrInputPins */
	1,			/* baSourceID (0) */
	1,			/* baSourcePin (0) */
	0,			/* iJack */


// B.5 Bulk OUT Endpoint Descriptors

//B.5.1 Standard Bulk OUT Endpoint Descriptor
	9,			/* bLenght */
	USBDESCR_ENDPOINT,	/* bDescriptorType = endpoint */
	0x1,			/* bEndpointAddress OUT endpoint number 1 */
	3,			/* bmAttributes: 2:Bulk, 3:Interrupt endpoint */
	8, 0,			/* wMaxPacketSize */
	10,			/* bIntervall in ms */
	0,			/* bRefresh */
	0,			/* bSyncAddress */

// B.5.2 Class-specific MS Bulk OUT Endpoint Descriptor
	5,			/* bLength of descriptor in bytes */
	37,			/* bDescriptorType */
	1,			/* bDescriptorSubtype */
	1,			/* bNumEmbMIDIJack  */
	1,			/* baAssocJackID (0) */


//B.6 Bulk IN Endpoint Descriptors

//B.6.1 Standard Bulk IN Endpoint Descriptor
	9,			/* bLenght */
	USBDESCR_ENDPOINT,	/* bDescriptorType = endpoint */
	0x81,			/* bEndpointAddress IN endpoint number 1 */
	3,			/* bmAttributes: 2: Bulk, 3: Interrupt endpoint */
	8, 0,			/* wMaxPacketSize */
	10,			/* bIntervall in ms */
	0,			/* bRefresh */
	0,			/* bSyncAddress */

// B.6.2 Class-specific MS Bulk IN Endpoint Descriptor
	5,			/* bLength of descriptor in bytes */
	37,			/* bDescriptorType */
	1,			/* bDescriptorSubtype */
	1,			/* bNumEmbMIDIJack (0) */
	3,			/* baAssocJackID (0) */
};


/* ------------------------------------------------------------------------- */

uchar usbFunctionDescriptor(usbRequest_t * rq)
{
	if (rq->wValue.bytes[1] == USBDESCR_DEVICE) {
		usbMsgPtr = (uchar *) deviceDescrMIDI;
		return sizeof(deviceDescrMIDI);
	}
	else {		
		/* must be config descriptor */
		usbMsgPtr = (uchar *) configDescrMIDI;
		return sizeof(configDescrMIDI);
	}
}

usbMsgLen_t usbFunctionSetup(uchar data[8]) {
	usbRequest_t *rq = (usbRequest_t*)((void *)data);
	// HID class request 
	if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS){    
		if(rq->bRequest == USBRQ_HID_GET_REPORT){ 
			return 1; 
        }else if(rq->bRequest == USBRQ_HID_SET_REPORT){
        }
    }else{
        //ignore vendor type requests, we don't use any 
    }
    return 0xff;
}


uchar usbFunctionRead(uchar *data, uchar len)
{
	return 7;
}

uchar usbFunctionWrite(uchar * data, uchar len)
{
  return 1;
}

void usbFunctionWriteOut(uchar *data, uchar len)
{
	VUsbMidi.read(data, len);
}

/* ------------------------------------------------------------------------- */
/* ------------------------ Oscillator Calibration ------------------------- */
/* ------------------------------------------------------------------------- */

/* Calibrate the RC oscillator to 8.25 MHz. The core clock of 16.5 MHz is
 * derived from the 66 MHz peripheral clock by dividing. Our timing reference
 * is the Start Of Frame signal (a single SE0 bit) available immediately after
 * a USB RESET. We first do a binary search for the OSCCAL value and then
 * optimize this value with a neighboorhod search.
 * This algorithm may also be used to calibrate the RC oscillator directly to
 * 12 MHz (no PLL involved, can therefore be used on almost ALL AVRs), but this
 * is wide outside the spec for the OSCCAL value and the required precision for
 * the 12 MHz clock! Use the RC oscillator calibrated to 12 MHz for
 * experimental purposes only!
 */
 
static void calibrateOscillator(void)
{
	uchar       step = 128;
	uchar       trialValue = 0, optimumValue;
	int         x, optimumDev, targetValue = (unsigned)(1499 * (double)F_CPU / 10.5e6 + 0.5);

	/* do a binary search: */
    do {
        OSCCAL = trialValue + step;
        x = usbMeasureFrameLength();    /* proportional to current real frequency */
        if(x < targetValue)             /* frequency still too low */
            trialValue += step;
        step >>= 1;
    } while (step > 0);

    /* We have a precision of +/- 1 for optimum OSCCAL here */
    /* now do a neighborhood search for optimum value */
    optimumValue = trialValue;
    optimumDev = x; /* this is certainly far away from optimum */
    for(OSCCAL = trialValue - 1; OSCCAL <= trialValue + 1; OSCCAL++) {
        x = usbMeasureFrameLength() - targetValue;
        if(x < 0) x = -x;
        if (x < optimumDev) {
            optimumDev = x;
            optimumValue = OSCCAL;
        }
    }
    OSCCAL = optimumValue;
}

/*
Note: This calibration algorithm may try OSCCAL values of up to 192 even if
the optimum value is far below 192. It may therefore exceed the allowed clock
frequency of the CPU in low voltage designs!
You may replace this search algorithm with any other algorithm you like if
you have additional constraints such as a maximum CPU clock.
For version 5.x RC oscillators (those with a split range of 2x128 steps, e.g.
ATTiny25, ATTiny45, ATTiny85), it may be useful to search for the optimum in
both regions.
*/

void hadUsbReset(void)
{
	VUsbMidi.OnUSBReset();
	/*cli();
	calibrateOscillator();
	sei();*/
   //eeprom_write_byte(0, OSCCAL); 
   //store the calibrated value in EEPROM byte 0
}


#ifdef __cplusplus
} // extern "C"
#endif



TFUsbMidi::TFUsbMidi() {
	_bufftop = 0;
	_bufflast = 0;
	_buffsize = 0;
	_optimalize_osc = false;
	_calibrate_osc = false;
};

void TFUsbMidi::begin(bool cal_osc) {
	uchar i;
	_calibrate_osc = cal_osc;
	// turn off timer0
	//TIMSK0&=!(1<<TOIE0);
    wdt_enable(WDTO_1S);
	
    usbInit();
    usbDeviceDisconnect();  // enforce re-enumeration, do this while interrupts are disabled!
    
    i = 0;
    while(--i){             /* fake USB disconnect for > 250 ms */
        wdt_reset();
        _delay_ms(1);
    }
   
    usbDeviceConnect();
    sei();
};

uint8_t TFUsbMidi::buffNext(uint8_t value) {
	return (value + VUSBMIDI_RINGBUFFER_ESIZE) % (VUSBMIDI_RINGBUFFER_ESIZE*VUSBMIDI_RINGBUFFER_SIZE);
};

uint8_t TFUsbMidi::buffPush(uint8_t *p) {
	if(buffNext(_bufflast) == _bufftop){
		return 0;
	}
	buffer[_bufflast] = *p;
	buffer[_bufflast+1] = *(p+1);
	buffer[_bufflast+2] = *(p+2);
	buffer[_bufflast+3] = *(p+3);
	_bufflast = buffNext(_bufflast);
	_buffsize ++;
	return 1;
};

uint8_t* TFUsbMidi::buffPop(void) {
	uint8_t *res = (uint8_t *)0;
	if(_bufftop != _bufflast){
		res = &(buffer[_bufftop]);
		_bufftop = buffNext(_bufftop);
		_buffsize --;
	}
	return res;
};

TFMidiType TFUsbMidi::getMessageType(byte *p) {
	TFMidiType type = InvalidType;
	switch ((*(p + 1) & 0xf0)) {
		case TFMidiType::NoteOn:				type = TFMidiType::NoteOn;	break;
		case TFMidiType::NoteOff:				type = TFMidiType::NoteOff;	break;
		case TFMidiType::ControlChange:			type = TFMidiType::ControlChange;	break;
		case TFMidiType::AfterTouchPoly:		type = TFMidiType::AfterTouchPoly;	break;
		case TFMidiType::ProgramChange:			type = TFMidiType::ProgramChange;	break;
		case TFMidiType::AfterTouchChannel:		type = TFMidiType::AfterTouchChannel;	break;
		case TFMidiType::PitchBend:				type = TFMidiType::PitchBend;	break;
		case TFMidiType::SystemExclusive:		type = TFMidiType::SystemExclusive;	break;
		case TFMidiType::TimeCodeQuarterFrame:	type = TFMidiType::TimeCodeQuarterFrame;	break;
		case TFMidiType::SongPosition:			type = TFMidiType::SongPosition;	break;
		case TFMidiType::SongSelect:			type = TFMidiType::SongSelect;	break;
		case TFMidiType::TuneRequest:			type = TFMidiType::TuneRequest;	break;
		case TFMidiType::Clock:					type = TFMidiType::Clock;	break;
		case TFMidiType::Start:					type = TFMidiType::Start;	break;
		case TFMidiType::Continue:				type = TFMidiType::Continue;	break;
		case TFMidiType::Stop:					type = TFMidiType::Stop;	break;
		case TFMidiType::ActiveSensing:			type = TFMidiType::ActiveSensing;	break;
		case TFMidiType::SystemReset:			type = TFMidiType::SystemReset;	break;
		case TFMidiType::InvalidType:			type = TFMidiType::InvalidType;	break;
	}
	return type;
};

void TFUsbMidi::processMessage() {
	uint8_t *pbuf;
	TFMidiMessage message = {};
	message.type = InvalidType;
	if (_buffsize>0) {
		pbuf = buffPop();
		if (pbuf != 0) {
			message.type = getMessageType(pbuf);
			if (message.type !=  InvalidType) {
				message.channel = *(pbuf+1)&0x0f;
				message.data1 = *(pbuf+2)&0x7f;
				message.data2 = *(pbuf+3)&0x7f;
				
				if (_onMsgCallback != NULL) {
					_onMsgCallback(message);
				}
			}
		}
	}
};

void TFUsbMidi::OnUSBReset(void) {
	if (_calibrate_osc) {
		calibrateOSC();
		if (_optimalize_osc) { _calibrate_osc = true; }else {
			_calibrate_osc = false;
		};
	}
};

void TFUsbMidi::calibrateOSC(void) {
	cli();
	calibrateOscillator();
	sei();
};

void TFUsbMidi::OnMsg(void (*onMsgCallback)(TFMidiMessage)) {
	_onMsgCallback = onMsgCallback;
};

void TFUsbMidi::refresh() {
	wdt_reset();
	processMessage();
	usbPoll();	
};

void TFUsbMidi::read(uchar *data, uchar len) {
	cli();
	buffPush(data);
	if (len > 4) {
		buffPush(data+4);
	}
	sei();
};

void TFUsbMidi::NoteOn(byte ch, byte note, byte velocity) {
	TFMidiMessage msg;
	msg.type = (velocity == 0) ? TFMidiType::NoteOff : TFMidiType::NoteOn;
	msg.channel = ch;
	msg.data1 = note;
	msg.data2 = velocity;
	write(msg);
};

void TFUsbMidi::NoteOff(byte ch, byte note) {
	NoteOn(ch,note,0);
};

void TFUsbMidi::ControlChange(byte ch, byte num, byte val) {
	TFMidiMessage msg;
	msg.type = TFMidiType::ControlChange;
	msg.channel = ch;
	msg.data1 = num;
	msg.data2 = val;
	write(msg);
};

void TFUsbMidi::write(TFMidiMessage msg) {
	byte buffer[4];
	buffer[0] = msg.type >> 4;
	buffer[1] = msg.type | msg.channel;
	buffer[2] = 0x7f & msg.data1;
	buffer[3] = (msg.data2 == 0) ? 0 : 0x7f & msg.data2;
	write(buffer,4);
};

void TFUsbMidi::write(byte *buffer, byte size) {
	while (!usbInterruptIsReady()) {
		usbPoll();
	}
	usbSetInterrupt(buffer, size);
};


TFUsbMidi VUsbMidi = TFUsbMidi();
