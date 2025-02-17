

#ifndef TIC_DEF
#define TIC_DEF

#include <Arduino.h>

#define TIC

#define STX 0x02  // Début de la trame : 0x02 (<STX>)
#define ETX 0x03  // Fin de la trame : 0x03 (<ETX>) End Of Text
#define EOT 0x04  //End Of Transmission alias  fin de transmission
#define LF 0x0A   // Code ASCII pour <LF>, debut de groupe d'information
#define CR 0x0D   // Fin de groupe d'information
#define HT 0x09   //separateur dans groupe
#define SP 0x20   //separateur dans groupe
#define NONUTILE "NONUTILE"

#define MAX_CHAR_ETIQUETTE 9
#define NB_ETIQUETTE 39

struct GroupDetail {
  String name;
  String value;
  String horodate;
};

struct RegistreStatusBits {
  uint32_t contactsec : 1;            //bit 0
  uint32_t organeCoupure : 3;         //bit 1 - 3
  uint32_t cache : 1;                 //bit 4
  uint32_t : 1;                       //bit 5
  uint32_t surtension : 1;            //bit 6
  uint32_t depassementPuissance : 1;  //bit 7-----
  uint32_t consoProd : 1;             //bit 8
  uint32_t senseActiveEnergy : 1;     //bit 9
  uint32_t tarifIndexConso : 4;       //bit 10 -13
  uint32_t tarifIndexProd : 2;        //bit 14 - 15 -----
  uint32_t horlogeState : 1;          //bit 16
  uint32_t ticState : 1;              //17
  uint32_t : 1;                       //18
  uint32_t comEuridis : 2;            //19-20
  uint32_t cplState : 2;              //21-22
  uint32_t cplSynchro : 1;            //23
  uint32_t tempo : 2;                 //24-25
  uint32_t tempoNextDay : 2;          //26-27
  uint32_t preavisPM : 2;             //28-29
  uint32_t PM : 2;                    //30-31
};

struct RelaisStatusBits {
  uint8_t relaisSec : 1;  //bit 1
  uint8_t relais1 : 1;    //bit 2
  uint8_t relais2 : 1;    //bit 3
  uint8_t relais3 : 1;    //bit 4
  uint8_t relais4 : 1;    //bit 5
  uint8_t relais5 : 1;    //bit 6-----
  uint8_t relais6 : 1;    //bit 7-----
  uint8_t relais7 : 1;    //bit 8-----
};

struct ActionsCalendrierBits {
  uint8_t index : 4;       //bit 3 - 0-----
  uint8_t relais1 : 1;     //bit 4-----
  uint8_t relais2 : 1;     //bit 8-----
  uint8_t relais3 : 1;     //bit 8-----
  uint8_t relais4 : 1;     //bit 8-----
  uint8_t relais5 : 1;     //bit 7-----
  uint8_t relais6 : 1;     //bit 6-----
  uint8_t relais7 : 1;     //bit 10
  uint8_t : 3;             //bit 11-13
  uint16_t relaisSec : 2;  //Contact Sec bit 15-14
};
union ActionCalendrier {
  uint16_t ui;
  ActionsCalendrierBits bits;
};

struct Action {
  char startTime[4];
  ActionCalendrier action;
};

union RelaisStatus {
  uint8_t ui;
  RelaisStatusBits bits;
};

union RegistreStatus {
  uint32_t uli;
  RegistreStatusBits bits;
};



const static String SelectedEtiquette[NB_ETIQUETTE] = { "ADSC", "DATE", "NGTF", "LTARF", "EAST", "EASF01", "EASF02", "EASF03", "EASF04", "EASD01", "EASD02", "EASD03", "EASD04", "IRMS1", "IRMS2", "IRMS3", "URMS1", "URMS2", "URMS3", "PCOUP", "SINSTS", "SINSTS1", "SINSTS2", "SINSTS3", "STGE", "DPM1", "FPM1", "DPM2", "FPM2", "DPM3", "FPM3", "RELAIS", "NTARF", "NJOURF", "NJOURF+1", "PJOURF+1", "MSG1", "MSG2", "PPOINTE" };
extern struct GroupDetail TicValues[NB_ETIQUETTE];  //records the values of the identified group

//Constantes pour definition des statuts
const static String kContactStatus[2] = { "closed", "open" };  //relais et contacteurs
const static String kCoupure[7] = { "closed",
                                    "open_overpower",
                                    "open_overvoltage",
                                    "open_shedding",
                                    "open_euridis",
                                    "open_overheat-with-overcurrent",
                                    "open_overheat-without-overcurrent" };
const static String kOverVoltage[2] = { "no_overvoltage", "overvoltage" };
const static String kOverPower[2] = { "no_overpower", "overpower" };
const static String kProducer[2] = { "consummer", "producer" };
const static String kActivePower[2] = { "positive", "negative" };
const static String kHour[2] = { "ok", "degraded" };
const static String kTicMode[2] = { "historique", "standard" };
const static String kEuridis[4] = { "deactivated", "activated_without_security", "!!!", "activated_with_security" };
const static String kCpl[4] = { "new_unlock", "new_lock", "!!!", "registered" };
const static String kCplSynchro[2] = { "not_synchro", "synchro" };
const static String kTempoColor[4] = { "no", "blue", "white", "red" };
const static String kPointeMobile[4] = { "no", "PM1", "PM2", "PM3" };
//struct GroupDetail processGroup(String group);
//void processTrame(String &data);
void readTicPort();
String ticValuesAsJson();
#endif