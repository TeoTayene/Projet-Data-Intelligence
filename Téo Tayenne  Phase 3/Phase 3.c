#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>

// Définir les constantes
#define NBTEMPS 600
#define NB_VALUES 600
#define NB_MOVE 6

double tabTolerance[NB_MOVE] = {1, 1, 1, 1, 1, 1};
double bestTolerance[NB_MOVE];
double bestRate = 0.0;

// Structure pour représenter un modèle de déplacement
struct model {
    int move;
    int lastIndex;
    double v_acc[NBTEMPS];
};

// Structure pour stocker les résultats des classes estimées et réelles
struct ResultClass {
    int realClasses[NB_MOVE * 24];
    int estimateClasses[NB_MOVE * 24];
    int nbTests;
    int totalSuccess[NB_MOVE];
    int totalFails[NB_MOVE];
    int patternMissAttractCount[NB_MOVE];
};

void convertFiletoTablePattern(FILE *file, struct model *models);
void convertFiletoTableTestSet(FILE *file, struct model *models);
double euclideanDistance(const double *e, const double *p, int length);
double rateSuccess(struct model pattern[], struct model test[], struct ResultClass *res);
double toleranceRate(struct ResultClass *res);
void displayStats(struct ResultClass *results);
int findClosestModel(struct model *models, double *testValues, double *rangsValues);
void filter(struct model *modelsPattern, struct model *modelsTest, struct ResultClass *resultClass, double step);
void displayStats(struct ResultClass *results);

void convertFiletoTablePattern(FILE *file, struct model *models) {
    char buffer[(13*NB_VALUES)+10];
    int index = 0;

    while (fgets(buffer, sizeof(buffer), file) != NULL && index < NB_MOVE) {
        char *token = strtok(buffer, ",");
        if (token != NULL) {
            //models[index].move = atoi(token);
            sscanf(token, "%d", &models[index].move);
            int valIndex = 0;

            while ((token = strtok(NULL, ",")) != NULL && valIndex < NB_VALUES) {
                //models[index].v_acc[valIndex++] = atof(token);
                sscanf(token, "%lf", &models[index].v_acc[valIndex]);
            	valIndex++;
			}
        }
        index++;
    }
}

void convertFiletoTableTestSet(FILE *file, struct model *models) {
    char buffer[(13*3500)+30];
    int index = 0;
	fgets(buffer, sizeof(buffer), file);

	printf(" 2.1 ");
    while (fgets(buffer, sizeof(buffer), file) && index < 360) {
        char *token = strtok(buffer, ",");
        if (token != NULL) {
            //models[index].move = atoi(token);
            int nbMove;
            sscanf(token, "%d", &nbMove);
            int subIndex;
            int valIndex = 0;
            
            token = strtok(NULL, ",");
            token = strtok(NULL, ",");
            
            sscanf(token, "%d", &subIndex);
            models[((nbMove-1)*24) +subIndex-1].move = nbMove;
            
            
			valIndex = models[((nbMove-1)*24) +subIndex-1].lastIndex;
            while ((token = strtok(NULL, ",")) != NULL && valIndex < NB_VALUES) {
            //printf("  |--| %d            (Move: %d Sub :%d (%d)                  fichier -> %d\n",((nbMove-1)*24) +subIndex, nbMove,subIndex,valIndex,index);	
                //models[index].v_acc[valIndex++] = atof(token);
                sscanf(token, "%lf", &models[((nbMove-1)*24) +subIndex-1].v_acc[valIndex]);
            	valIndex++;
			}
			valIndex = models[((nbMove-1)*24) +subIndex-1].lastIndex= valIndex;
        }
        else {
				printf("\n Stop at %d \n",index);
			}
        index++;
    }
}

double euclideanDistance(const double *e, const double *p, int length) {
    double sum = 0.0;
    for (int i = 0; i < length; i++) {
        double diff = e[i] - p[i];
        sum += diff * diff;
    }
    return sqrt(sum);
}

int findClosestModel(struct model *models, double *testValues, double *rangsValues) {
    int closestIndex = -1;
    double minDistance = INFINITY;

    for (int i = 0; i < NB_MOVE; i++) {
        double distance = euclideanDistance(models[i].v_acc, testValues, NB_VALUES) * tabTolerance[i];
        rangsValues[i] = distance;
        if (distance < minDistance) {
            minDistance = distance;
            closestIndex = i;
        }
    }
    return closestIndex + 1;
}

double rateSuccess(struct model pattern[], struct model test[], struct ResultClass *res) {
    int totalSuccess = 0;
    int totalFails = 0;

    memset(res->totalSuccess, 0, sizeof(res->totalSuccess));
    memset(res->totalFails, 0, sizeof(res->totalFails));
    memset(res->patternMissAttractCount, 0, sizeof(res->patternMissAttractCount));

    for (int i = 0; i < NB_MOVE * 24; i++) {
        double rangsValues[NB_MOVE];
        int guessedPattern = findClosestModel(pattern, test[i].v_acc, rangsValues);

        res->realClasses[i] = test[i].move;
        res->estimateClasses[i] = guessedPattern;

        if (guessedPattern == test[i].move) {
            res->totalSuccess[test[i].move - 1]++;
            totalSuccess++;
        } else {
            res->totalFails[test[i].move - 1]++;
            res->patternMissAttractCount[guessedPattern - 1]++;
            totalFails++;
        }
    }

    res->nbTests = totalSuccess + totalFails;
    return ((double) totalSuccess / res->nbTests) * 100;
}

double toleranceRate(struct ResultClass *res) {
    if (res->nbTests == 0) return 0.0;

    int count = 0;
    for (int i = 0; i < res->nbTests; i++) {
        if (res->realClasses[i] != res->estimateClasses[i]) {
            count++;
        }
    }
    return (double)count / res->nbTests * 100.0;
}

void filter(struct model *modelsPattern, struct model *modelsTest, struct ResultClass *resultClass, double step) {
    double loopMax = 1.5;
    double margin = step * loopMax;
    double start[NB_MOVE], end[NB_MOVE];

    for (int i = 0; i < NB_MOVE; i++) {
        start[i] = tabTolerance[i] - margin;
        end[i] = tabTolerance[i] + margin;
    }

    double move[NB_MOVE];
    for (move[0] = start[0]; move[0] <= end[0]; move[0] += step) {
        for (move[1] = start[1]; move[1] <= end[1]; move[1] += step) {
            for (move[2] = start[2]; move[2] <= end[2]; move[2] += step) {
                for (move[3] = start[3]; move[3] <= end[3]; move[3] += step) {
                    for (start[4]; move[4] <= end[4]; move[4] += step) {
                        for (move[5] = start[5]; move[5] <= end[5]; move[5] += step) {
                            for (int i = 0; i < NB_MOVE; i++) {
                                tabTolerance[i] = move[i];
                            }

                            double taux = rateSuccess(modelsPattern, modelsTest, resultClass);
                            if (taux > bestRate) {
                                bestRate = taux;
                                memcpy(bestTolerance, tabTolerance, sizeof(tabTolerance));
                            }
                        }
                    }
                }
            }
        }
    }

    memcpy(tabTolerance, bestTolerance, sizeof(tabTolerance));
    rateSuccess(modelsPattern, modelsTest, resultClass);
}

void displayStats(struct ResultClass *results) {
    if (!results) {
        printf("Erreur: Pas de données de résultats fournies.\n");
        return;
    }

    int globalSuccess = 0;
    int globalFail = 0;
    int globalTotal = 0;

    printf("Statistiques :\n");
    printf("--------------\n");
    for (int i = 0; i < NB_MOVE; i++) {
        int total = results->totalSuccess[i] + results->totalFails[i];
        double pourcentage = total != 0 ? (double)results->totalSuccess[i] / total * 100 : 0;

        printf("Mouvement %d :\n", i + 1);
        printf("  - Succes : %d\n", results->totalSuccess[i]);
        printf("  - echecs : %d\n", results->totalFails[i]);
        printf("  - Total : %d\n", total);
        printf("  - Pourcentage de succes : %.2f%%\n", pourcentage);
        printf("  - Attraction : %d\n", results->patternMissAttractCount[i]);
        printf("\n");

        globalSuccess += results->totalSuccess[i];
        globalFail += results->totalFails[i];
        globalTotal += total;
    }

    double pourcentageGlobal = globalTotal != 0 ? (double)globalSuccess / globalTotal * 100 : 0;
    printf("Taux de succes global : %.2f%% (%d/%d)\n", pourcentageGlobal, globalSuccess, globalTotal);
}


int main() {
    FILE *fTestSet = fopen("testSet.csv", "r");
    FILE *fFiModele = fopen("fiModele.csv", "r");

    if (!fTestSet || !fFiModele) {
        if (!fTestSet) printf("Impossible d'ouvrir testSet.csv\n");
        if (!fFiModele) printf("Impossible d'ouvrir fiModele.csv\n");
        return -1;
    }

    struct model modelsPattern[NB_MOVE];
    convertFiletoTablePattern(fFiModele, modelsPattern);

    struct model modelsTest[NB_MOVE * 24];
    convertFiletoTableTestSet(fTestSet, modelsTest);

    struct ResultClass resultats;
    filter(modelsPattern, modelsTest, &resultats, 0.1);
    displayStats(&resultats);

    fclose(fTestSet);
    fclose(fFiModele);

    return 0;
}
