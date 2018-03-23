# WordClock

## Scopo
visualizzazione dell'ora mendiante finestre che vengono illuminate dall'accensione
di led sottostanti:
Ogni colnna di finestre rappresenta uha cifra in binario, il bit di peso maggiore 
è in alto
La somma del peso corrispondente alle finestre accese fornisce il valore della cifra

```

                        .------------------------------.    peso
                        |                              |
                        |  .---.  .---.  .---.  .---.  |
                        |  |   |  |   |  |   |  |   |  |     8
                        |  |   |  |   |  |   |  |   |  |
                        |  '---'  '---'  '---'  '---'  |
                        |                              |
                        |  .---.  .---.  .---.  .---.  |
                        |  |   |  |   |  |   |  |   |  |     4
                        |  |   |  |   |  |   |  |   |  |
                        |  '---'  '---'  '---'  '---'  |
                        |                              |
                        |  .---.  .---.  .---.  .---.  |
                        |  |   |  |   |  |   |  |   |  |     2
                        |  |   |  |   |  |   |  |   |  |
                        |  '---'  '---'  '---'  '---'  |
                        |                              |
                        |  .---.  .---.  .---.  .---.  |
                        |  |   |  |   |  |   |  |   |  |
                        |  |   |  |   |  |   |  |   |  |     1
                        |  |   |  |   |  |   |  |   |  |
                        '------------------------------'
                             ^      ^      ^      ^ 
                             |      |      |      |
ore    colonna delle decine -'      |      |      |
       colonna delle unità  --------'      |      |
minuti colonna delle decine ---------------'      |
       colonna delle unità  ----------------------'
       
```

Esempio: 16:59 (le X corrispondono ai led accesi, - a led spenti)

          -  -  -  X
          -  X  X  -
          -  X  -  -
          X  -  X  X

è prevista una gestione automatica dell'ora legale valida fino al 2031

## HW utilizzato:
  * 1 x Arduino Mega 2560
  * 4 x uln2803 (Darlington transistor array)
  * 1 x Modulo RTC DS3231 (Real Time Clock)
  * 1 x Modulo HC-06 (Bluetooth)
  * Stricia LED tagliata in segmenti opportuni per le varie finestre
  * 1 x TIP122 (Power Darlington)
  * 4 x pulsanti
  * resistenze
  * alimentatore
