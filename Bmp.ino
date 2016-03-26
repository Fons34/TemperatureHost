#include <Wire.h>
#define BMP180_ADDRESS 0x77
const unsigned char OSS = 0;  // Oversampling Setting for BMP180

// Calibration values for BMP180
int ac1;
int ac2; 
int ac3; 
unsigned int ac4;
unsigned int ac5;
unsigned int ac6;
int b1; 
int b2;
int mb;
int mc;
int md;
// b5 is calculated in BMP180GetTemperature(...), this variable is also used in BMP180GetPressure(...)
// so ...Temperature(...) must be called before ...Pressure(...).
long b5;

void Bmp_Start() {
  Wire.begin();  
  BMP180Calibration();
}

void Bmp_Read(short *temp, long *pres) {
  // temp in units of 0.1 deg C
  // pres in units of 0.01 hPa
  *temp = BMP180GetTemperature(BMP180ReadUT());
  *pres = BMP180GetPressure(BMP180ReadUP());  
}

// Stores all of the BMP180's calibration values into global variables
// Calibration values are required to calculate temp and pressure
// This function should be called at the beginning of the program
void BMP180Calibration()
{
  ac1 = BMP180ReadInt(0xAA);
  ac2 = BMP180ReadInt(0xAC);
  ac3 = BMP180ReadInt(0xAE);
  ac4 = BMP180ReadInt(0xB0);
  ac5 = BMP180ReadInt(0xB2);
  ac6 = BMP180ReadInt(0xB4);
  b1 = BMP180ReadInt(0xB6);
  b2 = BMP180ReadInt(0xB8);
  mb = BMP180ReadInt(0xBA);
  mc = BMP180ReadInt(0xBC);
  md = BMP180ReadInt(0xBE);
}

// Calculate temperature given ut.
// Value returned will be in units of 0.1 deg C
short BMP180GetTemperature(unsigned int ut)
{
  long x1, x2;
  
  x1 = (((long)ut - (long)ac6)*(long)ac5) >> 15;
  x2 = ((long)mc << 11)/(x1 + md);
  b5 = x1 + x2;

  return ((b5 + 8)>>4);  
}

// Calculate pressure given up
// calibration values must be known
// b5 is also required so BMP180GetTemperature(...) must be called first.
// Value returned will be pressure in units of Pa.
long BMP180GetPressure(unsigned long up)
{
  long x1, x2, x3, b3, b6, p;
  unsigned long b4, b7;
  
  b6 = b5 - 4000;
  // Calculate B3
  x1 = (b2 * (b6 * b6)>>12)>>11;
  x2 = (ac2 * b6)>>11;
  x3 = x1 + x2;
  b3 = (((((long)ac1)*4 + x3)<<OSS) + 2)>>2;
  
  // Calculate B4
  x1 = (ac3 * b6)>>13;
  x2 = (b1 * ((b6 * b6)>>12))>>16;
  x3 = ((x1 + x2) + 2)>>2;
  b4 = (ac4 * (unsigned long)(x3 + 32768))>>15;
  
  b7 = ((unsigned long)(up - b3) * (50000>>OSS));
  if (b7 < 0x80000000)
    p = (b7<<1)/b4;
  else
    p = (b7/b4)<<1;
    
  x1 = (p>>8) * (p>>8);
  x1 = (x1 * 3038)>>16;
  x2 = (-7357 * p)>>16;
  p += (x1 + x2 + 3791)>>4;
  
  return p;
}


// Read 1 byte from the BMP180 at 'address'
char BMP180Read(unsigned char address)
{
  unsigned char data;
  
  Wire.beginTransmission(BMP180_ADDRESS);
  Wire.write(address);
  Wire.endTransmission();
  
  Wire.requestFrom(BMP180_ADDRESS, 1);
  while(!Wire.available())
    ;
    
  return Wire.read();
}

// Read 2 bytes from the BMP180
// First byte will be from 'address'
// Second byte will be from 'address'+1
int BMP180ReadInt(unsigned char address)
{
  unsigned char msb, lsb;
  
  Wire.beginTransmission(BMP180_ADDRESS);
  Wire.write(address);
  Wire.endTransmission();
  
  Wire.requestFrom(BMP180_ADDRESS, 2);
  while(Wire.available()<2)
    ;
  msb = Wire.read();
  lsb = Wire.read();
  
  return (int) msb<<8 | lsb;
}

// Read the uncompensated temperature value from BMP180
unsigned int BMP180ReadUT()
{
  unsigned int ut;
  
  // Write 0x2E into Register 0xF4
  // This requests a temperature reading
  Wire.beginTransmission(BMP180_ADDRESS);
  Wire.write(0xF4);
  Wire.write(0x2E);
  Wire.endTransmission();
  
  // Wait at least 4.5ms
  delay(5);
  
  // Read two bytes from registers 0xF6 and 0xF7
  ut = BMP180ReadInt(0xF6);
  return ut;
}

// Read the uncompensated pressure value from BMP180
unsigned long BMP180ReadUP()
{
  unsigned char msb, lsb, xlsb;
  unsigned long up = 0;
  
  // Write 0x34+(OSS<<6) into register 0xF4
  // Request a pressure reading w/ oversampling setting
  Wire.beginTransmission(BMP180_ADDRESS);
  Wire.write(0xF4);
  Wire.write(0x34 + (OSS<<6));
  Wire.endTransmission();
  
  // Wait for conversion, delay time dependent on OSS
  delay(2 + (3<<OSS));
  
  // Read register 0xF6 (MSB), 0xF7 (LSB), and 0xF8 (XLSB)
  Wire.beginTransmission(BMP180_ADDRESS);
  Wire.write(0xF6);
  Wire.endTransmission();
  Wire.requestFrom(BMP180_ADDRESS, 3);
  
  // Wait for data to become available
  while(Wire.available() < 3)
    ;
  msb = Wire.read();
  lsb = Wire.read();
  xlsb = Wire.read();
  
  up = (((unsigned long) msb << 16) | ((unsigned long) lsb << 8) | (unsigned long) xlsb) >> (8-OSS);
  
  return up;
}
