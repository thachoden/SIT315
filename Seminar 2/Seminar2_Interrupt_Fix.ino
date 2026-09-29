const uint8_t BTN_PIN = 2;    // Button on pin 2 = external interrupt INT0
const uint8_t LED_PIN = 13;   // LED output pin

// Shared between the ISR and loop(), so they must be volatile
volatile uint8_t ledState = LOW;
volatile bool buttonEvent = false;           // flag set by the ISR
volatile unsigned long lastInterruptTime = 0;

void setup()
{
  pinMode(BTN_PIN, INPUT);        // external pull-down resistor is used
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(9600);
  // Call buttonISR() on every change (press and release) of pin 2
  attachInterrupt(digitalPinToInterrupt(BTN_PIN), buttonISR, CHANGE);
}

// ISR: runs immediately when the button changes. Kept short, no delay()
void buttonISR()
{
  unsigned long now = millis();
  if (now - lastInterruptTime < 50) return;  // ignore switch bounce (<50 ms)
  lastInterruptTime = now;

  ledState = !ledState;              // ACT immediately
  digitalWrite(LED_PIN, ledState);
  buttonEvent = true;                // tell loop() something happened
}

void loop()
{
  // Handle the event outside the ISR (Serial is too slow for an ISR)
  if (buttonEvent)
  {
    noInterrupts();                  // read shared variables safely
    buttonEvent = false;
    uint8_t led = ledState;
    interrupts();

    Serial.print("Button changed -> LED = ");
    Serial.println(led);
  }

  delay(500);  // other work: no longer affects the button response
}
