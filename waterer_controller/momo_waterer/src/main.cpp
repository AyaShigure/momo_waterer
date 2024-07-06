#include <Arduino.h>
// CONNECTIONS:
// DS1302 CLK/SCLK --> 5
// DS1302 DAT/IO --> 4
// DS1302 RST/CE --> 2
// DS1302 VCC --> 3.3v - 5v
// DS1302 GND --> GND
#include <ThreeWire.h>  
#include <RtcDS1302.h>


/*
Note by Aya at 2024-7-6: Go create a state machine diagram first!!

Controll logic:
1.  Water the plant at 9 am every 3 days
2.  Design the state machine
*/


// RTCDS1302 module
ThreeWire myWire(4,5,2); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);
#define countof(a) (sizeof(a) / sizeof(a[0]))

void printDateTime(const RtcDateTime& dt)
{
    char datestring[20];

    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
            dt.Month(),
            dt.Day(),
            dt.Year(),
            dt.Hour(),
            dt.Minute(),
            dt.Second() );
    Serial.print(datestring);
}


// Global variables
const int waterThePlantEveryHowManyDays = 3;
int todayIsTheDayNumberWhat;

boolean startTimeIsSet = false;
int StartTimeInitializations(const RtcDateTime& now, boolean startTimeIsSet)
{
    if(startTimeIsSet == true)
    {
        Serial.println("Start date is (re)set");
        return 0;
    }



}

void UpdateClockUntilNextWatering(const RtcDateTime& now)
{
    
}


void WaterTheFlowerToday(const RtcDateTime& now){

}


int CheckWaterLevel(){
    int waterLevelStatus;
    float targetWaterLevel = 0.6; // Target water level is 70% full
    float currentWaterLevel = (float)analogRead(A0) / 255.0 ; // Water level normalized to 1

    if(currentWaterLevel < targetWaterLevel)
    {
        waterLevelStatus = 0; // Water level: LOW
    }
    else if (currentWaterLevel > targetWaterLevel && currentWaterLevel < targetWaterLevel+0.1)
    {
        waterLevelStatus = 1; // Water level: OK
    }
    else{
        waterLevelStatus = 2; // Water level: OVERFILLED
    }    
    
    return waterLevelStatus;
}











void setup () 
{
    Serial.begin(9600);

    Serial.print("compiled: ");
    Serial.print(__DATE__);
    Serial.println(__TIME__);

    Rtc.Begin();

    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    printDateTime(compiled);
    Serial.println();

    if (!Rtc.IsDateTimeValid()) 
    {
        // Common Causes:
        //    1) first time you ran and the device wasn't running yet
        //    2) the battery on the device is low or even missing

        Serial.println("RTC lost confidence in the DateTime!");
        Rtc.SetDateTime(compiled);
    }

    if (Rtc.GetIsWriteProtected())
    {
        Serial.println("RTC was write protected, enabling writing now");
        Rtc.SetIsWriteProtected(false);
    }

    if (!Rtc.GetIsRunning())
    {
        Serial.println("RTC was not actively running, starting now");
        Rtc.SetIsRunning(true);
    }

    RtcDateTime now = Rtc.GetDateTime();
    if (now < compiled) 
    {
        Serial.println("RTC is older than compile time!  (Updating DateTime)");
        Rtc.SetDateTime(compiled);
    }
    else if (now > compiled) 
    {
        Serial.println("RTC is newer than compile time. (this is expected)");
    }
    else if (now == compiled) 
    {
        Serial.println("RTC is the same as compile time! (not expected but all is fine)");
    }
}





void loop () 
{
    RtcDateTime now = Rtc.GetDateTime();

//    printDateTime(now);
    // Serial.print(now.Hour());
    // Serial.print(now.Minute());
    // Serial.print(now.Second());
    Serial.print("Week number: ");
    // Serial.print(now.DayOfWeek());
    Serial.print(now.DayOfWeek());
    Serial.print(" ");

    Serial.println();

    if (!now.IsValid())
    {
        // Common Causes:
        //    1) the battery on the device is low or even missing and the power line was disconnected
        Serial.println("RTC lost confidence in the DateTime!");
    }

    delay(1000); // five seconds
}

