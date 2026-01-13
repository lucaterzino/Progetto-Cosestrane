#include "gamelib.h"

// --- VARIABILI GLOBALI STATICHE ---
static struct Giocatore* giocatori[4] = {NULL, NULL, NULL, NULL};
static int numero_giocatori = 0;

static struct Zona_mondoreale* prima_zona_mondoreale = NULL;
static struct Zona_soprasotto* prima_zona_soprasotto = NULL;

static int undici_preso = 0;
static int gioco_pronto = 0;
static int gioco_terminato = 0;

// Albo d'oro per i crediti (ultimi 3 vincitori)
static char albo_doro[3][100] = {"-", "-", "-"};

// --- PROTOTIPI FUNZIONI INTERNE ---
static void menu_turno_giocatore(struct Giocatore* g);
static void avanza(struct Giocatore* g, int* azione_eseguita);
static void indietreggia(struct Giocatore* g, int* azione_eseguita);
static void cambia_mondo(struct Giocatore* g, int* azione_eseguita);
static void combatti(struct Giocatore* g);
static void stampa_giocatore(struct Giocatore* g);
static void stampa_zona(struct Giocatore* g);
static void raccogli_oggetto(struct Giocatore* g);
static void utilizza_oggetto(struct Giocatore* g);
static void passa(struct Giocatore* g);

// --- FUNZIONI DI UTILITÀ ---

static int casuale(int min, int max) {
    return rand() % (max - min + 1) + min;
}

static void pulisci_buffer() {
    while (getchar() != '\n');
}

static void aggiungi_vincitore(const char* nome) {
    // Shift dei vincitori (il più vecchio esce)
    strcpy(albo_doro[2], albo_doro[1]);
    strcpy(albo_doro[1], albo_doro[0]);
    strcpy(albo_doro[0], nome);
}

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
    gioco_terminato = 0;
    printf("Memoria liberata.\n");
}

static struct Zona_mondoreale* ottieni_zona_mr(int indice) {
    struct Zona_mondoreale* p = prima_zona_mondoreale;
    int count = 0;
    while (p != NULL && count < indice) { p = p->avanti; count++; }
    return p;
}

static int conta_zone() {
    int count = 0;
    struct Zona_mondoreale* p = prima_zona_mondoreale;
    while (p != NULL) { count++; p = p->avanti; }
    return count;
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

static const char* nome_oggetto(Tipo_oggetto t) {
    switch(t) {
        case nessun_oggetto: return "Vuoto";
        case bicicletta: return "Bicicletta";
        case maglietta_fuocoinferno: return "Maglietta Hellfire";
        case bussola: return "Bussola";
        case schitarrata_metallica: return "Schitarrata Metallica";
        default: return "Ignoto";
    }
}

// --- FUNZIONI CREAZIONE MAPPA ---

static void genera_mappa() {
    if (prima_zona_mondoreale != NULL) dealloca_mappa();
    struct Zona_mondoreale* coda_mr = NULL;
    struct Zona_soprasotto* coda_ss = NULL;

    for (int i = 0; i < 15; i++) {
        struct Zona_mondoreale* nuova_mr = (struct Zona_mondoreale*) malloc(sizeof(struct Zona_mondoreale));
        struct Zona_soprasotto* nuova_ss = (struct Zona_soprasotto*) malloc(sizeof(struct Zona_soprasotto));
        Tipo_zona tipo = (Tipo_zona) casuale(0, 9);
        nuova_mr->tipo = tipo; nuova_ss->tipo = tipo;

        // Generazione nemici MR
        int r_nem = casuale(1, 100);
        if (r_nem <= 60) nuova_mr->nemico = nessun_nemico;
        else if (r_nem <= 90) nuova_mr->nemico = democane;
        else nuova_mr->nemico = billi;

        // Generazione nemici SS (Escludiamo Demotorzone qui)
        r_nem = casuale(1, 100);
        if (r_nem <= 60) nuova_ss->nemico = nessun_nemico;
        else nuova_ss->nemico = democane; 

        // Generazione Oggetti MR
        if (casuale(0, 1) == 0) nuova_mr->oggetto = nessun_oggetto;
        else nuova_mr->oggetto = (Tipo_oggetto) casuale(1, 4);

        nuova_mr->link_soprasotto = nuova_ss; nuova_ss->link_mondoreale = nuova_mr;
        nuova_mr->avanti = NULL; nuova_ss->avanti = NULL;

        if (prima_zona_mondoreale == NULL) {
            prima_zona_mondoreale = nuova_mr; prima_zona_soprasotto = nuova_ss;
            nuova_mr->indietro = NULL; nuova_ss->indietro = NULL;
        } else {
            coda_mr->avanti = nuova_mr; nuova_mr->indietro = coda_mr;
            coda_ss->avanti = nuova_ss; nuova_ss->indietro = coda_ss;
        }
        coda_mr = nuova_mr; coda_ss = nuova_ss;
    }

    // PIAZZAMENTO UNICO DEMOTORZONE
    int indice_boss = casuale(0, 14);
    struct Zona_soprasotto* cursore = prima_zona_soprasotto;
    for (int k = 0; k < indice_boss; k++) {
        cursore = cursore->avanti;
    }
    cursore->nemico = demotorzone;

    printf("Mappa generata (15 zone). Il Demotorzone si nasconde nell'oscurita'...\n");
}

static void inserisci_zona() {
    int posizione;
    int num_zone = conta_zone();
    printf("Posizione (0 - %d): ", num_zone);
    scanf("%d", &posizione); pulisci_buffer();
    if (posizione < 0 || posizione > num_zone) return;

    struct Zona_mondoreale* nuova_mr = (struct Zona_mondoreale*) malloc(sizeof(struct Zona_mondoreale));
    struct Zona_soprasotto* nuova_ss = (struct Zona_soprasotto*) malloc(sizeof(struct Zona_soprasotto));

    printf("Tipo Zona (0-9): "); int t; scanf("%d", &t); nuova_mr->tipo = (Tipo_zona)t; nuova_ss->tipo = (Tipo_zona)t;
    
    printf("Nemico MR (0=Nessuno, 1=Billi, 2=Democane): "); scanf("%d", &t); 
    if(t==1) nuova_mr->nemico = billi; else if(t==2) nuova_mr->nemico = democane; else nuova_mr->nemico = nessun_nemico;
    
    printf("Oggetto MR (0-4): "); scanf("%d", &t); nuova_mr->oggetto = (Tipo_oggetto)t;
    
    printf("Nemico SS (0=Nessuno, 2=Democane, 3=Demotorzone): "); scanf("%d", &t);
    if(t==2) nuova_ss->nemico = democane; else if(t==3) nuova_ss->nemico = demotorzone; else nuova_ss->nemico = nessun_nemico;
    pulisci_buffer();

    nuova_mr->link_soprasotto = nuova_ss; nuova_ss->link_mondoreale = nuova_mr;

    if (posizione == 0) {
        nuova_mr->avanti = prima_zona_mondoreale; nuova_ss->avanti = prima_zona_soprasotto;
        nuova_mr->indietro = NULL; nuova_ss->indietro = NULL;
        if (prima_zona_mondoreale) { prima_zona_mondoreale->indietro = nuova_mr; prima_zona_soprasotto->indietro = nuova_ss; }
        prima_zona_mondoreale = nuova_mr; prima_zona_soprasotto = nuova_ss;
    } else {
        struct Zona_mondoreale* prec_mr = ottieni_zona_mr(posizione - 1);
        struct Zona_soprasotto* prec_ss = prec_mr->link_soprasotto;
        
        nuova_mr->avanti = prec_mr->avanti; nuova_ss->avanti = prec_ss->avanti;
        nuova_mr->indietro = prec_mr; nuova_ss->indietro = prec_ss;
        
        if (prec_mr->avanti) { prec_mr->avanti->indietro = nuova_mr; prec_ss->avanti->indietro = nuova_ss; }
        prec_mr->avanti = nuova_mr; prec_ss->avanti = nuova_ss;
    }
    printf("Zona inserita.\n");
}

static void cancella_zona() {
    int posizione;
    int num_zone = conta_zone();
    if (num_zone == 0) return;
    printf("Posizione da cancellare (0 - %d): ", num_zone - 1);
    scanf("%d", &posizione); pulisci_buffer();
    if (posizione < 0 || posizione >= num_zone) return;

    struct Zona_mondoreale* del_mr = ottieni_zona_mr(posizione);
    struct Zona_soprasotto* del_ss = del_mr->link_soprasotto;

    if (del_mr->indietro) { del_mr->indietro->avanti = del_mr->avanti; del_ss->indietro->avanti = del_ss->avanti; }
    else { prima_zona_mondoreale = del_mr->avanti; prima_zona_soprasotto = del_ss->avanti; }

    if (del_mr->avanti) { del_mr->avanti->indietro = del_mr->indietro; del_ss->avanti->indietro = del_ss->indietro; }

    free(del_mr); free(del_ss);
    printf("Zona cancellata.\n");
}

static void stampa_mappa_debug() {
    if (!prima_zona_mondoreale) { printf("Mappa vuota.\n"); return; }
    int scelta; printf("1) MR 2) SS: "); scanf("%d", &scelta); pulisci_buffer();
    if (scelta == 1) {
        struct Zona_mondoreale* p = prima_zona_mondoreale; int i = 0;
        while (p) { printf("[%d] %s | N: %s | O: %s\n", i++, nome_zona(p->tipo), nome_nemico(p->nemico), nome_oggetto(p->oggetto)); p = p->avanti; }
    } else {
        struct Zona_soprasotto* p = prima_zona_soprasotto; int i = 0;
        while (p) { printf("[%d] %s | N: %s\n", i++, nome_zona(p->tipo), nome_nemico(p->nemico)); p = p->avanti; }
    }
}

static void stampa_dettaglio_zona() {
    int posizione; printf("Indice: "); scanf("%d", &posizione); pulisci_buffer();
    struct Zona_mondoreale* p = ottieni_zona_mr(posizione);
    if (!p) return;
    printf("Zona %d: %s\nMR: %s, %s\nSS: %s\n", posizione, nome_zona(p->tipo), nome_nemico(p->nemico), nome_oggetto(p->oggetto), nome_nemico(p->link_soprasotto->nemico));
}

static void chiudi_mappa() {
    int n_zone = conta_zone();
    if (n_zone < 15) { printf("Errore: Servono almeno 15 zone.\n"); return; }
    int demo = 0;
    struct Zona_soprasotto* p = prima_zona_soprasotto;
    while (p) { if (p->nemico == demotorzone) demo++; p = p->avanti; }
    if (demo != 1) { printf("Errore: Deve esserci esattamente 1 Demotorzone (trovati: %d).\n", demo); return; }
    gioco_pronto = 1; printf("Mappa chiusa. Gioco pronto!\n");
}

// --- HELPER COMBATTIMENTO E GIOCO ---

static void rimuovi_giocatore(struct Giocatore* g) {
    printf("\n☠️  %s E' MORTO! ☠️\n", g->nome);
    
    int giocatori_vivi = 0;
    for (int i = 0; i < 4; i++) {
        if (giocatori[i] == g) {
            free(giocatori[i]);
            giocatori[i] = NULL;
        } else if (giocatori[i] != NULL) {
            giocatori_vivi++;
        }
    }

    if (giocatori_vivi == 0) {
        printf("Tutti i giocatori sono periti nel Sottosopra. GAME OVER.\n");
        gioco_terminato = 1;
    }
}

static int utilizza_oggetto_logic(struct Giocatore* g, int* bonus_attacco, int* bonus_difesa, int* hp_recupero, int in_combattimento) {
    printf("\n--- ZAINO ---\n");
    int count_oggetti = 0;
    for (int i = 0; i < 3; i++) {
        printf("%d) %s\n", i + 1, nome_oggetto(g->zaino[i]));
        if (g->zaino[i] != nessun_oggetto) count_oggetti++;
    }
    
    if (count_oggetti == 0) {
        printf("Lo zaino è vuoto o contiene solo cianfrusaglie inutili.\n");
        return 0; 
    }

    printf("Scegli oggetto da usare (0 per annullare): ");
    int scelta;
    scanf("%d", &scelta); pulisci_buffer();

    if (scelta < 1 || scelta > 3 || g->zaino[scelta-1] == nessun_oggetto) return 0;

    Tipo_oggetto obj = g->zaino[scelta-1];

    switch (obj) {
        case maglietta_fuocoinferno:
            if(in_combattimento) {
                printf("Indossi la Maglietta Hellfire! (+5 Difesa)\n");
                *bonus_difesa += 5;
            } else {
                printf("Indossi la maglietta. Ti senti molto 'metal', ma non succede nulla di pratico.\n");
            }
            break;
        case schitarrata_metallica:
            if(in_combattimento) {
                printf("SUONI UN ASSOLO LEGGENDARIO! (+10 Attacco)\n");
                *bonus_attacco += 10;
                g->zaino[scelta-1] = nessun_oggetto; // Si consuma
                return 1;
            } else {
                printf("Suoni un assolo nel nulla. Gli scoiattoli scappano terrorizzati.\n");
            }
            break;
        case bicicletta:
            if(in_combattimento) {
                 printf("Usi la bicicletta per schivare e recuperare fiato! (+10 HP)\n");
                 *hp_recupero += 10;
                 return 1;
            } else {
                 printf("Fai un giro in bici. La tua condizione fisica migliora leggermente. (Solo scenico)\n");
            }
            break;
        case bussola:
            printf("La bussola gira impazzita... indica che il Demotorzone è da qualche parte nel Soprasotto.\n");
            break;
        default:
            printf("Oggetto non utilizzabile.\n");
    }
    return 1; 
}

// --- FUNZIONI DI GIOCO ---

static void avanza(struct Giocatore* g, int* azione_eseguita) {
    if (*azione_eseguita) { printf("Hai già eseguito un'azione di movimento in questo turno!\n"); return; }
    
    Tipo_nemico nemico_presente;
    if (g->mondo == 0) nemico_presente = g->pos_mondoreale->nemico;
    else nemico_presente = g->pos_soprasotto->nemico;

    if (nemico_presente != nessun_nemico) {
        printf("Non puoi avanzare! C'è un nemico (%s) che ti blocca.\n", nome_nemico(nemico_presente));
        return;
    }

    if (g->mondo == 0) { 
        if (g->pos_mondoreale->avanti == NULL) {
            printf("Sei all'ultima zona, non puoi avanzare oltre!\n");
        } else {
            g->pos_mondoreale = g->pos_mondoreale->avanti;
            g->pos_soprasotto = g->pos_soprasotto->avanti;
            printf("%s avanza alla zona successiva (%s).\n", g->nome, nome_zona(g->pos_mondoreale->tipo));
            *azione_eseguita = 1;
        }
    } else { 
        if (g->pos_soprasotto->avanti == NULL) {
            printf("Sei all'ultima zona, non puoi avanzare oltre!\n");
        } else {
            g->pos_soprasotto = g->pos_soprasotto->avanti;
            g->pos_mondoreale = g->pos_mondoreale->avanti;
            printf("%s avanza alla zona successiva (%s).\n", g->nome, nome_zona(g->pos_soprasotto->tipo));
            *azione_eseguita = 1;
        }
    }
}

static void indietreggia(struct Giocatore* g, int* azione_eseguita) {
    if (*azione_eseguita) { printf("Hai già eseguito un'azione di movimento in questo turno!\n"); return; }

    Tipo_nemico nemico_presente;
    if (g->mondo == 0) nemico_presente = g->pos_mondoreale->nemico;
    else nemico_presente = g->pos_soprasotto->nemico;

    if (nemico_presente != nessun_nemico) {
        printf("Non puoi indietreggiare! C'è un nemico (%s) che ti blocca.\n", nome_nemico(nemico_presente));
        return;
    }

    if (g->mondo == 0) {
        if (g->pos_mondoreale->indietro == NULL) {
            printf("Sei all'inizio, non puoi indietreggiare!\n");
        } else {
            g->pos_mondoreale = g->pos_mondoreale->indietro;
            g->pos_soprasotto = g->pos_soprasotto->indietro;
            printf("%s torna indietro alla zona precedente (%s).\n", g->nome, nome_zona(g->pos_mondoreale->tipo));
            *azione_eseguita = 1;
        }
    } else {
        if (g->pos_soprasotto->indietro == NULL) {
            printf("Sei all'inizio, non puoi indietreggiare!\n");
        } else {
            g->pos_soprasotto = g->pos_soprasotto->indietro;
            g->pos_mondoreale = g->pos_mondoreale->indietro;
            printf("%s torna indietro alla zona precedente (%s).\n", g->nome, nome_zona(g->pos_soprasotto->tipo));
            *azione_eseguita = 1;
        }
    }
}

static void cambia_mondo(struct Giocatore* g, int* azione_eseguita) {
    if (*azione_eseguita) { printf("Hai già mosso in questo turno!\n"); return; }

    if (g->mondo == 0) { 
        if (g->pos_mondoreale->nemico != nessun_nemico) {
            printf("Non puoi cambiare mondo! Devi prima sconfiggere il nemico presente.\n");
            return;
        }
        g->mondo = 1;
        printf("%s viene catapultato nel SOPRASOTTO!\n", g->nome);
    } else {
        printf("Tentativo di fuga dal Soprasotto... (Tiro Fortuna)\n");
        int tiro = casuale(1, 20);
        printf("Hai tirato: %d (La tua Fortuna: %d)\n", tiro, g->fortuna);
        
        if (tiro < g->fortuna) {
            g->mondo = 0;
            printf("Successo! Sei tornato nel Mondo Reale.\n");
        } else {
            printf("Fallimento! Rimani intrappolato nel Soprasotto per questo turno.\n");
        }
    }
    *azione_eseguita = 1;
}

static void combatti(struct Giocatore* g) {
    Tipo_nemico nemico;
    void* zona_ptr;
    int is_mondo_reale = (g->mondo == 0);

    if (is_mondo_reale) {
        nemico = g->pos_mondoreale->nemico;
        zona_ptr = g->pos_mondoreale;
    } else {
        nemico = g->pos_soprasotto->nemico;
        zona_ptr = g->pos_soprasotto;
    }

    if (nemico == nessun_nemico) {
        printf("Non c'è nessun nemico qui da combattere.\n");
        return;
    }

    int hp_nemico, attacco_nemico, difesa_nemico;
    const char* nome_n = nome_nemico(nemico);

    switch (nemico) {
        case billi: hp_nemico = 20; attacco_nemico = 5; difesa_nemico = 2; break;
        case democane: hp_nemico = 40; attacco_nemico = 10; difesa_nemico = 5; break;
        case demotorzone: hp_nemico = 80; attacco_nemico = 15; difesa_nemico = 10; break;
        default: return;
    }

    int hp_giocatore = (g->difesa_pischica * 2) + 20;
    int bonus_attacco = 0, bonus_difesa = 0, hp_recupero = 0;

    printf("\n⚔️  INIZIO COMBATTIMENTO CONTRO %s ⚔️\n", nome_n);
    printf("HP Nemico: %d | Tuoi HP: %d\n", hp_nemico, hp_giocatore);

    while (hp_giocatore > 0 && hp_nemico > 0) {
        printf("\n--- SOTTOMENU COMBATTIMENTO ---\n");
        printf("1) Attacco Pischico\n2) Utilizza Oggetto\nScelta: ");
        int sc; scanf("%d", &sc); pulisci_buffer();
        int turno_usato = 0;

        if (sc == 1) {
            int tiro_fortuna = casuale(0, 20);
            int is_critico = (tiro_fortuna < g->fortuna);
            int danno = (g->attacco_pischico + bonus_attacco) - difesa_nemico + casuale(-2, 2);
            if (danno < 0) danno = 0;
            if (is_critico) { printf("✨ COLPO CRITICO! ✨\n"); danno *= 2; }
            hp_nemico -= danno;
            printf("Hai inflitto %d danni a %s!\n", danno, nome_n);
            turno_usato = 1;
        } else if (sc == 2) {
            hp_recupero = 0;
            turno_usato = utilizza_oggetto_logic(g, &bonus_attacco, &bonus_difesa, &hp_recupero, 1);
            hp_giocatore += hp_recupero;
        }

        if (hp_nemico <= 0) break;

        if (turno_usato) {
            int danno_subito = attacco_nemico - (g->difesa_pischica + bonus_difesa) + casuale(0, 5);
            if (danno_subito < 1) danno_subito = 1;
            hp_giocatore -= danno_subito;
            printf("%s attacca! Subisci %d danni. (Tuoi HP: %d)\n", nome_n, danno_subito, hp_giocatore);
        }
    }

    if (hp_giocatore <= 0) {
        rimuovi_giocatore(g);
    } else {
        printf("\n🎉 VITTORIA! Hai sconfitto %s! 🎉\n", nome_n);
        int prob = casuale(1, 100);
        if (prob <= 50) {
            printf("Il nemico svanisce...\n");
            if (is_mondo_reale) ((struct Zona_mondoreale*)zona_ptr)->nemico = nessun_nemico;
            else ((struct Zona_soprasotto*)zona_ptr)->nemico = nessun_nemico;
            if (nemico == demotorzone) {
                printf("\n🏆 HAI SCONFITTO IL BOSS FINALE! VITTORIA! 🏆\n");
                aggiungi_vincitore(g->nome);
                gioco_terminato = 1;
            }
        } else {
            printf("Il nemico è a terra ma il corpo rimane lì.\n");
        }
    }
}

static void stampa_giocatore(struct Giocatore* g) {
    printf("\n--- INFO %s ---\n", g->nome);
    printf("Mondo: %s\n", (g->mondo == 0) ? "Reale" : "Soprasotto");
    printf("Zaino: [1]%s [2]%s [3]%s\n", nome_oggetto(g->zaino[0]), nome_oggetto(g->zaino[1]), nome_oggetto(g->zaino[2]));
    printf("Stats: Atk %d | Def %d | Fortuna %d\n", g->attacco_pischico, g->difesa_pischica, g->fortuna);
}

static void stampa_zona(struct Giocatore* g) {
    printf("\n--- ZONA ATTUALE ---\n");
    if (g->mondo == 0) {
        printf("Luogo: %s (Reale)\n", nome_zona(g->pos_mondoreale->tipo));
        printf("Nemico: %s\n", nome_nemico(g->pos_mondoreale->nemico));
        printf("Oggetto: %s\n", nome_oggetto(g->pos_mondoreale->oggetto));
    } else {
        printf("Luogo: %s (Soprasotto)\n", nome_zona(g->pos_soprasotto->tipo));
        printf("Nemico: %s\n", nome_nemico(g->pos_soprasotto->nemico));
    }
}

static void raccogli_oggetto(struct Giocatore* g) {
    if (g->mondo == 1) { printf("Non ci sono oggetti nel Soprasotto.\n"); return; }
    if (g->pos_mondoreale->oggetto == nessun_oggetto) { printf("Nessun oggetto qui.\n"); return; }
    if (g->pos_mondoreale->nemico != nessun_nemico) { printf("Nemico presente! Sconfiggilo prima.\n"); return; }

    int slot = -1;
    for(int i=0; i<3; i++) if(g->zaino[i] == nessun_oggetto) { slot = i; break; }

    if (slot != -1) {
        g->zaino[slot] = g->pos_mondoreale->oggetto;
        printf("Hai raccolto: %s!\n", nome_oggetto(g->pos_mondoreale->oggetto));
        g->pos_mondoreale->oggetto = nessun_oggetto;
    } else {
        printf("Zaino pieno!\n");
    }
}

static void utilizza_oggetto(struct Giocatore* g) {
    int dummy1=0, dummy2=0, dummy3=0;
    utilizza_oggetto_logic(g, &dummy1, &dummy2, &dummy3, 0);
}

static void passa(struct Giocatore* g) {
    printf("%s passa il turno.\n", g->nome);
}

static void menu_turno_giocatore(struct Giocatore* g) {
    int scelta;
    int fine_turno = 0;
    int movimento_fatto = 0; 

    do {
        int vivo = 0;
        for(int i=0; i<4; i++) if(giocatori[i] == g) vivo = 1;
        if(!vivo) return;

        printf("\n=== TURNO DI %s ===\n", g->nome);
        printf("1) Avanza\n2) Indietreggia\n3) Cambia Mondo\n4) Combatti\n");
        printf("5) Stampa Giocatore\n6) Stampa Zona\n7) Raccogli Oggetto\n");
        printf("8) Utilizza Oggetto\n9) Passa\n");
        printf("Scelta: ");
        scanf("%d", &scelta); pulisci_buffer();

        switch(scelta) {
            case 1: avanza(g, &movimento_fatto); break;
            case 2: indietreggia(g, &movimento_fatto); break;
            case 3: cambia_mondo(g, &movimento_fatto); break;
            case 4: combatti(g); break;
            case 5: stampa_giocatore(g); break;
            case 6: stampa_zona(g); break;
            case 7: raccogli_oggetto(g); break;
            case 8: utilizza_oggetto(g); break;
            case 9: passa(g); fine_turno = 1; break;
            default: printf("Comando non valido.\n");
        }
    } while (!fine_turno && !gioco_terminato);
}

// --- FUNZIONI PUBBLICHE ---

void imposta_gioco() {
    if (numero_giocatori > 0 || prima_zona_mondoreale != NULL) dealloca_tutto();

    printf("\n--- IMPOSTAZIONE GIOCO ---\n");
    do {
        printf("Numero giocatori (1-4): ");
        if (scanf("%d", &numero_giocatori) != 1) { pulisci_buffer(); continue; }
    } while (numero_giocatori < 1 || numero_giocatori > 4);
    pulisci_buffer();

    for (int i = 0; i < numero_giocatori; i++) {
        printf("--- Giocatore %d ---\n", i + 1);
        giocatori[i] = (struct Giocatore*) malloc(sizeof(struct Giocatore));
        giocatori[i]->mondo = 0; 
        for(int k=0; k<3; k++) giocatori[i]->zaino[k] = nessun_oggetto;
        
        printf("Nome: "); fgets(giocatori[i]->nome, 100, stdin); 
        giocatori[i]->nome[strcspn(giocatori[i]->nome, "\n")] = 0;

        giocatori[i]->attacco_pischico = casuale(1, 20);
        giocatori[i]->difesa_pischica = casuale(1, 20);
        giocatori[i]->fortuna = casuale(1, 20);

        printf("Stats: Atk %d, Def %d, Luck %d\n", giocatori[i]->attacco_pischico, giocatori[i]->difesa_pischica, giocatori[i]->fortuna);
        printf("Modifiche: 0) No, 1) +3/-3, 2) -3/+3");
        if (!undici_preso) printf(", 3) Undici Special");
        printf("\nScelta: "); int sc; scanf("%d", &sc); pulisci_buffer();

        if (sc == 1) { giocatori[i]->attacco_pischico += 3; giocatori[i]->difesa_pischica -= 3; }
        else if (sc == 2) { giocatori[i]->attacco_pischico -= 3; giocatori[i]->difesa_pischica += 3; }
        else if (sc == 3 && !undici_preso) {
            giocatori[i]->attacco_pischico += 4; giocatori[i]->difesa_pischica += 4; giocatori[i]->fortuna -= 7;
            strcpy(giocatori[i]->nome, "Undici VirgolaCinque"); undici_preso = 1;
        }
    }

    int sm = 0;
    do {
        printf("\n--- CREAZIONE MAPPA ---\n");
        printf("1) Genera Casuale\n2) Inserisci Zona\n3) Cancella Zona\n4) Stampa\n5) Dettaglio\n6) Chiudi Mappa\nScelta: ");
        scanf("%d", &sm); pulisci_buffer();
        switch(sm) {
            case 1: genera_mappa(); break;
            case 2: inserisci_zona(); break;
            case 3: cancella_zona(); break;
            case 4: stampa_mappa_debug(); break;
            case 5: stampa_dettaglio_zona(); break;
            case 6: chiudi_mappa(); break;
        }
    } while (!gioco_pronto);
}

void gioca() {
    if (!gioco_pronto) { printf("Errore: Gioco non impostato.\n"); return; }
    gioco_terminato = 0;

    for(int i=0; i<numero_giocatori; i++) {
        if(giocatori[i] != NULL) {
            giocatori[i]->pos_mondoreale = prima_zona_mondoreale;
            giocatori[i]->pos_soprasotto = prima_zona_soprasotto;
            giocatori[i]->mondo = 0; 
        }
    }

    printf("\n--- INIZIO PARTITA ---\n");

    int round = 1;
    while (!gioco_terminato) {
        printf("\n=== ROUND %d ===\n", round++);
        
        int ordine[4] = {0, 1, 2, 3};
        for (int i = 0; i < numero_giocatori; i++) {
            int j = casuale(i, numero_giocatori - 1);
            int temp = ordine[i];
            ordine[i] = ordine[j];
            ordine[j] = temp;
        }

        for (int i = 0; i < numero_giocatori; i++) {
            int idx = ordine[i];
            if (giocatori[idx] == NULL) continue;
            menu_turno_giocatore(giocatori[idx]);
            if (gioco_terminato) break;
        }
        
        int vivi = 0;
        for(int k=0; k<numero_giocatori; k++) if(giocatori[k] != NULL) vivi++;
        if(vivi == 0 && numero_giocatori > 0) {
            printf("Tutti morti. Game Over.\n");
            gioco_terminato = 1;
        }
    }
}

void termina_gioco() {
    printf("Arrivederci!\n");
    dealloca_tutto();
}

void crediti() {
    printf("\n--- CREDITI ---\n");
    printf("Sviluppato da: Luca Terzino\n");
    printf("\n--- ALBO D'ORO (Ultimi 3 Vincitori) ---\n");
    for(int i=0; i<3; i++) printf("%d. %s\n", i+1, albo_doro[i]);
}