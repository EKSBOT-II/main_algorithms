#include <LiquidCrystal_I2C.h>
#include <Wire.h>

// OBJ & VAR
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); // Set the LCD I2C address

int button = 23;

int val = 1;
int prevVal = val;

int run = 1;
int prevRun = run;

void setup()
{
    lcd.begin(16, 2);
    Serial.begin(115200);

    pinMode(button, INPUT);

    lcd.clear();
    Serial.println("SETUP");
}

void loop()
{
    prevVal = val;
    val = digitalRead(button);
    if (val == 0 && val != prevVal)
    { //<!> why val==0?
        if (run <= 4)
        {
            prevRun = run;
            run++;
        }
        else
        {
            prevRun = run;
            run = 1;
        }
    }

    if (run != prevRun)
    {
        Serial.print(val);
        Serial.print("\t");
        Serial.println(run);
    }

    lcd.setCursor(0, 0);
    switch (val)
    {
    case 1:
        lcd.print("1");
        break;
    case 2:
        lcd.print("2");
        break;
    case 3:
        lcd.print("3");
        break;
    case 4:
        lcd.print("4");
        break;
    case 5:
        lcd.print("5");
        break;

    default:
        break;
    }
}