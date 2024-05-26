#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

#define XECARTTYPE 0.38875666
#define YECARTTYPE 0.61937128
#define ZECARTTYPE 0.4300345

#define XMOYENNE 0.00096087
#define YMOYENNE 0.05525659
#define ZMOYENNE 0.0352192

// valeur abérente

bool estAberrante(double, double, double);

int main()
{

    // test log
    printf("Debut \n");

    char path[128] = "dws_1/sub_1.csv"; // peut erte ne pas definir plus propre
    char pathsMoves[15][16] = {"dws_1/", "dws_2/", "dws_11/", "jog_9/", "jog_16/", "sit_5/", "sit_13/", "std_6/", "std_14/", "ups_3/", "ups_4/", "ups_12/", "wlk_7/", "wlk_8/", "wlk_15/"};
    char lastMove[4] = "    ";

    int subTab[24][3];

    int moveNumber = 0;
    // data_subjects_info.csv

    char ligne[2048];
    int numeroligne;
    char *token;
    double valeurs[3];
    int nbValeursAberrantes = 0;
    int iacc = 0;
    int iMove = 0;
    int iSub = 0;
    FILE *inputFile[3];

    FILE *valeursAberante = fopen("Valeur_Aberante.csv", "wb+");

    if (valeursAberante == NULL)
    {
        printf("Error: File not found1\n");
        perror("Error");
        return 1;
    }

    while (iMove < 15)
    {

        iSub = 0;

        while (iSub < 24)
        {

            // Création du chemin d'accès

            if (strncmp(pathsMoves[iMove], lastMove, 3) != 0)
            {
                strncpy(lastMove, pathsMoves[iMove], 4);
                moveNumber++;
            }

            sprintf(path, "%ssub_%d.csv ", pathsMoves[iMove], iSub + 1);
            inputFile[0] = fopen(path, "r");
            if (inputFile[0] == NULL)
            {
                printf("Cannot open file \n");
                // exit( 0 );
            }
            puts(path);
            // Lecture du fichier
            fgets(ligne, sizeof(ligne), inputFile[0]); // passer a la ligne
            while (fgets(ligne, sizeof(ligne), inputFile[0]))
            {

                iacc = 0;
                token = strtok(ligne, ",");
                numeroligne = atoi(token);
                while (iacc < 9)
                {

                    token = strtok(NULL, ",");
                    iacc++;
                }

                // Récupération des valeurs
                token = strtok(NULL, ",");
                valeurs[0] = atof(token);
                token = strtok(NULL, ",");
                valeurs[1] = atof(token);
                token = strtok(NULL, ",");
                valeurs[2] = atof(token);

                // Vérification si les valeurs sont aberrantes
                if (estAberrante(valeurs[0], XMOYENNE, XECARTTYPE) || estAberrante(valeurs[1], YMOYENNE, YECARTTYPE) || estAberrante(valeurs[2], ZMOYENNE, ZECARTTYPE))
                {

                    fprintf(valeursAberante, "%s, numéro ligne = %d ,", pathsMoves[iMove], numeroligne);
                    if (estAberrante(valeurs[0], XMOYENNE, XECARTTYPE))
                    {
                        fprintf(valeursAberante, " X :%lf ", valeurs[0]);
                        nbValeursAberrantes++;
                    }
                    if (estAberrante(valeurs[1], YMOYENNE, YECARTTYPE))
                    {
                        fprintf(valeursAberante, " Y : %lf ", valeurs[1]);
                        nbValeursAberrantes++;
                    }
                    if (estAberrante(valeurs[2], ZMOYENNE, ZECARTTYPE))
                    {
                        fprintf(valeursAberante, " Z : %lf.", valeurs[2]);
                        nbValeursAberrantes++;
                    }
                    fprintf(valeursAberante, "\n");
                }
            }

            fclose(inputFile[0]);
            iSub++;
        }
        iMove++;
    }
    printf("Nombre de valeurs aberrantes: %d\n", nbValeursAberrantes);
    fclose(valeursAberante);
    return 0;
}

bool estAberrante(double valeur, double moyenne, double ecartType)
{
    // Calculer le z-score
    double zScore = (valeur - moyenne) / ecartType;

    // Utiliser une table de z-scores pour déterminer si la valeur est aberrante
    return zScore > 3.0 || zScore < -3.0;
}
