# ESP8266_Transparent_TCP_Client_Bridge

Transparent TCP Client bridge for the ESP8266.

Based on https://github.com/beckdac/ESP8266-transparent-bridge

Connection µC:

ESP         (Arduino)
GPIO0-------RST
TX----------RX
RX----------TX

Common Connection:

ESP 
Vcc---------3.3V
GND---------GND
GPIO2i------(can be connected to an LED+Resistor typical 150Ohm Puls 0.5)
            (better use an supervisor IC to reset the ESP if it fail e.g. Sipex SP706x)
GPIO0-------0.5sec low-puls during initialisation.
TX-----------RX
RX-----------TX



