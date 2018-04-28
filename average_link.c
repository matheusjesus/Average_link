#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define TAM_MAX_LINHA 10000

typedef struct elemento{
	char nome[50];
	float *coord;
	int cluster;
} Elemento;

typedef struct cluster{
	int id;
	int qt_elementos;
	Elemento **elem;
} Cluster;

typedef struct clusters{
	Cluster *c;
	int qt_clusters;
	int total_elementos;
} Clusters;

typedef struct retorno{
	int menor[2];
	float **matriz;
} Retorno;

typedef struct distancias{
	int id1;
	float *distancia;
} Distancias;



int le_arquivo(char *arquivo, Elemento *el);

int conta_linhas(char *arquivo);

Retorno cria_matriz(float **matriz, Elemento *el);

float calc_dist(Elemento el1, Elemento el2);

void merge(int *menor, float **matriz, Elemento *el, Clusters *clus);

void recalcula_dist(float **matriz, Elemento *el, int id, Clusters *clus);

Retorno pega_menor(float **matriz);

void salvar(Elemento *el, char *nome_arquivo);

void salvar_clusters(Clusters clus, char *nome_arquivo);

//Funcoes para debug:
void exibe_matriz(float **matriz);

void exibe_clusters(Clusters clus);

void exibe_elementos(Elemento *el);


int num_el;
int num_coord;

int main(){
	int kmin, kmax, tam_nome, primeiro = 1, exibir =0, passo, porcentagem = 0, nmaxclu = 0, i, salvar_clu;
	char *ptr, nome_arquivo[100], nomeaux[100], semresult[50], nome_salvar[100];
	float **matriz;
	double time_spent;
	Clusters clus;
	Elemento *el;
	Retorno ret;
	clock_t begin = clock();
	clock_t end;
	
	memset(nome_arquivo, '\0', 100);
	memset(nomeaux, '\0', 100);
	memset(semresult, '\0', 50);

	printf("Entre com o Kmin: "); scanf("%d", &kmin);
	printf("Entre com o Kmax: "); scanf("%d", &kmax);
	printf("Entre com o nome do arquivo: "); scanf("%s", nome_arquivo);

	if(kmin > kmax){
		printf("Entrada invalida!");
		return 0;
	}
	
	num_el = conta_linhas(nome_arquivo);
	if(num_el == -1){
		printf("Erro ao abrir arquivo!\n");
		return 0;
	}

	printf("Gostaria de salvar os resultados agrupados por cluster? Digite 0 para nao, 1 para sim: ");	scanf("%d", &salvar_clu);
	while(salvar_clu != 0 && salvar_clu != 1){
		printf("\nEntrada invalida!\nGostaria de salvar os resultados agrupados por cluster? Digite 0 para nao, 1 para sim: ");	scanf("%d", &salvar_clu);
	}

	printf("Numero de elementos: %d\n\n", num_el);


	el = (Elemento *) malloc(num_el * sizeof(Elemento));
	
	clus.qt_clusters = 0;
	clus.total_elementos = 0;
	clus.c = (Cluster *) malloc(num_el * sizeof(Cluster));
	
	num_coord = le_arquivo(nome_arquivo, el);
	if(num_coord == -1){
		printf("Erro ao abrir o arquivo!\n");
		return 0;
	}

	ret = cria_matriz(matriz, el);
	matriz = ret.matriz;

	merge(ret.menor, matriz, el, &clus);

	if(num_el > 20){
		passo = (int) ceil(num_el / 20);
	}else{
		passo = 1;
	}

	while(1){
		ret = pega_menor(matriz);

		if(ret.menor[0] < 0){
			sprintf(semresult, "Nao ha resultados para k >= %d.\n", nmaxclu);
			break;
		}

		if(exibir % passo == 0){
			printf("Rodando.\n");
		}

		merge(ret.menor, matriz, el, &clus);

		if(clus.qt_clusters > nmaxclu){
			nmaxclu = clus.qt_clusters;
		}

		if(clus.total_elementos == num_el){
			if(clus.qt_clusters <= kmax && clus.qt_clusters >= kmin){
				tam_nome = strlen(nome_arquivo);
				strncpy(nomeaux, nome_arquivo, tam_nome-4);
				fflush(stdout);
				sprintf(nome_salvar, "Resultados_%s_%d.clu", nomeaux, clus.qt_clusters);
				salvar(el, nome_salvar);

				sprintf(nome_salvar, "Clusters_%d_%s.txt", clus.qt_clusters, nomeaux);
				if(salvar_clu){
					salvar_clusters(clus, nome_salvar);
				}

				if(primeiro == 1 && clus.qt_clusters < kmax){
					sprintf(semresult, "Nao ha resultados para k >= %d.\n", clus.qt_clusters);
				}

				if(clus.qt_clusters == kmin){
					break;
				}
			}

			primeiro = 0;
		}
		
		if(exibir % passo == 0){
			porcentagem += exibir / passo * 0.5;
			printf("Rodando... %d%\n", porcentagem);
		}

		exibir++;
	}
	
	printf("100%\n%s\n", semresult);

	end = clock();
	time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

	if(time_spent > 60){
		time_spent = time_spent/60;
		printf("A resposta foi salva nos arquivos!\nTempo de execucao: %lf minutos\n\n", time_spent);
	}else{
		printf("A resposta foi salva nos arquivos!\nTempo de execucao: %lf segundos\n\n", time_spent);
	}

/*	printf("Gostaria de exibir os resultados agrupados por cluster? Digite 0 para nao, 1 para sim: ");	scanf("%d", &exibir);
	while(exibir != 0 && exibir != 1){
		printf("\nEntrada invalida!\nGostaria de exibir os resultados agrupados por cluster? Digite 0 para nao, 1 para sim: ");	scanf("%d", &exibir);
	}

	if(exibir){
		exibe_clusters(clus);
	}*/

	free(el);
	for(i=0;i<clus.qt_clusters;i++){
		free(clus.c[i].elem);
	}
	free(clus.c);
	for(i = 0; i < num_el; i++){
		free(matriz[i]);
	}
	free(matriz);
}

int le_arquivo(char *arquivo, Elemento *el){
	int i, num_coord=0, j;
	char linha[TAM_MAX_LINHA], *token, conteudo[25];
	FILE *f;
	
	f = fopen(arquivo, "r");
	if(f == NULL){
		printf("Erro ao abrir o arquivo!\n\n");
		return -1;
	}
	fgets(linha, TAM_MAX_LINHA, f); //pula primeira linha de nomes
	
	//conta o numero de coordenadas/atributos que os elementos possuem
	fgets(linha, TAM_MAX_LINHA, f);
	for(i=0;linha[i] != '\n';i++){
		if(linha[i] == '\t'){
			num_coord++;
		}
	}
   	fseek(f, 0, SEEK_SET);

	fgets(linha, TAM_MAX_LINHA, f); //pula primeira linha de nomes
	//copia os elementos dos arquivos pra el
	for(i=0;i<num_el;i++){
		memset(linha, '\0', TAM_MAX_LINHA);

		fgets(linha, TAM_MAX_LINHA, f);
		
		token = strtok(linha, "\t");
		strcpy(el[i].nome, token);

		el[i].coord = (float *) malloc(num_coord * sizeof(float));
		for(j=0;j<num_coord;j++){
			token = strtok(NULL, "\t");
			strcpy(conteudo, token);
			el[i].coord[j] = (float) strtod(conteudo, NULL);
		}
		
		el[i].cluster = -1;
	}
	fclose(f);

	return num_coord;
}

int conta_linhas(char *arquivo){
	int linhas = 0;
	char ch;
	FILE *fp;
	
	fp = fopen(arquivo, "r");
	if(fp == NULL){
		return -1;
	}
	
	while(!feof(fp)){
		ch = fgetc(fp);
		if(ch == '\n'){
			linhas++;
		}
	}
	
	fclose(fp);
	return linhas - 1;		//-1 pra nao contar a primeira dos nomes
}

Retorno cria_matriz(float **matriz, Elemento *el){
	int i, j;
	float menor;
	Retorno ret;
	
	matriz = (float**) malloc(num_el * sizeof(float*));
	for(i = 0; i < num_el; i++){
		matriz[i] = (float *) malloc(num_el * sizeof(float));
	}
	
	menor = calc_dist(el[1], el[0]);
	ret.menor[0] = 1;
	ret.menor[1] = 0;
	
	for(i=0;i<num_el;i++){
		for(j=0;j<num_el;j++){
			if(j > i){
				matriz[i][j] = -1;
			}
			else if(i == j){
				matriz[i][j] = 0;
			}
			else{			
				matriz[i][j] = calc_dist(el[i], el[j]);
				if(matriz[i][j] < menor){
					ret.menor[0] = i;
					ret.menor[1] = j;
					
					menor = matriz[i][j];
				}
			}
		}
	}

	ret.matriz = matriz;

	return ret;
}

float calc_dist(Elemento el1, Elemento el2){
	int i;
	float soma = 0;

	for(i=0;i<num_coord;i++){
		soma = soma + pow((el1.coord[i] - el2.coord[i]), 2);
	}
	
	return sqrt(soma);
}

void merge(int *menor, float **matriz, Elemento *el, Clusters *clus){
	int i, j, k, l, qt, teste, id, idaux;
	Elemento **aux;
	
	i = menor[0];
	j = menor[1];
	qt = clus->qt_clusters;
	
	if(el[i].cluster == -1 && el[j].cluster == -1){
		//SIGNIFICA QUE OS DOIS ELEMENTOS A SEREM JUNTADOS NÃO PERTECEM A UM CLUSTER
		clus->c[qt].id = qt;
		clus->c[qt].qt_elementos = 2;
		clus->c[qt].elem = (Elemento **) malloc(2 * sizeof(Elemento*));
		clus->c[qt].elem[0] = &el[i];
		clus->c[qt].elem[1] = &el[j];
		
		el[i].cluster = qt;
		el[j].cluster = qt;
		
		
		clus->qt_clusters++;
		recalcula_dist(matriz, el, qt, clus);

		clus->total_elementos = clus->total_elementos + 2;
	}
	else if(el[i].cluster != -1 && el[j].cluster == -1){
		//SIGNIFICA QUE O ELEMENTO i PERTENCE A UM CLUSTER, MAS j NÃO
		id = el[i].cluster;
		el[j].cluster = id;
		
		aux = (Elemento **) malloc((clus->c[id].qt_elementos + 1) * sizeof(Elemento*));
		for(k=0;k<clus->c[id].qt_elementos;k++){
			aux[k] = clus->c[id].elem[k];
		}
		
		free(clus->c[id].elem);
		clus->c[id].elem = aux;
		
		
		clus->c[id].elem[clus->c[id].qt_elementos] = &el[j];
		clus->c[id].qt_elementos++;
		
		clus->total_elementos++;
		recalcula_dist(matriz, el, id, clus);		
	}
	else if(el[i].cluster == -1 && el[j].cluster != -1){
		//SIGNIFICA QUE O ELEMENTO j PERTENCE A UM CLUSTER, MAS i NÃO
		id = el[j].cluster;
		el[i].cluster = id;
		
		aux = (Elemento **) malloc((clus->c[id].qt_elementos + 1) * sizeof(Elemento*));
		for(k=0;k<clus->c[id].qt_elementos;k++){
			aux[k] = clus->c[id].elem[k];
		}
		
		free(clus->c[id].elem);
		clus->c[id].elem = aux;
		
		clus->c[id].elem[clus->c[id].qt_elementos] = &el[i];
		clus->c[id].qt_elementos++;
		
		clus->total_elementos++;
		recalcula_dist(matriz, el, id, clus);
	}
	else{
		//OS DOIS PERTENCEM A UM CLUSTER, FUSÃO DE CLUSTERS!
		//caso o cluster de i tenha id menor: (necessario sempre manter no menor para ficar consistente)
		if(el[i].cluster < el[j].cluster){
			id = el[i].cluster;
			idaux = el[j].cluster;
		}
		//caso o cluster de j tenha id menor
		else if(el[j].cluster < el[i].cluster){
			id = el[j].cluster;
			idaux = el[i].cluster;
		}
		
		//copiando elementos do cluster de j para o de i:
		aux = (Elemento **) malloc((clus->c[id].qt_elementos + clus->c[idaux].qt_elementos) * sizeof(Elemento*));
		for(k=0;k<clus->c[id].qt_elementos;k++){
			aux[k] = clus->c[id].elem[k];
		}
			
		free(clus->c[id].elem);
		clus->c[id].elem = aux;
			
		for(k=0;k<clus->c[idaux].qt_elementos;k++){
			clus->c[id].elem[clus->c[id].qt_elementos] = clus->c[idaux].elem[k];
			clus->c[id].elem[clus->c[id].qt_elementos]->cluster = id;
				
			clus->c[id].qt_elementos++;
		}
			
		//apagando o cluster de j, dando shift e renomeando todos os clusters depois dele:
		for(k=idaux;k<clus->qt_clusters-1;k++){
			clus->c[k] = clus->c[k+1];
			for(l=0;l<clus->c[k].qt_elementos;l++){
				clus->c[k].elem[l]->cluster--;
			}
			clus->c[k].id--;
		}
		clus->c[k].elem = NULL;
		clus->qt_clusters--;
			
		recalcula_dist(matriz, el, id, clus);
	}
}

void recalcula_dist(float **matriz, Elemento *el, int id, Clusters *clus){
	int i, j, k, l, teste, icluster, jcluster;
	float soma;
	Distancias *dists;
	
	dists = (Distancias *) malloc(clus->qt_clusters * sizeof(Distancias));
	for(i=0;i<clus->qt_clusters;i++){
		dists[i].id1 = i;
		dists[i].distancia = (float*) malloc(clus->qt_clusters * sizeof(float));
		for(j=0;j<clus->qt_clusters;j++){
			dists[i].distancia[j] = -1;
		}
		dists[i].distancia[i] = 0;
	}
	
	for(i=0;i<num_el;i++){
		for(j=0;j<num_el;j++){
			//só faz as contas para as iteracoes que nos interessam (sem repeticao, sem contar dist 0)
			if(j < i){
				if(matriz[i][j] == -2){
					soma = 0;
				}
				//para os casos de um pertencer a um cluster e outro não:
				//se o que não pertence a um cluster é j:
				else if(el[i].cluster == id && el[j].cluster == -1){
					soma = 0;
					for(k=0;k<clus->c[id].qt_elementos;k++){
						soma = soma + calc_dist(*(clus->c[id].elem[k]), el[j]);
					}
					soma = soma / clus->c[id].qt_elementos;
						
					matriz[i][j] = soma;
				}
				//se o que não pertence a um cluster é i:
				else if(el[i].cluster == -1 && el[j].cluster == id){
					soma = 0;
					for(k=0;k<clus->c[id].qt_elementos;k++){
						soma = soma + calc_dist(*(clus->c[id].elem[k]), el[i]);
					}
					soma = soma / clus->c[id].qt_elementos;
						
					matriz[i][j] = soma;
				}
				//se os dois pertencem ao mesmo cluster:
				else if(el[i].cluster == id && el[j].cluster == id){
					matriz[i][j] = -2;		//faz elementos do mesmo cluster ficarem com dist -2
				}
				//para os casos de os dois pertencerem a um cluster (nao o mesmo, checado com -2 de dist):
				//para os casos em que o cluster que demos merge ser o i:
				else if(el[i].cluster == id && el[j].cluster != -1){
					//checa se a distancia entre os dois já foi calculada (se sim, está guardada na struct dists); 
					//Se não:
					if(dists[el[i].cluster].distancia[el[j].cluster] == -1){
						//calcula a distancia de cada elemento pra cada elemento, armazena em soma, divide tudo no final por |clusterA|*|clusterB|
						soma = 0;
						for(k=0;k<clus->c[id].qt_elementos;k++){
							for(l = 0;l<clus->c[el[j].cluster].qt_elementos;l++){
								soma = soma + calc_dist(*(clus->c[id].elem[k]), *(clus->c[el[j].cluster].elem[l]));
							}
						}
						
					
						soma = soma / (clus->c[id].qt_elementos * clus->c[el[j].cluster].qt_elementos);
					
						matriz[i][j] = soma;
						
						//adicionando essa distância (reflexiva) na struct de distâncias:
						dists[el[i].cluster].distancia[el[j].cluster] = soma;
						dists[el[j].cluster].distancia[el[i].cluster] = soma;
					}
					//Se sim:
					else{
						matriz[i][j] = dists[el[i].cluster].distancia[el[j].cluster];
					}
				}
				//para os casos em que o cluster que demos merge ser o j: (consultar o elseif acima para detalhes)
				else if(el[i].cluster != -1 && el[j].cluster == id){				
					if(dists[el[j].cluster].distancia[el[i].cluster] == -1){
						soma = 0;
						for(k=0;k<clus->c[id].qt_elementos;k++){
							for(l = 0;l<clus->c[el[i].cluster].qt_elementos;l++){
								soma = soma + calc_dist(*(clus->c[id].elem[k]), *(clus->c[el[i].cluster].elem[l]));
							}
						}
					
						soma = soma / (clus->c[id].qt_elementos * clus->c[el[i].cluster].qt_elementos);
					
						matriz[i][j] = soma;
						
						dists[el[i].cluster].distancia[el[j].cluster] = soma;
						dists[el[j].cluster].distancia[el[i].cluster] = soma;
					}
					else{

						matriz[i][j] = dists[el[i].cluster].distancia[el[j].cluster];			//nao me preocupo com o que é i e o que é j porque a dist é reflexiva
					}
				}
			}
		}
	}
	
	for(i=0;i<clus->qt_clusters;i++){
		free(dists[i].distancia);
	}
	free(dists);
}

Retorno pega_menor(float **matriz){
	int i, j, teste, flag = 0;
	float menor;
	Retorno ret;
	

	for(i=0;i<num_el;i++){
		for(j=0;j<num_el;j++){
			if(matriz[i][j] > 0){
				if(flag == 0){
					menor = matriz[i][j];
					ret.menor[0] = i;
					ret.menor[1] = j;
					
					flag = 1;
				}				
				else if(matriz[i][j] < menor){
					ret.menor[0] = i;
					ret.menor[1] = j;
				
					menor = matriz[i][j];
				}
			}
		}
	}
	
	ret.matriz = matriz;

	if(flag == 0){
		ret.menor[0] = -1;
	}
	return ret;
}

void salvar(Elemento *el, char *nome_arquivo) {
	int i;
	FILE *arquivo;

	arquivo = fopen(nome_arquivo, "w"); //cria novo arquivo

	for(i=0;i<num_el;i++){       //para cada cluster
		fprintf(arquivo,"%s\t%d\n", el[i].nome, el[i].cluster); //escreve registros
	}
    
	fclose(arquivo);
}

void salvar_clusters(Clusters clus, char *nome_arquivo){
	int i, j, k;
	FILE *arquivo;

	arquivo = fopen(nome_arquivo, "w");
	
	fprintf(arquivo, "Quantidade de clusters: %d", clus.qt_clusters);
	
	for(i=0;i<clus.qt_clusters;i++){
		fprintf(arquivo, "\n\n**************************************\nCluster id: %d\nQuantidade de elementos: %d", clus.c[i].id, clus.c[i].qt_elementos);
		for(j=0;j<clus.c[i].qt_elementos;j++){
			fprintf(arquivo, "\n\nnome: %s\n", clus.c[i].elem[j]->nome);
			fprintf(arquivo, "cluster: %d\n", clus.c[i].elem[j]->cluster);

			fprintf(arquivo, "coordenada %d: %f", 0+1, clus.c[i].elem[j]->coord[0]);
			for(k=1;k<num_coord;k++){
				fprintf(arquivo, "          coordenada %d: %f", k+1, clus.c[i].elem[j]->coord[k]);
			}
		}
	}
}


//Funcoes utilizadas para debug
void exibe_clusters(Clusters clus){
	int i, j, k;
	
	printf("\n\n\n********************************\nquantidade de clusters: %d\n\n", clus.qt_clusters);
	
	for(i=0;i<clus.qt_clusters;i++){
		printf("*******************\ncluster id: %d\nqt_elementos: %d\n", clus.c[i].id, clus.c[i].qt_elementos);
		printf("\n");
		for(j=0;j<clus.c[i].qt_elementos;j++){
			printf("nome: %s\n", clus.c[i].elem[j]->nome);
			for(k=0;k<num_coord;k++){
				printf("coordenada %d: %f          ", k+1, clus.c[i].elem[j]->coord[k]);
			}
			printf("\ncluster: %d\n\n", clus.c[i].elem[j]->cluster);
		}
	}
}

void exibe_matriz(float **matriz){
	int i, j;
	for(i=0;i<num_el;i++){
		for(j=0;j<num_el;j++){
			printf("%f\t", matriz[i][j]);
		}
		printf("\n");
	}
}

void exibe_elementos(Elemento *el){
	int i, j;
	for(i=0;i<num_el;i++){
		printf("nome: %s\n", el[i].nome);
		for(j=0;j<num_coord;j++){
			printf("coordenada %d: %f\t", j+1, el[i].coord[j]);
		}
		printf("\ncluster: %d\n\n", el[i].cluster);
	}
}