/*
 Steuert die Deckenlampe in Helligkeit (Strip 1) und Farbe (Strip 2); Steuerung per IR-Fernbedienung; auto.
 Helligkeitssteuerung per LDR;
*/
#include <Arduino.h>
#include <IRremote.h>
#define RECV_PIN 7
#define ldrPIN A5
//

int color = 0;                      //Uebergabe Farbe (0=w, 1=r, 2=g, 3=b, 4=dummy)
const int step = 5;                 //Schrittgroesse PWM Aenderung
const int pwmRANGE = 255;           // PWM Maximum
const int pinPWM[] = {10,5,6,9};    //PWM Pins (weiss, rot, gruen, blau)
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
pin 3 & 11 liegen an TIMER2, welcher von der IRremote lib verwendet wird;
pin 3 kann nur als digitaler Ausgang arbeiten, 11 lässt den Arduino abstürzen.
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
int cmdPWM[] = {255,0,0,0};         //PWM Befehl Werte 0->255
int ledPWM[] = {0,0,0,0};           //PWM Werte LED
const int fadeIncrement = 5;
const long ldrCLK = 600;           //LDR Messungsintervall
long fadeCLK = 10;             //Fader Zeitintervall
unsigned long currentMillis = 0;    //aktive Zeit
unsigned long preAutoMillis = 0;    //letzter Zeitpunkt LDR check
unsigned long preFadeMillis = 0;    //letzter Zeitpunkt Fade
boolean brighten = false;           //dimmen (false) oder (true)
boolean repeat = false;             //Wiederholungsfunktion Fernbedienung; je nach Taste setzen
boolean autoDim = false;            //Automatisches dimmen per LDR
boolean busy = true;
boolean saveSett = false;
IRrecv irrecv(RECV_PIN);
decode_results results;

void setup()
{
irrecv.enableIRIn();                // IR lib aktivieren
Serial.begin(9600);
readFlash();
}

void loop()
/*
 * Je nach Taste muss repeat de-/aktiviert werden.
 * Auto Dimmen umrechnung noch offen
 */
{
 currentMillis = millis();
 if (irrecv.decode(&results))   //IR Dekodieren
 {
 irrecv.resume();               // Signal empfangen


 switch(results.value)          // Abhaengig vom Signal in den richtigen Fall springen
 {

 case 16726725:                 //IR-Signal Weiss heller
     {
     brighten = true;
     repeat = true;
     color = 0;
     dimmer(color,brighten);   //Funktion Dimmer
     Serial.println("weiss heller");
     }
     break;

 case 16759365:                 //IR-Signal Weiss dunkler
     {
     brighten = false;
     repeat = true;
     color = 0;
     dimmer(color,brighten);
     Serial.println("weiss dunkler");
     }
     break;

 case 16720605:                 //IR-Signal Weiss 100%
     {
     repeat = false;
     busy = true;
     nuller();
     cmdPWM[0] = 255;
     fader();
     Serial.println("weiss an");
     }
     break;

 case 16722135:                 //IR-Signal Rot heller
     {
     brighten = true;
     repeat = true;
     color = 1;
     dimmer(color,brighten);   //Funktion Dimmer
     Serial.println("rot heller");
     }
     break;

 case 16713975:                 //IR-Signal Rot dunkler
     {
     brighten = false;
     repeat = true;
     color = 1;
     dimmer(color,brighten);
     Serial.println("rot dunkler");
     }
     break;

 case 16718565:                 //IR-Signal Rot 100%
     {
     nuller();
     busy = true;
     repeat = false;
     cmdPWM[1] = 255;
     fader();
     Serial.println("rot an");
     }
     break;

 case 16754775:                 //IR-Signal Gruen heller
     {
     brighten = true;
     repeat = true;
     color = 2;
     dimmer(color,brighten);   //Funktion Dimmer
     Serial.println("gruen heller");
     }
     break;

 case 16746615:                 //IR-Signal Gruen dunkler
     {
     brighten = false;
     repeat = true;
     color = 2;
     dimmer(color,brighten);
     Serial.println("gruen dunkler");
     }
     break;

 case 16751205:                 //IR-Signal Gruen 100%
     {
     nuller();
     busy = true;
     repeat = false;
     cmdPWM[2] = 255;
     fader();
     Serial.println("gruen an");
     }
     break;

 case 16738455:                 //IR-Signal Blau heller
     {
     brighten = true;
     repeat = true;
     color = 3;
     dimmer(color,brighten);   //Funktion Dimmer
     Serial.println("blau heller");
     }
     break;

 case 16730295:                 //IR-Signal Blau dunkler
     {
     brighten = false;
     repeat = true;
     color = 3;
     dimmer(color,brighten);
     Serial.println("blau dunkler");
     }
     break;

 case 16753245:                 //IR-Signal Blau 100%
     {
     nuller();
     busy = true;
     repeat = false;
     cmdPWM[3] = 255;
     fader();
     Serial.println("blau an");
     }
     break;

 case 16712445:                 //IR-Signal Aus
     {
     nuller();
     busy = true;
     repeat = false;
     fader();
     Serial.println("aus");
     }
     break;

 case 16773135:                 //IR-Signal Auto dimmen
     {
     repeat = false;
     autoDim = !autoDim;
     Serial.println("Auto dimmen, Status:" );
     Serial.println(autoDim);
     break;
     }
 case 16771095:                 //IR-Signal schnell faden
     {
     repeat = false;
     fadeCLK = 10;
     break;
     }
 case 16762935:                 //IR-Signal langsam faden
     {
     repeat = false;
     fadeCLK = 35;
     break;
     }
 case 16745085:
     {
     repeat = false;
     saveSett = ! saveSett;
     if (saveSett == true)
     {
         writeFlash(true);
     }
     else
     {
         writeFlash(false);
     }
     }
 case 4294967295:                 //IR-Signal Taste gedr�ckt
     {
     if (repeat == true)
     {
     dimmer(color, brighten);
     Serial.println("wiederholung");
     }
     break;
     }
 }
}
if (autoDim == true)
    {
    autodimm();
    }
if (busy == true)
    {
    fader();
    }

//analogWrite(dimPIN,ledPWM[0]);              //PWM Wert auf Ausgang schreiben
//analogWrite(redPIN,ledPWM[1]);
//analogWrite(greenPIN,ledPWM[2]);
//analogWrite(bluePIN,ledPWM[3]);
}

void dimmer(int col,boolean dir)
{
//    if (col == 4)
//    {
//      ledPWM[5] = 0;
//    }
//    else
//    {
    int temp = ledPWM[col];
    if (dir == true)
        {
        temp += step;
        if (temp > 255)
            {
            temp = 255;
            repeat = false;
            }
        }
    if (dir == false)
        {
        temp -= step;
        if (temp < 0)
            {
            temp = 0;
            repeat = false;
            }
        }
    ledPWM[col] = temp;
    analogWrite(pinPWM[col],ledPWM[col]);
//    }
}

void autodimm()
{
    int ldrValue = 0;
    int corrValue = 0;
    if (currentMillis - preAutoMillis >= ldrCLK)
        {
        preAutoMillis = currentMillis;
        if (autoDim == true)
            {
            ldrValue = analogRead(ldrPIN);
            ldrValue = map(ldrValue, 0, 1023, 0, 255);
            Serial.println(ldrValue);
            corrValue = pwmRANGE - ldrValue;
            cmdPWM[0] = corrValue;
            busy = true;
            fader();
//            Serial.println(ledPWM[0]);
//            if (ledPWM[0] > 255)
//            {
//                ledPWM[0] = 255;
//            }
//            analogWrite(pinPWM[0], ledPWM[0]);
            }
        }
}

void fader()
{
    int tempVAL = 0;

    for (int x = 0; x < 4; x++)
    {
    tempVAL = ledPWM[x] - cmdPWM[x];
    if (tempVAL != 0 && currentMillis - preFadeMillis >= fadeCLK)
        {
            if (tempVAL < 0)
            {
              for (tempVAL; tempVAL <= 0;)//; tempVAL += fadeIncrement)
              {
                currentMillis = millis();
                if (currentMillis - preFadeMillis >= fadeCLK)
                    {
                    ledPWM[x] += fadeIncrement;
                    if (ledPWM[x] >= cmdPWM[x])
                        {
                        ledPWM[x] = cmdPWM[x];
                        }
                    tempVAL += fadeIncrement;
                    preFadeMillis = currentMillis;
                    analogWrite(pinPWM[x], ledPWM[x]);
                    }
                else
                    {
                    break;
                    }
              }
            }
            else //if (tempVAL > 0)
            {
              for (tempVAL; tempVAL >= 0;)// tempVAL -= fadeIncrement)
              {
                currentMillis = millis();
                if    (currentMillis - preFadeMillis >= fadeCLK)
                    {
                    ledPWM[x] -= fadeIncrement;
                    if (ledPWM[x] <= cmdPWM[x])
                        {
                        ledPWM[x] = cmdPWM[x];
                        }
                    tempVAL -= fadeIncrement;
                    preFadeMillis = currentMillis;
                    analogWrite(pinPWM[x], ledPWM[x]);
                    }
                else
                {
                    break;
                }
            }
        }
    }
        if (currentMillis - preFadeMillis < fadeCLK)
    {
        break;
    }

    }
    if (ledPWM[0] == cmdPWM[0] && ledPWM[1] == cmdPWM[1] && ledPWM[2] == cmdPWM[2] && ledPWM[3] == cmdPWM[3])
    {
             busy = false;
             Serial.println("busy false");
    }

}

void nuller()
{
    color = 4;
    for (int i = 0; i<4; i++)
    {
        cmdPWM[i] = 0;
    }
}
void readFlash()
{
    //auf "gespeichert"-Bit im EEPROM schauen und bei Bedarf Werte auf cmdPWM laden
}
void writeFlash(boolean order)
{
    //wenn order == true ledPWM in EEPROM schreiben und "gespeichert"-Bit dort setzen;
    //wenn order == false "gespeichert"-Bit zurücksetzen;
}
