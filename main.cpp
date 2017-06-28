#include <iostream>
#include <vector>
#include <list>
#include <fstream>
#include <sstream>

using namespace std;

const int cPenalties[10] = {20, 10, 10, 10, 10, 10, 50, 10, 10, 10};
const int UNUSED_CONSTRAINT         = -1;
const int ALLOCATION_CONSTRAINT     =  0;
const int NONALLOCATION_CONSTRAINT  =  1;
const int ONEOF_CONSTRAINT          =  2;
const int CAPACITY_CONSTRAINT       =  3;
const int SAMEROOM_CONSTRAINT       =  4;
const int NOTSAMEROOM_CONSTRAINT    =  5;
const int NOTSHARING_CONSTRAINT     =  6;
const int ADJACENCY_CONSTRAINT      =  7;
const int NEARBY_CONSTRAINT         =  8;
const int AWAYFROM_CONSTRAINT       =  9;

void show_usage(string name);
int selectVariable(list<int> entities, vector< vector< vector<int> > > eConstraintsMatrix, int iOrder);
void restore(int value, vector< vector< vector<int> > > eConstraintsMatrix, int **domain);
bool check_forward(int value, vector< vector< vector<int> > > eConstraintsMatrix, int **domain);
void forward_checking(int entity, list<int> entities, float *eCapacities, float *rCapacities, vector< vector< vector<int> > > eConstraintsMatrix, vector< vector< vector<int> > > rConstraintsMatrix, int **domain);



int main(int argc, char* argv[])
{
    string dataset, instance;
    ifstream file;
    int eTotal = 0;
    int rTotal = 0;
    int cTotal = 0;
    int iOrder;
    float *eCapacities;
    float *rCapacities;
    int *solution;
    int *optimalSolution;
    int **domain;
    list<int> entities;
    vector < vector<int> > brokenSCList; // Save cID and cType
    vector< vector<int> > rAdjacency;
    vector< vector<int> > rProximity;
    vector< vector< vector<int> > > eConstraints;
    vector< vector< vector<int> > > rConstraints;


    // Read program arguments

    if (argc < 5)
    {
        show_usage(argv[0]);
        return 1;
    }
    else {
        for (int i=1; i < argc; i++)
        {
            string arg = (string) argv[i];

            if(arg == "-d" ||  arg == "--dataset"){
                if (i + 1 < argc) dataset = (string) argv[++i];
                else{
                    show_usage(argv[0]);
                    return 1;
                }
            }else if (arg == "-i" || arg == "--instance"){
                if (i + 1 < argc) instance = (string) argv[++i];
                else{
                    show_usage(argv[0]);
                    return 1;
                }
            }else if (arg == "-io" || arg == "--instantiation-order"){
                if (i + 1 < argc) {
                    stringstream ss;
                    ss << argv[++i];
                    ss >> iOrder;
                }else {
                    show_usage(argv[0]);
                    return 1;
                }
            }
        }

        if (dataset.size() == 0 || instance.size() == 0 )
        {
            show_usage(argv[0]);
            return 1;
        }
    }


    // Open file

    string path = "./datasets/" + dataset + "/" + instance + ".txt";
    vector<char> constPath(path.begin(), path.end());
    constPath.push_back('\0');
    file.open(&constPath[0]);

    if (!file.is_open())
    {
        cerr << "Error al abrir el archivo :" << path << endl;
        return 1;
    }
    else
    {
        // Reading and representation process

        int step = 0;
        int lineCount = 0;
        vector<int> proximity;
        int startRoom = -1;
        int endRoom = -1;
        int previousFloor = -1;
        string line;

        while(getline(file, line))
        {
            istringstream ss(line);
            lineCount++;
            if (line.empty() || (line.size() <= 2 && line[0] == '\r'))
            {
                step = step + 1 <= 4 ? step + 1 : 4;
                if (step == 1)
                {
                    // Assign memory

                    eCapacities = new float[eTotal];
                    rCapacities = new float[rTotal];
                    solution = new int[eTotal];
                    optimalSolution = new int[eTotal];

                    // Assign domain memory and Innitialize variables

                    domain = new int*[eTotal];
                    for(int i = 0; i < eTotal; i++) {
                        domain[i] = new int[rTotal];
                        for(int j = 0; j < rTotal; j++) {
                            domain[i][j] = j;
                        }

                        solution[i] = -1;
                        optimalSolution[i] = -1;
                        entities.push_back(i);
                    }

                    // Innitialize vectors to fit index with ID
                    eConstraints.resize(eTotal, vector< vector<int> >(1, vector<int>()));
                    rConstraints.resize(rTotal, vector< vector<int> >(1, vector<int>()));
                }
                else if (step == 3) {
                    // Add last proximity vectors

                    for (int i = startRoom; i <= endRoom; i++) {
                        rProximity.push_back(proximity);
                    }

                }
                getline(file, line); // Skip block name: ENTITIES, ROOMS, CONSTRAINTS
                continue;
            }

            switch(step)
            {
                case 1:
                    {
                        // Read entities

                        int eID;
                        int eGroupID;
                        float eCapacity;
                        ss >> eID >> eGroupID >> eCapacity;
                        eCapacities[eID] = eCapacity;
                    }
                    break;
                case 2:
                    {
                        // Read rooms

                        int rID;
                        int rFloor;
                        int rAdjacencySize;
                        float rCapacity;
                        int value;
                        vector<int> adjacencyVector;

                        ss >> rID >> rFloor >> rCapacity >> rAdjacencySize;

                        rCapacities[rID] = rCapacity;

                        // Build Adjacency vector

                        for(int i = 0; i < rAdjacencySize; i++ ){
                            ss >> value;
                            adjacencyVector.push_back(value);
                        }

                        adjacencyVector.push_back(rID);
                        rAdjacency.push_back(adjacencyVector);

                        if (rFloor != previousFloor && previousFloor != -1) {
                            for (int i = startRoom; i <= endRoom; i++) {
                                rProximity.push_back(proximity);
                            }

                            previousFloor = rFloor;
                            startRoom = rID;
                            endRoom = rID;
                            proximity.clear();
                            proximity.push_back(rID);
                        }else {
                            previousFloor = rFloor;
                            startRoom = startRoom == -1 ? rID : startRoom;
                            endRoom = rID;
                            proximity.push_back(rID);
                        }
                    }
                    break;
                case 3:
                    // Read constraints
                    int cID;
                    int cType;
                    int cHardness;
                    int param1;
                    int param2;

                    ss >> cID >> cType >> cHardness >> param1 >> param2;

                    if (cType != 3) {
                        // Entity constraints

                        if (eConstraints[param1][0].size() == 0) {
                            eConstraints[param1][0].push_back(cID);
                            eConstraints[param1][0].push_back(cType);
                            eConstraints[param1][0].push_back(cHardness);
                            eConstraints[param1][0].push_back(param2);
                        }else {
                            vector<int> constraint;

                            constraint.push_back(cID);
                            constraint.push_back(cType);
                            constraint.push_back(cHardness);
                            constraint.push_back(param2);

                            eConstraints[param1].push_back(constraint);
                        }

                        if (cType >=4 && cType != 6) {
                            // Save restriction to both entities

                            if (eConstraints[param2][0].size() == 0) {
                                eConstraints[param2][0].push_back(cID);
                                eConstraints[param2][0].push_back(cType);
                                eConstraints[param2][0].push_back(cHardness);
                                eConstraints[param2][0].push_back(param1);
                            }else {
                                vector<int> mirrorConstraint;

                                mirrorConstraint.push_back(cID);
                                mirrorConstraint.push_back(cType);
                                mirrorConstraint.push_back(cHardness);
                                mirrorConstraint.push_back(param1);

                                eConstraints[param2].push_back(mirrorConstraint);
                            }
                        }
                    }else {
                        // Room constraints
                        rConstraints[param1][0].push_back(cID);
                        rConstraints[param1][0].push_back(3);
                        rConstraints[param1][0].push_back(cHardness);
                        rConstraints[param1][0].push_back(-1);
                    }

                    break;
                default:
                    // Read headers
                    int value = 0;
                    string trash;

                    ss >> trash >> value;
                    eTotal = lineCount == 1 ? value : eTotal;
                    rTotal = lineCount == 2 ? value : rTotal;
                    cTotal = dataset == "nott_data" && lineCount == 3? value : cTotal;
                    cTotal = dataset != "nott_data" && lineCount == 4? value : cTotal;
            };
        };

        file.close();
    }


    // Solution search process
    for (int i = 0; i < eTotal; i++)
    {
        cout << "eCapacities[" << i << "]: " << eCapacities[i] << endl;
    }
    cout << "ROOMS" << endl << endl;
    for(int i = 0; i < rTotal; i++)
    {
        cout << "rRooms[" << i << "]: " << rCapacities[i] << endl;
        // show adjacency vector
        cout << "Lista de adjancia lista de: " << i << endl;
        for(unsigned int j = 0; j < rAdjacency[i].size(); j++) {
            cout << rAdjacency[i][j] << ", " ;
        }
        cout << endl;
        cout << "Lista de proximidad lista de: " << i << endl;
        for(unsigned int j = 0; j < rProximity[i].size(); j++) {
            cout << rProximity[i][j] << ", " ;
        }
        cout << endl;
    }
    cout << "Entities: " << eTotal << "\tRooms: " << rTotal << "\tConstraints: "<< cTotal <<endl;

    cout << "Restricciones de entidades" <<endl;
    cout << "-----------------------------------" << endl;
    for(int i = 0; i < eTotal; i++) {
        cout << "Restricciones de :" << i <<endl;
        cout << "===========================================" << endl;
        cout << "cID\tcType\tcHardness\tParam" << endl;
        for (unsigned int j = 0; j < eConstraints[i].size(); j++) {
            if (eConstraints[i][j].size()) {
                cout << eConstraints[i][j][0] << "\t"
                     << eConstraints[i][j][1] << "\t"
                     << eConstraints[i][j][2] << "\t\t"
                     << eConstraints[i][j][3] << "\n";
            }
        }
        cout << "===========================================" << endl;
    }

    cout << "Restricciones de cuartos" <<endl;
    cout << "-----------------------------------" << endl;
    for(int i = 0; i < rTotal; i++) {
        cout << "Restricciones de :" << i <<endl;
        cout << "===========================================" << endl;
        if (rConstraints[i][0].size()) {
            cout << "cID\tcType\tcHardness\tParam" << endl;
            cout << rConstraints[i][0][0] << "\t"
                 << rConstraints[i][0][1] << "\t"
                 << rConstraints[i][0][2] << "\t\t"
                 << rConstraints[i][0][3] << "\n";
        }
    }


    int startEntity = selectVariable(entities, eConstraints, iOrder);
    forward_checking(startEntity, entities, eCapacities, rCapacities, eConstraints, rConstraints, domain);

    // FORWARD CHECKING

    /*int v = NULL;
    int previousV = NULL;
    int value;
    bool assign = false;
    vector<int> variables;
    vector<int> selectedVariables;

    // Guardar lista de variables a instanciar
    for(int i = 0; i < eTotal; i++) {
        variables.push_back(i);
    }

    vector<int> variableList(variables.begin(), veriables.end());

    // Mientras existan variables con valores sin revisar

    while(!fc::isSearchFinish(domain)) {
        // Seleccionar variable según criterio (ej: secuencial, + conectada)
        v = fc::selectVariable(v, variableList, selectedVariables, criterion);

        // Seleccionar valor del dominio
        // Si es un backtracking continuar con el siguiente valor, si no tomar desde el primero.
        for(int i = solution[v] ? solution[v] + 1 : 0; i < domain[v].size(); i++) {
            value = domain[v][i];

            // Verificar que el valor sea consistente
            if (fc::constraintCheck(v, value, eConstraints[v], rConstraints[v])) {

                // Chequear valores inconsistentes con la variable instanciada y
                // filtrarlos de los dominios de variables vecinas
                fc::checkAndReduce(v, value, eConstraints, domain);
                assign = true;
                break;
            }
        }

        if (assing){
            // Agregar a las variables instanciadas y agregar valor a la solución
            selectedVariables.push_back(v);
            solution[v] = value;

            // Si el total de variables instanciadas es igual al total de entidades
            // se encontró una solución parcial

            if(selectedVariables.size() ===  eTotal) {

                // Sobreescribir la solución óptima si la encontrada es mejor
                optimalSolution = fc::evaluate(optimalSolution, solution);

                // HERE!!! build output for parcial solution

                // Restaurar valores eliminados de dominios vecinos por inconsistencias con el valor instanciado
                fc::restore(solution[selectedVariables.pop_back()], v, eConstraints, domain[v]);
            }

            assign = false;
        }else {
            // Restaurar valores eliminados de dominios vecinos por inconsistencias con el valor instanciado
            fc::restore(solution[selectedVariables.pop_back()], v, eConstraints, domain[v]);
        }
    }*/

    return 0;
}

int selectVariable(list<int> entities, vector< vector< vector<int> > > eConstraintsMatrix, int iOrder){
    return 0;
}

bool check_forward(int value, vector< vector<int> > eConstraints, int **domain)
{
    return false;
}

void restore(int value, vector< vector<int> > eConstraints, int **domain)
{

}

void forward_checking(int entity, list<int> entities, float *eCapacities, float *rCapacities, vector< vector< vector<int> > > eConstraints, vector< vector< vector<int> > > rConstraints, int **domain)
{

}


void show_usage(string name)
{
    cerr << "Instrucciones de uso: " << name << "\n"
         << "Opciones:\n"
         << "\t-d, --dataset\t\tEl siguiente argumento luego de este parámetro\n"
         << "\t\t\t\tdebe ser el nombre del dataset a utilizar\n"
         << endl
         << "\t-i, --instance\t\tEl siguiente argumento luego de este parámetro\n"
         << "\t\t\t\tdebe ser el nombre de la instancia a utilizar\n"
         << "\t\t\t\tsin incluir la extensión.\n\n"
         << "\t** Ambas opciones son requeridas por el programa.\n\n"
         << "\t-io, --instantiation-order\t\tEl siguiente argumento puede ser:\n"
         << "\t\t\t\t - 0 : orden de instanciacion secuencial (ej: 0,1,2,...,n)\n"
         << "\t\t\t\t - 1 : orden de instanciacion por variable más conectada\n"
         << "Ejemplo: \n"
         << "\t" << name << " -d nott_data -i nott1\n\n" ;
}
