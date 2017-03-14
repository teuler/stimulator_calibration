// ---------------------------------------------
#define       startCh    "#"
#define       endCh      ";"
#define       sepCh      ","

#define       pin_AI0    A0
#define       pin_AI1    A1
#define       pin_LED    13
#define       rate_us    5000

bool          toggleLED;
unsigned long t0_us, t1_us;
int           val_AI0, val_AI1;

// ---------------------------------------------
void setup() {
  Serial.begin(115200);

  toggleLED = false;
  t0_us     = micros();
  
  // Change the analog input reference
  // INTERNAL: an built-in reference, equal to 1.1 volts on the ATmega168 or ATmega328 
  // and 2.56 volts on the ATmega8 (not available on the Arduino Mega) 
  //
  analogReference(INTERNAL);
}


// ---------------------------------------------
void loop() {
  do {
    t1_us = micros();
  }
  while ((t1_us -t0_us) < rate_us);

  val_AI0 = analogRead(pin_AI0);
  val_AI1 = analogRead(pin_AI1);

  Serial.print(startCh);
  Serial.print(millis(), DEC);
  Serial.print(sepCh);  
  Serial.print(t1_us -t0_us, DEC);
  Serial.print(sepCh);  
  Serial.print(val_AI0, DEC);
  Serial.print(sepCh);  
  Serial.print(val_AI1, DEC);
  Serial.println(endCh);
  t0_us = t1_us;
  
  digitalWrite(pin_LED, toggleLED);
  toggleLED != toggleLED;
}



// ---------------------------------------------
