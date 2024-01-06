/*
    Created: -
    Changed: 11 Nov 2023, 14:55

    This is the main template program for testing purposes
    Still in development, as of 1 Nov 2023
    Communications built based on HARDWARE-SERIAL-(MASTER/SLAVE).ino, but is no longer used.
    Asynchronous tasks built based on ASYNC-(MASTER/SLAVE).ino

    Features:
    - Async Tasks (done)
    - Menu (done)

    01. Current menu works as follows:
        Upon pressing the push button, the PB pin will read a HIGH signal.
        The previous value of the button is stored in pbValPrev, and pbVal takes in the new value.
        Interrupt reads on RISING and the menu increments.
        Programs can be put in case statements representing the menu.
        There is no selection time, meaning when the button is pressed the next program runs immediately.
    02. Inputs are read and processed by task1, while all other programs are in the main loop.
    03. Number of menu can be modified by adding/removing cases.
        Modify the increment limiter in pbHighISR() accordingly.
        Default/starting selection is 0.



    >> PINS MAY BE IRRELEVANT <<
*/

/* =============== PINS =============== */
int PB = 36,
    LM = 23,

    IR1 = 25,
    IR2 = 33,
    IR3 = 32,
    IR4 = 35,
    IR5 = 34,

    LC1 = 14,
    LC2 = 12,
    RC1 = 26,
    RC2 = 27,

    RE = 4,
    R2 = 16,
    R1 = 17,
    L2 = 5,
    L1 = 18,
    LE = 19,

    M1 = 13,
    M2 = 15,

    LED = 2;
/* =============== PINS =============== */

/* =============== VARS & OBJ =============== */
// Task1 Object
TaskHandle_t Task1;

// PID Vars
int prevError, error,
    Kp = 25,
    Ki = 0,
    Kd = 0,
    pidOut;
// int sensorWeights[5] = {-4, -2, 1, 2, 4};

// COUNTERS
int pbVal, pbValPrev; // push buttons
long lastISR = 0;     // push button last ISR call
int selected = 0;     // current selection

// SENSOR & ACTUATOR VALUES
int id1, id2, id3, id4, id5;      // ir data
int posL = 0, posR = 0;           // encoder position data
int limit;                        // limit switch
int LcurrentSpeed, RcurrentSpeed; // motor speed recorder
/* =============== VARS & OBJ =============== */

/* =============== SEQUENCES =============== */
// Motor Test Sequence
int motorTestSequence[5][3] = {
    {0, 0, 1000},
    {100, 100, 1000},
    {-100, -100, 1000},
    {100, -100, 1000},
    {-100, 100, 1000},
};
int motorTestSequenceIdx = 0;
/* =============== SEQUENCES =============== */

/* =============== TASKS & ROUTINES =============== */
// Task 1 Core 1
void task1(void *parameter)
{
    for (;;)
    {
        // Sensor Reading
        limit = digitalRead(LM);

        id1 = digitalRead(IR1);
        id2 = digitalRead(IR2);
        id3 = digitalRead(IR3);
        id4 = digitalRead(IR4);
        id5 = digitalRead(IR5);

        // ln1 = digitalRead(LC1);
        // ln2 = digitalRead(LC2);
        // rn1 = digitalRead(RC1);
        // rn2 = digitalRead(RC2);

        // Error Calculation
        error = (id1 + id2) - (id4 + id5); // (+) = error to right, max 2
        pidOut = Kp * error + Ki * error + Kd * error;

        // LED On Button
        pbVal = digitalRead(PB);
        if (pbVal == HIGH)
        {
            digitalWrite(LED, HIGH);
        }
        else
        {
            digitalWrite(LED, LOW);
        }
    }
}

// Push Button ISR
void IRAM_ATTR pbHighISR()
{
    if ((millis() - lastISR) >= 200)
    {
        if (selected < 5)
        {
            selected++;
        }
        else
        {
            selected = 0;
        }
        lastISR = millis();

        // All index reset
        // motorTestSequenceIdx = 0;
    }
}

// Left Encoder ISR
void IRAM_ATTR encoderLISR()
{
    LC2 ? posL++ : posL--;
}
// Right Encoder ISR
void IRAM_ATTR encoderRISR()
{
    RC2 ? posR++ : posR--;
}
/* =============== TASKS & ROUTINES =============== */

/* =============== SETUP =============== */
void setup()
{
    Serial.begin(115200);

    pinMode(0, INPUT_PULLUP); // reset GPIO 0 to pullup for bootloader

    pinMode(PB, INPUT);
    pinMode(LM, INPUT);

    pinMode(IR1, INPUT);
    pinMode(IR2, INPUT);
    pinMode(IR3, INPUT);
    pinMode(IR4, INPUT);
    pinMode(IR5, INPUT);

    pinMode(LC1, INPUT);
    pinMode(LC2, INPUT);
    pinMode(RC1, INPUT);
    pinMode(RC2, INPUT);

    pinMode(LE, OUTPUT);
    pinMode(L1, OUTPUT);
    pinMode(L2, OUTPUT);
    pinMode(RE, OUTPUT);
    pinMode(R1, OUTPUT);
    pinMode(R2, OUTPUT);

    pinMode(M1, OUTPUT);
    pinMode(M2, OUTPUT);

    pinMode(LED, OUTPUT);

    digitalWrite(M1, HIGH);
    digitalWrite(M2, HIGH);

    attachInterrupt(PB, pbHighISR, RISING);

    attachInterrupt(LC1, encoderLISR, RISING);
    attachInterrupt(LC2, encoderLISR, RISING);
    attachInterrupt(RC1, encoderRISR, RISING);
    attachInterrupt(RC2, encoderRISR, RISING);

    xTaskCreatePinnedToCore(
        task1,
        "Task_1",
        1000,
        NULL,
        1,
        &Task1,
        1 // loop
    );
}
/* =============== SETUP =============== */

/* =============== MAIN =============== */
void loop()
{
    motorRun(0, 0);
    // Menu
    switch (selected)
    {
    case 1:
        Serial.print("[PR_1]: Sequence: ");
        Serial.println(motorTestSequenceIdx);

        programOne(motorTestSequence[motorTestSequenceIdx]);
        if (motorTestSequenceIdx < 4)
        {
            motorTestSequenceIdx++;
        }
        else
        {
            motorTestSequenceIdx = 0;
        }
        break;
    case 2:
        // Serial.println("[MAIN]: Program 2");
        programTwo();
        break;
    case 3:
        // Serial.println("[MAIN]: Program 3");
        programThree();
        break;
    case 4:
        // Serial.println("[MAIN]: Program 4");
        programFour();
        break;
    case 5:
        Serial.println("[MAIN]: Program 5");
        // programFive();
        break;
    default:
        Serial.println("[MAIN]: Program ZERO");
        break;
    }
}
/* =============== MAIN =============== */

/* =============== PROGRAMS =============== */
// Motor Test
void programOne(int param[3])
{
    Serial.println("[PR_1]: Running");
    motorRun(param[0], param[1]);
    delay(param[2]);
}

// Infrared & Encoder Reading
void programTwo()
{
    Serial.print("[PR_2]: ");
    Serial.print("\t");
    Serial.print(id1);
    Serial.print("\t");
    Serial.print(id2);
    Serial.print("\t");
    Serial.print(id3);
    Serial.print("\t");
    Serial.print(id4);
    Serial.print("\t");
    Serial.print(id5);
    Serial.print("\t");
    Serial.print(posL);
    Serial.print("\t");
    Serial.println(posR);
    delay(100);
}

// Relay & Limit Switch
void programThree()
{
    /*
        LOW = active, HIGH = deactive (normal)
        LOW = COM + NO, LED ON.
        Assume LOW = normal:
        on idle: LOW
        LOW = GND (0)
        HIGH = 3v3 (1)
        config :
            GND -> NO
            3v3 -> NC
            COM -> Motor1
    */
    if (limit)
    {
        Serial.print("[PR_3]: LOW - LOW");
        Serial.print("\t");
        Serial.println(limit);
        // Stop
        digitalWrite(M1, LOW);
        digitalWrite(M2, LOW);
    }
    else
    {
        Serial.print("[PR_3]: HIGH - LOW");
        Serial.print("\t");
        Serial.println(limit);
        // Up Movement
        digitalWrite(M1, HIGH);
        digitalWrite(M2, LOW);
    }
}

// Line PID
void programFour()
{
    motorRun((50 - pidOut), (50 + pidOut));
    Serial.print("[PR_4]: ");
    Serial.print(pidOut);
    Serial.print("\t");
    Serial.print(id1);
    Serial.print("\t");
    Serial.print(id2);
    Serial.print("\t");
    Serial.print(id3);
    Serial.print("\t");
    Serial.print(id4);
    Serial.print("\t");
    Serial.print(id5);
    Serial.print("\t");
    Serial.print(posL);
    Serial.print("\t");
    Serial.println(posR);
}

void programFive()
{
    /*
        Hardcoded for Mobile Robot
    */
    // Serial
}
/* =============== PROGRAMS =============== */

/* =============== FUNCS & UTILS =============== */
// ADC (4095 to 100) Mapping
int adc(int val)
{
    return map(val, 0, 4095, 0, 100);
    // return (val / 4095) * 1000;
}
// Speed Mapping
int speed(float val)
{
    return map(val, 0, 100, 0, 255);
}

// Motor Run
void motorRun(int Lnum, int Rnum)
{
    analogWrite(LE, speed(abs(Lnum)));
    LcurrentSpeed = Lnum;
    if (Lnum > 0)
    {
        digitalWrite(L1, LOW);
        digitalWrite(L2, HIGH);
    }
    else if (Lnum < 0)
    {
        digitalWrite(L1, HIGH);
        digitalWrite(L2, LOW);
    }
    else
    {
        digitalWrite(L1, HIGH);
        digitalWrite(L2, HIGH);
    }

    analogWrite(RE, speed(abs(Rnum)));
    RcurrentSpeed = Rnum;
    if (Rnum > 0)
    {
        digitalWrite(R1, LOW);
        digitalWrite(R2, HIGH);
    }
    else if (Rnum < 0)
    {
        digitalWrite(R1, HIGH);
        digitalWrite(R2, LOW);
    }
    else
    {
        digitalWrite(R1, HIGH);
        digitalWrite(R2, HIGH);
    }
}
/* =============== FUNCS & UTILS =============== */