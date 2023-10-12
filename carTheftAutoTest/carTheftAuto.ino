#include <SoftwareSerial.h>
#define SLEEP_PIN D2 // Make this pin HIGH to make A9G board to go to sleep mode

SoftwareSerial A9GSerial(D6, D7); // RX, TX (Connect A9G's TX to NodeMCU's D7, and A9G's RX to NodeMCU's D6)
String SOS_NUM = "+918692870096"; // Add a number on which you want to receive call or SMS


// Necessary Variables
boolean stringComplete = false;
String inputString = "";
String fromGSM = "";
bool CALL_END = 1;
char* response = " ";
String res = "";
int c = 0;

const int buttonPin = D2; // Button connected to D2
bool buttonPressed = false;

void setup() {
  Serial.begin(115200); // For serial monitor
  A9GSerial.begin(9600); // A9G communication
  pinMode(buttonPin, INPUT_PULLUP); // Button as input with internal pull-up resistor
  pinMode(D5, OUTPUT); // PWRKEY pin as OUTPUT
  digitalWrite(D5, LOW); // Power off A9G initially
  delay(20000);
  

  digitalWrite(SLEEP_PIN, LOW); // Sleep Mode OFF

  A9GSerial.println("AT");               // Just Checking
  delay(1000);

  A9GSerial.println("AT+GPS = 1");      // Turning ON GPS
  delay(1000);

  A9GSerial.println("AT+GPSLP = 2");      // GPS low power
  delay(1000);

  A9GSerial.println("AT+SLEEP = 1");    // Configuring Sleep Mode to 1
  delay(1000);

  A9GSerial.println("AT+CMGF = 1");
  delay(1000);

  A9GSerial.println("AT+CSMP  = 17,167,0,0 ");
  delay(1000);

  A9GSerial.println("AT+CPMS = \"SM\",\"ME\",\"SM\" ");
  delay(1000);
  
  digitalWrite(SLEEP_PIN, HIGH); // Sleep Mode ON


}

void Get_gmap_link(bool makeCall)
{


  digitalWrite(SLEEP_PIN, LOW);
  delay(1000);
  A9GSerial.println("AT+LOCATION = 2");
  Serial.println("AT+LOCATION = 2");

  while (!A9GSerial.available());
  while (A9GSerial.available())
  {
    char add = A9GSerial.read();
    res = res + add;
    delay(1);
  }

  res = res.substring(17, 38);
  response = &res[0];

  Serial.print("Recevied Data - "); Serial.println(response); // printin the String in lower character form
  Serial.println("\n");

  if (strstr(response, "GPS NOT"))
  {
    Serial.println("No Location data");
    //------------------------------------- Sending SMS without any location
    A9GSerial.println("AT+CMGF=1");
    delay(1000);
    A9GSerial.println("AT+CMGS=\"" + SOS_NUM + "\"\r");
    delay(1000);

    A9GSerial.println ("Unable to fetch location. Please try again");
    delay(1000);
    A9GSerial.println((char)26);
    delay(1000);
  }
  else
  {

    int i = 0;
    while (response[i] != ',')
      i++;

    String location = (String)response;
    String lat = location.substring(2, i);
    String longi = location.substring(i + 1);
    Serial.println(lat);
    Serial.println(longi);

    String Gmaps_link = ( "http://maps.google.com/maps?q=" + lat + "+" + longi); //http://maps.google.com/maps?q=38.9419+-78.3020
    //------------------------------------- Sending SMS with Google Maps Link with our Location
    A9GSerial.println("AT+CMGF=1");
    delay(1000);
    A9GSerial.println("AT+CMGS=\"" + SOS_NUM + "\"\r");
    delay(1000);

    A9GSerial.println ("I'm here " + Gmaps_link);
    delay(1000);
    A9GSerial.println((char)26);
    delay(1000);

    A9GSerial.println("AT+CMGD=1,4"); // delete stored SMS to save memory
    delay(5000);
  }
  response = "";
  res = "";
  if (makeCall)
  {
    Serial.println("Calling Now");
    A9GSerial.println("ATD" + SOS_NUM);
    CALL_END = 0;
  }
}

void loop()
{
  //Check if the button is pressed
  if (digitalRead(buttonPin) == LOW && !buttonPressed) {
    buttonPressed = true;
    Get_gmap_link(0);
  } else if (digitalRead(buttonPin) == HIGH) {
    buttonPressed = false;
  }
  
  // Check for A9G responses
  while (A9GSerial.available()) {
    Serial.write(A9GSerial.read());
  }

  // Your code here - process A9G responses and location data
  {
    //listen from GSM Module
    if (A9GSerial.available())
    {
      char inChar = A9GSerial.read();

      if (inChar == '\n') {

        //check the state
        if (fromGSM == "SEND LOCATION\r")
        {
          Get_gmap_link(0);  // Send Location without Call
          digitalWrite(SLEEP_PIN, HIGH);// Sleep Mode ON
          
        }

        else if (fromGSM == "RING\r")
        {
          digitalWrite(SLEEP_PIN, LOW); // Sleep Mode OFF
          Serial.println("---------ITS RINGING-------");
          A9GSerial.println("ATA"); //Ans the call
        }

        else if (fromGSM == "NO CARRIER\r")
        {
          Serial.println("---------CALL ENDS-------");
          CALL_END = 1;
          digitalWrite(SLEEP_PIN, HIGH);// Sleep Mode ON
        }

        //write the actual response
        Serial.println(fromGSM);
        //clear the buffer
        fromGSM = "";

      }
      else
      {
        fromGSM += inChar;
      }
      delay(20);
    }

    // read from port 0, send to port 1:
    if (Serial.available()) {
      int inByte = Serial.read();
      A9GSerial.write(inByte);
    }
 }
}

// void sendLocationRequest() {
//   // Power on the A9G module
//   digitalWrite(D5, HIGH); // Power on A9G
//   delay(1000);

//   // Send AT command to request location
//   A9GSerial.println("AT+QCELLLOC=2");
//   delay(100);

//   // Power off the A9G module
//   digitalWrite(D5, LOW); // Power off A9G
// }


