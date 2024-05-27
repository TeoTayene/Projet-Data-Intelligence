#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>


#define NB_MOVE 6
#define NB_VALUES 600

double tabTolerance[NB_MOVE] =
//{1.17,0.88,1.27,1.27,1.22,1.12};
//{1.070000,0.813000,1.302000,1.302000,1.120000,1.020000};
//{0.996000,0.713000,1.215000,1.215000,1.046000,0.959000};//73.....%
//{1.0050,0.713000,1.215000,1.215000,1.056000,0.970000};//73 avec ajustemenyt main
//{1.007761,0.6860,1.215000,1.2240300,1.062,0.975311333};
{1, 1, 1, 1, 1, 1};
// {1.010778,0.690375,1.216267,1.223939,1.063267,0.977936};//test
double bestTolerance[NB_MOVE];
float bestRate = 0;


double rangsValues[144][6];

struct model
{
int move;
int lastIndex;
double v_acc[NB_VALUES];
};

struct ResultClass
{
	int totalSuccess[NB_MOVE];
	int totalFails[NB_MOVE];
	int patternMissAttractCount[NB_MOVE];
	
};

void convertFiletoTable(FILE *file, struct model *models);
void convertFiletoTable2(FILE *file, struct model *models);
float rateSuccess (struct model  *modelsPattern,struct model  *modelsTest,struct ResultClass *resultClass);
void displayStats(struct ResultClass *results);
void bestFilterFinder(struct model *modelsPattern, struct model *modelsTest, struct ResultClass *resultClass,double step);
double euclideanDistance(const double *v1, const double *v2, int length);
int findClosestModel(struct model *models, double *testValues,double *rangsValues);


int main(){
	
	
	FILE * fTestSet = fopen( "testSet.csv", "r" ); // utilisé pour check les patterns
    FILE * fFiModele = fopen( "fiModele.csv", "r" );// fichier des patterns

	if (!fTestSet || !fFiModele) {
        if (!fTestSet) printf("Cannot open testSet.csv\n");
        if (!fFiModele) printf("Cannot open fiModele.csv\n");
        return -1;
    }
    
    struct model  modelsPattern [NB_MOVE];
	convertFiletoTablePattern(fFiModele,modelsPattern);
	 
	struct model  modelsTest [6*24];
	convertFiletoTableTestSet(fTestSet,modelsTest);
	
				
	struct ResultClass resultClass;

	rateSuccess(modelsPattern, modelsTest, &resultClass);
	displayStats( &resultClass);
	
	printf("\nTapez 'y' pour trouver un filtre qui améliore les résultats ou autre si vous voulez passer\n");
	char response  = getchar();
	
	if (response = 'y') {
	double bastRate =0;
	int stagne =0;
	double step =0.245676;
	
	while (stagne <6){
	
		bestFilterFinder(modelsPattern, modelsTest, &resultClass,step);
		
		double rate = rateSuccess(modelsPattern, modelsTest, &resultClass);
		if (bestRate < rate ){
			bestRate = rate;
			stagne =0;
		}
		else {
			stagne ++;
			if (stagne >3){
				step *= 0.4;
			}
			
		}
    	//displayStats(&resultClass);
	
		step *=0.66667;
	}
	
	displayStats( &resultClass);
	}
	
	
    fclose(fTestSet);
    fclose(fFiModele);

    

	return 0;
}

void bestFilterFinder(struct model *modelsPattern, struct model *modelsTest, struct ResultClass *resultClass,double step) {
    double loopMax = 1.5;  // la vraie valeur est = (n*2)+1
    double margin = step * loopMax;
    double start[NB_MOVE], end[NB_MOVE];

    // Initialisation des bornes de recherche pour chaque tolérance
    int index = 0;
    while (index < NB_MOVE) {
        start[index] = tabTolerance[index] - margin;
        end[index] = tabTolerance[index] + margin;
        index++;
    }

    float bestRate = 0.0; // Assurez-vous que cette variable est correctement gérée
    float move1 = start[0];
    while (move1 <= end[0]) {
        float move2 = start[1];
        while (move2 <= end[1]) {
            float move3 = start[2];
            while (move3 <= end[2]) {
                float move4 = start[3];
                while (move4 <= end[3]) {
                    float move5 = start[4];
                    while (move5 <= end[4]) {
                        float move6 = start[5];
                        while (move6 <= end[5]) {
                            // Assignation des nouvelles valeurs de tolérance
                            tabTolerance[0] = move1;
                            tabTolerance[1] = move2;
                            tabTolerance[2] = move3;
                            tabTolerance[3] = move4;
                            tabTolerance[4] = move5;
                            tabTolerance[5] = move6;

                            // Calcul du taux de succès
                            double rate = rateSuccess(modelsPattern, modelsTest, resultClass);
                            if (rate > bestRate) {
                                bestRate = rate;
                                bestTolerance[0] = move1;
                                bestTolerance[1] = move2;
                                bestTolerance[2] = move3;
                                bestTolerance[3] = move4;
                                bestTolerance[4] = move5;
                                bestTolerance[5] = move6;
                            }

                            move6 += step;
                        }
                        move5 += step;
                    }
                    move4 += step;
                }
                move3 += step;
            }
            move2 += step;
        }
        move1 += step;
    }

    // Affichage des résultats finaux
    printf("+-----------------------------------------------[Best Rate: %.2f%%]--------------------------------------------------+\n", bestRate);
    printf("| Tolerances: {%f, %f, %f, %f, %f, %f}\n", bestTolerance[0], bestTolerance[1], bestTolerance[2], bestTolerance[3], bestTolerance[4], bestTolerance[5]);

	tabTolerance[0] = bestTolerance[0];
    tabTolerance[1] = bestTolerance[1];
    tabTolerance[2] = bestTolerance[2];
    tabTolerance[3] = bestTolerance[3];
    tabTolerance[4] = bestTolerance[4];
    tabTolerance[5] = bestTolerance[5];
    
	rateSuccess (modelsPattern,modelsTest,&resultClass);
}






void displayStats(struct ResultClass *results) {
    int i = 0;
    int globalSuccess = 0;
    int globalFail = 0;
    int globalTotal = 0;

    printf("Move\tSuccess + Fails /Total\t| Percentage\t| Attract\n");
    printf("-----\t-----------------------\t| -----------\t| -------\n");
    while (i < NB_MOVE) {
        int total = results->totalSuccess[i] + results->totalFails[i];
        double percentage = total != 0 ? (double)results->totalSuccess[i] / total * 100 : 0;

        printf("%d\t%d + %d / %d\t\t| %.2f%%\t| Attract %d\n",
               i + 1, results->totalSuccess[i], results->totalFails[i], total, percentage, results->patternMissAttractCount[i]);

        globalSuccess += results->totalSuccess[i];
        globalFail += results->totalFails[i];
        globalTotal += total;
        i++;
    }

    double globalPercentage = globalTotal != 0 ? (double)globalSuccess / globalTotal * 100 : 0;
    printf("\nGlobal Success Rate: %.2f%% (%d/%d)\n", globalPercentage, globalSuccess, globalTotal);
}

float rateSuccess(struct model *modelsPattern, struct model *modelsTest, struct ResultClass *resultClass) {
    int ca = 0;
    while (ca < NB_MOVE) {
        resultClass->totalSuccess[ca] = 0;
        resultClass->totalFails[ca] = 0;
        resultClass->patternMissAttractCount[ca] = 0;
        ca++;
    }

    ca = 0;
    while (ca < 144) {
        int guesPattern = findClosestModel(modelsPattern, modelsTest[ca].v_acc, rangsValues[ca]);
        if (guesPattern == modelsTest[ca].move) {
            resultClass->totalSuccess[modelsTest[ca].move - 1]++;
        } else {
            resultClass->totalFails[modelsTest[ca].move - 1]++;
            resultClass->patternMissAttractCount[guesPattern - 1]++;
        }
        ca++;
    }

    int totalTotalFails = 0;
    int totalTotalSuccess = 0;
    int a = 0;
    while (a < NB_MOVE) {
        totalTotalSuccess += resultClass->totalSuccess[a];
        totalTotalFails += resultClass->totalFails[a];
        a++;
    }

    return ((float)totalTotalSuccess / (totalTotalSuccess + totalTotalFails)) * 100;
}









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

double euclideanDistance(const double *v1, const double *v2, int length) {
    double sum = 0.0;
    int i = 0;
    while ( i < length) {
        double diff = v1[i] - v2[i];
        sum += diff * diff;
        i++;
    }
    return sqrt(sum);
}

int findClosestModel(struct model *models, double *testValues,double *rangsValues) {
    int closestIndex = -1;
    double minDistance = INFINITY; // Utilise INFINITY défini dans <math.h>
	
	int i = 0;
    while( i < NB_MOVE) {
    	//printf("-%d=|",i);
        double distance = euclideanDistance(models[i].v_acc, testValues, NB_VALUES)*tabTolerance[i];
        rangsValues[i] = distance;
        if (distance < minDistance) {
            minDistance = distance;
            closestIndex = i;
            
        }
        i++;
    }
    return closestIndex+1;
}



/*
void displayModels(struct model *models) {
    for (int i = 0; i < NB_MOVE; i++) {
        printf("Move %d: ", models[i].move);
        for (int j = 0; j < NB_VALUES; j++) {
            printf("%lf ", models[i].v_acc[j]);
        }
        printf("\n");
    }
}
*/
