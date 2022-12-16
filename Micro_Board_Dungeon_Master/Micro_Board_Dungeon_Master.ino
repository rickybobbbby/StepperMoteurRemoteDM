/*
  Yannick Langlois
  Tutorial original https://RandomNerdTutorials.com/stepper-motor-esp32-websocket/

  Objectif: faire fonctionner par websocket un Stepper Motor en remote sur le meme wifi
  Concept de project: Faire fonctionner un jeu de table simple qui utile le motor come dés
  Dés possible: 1 dés 20, 1 dés 8, 1 dés 6, 1 dés 4.

  travail comprend deux section code: le site present dans data et le scipt Arduino.

  inclus aussi un debug menu qui suis la structure initial du projet proposer par le tutorial.
*/
/* Library utiliser */
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include <Stepper.h>


const int stepsPerRevolution = 2048;  // changer cela pour adapter le nombre de révolution requise par le motor pour faire un 360(defaut:2048)
#define IN1 19 //motor input pins
#define IN2 18 //motor input pins
#define IN3 5 //motor input pins
#define IN4 15 //motor input pins
Stepper myStepper(stepsPerRevolution, IN1, IN3, IN2, IN4); //set de l'ordre de revolution des pin du moteur

// ------- Network Log -------
// Replacer par vos network credentials du wifi local
//const char* ssid = "REPLACE_WITH_YOUR_SSID";
//const char* password = "REPLACE_WITH_YOUR_PASSWORD";

const char* ssid = "Yannick96"; //pour les teste personnel
const char* password = "1234yann"; //pour les teste personnel

//const char* ssid = "reseau2"; // pour l'exemple sur le wifi en class

// ------- AsyncWebServer object -------
// Crée un AsyncWebServer objet au port 80
AsyncWebServer server(80);

// Crée un WebSocket objet
AsyncWebSocket ws("/ws");

// ------- VARIABLES -------
//Les variable utiliser pour sauvée les valeurs du HTML form
String message = ""; //pessage represent le contact premiers entre se script et le site. 
String direction ="Start"; //variable de direction, seulement utile pour le debug menu
String steps; //variable du nombre de steps demander (EX: 2048), seulement utile pour le debug menu

String actionType ="None"; //variable du type d'action demander par le remote (dans cette exemple, cela permet d'entreposé les tag des dés)
String DiceSteps; //variable est la variable qui represent le nombre de step demander au moteur apres calcule deprendant du dés.
int DiceSection; //variable qui est modifié dependant du tag de dés demander (permet de sectionner le tableau de jeu et s'assuré que les meme resultat revienne les meme movement)
int r; //variable conteneur au calcule de randomisation plus tard dans le code.

bool newRequest = false; //variable activé si un request est en cours pour eviter plusieurs requete pendant le movement.

// ------- Initialize SPIFFS -------
// script d'initialization des fonction de la librerie spiffs qui serve a entreposer notre site (site present dans data).
void initSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  else{
    Serial.println("SPIFFS mounted successfully");
  }
}

// ------- Initialize WiFi -------
// initialisation du wifi
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password); //si le wifi a un mot de passe set
  //WiFi.begin(ssid); //si le wifi n'a pas de mot de passe
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP()); // on retourn le IP du site. peu etre ouvert sur tous les apareil du reseau.
}

// ------- Notify All Clients -------
//script utiliser pour envoyer des retour d'information par notre websocket jusqu'au client.
void notifyClients(String state) { 
  ws.textAll(state); // a noté que la fonction textAll() provien de la librerie AsyncWebSocket
}

// ------- handle WebSocket Message -------
// Une fonction de rappel qui s'exécute chaque fois que nous recevons de nouvelles données des clients via le protocole WebSocket.
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    message = (char*)data; // une fois notre message adapté au bon forma data

    //on divise chaque variables a l'aide d'index par mot clés present dans le js sous la fonction submit
    steps = message.substring(message.indexOf("Steps")+5, message.indexOf("ActionType")); 
    actionType = message.substring(message.indexOf("ActionType")+10, message.indexOf("Direction"));
    direction = message.substring(message.indexOf("Direction")+9, message.length());

    //on appel en console tous les variable (a utiliser pour le debug)
    Serial.print("steps");
    Serial.println(steps);
    
    Serial.print("Action Type");
    Serial.println(actionType);
    
    Serial.print("direction");
    Serial.println(direction);

    //on en profite pour envoyer une notification au client.
    notifyClients(direction);
    newRequest = true; //on declare qu'il y a une nouvelle requete en execution
  }
}

// ------- WebSocket asynchronous event -------
//configuration d'un écouteur d'événement pour gérer les différentes étapes asynchrones du protocole WebSocket.
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      //Notify client of motor current state when it first connects
      notifyClients(direction);
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
        handleWebSocketMessage(arg, data, len);
        break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
     break;
  }
}

//initialisation du webSocket
void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}



// ------- start SETUP -------
void setup() {
  // Serial port for debugging purposes

  //setup et lancement des fonctionnalité
  Serial.begin(115200);
  initWiFi();
  initWebSocket();
  initSPIFFS();
  myStepper.setSpeed(12); // on peu set le speed du moteur ici (conseiler moin que 15)

  // Web Server Root URL - set du URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });
  
  server.serveStatic("/", SPIFFS, "/");//set du serveur

  server.begin();//debut du serveur
}

// ------- principal loop -------
void loop() {

  //si un requete est commencer
  if (newRequest){

    //// ------- FONCTIONNALITÉ DE JEU ------- 
    //// ------- gestion des action (dés) ------- 
    //ici sont present toute les calcule par rappot au tag de dés envoyer par le clients.
    //(A noté que tous les chiffre son decalé entre se script et les face actuel puisque 1 = 0 en terme de mouvement vs face. )
    
    //(A noté que tous se code aurai pu etre condenser en changent les actionType du client par des nombre.
    // raison d'un construction come cela est que originalment il devai y avoir des type d'action qui aurai inclus plus d'un dés.
    // Cela a été abandonner apres consultation avec le prof sur le niveau de complexité nessesaire.

    //Si 1 dés 4 est demander
    if (actionType == "1d4"){
      DiceSection = 2048 / 4; //on divise la revolution du moteur par nombre de face au dés virtuel
      Serial.print((int)DiceSection);
      r = rand() % 4; //on roule un random entre 0 et le nombre de face
      Serial.print((int)r);

      //on retourne au client la valeur rouler vs le max possible du dés
      String notifyString = String("Vous avez rouler un ")+(r+1)+ String(" sur 4");
      notifyClients(notifyString);
      
      DiceSteps = ((int)DiceSection) * ((int)r); // on set le nombre de steps actuel a faire
      myStepper.step(DiceSteps.toInt()); //on lance le mouvement
      Serial.print((String)DiceSteps);

      //apres 1 second, on lance le retour au point de depart.
      //(a noté que cela est mise en place du a l,incapacité au stepper motor de calculer un point d'origine).
      delay(1000);
      myStepper.step(-DiceSteps.toInt());
    }

    //Si 1 dés 6 est demander
    if (actionType == "1d6"){
      DiceSection = 2048 / 6; //on divise la revolution du moteur par nombre de face au dés virtuel
      Serial.print((int)DiceSection);
      r = rand() % 6; //on roule un random entre 0 et le nombre de face
      Serial.print((int)r);
      
      //on retourne au client la valeur rouler vs le max possible du dés
      String notifyString = String("Vous avez rouler un ")+(r+1)+ String(" sur 6");
      notifyClients(notifyString);
      
      DiceSteps = ((int)DiceSection) * ((int)r); // on set le nombre de steps actuel a faire
      myStepper.step(DiceSteps.toInt()); //on lance le mouvement
      Serial.print((String)DiceSteps);

      //apres 1 second, on lance le retour au point de depart.
      //(a noté que cela est mise en place du a l,incapacité au stepper motor de calculer un point d'origine).
      delay(1000);
      myStepper.step(-DiceSteps.toInt());
    }
    //Si 1 dés 8 est demander
    if (actionType == "1d8"){
      DiceSection = 2048 / 8; //on divise la revolution du moteur par nombre de face au dés virtuel
      Serial.print((int)DiceSection);
      r = rand() % 8; //on roule un random entre 0 et le nombre de face
      Serial.print((int)r);
      
      //on retourne au client la valeur rouler vs le max possible du dés
      String notifyString = String("Vous avez rouler un ")+(r+1)+ String(" sur 8");
      notifyClients(notifyString);
      
      DiceSteps = ((int)DiceSection) * ((int)r); // on set le nombre de steps actuel a faire
      myStepper.step(DiceSteps.toInt()); //on lance le mouvement
      Serial.print((String)DiceSteps);

      //apres 1 second, on lance le retour au point de depart.
      //(a noté que cela est mise en place du a l,incapacité au stepper motor de calculer un point d'origine).
      delay(1000);
      myStepper.step(-DiceSteps.toInt());
    }
    //Si 1 dés 20 est demander
    if (actionType == "1d20"){
      DiceSection = 2048 / 20; //on divise la revolution du moteur par nombre de face au dés virtuel
      Serial.print((int)DiceSection);
      r = rand() % 20; //on roule un random entre 0 et le nombre de face
      Serial.print((int)r);
      
      //on retourne au client la valeur rouler vs le max possible du dés
      String notifyString = String("Vous avez rouler un ")+(r+1)+ String(" sur 20");
      notifyClients(notifyString);
      
      DiceSteps = ((int)DiceSection) * ((int)r); // on set le nombre de steps actuel a faire
      myStepper.step(DiceSteps.toInt()); //on lance le mouvement
      Serial.print((String)DiceSteps);

      //apres 1 second, on lance le retour au point de depart.
      //(a noté que cela est mise en place du a l,incapacité au stepper motor de calculer un point d'origine).
      delay(1000);
      myStepper.step(-DiceSteps.toInt());
    }


    //// ------- gestion debug MENU ------- 
    //vous trouveré dans le client le debug menu sous un input qui permet d'ouvrir le menu.
    //Ces fonctionnalités sont essenciellement se qui compose le tutoriel suivi a la base.
    //le retour au point 0 a été ajouter.

    //si la direction demander en sens horaire
    if (direction == "CW"){
      myStepper.step(steps.toInt());
      delay(1000);
      myStepper.step(-steps.toInt());
      Serial.print("CW");
    }
    //si la direction demander en sens anti-horaire
    if (direction == "CCW"){
      myStepper.step(-steps.toInt());
      delay(1000);
      myStepper.step(steps.toInt());
      Serial.print("CCW");
    }

    //pour finir on envoi la fin de la requete
    delay(1000);
    newRequest = false;
    notifyClients("stop"); //on retour au client que le moteur c'est arreté
  }
  ws.cleanupClients(); //on lance un cleanup au niveau client.
}
