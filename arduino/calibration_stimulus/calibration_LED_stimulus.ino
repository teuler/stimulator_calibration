/* ------------------------------------------------------------------------------------
 Flash, BGW, 3 intensities

 Adjusting timers 
 (Only in case it is necscessary to improve the timing):   
 Pins 9 and 10: controlled by timer 1
 Setting 	Divisor 	Frequency
 0x01 	 	1 	 	31250
 0x02 	 	8 	 	3906.25
 0x03  		64 	 	488.28125  <-- default
 0x04  		256 	 	122.0703125
 0x05 	 	1024 	 	30.517578125

 TCCR1B = TCCR1B & 0b11111000 | <setting>;

 ------------------------------------------------------------------------------------*/
#define      showMarker      true
#define      noMarker        false
#define      isDuringRetrace LOW
#define      isDuringScan    HIGH

// If used in setup, set to 0; if tested outside w/o setup, set to 1
//
#define      ignoreBlank     0
 
// -----------------------------------------------------------------------------------
// User definitions
//
const  int   refreshSFreq    = 80;

// ##################################################################################
const  byte  minB            = 0;   // minimal and ...
const  byte  maxB            = 255;  // .. maximal blue LED intensity
const  byte  bkgB            = (maxB-minB)/2 +minB;

const  byte  minG            = 0;    // minimal and ...
const  byte  maxG            = 255;  // .. maximal green LED intensity
const  byte  bkgG            = (maxG-minG)/2 +minG;
// ##################################################################################

// Color sine wave stimuli
// -----------------------
const  byte  nRuns           = 6;    // number of complete runs
const  byte  nCyclPerCond    = 5;
const  float factSilenceB    = -0.1106;
const  float factWhiteB      =  0.8894;
const  float factIsoB        =  1.1106;

const  byte  levB_0          = (maxB+minB)/2;  // blue, green intensity before iso-wave ...
const  byte  levG_0          = (maxG+minG)/2;  // .. and after last (blue) wave
const  byte  levB_1          = (maxB+minB)/2;  // blue, green intensity before white-wave
const  byte  levG_1          = (maxG+minG)/2;
const  byte  levB_2          = (maxB+minB)/2;  // blue, green intensity before green-wave
const  byte  levG_2          = (maxG+minG)/2;
const  byte  levB_3          = (maxB+minB)/2;  // blue, green intensity before blue-wave
const  byte  levG_3          = (maxG+minG)/2;
const  byte  levB_4          = (maxB+2*minB)/3;// blue, green intensity before blue-wave
const  byte  levG_4          = (maxG+2*minG)/3;


// Other parameters
//
const  byte  lenMarker_frs   = 4;      // at 80 Hz -> 4*12.5 ms = 50 ms
const  long  lenMarker_us    = 50000;  // 50 ms
// ##################################################################################
const  long  lenPreWave_ms   = 15000;   // 20000 = 20 secs
// ##################################################################################

const  byte  fMarkerOrTest   = 1;    // 1=normal marker, 0=test pulse at refresh rate

/* ------------------------------------------------------------------------------------
   Internal definitions (do not change!)
*/
const  int   pBlueLED        = 9;
const  int   pGreenLED       = 10;
const  int   pSyncOut        = 12;
const  int   pBlank          = 2;
const  int   fsync           = 3;

const  byte  ISO             = 1;
const  byte  WHITE           = 2;
const  byte  GREEN           = 3;
const  byte  BLUE            = 4;
const  byte  SILENCE         = 5;

const  float Pi              = 3.14159265;

// Global variables
//
/*
int          sinArrB[refreshSFreq]; 
int          sinArrG[refreshSFreq]; 
int          sinArrG_silencing[refreshSFreq];   
int          sinArrG_white[refreshSFreq];
int          sinArrG_iso[refreshSFreq];
*/

unsigned int dt_us, dtMarker_ms;
int          stgreen, stblue;
int          iRun, on;
char         sCmd;

// -----------------------------------------------------------------------------------
void setup()
{
  int    iF;
  float  val;
 
  // Initialize the serial communication with PC
  //
  Serial.begin(9600);
  
  // Initialize pins
  //
  pinMode(pGreenLED, OUTPUT);
  pinMode(pBlueLED, OUTPUT);  
  pinMode(pSyncOut, OUTPUT);   
  
  // Set frequency of PWM 
  //
  TCCR1B = TCCR1B & 0b11111000 | 0x01; 
  digitalWrite(pSyncOut, 0);
  
  /*
  // Generate sine tables for defined refresh rate and the respective intensities
  // for green and blue
  //
  for(iF=0; iF<refreshSFreq; iF+=1) {
    val         = (1+ sin(iF*2*Pi/refreshSFreq))/2 *255;
    sinArrG[iF] = map(val, 0, 255, minG, maxG);
    sinArrB[iF] = map(val, 0, 255, minB, maxB);   
    val         = (1 +factSilenceB*sin(iF*2*Pi/refreshSFreq))/2 *255;
    sinArrG_silencing[iF] = map(val, 0, 255, minB, maxB); 
    val         = (1 +factWhiteB*sin(iF*2*Pi/refreshSFreq))/2 *255;
    sinArrG_white[iF]= map(val, 0, 255, minB, maxB);   
    val         = (1 +factIsoB*sin(iF*2*Pi/refreshSFreq))/2 *255;
    sinArrG_iso[iF]= map(val, 0, 255, minB, maxB);  
  }
  */
  
  // Some precalculations ...
  //
  dt_us       =(int)(1000000/(long)refreshSFreq);
//dt_us       = 12800;
  dtMarker_ms = lenMarker_frs*dt_us /1000;
  // ...
}

/* -----------------------------------------------------------------------------------
 Main loop
 ------------------------------------------------------------------------------------*/
void loop() 
{
  // Initialize some global variables
  //
  stgreen = 0;
  stblue  = 0;

  while(true) {  
    // Update LEDs
    //
    setLEDs_onBlankState(stgreen, stblue);

    // Check if command received from PC
    //
    if(Serial.available()) {
      sCmd = Serial.read();

      // Execute command
      //  
      switch (sCmd) {
        case '0':
          // Both LEDs off
          //
          stgreen = 0;
          stblue  = 0;  
          Serial.println("-> both LEDs off");                                
          break;
        
        case '1':
          // green LED to minimum
          //
          stgreen = minG;
          Serial.print("-> green LED to ");                                        
          Serial.println(minG, DEC);                                                
          break;

        case '2':
          // green LED to maximum
          //
          stgreen = maxG;     
          Serial.print("-> green LED to ");                                        
          Serial.println(maxG, DEC);                                                
          break;

        case '3':
          // blue LED to minimum
          //
          stblue  = minB;     
          Serial.print("-> blue LED to ");                                        
          Serial.println(minB, DEC);                                                
          break;

        case '4':
          // blue LED to maximum
          //
          stblue  = maxB;
          Serial.print("-> blue LED to ");                                        
          Serial.println(maxB, DEC);                                                
          break;
          
        case 's': 
          // Run sinewave stimulus ...
          //
          Serial.print("DEACTIVATED");
          /*
          beginStimulus();
          wait_ms(levG_0, levB_0, lenPreWave_ms, noMarker);
          for(iRun=0; iRun<nRuns; iRun+=1) {
            playWave_wMarker(0, long(refreshSFreq)/2, ISO); 
          //wait_ms(levG_1, levB_1, 1000);
            playWave_wMarker(0, 0, WHITE); 
          //wait_ms(levG_3, levB_3, 1000);
            playWave_wMarker(0, 0, BLUE);
          //wait_ms(levG_2, levB_2, 1000);
            playWave_wMarker(0, 0, GREEN); 
          //wait_ms(levG_3, levB_3, 1000);
            playWave_wMarker(0, 0, SILENCE); 
          }
          wait_ms(levG_0, levB_0, 1000, noMarker);        
          endStimulus();
          */
          break;
          
       case '5':
          // Alternating steps, 2 conditions [blue, green]
          //
          beginStimulus();          
          playSteps_wMarker(7, 2.5); 
          endStimulus();
          break;

// ##################################################################################
        case '6':
          // CC, White flashes - min to max - 15 sec adaptation - with min background level
          //
          beginStimulus();
          myStimulus6();
          endStimulus();
          break;

        case '7':
          // CC, GBW stimulus - min to max - 15 sec adaptation - with min background level
          //
          beginStimulus();
          myStimulus7();
          endStimulus();
          break;
        
        case '8':
          // CC, White flashes - 0 to max - No adaptation - No background level
          //
          beginStimulus();
          myStimulus8();
          endStimulus();
          break;
          
        case '9':
          // CC, Dark flashes - max to 0 - No adaptation - Background = max white light
          //
          beginStimulus();
          myStimulus9();
          endStimulus();
          break;
          
          case 'a':
          // CC, White(max) and Dark(0) flashes - No adaptation - Background = max/2
          //
          beginStimulus();
          myStimulusa();
          endStimulus();
          break;
          
          case 'c':
          // CC, W, G, B(max) and Dark, G, B(0) flashes - No adaptation - Background = max/2
          //
          beginStimulus();
          myStimulusc();
          endStimulus();
          break;
          
           case 'd':
          // CC, G, B, Dark flashes - No adaptation - Background = max white light
          //
          beginStimulus();
          myStimulusd();
          endStimulus();
          break;
          
          
          
  
        // Calibration
        case 'g':
          // G calibration
          //
          beginStimulus();
          myStimulusRamp(1,0);
          endStimulus();
          break;

        case 'b':
          // B calibration
          //
          beginStimulus();
          myStimulusRamp(0,1);
          endStimulus();
          break;

        case 'w':
          // W calibration
          //
          beginStimulus();
          myStimulusRamp(1,1);
          endStimulus();
          break;

          
        default :
          Serial.println("Unknown command. ");
          Serial.println("The following commands are valid:");        
          Serial.println("s - sine wave [iso, white, B, G, silence] stimulus"); 
          Serial.println("0 - both LEDs off");                
          Serial.println("1 - green LED to minimum");                        
          Serial.println("2 - green LED to maximum");                        
          Serial.println("3 - blue LED to minimum");                        
          Serial.println("4 - blue LED to maximum");       
          Serial.println("5 - RK, alternating step stimulus,2 conditions blue green");  
          
          Serial.println("6 - CC, White flashes - min to max - 15 sec adaptation - with min background level");   
          Serial.println("7 - CC, GBW stimulus - min to max - 15 sec adaptation - with min background level ");   
          Serial.println("8 - CC, White flashes - 0 to max - No adaptation - No background level");   
          Serial.println("9 - CC, Dark flashes - max to 0 - No adaptation - Background = max white light");   
          Serial.println("a - CC, White(max) and Dark(0) flashes - No adaptation - Background = max/2" ); 
          Serial.println("c - CC, W, G, B(max) and Dark, G, B(0) flashes - No adaptation - Background = max/2" );
          Serial.println("d - CC, G, B, Dark flashes - No adaptation - Background = max white light" ); 
          
          // For calibration
          Serial.println("g - ramp the green LED (0...255, by 5, 0.5/2.0/0.5s)");   
          Serial.println("b - ramp the blue LED  (0...255, by 5, 0.5/2.0/0.5s)");   
          Serial.println("w - ramp both LEDs     (0...255, by 5, 0.5/2.0/0.5s)");   
          break; 
          
// ##################################################################################          
      }
    }
  }
}

/* -----------------------------------------------------------------------------------
 USER DEFINED STIMULI
 ------------------------------------------------------------------------------------*/
// ################################################################################## 

// White flashes - min to max - 15 sec adaptation - with min background level
void myStimulus6 () 
{
  int  nConds    =1;
  byte xG        = maxG-minG;
  byte xB        = maxB-minB;
  
  byte condsG[1] = {xG};
  byte condsB[1] = {xB};
  int  nTrials   =200;    
  byte _bkgG     = minG; //bkgG;    
  byte _bkgB     = minB; //bkgB;
  
  wait_ms(_bkgG, _bkgB, lenPreWave_ms, noMarker);
  playSteps_wMarkerExt(_bkgG, _bkgB, condsG, condsB, nConds, nTrials, 0.5, 1.0, 3.5);
} 


// GBW stimulus - min to max - 15 sec adaptation - with min background level
void myStimulus7 () 
{
  int  nConds    = 3;
  byte xG        = maxG-minG;
  byte xB        = maxB-minB;
  
  byte condsG[3] = {xG, 0, xG};
  byte condsB[3] = {0, xB, xB};
  int  nTrials   = 20;      
  byte _bkgG     = minG; //bkgG;    
  byte _bkgB     = minB; //bkgB;
  
  wait_ms(_bkgG, _bkgB, lenPreWave_ms, noMarker);
  playSteps_wMarkerExt(_bkgG, _bkgB, condsG, condsB, nConds, nTrials, 0.5, 1.0, 3.5);
}  


// White flashes - 0 to max - No adaptation - No background level
void myStimulus8 () 
{
  int  nConds    =1;
  byte xG        = maxG;
  byte xB        = maxB;
  
  byte condsG[1] = {xG};
  byte condsB[1] = {xB};
  int  nTrials   = 60;    
  byte _bkgG     = 0; //bkgG;    
  byte _bkgB     = 0; //bkgB;
  
//  wait_ms(_bkgG, _bkgB, lenPreWave_ms, noMarker);
  playSteps_wMarkerExt(_bkgG, _bkgB, condsG, condsB, nConds, nTrials, 0.5, 1.0, 3.5);
} 


// Dark flashes - max to 0 - No adaptation - Background = max white light
void myStimulus9 () 
{
  int  nConds    =1;
  byte xG        = -maxG;
  byte xB        = -maxB;
  
  byte condsG[1] = {xG};
  byte condsB[1] = {xB};
  int  nTrials   = 60;    
  byte _bkgG     = maxG; //bkgG;    
  byte _bkgB     = maxB; //bkgB;
  
//  wait_ms(_bkgG, _bkgB, lenPreWave_ms, noMarker);
  playSteps_wMarkerExt(_bkgG, _bkgB, condsG, condsB, nConds, nTrials, 0.5, 1.0, 3.5);
}



// White(max) and Dark(0) flashes - No adaptation - Background = max/2 
void myStimulusa () 
{
  int  nConds    =2;
  byte iG1        = -maxG/2;
  byte iB1        = -maxB/2;
  byte iG2        = maxG/2;
  byte iB2        = maxB/2;
  
  
  byte condsG[2] = {iG1, iG2 };
  byte condsB[2] = {iB1, iB2 };
  int  nTrials   = 60;    
  byte _bkgG     = maxG/2; //bkgG;    
  byte _bkgB     = maxB/2; //bkgB;
  
//  wait_ms(_bkgG, _bkgB, lenPreWave_ms, noMarker);
  playSteps_wMarkerExt(_bkgG, _bkgB, condsG, condsB, nConds, nTrials, 0.5, 1.0, 3.5);
}


// W, G, B(max) and Dark, G, B(0) flashes - No adaptation - Background = max/2"
void myStimulusc () 
{
  int  nConds    =6;
  byte iG1        = -maxG/2;
  byte iB1        = -maxB/2;
  byte iG2        = maxG/2;
  byte iB2        = maxB/2;
  
  
  byte condsG[6] = {iG1, 0  , iG1, iG2,  0 , iG2 };
  byte condsB[6] = {0  , iB2, iB1, 0  , iB1, iB2 };
  int  nTrials   = 60;    
  byte _bkgG     = maxG/2; //bkgG;    
  byte _bkgB     = maxB/2; //bkgB;
  
//  wait_ms(_bkgG, _bkgB, lenPreWave_ms, noMarker);
  playSteps_wMarkerExt(_bkgG, _bkgB, condsG, condsB, nConds, nTrials, 0.5, 1.0, 3.5);
}


// CC, G, B, Dark flashes - No adaptation - Background = max white light
void myStimulusd () 
{
  int  nConds    =3;
  byte xG        = -maxG;
  byte xB        = -maxB;
  
  byte condsG[3] = {xG,0 ,xG};
  byte condsB[3] = {0 ,xB,xB};
  int  nTrials   = 60;    
  byte _bkgG     = maxG; //bkgG;    
  byte _bkgB     = maxB; //bkgB;
  
//  wait_ms(_bkgG, _bkgB, lenPreWave_ms, noMarker);
  playSteps_wMarkerExt(_bkgG, _bkgB, condsG, condsB, nConds, nTrials, 0.5, 1.0, 3.5);
}


// For calibration
void myStimulusRamp (int GOn, int BOn) 
{
  int  dStep     = 5;
  int  nConds    = 255/dStep +1; // 255/5 +1 (0 as first step)
  
  byte condsG[nConds];
  byte condsB[nConds];
  for(int j=0; j<nConds; j+=1) {
    condsG[j]  = GOn *j*5;
    condsB[j]  = BOn *j*5;    
  }
  int  nTrials   = 1;    
  byte _bkgG     = 0;
  byte _bkgB     = 0;
  
//wait_ms(_bkgG, _bkgB, lenPreWave_ms, noMarker);
  playSteps_wMarkerExt(_bkgG, _bkgB, condsG, condsB, nConds, nTrials, 0.00, 5.00, 0.00); 

  for(int j=0; j<nConds; j+=1) {
    condsG[j]  = GOn *(255 - j*5);
    condsB[j]  = BOn *(255 - j*5);    
  }

//wait_ms(_bkgG, _bkgB, lenPreWave_ms, noMarker);
  playSteps_wMarkerExt(_bkgG, _bkgB, condsG, condsB, nConds, nTrials, 0.00, 5.00, 0.0);
} 

// ##################################################################################
/* -----------------------------------------------------------------------------------
 Internal methods
 ------------------------------------------------------------------------------------*/
void  setLEDs_onBlankState (byte levG, byte levB)
// Set LED intensity while considering blanking signal from the microscope scanner;
// if blanking signal is HIGH then the LEDs are on, otherwise they are off
//
// levG, levB  : intensity levels for green and blue (0..255)
{
  int blankState = digitalRead(pBlank);

  if(ignoreBlank) {
    analogWrite(pGreenLED, levG);     
    analogWrite(pBlueLED,  levB);         
    return;
  }   
     
  if((blankState == isDuringRetrace) && (on == 0)) {
    on       = 1;
    analogWrite(pGreenLED, levG);     
    analogWrite(pBlueLED,  levB);         
  }
  if((blankState == isDuringScan) && (on == 1)) {
    on       = 0;
    analogWrite(pGreenLED, 0);
    analogWrite(pBlueLED,  0);
  }
}  

// -----------------------------------------------------------------------------------
void  wait_ms (byte levG, byte levB, long period_ms, boolean doMarker)
// Wait for a certain period while taking care of the LEDs being on and off depending
// on the blanking signal
//
// levG, levB  : intensity levels for green and blue (0..255)
// period_ms   : time to wait in ms
// doMarker    : if true then set the marker signal (sync out pin)
{
  long now_ms, tStart_ms;
   
  on           =  1;
  tStart_ms    = millis();
  if(doMarker) {
    digitalWrite(pSyncOut, 1);  
  }  
  analogWrite(pGreenLED, levG);     
  analogWrite(pBlueLED,  levB);           
  
  do {
    now_ms     = millis(); 
    setLEDs_onBlankState(levG, levB);
    if((now_ms -tStart_ms) >= dtMarker_ms) {
      digitalWrite(pSyncOut, 0);
    }
  }  
  while((now_ms -tStart_ms) < period_ms);   
}
  
// -----------------------------------------------------------------------------------
/*
void  playWave_wMarker (int iG, int iB, byte waveMode) 
// Play a sinusoidal wave
//
// levG, levB  : intensity levels for green and blue (0..255)
// iG, iB      : starting index in sin wave array (= basically the phase)
// waveMode    : iso, white, green, blue, or silence
{  
  int           iCycl, blankState;
  unsigned long t_us; 

  iCycl = 0;
  t_us  = micros();
  on    = 1;
 
  while (iCycl < nCyclPerCond) {
    // Switch sync signal
    //
    if(fMarkerOrTest == 0) {
      if((iG % 2) > 0) {
        digitalWrite(pSyncOut, 1);      
      }  
      else {
        digitalWrite(pSyncOut, 0);    
      } 
    } 
    else {
      if(iG == 0) {
        digitalWrite(pSyncOut, 1);
      }  
      if(iG == (lenMarker_frs +1)) {
        digitalWrite(pSyncOut, 0);
      }  
    }  
  
    // Check if time for a 'frame' is up ...
    //
    blankState = digitalRead(pBlank);
   
    if((blankState == isDuringRetrace)&& (on == 0)) {
      on = 1; 
      switch (waveMode) {
        case ISO:
          analogWrite(pGreenLED, sinArrG_iso[iG]);     
          analogWrite(pBlueLED,  sinArrB[iB]);           
          break;
 
        case WHITE:
          analogWrite(pGreenLED, sinArrG_white[iG]);     
          analogWrite(pBlueLED,  sinArrB[iB]);           
          break;
        
        case GREEN:
          analogWrite(pGreenLED, sinArrG[iG]);     
          analogWrite(pBlueLED,  levB_0  );           
          break;
          
        case BLUE:
          analogWrite(pGreenLED, levB_0  );     
          analogWrite(pBlueLED,  sinArrB[iB]);           
          break;
          
        case SILENCE:
          analogWrite(pGreenLED, sinArrG_silencing[iG] );     
          analogWrite(pBlueLED,  sinArrB[iB]);           
          break;
      }   
    }
    if((blankState == isDuringScan) && (on == 1)) {
      on = 0;
      analogWrite(pGreenLED, 0);
      analogWrite(pBlueLED,  0);
    }
   
    if((micros() -t_us) > dt_us) {
      // Save time 
      //
      t_us = micros();

      // Change PWM for the two LEDs depending on sine array
      //
      iG += 1;
      iB += 1;
      if(iG >= refreshSFreq) {
        iG     = 0;
        iCycl += 1;
      }  
      if(iB >= refreshSFreq) {
        iB     = 0;
      }  
    }
  } 
}  
*/

// -----------------------------------------------------------------------------------  
void  playSteps_wMarker (int nCycle, double duration) 
{  
  int           iCycl, blankState;
  unsigned long t_us; 

  iCycl = 0;
  t_us  = micros();
  on    = 1;

  digitalWrite(pSyncOut, 1);

  while (iCycl < 4*nCycle) {
    // Switch sync signal
    //
    if(fMarkerOrTest == 0) {
      if((iCycl % 2) > 0) {
        digitalWrite(pSyncOut, 1);      
      }  
      else {
        digitalWrite(pSyncOut, 0);    
      } 
    } 
    else {
      if(micros()-t_us > lenMarker_us) {
        digitalWrite(pSyncOut, 0);
      }  
    }  
  
    // Check if time for a 'frame' is up ...
    //
    blankState = digitalRead(pBlank);
   
    if((blankState == isDuringRetrace) && (on == 0)) {
      on = 1; 
      switch (iCycl % 4) {
        case 1:
          analogWrite(pGreenLED, maxG);     
          analogWrite(pBlueLED,  minB);           
          break;
        
        case 0:
          analogWrite(pGreenLED, minG);     
          analogWrite(pBlueLED,  minB);           
          break;
          
        case 3:
          analogWrite(pGreenLED, minG);     
          analogWrite(pBlueLED,  maxB);           
          break;
          
        case 2:
          analogWrite(pGreenLED, minG);     
          analogWrite(pBlueLED,  minB);           
          break;
      }   
    }
    if((blankState == isDuringScan) && (on == 1)) { 
      on = 0;
      analogWrite(pGreenLED, 0);
      analogWrite(pBlueLED,  0);
    }

    if((micros() -t_us) > (long)(duration*1000000)) {
      // Save time 
      //
      t_us   = micros();
      iCycl += 1;
      digitalWrite(pSyncOut, 1);    
    }  
  } 
}  

// -----------------------------------------------------------------------------------  
void  playSteps_wMarkerExt (byte _bkgG, byte _bkgB, byte _levG[], byte _levB[], int _nCond, 
                            int _nTrials, float _durPre_s, float _durStep_s, float _durPost_s) 
{  
  int           blankState, stimState;
  int           iTrial, iCond, n;
  unsigned long t_us; 
  float         durPresent_s;

  on           = 1;
  n            = 0;
  stimState    = 0;
  durPresent_s = _durPre_s +_durStep_s +_durPost_s;

  digitalWrite(pSyncOut, 1);
  
  for(iTrial=0; iTrial<_nTrials; iTrial++) {
    for(iCond=0; iCond<_nCond; iCond++) {

      t_us  = micros();
      while((micros() -t_us) < (long)(durPresent_s*1000000)) {
        // Switch sync signal
        //
        if(fMarkerOrTest == 0) {
          if((n % 2) > 0) 
            digitalWrite(pSyncOut, 1);      
          else 
            digitalWrite(pSyncOut, 0);    
        } 
        else {
          if(micros()-t_us > lenMarker_us) {
            digitalWrite(pSyncOut, 0);
          }  
        }  
 
        // Control LEDs depending on blanking state
        //
        blankState = digitalRead(pBlank);
        if(ignoreBlank) {
          blankState = isDuringRetrace;
          on         = 0;
        }

        if((blankState == isDuringRetrace) && (on == 0)) {
          // Microscope is retracing, now switch the LEDs on
          //
          on = 1; 
          switch (stimState) {
            case 0: // before step (pre)
              if((micros() -t_us) > (long)(_durPre_s*1000000))
                stimState += 1;
              else {  
                analogWrite(pGreenLED, _bkgG);     
                analogWrite(pBlueLED,  _bkgB);           
              }
              break;
            
            case 1: // during step 
              if((micros() -t_us) > (long)((_durPre_s +_durStep_s)*1000000))
                stimState += 1;
              else {  
                analogWrite(pGreenLED, _bkgG +_levG[iCond]);     
                analogWrite(pBlueLED,  _bkgB +_levB[iCond]);      
              }
              break;
              
            case 2: // after step (post)
              analogWrite(pGreenLED, _bkgG);     
              analogWrite(pBlueLED,  _bkgB);           
              break;
          }
        }    
        if((blankState == isDuringScan) && (on == 1)) { 
          // Microscope scans a line, keep LEDs off
          //
          on = 0;
          analogWrite(pGreenLED, 0);
          analogWrite(pBlueLED,  0);
        }
      }
      // Single presentation done, next ...
      //   
      n         += 1;
      stimState  = 0;
      digitalWrite(pSyncOut, 1);    
    }  
  }     
}  

// -----------------------------------------------------------------------------------
void  establishContact() 
{
  while (Serial.available() <= 0) {
    delay(250);
  }
  Serial.println("ready.");   
}

// -----------------------------------------------------------------------------------
void  beginStimulus()
{
  Serial.print("-> running stimulus ...");
}  

void  endStimulus()
{
  digitalWrite(pSyncOut, 0);  
  Serial.println(" done");   
  stgreen = 0;
  stblue  = 0;   
}  

// -----------------------------------------------------------------------------------


