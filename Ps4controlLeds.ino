#include <PS4Controller.h>

int   LLeft = 15;
int   LUp = 16;
int   LRight = 17;
int   MLeft = 18;
int   MRight = 19;
int   MUp = 21;


void notify()
{
  digitalWrite(LLeft, PS4.Left());
  digitalWrite(LUp, PS4.L1());
  digitalWrite(LRight, PS4.Right());

  digitalWrite(MLeft, PS4.Square());
  digitalWrite(MRight, PS4.Circle());
  digitalWrite(MUp, PS4.R1());
}

void onConnect()
{
  Serial.println("Connected!.");
}

void onDisConnect()
{
  Serial.println("Disconnected!.");    
}

void setUpPinModes()
{
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);      

  pinMode(led5, OUTPUT);
  pinMode(led6, OUTPUT); 
}

void setup() 
{
  setUpPinModes();
  Serial.begin(115200);
  PS4.attach(notify);
  PS4.attachOnConnect(onConnect);
  PS4.attachOnDisconnect(onDisConnect);
  PS4.begin("80:c5:f2:a6:bb:5a");
  Serial.println("Ready.");
}

void loop() 
{

}
