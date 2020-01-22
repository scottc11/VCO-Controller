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

void MCP23017::setInterupt(char _port, char _value) {
	i2cSend(REG_GPINTEN + _port, _value );
}

void MCP23017::digitalWrite(char _port, char _value) {
	i2cSend(REG_GPIO + _port, _value);
}

char MCP23017::digitalRead(char _port) {
	return i2cRead(REG_GPIO + _port);
}
