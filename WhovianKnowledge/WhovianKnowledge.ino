// This #include statement was automatically added by the Spark IDE.
#include "LiquidCrystal/LiquidCrystal.h"


//Wikipedia relevant stuff
#define WIKIURL "en.wikipedia.org" // name address for wikipedia (using DNS)
#define START_YEAR 500

//reading tcp info relevant stuff
#define BUFFER_SIZE_MAX 4096  
#define SKIP_CHARS 300 //do not read first 256 characters of reply.
#define HOST_DELAY 5000 //nr of milliseconds we do nothing after connection is established to give host some time.
#define HOST_TIMEOUT 10000 //nr of milliseconds we will wait if host does not close connection
#define ADCMAXVAL 4095 //maximum value of ADC

#define DEBUG_SERIAL true

//inputs:
#define INPINSTARTALL D6
#define INPINMONTH A0
#define INPINDAY A1


//outputs:
#define OUTPINSOUNDSTART D7
#define OUTPINLCD0 D0
#define OUTPINLCD1 D1
#define OUTPINLCD2 D2
#define OUTPINLCD3 D3
#define OUTPINLCD4 D4
#define OUTPINLCD5 D5

TCPClient client;

LiquidCrystal *lcd;

const String monthLookUp[12]={"January","February","March","April","May","June","July","August","October","November","December"};

void setup() {
    
    //pin to start TARDIS sound
    pinMode(OUTPINSOUNDSTART,OUTPUT);
    digitalWrite(OUTPINSOUNDSTART,LOW);
    
    //pins to read day and month
    pinMode(INPINMONTH,INPUT);
    pinMode(INPINDAY,INPUT);
    
    //pin to read "go" button
    pinMode(INPINSTARTALL,INPUT);
    
    //start serial communication with EMIC2 unit on RX-TX pins
    Serial1.begin(9600); //this is the Serial communiction over rx & tx pins. the emic2 is connected to those pins
    Serial1.print('\n');             // Send a CR in case the system is already up
    while (Serial1.read() != ':');   // When the Emic 2 has initialized and is ready, it will send a single ':' character, so wait here until we receive it
    delay(10);                          // Short delay
    Serial1.flush();                 // Flush the receive buffer

    //start LCD screen
    lcd = new LiquidCrystal(OUTPINLCD0, OUTPINLCD1, OUTPINLCD2, OUTPINLCD3, OUTPINLCD4, OUTPINLCD5); //Make sure to update these to match how you've wired your pins
    lcd->begin(16, 2); // set up the LCD's number of columns and rows: 
    lcd->print("Hello, Sparky!"); // Print a message to the LCD.
    
    //start serial communication over USB for debugging
    if (DEBUG_SERIAL) Serial.begin(9600); //if debugging, give some time to open connection.
    if (DEBUG_SERIAL) delay(15000); //if debugging, give some time to open connection.
    if (DEBUG_SERIAL) Serial.println("Welcome!");
    
    //wait 5 seconds to allow all sub-sytems to come online.
    delay(5000);

}

void loop() {
    
    if (DEBUG_SERIAL) Serial.println("Let's do this");

    int month=map(analogRead(INPINMONTH),0,ADCMAXVAL,1,12);
    int day=map(analogRead(INPINDAY),0,ADCMAXVAL,1,31);
    String targetDate = monthLookUp[month-1] + "_" + String(day);
    
    

    lcd->clear();
    lcd->setCursor(0,0);
    lcd->print(monthLookUp[month-1] + " " + String(day));
    lcd->setCursor(0,1);
    //lcd->print("zoeken we op");


    

    //make an API-call to wikiPedia to get a single fact from a single day
    if (digitalRead(INPINSTARTALL)==HIGH){
        msgFromIW(targetDate);
        delay(2000);
    }
    
    delay(100); //to not make LCD go crazy!    

}

String thisDayInHistory(String dateStamp){
    char buffer[BUFFER_SIZE_MAX];
    int i = 0;
    int k = 0;
    int j = 0;
    bool printOnce = false;
    
    String response = "";
    String responseBuffer = "0123456789012";
    String searchString = "== Events ==\n";
    char server[] = WIKIURL;
    boolean lookingForEvents = true;
    
    //choose a random year to draw a fact from.
    int focusYear = (int) random(START_YEAR,2014);
    int prevYear = START_YEAR;
    
    if (DEBUG_SERIAL) Serial.println("\nFocusYear is: " + (String) focusYear);  

  
    //Connect to server: wikipedia.
    if (DEBUG_SERIAL) Serial.println("\nStarting connection to server...");
    // if you get a connection, report back via serial:
    if (client.connect(server, 80)) {
        if (DEBUG_SERIAL) Serial.println("connected to server");
        // Make a HTTP request:
        client.println("GET /w/api.php?action=query&prop=extracts&exchars=" + (String) BUFFER_SIZE_MAX + "&titles=" + dateStamp + "&explaintext&format=txt");
        //    client.println("GET /wiki/December_21.html");
        client.println("Host: en.wikipedia.org");
        client.println("Connection: close");
        client.println();
    } 
    else {
        if(DEBUG_SERIAL) Serial.println("\r\n\r\nConnection Failed!");
        client.flush();
        client.stop();
        response += "there was a problem reaching wikipedia";
        return (String) response;      
    }
    
    
    // wait HOST_DELAY seconds or less for the host to respond
    uint32_t startTime = millis();
    while(!client.available() && (millis() - startTime) < HOST_DELAY);

    if(DEBUG_SERIAL) Serial.println("\r\n\r\nREADING HOST DATA......");
    
    uint32_t lastRead = millis();
    // If the host doesn't close it's connection, we'll timeout in HOST_TIMEOUT seconds.
    while (client.connected() && (millis() - lastRead) < HOST_TIMEOUT) {
        while (client.available()) {
            char c = client.read();
            if(c == -1) {
                Serial.print("\r\n\r\nERROR......\r\n\r\n");
                client.flush();
                client.stop();
            }
            if(j++ >= SKIP_CHARS) { // don't buffer the first X bytes to save memory
                if(DEBUG_SERIAL && !printOnce) {
                    Serial.print("\r\n\r\nSAVING......\r\n\r\n");
                    printOnce = true;
                }
                if (lookingForEvents) {
                    responseBuffer=responseBuffer.substring(1) + c;
                    if (responseBuffer.equals(searchString)){
                        lookingForEvents = false;
                        if (DEBUG_SERIAL) Serial.println("\r\n\r\nEVENTS FOUND......\r\n\r\n");
                    }
                }
                else {
                    //add character to string response
                    k++;
                    buffer[i++]=c;
                    
                    if (c=='\n'){
                        buffer[i++]='\0';
                        response.remove(0);
                        response += buffer;
                        response.trim();
                        if (DEBUG_SERIAL) Serial.println("enter found. response so far: " + response);
                        
                        int location = response.indexOf('–');
                        if (DEBUG_SERIAL) Serial.println("found seperator at location: " + (String) location);
                        
                        int year = response.substring(0,location-1).toInt();
                        String fact = response.substring(location+1);
                        if (DEBUG_SERIAL) Serial.println(year);
                        if (DEBUG_SERIAL) Serial.println(response);
                        
                        if ((focusYear >= prevYear) && (year>=focusYear)){
                            if (DEBUG_SERIAL) Serial.println("response processed. disconnecting from server.");
                            client.flush();
                            client.stop();
                            String date2Say=dateStamp;
                            date2Say.replace("_"," ");
                            return (String) "Did you know that on " + date2Say + " in " + (String) year + fact;
                        } 
                        else {
                            response.remove(0);
                            prevYear = year;
                            for (int n=i ; j<BUFFER_SIZE_MAX; j++){
                                buffer[n]='\0';
                            }
                            i=0;
                        }
                    }
                    
                    
                    if((i >= BUFFER_SIZE_MAX)||(k >= BUFFER_SIZE_MAX)) { // if we reach the end of our buffer, just bail.
                        if(DEBUG_SERIAL) Serial.print("\r\n\r\nOUT OF BUFFER SPACE......\r\n\r\n");
                        client.flush();
                        client.stop();
                    }
                    
                }
                if(DEBUG_SERIAL) Serial.print(c);
            }
            delayMicroseconds(150);
            // as long as we're reading data, reset the lastRead time.
            lastRead = millis();
        } // END while(client.available())
    } // END while(client.connected())
    
    //to be sure, kill the client
    client.flush();
    client.stop();
    
    //we now have wikipedia's response in our buffer. parse this to select a random year
    
    return (String) response;

}

int drSays(String response){
    Serial1.print("N0\n");
    Serial1.print("W100\n");
    Serial1.print("V12\n");
    Serial1.print('S');
    Serial1.print(response);  // Send the desired string to convert to speech
    Serial1.print('\n');
    while (Serial1.read() != ':');   // Wait here until the Emic 2 responds with a ":" indicating it's ready to accept the next command
    return 1;
}

int msgFromIW(String targetDate){
    digitalWrite(OUTPINSOUNDSTART,HIGH);
    delay(30);
    digitalWrite(OUTPINSOUNDSTART,LOW);
    uint32_t startSoundTime = millis();
    Spark.disconnect();
    String response = thisDayInHistory(targetDate);
    Spark.connect();

    //wait for the TARDIS sound to finish.
    while ((millis()-startSoundTime)<15000);
    
    //TODO open the TARDIS door
    
    //send the received response to the emic2
    drSays(response);
    return 1;
}

