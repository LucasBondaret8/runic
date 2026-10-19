//Bondaret Lucas 312CB
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
	//Struct care retine succesorul pt fiecare simbol
	char simbol;
	char *succesor;
} regula;

typedef struct {
	//Struct pentru L_System
	char *axioma;
	regula *reguli;
	int nr_reguli;
	char *nume_fisier;
} L_system;

typedef struct imagine imagine;

typedef struct font font;

typedef struct {
	//Struct pentru a retine starea programului
	//at some point si pe care l salvam in stive
	L_system *sistem;
	imagine *image;
	font *the_font;
} stare;

typedef struct istoric {
	//Structul stiva practic
	stare *stare_salvata;
	struct istoric *next;
} istoric;

void curatare(L_system **sistem)
{

	//Functie pentru curatarea la absolut tot ceea
	// ce a fost retinut in sistem si structuri
	if (!*sistem)
		return;
	for (int i = 0; i < (*sistem)->nr_reguli; i++) {
		free((*sistem)->reguli[i].succesor);
	}
	free((*sistem)->axioma);
	if ((*sistem)->nume_fisier)
		free((*sistem)->nume_fisier);
	free((*sistem)->reguli);
	free(*sistem);
	*sistem = NULL;
}

L_system *copy_sistem(L_system *sursa)
{
	//Functie care primeste un sistem si returneaza
	// o copie cu absolut tot ce contine
	if (!sursa)
		return NULL;
	L_system *destinatie = malloc(sizeof(L_system));
	destinatie->nr_reguli = sursa->nr_reguli;
	destinatie->axioma = strdup(sursa->axioma);
	if (sursa->nume_fisier)
		destinatie->nume_fisier = strdup(sursa->nume_fisier);
	else
		destinatie->nume_fisier = NULL;
	destinatie->reguli = malloc(sursa->nr_reguli * sizeof(regula));
	for (int i = 0; i < sursa->nr_reguli; i++) {
		destinatie->reguli[i].simbol = sursa->reguli[i].simbol;
		destinatie->reguli[i].succesor = strdup(sursa->reguli[i].succesor);
	}
	return destinatie;
}

void load_sistem(L_system **sistem, char *nume_fisier)
{
	//Functie pentru incarcarea sistemului
	FILE *f;
	f = fopen(nume_fisier, "rt");
	if (!f) {
		printf("Failed to load %s\n", nume_fisier);
		//Cazul in care fisierul deschis este gol cu mesajul corespunzator
		return;
	}
	curatare(sistem);
	*sistem = malloc(sizeof(L_system));
	(*sistem)->nume_fisier = strdup(nume_fisier);
	char *buffer = malloc(20000 * sizeof(char));
	//Folosesc buffer mai mare ca sa vad mai
	// intai marimea iar apoi aloc in functie de ea
	fscanf(f, "%s", buffer);
	//Citesc axioma
	(*sistem)->axioma = malloc(strlen(buffer) + 1);
	strcpy((*sistem)->axioma, buffer);
	fscanf(f, "%d", &(*sistem)->nr_reguli);
	//Citesc numar de reguli
	(*sistem)->reguli = malloc((*sistem)->nr_reguli * sizeof(regula));
	for (int i = 0; i < (*sistem)->nr_reguli; i++) {
		//Citesc pentru fiecare simbol succesorul lui
		fscanf(f, " %c", &(*sistem)->reguli[i].simbol);
		fscanf(f, "%s", buffer);
		(*sistem)->reguli[i].succesor = malloc(strlen(buffer) + 1);
		strcpy((*sistem)->reguli[i].succesor, buffer);
	}
	fclose(f);
	printf("Loaded %s (L-system with %d rules)\n",
		   nume_fisier, (*sistem)->nr_reguli);
	free(buffer);
}

char *exista_regula(char litera, L_system *sistem)
{
	//Functie ajutatoare pentru functia de derivare,
	// aici imi caut daca exista succesor pentru simbolul respectiv
	for (int i = 0; i < sistem->nr_reguli; i++)
		if (litera == sistem->reguli[i].simbol)
			return sistem->reguli[i].succesor;
	return NULL;
}

void derivare(L_system *sistem, int n)
{
	//Functie de derivare care primeste ca parametru
	// sistemul respectiv si numarul de derivari pe care il face
	char *curent = strdup(sistem->axioma);
	for (int i = 0; i < n; i++) {
		long lungime = 0;
		long lungime_curent = strlen(curent);
		for (long j = 0; j < lungime_curent; j++) {
			//Partea asta calculeaza lungimea viitoare
			// dupa ce a avut loc derivarea
			char *succesor_simbol = exista_regula(curent[j], sistem);
			if (!succesor_simbol)
				lungime++;
			else
				lungime = lungime + strlen(succesor_simbol);
		}
		char *nou = malloc(lungime + 1);
		long pozitie = 0;
		for (long j = 0; j < lungime_curent; j++) {
			//Partea asta compune sirul nou
			// din simbolurile derivate cu ajutorul variabilei
			// "pozitie" care se incrementeaza pe parcurs
			// si tine minte unde in sirul nou sa adauge
			char *succesor_simbol = exista_regula(curent[j], sistem);
			if (succesor_simbol) {
				strcpy(nou + pozitie, succesor_simbol);
				pozitie = pozitie + strlen(succesor_simbol);
			} else {
				nou[pozitie] = curent[j];
				pozitie++;
			}
		}
		nou[pozitie] = '\0';
		free(curent);
		curent = nou;
	}
	printf("%s\n", curent);
	free(curent);

}

void push(istoric **top, stare *stare_noua)
{
	//Functia care face push in stiva practic,
	// primeste adresa capului stivei si face starea noua noul cap
	istoric *nou = malloc(sizeof(istoric));
	nou->stare_salvata = stare_noua;
	nou->next = *top;
	*top = nou;
}

stare *pop(istoric **top)
{
	//Functie de pop in stiva care mi returneaza inapoi doar capu
	if (!*top)
		return NULL;
	istoric *temporar = *top;
	stare *returneaza = temporar->stare_salvata;
	*top = temporar->next;
	free(temporar);
	return returneaza;
}

void clear(istoric **top)
{
	//Functie de clear a stivei, efectiv sterge tot
	while (*top) {
		stare *sis = pop(top);
		curatare(&sis->sistem);
		if (sis->sistem)
			curatare(&sis->sistem);
		free(sis);
	}
}

void LSYSTEM(L_system **sistem, istoric **UNDO,
			 istoric **REDO, char *nume_fisier)
{
	stare *sis = malloc(sizeof(stare));
	//Adaug o stare noua in care clonez prezentu si salvez in "trecut"
	// cum ar veni si sterg si viitorul
	sis->sistem = copy_sistem(*sistem);
	sis->image = NULL;
	sis->the_font = NULL;
	push(UNDO, sis);
	clear(REDO);
	load_sistem(sistem, nume_fisier);
	//A trebuit sa fac functie pentru acest proces
	// ca nu mai aveam loc in main fara sa mi dea
	//eroare la coding style
}

int main(void)
{
	L_system *sistem = NULL;
	char *comanda = malloc(256 * sizeof(char));
	istoric *UNDO = NULL;
	istoric *REDO = NULL;
	while (1) {
		bool ok = 0;
		if (scanf("%s", comanda) == EOF)
			break;
		if (strcmp(comanda, "EXIT") == 0) {
			//Contextul in care comanda este EXIT pur si simplu curatam si iesim
			ok = 1;
			curatare(&sistem);
			clear(&UNDO);
			clear(&REDO);
			break;
		}
		if (strcmp(comanda, "LSYSTEM") == 0) {
			//Contextul in care ni se cere sa adaugam un sistem,
			// citim numele lui si apelam functia load_sistem
			ok = 1;
			char *nume_fisier = malloc(256 * sizeof(char));
			scanf("%s", nume_fisier);
			LSYSTEM(&sistem, &UNDO, &REDO, nume_fisier);
			free(nume_fisier);
		}
		if (strcmp(comanda, "DERIVE") == 0) {
			//Contextul in care ni se cere sa derivam,
			// mai intai verificam daca avem ce, daca avem se
			//apeleaza functia de derivare cu nr de pasi cerut,
			// daca n avem o sa scoata mesaju de eroare corespunzator
			ok = 1;
			int pasi;
			scanf("%d", &pasi);
			if (!sistem)
				printf("No L-system loaded\n");
			else
				derivare(sistem, pasi);
		}
		if (strcmp(comanda, "UNDO") == 0) {//Comanda de UNDO
			ok = 1;
			if (!UNDO)
				printf("Nothing to undo\n");
			else {
				//Salvez starea actuala in REDO si merg inapoi
				//in starea anterioara
				stare *sis_redo = malloc(sizeof(stare));
				sis_redo->sistem = copy_sistem(sistem);
				push(&REDO, sis_redo);
				curatare(&sistem);
				stare *sis_undo = pop(&UNDO);
				sistem = sis_undo->sistem;
				free(sis_undo);
			}
		}
		if (strcmp(comanda, "REDO") == 0) {
			ok = 1;
			if (!REDO)
				printf("Nothing to redo\n");
			else {
				//Spre deosebire de UNDO, aicia salvam mai intai
				//in UNDO si merg in stare anterioara din REDO
				stare *sis_undo = malloc(sizeof(stare));
				sis_undo->sistem = copy_sistem(sistem);
				push(&UNDO, sis_undo);
				curatare(&sistem);
				stare *sis_redo = pop(&REDO);
				sistem = sis_redo->sistem;
				free(sis_redo);
				if (sistem)
					printf("Loaded %s (L-system with %d rules)\n",
						   sistem->nume_fisier, sistem->nr_reguli);
			}
		}
		if (ok == 0) {
			printf("Invalid command\n");
		}
	}
	free(comanda);
	return 0;
}
