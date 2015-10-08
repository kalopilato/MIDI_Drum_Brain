#include <Wire.h>
#include <LiquidCrystal_I2C_DFR.h>

LiquidCrystal_I2C_DFR lcd(0x27);

/* Drum Pad Input Pins */
int PIN_COUNT = 6;              // Defines number of pins in use.  Pins are scanned in sequence from A0 to A(PIN_COUNT - 1)

/* Tunable Values */
byte ANALOG_SAMPLES = 8;        // Defines how many consecutive reads to take from one strike
byte MAX_STRIKE_VOLTAGE = 150;  // Defines the maximum voltage limit to be read from one strike (higher voltages are clipped to this value)
byte MIN_VEL = 5;               // Defines the minimum output MIDI velocity
byte DELAY = 3;                // Defines how many ms to wait after detecting a strike before polling for another
byte NOTE_HOLD = 3;             // Defines how long the midi note will hold for
byte PEDAL_VELOCITY = 1;       // Defines velocity of pedal changes

/* MIDI Command Values */
int noteON = 144;               // 10010000 in binary, note on command
int noteOFF = 128;              // 10000000 in binary, note off command

/* MIDI Note Values */
int PEDAL_OFF = 59;
int PEDAL_ON = 48;
int KICK = 36;
int SNARE = 38;
int TOM1 = 71;
int TOM2 = 69;
int HAT_OPEN = 54;
int HAT_CLOSED = 49;
int CRASH_1 = 77;
int SNARE_RIM = 37;
int SNARE_SIDESTICK = 42;

/* Hat Pedal */
int hatPedal = 4;               // Defines which pin hat pedal is connected to
int pedalState = HIGH;          // Defines initial pedal state

/* Menu Controls */
int editButton = 7;
int backButton = 9;
int forwardButton = 8;
int enterButton = 10;

int backState = HIGH;
int forwardState = HIGH;
int editState = HIGH;
int enterState = HIGH;
int buttonState[] = {backState, forwardState, editState, enterState};
byte backPos = 0;
byte forwardPos = 1;
byte editPos = 2;
byte enterPos = 3;

void setup(){

  // Start and Initialise the LCD
  lcd.begin(16,2);
  lcd.backlight();
  displayDefault();

  Serial1.begin(31250);         // MIDI Serial is run over TX Pin so uses 'Serial1' for Leonardo.
                                // For other Arduino variants change this back to 'Serial'
  pinMode(hatPedal, INPUT_PULLUP);
  pinMode(editButton, INPUT_PULLUP);
  pinMode(enterButton, INPUT_PULLUP);
  pinMode(backButton, INPUT_PULLUP);
  pinMode(forwardButton, INPUT_PULLUP);
}

void loop(){

  // Iterate through analog pads reading voltages and sending MIDI output
  for(int i = 0; i < PIN_COUNT; i++){

    // Check if pedal state has changed
    if(digitalRead(hatPedal) != pedalState){
      if(pedalState == HIGH) MIDIoutput(PEDAL_ON, PEDAL_VELOCITY);
      else MIDIoutput(PEDAL_OFF, PEDAL_VELOCITY);
      pedalState = !pedalState;
    }

    if(analogRead(i) > 1){

      byte maxVal = 0;

      for(int read = 0; read < ANALOG_SAMPLES; read++){
        byte val = analogRead(i);
        if(val > maxVal) maxVal = val;
      }

      // Calculate note velocity
      if(maxVal > MAX_STRIKE_VOLTAGE) maxVal = MAX_STRIKE_VOLTAGE;
      byte velocity = (float) maxVal / MAX_STRIKE_VOLTAGE * 127;
      if(velocity < MIN_VEL) velocity = MIN_VEL;  // Limit minimum velocity value to MIN_VEL (defined in class)*/

      // Output MIDI
      int ccNote = 0;
      if(i == 0) ccNote = KICK;
      if(i == 1) ccNote = SNARE;
      if(i == 2) ccNote = TOM1;
      if(i == 3) ccNote = TOM2;
      if(i == 4) ccNote = getHat();
      if(i == 5) ccNote = CRASH_1;
      MIDIoutput(ccNote, velocity);
    }
  }
  delay(DELAY);
}

/* Write a single on/off signal to MIDI output with velocity */
void MIDIoutput(int MIDInote, int MIDIvelocity){
  Serial1.write(noteON);
  Serial1.write(MIDInote);
  Serial1.write(MIDIvelocity);
  delay(NOTE_HOLD);
  Serial1.write(noteOFF);
  Serial1.write(MIDInote);
  Serial1.write(MIDIvelocity);
}

/* Check if Hat should be open or closed and return the appropriate MIDI note */
int getHat(){
  if(pedalState == LOW) return HAT_CLOSED;
  else return HAT_OPEN;
}

/* Checks if a button has been pressed */
boolean buttonPressed(int button, byte statePos, boolean debounce){
  int state1 = digitalRead(button);
  if(debounce){
    delay(5);
    int state2 = digitalRead(button);
    if(state1 != state2) return false;
  }
  boolean pressed = false;
  int lastState = buttonState[statePos];
  if(state1 != lastState){
    if(state1 == LOW) pressed = true;
    buttonState[statePos] = !lastState;
  }
  return pressed;
}

void displayDefault(){
  lcd.print("MIDI Drum Brain");
  lcd.setCursor(0, 1);
  lcd.print("v1.0");
}
