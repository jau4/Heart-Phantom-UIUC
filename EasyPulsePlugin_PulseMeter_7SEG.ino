/* Project: Digital Pulse Meter
   Description: Arduino processes the output of Easy Pulse Plugin sensor to compute
   the heart beat rate and display it on seven segment LEDs.
   
   Written by: Raj Bhatt (www.Embedded-Lab.com)
   Date: May 5, 2015

*/

#include "LedControl.h"
// Arduino Pin 7 to DIN, 6 to Clk, 5 to LOAD, no.of devices is 1
LedControl lc=LedControl(7,6,5,1);

#define Sampling_Time 5
#define Num_Samples 400
#define Peak_Threshold_Factor 85
#define Minimum_Range 254
#define Minimum_Peak_Separation 75  // 75*5=375 ms
#define DC_Added 10;
#define Moving_Average_Num 5

void setup() {
   // Initialize the MAX7219 device
  lc.shutdown(0,false);   // Enable display
  lc.setIntensity(0,15);  // Set brightness level (0 is min, 15 is max)
  lc.clearDisplay(0);     // Clear display register
  Serial.begin(9600);
  pinMode(DAC0,OUTPUT);
}

int ADC_Samples[Num_Samples], Index1, Index2, Index3, Peak1, Peak2, Peak3;
long Pulse_Rate, Temp1=1L, Pulse_Time1, Pulse_Time2;
int Peak_Magnitude, Peak_Threshold, Minima, Range;

void loop() {
  
  Read_ADC_Samples();
  Serial.println("Sample Read Finished ");
  Remove_DC();
  Serial.println("DC component subtracted ");
  Scale_Data();
  Serial.println("Data scaled ");
  
  if (Range > Minimum_Range){  // ADC range is > 70, otherwise increase gain
    Filter_Data();
    Serial.println("Data Filtered ");
  }
  else{
    Print_Error_Message();
  }
  
}  // Main Loop
    
// Read ADC samples at a predefined interval
 void Read_ADC_Samples(){
   for (int i = 0; i < Num_Samples; i++){
      ADC_Samples[i] = analogRead(A0);
      delay(Sampling_Time);  
   }  
 }

 void Remove_DC(){
   Find_Minima(0);
   Serial.print("Minima = ");
   Serial.println(Minima);
   // Subtract DC (minima) 
   for (int i = 0; i < Num_Samples; i++){
     ADC_Samples[i] = ADC_Samples[i] - Minima;
   }
   Minima = 0;  // New minima is zero  
 }  // Remove_DC

void Find_Minima(int Num){
  Minima = 1024;
  for (int m = Num; m < Num_Samples-Num; m++){
      if(Minima > ADC_Samples[m]){
         Minima = ADC_Samples[m];
      }
  }
}  // Find_Minima  

void Scale_Data(){
  // Find peak value
  Find_Peak(0);
  Serial.print("Peak = ");
  Serial.println(Peak_Magnitude);
  Serial.print("Minima = ");
  Serial.println(Minima);
  Range = Peak_Magnitude - Minima;
  Serial.print("Range = ");
  Serial.println(Range);
  // Sclae from 1 to 1023 
  for (int i = 0; i < Num_Samples; i++){
     ADC_Samples[i] = map(ADC_Samples[i], 0, Range, 0, 255);
     
  }
  Find_Peak(0);
  Find_Minima(0);
  Serial.print("Scaled Peak = ");
  Serial.println(Peak_Magnitude);
  Serial.print("Scaled Minima = ");
  Serial.println(Minima);
  
} // Scale_Data


void Filter_Data(){
  int Num_Points = 2*Moving_Average_Num+1;
  for (int i = Moving_Average_Num; i < Num_Samples-Moving_Average_Num; i++){
    int Sum_Points = 0;
    for(int k =0; k < Num_Points; k++){   
      Sum_Points = Sum_Points + ADC_Samples[i-Moving_Average_Num+k]; 
    }    
    ADC_Samples[i] = Sum_Points/Num_Points; 
  } 
}  // Filter_Date

void Output_Signal(){
   for(int i = 0; i < Sampling_Time*Num_Samples; i++){
      analogWrite(DAC0, ADC_Samples[i]);
      delay(5);
   }
}
