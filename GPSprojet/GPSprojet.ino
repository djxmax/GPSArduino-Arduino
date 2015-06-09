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
float lat,lon, height;
char * point="";
unsigned long time, date;
int BPENState, BP1State, BP0State, choixAffichage;
boolean newdata = false;
boolean enregistrement = false ;
boolean etatMenu=false;


boolean feedgps(); //Vérifie l'arrivée et l'encodage des données GPS
void selectButton(); //Permet à l'utilisateur d'interagir avec les boutons du boitier
void GPSreader(); //Récupère les données GPS et les stocke dans des variables
void affichageGPS(); //Affiche à l'écran les données GPS en temps réel
void affichEcran(char *,char *);

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
  choixAffichage=0;
  
  //Affichage de la batterie sur le lcd au démarrage
  lcd.begin(8,2);
  lcd.setCursor(0,1);
  lcd.print("Bat= ");
  int valbat = analogRead(0); //récupération de la valeur de la batterie (analogique) sur la broche 0
  float V = (float) valbat * 6.2 / 1024.0; //conversion analogique/numérique
  lcd.print(V,4); 
  delay(4000);
  
  newdata = false;
  
  pinMode(10, OUTPUT); 
  if (!SD.begin(4)) {
      affichEcran(" ERROR","INIT SD");
      return;
  }else {
  Serial.println("INIT OK");
  
  //----- affiche le contenu du répertoire 

  myDir = SD.open("/"); // ouvre la SD Card à la racine

  afficheContenu(myDir, 1); // affiche contenu d'un répertoire avec 0 tab 

  Serial.println("Operation Terminee!");
  }
 
  
  //Creation du fichier .txt
  theFile = SD.open("point.txt", FILE_WRITE);
  theFile.close(); //Le fichier doit être fermé après sa création
  
  lcd.begin(8,2);
  lcd.clear();  
  lcd.setCursor(0,0);
  lcd.print("Make a");
  lcd.setCursor(0,1);
  lcd.print("choice..");
  delay(2000);
  
  //ecritureSD();
  //lectureSD();
}

void loop(){
  
  selectButton();
  GPSreader();

}

void selectButton(){
  //On récupère les valeurs logiques...
  delay(100);
  BPENState = digitalRead(BPEN);
  BP1State = digitalRead(BP1);
  BP0State = digitalRead(BP0);
  delay(100);
  //Les delay permettent de récupérer les valeurs lors d'une même action, il n'y a pas de risque de retard d'un des signaux
  
  if(BPENState == HIGH && BP1State == LOW && BP0State == LOW){ 
  //Bouton 1 =>Enregistrement données 
  
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("OK");
      lcd.setCursor(0,1);
      lcd.print("1");
                  
      if(enregistrement==false){
        enregistrement=true;
        affichEcran("  SAVE","   ON");
        delay(1000);        
        Serial.println("Début enregistrement ...\n"); //Premier flag de début pour l'app android
      }else if (enregistrement==true){
        enregistrement=false;
        affichEcran("  SAVE","  OFF");
        delay(1000);
        Serial.println("Fin enregistrement\n"); // Dernier flag de fin pour l'app Android
      }
  
  }else if(BPENState == HIGH && BP1State == LOW && BP0State == HIGH) { 
  //Bouton 2 =>choix environnement ( arbre , batiment, lampadaire et Transport en commun ... )
      
      etatMenu=true;
      lcd.clear(); // On réinitialise l'écran
      
      while(etatMenu==true){
        
        //Faire choix boutons multi
        
        lcd.setCursor(0,0);
        lcd.print("1-A 2-Bt");
        lcd.setCursor(0,1);
        lcd.print("3-L 4-TC");
        
        delay(100);
        BPENState = digitalRead(BPEN);
        BP1State = digitalRead(BP1);
        BP0State = digitalRead(BP0);
        delay(100);
        
        if(BPENState == HIGH && BP1State == LOW && BP0State == LOW){
          point=",Tree";
          affichEcran("Choice :","  Tree");
          etatMenu=false;
        }else if(BPENState == HIGH && BP1State == LOW && BP0State == HIGH) {
          point=",Buld";
          affichEcran("Choice :","Building");         
          etatMenu=false;
        }else if (BPENState == HIGH && BP1State == HIGH && BP0State == LOW) {
          point=",FLamp";
          affichEcran("Choice :","Flo-lamp");          
          etatMenu=false;
        }else if (BPENState == HIGH && BP1State == HIGH && BP0State == HIGH) {
          point=",PT";
          affichEcran("Choice :","Public T");  
          etatMenu=false;
        }
      }
      
      
  
  }else if (BPENState == HIGH && BP1State == HIGH && BP0State == LOW) { 
  //Bouton 3 => BONUS
      choixAffichage++;
      typeAffichage();
      if(choixAffichage==4){
        choixAffichage=0;
      }
      
  
  }else if (BPENState == HIGH && BP1State == HIGH && BP0State == HIGH) { 
  //Bouton 4 => Televerser
      
      if(enregistrement){
        affichEcran("  SAVE","STILL ON");
      }else{
        affichEcran(" Upload","  Data");     
        lectureSD();
      }
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
    height=gps.f_altitude(); //On enregistre l'altitude du point en mètres .
    
    if(enregistrement){    
    ecritureSD();        
    }
    
    //lectureSD();
    
    if(etatMenu==false){
    typeAffichage(); //Affichage sur l'écran lcd des coordonées GPS
    }
    
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

void typeAffichage(){
  
  if(choixAffichage==0){
  //affichage ecran coordonées GPS
  lcd.clear();
  lcd.setCursor(0,0); 
  lcd.print(lat,9); 
  lcd.setCursor(0,1); 
  lcd.print(lon,9);
  // Permet de verifier coordonnées sur le port série
  //  Serial.print(lat,DEC);
  //  Serial.print(",");
  //  Serial.print(lon,DEC);
  //  Serial.print(";\n");
  }else if (choixAffichage==1){
  //affichage ecran altitude en mètres
  lcd.clear();
  lcd.setCursor(0,0); 
  lcd.print("HEIGHT/m"); 
  lcd.setCursor(0,1); 
  lcd.print(height);
  }else if (choixAffichage==2){
  //affichage ecran date et heure au format UTC
  lcd.clear();
  lcd.setCursor(0,0); 
  lcd.print(date); 
  lcd.setCursor(0,1); 
  lcd.print(time);
  }else if (choixAffichage==3){
  //affichage ecran batterie du module GPS
  lcd.clear();  
  lcd.setCursor(0,1);
  lcd.print("Bat= ");
  int valbat = analogRead(0); //récupération de la valeur de la batterie (analogique) sur la broche 0
  float V = (float) valbat * 6.2 / 1024.0; //conversion analogique/numérique
  lcd.print(V,4);    
  }
  
}

void affichEcran(char * up, char * down){
  //affichage ecran
  lcd.clear();
  lcd.setCursor(0,0); 
  lcd.print(up); 
  lcd.setCursor(0,1); 
  lcd.print(down);
}

void ecritureSD(){
 
  theFile = SD.open("point.txt", FILE_WRITE); // ouverture de fichier.txt en écriture
  
  if (theFile) {
      
      theFile.print(lat);
      theFile.print(",");
      theFile.print(lon);
      theFile.print(",");
      theFile.print(height);
      theFile.print(",");
      theFile.print(date);
      theFile.print(",");
      theFile.print(time);
      theFile.print(point);
      theFile.print(";");
      theFile.print("\n");
      
      point=""; // On réinitialise le point .
      // Fermeture du fichier:
      theFile.close();
      //Serial.println("C'est écrit !");
    } else {
      // impossible d'ouvrir/créer le fichier:
      Serial.println("Erreur d'ouverture du fichier");
  }
}

void lectureSD(){
  theFile = SD.open("point.txt");
  if (theFile) {
    Serial.print("Latitude,Longitude,Altitude,Date,Heure,Point;\n");
    // lecture du fichier jusqu'à la fin:
    while (theFile.available()) {
    Serial.write(theFile.read());
  }
  // Fermeture du fichier:
  theFile.close();
  SD.remove("point.txt");
  } 
  else {
  // Ouverture impossible:
  affichEcran(" need a"," record");
  }
}

boolean feedgps(){ //Vérifie l'arrivée et l'encodage des données GPS
  while (uart_gps.available()){
    if (gps.encode(uart_gps.read()))
      return true;
  }
  return false;
}
