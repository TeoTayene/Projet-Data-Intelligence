#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h> 
#include <stdbool.h>
#define DATA_SIZE 3

#define AVERAGE_X 0.00096087
#define STANDARD_DEVIATION_X 0.38875666
#define AVERAGE_Y 0.05525659
#define STANDARD_DEVIATION_Y 0.61937128
#define AVERAGE_Z 0.0352192
#define STANDARD_DEVIATION_Z 0.4300345 

#define DATA_ARRAY { \
    {AVERAGE_X, STANDARD_DEVIATION_X}, \
    {AVERAGE_Y, STANDARD_DEVIATION_Y}, \
    {AVERAGE_Z, STANDARD_DEVIATION_Z} \
}

double ABERENT_DATA[DATA_SIZE][2] = DATA_ARRAY;

int nbError = 0;
int iN =0;
int nbVaccIndex =0;

int Vacc [361]={0};

bool isAberrant (double checkValue, int indexAxe);

void showLoading(int indexx, int max){
	printf("\r\t\t\t\tloading [");
	double index = (double)(indexx )/(double)(max) *10;
	int i = 0;
	int valueCara = 254;
	
	while (i<=index){
		printf("%c",valueCara);
		
		i++;
	}
	
	while (i < 10){
		printf(".");
		i++;
	}
	printf("]");
}



//info back up
//affiche tout ce qu'il a dans le premier mouvement de sub 1 a 24
// objectif suivant faire passer d'un dossier mouvement a un autre 
// puis faire en sorte de mettre 10% dans un autre fichier 

//a faire - 10% - check demande dans le dock - transformé les move en chiffre

int main( ) {
    //test log
    printf("\t\t\t\t\tDebut \n");

    char path[128] = "dws_1/sub_1.csv";//peut erte ne pas definir plus propre
    char pathsMoves[15][16] = {"dws_1/","dws_2/","dws_11/","jog_9/","jog_16/","sit_5/","sit_13/","std_6/","std_14/","ups_3/","ups_4/","ups_12/","wlk_7/","wlk_8/","wlk_15/" };
    char lastMove[4]= "    ";//variable qui garde le nom du dernier move pour ppouvoir le comparé au suivant et accrémenter moveNumber

    int subTab[24][4];//SubTab[numéroDuSub][0-3(0:weight,1:height,2:age,3:gender)]
    int moveNumber = 0;//numéro lié au move 1 = dws, 2 = jog,...
    //data_subjects_info.csv

    char ligne[2048];//contient une ligne dans document//meme en mettant a 14000 y a quand meme les retour a la ligne 
    char *token;//utilisé pour découper la ligne a chaque virgule
    double valeurs[3];//contient les valeurs des axes (provenant des découpes strok avec token

    FILE * subFile; //fichier info sub
    FILE * inputFile[3]; // erreur clean code (0: ouvre les fichier sub)(1: change entre newfile et testSet pour faire les 10%)
    FILE * newFile;// trainSet (erreur clean code )
    FILE * testSet;// testSet
    FILE * aberrantValues = fopen( "data_aberrant.csv", "wb+" );// info sur les valeurs aberrantes

    //+++++++++++++++++ patie donnée sub++++++++++++++++++
    //   Lit le fichier comptenant les infos sur les sub et ...
    //    les places dans SubTab[numéroDuSub][0-3(0:weight,1:height,2:age,3:gender)]
    int iSub = 0; 
    subFile = fopen( "data_subjects_info.csv", "r" );

    if ( subFile == NULL) {
        printf( "Cannot open file1 \n");
        exit( -1 );
    }
    
    fgets(ligne, sizeof(ligne), subFile);// passer a la ligne 
    while (fgets(ligne, sizeof(ligne), subFile)){
        
        token = strtok(ligne, ",");
        token = strtok(NULL, ",");
        sscanf(token, "%d", &subTab[iSub][0]);
        token = strtok(NULL, ",");
        sscanf(token, "%d", &subTab[iSub][1]);
        token = strtok(NULL, ",");
        sscanf(token, "%d", &subTab[iSub][2]);
        token = strtok(NULL, ",");
        sscanf(token, "%d", &subTab[iSub][3]);
        token = strtok(NULL, ",");
        
        iSub++;
    }
    iSub=0;
    //++++++++++++++++++++ partie calcul ++++++++++++
    
    //ouverture/création des fichiers a renmplir
    newFile = fopen( "trainSet.csv", "wb+" );
    testSet = fopen( "testSet.csv", "wb+" );
    printf("\t\t\t\t   Ouverture/Cr\x86ation \n");
    if (newFile == NULL || testSet == NULL) {
        printf( "Cannot open file wb+\n");
        exit( -1 );
    }

    //Création des libellé 
    fprintf(newFile,"Mouvement,Genre,Index,Vacceleration\n");
    fprintf(testSet,"Mouvement,Genre,Index,Vacceleration\n");
    fprintf(aberrantValues,"Patch,Taille (lignes),NB aberrante X,NB aberrante Y,NB aberrante Z\n");
    //plus besoin de la lignefputs(ligne, newFile);
    
    int nbLignes = 0;//utiliser pour savoir a la N°eme ligne on est et comparé avec ValeurRechece  (je sais plus trop
    //test log
    printf("\t\t\t\t\tecriture \n");
    
    // boucle sur chaque ligne 
    int iMove; //i pour aller dans chaque 15 dossiers de 24 subs
    int index = 1; // numéro de ligne (pk pas utiliser nblignes ?)
    int oldindex =1;

	fprintf(newFile,"%d,%s,%d",1,subTab[iSub][3] ? "Homme":"Femme",index);
    fprintf(testSet,"%d,%s,%d",1,subTab[iSub][3] ? "Homme":"Femme",index);    
    
    while(iMove<15){
    iSub=0;//le N°eme fichier sub dans le iMove dossier
    
    while (iSub <24){
    
    //nb valeurs fausses dans x y et z
    int nbAberrants[3] = {0,0,0};
    
    // prend le libellé du nouveau dossier move pour voir si on passe a un nouveau
    //  move et accrémenter moveNumber (6 moves diff donc moveNumber va valoir 6 a la fin 
	if (strncmp(pathsMoves[iMove],lastMove , 3) != 0){
        strncpy(lastMove, pathsMoves[iMove], 4);
        moveNumber ++;
    }
    
    // ecriture du path du fichier sub a ouvrir. path = nom dossier move + "sub_" + numéro fichier sub
    sprintf(path, "%ssub_%d.csv ",pathsMoves[iMove], iSub+1);
    inputFile[0] = fopen( path, "r" );
    if (inputFile[0] == NULL) {
        printf( "Cannot open file \n");
        //exit( 0 );
    }
    
    //vvvvvvvvvvvvvvvvv nbLigne vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    // parcourt le fichier a l'envers jusqu'a trouver un '\n' et récupere la valeur écrite apres. Donc l'index de la dernier ligne
    //  fichier sub qui nous donne le nombre de lignes total du fichier
    
    int caractere;
    int ValeurCherche;
    
    int iCara = 91;
    while (ligne[0] != '\\' && iCara < 300){
    fseek(inputFile[0], -iCara, SEEK_END);
    
    caractere = fgetc(inputFile[0]);
    if (caractere == '\n'){
        //printf("((((((((((((((%d))))))))))))))%c))))))))))))",ligne[0],ligne[0]);
        fgets(ligne, sizeof(ligne), inputFile[0]);
        token = strtok(ligne, ",");
        sscanf(token, "%d", &ValeurCherche);
        printf("(%d)", ValeurCherche);
        iCara = 9999;
    }
    //fgets(ligne, sizeof(ligne), fileTest);
    //printf("|%c|",caractere);
    
    iCara++;
    }
    fseek(inputFile[0], 0, SEEK_SET);
    
    nbLignes = -1;
    //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    
    fgets(ligne, sizeof(ligne), inputFile[0]);// lit la ligne contenant les libellés des collones
    
    bool error =false; // error valeur 
    int iV=0; // i vecteur (0:x,1:y,2:z)
    
    
    while (fgets(ligne, sizeof(ligne), inputFile[0]) ){// tant que fgets arrive a lire une lignes dans le fichier
    	int iToken=1; // mettre a 1 pcq quand on va l'utiliser il aura deja accré de 1
        //printf("%s\n", ligne);
        
        
        
        
        //change le ficher d'ecriture si on dépasse les 90% du fichier lu
        //mais changé a 15 % a cause de manque de valeur (valeur abérente (a test ))
    	inputFile[1] = nbLignes < ValeurCherche-((ValeurCherche/100)*10) ? newFile : testSet;
        
        
        token = strtok(ligne, ",");
        
		token = strtok(NULL, ",");

        // boucle sur chaque valeur de la ligne (rempli un tableau qui a cahque 3 fois écrit l'Vacc)
        // si valeur aberrante error=true donc n'écrit pas la valeur dans le fichier 
    
        while (token != NULL ) {
        
		if(iToken >=10) 	
		{
            //ecrit la valeur de token dans "valeurs"
            sscanf(token, "%lf", &valeurs[iV]);
            if (isAberrant(valeurs[iV],iV)&& iToken >=10){
            	nbAberrants[iV] ++;
            	error=true;
			}
            iV++;
            if (iV == 3){
                //fprintf(newFile,",%.6lf,%.6lf,%.6lf,(%.10lf)",valeurs[0],valeurs[1],valeurs[2],sqrt((valeurs[0]*valeurs[0])+(valeurs[1]*valeurs[1])+(valeurs[2]*valeurs[2])));
                if (error)
				{
             		error = false;
				}
				else
				{
					fprintf(inputFile[1],",%.10lf",sqrt((valeurs[0]*valeurs[0])+(valeurs[1]*valeurs[1])+(valeurs[2]*valeurs[2])));
					nbVaccIndex ++;// valeur de test a l'affichage (seulement infomartif)
					Vacc[index]++;// valeur de test a l'affichage (seulement infomartif)
				}
                iV=0;
            
            }
        }
            //fprintf(newFile,",%.6lf",valeurs[iV]);
            token = strtok(NULL,",");
            iToken++;
		//fin de boucle = lie la prochaine valeur de la ligne si il a une valeur  
        
		}
        
        if (oldindex != index){
        fprintf(newFile,"\n",index);
        fprintf(testSet,"\n",index);
        fprintf(newFile,"%d,%s,%d",moveNumber,subTab[iSub][3] ? "Homme":"Femme",iSub+1);
        fprintf(testSet,"%d,%s,%d",moveNumber,subTab[iSub][3] ? "Homme":"Femme",iSub+1);
        iN++;
        oldindex = index;}
    //}
        //fputs(ligne, newFile);
        nbLignes++;
    //fin
    }
    nbError += nbAberrants[0] + nbAberrants[1] +nbAberrants[2];
    fprintf(aberrantValues,"%s,%d,%d,%d,%d\n",path,ValeurCherche,nbAberrants[0],nbAberrants[1],nbAberrants[2]);
    //printf("\tfin ecriture %s (%d/360) => %d lignes[%d] \t%d\n",path,index,nbLignes,ValeurCherche,moveNumber);
    //variante d'afficahge
	showLoading(index, 360);

    fclose( inputFile[0] );
    
    iSub++;
    index++;
    }
    iMove++;
    }
    
    fclose( aberrantValues);
    fclose( newFile );
    fclose( testSet );
    
    printf("\n\t\t\t\t%d valeurs aberrantes\n",nbError);
	printf("\n\t\t\t\t%d valeurs iN\n",iN);
	
	/*
	int iii =0;
	int tot =0;
	while (iii <361){
		printf("%d: %d\t",iii,Vacc[iii]);
		tot += Vacc[iii];
		iii++;
	}
	printf("\t\t\t\t\t%d\n",tot);
	*/
    // test log
    printf("\t\t\t\t\tFIN %d\n",nbVaccIndex);
    return 0;
}


bool isAberrant (double checkValue, int indexAxe){
	//prend checkValue(la valeur a vérifée) et la compare au valeur fournie en constante. 
	//indexAxe vaut entre 0 et 2 (0 = x, 1 = y, 2 = z)
	return (checkValue > ABERENT_DATA[indexAxe][0]+(ABERENT_DATA[indexAxe][1]*3) || checkValue < ABERENT_DATA[indexAxe][0]-(ABERENT_DATA[indexAxe][1]*3));
}

