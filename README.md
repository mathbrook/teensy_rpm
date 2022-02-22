# teensy_rpm
Simple RPM logger implemented on Kennesaw Motorsports FSAE's car the ks5c.

Reads pulse frequency of two hall effect sensors and divides by tooth count to get frequency as Hertz, then multiplies by 60 to get RPM.

Also logs two brake pressure transducer voltages through an ADS1115 adc over i2c
