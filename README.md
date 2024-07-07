## Automatic Watering System for Mo

# ToDo:
1. Design the state machine and control flow, build the software.(DONE)
2. Build and test the circuits.(DONE)


# Note from Aya on 2024-7-7: The script is completed, but not throughtly tested.

What does this script do:

    1. On activation: the system will halt until the next 9am 
    2. At the first 9am, the system will activate the pump for 200s 
        if the waterLevel is also at status:0 (LOW)
    3. After watering the plant, the system will wait for 3 days,
        using WaitUnitlNext9AM() and WaitFor20Hour().
    4. Finally, the WaitFor3Days() will end on the 4th day morning before 9AM
    5. Loop back to beginning and wail until the next 9AM and water the flower..

