// MIT License
// 
// Copyright (c) 2017 Tullio Benzi
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// ================================================================================
// Scopo
// visualizzazione dell'ora mendiante finestre che vengono illuminate dall'accensione
// di led sottostanti:
// Ogni colnna di finestre rappresenta uha cifra in binario, il bit di peso maggiore 
// è in alto
// La somma del peso corrispondente alle finestre accese fornisce il valore della cifra
// 
//                         .------------------------------.    peso
//                         |                              |
//                         |  .---.  .---.  .---.  .---.  |
//                         |  |   |  |   |  |   |  |   |  |     8
//                         |  |   |  |   |  |   |  |   |  |
//                         |  '---'  '---'  '---'  '---'  |
//                         |                              |
//                         |  .---.  .---.  .---.  .---.  |
//                         |  |   |  |   |  |   |  |   |  |     4
//                         |  |   |  |   |  |   |  |   |  |
//                         |  '---'  '---'  '---'  '---'  |
//                         |                              |
//                         |  .---.  .---.  .---.  .---.  |
//                         |  |   |  |   |  |   |  |   |  |     2
//                         |  |   |  |   |  |   |  |   |  |
//                         |  '---'  '---'  '---'  '---'  |
//                         |                              |
//                         |  .---.  .---.  .---.  .---.  |
//                         |  |   |  |   |  |   |  |   |  |
//                         |  |   |  |   |  |   |  |   |  |     1
//                         |  |   |  |   |  |   |  |   |  |
//                         '------------------------------'
//                              ^      ^      ^      ^ 
//                              |      |      |      |
// ore    colonna delle decine -'      |      |      |
//        colonna delle unità  --------'      |      |
// minuti colonna delle decine ---------------'      |
//        colonna delle unità  ----------------------'
// 
// Esempio: 16:59 (le X corrispondono ai led accesi, - a led spenti)
// 
//           -  -  -  X
//           -  X  X  -
//           -  X  -  -
//           X  -  X  X
// 
// è prevista una gestione automatica dell'ora legale valida fino al 2031
//
// Oltre alla board Arduino è previsto l'utilizzo di
//  Real Time Clock(RTC) con modulo 3231
//  Bluetooth con modulo HTC05 o HTC06
//
// per gestire l'orologio sono previsti 4 pulsanti:
//      MODE    effettua il cambio di modalità
//      SET     imposta l'RTC
//      +       incrementa il valore delle ore e dei minuti
//      -       decrementa il valore delle ore e dei minuti
//
// collegando una seriale si:
//
//  - visualizza l'ora ed i minuti letti da RTC(in modalità STANDARTD)
//               l'ora ed i minuti che si imposteranno (in modalità SET_CLOCK)
//               la loro codifica in testo
//
//   possono dare dei comandi per:
//  - impostare l'RTC completamente anno, mese, mese, giorno, ore, minuti
//          inviando una stringa da 14 caratteri (aa,mm,gg,hh,mm)
//  - abilitare il test delle uscite
//          inviando una stringa da 2 caratteri ET (Enable Test) oppure qualunque coppia di caratteri per disabilitare
//  - variare la lunminosità dei led impostandi il duty cycle dell'uscta PWM utilizzata per variare (tramite darlington)
//          la tensione di alimentazione dei LED
//
// comandi previsti da seriale:
//  - una stringa da 14 caratteri (aa,mm,gg,hh,mm) set RTC
//  - una stringa da 2 caratteri ET (Enable Test) oppure qualunque coppia di caratteri
//       per disabilitare il test
//  - una stringa da 1 carattere (qualsiasi) per l'help dei comandi
//  - una stringa da 4 caretteri Lnnn dove nnn va da 000 a 255 valore duty cycle uscita PWM
//
// Connettendosi in bluetooth si possono fare le stesse cose che si fanno con la
// seriale PC
//
// Le scritte verso bluetooth sono pensate per utilizzare l'app "Bluetooth Electronics" presente su PlayStore
//  in cui ho fatto una maschera che ha:
//      Titolo
//      terminale
//      stringa da inviare
//  I testi che vanno nel Titolo sono preceduti da *# e terminano con *
//  I testi che vanno sul terminale sono preceduti da *@
// ================================================================================
// Modalità previste:
//  STANDARD:   normale visualizzazione dell'orario
//  SET_CLOCK:  impostazione dell'orologio
//  LED_TEST:   test ciclico dei led che si accendono tutti in sequenza
//
// Le modalità sono gestite con una macchina a stati pricipale (vedi myStatus Status[NUM_MODE])
//
// Le fasi della modalità set sono gestite con un'altra macchina a stati (vedi myStatus SetClockSubStatus[NUM_SETCLOCK_SUBSTATUS])
//
// ================================================================================
// Gestione pulsanti
//
// sono previsti 4 pulsanti:
//
// in modalità STANDARD (visualizzazione orario):
//
//      premere il pulsante MODE per almeno MIN_MS_TO_SET_MODE ms e si entra
//      in modalità SET_CLOCK
//
//      permere il pulsante SET e se entra in LED_TEST
//
//
// in modalità SET_CLOCK (impostazione orologio):
//
//      è attiva per prima l'impostazione dell'ora
//      impostazione ora:
//          le ore lampeggiano
//          premere + o - per aumentare  diminuire il valore dell'ora
//          quando si preme SET si memorizza l'ora visualizzata e
//          si passa all'impostazione dei minuti
//      impostazione minuti:
//          i minuti lampeggiano
//          premere + o - per aumentare  diminuire il valore dei minuti
//          quando si preme SET si memorizzano i minuti visualizzati,
//              si imposta l'orologio, si esce da SET_CLOCK e
//              si ritorna in modalità STANDARD
//
//      se si preme il pulsante MODE si esce da SET_CLOCK e
//          si ritorna in modalità STANDARD
//
//      se passano più di MAX_MS_X_SET_CLOCK senza premere uno dei pulsanti
//      +, -, SET si esce da SET_CLOCK e si ritorna in modalità STANDARD
//
// in modalità LED_TEST (test dei led):
//
//      se si preme un pulsante qualsiasi o comunque dopo MAX_MS_X_LED_TEST
//      si esce da SET_CLOCK e si ritorna in modalità STANDARD
//
// ================================================================================
// HW utilizzato:
//      1 x Arduino Mega 2560
//      4 x uln2803 (Darlington transistor array)
//      1 x Modulo RTC DS3231 (Real Time Clock)
//      1 x Modulo HC-06 (Bluetooth)
//      Stricia LED tagliata in segmenti opportuni per le varie parole
//
// Connessioni
// Arduino [pin]
//             .----------.                         .----------.
//            -|32K       |              [11] -->---|RX  HC-06 |
//            -|SQW RTC   |              [12] -->---|TX  Blue  |
//  [SCL] ->- -|SCL DS3231|              [GND] ->- -|GND tooth |
//  [SDA] ->- -|SDA       |              [5V] -->- -|VCC       |
//  [5V] -->- -|VCC       |                         '----------'
//  [GND] ->- -|GND       |
//             '----------'
//                                       ____ puls MODE
//                               .-------o  o--------[5V]
//                .---------.    |   .---------.
//  [22] --<------| 220 Ohm |----'---| 10K Ohm |-----[GND]
//                '---------'        '---------'
//
//                                       ____ puls NEXT_HM
//                               .-------o  o--------[5V]
//                .---------.    |   .---------.
//  [23] --<------| 220 Ohm |----'---| 10K Ohm |-----[GND]
//                '---------'        '---------'
//
//                                       ____ puls PREV_HM
//                               .-------o  o--------[5V]
//                .---------.    |   .---------.
//  [24] --<------| 220 Ohm |----'---| 10K Ohm |-----[GND]
//                '---------'        '---------'
//
//                                       ____ puls SET
//                               .-------o  o--------[5V]
//                .---------.    |   .---------.
//  [25] --<------| 220 Ohm |----'---| 10K Ohm |-----[GND]
//                '---------'        '---------'
//                .---v---.
//  [26] -->-- 1B-|       |-1C -->-- LED_1 DECINE ORE-----<|---(*)
//  [27] -->-- 2B-|       |-1C -->-- LED_8 UNITA ORE------<|---(*)
//  [28] -->-- 3B-|       |-2C -->-- LED_4 UNITA ORE------<|---(*)
//  [29] -->-- 4B-|       |-3C -->-- LED_2 UNITA ORE -----<|---(*)
//  [30] -->-- 5B-|ULN2003|-4C -->-- LED_1 UNITA ORE -----<|---(*)
//  [31] -->-- 6B-|       |-5C -->-- LED_8 DECINE MINUTI -<|---(*)
//  [32] -->-- 7B-|       |-6C -->-- LED_4 DECINE MINUTI -<|---(*)
//  [33] -->-- 8B-|       |-7C -->-- LED_2 DECINE MINUTI -<|---(*)
//  [GND] ->- GND-|       |-COM-
//                '-------'
//                .---v---.
//  [34] -->-- 1B-|       |-8C -->-- LED_1 DECINE MINUTI -<|---(*)
//             2B-|       |-1C -->-- 
//             3B-|       |-2C -->-- 
//             4B-|       |-3C -->-- 
//             5B-|ULN2003|-4C -->-- 
//             6B-|       |-5C
//             7B-|       |-6C      .--------.            .--------.------.
//  [02]  ->-  8B-|       |-8C -->--| 1K Ohm |--------- b-|        | .--. |
//  [GND] ->- GND-|       |-COM     '--------'   +12V-- c-| TIP122 | |  | |
//                '-------'                        (*)- e-|        | '--' |
//                                                        '--------'------'
//
// ================================================================================

#include <stdio.h>          // for function sprintf
#include <DS3231.h>         // library for RTC
#include <Wire.h>           // to communicate with I2C
#include <SoftwareSerial.h> // to communicate with bluetooth

// indici dei led
enum E_LED {
	LED_1_DEC_ORE,	//  0   meno significativo
	LED_1_UNI_ORE,  //  1   meno significativo
	LED_2_UNI_ORE,  //  2   
	LED_4_UNI_ORE,  //  3
	LED_8_UNI_ORE,  //  4   più significativo
	LED_1_DEC_MIN,  //  5   meno significativo
	LED_2_DEC_MIN,  //  6
	LED_4_DEC_MIN,  //  7
	LED_8_DEC_MIN,  //  8   più significativo
	LED_1_UNI_MIN,  //  9   meno significativo
	LED_2_UNI_MIN,  // 10
	LED_4_UNI_MIN,  // 11
	LED_8_UNI_MIN,  // 12   più significativo
	NUM_LED         // 13
};

// indici dei pulsanti
enum E_PULS {
	PULS_MODE,    // 0
	PULS_NEXT_HM, // 1
	PULS_PREV_HM, // 2
	PULS_SET,     // 3
	NUM_PULS      // 4
};


// struttura associa ad ogni LED
struct myLed {
	const char*   associateString;        // testo associato
	byte          outputAddress;          // indirizzo dell'output associato
	boolean       bOut;                   // valore della uscita attuale
	boolean       bOldOut;                // valore della uscita al giro precedente
	boolean       bBlinkOut;              // led che deve lampeggiare
};

// struttura associa ad ogni PULSANTE
struct myPuls {
	byte    inputAddress;         // indirizzo dell'input associato
	boolean bIn;                  // valore dell'ingresso al giro attuale
	boolean bOldIn;               // valore dell'ingresso al giro precedente
	boolean bRiseEdge;            // fronte di salita nel ciclo precedente e livello 1 nell'attuale
	boolean bFallEdge;            // fronte di discesa nel ciclo precedente e livello 0 nell'attuale
	boolean bTmpRiseEdge;         // fronte di salita nel ciclo attuale
	boolean bTmpFallEdge;         // fronte di discesa nel ciclo attuale
	int     msInLevel;            // numero di ms nel livello
};

// definizione e inizializzazione dei pulsanti
myPuls Puls[NUM_PULS] = {
//	 add  in     oldIn  Rise   Fall   RisTmp FalTmp ms
	{22,  false, false, false, false, false, false, 0}, // MODE
	{23,  false, false, false, false, false, false, 0}, // NEXT_HM
	{24,  false, false, false, false, false, false, 0}, // PREV_HM
	{25,  false, false, false, false, false, false, 0}  // SET
};

// definizione e inizializzazione dei led
myLed Led[NUM_LED] = {
//   string  add   out   OldOut blink
	{" 1dh",  26, false, false, false},   //  0  LED_1_DEC_ORE
	{" 8uo",  27, false, false, false},   //  1  LED_8_UNI_ORE
	{" 4uo",  28, false, false, false},   //  2  LED_4_UNI_ORE
	{" 2uo",  29, false, false, false},   //  3  LED_2_UNI_ORE
	{" 1uo",  30, false, false, false},   //  4  LED_1_UNI_ORE
	{" 8dm",  31, false, false, false},   //  5  LED_8_DEC_MIN
	{" 4dm",  32, false, false, false},   //  6  LED_4_DEC_MIN
	{" 2dm",  33, false, false, false},   //  7  LED_2_DEC_MIN
	{" 1ms",  34, false, false, false},   //  8  LED_1_DEC_MIN
	{" 8um",  35, false, false, false},   //  9  LED_8_UNI_MIN
	{" 4um",  36, false, false, false},   // 10  LED_4_UNI_MIN
	{" 2um",  37, false, false, false},   // 11  LED_2_UNI_MIN
	{" 1um",  38, false, false, false},   // 12  LED_1_UNI_MIN
};

// --------------------------------------------------------------------
// modi di funzionamento
typedef enum E_MODE {
	STANDARD_MODE,  // visualizzazione orario
	SET_CLOCK_MODE, // set dell'orologio
	LED_TEST_MODE,  // test ciclico delle uscite
	NUM_MODE
} ENUM_MODE;

CStateMachine States;

//
// --------------------------------------------------------------------

// --------------------------------------------------------------------
// sottostati di SET_CLOCK_MODE
typedef enum E_SETCLOCK_SUBSTATUS {
	SET_HOUR,
	SET_MINUTES,
	SET_FINISH,
	NUM_SETCLOCK_SUBSTATUS
} ENUM_SETCLOCK_SUBSTATUS;

CStateMachine SetClockSubStates;

//
// --------------------------------------------------------------------

// definizione di costanti
#define MS_CYCLE 100
#define MIN_MS_TO_SET_MODE 3000
#define MS_X_REFRESH 1000
#define MS_X_BLINK 500
#define CICLE_REFRESH_X_WRITE_ON_SERIAL 5

typedef enum E_SERIAL_TYPE {
	SERIAL_LINE,  	// hardware serial
	BLUETOOTH_LINE, // bluetooth serial
} ENUM_SERIAL_TYPE;

//canali i/o utilizzati per il colloquio Arduino <-> modulo Bluetooth
typedef enum E_BLUETOOTH_PIN {
	BT_RX_PIN = 11,
	BT_TX_PIN = 12
} ENUM_BLUETOOTH_PIN;

typedef enum E_SERIAL_CMD {
	SERIAL_CMD_HELP           = 1,  //  comando da 1 carattere H help comadi seriale
	SERIAL_CMD_ENABLE_LEDTEST = 2,  //  comando da 2 caratteri ET (Enable Test) per abilitare il test delle uscite
									//  oppure qualunque coppia di caratteri per disabilitare
	SERIAL_CMD_SET_CLOCK      = 14, //  comando da 14 caratteri (aa,mm,gg,hh,mm) per set clock
	SERIAL_CMD_LIGHT_SET      =  4, //  comando da 4 caratteri Lnnn (L000 - L255) per variare la luminosità dei led
	SERIAL_CMD_MAX_LEN        = 14
} ENUM_SERIAL_CMD;

DS3231 Clock;   // real time clock

SoftwareSerial bluetooth  = SoftwareSerial(BT_RX_PIN, BT_TX_PIN);   // seriale bluetooth

// valori letti da RTC e usati per set RTC
boolean Century = false;
boolean h12;
boolean PM;
byte    second;
byte    minute;
byte    hour;
byte    day;
byte    month;
byte    year;

boolean PMNoDst;
byte    secondNoDst;
byte    minuteNoDst;
byte    hourNoDst;
byte    dayNoDst;
byte    monthNoDst;
byte    yearNoDst;
// byte  temperature;   NON UTILIZZATO

boolean bLedBlink     = false;  // abilitazione al lampeggio dei LED (modalità SET_CLOCK_MODE)
boolean bLedOn        = true;   // led lampeggianti accesi (modalità SET_CLOCK_MODE)
boolean bToSet        = false;  // abilitazione al set dell'RTC in uscita dalla modalità SET_CLOCK_MODE
byte    ledInTest     = 255;    // numero del LED in test (modalità LED_TEST_MODE)

static bool USB_SERIAL = true;
static bool BLUETOOTH_SERIAL = false;

//ENUM_MODE mode;                   // modalità attiva
//ENUM_MODE oldMode;                // modalità attiva precedente
int mode;                   // modalità attiva
int oldMode;                // modalità attiva precedente

//ENUM_SETCLOCK_SUBSTATUS setClockSubStatus;        // sotto stato set_clock
//ENUM_SETCLOCK_SUBSTATUS oldsSetClockSubStatus;    // sotto stato set_clock set precedente
int setClockSubStatus;      // sotto stato set_clock
int oldsSetClockSubStatus;  // sotto stato set_clock set precedente

boolean bChangeToSTANDARD_MODE = false;     // cambio mode da seriale
boolean bChangeToLED_TEST_MODE = false;     // cambio mode da seriale

byte    setHours = 0;          // ora da impostare in modalità SET_CLOCK_MODE
byte    setMin   = 0;          // minuti da impostare in modalità SET_CLOCK_MODE

int cntRefreshXWriteOnSerial = 0;   // contatore ms per calcolo ciclo di scrittura su seriale
int cntMsRefresh = 0;               // contatore ms per calcolo ciclo di refresh
int msBlink      = 0;               // millisecondi in blink

boolean bRefreshCycle  = false; // ciclo di refresh attivo: lettura di RTC, ricalcolo output, test LED
boolean bWriteOnSerial = false; // cliclo di scrittura valori RTC e WordClock su seriale attivo

byte day4Month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

 enum E_CHECKDATATIME_CODE {
	CHECKDATATIME_OK,         // 0
	CHECKDATATIME_WRONG_YY,   // 1
	CHECKDATATIME_WRONG_MM,   // 2
	CHECKDATATIME_WRONG_DD,   // 3
	CHECKDATATIME_WRONG_hh,   // 4
	CHECKDATATIME_WRONG_mm,   // 5
	CHECKDATATIME_WRONG_ss,   // 6
	NUM_CHECKDATATIME_CODE
};

struct a {
	const char* s;
};
a checkDateDimeString[NUM_CHECKDATATIME_CODE] = {
	{"OK"},
	{"wrong years value. Must be in 17 .. 31 range"},
	{"wrong months value"},
	{"wrong days value"},
	{"wrong hours value"},
	{"wrong minutes value"},
	{"wrong seconds value"},
};

// ***************************************************************************************************
// Gestione PWM per variare la luminosità dei led
//
#define OUT_PWM 2
byte PWM_dutyCycle;
byte old_PWM_dutyCycle;

// ***************************************************************************************************
// GESTIONE ORA LEGALE
/*
  nel 2010 l'Italia con l'art. 22 della legge 96, recependo la direttiva 2000/84/CE del Parlamento europeo
  (direttiva a firma del presidente N. FONTAINE),
  fissò:
	l'inizio dell'ora legale alle ore 2:00 del mattino dell'ultima domenica di marzo e
	il termine alle 3:00 del mattino dell'ultima domenica di ottobre
	NOTARE che le 3:00 sono in oraa legale quaindi riferendosi all'ora solare il termine è alle ore 2:00
*/
#define HOURSTARTDST 2
#define HOURSTOPDST 2

byte monthStartDST = 3; // marzo    mese di inizio
byte monthStopDST = 10; // ottobre  mese di fine

struct myDayDST {
	byte start; // giorno di inizio dell'ora legale in funzione dell'anno alle ore 02:00 mettere l’orologio un’ora avanti
	byte stop;  // giorno di fine dell'ora legale in funzione dell'anno alle ore 03:00 mettere l’orologio un’ora indietro
};

myDayDST dayDST[] = {
//  start stop
	{26,   29},   // 2017
	{25,   28},   // 2018
	{31,   27},   // 2019
	{29,   25},   // 2020
	{28,   31},   // 2021
	{27,   30},   // 2022
	{26,   29},   // 2023
	{31,   27},   // 2024
	{30,   26},   // 2025
	{29,   25},   // 2026
	{28,   31},   // 2027
	{26,   29},   // 2028
	{25,   28},   // 2029
	{31,   27},   // 2030
	{30,   26}    // 2031
};

// ###################################################################################################
//
//      CODICE
//
// ###################################################################################################

    
/* ***************************************************************************************
  lettura degli input
  aggiornamento dei fronti di salita e di discesa e del tempo di permanenza
  nel livello dei segnali
 * **************************************************************************************/
void ReadInput ()
{
	register myPuls* pPuls = &Puls[0];
	boolean bprint = false;
	for (byte item = 0; item < NUM_PULS; item ++, pPuls++)
	{
		pPuls->bOldIn = pPuls->bIn;                         // valore al giro precedente
		pPuls->bIn    = digitalRead(pPuls->inputAddress);   // lettura input

		// se ho un fronte lo memorizzo, se al giro dopo ho il livello congruente con il fronte
		// rilevato dico di avere avuto un fronte, altrimenti nessun fronte
		if (pPuls->bIn != pPuls->bOldIn)
		{ // calcolo fronti di salita e discesa temporanei
			bprint = true;
			pPuls->msInLevel = 0;
			pPuls->bRiseEdge = false;
			pPuls->bFallEdge = false;
			pPuls->bTmpRiseEdge = pPuls->bIn;
			pPuls->bTmpFallEdge = !pPuls->bIn;
		}
		else
		{ // calcolo permanenza nel livello e fronti e livelli filtrati
			if (pPuls->bTmpRiseEdge)
			{
				bprint = true;
				pPuls->bRiseEdge = pPuls->bIn;
				pPuls->bTmpRiseEdge = false;
				pPuls->bTmpFallEdge = false;
			}
			else if (pPuls->bTmpFallEdge)
			{
				bprint = true;
				pPuls->bFallEdge = !pPuls->bIn;
				pPuls->bTmpRiseEdge = false;
				pPuls->bTmpFallEdge = false;
			}
			else
			{
				pPuls->msInLevel += MS_CYCLE;
				pPuls->bTmpRiseEdge = false;
				pPuls->bTmpFallEdge = false;
				pPuls->bRiseEdge = false;
				pPuls->bFallEdge = false;
			}
		}
		if (bprint)
		{
			char txt[64];
			sprintf(txt, "input:%2d value:%1d (old:%1d) RiseEdge:%1d FallEdge:%1d",
			pPuls->inputAddress,
			pPuls->bIn,
			pPuls->bOldIn,
			pPuls->bRiseEdge,
			pPuls->bFallEdge);
			Serial.println (txt);
			bprint = false;
		}
	}
}

/* ***************************************************************************************
  Scrittura degli output
  Sono scritti solo se diversi dal giro precedente
 * **************************************************************************************/
void WriteOutput ()
{
	register myLed* pLed = &Led[0];

	if (PWM_dutyCycle != old_PWM_dutyCycle)
	{
		analogWrite(OUT_PWM, PWM_dutyCycle);
		old_PWM_dutyCycle = PWM_dutyCycle;
	}

	for (byte item = 0; item < NUM_LED; item ++, pLed++)
	{
		if (pLed->bOut != pLed->bOldOut)
		{
			digitalWrite(pLed->outputAddress, pLed->bOut ? HIGH : LOW);
			pLed->bOldOut = pLed->bOut;
		}
	}

	// scrittura su seriale dell'ora
	if (bWriteOnSerial)
	{
		char txt [64];
//		Serial.print(Status[mode].associateString);
		Serial.print(States.GetStatusName());
		Serial.print(" - ");
		if (mode != LED_TEST_MODE)
		{
			Serial.print("It is");
		}
		pLed = &Led[0];
		for (byte item = 0; item < NUM_LED; item ++, pLed++)
		{
			if (pLed->bOut)
			{
				if (mode == LED_TEST_MODE)
				{
					sprintf (txt, "led#:%d out addr:%d -> ", item, pLed->outputAddress);
					Serial.print (txt);
				}
				Serial.print(pLed->associateString);
			}
		}
		Serial.print("\n");
//		sprintf (txt, "*#WordClock - %s *", Status[mode].associateString);
		sprintf (txt, "*#WordClock - %s *", States.GetStatusName());
		bluetooth.print(txt);
	}
}

/* ***************************************************************************************
  Calcolo ora legale
 * **************************************************************************************/
bool NowInDST()
{
	if ((monthStartDST < monthNoDst) && (monthNoDst < monthStopDST))
	{
		return true;
	}
	byte ind = yearNoDst - 17;
	if (monthStartDST == monthNoDst)
	{
		if(dayNoDst > dayDST[ind].start)
		{
			return true;
		}
		if(dayNoDst < dayDST[ind].start)
		{
			return false;
		}
		byte hour24 = (PM==0)? hourNoDst : hourNoDst +12;
		if (hour24 < HOURSTARTDST)
		{
			return false;
		}
		return true;
	}

	if (monthStopDST == monthNoDst)
	{
		if(dayNoDst > dayDST[ind].stop)
		{
			return false;
		}
		if(dayNoDst < dayDST[ind].stop)
		{
			return true;
		}
		byte hour24 = (PM==0)? hourNoDst : hourNoDst +12;
		if (hour24 < HOURSTOPDST)
		{
			return true;
		}
		if (hour24 >= HOURSTOPDST)
		{
			return false;
		}
		return false;
	}
	return false;
}

/* =======================================================================================
   =======================================================================================
	GESTIONE Real Time Clock
   =======================================================================================
   ======================================================================================= */
/* ***************************************************************************************
  Set del Real Time Clock
 * **************************************************************************************/
void SetDS3231()
{
	if (NowInDST())
	{
		hourNoDst = hour - 1;
		if (hourNoDst == 255)
		{
			hourNoDst = 23;
		}
	}
	else
	{
		hourNoDst = hour;
	}
	Clock.setSecond(00);        //Set the second
	Clock.setMinute(minute);    //Set the minute
	Clock.setHour(hourNoDst);   //Set the hour
//  Clock.setDoW(1);            //Set the day of the week
	Clock.setDate(day);         //Set the date of the month
	Clock.setMonth(month);      //Set the month of the year
	Clock.setYear(year);        //Set the year (Last two digits of the year)
	Clock.setClockMode(true);
}

/* ***************************************************************************************
	Legge i valori da RTC e
	li scrive su seriale se il modo è STANDARD_MODE o SET_CLOCK_MOSE e se
	il ciclo è abilitato alla scrittura su seriale
* **************************************************************************************/
void ReadDS3231()
{
	secondNoDst = Clock.getSecond();
	minuteNoDst = Clock.getMinute();
	hourNoDst   = Clock.getHour(h12, PM);
	dayNoDst    = Clock.getDate();
	monthNoDst  = Clock.getMonth(Century);
	yearNoDst   = Clock.getYear();
//  temperature = Clock.getTemperature();
	second = secondNoDst;
	minute = minuteNoDst;
	day    = dayNoDst;
	month  = monthNoDst;
	year   = yearNoDst;
	if (NowInDST())
	{
		hour = hourNoDst + 1;
		if (hour >= 24)
		{
			hour = 1;
		}
	}
	else
	{
		hour = hourNoDst;
	}

	if (mode == LED_TEST_MODE)
	{
		return;
	}
	if (bWriteOnSerial)
	{
		ShowDateTimeOnSerial();
	}
}

/* ***************************************************************************************
	Scrive su seriale i valori letti da RTC
 * **************************************************************************************/
void ShowDateTimeOnSerial()
{
	char txt[64];
	sprintf(txt, "From RTC %2d-%2d-%2d %2d:%2d:%2d %s DST:%s transalte: ",
					yearNoDst,
					monthNoDst,
					dayNoDst,
					hourNoDst,
					minuteNoDst,
					secondNoDst,
					(PM==1)?"PM":"AM",
					NowInDST()? "YES":"NO");
	Serial.print (txt);
}

/* =======================================================================================
   =======================================================================================
	MACCHINA A STATI PRINCIPALE
   =======================================================================================
   ======================================================================================= */
 void ResetCouterAndFlag()
 {
// !!!!!!!!!!!!!	Status[mode].msInStatus = 0;
// !!!!!!!!!!!!!	Status[mode].bmaxMsInStatus = false;
 }

/* =======================================================================================
   =======================================================================================
   Gestione di STANDARD MODE
   =======================================================================================
   ======================================================================================= */
/* ***************************************************************************************
	Gestione dello stato STANDARD_MODE
	Ad ogni ciclo di refresch aggiorna i valori di ore e minuti leggendo l'RTC
	Calcola i led da accendere in funzione del valore di ore e minuti
 * ***************************************************************************************/
void StandardModeStatus(void* pStructData)
{
	if (bRefreshCycle)
	{
		ReadDS3231();
	}
	CalcWordClock(minute, hour);
}

/* ***************************************************************************************
	Gestione ingresso nello stato STANDARD_MODE
 * **************************************************************************************/
void StandardModePickUp(void* pStructData)
{
	ResetCouterAndFlag();
	bChangeToSTANDARD_MODE = false;
}

/* ***************************************************************************************
	Gestione cambio stato STANDARD_MODE:
	premendo il pulsante Mode per almeno MIN_MS_TO_SET_MODE ms si entra
	in modalità SET_CLOCK
	permendo il pulsante SET o ricevendo il comando "ET" da seriale si entra in LED_TEST
 * ***************************************************************************************/
int StandardModeChangeStatus(void* pStructData)
{
	if ((Puls[PULS_MODE].bRiseEdge || Puls[PULS_MODE].bIn))
	{
		if (Puls[PULS_MODE].msInLevel >= MIN_MS_TO_SET_MODE)    // il pulsante deve essere premuto almeno MIN_MS_TO_SET_MODE
		{
			return SET_CLOCK_MODE;
		}
	}
	else if ((Puls[PULS_SET].bRiseEdge) ||
			 bChangeToLED_TEST_MODE)
	{
		return LED_TEST_MODE;
	}
	return STANDARD_MODE;
}

/* =======================================================================================
   =======================================================================================
   Gestione di SET_CLOCK MODE
   =======================================================================================
   ======================================================================================= */
/* ***************************************************************************************
  lampeggio dei led dei minuti o delle ore a seconda
  di cosa si sta settando
 * **************************************************************************************/
void BlinkSettingLed()
{
	register myLed* pLed = &Led[0];

	bLedOn = !bLedOn;
	msBlink = 0;
	if (!bLedOn)
	{ // i led che lampeggiano devono essere spenti
		switch (setClockSubStatus)
		{
			case SET_HOUR:
				for (byte item = 0; item < NUM_LED; item++, pLed++)
				{
					if (!pLed->bAssociateToMinutes)
					{
						pLed->bOut = false;
					}
				}
				break;
			case SET_MINUTES:
				for (byte item = 0; item < NUM_LED; item++, pLed++)
				{
					if (pLed->bAssociateToMinutes)
					{
						pLed->bOut = false;
					}
				}
				break;
			default:
				break;
		}
	}
	else
	{
		for (byte item = 0; item < NUM_LED; item++, pLed++)
		{
			pLed->bOut = pLed->bBlinkOut;
		}
	}
}

/* ***************************************************************************************
	in funzione di quale pulsante è premuto incrementa (PULS_NEXT_HM) o
	decrementa (PULS_PREV_HM) il valore passato
	ad ogni pressione di pulsante di incremento o decremento
	azzera il contatore di permanenza nello stato SET_CLOCL_MODE
 * **************************************************************************************/
byte IncDecValue(byte value, byte maxVal)
{
	if (Puls[PULS_NEXT_HM].bRiseEdge)
	{
// !!!!!!!!!!!		Status[mode].msInStatus = 0;
		value++;
		if (value >= maxVal)
		{
			value = 0;
		}
	}
	else if (Puls[PULS_PREV_HM].bRiseEdge)
	{
// !!!!!!!!!!!		Status[mode].msInStatus = 0;
		value--;
		if (value == 255)
		{
			value = maxVal - 1;
		}
	}
	return value;
}

/* ***************************************************************************************
	Gestione stato SET_HOUR
	incrementa o decerementa il valore dell'ora
 * **************************************************************************************/
void SetHourStatus(void* pStructData)
{
	setHours = IncDecValue(setHours, 24);
}

/* ***************************************************************************************
	Gestione cambio stato SET_HOUR
	premendo il pulsate SET si passa ad impostare i minuti
 * **************************************************************************************/
int SetHourChangeStatus(void* pStructData)
{
	if (Puls[PULS_SET].bRiseEdge)
	{
		return SET_MINUTES;
	}
	return SET_HOUR;
}

/* ***************************************************************************************
	Gestione stato SET_MINUTES
	incrementa o decerementa il valore dei minuti
 * **************************************************************************************/
void SetMinutesStatus(void* pStructData)
{
	setMin = IncDecValue(setMin, 60);
}

/* ***************************************************************************************
	Gestione ingresso nello stato SET_MINUTES
	azzera il contatore di permanenza nello stato SET_CLOCL_MODE
 * **************************************************************************************/
void SetMinutesPickUp (void* pStructData)
{
	// !!!!!!!!!!!!!!! Status[mode].msInStatus = 0;
}

/* ***************************************************************************************
	Gestione cambio stato SET_MINUTES
	premendo il pulsate SET si passa nello stato FINISH dove si imposta l'RTC
 * **************************************************************************************/
int SetMinutesChangeStatus(void* pStructData)
{
	if (Puls[PULS_SET].bRiseEdge)
	{
		return SET_FINISH;
	}
	return SET_MINUTES;
}

/* ***************************************************************************************
	Gestione ingresso nello stato SET_FINISH
 * **************************************************************************************/
void SetFinishPickUp(void* pStructData)
{
	bLedBlink = false;
	bToSet = true;
}

/* ***************************************************************************************
	Gestione cambio stato SET_FINISH
 * **************************************************************************************/
int SetFinishChangeStatus(void* pStructData)
{
	return SET_FINISH;
}

/* ***************************************************************************************
	Gestione dello stato SET_CLOCK_MODE
	abilita il lampeggio dei led che si stanno impostando
	gestisce il set di ore e minuti (macchina a stati)
	calcola i led da accendere in funzione dei valori di ore e minuti che si stanno impostando
 * **************************************************************************************/
void SetClockModeStatus(void* pStructData)
{
	register myLed* pLed = &Led[0];

	if (msBlink >= MS_X_BLINK)
	{
		BlinkSettingLed();
	}

	oldsSetClockSubStatus = SetClockSubStates.GetStatusInd();
	SetClockSubStates.Manage();
	setClockSubStatus = SetClockSubStates.GetStatusInd();
//	StateMachine(SetClockSubStatus, (int&)setClockSubStatus, (int&)oldsSetClockSubStatus);

	CalcWordClock(setMin, setHours);

	for (byte item = 0; item < NUM_LED; item ++, pLed++)
	{
		pLed->bBlinkOut = pLed->bOut;
	}
}

/* ***************************************************************************************
	Gestione ingresso nello stato SET_CLOCK_MODE
 * **************************************************************************************/
void SetClockModePickUp (void* pStructData)
{
	register myLed* pLed = &Led[0];

	ResetCouterAndFlag();
	setClockSubStatus       = SET_HOUR;
	oldsSetClockSubStatus   = SET_HOUR;
	setMin      = minute;
	setHours    = (PM ? hour + 12 : hour);
	bLedBlink   = true;
	bLedOn      = true;
	bToSet      = false;
	for (byte item = 0; item < NUM_LED; item ++, pLed++)
	{
		pLed->bBlinkOut = pLed->bOut;
	}
}

/* ***************************************************************************************
	Gestione uscita dallo stato SET_CLOCK_MODE
	imposta l'RTC se deve farlo
	disabilita il lampeggio dei led
 * **************************************************************************************/
void SetClockModeDropOut (void* pStructData)
{
	if (bToSet)  // devo fare il set di RTC
	{
		bToSet = false;
		minute = setMin;
		hour   = setHours;
		SetDS3231();
	}
	bLedBlink = false;
}

/* ***************************************************************************************
	Gestione cambio stato SET_CLOCK_MODE
	si ritorna in STANDARD_MODE:
	- se si è concluso il set di ore e minuti (si esegue il set di RTC)
	- premendo il pulsnato MODE (non si esegue alcun set di RTC)
	- è superato il tempo massimo di permanenza nello stato (non si esegue alcun set di RTC)
 * **************************************************************************************/
int SetClockModeChangeStatus(void* pStructData)
{
	if ((setClockSubStatus == SET_FINISH) ||
		Puls[PULS_MODE].bRiseEdge // !!!!!!!!!!! ||
		// !!!!!!!!!!! Status[SET_CLOCK_MODE].bmaxMsInStatus
		)
	{
		return STANDARD_MODE;
	}
	return SET_CLOCK_MODE;
}

/* =======================================================================================
   =======================================================================================
   Gestione di LED_TEST_MODE
   =======================================================================================
   ======================================================================================= */
/* ***************************************************************************************
   Gestione dello stato LED_TEST_MODE
   Accende, uno alla volta, tutti i led csambiando il led acceso ad ogni clclo di refresh
 * **************************************************************************************/
void LedTestModeStatus(void* pStructData)
{
	if (!bRefreshCycle)
	{
		return;
	}
	if (ledInTest != 255)
	{
		Led[ledInTest].bOut = false;
	}

	ledInTest++;

	if (ledInTest >= NUM_LED)
	{
		ledInTest = 0;
	}

	Led[ledInTest].bOut = true;
}

/* ***************************************************************************************
	Gestione ingresso nello stato LED_TEST_MODE
	spegne tutti i led
 * **************************************************************************************/
void LedTestModePickUp (void* pStructData)
{
	register myLed* pLed = &Led[0];

	ResetCouterAndFlag();

	bChangeToLED_TEST_MODE = false;
	for (byte item = 0; item < NUM_LED; item++, pLed++)
	{
		pLed->bOut = false;
	}
}

/* ***************************************************************************************
	Gestione cambio stato LED_TEST_MODE
	qualuque pulsante si prema o
	se è arrivato comando da seriale o
	se è passato il tempo massimo nello stato torna a STANDARD_MODE
 * **************************************************************************************/
int LedTestModeChangeStatus(void* pStructData)
{
	if (Puls[PULS_MODE].bRiseEdge       ||
		Puls[PULS_SET].bRiseEdge        ||
		Puls[PULS_NEXT_HM].bRiseEdge    ||
		Puls[PULS_PREV_HM].bRiseEdge    ||
		bChangeToSTANDARD_MODE // !!!!!!!!!!! ||
		// !!!!!!!!!!! Status[LED_TEST_MODE].bmaxMsInStatus
		)
	{
		return STANDARD_MODE;
	}
	return LED_TEST_MODE;
}

/* =======================================================================================
   =======================================================================================
   Gestione input da seriale
   =======================================================================================
   ======================================================================================= */

 // DA FARE
int CharAvailableOnSerial(bool bSerial)
{
	if (bSerial)
	{
		return Serial.available();
	}
	else
	{
		return bluetooth.available();
	}
}

char SerialRead(bool bSerial)
{
	if (bSerial)
	{
		return Serial.read();
	}
	else
	{
		return bluetooth.read();
	}
}

int SerialParseInt(bool bSerial)
{
	if (bSerial)
	{
		return Serial.parseInt();
	}
	else
	{
		return bluetooth.parseInt();
	}
}

size_t SerialPrintln(bool bSerial, const char* c)
{
	if (bSerial)
	{
		return Serial.println(c);
	}
	else
	{
		return bluetooth.println(c);
	}
}

size_t SerialPrintln(bool bSerial, int val, int format)
{
	if (bSerial)
	{
		return Serial.println(val, format);
	}
	else
	{
		return bluetooth.println(val, format);
	}
}

/* ***************************************************************************************
   controlla i valori di data ed ora passati, se ok return TRUE
 * **************************************************************************************/
byte CheckDateAndHourValue(byte yy, byte MM, byte dd, byte hh, byte mm, byte ss)
{
	if ((yy < 17) || (yy > 31))
	{
		return CHECKDATATIME_WRONG_YY;
	}
	if (MM > 12)
	{
		return CHECKDATATIME_WRONG_MM;
	}
	if (MM == 2)    // controllo su anno bisestile
	{
		if (dd > (((2000+yy)%4)==0) ? 29 : 28)
		{
			return CHECKDATATIME_WRONG_DD;
		}
	}
	else if (dd > day4Month [MM])
	{
		return CHECKDATATIME_WRONG_DD;
	}
	if (hh > 24)
	{
		return CHECKDATATIME_WRONG_hh;
	}
	if (mm > 60)
	{
		return CHECKDATATIME_WRONG_mm;
	}
	if (ss > 60)
	{
		return CHECKDATATIME_WRONG_ss;
	}
	return CHECKDATATIME_OK;
}

byte ParseToDateTime (bool bSerial)
{
	year    = SerialParseInt(bSerial);
	month   = SerialParseInt(bSerial);
	day     = SerialParseInt(bSerial);
	hour    = SerialParseInt(bSerial);
	minute  = SerialParseInt(bSerial);
	second  = SerialParseInt(bSerial);
	return CheckDateAndHourValue(year, month, day, hour, minute, second);
}

/* ***************************************************************************************
   Controlla se il carattere passato è corretto per il comando SERIAL_CMD_LIGHT_SET
   return true se ok, false altrimenti
 * **************************************************************************************/
bool IsCharSetLightLevel(char c)
{
	return ((c == 'L') || (c == 'l'));
}

bool ParseSetLightLevel(bool bSerial)
{
	char c1 = SerialRead(bSerial);
	if (IsCharSetLightLevel(c1))
	{
		PWM_dutyCycle = SerialParseInt(bSerial);
		return true;
	}
	return false;
}


/* ***************************************************************************************
   abilita o disabilita il LED TEST in funzine dei valori letti
	   "ET" abilita led test mode (case insensitive)
	   qualunque altra coppia di caratteri lo disabilite
 * **************************************************************************************/
void CheckLedTestEnable (char c1, char c2)
{
	if (((c1 == 'E') || (c1 == 'e')) &&
		((c2 == 'T') || (c2 == 't')))
	{
		bChangeToLED_TEST_MODE = true;
	}
	else
	{
		bChangeToSTANDARD_MODE = true;
	}
}

void ParseLedTestEnable(bool bSerial)
{
	char c1 = SerialRead(bSerial);
	char c2 = SerialRead(bSerial);
	CheckLedTestEnable (c1, c2);
}

void SerialHelp()
{
	Serial.read();
	Serial.println("Available command: into []");
	Serial.println("[any 1 char] this help");
	Serial.println("[ET] to Enable Led Test");
	Serial.println("     any other couple of char disable Led Test");
	Serial.println("[AA,MM,DD,hh,mm] to clock set");
	Serial.println("[Lnnn] to change Led light intensity");
	Serial.println("       nnn value from 000 (off) to 255 (full light)");
}

/* ***************************************************************************************
   Bluetoot HELP
 * **************************************************************************************/
void BluetoothHelp()
{
	//char c1 = bluetooth.read();
	bluetooth.read();
	bluetooth.println("*@Available command: into []");
	bluetooth.println("*@[any 1 char] this help");
	bluetooth.println("*@[ET] to Enable Led Test");
	bluetooth.println("*@     any other couple of char disable Led Test");
	bluetooth.println("*@[AA,MM,DD,hh,mm] to clock set");
	bluetooth.println("*@[Lnnn] to change Led light intensity");
	bluetooth.println("*@       nnn value from 000 (off) to 255 (full light)");
}

void ManageAllSerialCommand(bool bSerial)
{
	static byte serialNumLoop = 0;
	static byte bluetoothNumLoop = 0;
	static byte maxLoop = 5;
	byte numLoop = (bSerial) ? serialNumLoop : bluetoothNumLoop;
	char txt[64];

	switch (CharAvailableOnSerial(bSerial))
	{
		int nc;
		nc = CharAvailableOnSerial(bSerial);
		if ((nc > SERIAL_CMD_MAX_LEN) || (numLoop > maxLoop))
		{
			while (CharAvailableOnSerial(bSerial))
			{
				 SerialRead(bSerial);
			}
			if (bSerial) sprintf(txt, "flushing %d chararacters", nc);
			else sprintf(txt, "*@flushing %d chararacters", nc);
			SerialPrintln(bSerial, txt);
		}
		case SERIAL_CMD_SET_CLOCK: // acquisisce la stringa con data ed ora
				numLoop = 0;
				byte retCode;
				retCode = ParseToDateTime(bSerial);
				if (retCode == CHECKDATATIME_OK)
				{
					SetDS3231();
					if (bSerial) sprintf(txt, "--- Well done, clock set! :)");
					else sprintf(txt, "*@--- Well done, clock set! :)");
					SerialPrintln(bSerial, txt);
				}
				else
				{
					if (bSerial)
					{
						Serial.println("*** ERROR parsing command!***");
						Serial.println(checkDateDimeString[retCode].s);
						Serial.println("check the command string");
						Serial.println("must be in YY;MM;DD;hh:mm format");
						Serial.println("and have valid values");
					}
					else
					{
						bluetooth.println("*@*** ERROR parsing command!***");
						sprintf(txt, "*@%s",checkDateDimeString[retCode].s);
						bluetooth.println(txt);
						bluetooth.println("*@check the command string");
						bluetooth.println("*@must be in YY;MM;DD;hh:mm format");
						bluetooth.println("*@and have valid values");
					}
				}
				break;
		case SERIAL_CMD_HELP:
				numLoop = 0;
				(bSerial) ? SerialHelp() : BluetoothHelp();
		case SERIAL_CMD_ENABLE_LEDTEST: // controlla se e' in arrivo una stringa da 2 caratteri ET (Enable Test) oppure qualunque coppia di caratteri per disabilitare
				numLoop = 0;
				ParseLedTestEnable(bSerial);
				break;
		case SERIAL_CMD_LIGHT_SET:
				if (!ParseSetLightLevel(bSerial))
				{
					if (bSerial) Serial.println("*** ERROR parsing command!***");
					else bluetooth.println("*@*** ERROR parsing command!***");
				}
				else
				{
					if (bSerial) Serial.println("--- Well done, duty cycle uptated! :)");
					else bluetooth.println("*@--- Well done, duty cycle uptated! :)");
				}
				break;
		default:
				numLoop++;
				break;
	}
	(bSerial) ? (serialNumLoop = numLoop) : (bluetoothNumLoop = numLoop);
}

/* =======================================================================================
   =======================================================================================
	Gestione HOUSE CLOCK
   =======================================================================================
   ======================================================================================= */

/* ***************************************************************************************
	Se il ciclo attuale è un ciclo di refresh calcola i led da accendere
	in funzione dei valori di ore e minuti passati
	in STANDARD_MODE i valori delle ore arrivano a 12
	in SET_ClOCK_MODE i valori delle ore arrivano a 24
 * **************************************************************************************/
void CalcHouseClock(byte vminute, byte vhour)
{
	boolean bPM;

	if (!bRefreshCycle)
	{
		return;
	}

	register myLed* pLed = &Led[0];
	for (byte item = 0; item < NUM_LED; item++, pLed++)
	{
		pLed->bOut = false;
	}

    // BOZZA DI ROUTINE DI VISUALIZZAZIONE
    
    #define NCOLONNE 4
    #define NRIGHE 4
    
    byte ore_minuti[NCOLONNE];
    
    byte nrigaxcolonna[NCOLONNE] = {1,4,4,4};
    
    // con modulo e divisione recupero decine ed unità dai valori di ore e minuti
    ore_minuti[0] = vhour / 10;       // ore decine
    ore_minuti[1] = vhour % 10;       // ore unità
    ore_minuti[2] = vminute / 10;    // minuti decine
    ore_minuti[3] = vminute % 10;    // minuti unità
    
    // for sulle colonne in modo da calcolare l'accensione dei led di una colonna per volta
    for (int colonna = 0; colonna < NCOLONNE; riga++)
    {
        // for sulle righe partendo dal basso dove c'è il led che identifica il bit meno significativo
        for (int riga = nrigaxcolonna[colonna] -1; riga >= 0; riga--)
        {
            // facendo l'AND bit a bit con 1 ottengo il valore del bit meno significativo che
            // corrisponde al led acceso / spento
            pLed[(colonna * NRIGHE) - riga].bOut = (ore_minuti[riga] && 1) == 1;
            // per passare al bit successivo faccio uno shift a destra di una posizione del valore
            ore_minuti[riga] = ore_minuti[riga] >> 1;
        }
    }

/* calcolo dell'indice del vettore Led -> indice = (colonna * NRIGHE) - riga
colonna = 0
    riga = 0   = 0 x 4 + 0       //	LED_1_DEC_ORE,	  0   meno significativo
                     
colonna = 1          
    riga = 0   = 1 x 4 - 3 = 1   //	LED_1_UNI_ORE,    1   meno significativo
    riga = 1   = 1 x 4 - 2 = 2   //	LED_2_UNI_ORE,    2   
    riga = 2   = 1 x 4 - 1 = 3   //	LED_4_UNI_ORE,    3
    riga = 3   = 1 x 4 - 0 = 4   //	LED_8_UNI_ORE,    4   più significativo
                     
colonna = 2          
    riga = 0   = 2 x 4 - 3 = 5   //	LED_1_DEC_MIN,    5   meno significativo
    riga = 1   = 2 x 4 - 2 = 6   //	LED_2_DEC_MIN,    6
    riga = 2   = 2 x 4 - 1 = 7   //	LED_4_DEC_MIN,    7
    riga = 3   = 2 x 4 - 0 = 8   //	LED_8_DEC_MIN,    8   più significativo
                     
colonna = 3          
    riga = 0   = 3 x 4 - 3 = 9   //	LED_1_UNI_MIN,    9   meno significativo
    riga = 1   = 3 x 4 - 2 = 10  //	LED_2_UNI_MIN,   10
    riga = 2   = 3 x 4 - 1 = 11  //	LED_4_UNI_MIN,   11
    riga = 3   = 3 x 4 - 0 = 12  //	LED_8_UNI_MIN,   12   più significativo
                
*/
}


/* ***************************************************************************************
	Calcoli vari in funzione dei cicli:
	- bRefreshCycle:  se il ciclo attuale se è un ciclo di refresh
	- bWriteOnSerial: se il ciclo attuale è abilitato alla scrittura su seriale
	- msBlink: se attivo il blink calcola il tempo di blink
 * **************************************************************************************/
void ManageCycleCounters()
{
	// determina se nel ciclo attuale deve essere fatto un refresh
	cntMsRefresh += MS_CYCLE;
	if (cntMsRefresh >= MS_X_REFRESH)
	{
		bRefreshCycle = true;
		cntMsRefresh = 0;
	}

	// in caso di modalità LED_TEST_MODE il ciclo di scrittura corrisponde a quello di refresh
	if (mode == LED_TEST_MODE)
	{
		cntRefreshXWriteOnSerial = 0;
		bWriteOnSerial = bRefreshCycle;
	}
	else
	{
		// determina se nel ciclo attuale deve essere fatta la scrittura su seriale
		if (bRefreshCycle)
		{
			cntRefreshXWriteOnSerial ++;
		}
		if (cntRefreshXWriteOnSerial >= CICLE_REFRESH_X_WRITE_ON_SERIAL)
		{
			bWriteOnSerial = true;
			cntRefreshXWriteOnSerial = 0;
		}
	}

	// se attivo il blink calcola il tempo di blink
	if (bLedBlink)
	{
		msBlink += MS_CYCLE;
	}
}

/* =======================================================================================
   =======================================================================================
	INIZIALIZZAZIONI
   =======================================================================================
   ======================================================================================= */
void setup()
{
	// Start the I2C interface
	Wire.begin();

	// Start the serial interface
	Serial.begin(115200);

	bluetooth.begin(9600);

	States.AssignData(nullptr, 0);
	
	States.AssignState( 0, 
						StandardModeStatus,
						nullptr,
						nullptr,
						StandardModePickUp,
						StandardModeChangeStatus,
						0,
						0,
						"Standard"); 
	
	States.AssignState(	1, 
						SetClockModeStatus,
						SetClockModeDropOut,
						nullptr,
						SetClockModePickUp,
						SetClockModeChangeStatus,
						0,
						0,
						"SetClock"); 
	
	States.AssignState(	1, 
						LedTestModeStatus,
						nullptr,
						nullptr,
						LedTestModePickUp,
						LedTestModeChangeStatus,
						0,
						0,
						"TestLed"); 

	SetClockSubStates.AssignData(nullptr, 0);
	SetClockSubStates.AssignState( 0, 
						SetHourStatus,
						nullptr,
						nullptr,
						nullptr,
						SetHourChangeStatus,
						0,
						0,
						"SET_HOUR"); 
	
	SetClockSubStates.AssignState(	1, 
						SetMinutesStatus,
						nullptr,
						nullptr,
						SetMinutesPickUp,
						SetMinutesChangeStatus,
						0,
						0,
						"SET_MINUTES"); 
	
	SetClockSubStates.AssignState(	1, 
						nullptr,
						nullptr,
						nullptr,
						SetFinishPickUp,
						SetFinishChangeStatus,
						0,
						0,
						"SET_FINISH"); 
						
	byte    item;
	myLed*  pLed  = &Led[0];
	myPuls* pPuls = &Puls[0];

	// inizializzazione input
	for (item = 0; item < NUM_PULS; item++, pPuls++)
	{
		pinMode(Puls->inputAddress, INPUT);
	//  digitalWrite(Puls->inputAddress, HIGH); le resistenze si pull-down sono hw
	}

	// inizializzazione output
	for (item = 0; item < NUM_LED; item++, pLed++)
	{
		pLed->bOut = false;
		pLed->bOldOut = false;
		pinMode(pLed->outputAddress, OUTPUT);
	}

	// duty cycle per variare luminosità dei led
	pinMode(OUT_PWM, OUTPUT);
	PWM_dutyCycle = 255;
	old_PWM_dutyCycle = 255;

	mode    = STANDARD_MODE;
	oldMode = STANDARD_MODE;
	cntMsRefresh = 0;
	msBlink      = 0;
	bLedBlink    = false;
	bLedOn       = true;
	bToSet       = false;

	delay(100);
	Serial.println("Type: AA;MM,GG,hh,mm to set Clock");
	Serial.println("      ET  (Enable Test) to change mode to LED_TEST");
	Serial.println("      Lnnn to change led light intensity");
	Serial.println("           nnn value from 000 (off) to 255 (full light)");
	//Serial.println("      SET to change mode to SET");
	Serial.print("\n");
	delay(500);
}

/* =======================================================================================
   =======================================================================================
	MAIN LOOP
   =======================================================================================
   ======================================================================================= */
void loop()
{
	ManageCycleCounters();

	ManageAllSerialCommand(USB_SERIAL);
	ManageAllSerialCommand(BLUETOOTH_SERIAL);

	ReadInput();

	oldMode = States.GetStatusInd();
	States.Manage();
	mode = States.GetStatusInd();

	WriteOutput();

	bRefreshCycle  = false;
	bWriteOnSerial = false;

	delay(MS_CYCLE);
}
