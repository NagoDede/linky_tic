#include "tic.h"
#include "serial.h"
#include <Arduino.h>

struct GroupDetail TicValues[NB_ETIQUETTE] = {};

String data = "";             // Variable pour stocker la trame complète
boolean isReceiving = false;  // Indicateur pour savoir si on est dans une trame
RegistreStatus regStatus;     // definition du registre status
RelaisStatus relaisStatus;    //definition du relais status
Action actionJp1[11];         //actions définie pour jour +1
int nbActions;

static struct GroupDetail processGroup(String group) {
  struct GroupDetail gd;
  int indexgrp = group.indexOf(HT);
  gd.name = group.substring(0, indexgrp);

  group = group.substring(indexgrp + 1);
  indexgrp = group.indexOf(HT);
  gd.value = group.substring(0, indexgrp);

  group = group.substring(indexgrp + 1);
  indexgrp = group.indexOf(HT);
  String key = group.substring(0, indexgrp);

  group = group.substring(indexgrp + 1);
  indexgrp = group.indexOf(HT);
  if (indexgrp != -1)  //some parameters may have hour recording.
  {
    gd.horodate = gd.value;
    gd.value = key;
  }
  return gd;
}

static void processStge(RegistreStatus *rs, String value) {
  char stge[9] = "";
  //copy in the char array
  strncpy(stge, value.c_str(), 8);
  stge[8] = '\0';
  unsigned long l = strtoul(stge, NULL, 16);  // Convert hex pair to unsigned long
  rs->uli = l;
}

static void processRelais(RelaisStatus *rs, String value) {
  char stge[4] = "";
  //copy in the char array
  strncpy(stge, value.c_str(), 3);
  stge[4] = '\0';
  rs->ui = strtoul(stge, NULL, 16);
}

static void processActionsCalendrier(String value) {
  nbActions = 0;
  String s = value;
  while (s.length() > 0) {
    int index = s.indexOf(SP);
    if (index == -1)  // No space found
    {
      break;
    } else {
      char data[9] = "";
      data[8] = '\0';
      strncpy(data, s.substring(0, index).c_str(), 8);
      if (strncmp(data, NONUTILE, 8) != 0) {
        char stge[5] = "";
        //copy ssss field
        memcpy(stge, &data[4], 4);
        actionJp1[nbActions].action.ui = strtoul(stge, NULL, 16);
        //copt hhmm
        memcpy(actionJp1[nbActions].startTime, &data[0], 4);
        ++nbActions;
      }
      s = s.substring(index + 1);
    }
  }
}

static void processTrame(String &data) {
  while (data.length() > 0) {
    int index = data.indexOf(CR);
    if (index == -1)  // No space found
    {
      break;
    } else {
      String group = data.substring(1, index);
      auto gd = processGroup(group);

      //check if etiquette is user selected
      int t = 0;
      while ((SelectedEtiquette[t] != gd.name) && (t < NB_ETIQUETTE)) { ++t; }
      if (t < NB_ETIQUETTE) {
        TicValues[t] = gd;
        if (gd.name == "STGE") {
          processStge(&regStatus, gd.value);
        } else if (gd.name == "RELAIS") {
          processRelais(&relaisStatus, gd.value);
        } else if (gd.name == "PJOURF+1") {
          processActionsCalendrier(gd.value);
        }
      }
      data = data.substring(index + 1);
    }
  }
}

static char *actionJp1AsJson() {

  const int bufferSize = 1000;
  static char jsonBuffer[bufferSize];  // Adjust size as needed
  snprintf(jsonBuffer, bufferSize, "\"PJOURF+1\": [");

  for (int i = 0; i < nbActions; i++) {
    // Format each action
    char actionJson[256];  // To store individual action JSON string
    String relaisSec = "";
    switch ((unsigned int)actionJp1[i].action.bits.relaisSec) {
      case 0:
        relaisSec = "no change";
        break;
      case 1:
        relaisSec = "tempo";
        break;
      case 2:
        relaisSec = "open";
        break;
      case 3:
        relaisSec = "closed";
        break;
      default:
        relaisSec = "unknown";
    }
    snprintf(actionJson, sizeof(actionJson),
             "  { \"startTime\": \"%c%c%c%c\", "
             "\"relaisSec\": \"%s\", "
             "\"relais7\": %u, \"relais6\": %u, \"relais5\": %u, \"relais4\": %u, "
             "\"relais3\": %u, \"relais2\": %u, \"relais1\": %u, \"index\": %u }",
             actionJp1[i].startTime[0], actionJp1[i].startTime[1], actionJp1[i].startTime[2], actionJp1[i].startTime[3],
             relaisSec.c_str(),
             actionJp1[i].action.bits.relais7, actionJp1[i].action.bits.relais6, actionJp1[i].action.bits.relais5,
             actionJp1[i].action.bits.relais4, actionJp1[i].action.bits.relais3, actionJp1[i].action.bits.relais2,
             actionJp1[i].action.bits.relais1, actionJp1[i].action.bits.index);

    // Append the current action's JSON to the overall JSON buffer
    if (i == (nbActions - 1)) {  // Last item, no comma at the end
      strncat(jsonBuffer, actionJson, bufferSize - strlen(jsonBuffer) - 1);
    } else {
      strncat(jsonBuffer, actionJson, bufferSize - strlen(jsonBuffer) - 1);
      strncat(jsonBuffer, ",", bufferSize - strlen(jsonBuffer) - 1);
    }
  }

  // End the JSON array
  strncat(jsonBuffer, "]", bufferSize - strlen(jsonBuffer) - 1);
  return jsonBuffer;
}


static char *relaisStatusAsJson(RelaisStatusBits *status, String rawValue) {
  // Pre-allocate buffer large enough to hold the JSON string
  static char response[150];  // Adjust size as needed
  // Use snprintf to construct the JSON string efficiently
  snprintf(response, sizeof(response),
           "\"RELAIS\": "
           "{"
           "\"value\": \"%s\", "
           "\"relaisSec\": %d, "
           "\"relais1\": %d, "
           "\"relais2\": %d, "
           "\"relais3\": %d, "
           "\"relais4\": %d, "
           "\"relais5\": %d, "
           "\"relais6\": %d, "
           "\"relais7\": %d "
           "}",
           rawValue.c_str(),
           status->relaisSec,
           status->relais1,
           status->relais2,
           status->relais3,
           status->relais4,
           status->relais5,
           status->relais6,
           status->relais7);
  return response;
}

/*
static char *registreStatusAsJson(RegistreStatusBits *status, String rawValue) {
  // Pre-allocate buffer large enough to hold the JSON string
  static char response[400];  // Adjust size as needed

  // Use snprintf to construct the JSON string efficiently
  snprintf(response, sizeof(response),
           "\"STGE\": "
           "{"
           "\"value\": \"%s\", "
           "\"contactsec\": %d, "
           "\"organeCoupure\": %d, "
           "\"cache\": %d, "
           "\"surtension\": %d, "
           "\"depassementPuissance\": %d, "
           "\"consoProd\": %d, "
           "\"senseActiveEnergy\": %d, "
           "\"tarifIndexConso\": %d, "
           "\"tarifIndexProd\": %d, "
           "\"horlogeState\": %d, "
           "\"ticState\": %d, "
           "\"comEuridis\": %d, "
           "\"cplState\": %d, "
           "\"cplSynchro\": %d, "
           "\"tempo\": %d, "
           "\"tempoNextDay\": %d, "
           "\"preavisPM\": %d, "
           "\"PM\": %d"
           "}",
           rawValue.c_str(),
           status->contactsec,
           status->organeCoupure,
           status->cache,
           status->surtension,
           status->depassementPuissance,
           status->consoProd,
           status->senseActiveEnergy,
           status->tarifIndexConso,
           status->tarifIndexProd,
           status->horlogeState,
           status->ticState,
           status->comEuridis,
           status->cplState,
           status->cplSynchro,
           status->tempo,
           status->tempoNextDay,
           status->preavisPM,
           status->PM);
  return response;
}*/

static char *registreStatusAsJson(RegistreStatusBits *status, String rawValue) {
  // Pre-allocate buffer large enough to hold the JSON string
  static char response[1000];  // Adjust size as needed

  // Use snprintf to construct the JSON string efficiently
  snprintf(response, sizeof(response),
           "\"STGE\": "
           "{"
           "\"value\": \"%s\", "
           "\"contactsec\": \"%s\", "
           "\"organeCoupure\": \"%s\", "
           "\"cache\": \"%s\", "
           "\"surtension\": \"%s\", "
           "\"depassementPuissance\": \"%s\", "
           "\"consoProd\": \"%s\", "
           "\"senseActiveEnergy\": \"%s\", "
           "\"tarifIndexConso\": %d, "
           "\"tarifIndexProd\": %d, "
           "\"horlogeState\": \"%s\", "
           "\"ticState\": \"%s\", "
           "\"comEuridis\": \"%s\", "
           "\"cplState\": \"%s\", "
           "\"cplSynchro\": \"%s\", "
           "\"tempo\": \"%s\", "
           "\"tempoNextDay\": \"%s\", "
           "\"preavisPM\": \" preavis %s\", "
           "\"PM\": \"%s\""
           "}",
           rawValue.c_str(),
           kContactStatus[status->contactsec].c_str(),
           kCoupure[status->organeCoupure].c_str(),
           kContactStatus[status->cache].c_str(),
           kOverVoltage[status->surtension].c_str(),
           kOverPower[status->depassementPuissance].c_str(),
           kProducer[status->consoProd].c_str(),
           kActivePower[status->senseActiveEnergy].c_str(),
           status->tarifIndexConso + 1,
           status->tarifIndexProd + 1,
           kHour[status->horlogeState].c_str(),
           kTicMode[status->ticState].c_str(),
           kEuridis[status->comEuridis].c_str(),
           kCpl[status->cplState].c_str(),
           kCplSynchro[status->cplSynchro].c_str(),
           kTempoColor[status->tempo].c_str(),
           kTempoColor[status->tempoNextDay].c_str(),
           kPointeMobile[status->preavisPM].c_str(),
           kPointeMobile[status->PM].c_str());
  return response;
}

/*
static void printbits(const RegistreStatus *c) {
  // Iterate over each bit in the 32-bit value (uli)
  for (int i = 31; i >= 0; --i) {
    // Print '1' if the corresponding bit is set, otherwise print '0'
    DebugPort.print(((c->uli >> i) & 1) ? "1" : "0");
  }
  DebugPort.print("\n");  // New line after printing all bits
}*/


String ticValuesAsJson() {
  String response = "{";

  for (int i = 0; i < NB_ETIQUETTE; ++i) {

    if (SelectedEtiquette[i] == "STGE") {
      response += registreStatusAsJson(&regStatus.bits, TicValues[i].value);
    } else if (SelectedEtiquette[i] == "RELAIS") {
      response += relaisStatusAsJson(&relaisStatus.bits, TicValues[i].value);
    } else if (SelectedEtiquette[i] == "PJOURF+1") {
      response += actionJp1AsJson();
    } else {
      static char jres[150];  // Adjust size as needed

      // Use snprintf to construct the JSON string efficiently
      snprintf(jres, sizeof(jres),
               "\"%s\": \"%s\"",
               SelectedEtiquette[i].c_str(),
               TicValues[i].value.c_str());
      response += jres;

      //response += "\"" + SelectedEtiquette[i] + "\": ";
      //response += "\"" + TicValues[i].value + "\"";
    }

    if (i < (NB_ETIQUETTE - 1)) {
      response += ',';
    }
  }
  response += "}";
  return response;
}



void readTicPort() {
  if (TicPort.available()) {
    byte incomingByte = TicPort.read();  // Lire un octet

    if (incomingByte == EOT) {
      //Fin forcée de transmission
      //Tout est rejeté
      isReceiving = false;
    }

    if (isReceiving) {
      // Vérifier si la fin de la trame est atteinte
      if (incomingByte == ETX) {
        // Extraire la partie utile de la trame
        processTrame(data);
        // Afficher la donnée extraite
        isReceiving = false;

        //debug
        for (int i = 0; i < NB_ETIQUETTE; ++i) {
          DebugPort.print(TicValues[i].name);
          DebugPort.print(":");
          DebugPort.println(TicValues[i].value);
        }
        //enddebug
      } else {
        data += (char)incomingByte;  // Ajouter l'octet à la trame en cours
      }
    } else {
      // Chercher le début de la trame (<STX>)
      if (incomingByte == STX) {
        isReceiving = true;
        data = "";  // Réinitialiser pour commencer une nouvelle trame
      }
    }
  }
}