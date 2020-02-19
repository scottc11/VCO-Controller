/* MCP4922 Driver Library
 * Copyright (c) 2014 Neil Thiessen
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "MCP4922.h"

MCP4922::MCP4922(PinName mosi, PinName sclk, PinName cs, int hz) : m_SPI(mosi, NC, sclk), m_CS(cs, 1)
{
    //Initialize the member variables
    m_DacValueA = 0;
    m_DacValueB = 0;

    //Set the SPI format and bus frequency
    m_SPI.format(16, 0);
    m_SPI.frequency(hz);

    //Perform an initial write to both DACs so the variables are in sync
    writeDac(m_DacValueA | (DAC_A << 15));
    writeDac(m_DacValueB | (DAC_B << 15));
}

MCP4922::ReferenceMode MCP4922::referenceMode(_DAC dac)
{
    //Return the current reference mode for the specified DAC
    if (dac == DAC_A)
        return (ReferenceMode)((m_DacValueA >> 14) & 0x01);
    else
        return (ReferenceMode)((m_DacValueB >> 14) & 0x01);
}

void MCP4922::referenceMode(_DAC dac, ReferenceMode mode)
{
    //Update the reference mode for the specified DAC
    if (dac == DAC_A) {
        //Mask off the old mode, and set the new one
        m_DacValueA &= ~(1 << 14);
        m_DacValueA |= (mode << 14);

        //Update the DAC A
        writeDac(m_DacValueA | (dac << 15));
    } else {
        //Mask off the old mode, and set the new one
        m_DacValueB &= ~(1 << 14);
        m_DacValueB |= (mode << 14);

        //Update the DAC B
        writeDac(m_DacValueB | (dac << 15));
    }
}

MCP4922::GainMode MCP4922::gainMode(_DAC dac)
{
    //Return the current gain mode for the specified DAC
    if (dac == DAC_A)
        return (GainMode)((m_DacValueA >> 13) & 0x01);
    else
        return (GainMode)((m_DacValueB >> 13) & 0x01);
}

void MCP4922::gainMode(_DAC dac, GainMode mode)
{
    //Update the gain mode for the specified DAC
    if (dac == DAC_A) {
        //Mask off the old mode, and set the new one
        m_DacValueA &= ~(1 << 13);
        m_DacValueA |= (mode << 13);

        //Update the DAC A
        writeDac(m_DacValueA | (dac << 15));
    } else {
        //Mask off the old mode, and set the new one
        m_DacValueB &= ~(1 << 13);
        m_DacValueB |= (mode << 13);

        //Update the DAC B
        writeDac(m_DacValueB | (dac << 15));
    }
}

MCP4922::PowerMode MCP4922::powerMode(_DAC dac)
{
    //Return the current power mode for the specified DAC
    if (dac == DAC_A)
        return (PowerMode)((m_DacValueA >> 12) & 0x01);
    else
        return (PowerMode)((m_DacValueB >> 12) & 0x01);
}

void MCP4922::powerMode(_DAC dac, PowerMode mode)
{
    //Update the power mode for the specified DAC
    if (dac == DAC_A) {
        //Mask off the old mode, and set the new one
        m_DacValueA &= ~(1 << 12);
        m_DacValueA |= (mode << 12);

        //Update the DAC A
        writeDac(m_DacValueA | (dac << 15));
    } else {
        //Mask off the old mode, and set the new one
        m_DacValueB &= ~(1 << 12);
        m_DacValueB |= (mode << 12);

        //Update the DAC B
        writeDac(m_DacValueB | (dac << 15));
    }
}

float MCP4922::read(_DAC dac)
{
    //Return the current value for the specified DAC as a float
    if (dac == DAC_A)
        return (m_DacValueA & 0x0FFF) / 4095.0;
    else
        return (m_DacValueB & 0x0FFF) / 4095.0;
}

void MCP4922::write(_DAC dac, float value)
{
    //Range limit value
    if (value < 0.0)
        value = 0.0;
    else if (value > 1.0)
        value = 1.0;

    //Convert value to an unsigned short, and pass it to write_u16()
    write_u16(dac, (unsigned short)(value * 4095) << 4);
}

void MCP4922::write_u12(_DAC dac, unsigned short value) {
    write_u16(dac, value << 4);
}

void MCP4922::write_u16(_DAC dac, unsigned short value)
{
    //Update the value for the specified DAC
    if (dac == DAC_A) {
        //Mask off the old value, and set the new one
        m_DacValueA &= 0xF000;
        m_DacValueA |= value >> 4;

        //Update the DAC A
        writeDac(m_DacValueA | (dac << 15));
    } else {
        //Mask off the old value, and set the new one
        m_DacValueB &= 0xF000;
        m_DacValueB |= value >> 4;

        //Update the DAC B
        writeDac(m_DacValueB | (dac << 15));
    }
}

void MCP4922::writeDac(unsigned short value)
{
    //Pull CS low
    m_CS = 0;

    //Perform the 16-bit write
    m_SPI.write(value);

    //Pull CS high
    m_CS = 1;
}
