#include "math.h"
#include "cellular_hal.h"

#define MAX_PHONE_NUMBER 14
#define CTRL_Z 0x1A
#define TIMEOUT 10000

#define RL_VALUE 5                    // load on the rensor
#define R0_CLEAN_AIR_FACTOR 9.83      // standard air/RO

#define CALIBRATION_TIMES 10            // number of samples when calibrating
#define CALIBRATION_INTERVAL 200        // time interval (milliseconds) between each sample

#define READ_INTERVAL 20                // number of samples in a normal run
#define READ_TIMES 5                    // time interval (milliseconds) between each sample

#define LPG_GAS 0
#define CO_GAS 1
#define SMOKE_GAS 2

float LPGCurve[3] = {2.3, 0.21, -0.47};
float COCurve[3] = {2.3, 0.72, -0.34};
float SmokeCurve[3] = {2.3, 0.53, -0.44};

float R0 = 10;

unsigned char id;
boolean gain;     // Gain setting, 0 = X1, 1 = X16;
unsigned int ms;  // Integration ("shutter") time in milliseconds
unsigned int data1;
unsigned int data2;
volatile int readPin = D7;

char szPhoneNumber[MAX_PHONE_NUMBER] = "14038001118"; //default phone number that will be used for SMS texts

int lowerLimit = 10;
boolean smsSend = false;

int prnToScr(String command);
int setMotion(String command);  //for the "call function "
int setMotionSMS = -1;

STARTUP(cellular_credentials_set("isp.telus.com", "", "", NULL)); //required fer Telus SIM cards

void setup()
{
    // Subscribe to the integration response event:
    Particle.subscribe("hook-response/Motion Detected", myHandler, MY_DEVICES);

    Particle.function("test", prnToScr);
    Serial.begin(115200); //set serial baud rate for console
    Particle.function("setMotion", setMotion); //enables the Android app and Particle Console to turn the motion sensor off or on
    setMotionOff();
//    setMotionOn(); // ONLY FOR TESTING, DELETE WHEN APP IMPLEMENTED
}

void setupagain()
{
    // Subscribe to the integration response event:
    Particle.subscribe("hook-response/Motion Detected", myHandler, MY_DEVICES);

    Particle.function("test", prnToScr);
    Serial.begin(115200); //set serial baud rate for console
    Particle.function("setMotion", setMotion); //enables the Android app and Particle Console to turn the motion sensor off or on
}

void loop()
{
  setupagain();
  if(setMotionSMS == 0)
  {
    disarmPrint();
  }
  else if(setMotionSMS == 1)
  {
    armPrint();
  }
  else
  {
    setMotionSMS = -1;
  }

  if (digitalRead(readPin) == HIGH) //if the pin is triggered by the motion sensor...
  {
    Serial.println("MOTION DETECTED"); //print message to screen (using Putty on Serial Port 9 at 115200)
    Serial.println();

    // Get some data:
    String data = String(10);
    // Trigger the integration onto the web through ThingSpeak:
    Particle.publish("Motion Detected", data, PRIVATE);

    char szMessage[64] = "Motion Detected!"; //sets the message to be sent to the customer
    sendMessage(szMessage); // sends message to the cell phone number on file

//    Serial.println(Time.format(TIME_FORMAT_ISO8601_FULL));

    delay(5000); //delay needed so that the Particle.io account deos not get "spammed" by this firmware. Can be set lower if customer needs more datapoints, but custower should then contact Particle Cloud to inform them of the higher rate requirement on their account.
  }
  else
  {
    Serial.println("No motion");  //print message to screen (using Putty on Serial Port 9 at 115200)
  }
    delay(1000);

    float R0 = MQCalibration();
    printGasValues(R0);
}

void disarmPrint() {
  Serial.println("Sending disarmed SMS...");
  char szMotionOffMessage[64] = "Motion disarmed."; //sets the message to be sent through sms
  sendMessage(szMotionOffMessage);  //sends the sms text to the original number
  numberChanger("14034644043"); //calls the number changer function to change the phone number to this new number
  sendMessage(szMotionOffMessage);  //sends the sms text to the new number
  numberChanger("14038001118"); //calls the number changer function to change the phone number back to the number
  setMotionSMS = -1;
}

void armPrint() {
  Serial.println("Sending ARMED SMS...");
  char szMotionOnMessage[64] = "Motion ARMED!"; //sets the message to be sent through sms
  sendMessage(szMotionOnMessage);  //sends the sms text to the original number
  numberChanger("14034644043"); //calls the number changer function to change the phone number to this new number
  sendMessage(szMotionOnMessage);  //sends the sms text to the new number
  numberChanger("14038001118"); //calls the number changer function to change the phone number back to the number
  setMotionSMS = -1;
}

int callback(int type, const char* buf, int len, char* param)
{
    Serial.print("Return: ");
    Serial.write((const uint8_t*)buf, len);
    Serial.println();

    return WAIT;
}

char* getPhoneNumber()
{
    return szPhoneNumber;
}

int setLowerLimit(String args)
{
    lowerLimit = args.toInt();

    return lowerLimit;
}

void numberChanger(String num)  //phone number changer function
{
    char charBuf[MAX_PHONE_NUMBER];
    num.toCharArray(charBuf, MAX_PHONE_NUMBER);

    for (int i = 0; i < MAX_PHONE_NUMBER; i++)
    {
        szPhoneNumber[i] = charBuf[i];
    }
}

int sendMessage(char* pMessage) //for sending sms text message to cell phone
{
    char szCmd[64];

    sprintf(szCmd, "AT+CMGS=\"+%s\",145\r\n", szPhoneNumber);

    Serial.print("Sending command to SMS Centre...");
    Serial.print(szCmd);
    Serial.println();

    char szReturn[32] = "";

    Cellular.command(callback, szReturn, TIMEOUT, "AT+CMGF=1\r\n");
    Cellular.command(callback, szReturn, TIMEOUT, szCmd);
    Cellular.command(callback, szReturn, TIMEOUT, pMessage);

    sprintf(szCmd, "%c", CTRL_Z);

    int retVal = Cellular.command(callback, szReturn, TIMEOUT, szCmd);

    if(RESP_OK == retVal)
    {
        Serial.println("+OK, SMS text sent!");
    }
    else
    {
        Serial.println("+ERROR, error sending SMS text.");
    }
    return retVal;
}

int setMotion(String command) //function for the received POST from the Android App or Particle Cloud function call module
{
  if(command == "Disarm") //if Disarm "message" is received from the Android App or Particle Cloud function call module
  {
    setMotionOff(); //calls the function that turns the motion sensor off
    Serial.println("Motion disarmed."); //print message to screen (using Putty on Serial Port 9 at 115200)
    setMotionSMS = 0;
    return 0;
  }
  else if(command == "Arm") //if Arm "message" is received from the Android App or Particle Cloud function call module
  {
    setMotionOn(); //calls the function that turns the motion sensor off
    Serial.println("Motion ARMED!"); //print message to screen (using Putty on Serial Port 9 at 115200)
    setMotionSMS = 1;
    return 1;
  }
  else
  {
    setMotionSMS = -1;
    return -1;
  }
}

int prnToScr(String command)
{
    Serial.println("called func successfully");
    return 1;
}

void setMotionOn()  //turns the motion sensor on
{
    pinMode(readPin,INPUT);
    digitalRead(readPin);

}

void setMotionOff() //turns the motion sensor off
{
    pinMode(readPin,OUTPUT);
    digitalWrite(readPin, LOW);
}

void myHandler(const char *event, const char *data)
{
  // Handle the integration response
}

// calibrates the sensor by using the normal air quality value
float MQCalibration()
{
    int i;
    float calibration = 0;
    for (i = 0; i < READ_TIMES; i++)
    {
      calibration += MQResistanceCalculation(analogRead(A0));
      delay(CALIBRATION_INTERVAL);
    }
    calibration = calibration/CALIBRATION_TIMES;
    calibration = calibration/R0_CLEAN_AIR_FACTOR;
    return calibration;
}

// reads from the sensor to get gas value
float MQRead()
{
    int i;
    float rs = 0;

    for (i = 0; i < READ_TIMES; i++) {
      rs += MQResistanceCalculation(analogRead(A0));
      delay(READ_INTERVAL);
    }
    rs = rs/READ_TIMES;
    return rs;
}

// calculates the sensor resistance
float MQResistanceCalculation(int adc) {
    return ( ((float)RL_VALUE*(1023-adc)/adc));
}


int getGasPercentage(float rs_ro_ratio, int gas_id)
{
    if (gas_id == LPG_GAS) {
      return MQGetPercentage(rs_ro_ratio,LPGCurve);
    }
    else if (gas_id == CO_GAS) {
      return MQGetPercentage(rs_ro_ratio,COCurve);
    }
    else if (gas_id == SMOKE_GAS) {
      return MQGetPercentage(rs_ro_ratio,SmokeCurve);
    }
    return 1;                 // the 1 is just to see if this effects anything
}

int MQGetPercentage(float rs_ro_ratio, float *pcurve)
{
    return (pow(10,( ((log(rs_ro_ratio)-pcurve[1])/pcurve[2]) + pcurve[0])));
}

void printGasValues(float r0Value) {
  Serial.println("UPDATE");
  Serial.println("======");
  Serial.print("Air Purity (R0) [higher # = cleaner air]: ");
  Serial.println(r0Value);
//  Particle.publish("Gas Sensor Readings", r0Value, PRIVATE);  //for gas sensor webhook
  Serial.print("Liquified Petroleum Gas (LPG): ");
  Serial.print(getGasPercentage(MQRead()/r0Value, LPG_GAS) );
  int LPG_GASVALUE = getGasPercentage(MQRead()/r0Value, LPG_GAS);
//  Particle.publish("Gas Sensor Readings", LPG_GASVALUE, PRIVATE);  //for gas sensor webhook
  Serial.println( " ppm" );
  Serial.print("Carbon Monoxide (CO): ");
  Serial.print(getGasPercentage(MQRead()/r0Value, CO_GAS) );
  int CO_GASVALUE = getGasPercentage(MQRead()/r0Value, CO_GAS);
//  Particle.publish("Gas Sensor Readings", CO_GASVALUE, PRIVATE);  //for gas sensor webhook
  Serial.println( " ppm" );
  Serial.print("SMOKE: ");
  Serial.print(getGasPercentage(MQRead()/r0Value, SMOKE_GAS) );
  int SMOKE_GASVALUE = getGasPercentage(MQRead()/r0Value, SMOKE_GAS);
//  Particle.publish("Gas Sensor Readings", SMOKE_GASVALUE, PRIVATE);  //for gas sensor webhook
  Serial.println( " ppm" );
  Serial.println();
}
