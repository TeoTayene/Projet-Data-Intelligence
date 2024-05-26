#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define NBTEMPS 601
#define NBMOVES 6
#define MAXLINE 1024

struct model {
    int move;
    double v_acc[NBTEMPS];
};

void convertFiletoTable(FILE *pFiModel, struct model models[NBMOVES]) {
    char line[MAXLINE];
    int i = 0;

    while (fgets(line, MAXLINE, pFiModel) != NULL && i < NBMOVES) {
        char *token = strtok(line, ",");
        models[i].move = atoi(token);
        for (int j = 0; j < NBTEMPS; j++) {
            token = strtok(NULL, ",");
            models[i].v_acc[j] = atof(token);
        }
        i++;
    }
}

double calculateEuclideanDistance(double *e, double *p, int length) {
    double sum = 0.0;
    for (int i = 0; i < length; i++) {
        sum += (e[i] - p[i]) * (e[i] - p[i]);
    }
    return sqrt(sum);
}

void classify(FILE *pFiTest, struct model models[NBMOVES], int *realClasses, int *estimateClasses, int *nbTests) {
    char line[MAXLINE];
    int testIndex = 0;

    while (fgets(line, MAXLINE, pFiTest) != NULL) {
        double v_acc[NBTEMPS];
        char *token = strtok(line, ",");
        realClasses[testIndex] = atoi(token);

        for (int i = 0; i < NBTEMPS; i++) {
            token = strtok(NULL, ",");
            v_acc[i] = atof(token);
        }

        double minDistance = calculateEuclideanDistance(v_acc, models[0].v_acc, NBTEMPS);
        estimateClasses[testIndex] = models[0].move;

        for (int i = 1; i < NBMOVES; i++) {
            double distance = calculateEuclideanDistance(v_acc, models[i].v_acc, NBTEMPS);
            if (distance < minDistance) {
                minDistance = distance;
                estimateClasses[testIndex] = models[i].move;
            }
        }

        testIndex++;
    }
    *nbTests = testIndex;
}

void displayResultsForEachClass(int *realClasses, int *estimateClasses, int nbTests) {
    int correctCounts[NBMOVES] = {0};
    int totalCounts[NBMOVES] = {0};

    for (int i = 0; i < nbTests; i++) {
        totalCounts[realClasses[i] - 1]++;
        if (realClasses[i] == estimateClasses[i]) {
            correctCounts[realClasses[i] - 1]++;
        }
    }

    printf("Results for each class:\n");
    for (int i = 0; i < NBMOVES; i++) {
        printf("Class %d: %d/%d correct (%.2f%%)\n", i + 1, correctCounts[i], totalCounts[i],
               100.0 * correctCounts[i] / totalCounts[i]);
    }
}

void displayAccuracy(int *realClasses, int *estimateClasses, int nbTests) {
    int correct = 0;
    for (int i = 0; i < nbTests; i++) {
        if (realClasses[i] == estimateClasses[i]) {
            correct++;
        }
    }
    printf("Overall accuracy: %.2f%%\n", 100.0 * correct / nbTests);
}

void displayClass(int *realClasses, int *estimateClasses, int nbTests) {
    printf("Real vs Estimated Classes:\n");
    for (int i = 0; i < nbTests; i++) {
        printf("Real: %d, Estimated: %d\n", realClasses[i], estimateClasses[i]);
    }
}

int main() {
    FILE *pFiModel = fopen("fiModel.csv", "r");
    FILE *pFiTest = fopen("testSet.csv", "r");

    if (pFiModel && pFiTest) {
        struct model models[NBMOVES];
        convertFiletoTable(pFiModel, models);

        int realClasses[MAXLINE];
        int estimateClasses[MAXLINE];
        int nbTests;

        classify(pFiTest, models, realClasses, estimateClasses, &nbTests);
        displayResultsForEachClass(realClasses, estimateClasses, nbTests);
        displayAccuracy(realClasses, estimateClasses, nbTests);
        displayClass(realClasses, estimateClasses, nbTests);

        fclose(pFiModel);
        fclose(pFiTest);
    } else {
        printf("Erreur d'ouverture de fichier\n");
    }

    return 0;
}
