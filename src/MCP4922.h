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

#ifndef MCP4922_H
#define MCP4922_H

#include "mbed.h"

/** MCP4922 class.
 *  Used for controlling an MCP4922 DAC connected via SPI.
 *
 * Example:
 * @code
 * #include "mbed.h"
 * #include "MCP4922.h"
 *
 * //Create an MCP4922 object
 * MCP4922 dac(p11, p13, p14);
 *
 * int main()
 * {
 *     //Configure DAC A
 *     dac.referenceMode(MCP4922::DAC_A, MCP4922::REF_UNBUFFERED);
 *     dac.gainMode(MCP4922::DAC_A, MCP4922::GAIN_1X);
 *     dac.powerMode(MCP4922::DAC_A, MCP4922::POWER_NORMAL);
 *
 *     while (1) {
 *         //Generate a sine wave on DAC A
 *         for (float i = 0.0; i < 360.0; i += 0.1)
 *             dac.write(MCP4922::DAC_A, 0.5 * (sinf(i * 3.14159265 / 180.0) + 1));
 *     }
 * }
 * @endcode
 */
class MCP4922
{
public:
    /** Represents the different DACs in the MCP4922
     */
    enum _DAC {
        DAC_A,              /**< DAC A */
        DAC_B               /**< DAC B */
    };

    /** Represents the reference buffer mode of DAC A or B in the MCP4922
     */
    enum ReferenceMode {
        REF_UNBUFFERED,     /**< DAC VREF is unbuffered */
        REF_BUFFERED        /**< DAC VREF is buffered */
    };

    /** Represents the gain mode of DAC A or B in the MCP4922
     */
    enum GainMode {
        GAIN_2X,            /**< DAC output = 2 * VREF * D/4096 */
        GAIN_1X             /**< DAC output = VREF * D/4096 */
    };

    /** Represents the power mode of DAC A or B in the MCP4922
     */
    enum PowerMode {
        POWER_SHUTDOWN,     /**< DAC output is high impedance */
        POWER_NORMAL        /**< DAC output is enabled */
    };

    /** Create an MCP4922 object connected to the specified SPI pins
     *
     * @param mosi The SPI data out pin.
     * @param sclk The SPI clock pin.
     * @param cs The SPI chip select pin.
     * @param hz The SPI bus frequency (defaults to 20MHz).
     */
    MCP4922(PinName mosi, PinName sclk, PinName cs, int hz = 20000000);

    /** Get the current reference mode of the specified DAC in the MCP4922
     *
     * @param dac The DAC to read from.
     *
     * @returns The current reference mode of the specified DAC as a ReferenceMode enum.
     */
    MCP4922::ReferenceMode referenceMode(_DAC dac);

    /** Set the reference mode of the specified DAC in the MCP4922
     *
     * @param dac The DAC to write to.
     * @param mode The new reference mode for the specified DAC as a ReferenceMode enum.
     */
    void referenceMode(_DAC dac, ReferenceMode mode);

    /** Get the current gain mode of the specified DAC in the MCP4922
     *
     * @param dac The DAC to read from.
     *
     * @returns The current gain mode of the specified DAC as a GainMode enum.
     */
    MCP4922::GainMode gainMode(_DAC dac);

    /** Set the gain mode of the specified DAC in the MCP4922
     *
     * @param dac The DAC to write to.
     * @param mode The new gain mode for the specified DAC as a GainMode enum.
     */
    void gainMode(_DAC dac, GainMode mode);

    /** Get the current power mode of the specified DAC in the MCP4922
     *
     * @param dac The DAC to read from.
     *
     * @returns The current power mode of the specified DAC as a PowerMode enum.
     */
    MCP4922::PowerMode powerMode(_DAC dac);

    /** Set the power mode of the specified DAC in the MCP4922
     *
     * @param dac The DAC to write to.
     * @param mode The new power mode for the specified DAC as a PowerMode enum.
     */
    void powerMode(_DAC dac, PowerMode mode);

    /** Get the current output voltage of the specified DAC in the MCP4922 as a percentage
     *
     * @param dac The DAC to read from.
     *
     * @returns The current output voltage of the specified DAC as a percentage (0.0 to 1.0 * VDD).
     */
    float read(_DAC dac);

    /** Set the output voltage of the specified DAC in the MCP4922 from a percentage
     *
     * @param dac The DAC to write to.
     * @param value The new output voltage for the specified DAC as a percentage (0.0 to 1.0 * VDD).
     */
    void write(_DAC dac, float value);

    /** Set the output voltage of the specified DAC in the MCP4922 from a 16-bit range
     *
     * @param dac The DAC to write to.
     * @param value The new output voltage for the specified DAC as a 16-bit unsigned short (0x0000 to 0xFFFF).
     */
    void write_u16(_DAC dac, unsigned short value);

private:
    //SPI member variables
    SPI m_SPI;
    DigitalOut m_CS;

    //DAC settings member variables
    unsigned short m_DacValueA;
    unsigned short m_DacValueB;

    //Internal functions
    void writeDac(unsigned short value);
};

#endif
