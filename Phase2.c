#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define NB_MOVE 6
#define NB_VALUES 600

int main(){
	printf(">> log : Début\n");
	FILE * fTrainSet = fopen( "trainSet.csv", "r" ); // utilisé pour créé les pattern
    FILE * fiModele = fopen( "fiModele.csv", "wb+" );// création du fichier pattern
    
    char ligne[13*1010];//contient une ligne dans document
    double totalValues[NB_MOVE][NB_VALUES] = {0};
    int nbLignesMove[NB_MOVE] = {0};
    

	if ( fTrainSet == NULL) {
        printf( "Cannot open trainSet \n");
        exit( -1 );
    }
	if ( fiModele == NULL) {
        printf( "Cannot open fiModele \n");
        exit( -1 );
    }
    
    printf(">> log : fichiers ouvert\n");
	//fgets dans le vide pour passer la ligne des libellés
	fgets(ligne, sizeof(ligne), fTrainSet);
	
	int iMove = 1;//a acrémenté et quand > que 7 => stop
	int index = 0;// index pour identifer la position dans le fichier
	int iLigne;
	
	printf(">> log : Lancement du processus\n");
	while (iMove <= NB_MOVE && index < 360){
	//fprintf(fiModele,"%d,",iMove);// écrit le nuémro du move du pattern
	printf(">> log : start un move --------------------- %d -----------------------\n", iMove);
	
	
	
		char *token;// 
		
		iLigne = 1; //max 360
		int iOldMove = iMove;
		while (iOldMove == iMove && fgets(ligne, sizeof(ligne), fTrainSet)){ // boucle sur chaque ligne tant que le move ne chage pas 
			printf("Ligne %d| ", iLigne);
			

			double valeurMove;
			token = strtok(ligne, ",");
			sscanf(token, "%lf", &valeurMove);
			printf("Move : %lf", valeurMove);
			token = strtok(NULL, ",");
			if (((int) valeurMove == iMove || (int) valeurMove == iMove+1 ) &&((char) token[0] == 'H' || (char) token[0] == 'F')){ // vérif si la ligne commence par un iMove
				iLigne++;// ligne correct donc ++
				index++;
				
				iMove = (int)valeurMove;
				
				printf("  Genre : %d %d", token[0], 'H');
        		printf("  Genre : %s", token);
        		token = strtok(NULL, ",");
        		//sscanf(token, "%d", &index);
        		//printf("  Index : %d", index);
        		
        		
        		int iValue =0;
        		token = strtok(NULL, ","); // engage la boucle ave un premier strtok pour voir si pas null 
        		while (iValue <NB_VALUES && token != NULL){ //boucle sur chaque valeur tant que pas 600 obtenue 
        			double valueAcc = atof(token);
        			
        			//sscanf(token, "%.10lf", &valueAcc);
        			//printf("  (value : %s => %.10lf )", token, valueAcc);
        			totalValues[iMove-1][iValue] += valueAcc;
        			
        			token = strtok(NULL, ",");
        			iValue++;
				}
				printf("||||%d|||||",iValue);
			}
        	

			printf("\n");
			nbLignesMove[iMove-1] = iLigne; // Au prochain move on enregistre le nombre de ligne du move précedent 
		}
		
		
	
		printf(">> log : Move suivant (%d lignes pour last move [%d]) index %d\n", nbLignesMove[iMove-2],iMove-2,index);
	}
	nbLignesMove[0] --;// il ne lui manque pas la ligne de transition de move et une ligne en trop den passant au move 2 
	nbLignesMove[iMove-1] = iLigne++;// a l'inverse il lui manque 1 ligne prise par 5 et n'a pas de move 7 qui rééquilibre donc ++
	
	int ii =0;
	while (ii< NB_MOVE){
		printf ("%d : %d | ",ii+1,nbLignesMove[ii]);
		ii++;
	}
	printf("\n");
	
	int move = 0; // Remplace i pour les lignes
    while (move < NB_MOVE) {
    	printf("\nMove -------------------------\n");
    	fprintf(fiModele,"%d",move+1);
        int value = 0; // Remplace j pour les colonnes
        while (value < NB_VALUES) {
            // Affiche la valeur actuelle
            fprintf(fiModele,",%.10lf", totalValues[move][value]/nbLignesMove[move]);
            value++;
        }
        // Passe à la ligne suivante
        fprintf(fiModele,"\n");
        move++;
    }

	printf(">> log : Fin\n");

	fclose(fiModele);
	fclose(fTrainSet);
	return 0;
}
