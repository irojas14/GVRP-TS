#include <cmath>
#include <random>
#include <utility>
#include <algorithm>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <iterator>
#include <vector>
#include <cstring>
#include <math.h>
#include <time.h>
#include <cstdlib>

using namespace std;

struct nodo
{  
     string ID;  
     string type;  
     double longitude;  
     double latitude; 
};

struct parametros
{
	int Q;
	float r;
	int TL;
	int v;
	int m;
};


//Funcion que transforma grados en radianes
double toRadians(double degrees)
{
    return((degrees * M_PI)/180);
}

//Funcion Haversine que retorna la distancia (en millas) entre dos nodos a partir de  sus latitudes y longitudes
double haversine(double lat1, double lat2, double lon1, double lon2) 
{
    double radiusOfEarth = 4182.44949; // miles, 6371km;
    double dLat = toRadians(lat2 - lat1);
    double dLon = toRadians(lon2 - lon1);
    double a = sin(dLat / 2) * sin(dLat / 2) + cos(toRadians(lat1)) *
                                                         cos(toRadians(lat2)) * sin(dLon / 2) *
                                                         sin(dLon / 2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    double distance = radiusOfEarth * c;

    return distance;
}

//Función que toma el archivo de texto y a partir de sus datos, crea nodos y define parámetros.
void generarNodosYParametros(string instancia, vector<struct nodo> &nodos, struct parametros &my_parametros)
{
	int i;
	bool vacia = false; //Bandera que, al leer lineas, verifica cual está vacía (para separar nodos de parámetros del archivo).
	int contador = 0;
	int contador2= 0;
	int contlin= 0;
	string linea;
	fstream my_file;
	my_file.open(instancia, ios::in);
	if (my_file.fail())
		cout<<"Error abriendo archivo."<<endl;	
	getline(my_file, linea);
	while (getline(my_file, linea, '\n'))
	{

		if(!(linea.size()-1)) //Si la línea está vacía, la bandera toma el valor de true. Esto nos permitirá saber si dejamos de leer nodos para comenzar
							  //a leer parámetros.
		{
			vacia = true;
			contador = contador + 1;
		}

		else{
			vacia = false;
		}

		if(contador == 0) //Si el contador es 0, significa que no se han encontrado lineas vacías, por lo que aún estamos leyendo lineas correspondientes
						  // a nodos.
		{
			vector <string> tempstr;
	    	stringstream ss(linea);
	    	string temp;
	    	struct nodo my_nodo;
	    	while (getline(ss, temp, '\t'))    
	        	tempstr.push_back(temp);
	    	my_nodo.ID = tempstr[0]; //ID del nodo
			my_nodo.type = tempstr[1]; //Tipo de nodo
			my_nodo.longitude = stod(tempstr[2]); //Longitud del nodo
			my_nodo.latitude = stod(tempstr[3]); //Latitud del nodo
			if(contlin !=1) //Linea en la cual el depósito de vehículos se repite
			{
			nodos.push_back(my_nodo);
			}
		}

		if (vacia == false and contador != 0) //A partir de aquí, se comienzan a leer los parámetros del archivo.
		{
			vector <string> tempstr2; 
			stringstream ss2(linea);
			string temp2; 
			while (getline(ss2, temp2, '/'))
			{
				tempstr2.push_back(temp2);
			}
			if (contador2 == 0)
			{
				my_parametros.Q = stof(tempstr2[1]); //Capacidad del tanque de bencina
				contador2+=1;
			}
			else if (contador2 == 1)
			{
				my_parametros.r = stof(tempstr2[1]); //Tasa de consumo de combustible
				contador2+=1;
			}
			else if (contador2 == 2)
			{
				my_parametros.TL = stof(tempstr2[1]); //Tamaño del tour 
				contador2+=1;
			}
			else if (contador2 == 3)
			{
				my_parametros.v = stof(tempstr2[1]); //Velocidad media
				contador2+=1;
			}
			else
			{
				my_parametros.m = stof(tempstr2[1]); //Cantidad de vehículos
			}
		}
		
	contlin++;   
	}

	my_file.close();
}


void distancia(vector<struct nodo> n, vector<vector<double>> &d)
{

	double aux;
	int n_nodos = (int) n.size();
	for (int i = 0; i < n_nodos; ++i) {
        vector<double> row; //Empty row
        for (int j = 0; j < n_nodos; ++j) {
            row.push_back(0);
        }
        d.push_back(row);
    }

    for (int i = 0; i < n_nodos; ++i) {
        d[i][i] = 9999;
        for (int j = 0; j < i; ++j) {
            aux = haversine(n[i].latitude,n[j].latitude, n[i].longitude, n[j].longitude);
            if (aux == 0)
                aux = 9999; //En caso de estar en el mismo nodo, al aplicar greedy se evita visitarlo nuevamente,
            d[i][j] = aux;
            d[j][i] = aux;
        }
    }
	
}


//Evaluación en la función objetivo de la solución propuesta
double evaluarDistancia(vector<string> sol, vector<vector<double> > d)
{
    double dist = 0;
    int prev = 0;
    for(vector<string>::iterator it = sol.begin() + 1; it != sol.begin() + sol.size(); ++it) 
	{
        dist+= d[prev][stof(*it)];
        prev = stof(*it);
    }
    return(dist);
}


//función que verifica si un nodo ya existe en un vector, es decir, si el nodo ya fue visitado.
bool existeEnVector(vector<string> v, string busqueda) 
{
    return find(v.begin(), v.end(), busqueda) != v.end();
}


//Comprueba si la solución es factible, es decir, si se satisfacen las restricciones
//de combustible y tiempo
bool esFactible(vector<struct nodo> n, vector<string> sol, vector<vector<double> > d, struct parametros p){
    double Q = p.Q;
	double r = p.r;
	double TL = p.TL;
	double v = p.v; //millas por hora
    v = v/60; //milas por minuto
    int n_nodos = (int)sol.size();

    int prev = stoi(sol[0]);
    int time = 0;

    for (int i = 1; i < n_nodos; ++i) {
        if(d[stoi(sol[prev])][stoi(sol[i])] == 9999) //Se verifica si estoy en el mismo nodo
            continue;

        Q -= r * d[stoi(sol[prev])][stoi(sol[i])];
		//cout<<"Nos quedamos con "<<Q<<" de bencina al realizar el recorrido de "<<stoi(sol[prev])<<" hacia el nodo "<<stoi(sol[i])<<endl;
        time += d[stoi(sol[prev])][stoi(sol[i])]/v;  
		//cout<<"Se gastan "<<time<<" minutos al realizar el recorrido de "<<stoi(sol[prev])<<" hacia el nodo "<<stoi(sol[i])<<endl;

        if (Q < 0) 
		{
			//cout<<"Cagó, sin bencina desde el nodo "<<stoi(sol[prev])<<" hacia el nodo "<<stoi(sol[i])<<endl;
			//cout<<"Bencina: "<<Q<<endl;
            return false;
        }

        if (n[stoi(sol[i])].type == "f") {
            Q = p.Q;
            time += 15;
        } else if (n[stoi(sol[i])].type == "c"){
            time += 30;
        }else{
            Q = p.Q;
            time = 0;
        }

        if(time >= 60*TL) {
            //cout<<"Cagó, se quedó sin tiempo desde el nodo "<<stoi(sol[prev])<<" hacia el nodo "<<stoi(sol[i])<<endl;
			//cout<<"Tiempo: "<<time<< " el cual es mayor a "<<60*TL<<endl;
			return false;
        }

        prev = i;
    }
    return true;
} 


//Se intercambian los nodos A y B del recorrido de la solución
vector <string> dosOpt(vector<string>recorrido, int nodoA, int nodoB)
{
	vector <string> cambio;
	int n;
	int n_nodos = (int) recorrido.size();
	for(n=0; n<nodoA; n++)
	{
		cambio.push_back(recorrido[n]);
	}
	for(n=nodoB; n>=nodoA; n--)
	{
		cambio.push_back(recorrido[n]);
	}
	for(n=nodoB + 1; n < n_nodos; n++)
	{
		cambio.push_back(recorrido[n]);
	}
	recorrido = cambio;
	return recorrido;
}

//Función que retorna una función inicial. Se utilizan criterios tanto de combustible como de tiempo para escoger los nodos que se visitan.
vector<string>recorrido(vector<struct nodo> n, vector<vector<double>> d, struct parametros p, double Q, double r, double TL, double v)
{
    int n_nodos = (int) n.size(); //cantidad de nodos
    //int siguienteNodo = rand() % n_nodos; //siguiente nodo a visitar
	double minDistancia;
	vector<string> f; //Vector en el que se almacena el índice del nodo que sea estación de servicio.
    vector<string> nodosVisitados; //Vector en el que se almacena el recorrido.
	int indice;
	int contadorDyF = 0; //Contador para identificar la cantidad de estaciones de servicio existentes, junto al depósito.
	double minQ = 0;
	float tiempo = 0;
	bool nodoFueVisitado;
	vector<double> Fs; //Almacena la distancia de cada nodo con la estación de servicio más cercana.


	vector<string> nodos; //Variable auxiliar para determinar los nodos cercanos,

	for(int i=0; i<n_nodos; i++)
	{
		//cout<<n[i].ID<<endl;
		nodos.push_back(to_string(i));
		if(n[i].type == "d" || n[i].type=="f")
		{
			contadorDyF++;
			f.push_back(to_string(i));
		} 
	}

	
	double minF = 9999;
	for (int i = 0; i<nodos.size(); i++)
	{
		minF = 9999;
		for (int j = 1; j<f.size(); j++ ) 
		{
        	if (d[stof(nodos[i])][stof(nodos[j])] < minF) 
			{
            	minF = d[stof(nodos[i])][stof(nodos[j])];	
        	}
        }
		Fs.push_back(minF);
	}		


	nodosVisitados.push_back(to_string(0)); //siempre se parte del depósito



	for(int i=0; i<nodosVisitados.size(); i++) 
	{
		minDistancia = 9999;
		indice = 0;
		nodoFueVisitado = false;
		for(int j=1; j<nodos.size(); j++) 
		{
			//Se le agrega el tiempo solo para verificar que puede realizar dicha ruta, o en su defecto ir al deoósito o ir a la estación de servicio más cercana.
			//Si puede, al final se le borra.
			tiempo = tiempo + d[stof(nodosVisitados[i])][stof(nodos[j])]/v + min(d[0][stof(nodos[j])]/v, Fs[stof(nodos[j])]/v); 

			//Se verifica que el nodo anterior es distinto al que quiero visitar.
			if(nodosVisitados[nodosVisitados.size()-1] != nodos[j]) 
			{
				//Si no sobrepaso mi capacidad de bencina o si el nodo a visitar es distinto de cliente o si mi bencina actual me permite llegar a la estación
				//de servicio más cercana, entro al if.
				if((d[0][stof(nodos[j])]+minQ)*r <= Q || n[stof(nodos[j])].type != "c" || (Fs[stof(nodos[j])]+minQ)*r <= Q)
				{

					//Si estoy visitando a un nodo de tipo cliente, veo si al satisfacerlo no me paso del tiempo o, si visito una estación de servicio
					//verifico que al llenar el estanque no me paso del tiempo máximo.
					if((n[stof(nodos[j])].type == "c" && tiempo+30<=TL*60) || (n[stof(nodos[j])].type == "f" && tiempo+15<=TL*60)) 
					{

						//Verifico que estoy visitando al nodo que está más cerca del nodo anterior.
						if(d[stof(nodosVisitados[i])][stof(nodos[j])] < minDistancia)
						{

							//Evito pasar de una estación de servicio a otra. 
							if(n[stof(nodosVisitados[nodosVisitados.size()-1])].type != "c" && n[stof(nodos[j])].type !="c") 
							{	
								tiempo = tiempo - d[stof(nodosVisitados[i])][stof(nodos[j])]/v - min(d[0][stof(nodos[j])]/v, Fs[stof(nodos[j])]/v);
								continue;
							}
							minDistancia = d[stof(nodosVisitados[i])][stof(nodos[j])];
							nodoFueVisitado = true;
							indice = j;	 
						}
					}
				}
			}
			//Le devolvemos el tiempo que le añadimos como supuesto.
			tiempo = tiempo - d[stof(nodosVisitados[i])][stof(nodos[j])]/v - min(d[0][stof(nodos[j])]/v, Fs[stof(nodos[j])]/v);

		}					
		
		//Si la cantidad de bencina me alcanza para moverme al siguiente nodo y este nodo fue considerado para ser visitado, entonces
		//le agregamos a minQ la cantidad de bencina que vamos a emplear, le agregamos el tiempo que vamos a emplear para viajar al nodo
		//y el nodo se agrega como parte de la solución.
		if(minQ + d[stof(nodosVisitados[nodosVisitados.size()-1])][stof(nodos[indice])]*r < Q && nodoFueVisitado)
		{
				minQ = minQ + d[stof(nodosVisitados[nodosVisitados.size()-1])][stof(nodos[indice])]*r;
				tiempo += d[stof(nodosVisitados[nodosVisitados.size()-1])][stof(nodos[indice])]/v;
				nodosVisitados.push_back(nodos[indice]);
				
				if(n[stof(nodosVisitados[nodosVisitados.size()-1])].type == "c")
				{
					tiempo += 30;
					nodos.erase(nodos.begin() + indice);
					Fs.erase(Fs.begin() + indice); //elimino la distancia de ese nodo a una bencinera y/o depósito.
				}
				else if(n[stof(nodosVisitados[nodosVisitados.size()-1])].type == "f")
				{
					tiempo += 15;
					minQ = 0;

				}
				else
				{
					tiempo = 0;
					minQ = 0;
				}
		}

		else
		{
			if(nodosVisitados[nodosVisitados.size()-1] != to_string(0))
			{
				nodosVisitados.push_back(to_string(0));
			}
			minQ = 0;
			tiempo = 0;
		}

		if(nodos.size()==contadorDyF)
		{
			break;
		}
	}

	if(nodosVisitados[nodosVisitados.size()-1]!= to_string(0))
	{
		nodosVisitados.push_back(to_string(0)); //siempre termina en el depósito
	}


	//En caso de que no me recorra todos los nodos cliente por la restricción de TL u otro error extraño, los añadimos a la fuerza al final del recorrido.
	int i=0;
	while(nodos.size()!=contadorDyF)
	{
		if(n[stof(nodos[i])].type == "c")
		{
			nodosVisitados.push_back(nodos[i]);
			nodos.erase(nodos.begin() + i);
			i=0;
		}
		i++;
	}
		
	if(nodosVisitados[nodosVisitados.size()-1]!= to_string(0))
	{
		nodosVisitados.push_back(to_string(0)); //siempre termina en el depósito
	}
	
	
		
	cout<<"****Solución Inicial****"<<endl;
    for(int i = 0 ; i < (int) nodosVisitados.size() ; i++)
		cout<<n[stoi(nodosVisitados[i])].ID<<"-";
	cout<<endl;
    cout<<"************************"<<endl;

	
    cout << "Distancia de solución inicial: " << evaluarDistancia(nodosVisitados, d) << endl;
    return(nodosVisitados);
}

bool sonIgualesLosVectores( vector<string>V1, vector <string>V2)
{	

	if(V1.size() == 0 || V2.size()== 0)
	{
		return false;
	}

	for(int i=0; i < V1.size(); i++)
	{
		if( V1[i] == V2[i])
		{
			continue;
		}

		else
		{
			return false;
		}
	}
	return true;
}


vector <string> TabuSearch( vector <string> solucion, int numIteraciones, vector<vector<double>> d, vector<struct nodo> n, struct parametros p)
	{
		vector <string> solucionCandidata; 
		vector <string> mejorSolucionCandidata; //La mejor de todas las soluciones obtenidas
		vector <string> solAuxiliarCandidato; //Corresponde a la mejor candidata de las peores soluciones. Permite la exploración

		//Crearemos una lista tabú. Más adelante se aplicarán criterios para definir el largo máximo de la lista.
		vector<vector <string>> ListaTabu; 
		
		double distSolCandidata; //Distancia de la solución que iré iterando en cada momento.
		double mejorSolucion = evaluarDistancia(solucion, d); //Distancia de la mejor solución que he encontrado
		double solucionActual = evaluarDistancia(solucion, d); //Distancia actual de la solución.
		float aux = 9999; 
		int mejorIteracion;

		int n_nodos = (int) solucion.size()-1;

		//Aquí se verifica que el proceso se repita hasta que se cumpla la cantidad de iteraciones ingresadas
		for(int contador = 0; contador < numIteraciones; contador++)
		{
			int cambio = 0; //Cada vez que se agrega un vecino, cambio valdrá 1.
			int repetido;	//Para verificar si se repite o no la solución en la lista tabú.	

			for(int i=0; i<n_nodos; i++)
			{
				for (int j=0; j<n_nodos; j++)
				{

					vector<string> vecino; //La solución que se generará al aplicarle el movimiento 2opt.

					if(i!=j)
					{
						vecino = dosOpt(solucion, i, j);
					}

					else
					{
						continue;
					}
					

					distSolCandidata = evaluarDistancia(vecino, d); 
					repetido = 0;

					//Ahora vemos si el movimiento ya se encuentra en la lista tabú.
					for(int contador2 = 0; contador2<(int)ListaTabu.size(); contador2++)
					{

						if(sonIgualesLosVectores(ListaTabu[contador2], vecino))
						{
							repetido = 1;
							break;
						}

						else
						{
							continue;
						}
					}

					
					if(distSolCandidata < solucionActual && distSolCandidata < mejorSolucion && repetido == 0)
					{

						//En caso de que la lista tabú esté llena, se aplicará un FIFO que elimine los primeros elementos. 
						//Es decir, eliminará los vecinos más "antiguos". La lista tabú tendrá un máximo de 15 movimientos.
						if(ListaTabu.size()==15)
						{
							ListaTabu.erase(ListaTabu.begin());
							ListaTabu.push_back(vecino);
							if(esFactible(n, vecino, d, p) == false)
							{
								mejorSolucion = distSolCandidata + 200; //Si no es factible, se le aplica un castigo de 200 millas
							}
							else
							{
								mejorSolucion = distSolCandidata; //La mejor solución es la del vecino que acabo de almacenar.
							}
							mejorSolucionCandidata = vecino; //El vecino es el mejor candidato a solución óptimo local.
							cambio=1; //Significa que hubo cambios en la lista tabú.
							mejorIteracion = contador;
							
						}

						else
						//En caso de que la lista no se encuentre llena, simplemente se agrega el nuevo vecino.
						{
							ListaTabu.push_back(vecino);
							if(esFactible(n, vecino, d, p) == false)
							{
								mejorSolucion = distSolCandidata + 200; //Si no es factible, se le aplica un castigo de 200 millas
							}
							else
							{
								mejorSolucion = distSolCandidata; //La mejor solución es la del vecino que acabo de almacenar.
							}
							mejorSolucionCandidata = vecino;
							cambio=1;
							mejorIteracion = contador;
						}
						
					}
					//En caso de que el nuevo vecino no sea el mejor candidato pero si mejor que el valor anterior.
					else if(distSolCandidata < solucionActual && repetido ==0)
					{
						if(ListaTabu.size()==15)
						{
							ListaTabu.erase(ListaTabu.begin());
							ListaTabu.push_back(vecino);
							if(esFactible(n, vecino, d, p) == false)
							{
								solucionActual = distSolCandidata+200;
							}
							else
							{
								solucionActual = distSolCandidata;
							}
							solucionCandidata = vecino;
							cambio = 1;

						}

						else
						{
							ListaTabu.push_back(vecino);
							if(esFactible(n, vecino, d, p) == false)
							{
								solucionActual = distSolCandidata+200;
							}
							else
							{
								solucionActual = distSolCandidata;
							}
							solucionCandidata = vecino;
							cambio = 1;
						}
					}

					//Sino, se guarda la mejor de las peores soluciones. Eso es en caso de que no haya un vecino mejor que el candidato actual
					else
					{
						if(repetido == 0)
						{
							if(ListaTabu.size() == 15)
							{
								if(aux > evaluarDistancia(vecino, d))
								{
									if(esFactible(n, vecino, d, p) == false)
									{
										aux = evaluarDistancia(vecino, d) + 200;
									}
									else
									{
										aux = evaluarDistancia(vecino, d);
									}
									
									ListaTabu.erase(ListaTabu.begin());
									ListaTabu.push_back(vecino);
									solAuxiliarCandidato = vecino;
								}
							}

							else
							{
								if(aux > evaluarDistancia(vecino, d))
								{
									if(solAuxiliarCandidato.size()== 0 || vecino.size() == 0)
									{
										if(esFactible(n, vecino, d, p) == false)
										{
											aux = evaluarDistancia(vecino, d) + 200;
										}
										else
										{
											aux = evaluarDistancia(vecino, d);
										}
										solAuxiliarCandidato = vecino;
									}
								} 
									
								else
								{
									if(esFactible(n, vecino, d, p) == false)
									{
										aux = evaluarDistancia(vecino, d) + 200;
									}
									else
									{
										aux = evaluarDistancia(vecino, d);
									}
									ListaTabu.push_back(vecino);
									solAuxiliarCandidato = vecino;
								}
							}
							
						}
					}
				}
					
			}

			if( cambio == 1)
			{
				solucion = solucionCandidata;
			}

			else
			{
				solucionActual = aux;
				solucionCandidata = solAuxiliarCandidato;
			}

		}

		if(mejorSolucion <= solucionActual)
		{
			solucion = mejorSolucionCandidata;
			cout<<"**********Mejor solución encontrada**********"<<endl;
			for(int i=0; i<solucion.size(); i++)
			{
				cout<<n[stoi(solucion[i])].ID<<"-";
			}
			cout<<endl;
		
			cout << "Distancia de la mejor solución: " << evaluarDistancia(solucion, d) << endl;
			
			cout << "La mejor solución se encontró en la iteración: " << mejorIteracion << endl;
			
			return solucion;
		}

		else
		{
			cout<<"**********Mejor solución encontrada**********"<<endl;
			for(int i=0; i<solucion.size(); i++)
			{
				cout<<n[stoi(solucion[i])].ID<<"-";
			}
			cout<<endl;
		
			cout << "Distancia de la mejor solución: " << evaluarDistancia(solucion, d) << endl;

			cout << "La mejor solución se encontró en la iteración: " << mejorIteracion << endl;
			return solucion;
		}
			
	}

		

int main(int argc, char** argv){

	vector<struct nodo> n;
	struct parametros parametros;
	string temp(argv[1]);
	generarNodosYParametros(temp, n, parametros);
	vector<vector<double>> distances;
	distancia(n, distances);
	double Q = parametros.Q;
	double r = parametros.r;
	double TL = parametros.TL;
	double v = parametros.v; //Millas por hora
	v = v/60; //Millas por minuto	
	double m = parametros.m;
	int iteraciones(stoi(argv[2]));

    vector <string> solucionInicial = recorrido(n, distances, parametros, Q, r, TL, v);
	
	clock_t begin = clock();
	vector <string> TS = TabuSearch(solucionInicial, iteraciones, distances, n, parametros);
	clock_t end = clock();
	double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	cout << "Tiempo de ejecucion: " << time_spent << " segundos" << endl;
	return 0;
}

