#include <TinyGPS.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#include <SD.h>
#include <SPI.h>

#define BPEN 17
#define BP1 15
#define BP0 16

LiquidCrystal lcd(4, 5, 6, 7, 8, 9); //Définition de l'écran lcd
TinyGPS gps; //Définition de l'objet GPS
File myFile; //Définition du fichier
SoftwareSerial uart_gps(3,2); //Définition du module GPS

//Nous utiliserons des variables globales...
float lat,lon;
unsigned long time, date, id;
int BPENState, BP1State, BP0State, type, choixAffichage, etat;
boolean newdata = false;

boolean feedgps();

void setup(){
  lcd.clear();
  Serial.begin(9600);//configuration du port série
  uart_gps.begin(4800); //configuration du gps
  
  //Définition des bouttons
  pinMode(BPEN,INPUT);
  pinMode(BP1,INPUT);
  pinMode(BP0,INPUT);
  
  //Initialisation des variables
  BPENState = 0;
  BP1State = 0;
  BP0State = 0;
  choixAffichage = 0;
  newdata = false;
  etat = 0;
  id = 0;
  
  lcd.begin(8,2);
  lcd.clear();  
  lcd.setCursor(0,0);
  lcd.print("Press a");
  lcd.setCursor(0,1);
  lcd.print("button..");  
}

void loop(){
  GPSreader();

}

void GPSreader(){ //Récupère les données GPS et les stocke dans des variables
  unsigned long start = millis();
  while (millis() - start < 1000){ //Permet de récupérer les données à intervalle d'une seconde environ
   if (feedgps()) //si le GPS reçoit des données et qu'il les encode bien...
      newdata = true;
  }
  if (newdata){  //Si on a récupéré de nouvelles données...
    gps.f_get_position(&lat, &lon); //on enregistre les latitudes et longitudes   
    gps.get_datetime(&date, &time); //la date et l'heure d'enregistrement du point (format UTC)
    
    affichage(); //Affichage sur l'écran lcd
    id++; //Incrémentation de l'identifiant du point   
  }
  return;
}

void affichage(){
  //affichage ecran
  lcd.clear();
  lcd.setCursor(0,0); 
  lcd.print(lat,9); 
  lcd.setCursor(0,1); 
  lcd.print(lon,9);
  
  //envoie serial
  Serial.print(lat,DEC);
  Serial.print(",");
  Serial.print(lon,DEC);
  Serial.print(";\n");
  
}


boolean feedgps(){ //Vérifie l'arrivée et l'encodage des données GPS
  while (uart_gps.available()){
    if (gps.encode(uart_gps.read()))
      return true;
  }
  return false;
}
