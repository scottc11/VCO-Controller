/* MCP23017 Library for mbed
 * Copyright (c) 2014, Takuya Urakawa
 * 
 * This library is released under the MIT License
 * See http://opensource.org/licenses/mit-license.php
 */

#include "MCP23017.h"

void MCP23017::init(void) {
	
	// init config, mirror interupts
	i2cSend(REG_IOCON, 0b01000000);

	// port 0
	i2cSend(REG_GPIO, 0, 0);

	// port direction all input
	i2cSend(REG_IODIR, 0, 0);

	// interupt off
	i2cSend(REG_GPINTEN, 0, 0);

	// clear interrupt
	digitalRead(MCP23017_PORTA);
	digitalRead(MCP23017_PORTB);
}

void MCP23017::setConfig(char _value) {
	
	i2cSend(REG_IOCON, _value);
}

void MCP23017::setDirection(char _port, char _value) {
	// io.setDirection(MCP23017_PORTA, 0xFF);    // set all of the PORTA pins to input
	// io.setDirection(MCP23017_PORTB, 0x00);    // sets all of the PORTB pins to output
	i2cSend(REG_IODIR + _port, _value);
}

void MCP23017::setPullUp(char _port, char _value) {
	i2cSend(REG_GPPU + _port, _value);
}

void MCP23017::setInputPolarity(char _port, char _value) {
	i2cSend(REG_IPOL + _port, _value);
}

/**
 * If enabled, the MCP23X17 activates the INTn interrupt output when one of the port pins changes state or when a pin does not match the preconfigured default.
 * Additionally, the INTn pins can be configured to mirror each other so that any interrupt will cause both pins to go active. This is controlled via IOCON.MIRROR.
 * There are two interrupt pins: INTA and INTB. By default, INTA is associated with GPAn pins (PORTA) and INTB is associated with GPBn pins (PORTB).
 * NOTE: Each port has an independent signal which is cleared if its associated GPIO or INTCAP register is read.
**/
void MCP23017::setInterupt(char _port, char _value) {
	i2cSend(REG_GPINTEN + _port, _value );
}

/**
 * This register allows the user to configure the polarity on the corresponding GPIO port bits.
 * If a bit is set, the corresponding GPIO register bit will reflect the inverted value on the pin.
 * for some reason, I think the default is inverted...
**/
void MCP23017::digitalWrite(char _port, char _value) {
	i2cSend(REG_GPIO + _port, _value);
}

char MCP23017::digitalRead(char _port) {
	return i2cRead(REG_GPIO + _port);
}


/**
 * read both ports and append into a 16 bit value 
*/
uint16_t MCP23017::digitalReadAB() {
	int portA = this->digitalRead(MCP23017_PORTA);
	int portB = this->digitalRead(MCP23017_PORTB);
	return (portB << 8) | portA;
}

int MCP23017::getBitStatus(int byte, int bit) {
	return (byte & (1 << bit));
}
