const uint8_t PIR_PIN = 2;
const uint8_t TILT_PIN = 4;
const uint8_t ALARM_LED = 13;

const unsigned long ALARM_DURATION_MS = 5000;

// Shared between the ISR and loop(): volatile so loop() always reads the latest value from RAM
volatile bool motionFlag = false;        // set by the ISR when motion is detected
volatile unsigned long motionTime = 0;   // time of the motion event, recorded by the ISR

bool motionEvent = false;
unsigned long eventTime = 0;
bool armed = false;
bool prevArmed = false;
bool alarmActive = false;
unsigned long alarmStart = 0;

void logEvent(const char *msg)
{
  Serial.print("[");
  Serial.print(millis());
  Serial.print(" ms] ");
  Serial.println(msg);
}

// ISR: runs immediately when the PIR output on D2 goes LOW -> HIGH (motion starts).
// Kept short and non-blocking: only sets a flag and records the time.
// No delay() or Serial here - the event is handled later in loop().
void motionISR()
{
  motionFlag = true;
  motionTime = millis();
}

void readSensors()
{
  armed = (digitalRead(TILT_PIN) == LOW);

  // Critical section: pause interrupts while copying and clearing the shared variables,
  // so the ISR cannot change them halfway (motionTime is 4 bytes on an 8-bit CPU).
  // A motion event during this block stays pending and the ISR runs right after interrupts().
  noInterrupts();
  motionEvent = motionFlag;
  eventTime = motionTime;
  motionFlag = false;   // clear the flag so each interrupt event is handled only once
  interrupts();
}

void processLogic()
{
  if (armed != prevArmed)
  {
    logEvent(armed ? "Unit upright -> system ARMED" : "Unit tilted -> system DISARMED");
    if (!armed && alarmActive)
    {
      alarmActive = false;
      logEvent("Alarm cancelled by disarm -> LED OFF");
    }
    prevArmed = armed;
  }

  if (motionEvent)
  {
    if (armed)
    {
      alarmStart = eventTime;
      if (!alarmActive)
      {
        alarmActive = true;
        logEvent("Motion detected (interrupt) + ARMED -> ALARM ON");
      }
      else
      {
        logEvent("Motion detected again -> alarm timer restarted");
      }
    }
    else
    {
      logEvent("Motion detected (interrupt) but DISARMED -> ignored");
    }
  }

  if (alarmActive && (millis() - alarmStart >= ALARM_DURATION_MS))
  {
    alarmActive = false;
    logEvent("5 s passed with no new motion -> ALARM OFF");
  }
}

void updateOutputs()
{
  digitalWrite(ALARM_LED, alarmActive ? HIGH : LOW);
}

void setup()
{
  pinMode(PIR_PIN, INPUT);
  pinMode(TILT_PIN, INPUT);
  pinMode(ALARM_LED, OUTPUT);
  Serial.begin(9600);

  // Attach external interrupt INT0 (pin D2) to motionISR, triggered on the RISING edge
  attachInterrupt(digitalPinToInterrupt(PIR_PIN), motionISR, RISING);

  armed = prevArmed = (digitalRead(TILT_PIN) == LOW);
  logEvent("System started. Rule: ALARM = MOTION (interrupt) AND ARMED (tilt sensor upright)");
  logEvent(armed ? "Initial state: ARMED" : "Initial state: DISARMED");
}

void loop()
{
  readSensors();
  processLogic();
  updateOutputs();
}