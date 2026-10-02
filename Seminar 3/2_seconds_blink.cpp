const byte LED_PIN = 13;
const byte METER_PIN = A4;

void startTimer();

void setup()
{
  pinMode(LED_PIN, OUTPUT);
  pinMode(METER_PIN, INPUT);

  Serial.begin(9600);

  startTimer();
}

void loop()
{
}

void startTimer(){
  noInterrupts();

  TCCR1A = 0;                            // clear Timer1 control registers
  TCCR1B = 0;
  TCNT1  = 0;                            // reset the counter

  OCR1A = 15624;                         // 16 MHz / 1024 = 15625 ticks/s
  TCCR1B |= (1 << WGM12);                // CTC mode: counter resets when it reaches OCR1A
  TCCR1B |= (1 << CS12) | (1 << CS10);   // prescaler 1024
  TIMSK1 |= (1 << OCIE1A);               // enable the Timer1 compare-match A interrupt

  interrupts();
}

ISR(TIMER1_COMPA_vect){
   digitalWrite(LED_PIN, digitalRead(LED_PIN) ^ 1);
}
