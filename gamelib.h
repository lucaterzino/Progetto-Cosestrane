#ifndef GAMELIB_H
#define GAMELIB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Definizioni Enum [cite: 38, 39, 40]
typedef enum {
    bosco, scuola, laboratorio, caverna, strada, giardino, 
    supermercato, centrale_elettrica, deposito_abbandonato, stazione_polizia
} Tipo_zona;

typedef enum {
    nessun_nemico, billi, democane, demotorzone
} Tipo_nemico;

typedef enum {
    nessun_oggetto, bicicletta, maglietta_fuocoinferno, bussola, schitarrata_metallica
} Tipo_oggetto;

// Forward declaration per permettere i puntatori incrociati tra le struct
struct Zona_soprasotto; 
struct Zona_mondoreale;

// Struttura Zona Mondo Reale [cite: 28]
typedef struct Zona_mondoreale {
    Tipo_zona tipo;                 // [cite: 29]
    Tipo_nemico nemico;             // [cite: 30]
    Tipo_oggetto oggetto;           // [cite: 30]
    struct Zona_mondoreale* avanti; // [cite: 31]
    struct Zona_mondoreale* indietro; // [cite: 31]
    struct Zona_soprasotto* link_soprasotto; // [cite: 32]
} Zona_mondoreale;

// Struttura Zona Soprasotto [cite: 33]
typedef struct Zona_soprasotto {
    Tipo_zona tipo;                 // [cite: 34]
    Tipo_nemico nemico;             // [cite: 35]
    // NON è presente il campo oggetto [cite: 35]
    struct Zona_soprasotto* avanti; // [cite: 36]
    struct Zona_soprasotto* indietro; // [cite: 36]
    struct Zona_mondoreale* link_mondoreale; // [cite: 37]
} Zona_soprasotto;

// Struttura Giocatore [cite: 25]
typedef struct Giocatore {
    char nome[100]; // Dimensione arbitraria sicura per il nome
    int mondo; // 0 = Mondo Reale, 1 = Soprasotto [cite: 26]
    Zona_mondoreale* pos_mondoreale; // [cite: 27]
    Zona_soprasotto* pos_soprasotto; // [cite: 27]
    int attacco_pischico; // Valore 1-20 [cite: 27]
    int difesa_pischica;  // Valore 1-20 [cite: 27]
    int fortuna;          // Valore 1-20 [cite: 27]
    Tipo_oggetto zaino[3]; // Array di 3 oggetti [cite: 27]
} Giocatore;

// Prototipi delle funzioni pubbliche [cite: 18, 24]
void imposta_gioco();
void gioca();
void termina_gioco();
void crediti();

#endif