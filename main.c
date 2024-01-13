#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#ifdef _WIN32
#define CLEAR "cls"
#else
#define CLEAR "clear"
#endif

#define NUM_CARS 20

// Struct de data pour voiture
typedef struct Car {
    char *driver;   // etiquette du driver
    int id;         // numéro de la voiture
    int pid;        // processId de la voiture
    int speedCar;   // vitesse de la voiture en km
    int secSpends;  // secondes écoulées en course
    sem_t semaphore; // Sémaphore pour la synchronisation
    struct Car *smAddr;
} Car;

// Fonction pour l'incrémenteur de 0
void *incrementeur(void *arg) {
    Car *car = (Car *) arg;
    int i;
    for (i = 0; i < 5; ++i) {
        // Attente du sémaphore de la voiture actuelle
        sem_wait(&car->semaphore);

        // Section critique (incrémentation de la variable partagée secSpends)
        car->secSpends++;

        // Libération du sémaphore pour permettre au prochain thread de continuer
        sem_post(&car->smAddr->semaphore);

        // Pause de 0.5 seconde
        sleep(1);
    }

    return NULL;
}

// Fonction pour afficher l'état des conducteurs
void afficherEtatConducteurs(Car *voitures, int numCars) {
    system(CLEAR); // Efface l'écran
    printf("|\tConducteur\t|\tIdCar\t|\tDriver\t\t|\tPID\t|\tSpeed\t|\tSpendSec\t|\n");

    for (int i = 0; i < numCars; ++i) {
        printf("|\t%d\t\t|\t%d\t|\t%s\t|\t%d\t|\t%d\t|\t%d\t\t|\n",
               i + 1, voitures[i].id, voitures[i].driver, voitures[i].pid, voitures[i].speedCar, voitures[i].secSpends);
    }

    fflush(stdout);
}

int main() {
    // Initialisation des voitures
    Car voitures[NUM_CARS];

    for (int i = 0; i < NUM_CARS; ++i) {
        char driverName[10];
        sprintf(driverName, "Driver%d", i + 1);
        voitures[i] = (Car) {driverName, i + 1, 100 + i, 40 + i, 0, NULL};
        sem_init(&voitures[i].semaphore, 0, 0);
    }

    // Mise à jour des pointeurs smAddr
    for (int i = 0; i < NUM_CARS; ++i) {
        voitures[i].smAddr = &voitures[(i + 1) % NUM_CARS];
    }

    // Création des threads incrémenteurs pour chaque voiture
    pthread_t threads[NUM_CARS];
    for (int i = 0; i < NUM_CARS; ++i) {
        pthread_create(&threads[i], NULL, incrementeur, &voitures[i]);
    }

    // Libération du premier sémaphore pour démarrer le premier thread
    sem_post(&voitures[0].semaphore);

    // Boucle pour afficher l'état toutes les secondes
    for (int i = 0; i < 5; ++i) {
        afficherEtatConducteurs(voitures, NUM_CARS);
        sleep(1); // Pause d'une seconde
    }

    // Attente de la fin des threads
    for (int i = 0; i < NUM_CARS; ++i) {
        pthread_join(threads[i], NULL);
    }

    // Destruction des sémaphores après utilisation
    for (int i = 0; i < NUM_CARS; ++i) {
        sem_destroy(&voitures[i].semaphore);
    }

    printf("\n"); // Pour passer à la ligne suivante après la fin des threads

    return 0;
}
