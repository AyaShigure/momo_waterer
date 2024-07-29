#define WATER_TIME 10
#define OVERRIDE_FLAG false
#include <Arduino.h>
// CONNECTIONS:
// DS1302 CLK/SCLK --> 5
// DS1302 DAT/IO --> 4
// DS1302 RST/CE --> 2
// DS1302 VCC --> 3.3v - 5v
// DS1302 GND --> GND

// PUMP ACTIVATION --> 10
// RTC ERROR --> 11

#include <ThreeWire.h>  
#include <RtcDS1302.h>

/*
Note from Aya on 2024-7-6: Go create a state machine diagram first!!

Controll logic:
1.  Water the plant at 9 am every 3 days (DONE)
2.  Design the state machine (DONE)
*/

/*
Note from Aya on 2024-7-7: The script is completed, but not throughtly tested.

What does this script do:

    1. On activation: the system will halt until the next 9am 
    2. At the first 9am, the system will activate the pump for 200s 
        if the waterLevel is also at status:0 (LOW)
    3. After watering the plant, the system will wait for 3 days,
        using WaitUnitlNext9AM() and WaitFor20Hour().
    4. Finally, the WaitFor3Days() will end on the 4th day morning before 9AM
    5. Loop back to beginning and wail until the next 9AM and water the flower..

*/

/*
Note from Aya on 2024-7-22: Measuring noise problem is... problematic, need some sort of filter

*/

// RTCDS1302 module
ThreeWire myWire(4,5,2); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);
#define countof(a) (sizeof(a) / sizeof(a[0]))
const int PUMP_CTRL_PIN = 10;
const int RTC_ERROR_LED_PIN = 11;
const int MAX_WATER_LEVEL = 500;
bool resetTimeOverride = OVERRIDE_FLAG;  // This flag force reset the time to 08:59:00

void PrintWaterLevel()
{
    float waterLevel = analogRead(A0);
    Serial.print("Current water level: ");
    Serial.println(waterLevel);
}

void PrintDateTime(const RtcDateTime& dt)
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
    Serial.print("[");
    Serial.print(datestring);
    Serial.print("] ");
}

void RtcValidCheck()
{
    /*
        This block checks if RTC is valid, if not, shut pump pin down and go into infinite loop.
        Also active the RTC_ERROR_LED.
    */
    RtcDateTime now = Rtc.GetDateTime();
    if (!now.IsValid())
    {
        // Common Causes:
        //    1) the battery on the device is low or even missing and the power line was disconnected
        digitalWrite(PUMP_CTRL_PIN, LOW);
        digitalWrite(RTC_ERROR_LED_PIN, HIGH);
        while(1){
            Serial.println("======================================");
            Serial.println("RTC lost confidence in the DateTime!");
            Serial.println("Check the battery on the device.");
            Serial.println("System will not automatically reset!");
            Serial.println("======================================");
            delay(5000);
        }
    }
    else
    {   
        Serial.println("RTC status: OK");
    }
    
}

// Trick: wait for 20h, then wait till 9am, this ensures the delay will not drift over days and eventually skip 9am~10am
void WaitFor20Hour() 
{
    /*
        This block will halt the program for 22h
    */
    RtcDateTime now = Rtc.GetDateTime(); // Get current time
    int hourBeforeDelay = now.Hour();  // Get hour
    int hourNumberSinceStart = 1;
    int delayOneSecond = 1000;

    while(hourNumberSinceStart != 20){  
        RtcValidCheck();
        now = Rtc.GetDateTime();
        if((hourBeforeDelay - now.Hour()) != 0) // Check if hour has changed
        {
            // Hour number has changed
            hourNumberSinceStart += 1;
        }

        now = Rtc.GetDateTime(); // Update time
        hourBeforeDelay = now.Hour();  // Updat hour
        delay(2 * delayOneSecond); // Delay 10s
        PrintWaterLevel();

        PrintDateTime(now);
        Serial.println("Waiting until 20 hours to past");
        Serial.print("Current hourNumberSinceStart = ");
        Serial.println(hourNumberSinceStart);
    }
}

void WaitUntilNext9AM()
{
    /*
        This block will halt the program until next 9 am.
        If started at 9am ~ 10am, this block will be skiped.
    */
    RtcDateTime now = Rtc.GetDateTime(); // Get current time
    int Hour = now.Hour();
    int delayOneSecond = 1000;

    while(Hour != 9){ 
        RtcValidCheck();
        now = Rtc.GetDateTime(); // Get current time
        Hour = now.Hour();  // Get hour

        delay(2 * delayOneSecond); // Delay 10s
        PrintDateTime(now);
        PrintWaterLevel();
        Serial.print("Waiting until next 9 am... ");
    }
}

int CheckWaterLevel(){
    int waterLevelStatus;
    float targetWaterLevel = MAX_WATER_LEVEL; // Target water level is 70% full
    float currentWaterLevel = analogRead(A0); // Water level rawinput

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
    Serial.print("Current water level(debug): ");
    Serial.println(currentWaterLevel);
    Serial.print("Current water status(debug): ");
    Serial.println(waterLevelStatus);
    // return waterLevelStatus;
    return 0; // water level override -> always ok.
}

void WaterThePlant()
{
    /*
        This block will active the pump if below conditions are met:
        1.  Activat time is smaller than maxActivationTime.
        2.  CheckWaterLevel() returns status "0" ("LOW")
    */

    int maxActivationTime = 10; // 20s
    int delayOneSecond = 1000;
    int timer = 0;

    int waterLevelStatus = CheckWaterLevel();
    // Water the plant until water level sensor returns OK level.
    Serial.print("Water level: ");
    Serial.print(waterLevelStatus);
    Serial.println();
    
    while(timer < maxActivationTime && waterLevelStatus == 0)
    {
        RtcValidCheck();
        digitalWrite(PUMP_CTRL_PIN, HIGH); // Active the pump


        waterLevelStatus = CheckWaterLevel(); // Update water level
        Serial.print("Water level: ");
        Serial.print(waterLevelStatus);
        Serial.println();

        delay(delayOneSecond);
        timer += 1; // Update timer
        Serial.print("Watering the plant... Current time: ");
        Serial.print(timer);
        Serial.println();
    }
    
    digitalWrite(PUMP_CTRL_PIN, LOW); // Stop the pump
    Serial.println("Pump is deactivated");
    Serial.println("Watering is completed");
    Serial.println();
}

void WaitFor3Days()
{
    /*
        This block will halt the program until the first 9am, then halt the program for additional 3 days,
        utill 9am ~ 10am on the third day 
    */

    int dayNumber = 0;
    int waitDays = 2; // 2024-7-30, revised form 3 to 2
    for(int i = 0; i < waitDays; i++){
        WaitUntilNext9AM();
        WaitFor20Hour();
        dayNumber += 1;
    }

}


void setup () 
{
    // system initializaitons
    pinMode(PUMP_CTRL_PIN, OUTPUT);
    pinMode(RTC_ERROR_LED_PIN, OUTPUT);

    digitalWrite(PUMP_CTRL_PIN, LOW);
    digitalWrite(RTC_ERROR_LED_PIN, LOW);

    Serial.begin(9600);

    Serial.print("compiled: ");
    Serial.print(__DATE__);
    Serial.println(__TIME__);

    Rtc.Begin();

    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    PrintDateTime(compiled);
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
        Serial.print("Updating the time to : ");
        PrintDateTime(compiled);
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



    if(resetTimeOverride == true)
    {
        char overrideDate[] = "Jul 7 2024";
        char overrideHMS[] = "08:59:50";
        RtcDateTime overrideTime = RtcDateTime(overrideDate, overrideHMS);

        // Serial.println(overrideTime);
        Serial.println();
        Serial.println("RTC time override!");
        Serial.print("Updating the time to override time: ");
        PrintDateTime(overrideTime);
        Rtc.SetDateTime(overrideTime);
    }

}


void loop () 
{
    Serial.println("System is activating..");

    while(1)
    {
        WaitUntilNext9AM();
        WaterThePlant();
        WaitFor3Days(); // Halt for 3x24=72 hours

        // WaterThePlant();
        // delay(10000);
    }

}


