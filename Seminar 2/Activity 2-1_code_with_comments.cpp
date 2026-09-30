// Pin assignments: push button on digital pin 2, LED on digital pin 13
const uint8_t BTN_PIN = 2;
const uint8_t LED_PIN = 13;

// Global state variables: button reading from the previous loop and the current LED state (both start LOW/off)
uint8_t buttonPrevState = LOW;
uint8_t ledState = LOW;

// setup() runs once when the board is powered on or reset
void setup()
{
  // Configure the button pin as a digital input with the internal pull-up resistor enabled
  pinMode(BTN_PIN, INPUT_PULLUP);
  // Configure the LED pin as a digital output
  pinMode(LED_PIN, OUTPUT);
  // Start serial communication at 9600 baud to monitor the states in the Serial Monitor
  Serial.begin(9600);
}

void loop()
{
  // SENSE: poll the button and read its current state (HIGH = pressed, LOW = released)
  uint8_t buttonState = digitalRead(BTN_PIN);
  
  // Print the current button state, previous button state and LED state on one line (e.g. "100")
  Serial.print(buttonState);
  Serial.print(buttonPrevState);
  Serial.print(ledState);
  Serial.println("");
  
  
  // THINK/ACT: if the button state has changed since the last check (pressed or released), toggle the LED
  if(buttonState != buttonPrevState)
  {
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
  }
  
  buttonPrevState = buttonState;
    
  // Wait 500 ms before the next loop iteration
  delay(500);
