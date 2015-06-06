#include <TinyGPS.h>

#include <SD.h>


#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

#include <SPI.h>

#define BPEN 17
#define BP1 15
#define BP0 16

LiquidCrystal lcd(4, 5, 6, 7, 8, 9); //Définition de l'écran lcd
TinyGPS gps; //Définition de l'objet GPS

File myDir; // objet file
File entry;
File theFile; //fichier SD



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
  
  pinMode(10, OUTPUT); 
  if (!SD.begin(10)) {
      Serial.println("Erreur à l'init!");
      return;
  }else {
  Serial.println("init Ok.");
  
  //----- affiche le contenu du répertoire 

  myDir = SD.open("/"); // ouvre la SD Card à la racine

  afficheContenu(myDir, 1); // affiche contenu d'un répertoire avec 0 tab 

  Serial.println("Operation Terminee!");
  }
 
  
  //Creation du fichier .txt
  //theFile = SD.open("fichier.txt", FILE_WRITE);
  //theFile.close(); //Le fichier doit être fermé après sa création
  
  lcd.begin(8,2);
  lcd.clear();  
  lcd.setCursor(0,0);
  lcd.print("Choisir");
  lcd.setCursor(0,1);
  lcd.print("bouton..");  
  
  //ecritureSD();
  //lectureSD();
}

void loop(){
  
  selectButton();
  //GPSreader();

}

void selectButton(){
  //On récupère les valeurs logiques...
  delay(100);
  BPENState = digitalRead(BPEN);
  BP1State = digitalRead(BP1);
  BP0State = digitalRead(BP0);
  delay(100);
  //Les delay permettent de récupérer les valeurs lors d'une même action, il n'y a pas de risque de retard d'un des signaux
  
  if(BPENState == HIGH && BP1State == LOW && BP0State == LOW){ //Bouton 1
  
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("OK");
      lcd.setCursor(0,1);
      lcd.print("1");
  
  }else if(BPENState == HIGH && BP1State == LOW && BP0State == HIGH) { //Bouton 2
  
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("OK");
      lcd.setCursor(0,1);
      lcd.print("2");
  
  }else if (BPENState == HIGH && BP1State == HIGH && BP0State == LOW) { //Bouton 3
      
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("OK");
      lcd.setCursor(0,1);
      lcd.print("3");
  
  }else if (BPENState == HIGH && BP1State == HIGH && BP0State == HIGH) { //Bouton 4
  
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("OK");
      lcd.setCursor(0,1);
      lcd.print("4");
  }
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
    
    
    
    ecritureSD();
    
    affichage(); //Affichage sur l'écran lcd
    id++; //Incrémentation de l'identifiant du point   
  }
  return;
}

void afficheContenu(File dir, int numTabs) { // la fonctin reçoit le rép et le décalage tab

   while(true) { // tant que vrai = crée une "loop" qui séxécute tant que contenu 
   
     // la sortie se fait par break;

     entry =  dir.openNextFile(); // ouvre le fichier ou repertoire suivant

     if (! entry) { // si aucun nouveau fichier /repertoire

       //Serial.println("** pas d'autre fichier ou repertoires**");
       entry.close();
       break; // sort de la fonction

     } // fin si aucun nouveau fichier / répertoire

     // affiche le nombre de tab voulu - 0 si racine, 1 si sous Rép, 2 si sous-sous rép, etc.. 
     for (int i=0; i<numTabs; i++) {
       Serial.print('\t');
     }

     Serial.print(entry.name()); // affiche le nom du fichier/repertoire

     if (entry.isDirectory()) { // si le fichier est un répertoire
       Serial.println("/"); // affiche un slash
       afficheContenu(entry, numTabs+1); // affiche le contenu en décalant d'un tab 
     } // fin si le fichier est un répertoire

     else { // sinon affiche la taille - les fichiers ont une taille, pas les répertoires
       Serial.print("\t\t"); // affiche de Tab de décalé
       Serial.print(entry.size(), DEC); // affiche la taille
       Serial.println(" octets"); // affiche la taille

     } // fin sinon = si pas un rép

   } // fin while(true)
   entry.close(); 

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

void ecritureSD(){
 
  theFile = SD.open("test.txt", FILE_WRITE); // ouverture de fichier.txt en écriture
  
  if (theFile) {
      Serial.print("Ecriture de données sur la premiere ligne");
      theFile.print(lat);
      theFile.print(",");
      theFile.print(lon);
      theFile.print(",");
      theFile.print(date);
      theFile.print(",");
      theFile.print(time);
      theFile.print("\n");
   // Fermeture du fichier:
      theFile.close();
      Serial.println("C'est écrit !");
    } else {
      // impossible d'ouvrir/créer le fichier:
      Serial.println("Erreur d'ouverture de fichier.txt");
  }
}

void lectureSD(){
  theFile = SD.open("test.txt");
  if (theFile) {
    Serial.println("test.txt:");
    // lecture du fichier jusqu'à la fin:
    while (theFile.available()) {
    Serial.write(theFile.read());
  }
  // Fermeture du fichier:
  theFile.close();
  } 
  else {
  // Ouverture impossible:
  Serial.println("Ouverture impossible de fichier.txt");
  }
}

boolean feedgps(){ //Vérifie l'arrivée et l'encodage des données GPS
  while (uart_gps.available()){
    if (gps.encode(uart_gps.read()))
      return true;
  }
  return false;
}
