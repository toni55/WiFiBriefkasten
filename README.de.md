# WiFiBriefkasten

Da ich schon älter bin und in der 2. Etage Wohne war ich es Leid, ständig die Treppe runter und rauf zu Laufen, auch wenn keine Post im Briefkasten war. Deshalb kam mir die Idee dieses Projekts. Es wird eine MQTT-Nachricht gesendet wenn entweder die Klappe oder Tür am Briefkasten geöffnet wird. Beim öffnen der Klappe wird ein Zähler hochgesetzt und "POST" auf 1 gesetzt. Wird die Tür geöffnet wird "POST" und "ANZAHL" wieder auf 0 gesetzt. Die Nachricht wird jede Stunde gesendet. Dazwischen geht die Schaltung in den "DeepSleep" und wird entweder durch den Timer oder durch eine Aktion (Klappe/Tür) aufgeweckt.
Zum Updaten der Software muß die Klappe/Tür geöffnet sein, damit über das OTA-Protokoll geflusht werden kann.

### MQTT-Nachricht

- RAW	Rohdaten der Akkuspannung ( int: 0 - 1023)
- AKKU  Akkuspannung in Volt (float: 0.0 - ...)
- SIGNAL  Signalstärke der Wifi-Verbindung (int: -100 - 0)
- KLAPPE. Zustand der Klappe (char: '0' / '1')
- TUER. Zustand der Tür (char: '0' / '1')
- POST  ist Post da (char: '0' / '1')
- ANZAHL. Anzahl der Klappenöffnungen (int: 0 - ....)

```
{"Raw":794,"Akku":3.34,"Signal":-66,"Klappe":0,"Tuer":0,"Post":0,"Anzahl":0}

```

### Schaltung

Die Schaltung kann mit 4xAA oder 1xLiPo/LiFePo betrieben werden. Bei Verwendung eines LiFePo entfallen die Bauteile C4/C5/C6 und U1. Zusätzlich müssen 2 Lötpads (siehe [PCB](./res/pcb.png)) überbrückt werden. Die Switches (NC) sind im geschlossenen Zustand des Briefkastens geöffnet! Wird die Klappe/Tür geöffnet so geht der Eingang des ESP auf Ground und über den/die Kondensator(en) wird der Reset-Eingang ebenfalls kurz auf Ground gelegt. Dadurch wacht der ESP auf und Sendet eine entsprechende MQTT-Nachricht.
Die Schaltung wurde mit [EasyEDA](https://easyeda.com/Toni55/briefkastenmelder) erstellt und ist im "res" Ordner als JSON-File abgelegt.

![schaltung](./res/schaltung.png)

## Bauteilliste

    Bauteil     Name
    - 10uF	    C3,C2 (KerKo)
    - 100nF	    C1,C5,C6 (KerKo)
    - 10kOhm	R2,R3,R4,R5,R1,R10,R11
    - 33kOhm	R6
    - klappe	JP2
    - tuer	    JP3
    - Bat	    JP4	1X02
    - 1N4148	D2,D1
    - 100uF	    C4 (ElKo)
    - HT7333-A	U1
    - ESP-07S	MK1


## Software

Die Software wurde mit [PlatformIO](https://platformio.org/) und dem Arduinoframwork kompiliert.

## Bibliotheken

- Arduino.h
- ArduinoOTA.h
- ESP8266WiFi.h
- **PubSubClient.h**
- EEPROM.h

## Authors

* **Toni Reichartz** - *Initial work*

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details
