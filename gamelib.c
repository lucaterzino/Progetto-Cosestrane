#include "gamelib.h"

// --- VARIABILI GLOBALI STATICHE ---
static struct Giocatore* giocatori[4] = {NULL, NULL, NULL, NULL};
static int numero_giocatori = 0;

static struct Zona_mondoreale* prima_zona_mondoreale = NULL;
static struct Zona_soprasotto* prima_zona_soprasotto = NULL;

static int undici_preso = 0;
static int gioco_pronto = 0;

// --- FUNZIONI DI UTILITÀ (STATICHE) ---

static int casuale(int min, int max) {
    return rand() % (max - min + 1) + min;
}

static void pulisci_buffer() {
    while (getchar() != '\n');
}

// Funzione helper per liberare la memoria della mappa
static void dealloca_mappa() {
    struct Zona_mondoreale* attuale_mr = prima_zona_mondoreale;
    struct Zona_soprasotto* attuale_ss = prima_zona_soprasotto;

    while (attuale_mr != NULL) {
        struct Zona_mondoreale* temp = attuale_mr;
        attuale_mr = attuale_mr->avanti;
        free(temp);
    }
    
    while (attuale_ss != NULL) {
        struct Zona_soprasotto* temp = attuale_ss;
        attuale_ss = attuale_ss->avanti;
        free(temp);
    }

    prima_zona_mondoreale = NULL;
    prima_zona_soprasotto = NULL;
}

static void dealloca_tutto() {
    for (int i = 0; i < 4; i++) {
        if (giocatori[i] != NULL) {
            free(giocatori[i]);
            giocatori[i] = NULL;
        }
    }
    numero_giocatori = 0;
    undici_preso = 0;

    dealloca_mappa();
    
    gioco_pronto = 0;
    printf("Memoria precedente liberata.\n");
}

// --- FUNZIONI GESTIONE MAPPA (STATICHE) ---

// Restituisce il puntatore alla zona MR in posizione 'indice' (o NULL se non esiste)
static struct Zona_mondoreale* ottieni_zona_mr(int indice) {
    struct Zona_mondoreale* p = prima_zona_mondoreale;
    int count = 0;
    while (p != NULL && count < indice) {
        p = p->avanti;
        count++;
    }
    return p;
}

// Restituisce il numero totale di zone
static int conta_zone() {
    int count = 0;
    struct Zona_mondoreale* p = prima_zona_mondoreale;
    while (p != NULL) {
        count++;
        p = p->avanti;
    }
    return count;
}

static void genera_mappa() {
    if (prima_zona_mondoreale != NULL) {
        dealloca_mappa();
    }

    struct Zona_mondoreale* coda_mr = NULL;
    struct Zona_soprasotto* coda_ss = NULL;

    // Creiamo 15 zone
    for (int i = 0; i < 15; i++) {
        // Allocazione
        struct Zona_mondoreale* nuova_mr = (struct Zona_mondoreale*) malloc(sizeof(struct Zona_mondoreale));
        struct Zona_soprasotto* nuova_ss = (struct Zona_soprasotto*) malloc(sizeof(struct Zona_soprasotto));

        // Tipo zona (uguale per entrambi)
        Tipo_zona tipo = (Tipo_zona) casuale(0, 9);
        nuova_mr->tipo = tipo;
        nuova_ss->tipo = tipo;

        // Nemici e Oggetti (Random base)
        // Mondo Reale: Nessun nemico (60%), Democane (30%), Billi (10%)
        int r_nem = casuale(1, 100);
        if (r_nem <= 60) nuova_mr->nemico = nessun_nemico;
        else if (r_nem <= 90) nuova_mr->nemico = democane;
        else nuova_mr->nemico = billi;

        // Soprasotto: Nessun nemico (60%), Democane (30%), Demotorzone (10%)
        r_nem = casuale(1, 100);
        if (r_nem <= 60) nuova_ss->nemico = nessun_nemico;
        else if (r_nem <= 90) nuova_ss->nemico = democane;
        else nuova_ss->nemico = demotorzone;

        // Oggetto (Solo MR): Nessuno (50%), Random (50%)
        // Modifica: rimosso r_ogg inutilizzato
        if (casuale(0, 1) == 0) nuova_mr->oggetto = nessun_oggetto;
        else nuova_mr->oggetto = (Tipo_oggetto) casuale(1, 4);

        // Collegamenti Link Verticali
        nuova_mr->link_soprasotto = nuova_ss;
        nuova_ss->link_mondoreale = nuova_mr;

        // Collegamenti Orizzontali (Lista doppia)
        nuova_mr->avanti = NULL;
        nuova_ss->avanti = NULL;

        if (prima_zona_mondoreale == NULL) {
            // Prima zona
            prima_zona_mondoreale = nuova_mr;
            prima_zona_soprasotto = nuova_ss;
            nuova_mr->indietro = NULL;
            nuova_ss->indietro = NULL;
        } else {
            // Aggiungi in coda
            coda_mr->avanti = nuova_mr;
            nuova_mr->indietro = coda_mr;
            
            coda_ss->avanti = nuova_ss;
            nuova_ss->indietro = coda_ss;
        }
        
        // Aggiorna coda
        coda_mr = nuova_mr;
        coda_ss = nuova_ss;
    }
    printf("Mappa generata casualmente (15 zone).\n");
}

static void inserisci_zona() {
    int posizione;
    int num_zone = conta_zone();
    
    printf("Inserisci posizione (0 - %d): ", num_zone);
    scanf("%d", &posizione);
    pulisci_buffer();

    if (posizione < 0 || posizione > num_zone) {
        printf("Posizione non valida.\n");
        return;
    }

    // Input Utente per la nuova zona
    struct Zona_mondoreale* nuova_mr = (struct Zona_mondoreale*) malloc(sizeof(struct Zona_mondoreale));
    struct Zona_soprasotto* nuova_ss = (struct Zona_soprasotto*) malloc(sizeof(struct Zona_soprasotto));

    printf("Tipo Zona (0-9): ");
    int t; scanf("%d", &t); nuova_mr->tipo = (Tipo_zona)t; nuova_ss->tipo = (Tipo_zona)t;
    
    printf("Nemico MR (0=Nessuno, 1=Billi, 2=Democane): ");
    scanf("%d", &t); 
    if(t==1) nuova_mr->nemico = billi; else if(t==2) nuova_mr->nemico = democane; else nuova_mr->nemico = nessun_nemico;

    printf("Oggetto MR (0-4): ");
    scanf("%d", &t); nuova_mr->oggetto = (Tipo_oggetto)t;

    printf("Nemico SS (0=Nessuno, 2=Democane, 3=Demotorzone): ");
    scanf("%d", &t);
    if(t==2) nuova_ss->nemico = democane; else if(t==3) nuova_ss->nemico = demotorzone; else nuova_ss->nemico = nessun_nemico;
    pulisci_buffer();

    // Link verticali
    nuova_mr->link_soprasotto = nuova_ss;
    nuova_ss->link_mondoreale = nuova_mr;

    // Inserimento in lista
    if (posizione == 0) {
        // Inserimento in testa
        nuova_mr->avanti = prima_zona_mondoreale;
        nuova_ss->avanti = prima_zona_soprasotto;
        nuova_mr->indietro = NULL;
        nuova_ss->indietro = NULL;
        
        if (prima_zona_mondoreale != NULL) {
            prima_zona_mondoreale->indietro = nuova_mr;
            prima_zona_soprasotto->indietro = nuova_ss;
        }
        prima_zona_mondoreale = nuova_mr;
        prima_zona_soprasotto = nuova_ss;

    } else {
        // Inserimento in mezzo o coda
        struct Zona_mondoreale* prec_mr = ottieni_zona_mr(posizione - 1);
        struct Zona_soprasotto* prec_ss = prec_mr->link_soprasotto; // Scorciatoia sicura

        nuova_mr->avanti = prec_mr->avanti;
        nuova_ss->avanti = prec_ss->avanti;
        
        nuova_mr->indietro = prec_mr;
        nuova_ss->indietro = prec_ss;

        if (prec_mr->avanti != NULL) {
            prec_mr->avanti->indietro = nuova_mr;
            prec_ss->avanti->indietro = nuova_ss;
        }

        prec_mr->avanti = nuova_mr;
        prec_ss->avanti = nuova_ss;
    }
    printf("Zona inserita con successo.\n");
}

static void cancella_zona() {
    int posizione;
    int num_zone = conta_zone();

    if (num_zone == 0) {
        printf("Mappa vuota.\n");
        return;
    }

    printf("Inserisci posizione da cancellare (0 - %d): ", num_zone - 1);
    scanf("%d", &posizione);
    pulisci_buffer();

    if (posizione < 0 || posizione >= num_zone) {
        printf("Posizione non valida.\n");
        return;
    }

    struct Zona_mondoreale* del_mr = ottieni_zona_mr(posizione);
    struct Zona_soprasotto* del_ss = del_mr->link_soprasotto;

    // Aggiornamento puntatori
    if (del_mr->indietro != NULL) {
        del_mr->indietro->avanti = del_mr->avanti;
        del_ss->indietro->avanti = del_ss->avanti;
    } else {
        // Stiamo cancellando la testa
        prima_zona_mondoreale = del_mr->avanti;
        prima_zona_soprasotto = del_ss->avanti;
    }

    if (del_mr->avanti != NULL) {
        del_mr->avanti->indietro = del_mr->indietro;
        del_ss->avanti->indietro = del_ss->indietro;
    }

    free(del_mr);
    free(del_ss);
    printf("Zona cancellata.\n");
}

static const char* nome_zona(Tipo_zona t) {
    switch(t) {
        case bosco: return "Bosco";
        case scuola: return "Scuola";
        case laboratorio: return "Laboratorio";
        case caverna: return "Caverna";
        case strada: return "Strada";
        case giardino: return "Giardino";
        case supermercato: return "Supermercato";
        case centrale_elettrica: return "Centrale Elettrica";
        case deposito_abbandonato: return "Deposito";
        case stazione_polizia: return "Polizia";
        default: return "Ignoto";
    }
}

static const char* nome_nemico(Tipo_nemico t) {
    switch(t) {
        case nessun_nemico: return "Nessuno";
        case billi: return "Billi";
        case democane: return "Democane";
        case demotorzone: return "Demotorzone";
        default: return "Ignoto";
    }
}

static void stampa_mappa() {
    if (prima_zona_mondoreale == NULL) {
        printf("Mappa vuota.\n");
        return;
    }
    
    int scelta;
    printf("Quale mappa stampare? 1) Mondo Reale 2) Soprasotto: ");
    scanf("%d", &scelta);
    pulisci_buffer();

    printf("\n--- VISUALIZZAZIONE MAPPA ---\n");
    if (scelta == 1) {
        struct Zona_mondoreale* p = prima_zona_mondoreale;
        int i = 0;
        while (p != NULL) {
            printf("[Zona %d] %s | Nemico: %s | Oggetto: %d\n", 
                i++, nome_zona(p->tipo), nome_nemico(p->nemico), p->oggetto);
            p = p->avanti;
        }
    } else {
        struct Zona_soprasotto* p = prima_zona_soprasotto;
        int i = 0;
        while (p != NULL) {
            printf("[Zona %d] %s | Nemico: %s\n", 
                i++, nome_zona(p->tipo), nome_nemico(p->nemico));
            p = p->avanti;
        }
    }
}

static void stampa_zona() {
    int posizione;
    printf("Inserisci indice zona: ");
    scanf("%d", &posizione);
    pulisci_buffer();

    struct Zona_mondoreale* p = ottieni_zona_mr(posizione);
    if (p == NULL) {
        printf("Zona inesistente.\n");
        return;
    }

    printf("\n--- DETTAGLIO ZONA %d ---\n", posizione);
    printf("Tipo: %s\n", nome_zona(p->tipo));
    printf("Mondo Reale -> Nemico: %s, Oggetto: %d\n", nome_nemico(p->nemico), p->oggetto);
    printf("Soprasotto  -> Nemico: %s\n", nome_nemico(p->link_soprasotto->nemico));
}

static void chiudi_mappa() {
    int n_zone = conta_zone();
    if (n_zone < 15) {
        printf("Errore: La mappa deve avere almeno 15 zone (attuali: %d).\n", n_zone);
        return;
    }

    // Conta Demotorzoni
    int demo_count = 0;
    struct Zona_soprasotto* p = prima_zona_soprasotto;
    while (p != NULL) {
        if (p->nemico == demotorzone) demo_count++;
        p = p->avanti;
    }

    if (demo_count != 1) {
        printf("Errore: Deve esserci esattamente 1 Demotorzone nel Soprasotto (trovati: %d).\n", demo_count);
        return;
    }

    gioco_pronto = 1;
    printf("Mappa chiusa correttamente. Il gioco può iniziare!\n");
}

// --- FUNZIONI PUBBLICHE ---

void imposta_gioco() {
    if (numero_giocatori > 0 || prima_zona_mondoreale != NULL) {
        dealloca_tutto();
    }

    printf("\n--- IMPOSTAZIONE GIOCO ---\n");

    // 1. Giocatori
    do {
        printf("Inserisci numero di giocatori (1-4): ");
        if (scanf("%d", &numero_giocatori) != 1) {
            pulisci_buffer();
            continue;
        }
    } while (numero_giocatori < 1 || numero_giocatori > 4);
    pulisci_buffer();

    for (int i = 0; i < numero_giocatori; i++) {
        printf("\n--- Configurazione Giocatore %d ---\n", i + 1);
        giocatori[i] = (struct Giocatore*) malloc(sizeof(struct Giocatore));
        if (!giocatori[i]) exit(1);

        giocatori[i]->mondo = 0; 
        giocatori[i]->pos_mondoreale = NULL;
        giocatori[i]->pos_soprasotto = NULL;
        for(int k=0; k<3; k++) giocatori[i]->zaino[k] = nessun_oggetto;

        printf("Nome giocatore: ");
        fgets(giocatori[i]->nome, 100, stdin); 
        giocatori[i]->nome[strcspn(giocatori[i]->nome, "\n")] = 0;

        giocatori[i]->attacco_pischico = casuale(1, 20);
        giocatori[i]->difesa_pischica = casuale(1, 20);
        giocatori[i]->fortuna = casuale(1, 20);

        printf("Stats: Atk %d, Def %d, Luck %d\n", 
               giocatori[i]->attacco_pischico, giocatori[i]->difesa_pischica, giocatori[i]->fortuna);

        int scelta_classe = 0;
        printf("Modifiche: 0) No, 1) +3/-3, 2) -3/+3");
        if (!undici_preso) printf(", 3) Undici Special (+4/+4/-7)");
        printf("\nScelta: ");
        scanf("%d", &scelta_classe);
        pulisci_buffer();

        if (scelta_classe == 1) {
            giocatori[i]->attacco_pischico += 3; giocatori[i]->difesa_pischica -= 3;
        } else if (scelta_classe == 2) {
            giocatori[i]->attacco_pischico -= 3; giocatori[i]->difesa_pischica += 3;
        } else if (scelta_classe == 3 && !undici_preso) {
            giocatori[i]->attacco_pischico += 4; giocatori[i]->difesa_pischica += 4;
            giocatori[i]->fortuna -= 7;
            strcpy(giocatori[i]->nome, "Undici VirgolaCinque");
            undici_preso = 1;
        }
    }

    // 2. Menu Creazione Mappa
    int scelta_mappa = 0;
    do {
        printf("\n--- MENU CREAZIONE MAPPA ---\n");
        printf("1) Genera Mappa casuale (15 zone)\n");
        printf("2) Inserisci zona manualmente\n");
        printf("3) Cancella zona\n");
        printf("4) Stampa mappa\n");
        printf("5) Stampa dettaglio zona\n");
        printf("6) Chiudi mappa e termina impostazioni\n");
        printf("Scelta: ");
        scanf("%d", &scelta_mappa);
        pulisci_buffer();

        switch(scelta_mappa) {
            case 1: genera_mappa(); break;
            case 2: inserisci_zona(); break;
            case 3: cancella_zona(); break;
            case 4: stampa_mappa(); break;
            case 5: stampa_zona(); break;
            case 6: chiudi_mappa(); break;
            default: printf("Comando errato.\n");
        }
    } while (!gioco_pronto);
}

void gioca() {
    if (!gioco_pronto) {
        printf("Errore: Gioco non impostato correttamente.\n");
        return;
    }
    printf("Inizio partita! (Funzionalità di gioco da implementare nella prossima fase).\n");
}

void termina_gioco() {
    printf("Uscita...\n");
    dealloca_tutto();
}

void crediti() {
    printf("\n--- CREDITI ---\n");
    printf("Sviluppato da: [Tuo Nome]\n");
}