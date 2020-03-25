```
/*
 *  Copyright (c) 2012 Neal Horman - http://www.wanlink.com
 *  
 *  License: MIT open source (http://opensource.org/licenses/MIT)
 *      Summary;
 *      Use / modify / distribute / publish it how you want and 
 *      if you use it, or don't, you can't hold me liable for how
 *      it does or doesn't work.
 *      If it doesn't work how you want, don't use it, or change
 *      it so that it does work.
 */
 
#include "mbed.h"
#include "Adafruit_SSD1306.h"
 
DigitalOut myled(LED1);

I2C i2c(I2C_SDA, I2C_SCL);

Adafruit_SSD1306_I2c oled(i2c, D6);
 
int main()
{   uint16_t x=0;
 
    gOled1.printf("%ux%u OLED Display\r\n", gOled1.width(), gOled1.height());
    gOled2.printf("%ux%u OLED Display\r\n", gOled2.width(), gOled2.height());
    
    while(1)
    {
        myled = !myled;
        gOled1.printf("%u\r",x);
        gOled1.display();
        gOled2.printf("%u\r",x);
        gOled2.display();
        x++;
        wait(1.0);
    }
}
```