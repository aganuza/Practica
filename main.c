#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <stdbool.h>
#include "estructuras.h"
#include "funciones.h"



int main(int argc, char *argv[]){

	if (argc != 2) {
		printf ("\n Formato incorrecto: ./main fichero_parametros.txt \n");
		exit (-1);
	}

	//Inicializar las variables de los parametros
	struct poblacion pobl;
	struct persona *personas;
	int i, lag, ale, vacunados, vivos;

	//Leer los parametros del fichero

	FILE *f1;
    f1=fopen("data.txt","r");
    int a;
    
    if(f1==NULL){
        printf("ERROR! No se pudo abrir el documento");
        exit(-1);
    }

    //Poblacionen datuak jaso
    fscanf(f1, "%d", &a);
    pobl.per_max=a;
    fscanf(f1, "%d", &a);
    pobl.radio_con=a;
    fscanf(f1, "%d", &a);
    pobl.periodo_inc=a;
    fscanf(f1, "%d", &a);
    pobl.periodo_rec=a;
    fscanf(f1, "%d", &a);
    pobl.vel_max=a;
    fscanf(f1, "%d", &a);
    pobl.vac_nec=a;
    fscanf(f1, "%d", &a);
    pobl.probabilidad_cont=a;
    fscanf(f1, "%d", &a);
    pobl.cambiovel=a;
    fscanf(f1, "%d", &a);
    pobl.limite=a;

    vacunados = 0;
    vivos = pobl.per_max;

	//Edades aleatorias usando libreria gsl
	int seed;
	float mu, beta, alfa;
	gsl_rng *r;
	seed = 1;
    mu = 100;
    alfa = 3;
    beta = 3;
	gsl_rng_env_setup();
    r = gsl_rng_alloc (gsl_rng_default);
    gsl_rng_set(r, seed);


	//Inicializar los parametros
	personas = malloc(pobl.per_max*sizeof(struct persona));

	for (lag=0; lag<pobl.per_max; lag++){
		personas[lag].estado = 0;
		personas[lag].edad = roundf(mu * gsl_ran_beta(r, alfa, beta));
		personas[lag].tiempo = 0;
		personas[lag].riesgo = rand()%101;
		personas[lag].posicion[0] = rand()%pobl.limite+1;
		personas[lag].posicion[1] = rand()%pobl.limite+1;
		personas[lag].velocidad[0] = (rand()%pobl.vel_max*2+1)-pobl.vel_max;
		personas[lag].velocidad[1] = (rand()%pobl.vel_max*2+1)-pobl.vel_max;
	}
	gsl_rng_free(r);

	//Elegir el paciente 0
	ale = rand()%pobl.per_max+1;
	personas[ale].estado = 2;

	//Iniciar el tiempo de ejecucion
	double t;
    struct timeval stop, start;
	gettimeofday(&start, NULL);

	//Iniciar simulación
	int k,j,p;
	int cont;
	bool vac=true;
	int porcent;
	bool sumistrada = false;

	while(vac && vivos != 0){
		mover(personas, &pobl);
		for(i=0; i<pobl.per_max; i++){
			for (j=i; j<pobl.per_max-1; j++){
				if(fabs(personas[i].posicion[0]-personas[j].posicion[0])<=pobl.radio_con && fabs(personas[i].posicion[1]-personas[j].posicion[1])<=pobl.radio_con){
					if((personas[i].estado==1 || personas[i].estado==2) && personas[j].estado==0){
						if(pobl.probabilidad_cont >= (rand()%101)) {
							personas[j].estado = 1;
						}
					}else if((personas[j].estado==1 || personas[j].estado==2) && personas[i].estado==0) {
						if(pobl.probabilidad_cont >= (rand()%101)) {
							personas[i].estado = 1;
						}
					}
				}
			}

		}
		//Aqui correremos toda la lista de la poblacion para tratarlon
		for(p=0; p<pobl.per_max; p++){
			//si una persona esta en estado 1 se empezara a sumar su tiempo
			if(personas[p].estado==1){
				personas[p].tiempo+=1;
				//cuando llegue al mismmo tiempo que el periodo de inc pasara a estado 2 y el tiempo de pondra a 0
				if(personas[p].tiempo >= pobl.periodo_inc){
					personas[p].estado=2;
					personas[p].tiempo=0;
				}
				//cuando una persona este en estado 2 se le ira sumando tiempo
			}else if(personas[p].estado==2){
				personas[p].tiempo+=1;
				//cuando la persona este aun tik de la recuperacion, se calculara con el riesgo si este morira, de ser asi, pasara a estado 5, si no a estado 3
				if(personas[p].tiempo == (pobl.periodo_rec-1)){
					if(personas[p].riesgo >= (rand()%101)){
						personas[p].estado=5;
						vivos--;
					}else{
						personas[p].tiempo+=1;
					}
				}else if(personas[p].tiempo >= pobl.periodo_rec){
					personas[p].estado=3;
					personas[p].tiempo=0;
					vacunados++;
				}		
			}	
		}
		printf("%d %d %d\n", vivos, vacunados,porcent);
		porcent = (100*vacunados/vivos);
		if(porcent<pobl.vac_nec){
			sumistrada = false;
			for(k = 0; k<=pobl.per_max && !sumistrada; k++){
				if(personas[k].estado == 0){
					personas[k].estado=4;
					vacunados++;
					sumistrada = true;
				}
			}
		}else{
			vac=false;
			/*se termina la simulacion*/
		}
	}

	//Tomar el tiempo de ejecución
	gettimeofday(&stop, NULL);
	t = (double)(stop.tv_usec - start.tv_usec) / 1000000 + (double)(stop.tv_sec - start.tv_sec);

	//Escribir resultados y posiciones en fichero
	simulacion(&pobl, personas, t);
	
	
	return(0);
}