
Using this module:

![image](https://github.com/user-attachments/assets/c5b22328-3900-4ad5-afee-bb1a66a9d547)




Fixing RBDdimmmer library for ESP8266:

Had the same thing. First resolved it by downgrading the ESP8266 board version to 2.5.0 in the board manager. However, the actual solution is adding in the file src/esp8266/RBDmcuESP8266.cpp the attribute ICACHE_RAM_ATTR  in front of the function isr_ext(), in the same way as it is in the function onTimerISR() (so void ICACHE_RAM_ATTR isr_ext())
