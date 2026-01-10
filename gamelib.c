#include "gamelib.h"

// --- VARIABILI GLOBALI STATICHE ---
// Array di puntatori ai giocatori (massimo 4)
static struct Giocatore* giocatori[4] = {NULL, NULL, NULL, NULL};
static int numero_giocatori = 0;

// Puntatori alle teste delle mappe
static struct Zona_mondoreale* prima_zona_mondoreale = NULL;
static struct Zona_soprasotto* prima_zona_soprasotto = NULL;

// Variabili di stato
static int undici_preso = 0; // Flag per la classe speciale
static int gioco_pronto = 0; // Flag per verificare se la mappa è chiusa

// --- FUNZIONI DI UTILITÀ (STATICHE) ---

// Genera un numero casuale tra min e max (inclusi)
static int casuale(int min, int max) {
    return rand() % (max - min + 1) + min;
}

// Pulisce il buffer di input
static void pulisci_buffer() {
    while (getchar() != '\n');
}

// Dealloca tutta la memoria
static void dealloca_tutto() {
    // 1. Dealloca giocatori
    for (int i = 0; i < 4; i++) {
        if (giocatori[i] != NULL) {
            free(giocatori[i]);
            giocatori[i] = NULL;
        }
    }
    numero_giocatori = 0;
    undici_preso = 0;

    // 2. Dealloca mappe (Implementazione futura)
    // Qui andrà il codice per liberare le liste delle zone
    prima_zona_mondoreale = NULL;
    prima_zona_soprasotto = NULL;
    
    gioco_pronto = 0;
    printf("Memoria precedente liberata.\n");
}

// --- FUNZIONI PUBBLICHE ---

void imposta_gioco() {
    // Se il gioco era già impostato, puliamo tutto
    if (numero_giocatori > 0 || prima_zona_mondoreale != NULL) {
        dealloca_tutto();
    }

    printf("\n--- IMPOSTAZIONE GIOCO ---\n");

    // 1. Inserimento numero giocatori
    do {
        printf("Inserisci numero di giocatori (1-4): ");
        if (scanf("%d", &numero_giocatori) != 1) {
            pulisci_buffer();
            continue;
        }
    } while (numero_giocatori < 1 || numero_giocatori > 4);
    pulisci_buffer(); 

    // 2. Creazione e configurazione giocatori
    for (int i = 0; i < numero_giocatori; i++) {
        printf("\n--- Configurazione Giocatore %d ---\n", i + 1);

        // Allocazione dinamica
        giocatori[i] = (struct Giocatore*) malloc(sizeof(struct Giocatore));
        if (giocatori[i] == NULL) {
            printf("Errore critico: memoria insufficiente.\n");
            exit(1);
        }

        // Inizializzazione base
        giocatori[i]->mondo = 0; 
        giocatori[i]->pos_mondoreale = NULL;
        giocatori[i]->pos_soprasotto = NULL;
        
        // Inizializza zaino vuoto
        for(int k=0; k<3; k++) giocatori[i]->zaino[k] = nessun_oggetto;

        // Inserimento Nome
        printf("Nome giocatore: ");
        fgets(giocatori[i]->nome, 100, stdin); 
        giocatori[i]->nome[strcspn(giocatori[i]->nome, "\n")] = 0; // Rimuove newline

        // Generazione statistiche iniziali
        giocatori[i]->attacco_pischico = casuale(1, 20);
        giocatori[i]->difesa_pischica = casuale(1, 20);
        giocatori[i]->fortuna = casuale(1, 20);

        printf("Statistiche tirate: Attacco %d, Difesa %d, Fortuna %d\n", 
               giocatori[i]->attacco_pischico, 
               giocatori[i]->difesa_pischica, 
               giocatori[i]->fortuna);

        // Menu scelta classe/modifiche
        int scelta_classe = 0;
        printf("Scegli una modifica:\n");
        printf("0) Nessuna modifica\n");
        printf("1) +3 Attacco / -3 Difesa\n");
        printf("2) -3 Attacco / +3 Difesa\n");
        if (!undici_preso) {
            printf("3) Diventa Undici VirgolaCinque (+4 Atk/Def, -7 Fortuna) [UNICO]\n");
        }
        
        do {
            printf("Scelta: ");
            scanf("%d", &scelta_classe);
            pulisci_buffer();
        } while (scelta_classe < 0 || scelta_classe > 3 || (scelta_classe == 3 && undici_preso));

        // Applicazione modifiche
        switch (scelta_classe) {
            case 1:
                giocatori[i]->attacco_pischico += 3;
                giocatori[i]->difesa_pischica -= 3;
                break;
            case 2:
                giocatori[i]->attacco_pischico -= 3;
                giocatori[i]->difesa_pischica += 3;
                break;
            case 3:
                giocatori[i]->attacco_pischico += 4;
                giocatori[i]->difesa_pischica += 4;
                giocatori[i]->fortuna -= 7;
                strcpy(giocatori[i]->nome, "Undici VirgolaCinque"); 
                undici_preso = 1; 
                break;
            default:
                break;
        }

        // Controllo limiti minimi statistiche
        if (giocatori[i]->attacco_pischico < 0) giocatori[i]->attacco_pischico = 0;
        if (giocatori[i]->difesa_pischica < 0) giocatori[i]->difesa_pischica = 0;
        if (giocatori[i]->fortuna < 0) giocatori[i]->fortuna = 0;

        printf("Statistiche Finali di %s -> Atk: %d, Def: %d, Fortuna: %d\n",
               giocatori[i]->nome, giocatori[i]->attacco_pischico, 
               giocatori[i]->difesa_pischica, giocatori[i]->fortuna);
    }

    printf("\nGiocatori impostati con successo!\n");
    // La logica per la creazione della mappa verrà aggiunta qui
}

void gioca() {
    if (!gioco_pronto) {
        printf("Errore: Il gioco non è stato ancora impostato o la mappa non è chiusa!\n");
        return;
    }
    printf("Funzione gioca chiamata.\n");
}

void termina_gioco() {
    printf("Uscita dal gioco. Deallocazione memoria...\n");
    dealloca_tutto();
}

void crediti() {
    printf("\n--- CREDITI ---\n");
    printf("Creato da: [Tuo Nome]\n"); // Inserisci qui il tuo nome
}