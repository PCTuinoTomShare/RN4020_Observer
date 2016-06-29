
// RN4020 BLE observer test.
// by PCT / tom hsieh.
// Board : Arduino M0 PRO ( MCU : ATSAMD21G18A, ARM Cortex M0+ ). 
// IDE : Arduino 1.7.10 ( Arduino.ORG ).
#include <Wire.h>

// Connect to Microchip RN4020 bluetooth LE module, presetting :
// - buadrate 9600pbs.
// - Central.
// - Not support MLDP.
#define MLDP     13  // RN4020 MLDP control pin.
#define HW_WAKE  12  // RN4020 hardware wake control pin.
#define SW_WAKE  11  // RN4020 software wake control pin.
#define CONN_LED 8   // RN4020 connect LED.
#define TEST_OUT 6   // Test output.

// RN4020 command, enter observer. 
const char cmd_observer_on[4] = {'J',',','1','\r'}; 
// RN4020 command, exit observer. 
const char cmd_observer_off[4] = {'J',',','0','\r'}; 
// RN4020 command, scan on. 
const char cmd_scan_on[2] = {'F','\r'};
// RN4020 command, scan off. 
const char cmd_scan_off[2] = {'X','\r'};
// Received ID check.
const char rec_id_chk[10] = {'0','2','0','1','0','4','0','5','F','F'};

char lcm_text_out[34];         // LCM output text
//char test_out[10];             // Test out. 
char uart_rec_data[32];        // UART received data. 

unsigned char temp1;           // Temporary #1. 
unsigned char temp2;           // Temporary #2.
unsigned char uart_rec_task;   // UART received task count.
unsigned char uart_rec_index;  // UART received index.

unsigned long temp3;           // Temporary #3.
unsigned long humidity;        // Humidity value.
unsigned long temperature;     // Temperarure value.
unsigned long ms_cur;          // Minisecond time counter current.
unsigned long ms_pre;          // Minisecond time counter previous.
unsigned long uart_rec_to;     // UART received timeout. 
unsigned long ms_10ms_past;    // Minisecond time 10ms past.

void setup() {

  // put your setup code here, to run once:
  // IO port initialize.
  pinMode( MLDP, OUTPUT );
  pinMode( HW_WAKE, OUTPUT );
  pinMode( SW_WAKE, OUTPUT );  
  pinMode( TEST_OUT, OUTPUT );
  pinMode( CONN_LED, INPUT );

  // UART begin.  
  Serial5.begin( 9600 ); // Arduino UART interface.   
  // I2C begin.
  Wire.begin();
  
  // RN4020 control.
  digitalWrite( MLDP, LOW );        // Command mode.
  digitalWrite( HW_WAKE, HIGH );    // Hardware wake.
  digitalWrite( SW_WAKE, HIGH );    // Software wake.
  //digitalWrite( TEST_OUT, LOW );
  
  delay( 100 ); 
  // Enter observer.
  Serial5.write( cmd_observer_on );
  //Serial.begin( 9600 ); // Atmel EDBG CDC port, for test output.
    
  delay( 100 ); 
  // Scan on.
  Serial5.write( cmd_scan_on );
  
  // Clear counter.
  uart_rec_task = 0;

  // Reset LCM show data.
  lcm_text_out[0] = '0';
  lcm_text_out[1] = '0';
  
  lcm_text_out[2] = 'H';
  lcm_text_out[3] = 'u';
  lcm_text_out[4] = 'm';
  lcm_text_out[5] = 'i';
  lcm_text_out[6] = ':';
  lcm_text_out[7] = ' ';
  lcm_text_out[8] = ' ';
  lcm_text_out[9] = ' ';
  
  lcm_text_out[10] = '0';  
  lcm_text_out[11] = '0';  
  lcm_text_out[12] = '0';
  lcm_text_out[13] = '.';
  lcm_text_out[14] = '0';
  
  lcm_text_out[15] = '%';  
  lcm_text_out[16] = 'R';
  lcm_text_out[17] = 'H';

  lcm_text_out[18] = 'T';
  lcm_text_out[19] = 'e';
  lcm_text_out[20] = 'm';
  lcm_text_out[21] = 'p';
  lcm_text_out[22] = ':';
  lcm_text_out[23] = ' '; 
  lcm_text_out[24] = ' ';
  lcm_text_out[25] = ' ';
  
  lcm_text_out[26] = ' ';  
  lcm_text_out[27] = '0';
  lcm_text_out[28] = '0';
  lcm_text_out[29] = '0';
  lcm_text_out[30] = '.';
  lcm_text_out[31] = '0';
  lcm_text_out[32] = ' ';    
  lcm_text_out[33] = 'C';

  //test_out[8] = '\r';
  //test_out[9] = '\n';  
}

void loop() {
  // put your main code here, to run repeatedly:

  // Time counting. 
  ms_cur = millis();  
  ms_10ms_past = ms_cur;
  ms_10ms_past -= ms_pre;
  if( ms_10ms_past > 10 ){
    // UART received tim eout.
    if( uart_rec_task ){ 
        ++uart_rec_to;
        if( uart_rec_to == 20 ){
           uart_rec_to = 0;
           uart_rec_task = 0;
        }
    }      
    // Keep current value as previous.
    ms_pre = ms_cur;    
  }
  
  // UART received data.
  if( Serial5.available() ){ 
    UART_Rec_Check();
  }
}

//---------------------------------------------------------------------------------------------------
// UART recieved check.
void UART_Rec_Check( void ){
  // Clear time out.
  uart_rec_to = 0;
  // Read data.
  temp1 = Serial5.read();
  
  switch( uart_rec_task ){
  
    case 0:
      // Check start byte.
      if( temp1 != ':' ){
        // Error
        return;
      }     
      // Clear index.
      uart_rec_index = 0;
      ++uart_rec_task;
      return;
    
    case 1:
      // Check ID.
      if( temp1 != rec_id_chk[ uart_rec_index ] ){
        // Error.  
        uart_rec_task = 0;
        return;
      }
      // Pass.
      ++uart_rec_index;
      if( uart_rec_index == 10 ){
        // All pass.
        uart_rec_index = 0;
        ++uart_rec_task; 
      }     
      return;

    case 2:
      // Receive data.
      uart_rec_data[ uart_rec_index ] = temp1;
      //test_out[ uart_rec_index ] = temp1;
      ++uart_rec_index;      
      if( uart_rec_index != 8 ){
        return;
      }
      uart_rec_task = 0;      
      break;  
  }

  //digitalWrite( TEST_OUT, HIGH );

  // Humidity value. 
  temp2 = uart_rec_data[0];
  ToValue();  
  humidity = temp1;
  humidity <<= 12;  
  temp2 = uart_rec_data[1];
  ToValue();  
  temp3 = temp1;
  temp3 <<= 8;
  humidity |= temp3;  
  temp2 = uart_rec_data[2];
  ToValue();  
  temp3 = temp1;
  temp3 <<= 4;
  humidity |= temp3;  
  temp2 = uart_rec_data[3];
  ToValue();  
  temp3 = temp1;
  humidity |= temp3;  
  humidity &= 0x3fff;  
  humidity *= 1000;
  humidity /= 16383;
  
  temp3 = humidity;
  temp3 /= 1000;
  humidity %= 1000;  
  lcm_text_out[10] = (unsigned char)temp3;
  lcm_text_out[10] += 0x30;  
  temp3 = humidity;
  temp3 /= 100;
  humidity %= 100;  
  lcm_text_out[11] = (unsigned char)temp3;
  lcm_text_out[11] += 0x30;  
  temp3 = humidity;
  temp3 /= 10;
  humidity %= 10;  
  lcm_text_out[12] = (unsigned char)temp3;
  lcm_text_out[12] += 0x30;  
  lcm_text_out[14] = (unsigned char)humidity;
  lcm_text_out[14] += 0x30;
  
  // Temperature value.
  temp2 = uart_rec_data[4];
  ToValue();  
  temperature = temp1;
  temperature <<= 12;  
  temp2 = uart_rec_data[5];
  ToValue();  
  temp3 = temp1;
  temp3 <<= 8;
  temperature |= temp3;  
  temp2 = uart_rec_data[6];
  ToValue();  
  temp3 = temp1;
  temp3 <<= 4;
  temperature |= temp3;  
  temp2 = uart_rec_data[7];
  ToValue();  
  temp3 = temp1;
  temperature |= temp3;  
  temperature >>= 2;
  
  temperature *= 1650;
  temperature /= 16383;
  
  lcm_text_out[26] = ' ';
  if( temperature < 400 ){
      temp3 = temperature; 
      temperature = 400;
      temperature -= temp3;
      lcm_text_out[26] = '-';
  }
  else{
    temperature -= 400;
  } 
  
  temp3 = temperature;
  temp3 /= 1000;
  temperature %= 1000;
  lcm_text_out[27] = (unsigned char)temp3;
  lcm_text_out[27] += 0x30;
  
  temp3 = temperature;
  temp3 /= 100;
  temperature %= 100;
  lcm_text_out[28] = (unsigned char)temp3;
  lcm_text_out[28] += 0x30;
  
  temp3 = temperature;
  temp3 /= 10;
  temperature %= 10;
  lcm_text_out[29] = (unsigned char)temp3;
  lcm_text_out[29] += 0x30;
  
  lcm_text_out[31] = (unsigned char)temperature;
  lcm_text_out[31] += 0x30;
   
  // send LCM display data via I2C.
  Wire.beginTransmission( 0x26 );
  Wire.write( lcm_text_out, 34 );
  Wire.endTransmission();      
  
  //Serial.write( test_out );
    
}
//---------------------------------------------------------------------------------------------------
// Character to value.
void ToValue( void ){  
  // '0' ~ '9'.
  temp1 = temp2;
  temp1 &= 0x10;
  if( temp1 ){
    temp1 = temp2;
    temp1 &= 0x0f;
    return;
  }
  // 'A' ~ 'F' or 'a' ~ 'f'.
  temp1 = temp2;
  temp1 &= 0x40;
  if( temp1 ){
    temp1 = temp2;
    temp1 &= 0x0f;
    temp1 += 9;
  }
}
//---------------------------------------------------------------------------------------------------

