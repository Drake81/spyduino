#include <TimerOne.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(2,3); // RX, TX
#define esp mySerial
#define dbg Serial

#define TASTER1 A0
#define TASTER2 A1
#define LED1 A4
#define LED2 A5
#define LED3 4
volatile int state = LOW;


#define WIFI_ON 1
#define WIFI_OFF 0
int WIFI_Status = WIFI_OFF;
#define SSID  "SSID"     // WIFI SSID
#define PASS  "PSSWORD"     // WIFI Passwort
#define IP "0.0.0.0."
#define PORT 1337 


#define BUFFER_SIZE 512
char OKRN[] = "OK\r\n";
char ERRORRN[] = "ERROR\r\n";
char CLOSEDRN[] = "CLOSED";
char LINKRN[] = "Linked\r\n";
char STATUSRN[] = "STATUS:3\r\n";
#define OK 0
#define LINK -1
#define STATUS -2
#define ERROR 1
#define CLOSED 2


// -- DEBUG ON
//#define DEBUG

//------------- Declare functions
int WIFI_Setup();
int wait_for_esp_response(int);
bool connect_server();
bool disconnect_server();
bool send_data();
void Button1();




//------------- Basic Hardware Setup
void
setup()
{
#ifdef DEBUG
    dbg.begin(9600);
    dbg.setTimeout(20000);
#endif
    dbg.println("\nDBG +++ SETUP ROUTINE +++");
    pinMode(LED1,OUTPUT);
    digitalWrite(LED1,LOW);
    pinMode(LED2,OUTPUT);
    digitalWrite(LED2,HIGH);
    pinMode(LED3,OUTPUT);
    digitalWrite(LED3,HIGH);

    Timer1.initialize(1000000);
    Timer1.attachInterrupt(check_button1);
    Timer1.stop();
    Timer1.start();

    pinMode(TASTER1,INPUT);
    pinMode(TASTER2,INPUT);
    digitalWrite(TASTER1,HIGH);
    digitalWrite(TASTER2,HIGH);

    dbg.println("\nDBG +++ ENTER LOOP ROUTINE +++");
}



//------------- Main Loop
void
loop()
{
    if(!WIFI_Status & state)
    {
        esp.begin(9600);
        esp.setTimeout(20000);
        WIFI_Status = WIFI_Setup();
        while (!WIFI_Status)
        {
            dbg.println("DBG WIFI SETUP FAILED");
            WIFI_Status = WIFI_Setup();
        }
        dbg.println("DBG WIFI ON");
        digitalWrite(LED3,LOW);
    }

    if(WIFI_Status && !state)
    {
        esp.println("AT+CWQAP");
        esp.end();
        WIFI_Status = WIFI_OFF;
        digitalWrite(LED3,HIGH);
        dbg.println("DBG WIFI OFF");
    }

    if(WIFI_Status && state)
    {
        if(connect_server())
        {
            while(test_server() && state)
            {
                digitalWrite(LED2,LOW);
                send_data();
            }
            digitalWrite(LED2,HIGH);
            disconnect_server();
        }
        delay(1000);
    }
}




//--------------------------------------------------------------------------
//------------------------ Functions ---------------------------------------
//--------------------------------------------------------------------------

    int
WIFI_Setup()
{
    dbg.println("DBG +++ WIFI SETUP +++");
    
    dbg.println("DBG SET CWMODE=1");
    esp.println("AT+CWMODE=1");
    wait_for_esp_response(5000);

    dbg.println("DBG CONNECT WIFI");
    esp.print("AT+CWJAP=\"");
    esp.print(SSID);
    esp.print("\",\"");
    esp.print(PASS);
    esp.println("\"");

    if(wait_for_esp_response(10000) == ERROR)
    {
        return WIFI_OFF;
    }

    /* dbg.println("DBG SHOW IP"); */
    /* esp.println("AT+CIFSR"); */
    /* wait_for_esp_response(1000); */
    return WIFI_ON;
}



bool
connect_server()
{
    dbg.println("DBG +++ CONNECT TO SERVER +++");

    esp.print("AT+CIPSTART=\"TCP\",\"");
    esp.print(IP);
    esp.print("\",");
    esp.println(PORT);
    int ret = wait_for_esp_response(10000);
    // Check ok
    if(ret == OK)
    {
        // Check LInked
        if(wait_for_esp_response(10000) == LINK)
        {
            dbg.println("DBG SERVER CONNECTED");
            return true;
        }
    }
    dbg.println("DBG SERVER CONNECT FAILED");
    return false;
}



bool
disconnect_server()
{
    dbg.println("DBG DISCONNECT SERVER");
    esp.println("AT+CIPCLOSE");
    return wait_for_esp_response(5000);
}

bool
test_server()
{
    esp.println("AT+CIPSTATUS");
    if(wait_for_esp_response(5000) == STATUS) {
        if(wait_for_esp_response(1000) == OK) {
            dbg.println("DBG SERVER STILL CONNECTED");
            return true;
        }
    }
    return false;
}

bool
send_data()
{
    dbg.println("DBG SEND DATA");
    esp.println("AT+CWLAP");
    String buffer = esp.readString();
    int len = buffer.length()-1;
    dbg.println(len);
    esp.print("AT+CIPSEND=");
    esp.println(len);
    esp.println(buffer);
    wait_for_esp_response(30000);
    dbg.println("DBG SEND ACK");
}



void
check_button1()
{
    if(digitalRead(TASTER1))
    {
        if(state == HIGH)
        {
            state = LOW;
        }
        else
        {
            state = HIGH;
        }    
        digitalWrite(LED1,state);
    }
}



int wait_for_esp_response(int timeout)
{
    char buffer[BUFFER_SIZE] = {0};
    unsigned long t=millis();
    int found=ERROR;
    int i=0;
    int lenO=strlen(OKRN);
    int lenE=strlen(ERRORRN);
    int lenC=strlen(CLOSEDRN);
    int lenL=strlen(LINKRN);
    int lenS=strlen(STATUSRN);
    // wait for at most timeout milliseconds
    // or if OK\r\n is found
    while(millis()<t+timeout) {
        if(esp.available()) {
            buffer[i++]=esp.read();
            if(i>=lenO) {
                if(strncmp(buffer+i-lenO, OKRN, lenO)==0)
                {
                    found=OK;
                    dbg.println("DBG OK");
                    break;
                }
            }
            if(i>=lenE) {
                if(strncmp(buffer+i-lenE, ERRORRN, lenE)==0)
                {
                    found=ERROR;
                    dbg.println("DBG ERROR");
                    break;
                }
            }
            if(i>=lenC) {
                if(strncmp(buffer+i-lenC, CLOSEDRN, lenC)==0)
                {
                    found=CLOSED;
                    dbg.println("DBG CLOSED");
                    break;
                }
            }
            if(i>=lenL) {
                if(strncmp(buffer+i-lenL, LINKRN, lenL)==0)
                {
                    found=LINK;
                    dbg.println("DBG LINK OK");
                    break;
                }
            }
            if(i>=lenS) {
                if(strncmp(buffer+i-lenS, STATUSRN, lenS)==0)
                {
                    found=STATUS;
                    dbg.println("DBG STATUS OK");
                    break;
                }
            }
        }
    }
    buffer[i]=0;
    dbg.println("+++ BUFFER START +++");
    dbg.println(buffer);
    dbg.println("+++  BUFFER END  +++\n\n\n");
    return found;
}

