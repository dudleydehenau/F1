#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>

// Structure pour stocker le temps généré par chaque processus
struct Affichage {
    int temps_voiture1;
    int temps_voiture2;
};

int nAleatoire() {
    srand(time(NULL));
    return rand() % 21 + 25;
}

int main() {
    key_t key = ftok("/home/dim/OS2/pm.c", 'R');
    if (key == -1) {
        perror("Erreur lors de la création de la clé IPC");
        return 1;
    }

    int shmid = shmget(key, sizeof(struct Affichage), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("Erreur lors de la création du segment de mémoire partagée");
        return 1;
    }

    struct Affichage *shared_affichage = (struct Affichage *)shmat(shmid, NULL, 0);
    if (shared_affichage == (struct Affichage *)(-1)) {
        perror("Erreur lors de l'attachement du segment de mémoire partagée");
        return 1;
    }

    pid_t pid1, pid2;

    pid1 = fork();

    if (pid1 == -1) {
        perror("Erreur lors de la création du processus voiture1");
        return 1;
    } else if (pid1 == 0) {
        // Code exécuté par le processus voiture1
        printf("Je suis voiture1. Mon PID est : %d\n", getpid());
        int temps1 = nAleatoire();
        printf("Temps généré par voiture1 : %d\n", temps1);

        // Stockez le temps généré par voiture1 dans la structure partagée
        shared_affichage->temps_voiture1 = temps1;

        return 0;
    } else {
        sleep(1);
        pid2 = fork();
        if (pid2 == -1) {
            perror("Erreur lors de la création du processus voiture2");
            return 1;
        } else if (pid2 == 0) {
            // Code exécuté par le processus voiture2
            printf("Je suis voiture2. Mon PID est : %d\n", getpid());
            int temps2 = nAleatoire();
            printf("Temps généré par voiture2 : %d\n", temps2);

            // Stockez le temps généré par voiture2 dans la structure partagée
            shared_affichage->temps_voiture2 = temps2;

            return 0;
        } else {
            // Code exécuté par le processus parent
            // Attendez que les processus enfants se terminent
            wait(NULL);
            wait(NULL);

            // Accédez à la structure partagée pour obtenir les temps générés
            printf("Temps généré par voiture1 (depuis le parent) : %d\n", shared_affichage->temps_voiture1);
            printf("Temps généré par voiture2 (depuis le parent) : %d\n", shared_affichage->temps_voiture2);
        }
    }

    // Détachez le segment de mémoire partagée lorsque vous avez terminé
    shmdt(shared_affichage);

    // Supprimez le segment de mémoire partagée
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}

