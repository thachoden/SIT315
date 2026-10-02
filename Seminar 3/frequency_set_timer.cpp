//variable for pin numbers
const byte LED_PIN = 13;
const byte METER_PIN = A4;

//Assumptions for the edge value for the system
const double MIN_FREQ = 0.25;   // Hz  (interrupt every 4 s)
const double MAX_FREQ = 10.0;   // Hz  (interrupt every 0.1 s)

// Configures Timer1 in CTC mode so that TIMER1_COMPA_vect fires timerFrequency times per second
void startTimer(double timerFrequency){
  const unsigned int prescalers[] = {1, 8, 64, 256, 1024};
  const byte csBits[] = {
    (1 << CS10),                 // /1
    (1 << CS11),                 // /8
    (1 << CS11) | (1 << CS10),   // /64
    (1 << CS12),                 // /256
    (1 << CS12) | (1 << CS10)    // /1024
  };

  // Pick the smallest prescaler whose compare value fits in the 16-bit OCR1A register
  byte i = 0;
  unsigned long ocr = 0;
  for (i = 0; i < 5; i++)
  {
    ocr = (unsigned long)(F_CPU / (prescalers[i] * timerFrequency) + 0.5) - 1;   // OCR = f_cpu / (N * f) - 1; +0.5 for rounding to nearest integer
    if (ocr <= 65535) break;
  }
  if (i == 5) { i = 4; ocr = 65535; }   // frequency too low: use the slowest possible setting

  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  OCR1A  = ocr;                          // compare value
  TCCR1B |= (1 << WGM12);                // CTC mode
  TCCR1B |= csBits[i];                   // chosen prescaler
  TIMSK1 |= (1 << OCIE1A);               // enable compare-match A interrupt
  interrupts();
}



int lastReading = -100;         // impossible value, so the first reading always starts the timer

void setup()
{
  pinMode(LED_PIN, OUTPUT);
  pinMode(METER_PIN, INPUT);

  Serial.begin(9600);
}

void loop()
{
  int reading = analogRead(METER_PIN);                 // 0..1023 from the potentiometer

  if (abs(reading - lastReading) > 10)                 // only reprogram the timer on a real change
  {
    lastReading = reading;
    double freq = MIN_FREQ + (reading / 1023.0) * (MAX_FREQ - MIN_FREQ);
    startTimer(freq);

    Serial.print("Pot = ");
    Serial.print(reading);
    Serial.print("  ->  timer frequency = ");
    Serial.print(freq);
    Serial.println(" Hz");
  }
}



ISR(TIMER1_COMPA_vect){
   digitalWrite(LED_PIN, digitalRead(LED_PIN) ^ 1);
}
