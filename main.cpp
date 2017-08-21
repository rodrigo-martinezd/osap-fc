#include <iostream>
#include <vector>
#include <list>
#include <set>
#include <fstream>
#include <sstream>
//#include <chrono> // Add only for benchmark

using namespace std;

const float cPenalties[10] = {20, 10, 10, 10, 10, 10, 50, 10, 10, 10};
const float MAX_EVALUATION = 1000000;
const int UNUSED_CONSTRAINT         = -1;
const int ALLOCATION_CONSTRAINT     =  0;
const int NONALLOCATION_CONSTRAINT  =  1;
const int ONEOF_CONSTRAINT          =  2; // NO se utiliza en las instancias dadas
const int CAPACITY_CONSTRAINT       =  3;
const int SAMEROOM_CONSTRAINT       =  4;
const int NOTSAMEROOM_CONSTRAINT    =  5;
const int NOTSHARING_CONSTRAINT     =  6;
const int ADJACENCY_CONSTRAINT      =  7;
const int NEARBY_CONSTRAINT         =  8;
const int AWAYFROM_CONSTRAINT       =  9;

auto bench_time = chrono::high_resolution_clock::now();
bool benchmark = false;
int benchmark_type = 0; // 1 = timing, 2 = checks, 3 = quality
int checks = 0;
string instance;
int eTotal = 0;
int rTotal = 0;
int cTotal = 0;
int partials_quantity = 1;
int partials = 0;
int *solution;
int *optimalSolution;
int minEvaluation = MAX_EVALUATION;
vector< vector<float> > optimalStadistics;
set < vector<int> > optimalBSConstraints;
vector< vector< vector<int> > > eConstraints;
vector< vector< vector<int> > > rConstraints;
vector< vector<int> > rAdjacency;
vector< vector<int> > rProximity;

// Utility functions
void show_usage(string name);
bool compare_max_connected(int first, int second);
bool compare_min_connected(int first, int second);
void save_solution(set< vector<int> >broken_constraints, int *solution, vector< vector<float> > stadistics, bool partial_flag);


// Benchmark functions
void save_benchmark_checks(int checks);
void save_benchmark_timing(auto begin, auto end, float misused_space);
void save_benchmark_quality(float misused_space, float penalties);

// Algorithm FC functions
int choose_entity(int entity, list<int> *entities);
bool check_room_overflow(int entity, int room, float roomCapacity, float *eCapacities, int *cSolution);
bool check_constraints(int entity, int room, float roomCapacity, float *eCapacities, vector< vector<int> > eConstraints, vector< vector<int> > rConstraints);
set< vector<int> > getBrokenConstraints(int *pSolution, float *eCapacities, float *rCapacities, int**domain);
vector< vector<float> > evaluate(set< vector<int> > constraints, float *eCapacities, float *rCapacities, int *optSolution, int *cSolution, int **domain);
list<int> getNeighbors(int next_entity, vector< vector<int> > eConstraints);
void restore(int entity,int next_entity, vector< vector<int> > eConstraints, int **domain);
bool check_forward(int entity, int next_entity, int room, float *eCapacitites, float *rCapacitites, vector< vector< vector<int> > > eConstraints,vector< vector< vector<int> > > rConstraints, int **domain);
void forward_checking(int entity, list<int> *entities, float *eCapacities, float *rCapacities, int **domain);



int main(int argc, char* argv[])
{
    string dataset;
    ifstream file;
    int iOrder = 0;
    float *eCapacities;
    float *rCapacities;
    int **domain;
    list<int> entities;


    // Leer parametros del programa

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
            }else if (arg == "-b" || arg == "--benchmark") {
                ::benchmark = true;
                stringstream ss;
                ss << argv[++i];
                ss >> ::benchmark_type;
                if (::benchmark_type != 1 && ::benchmark_type != 2 && ::benchmark_type != 3) {
                    show_usage(argv[0]);
                    return 1;
                }
            }else if (arg == "-pq" || arg == "--partials-quantity") {
                if (i + 1 < argc) {
                    stringstream ss;
                    ss << argv[++i];
                    ss >> ::partials_quantity;
                    ::partials_quantity = ::partials_quantity > 1 ? ::partials_quantity : 1;
                }
            }else if (arg == "--help") {
                show_usage(argv[0]);
                return 0;
            }
        }

        if (dataset.size() == 0 || instance.size() == 0 )
        {
            show_usage(argv[0]);
            return 1;
        }
    }


    // Abrir archivo

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
        // Lectura y representacion

        int step = 0;
        int lineCount = 0;
        vector<int> proximity;
        int startRoom = -1;
        int endRoom = -1;
        int previousFloor = -1;
        string line;

        // Lee linea por linea y separa el procesamiento en pasos
        // se cambia de paso cuando se encuentra una linea en blanco
        // paso 0: valores generales
        // paso 1: entidades
        // paso 2: cuartos
        // paso 3: restricciones
        while(getline(file, line))
        {
            istringstream ss(line);
            lineCount++;
            if (line.empty() || (line.size() <= 2 && line[0] == '\r'))
            {
                step = step + 1 <= 4 ? step + 1 : 4;
                if (step == 1)
                {
                    // Asignar memoria

                    eCapacities = new float[eTotal];
                    rCapacities = new float[rTotal];
                    solution = new int[eTotal];
                    optimalSolution = new int[eTotal];

                    // Inicializacion de variables

                    domain = new int*[eTotal];
                    for(int i = 0; i < eTotal; i++) {
                        domain[i] = new int[rTotal];
                        for(int j = 0; j < rTotal; j++) {
                            domain[i][j] = -1;
                        }

                        solution[i] = -1;
                        optimalSolution[i] = -1;
                        entities.push_back(i);
                    }

                    // Se ajusta la memoria de los vectores y se Inicializan
                    // para que los indices correspondan con el ID de entidades y
                    // cuartos
                    eConstraints.resize(eTotal, vector< vector<int> >(1, vector<int>()));
                    rConstraints.resize(rTotal, vector< vector<int> >(1, vector<int>()));
                }
                else if (step == 3) {
                    // Añade el ultimo vector de proximidad

                    for (int i = startRoom; i <= endRoom; i++) {
                        rProximity.push_back(proximity);
                    }

                }

                // Saltar nombres de bloques: ENTITIES, ROOMS, CONSTRAINTS
                getline(file, line);
                continue;
            }

            switch(step)
            {
                case 1:
                    {
                        // Leer entidades

                        int eID;
                        int eGroupID;
                        float eCapacity;
                        ss >> eID >> eGroupID >> eCapacity;
                        eCapacities[eID] = eCapacity;
                    }
                    break;
                case 2:
                    {
                        // Leer cuartos

                        int rID;
                        int rFloor;
                        int rAdjacencySize;
                        float rCapacity;
                        int value;
                        vector<int> adjacencyVector;

                        ss >> rID >> rFloor >> rCapacity >> rAdjacencySize;

                        rCapacities[rID] = rCapacity;

                        // Construir vector de adyacencia

                        for(int i = 0; i < rAdjacencySize; i++ ){
                            ss >> value;
                            adjacencyVector.push_back(value);
                        }

                        adjacencyVector.push_back(rID);
                        rAdjacency.push_back(adjacencyVector);

                        // Guardar vector de proximidad

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
                    // Leer restricciones

                    int cID;
                    int cType;
                    int cHardness;
                    int param1;
                    int param2;

                    ss >> cID >> cType >> cHardness >> param1 >> param2;

                    // Separar entre restricciones de entidades y de cuartos

                    if (cType != 3) {
                        // Restricciones de entidades
                        if(cType != 6) {

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

                            if (cType >=4) {
                                // Guardar una copia de la restriccion en la entidad objetivo
                                // para restricciones 4, 7, 8 y 9

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
                            // Copiar las restricciones de NOT SHARING como una restriccion
                            // NOT SAME ROOM con el resto de entidades.
                            // Esto permitira al instanciar la variable con esta restriccion
                            // filtrar el valor de instanciacion de todos los dominios del resto de variables

                            for(int i = 0; i < ::eTotal; i++) {
                                if (i != param1) {
                                    if (eConstraints[i][0].size() == 0) {
                                        eConstraints[i][0].push_back(cID);
                                        eConstraints[i][0].push_back(NOTSAMEROOM_CONSTRAINT);
                                        eConstraints[i][0].push_back(cHardness);
                                        eConstraints[i][0].push_back(param1);
                                    }else {
                                        vector<int> constraint;

                                        constraint.push_back(cID);
                                        constraint.push_back(NOTSAMEROOM_CONSTRAINT);
                                        constraint.push_back(cHardness);
                                        constraint.push_back(param1);

                                        eConstraints[i].push_back(constraint);
                                    }

                                    if (eConstraints[param1][0].size() == 0) {
                                        eConstraints[param1][0].push_back(cID);
                                        eConstraints[param1][0].push_back(NOTSAMEROOM_CONSTRAINT);
                                        eConstraints[param1][0].push_back(cHardness);
                                        eConstraints[param1][0].push_back(i);
                                    }else {
                                        vector<int> constraint;

                                        constraint.push_back(cID);
                                        constraint.push_back(NOTSAMEROOM_CONSTRAINT);
                                        constraint.push_back(cHardness);
                                        constraint.push_back(i);

                                        eConstraints[param1].push_back(constraint);
                                    }
                                }
                            }
                        }


                    }else {
                        // Restricciones de cuarto

                        rConstraints[param1][0].push_back(cID);
                        rConstraints[param1][0].push_back(CAPACITY_CONSTRAINT);
                        rConstraints[param1][0].push_back(cHardness);
                        rConstraints[param1][0].push_back(-1);
                    }

                    break;
                default:
                    // Lectura de valores de cabezera

                    int value = 0;
                    string trash;

                    ss >> trash >> value;
                    eTotal = lineCount == 1 ? value : eTotal;
                    rTotal = lineCount == 2 ? value : rTotal;

                    // Distinguir entre los 2 tipos de formatos

                    cTotal = dataset == "nott_data" && lineCount == 3? value : cTotal;
                    cTotal = dataset != "nott_data" && lineCount == 4? value : cTotal;
            };
        };

        file.close();
    }

    // Ordenar la lista de entidades según criterio de instanciación
    // default: secuencial
    if (iOrder == 1) {
        entities.sort(compare_max_connected);
    }else if (iOrder == 2) {
        entities.sort(compare_min_connected);
    }


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

    cout << endl;
    cout << endl;
    cout << "++++++++++++++++++++++++++++++++++++++++++++++++++++++" <<endl;
    cout << "iOrder: " << iOrder << endl;

    cout << "orden de variables" << endl;
    for(list<int>::iterator it=entities.begin(); it!=entities.end(); ++it) {
        cout << *it << ",";
    }
    cout << endl;

    /*
    ====================================================================
                             FORWARD CHECKING
    ====================================================================
    */
    //::bench_time = chrono::high_resolution_clock::now();
    // Obtener primera entidad a instanciar
    int startEntity = choose_entity(-1, &entities);

    // Iniciar proceso recursivo
    forward_checking(startEntity, &entities, eCapacities, rCapacities, domain);

    // Mostrar solución óptima
    cout << "=============================================" << endl;
    cout << "             OPTIMAL SOLUTION                " << endl;
    cout << "=============================================" << endl;
    cout << endl;
    for(int i=0; i < ::eTotal; i++) {
        cout << ::optimalSolution[i] << ", ";
    }
    cout << "PESO:\t" << minEvaluation << endl;

    save_solution(::optimalBSConstraints, ::optimalSolution, ::optimalStadistics, false);

    return 0;
}

/*
    ===============================================================
                    Lógica de Forward Checking
    ===============================================================
*/

int choose_entity(int entity, list<int> *entities){
    /*
        Retorna el valor siguiente en la secuencia de entidades
    */
    if (entity == -1) {
        return entities->front();
    }else{
        int next_entity = -1;
        for(list<int>::iterator it=entities->begin(); it!= entities->end(); ++it) {
            if (*(it) == entity ) {
                ++it;
                next_entity = it != entities->end() ? *(it) : -1;
                break;
            }
        }

        return next_entity;
    }
}

bool check_room_overflow(int entity, int room, float roomCapacity, float *eCapacities, int *cSolution) {
    /*
        verifica si la capacidad de un cuarto fue superada
    */
    float sum = 0;
    for (int i = 0; i < ::eTotal; i++) {
        if (cSolution[i] != -1 && cSolution[i] == room) {
            sum += eCapacities[i];
        }
    }

    // Esto evita conflictos cuando se llama la función después de que la solución
    // fue instanciada completamente para obtener la lista de restricciones blandas
    // quebrantadas. Como ya se instanciaron todas las entidades se pasa entity = -1
    if (entity != -1) {
        sum += eCapacities[entity];
    }

    return sum > roomCapacity;
}

bool check_constraints(int entity, int room, float roomCapacity, float *eCapacities, vector< vector<int> > eConstraints, vector< vector<int> > rConstraints) {
    /*
        Chequea si el valor de instanciación (room) es válido.

        Como los valores de dominio inválidos en entidades conectadas por restricciones
        son filtrados. Esta función solo verifica la consistencia con las restricciones
        individuales de la entidad.
    */

    bool consistent = true;
    for(unsigned int i=0; i < eConstraints.size(); i++ ) {
        /*
            Chequea las restricciones ALLOCATION_CONSTRAINT, NONALLOCATION_CONSTRAINT
            y NOTSHARING_CONSTRAINT.
        */

        if (eConstraints[i].size()) {
            if ((eConstraints[i][1] == ::ALLOCATION_CONSTRAINT && eConstraints[i][3] != room) ||
                (eConstraints[i][1] == ::NONALLOCATION_CONSTRAINT && eConstraints[i][3] == room)) {

                // Marcar como valor inconsistente sólo si la restriccion es dura
                if (eConstraints[i][2] == 1) {
                    consistent = false;
                    break;
                }
            }else if (eConstraints[i][1] == ::NOTSHARING_CONSTRAINT) {
                bool room_already_used = false;
                int room_uses = 0;
                for(int j = 0; j < ::eTotal; j++) {
                    if (::solution[j] == room) room_uses++;
                    if (room_uses > 0) {
                        room_already_used = true;
                        break;
                    }
                }

                if (room_already_used && eConstraints[i][2] == 1) {
                    consistent = false;
                }
            }
        }
    }

    // Si el valor es consistente se procede a validar si tiene restricciones de
    // cuarto y si su capacidad fue superada
    if (consistent && rConstraints.size() && rConstraints[0].size()) {
        bool is_room_overflow = check_room_overflow(entity, room, roomCapacity, eCapacities, ::solution);
        if (rConstraints[0][2] == 1 && is_room_overflow) {
            consistent = false;
        }
    }

    return consistent;
}

set< vector<int> > getBrokenConstraints(int *pSolution, float *eCapacities, float *rCapacities, int**domain) {
    /*
        Retorna todas las restricciones suaves quebrantadas por una solución
    */

    set< vector<int> > bsConstraints;
    for(int i = 0; i < ::eTotal; i++) {
        if (::eConstraints.size()) {
            for(unsigned int j=0; j < ::eConstraints[i].size(); j++) {
                if (::eConstraints[i][j].size()) {
                    int param = ::eConstraints[i][j][3];
                    int hardness = ::eConstraints[i][j][2];
                    bool save_constraint = false;

                    // Se verifica cada restriccion contra los valores de la solucion
                    // si el valor es inválido se guarda para posterior retorno
                    switch(::eConstraints[i][j][1]) {
                        case ::ALLOCATION_CONSTRAINT:
                        {
                            if (pSolution[i] != param && hardness == 0) {
                                save_constraint = true;
                            }
                            break;
                        }
                        case ::NONALLOCATION_CONSTRAINT:
                        {
                            if (pSolution[i] == param && hardness == 0) {
                                save_constraint = true;
                            }
                            break;
                        }
                        case ::CAPACITY_CONSTRAINT:
                        {
                            int room = pSolution[i];
                            if (rConstraints[room][0].size()) {
                                if (check_room_overflow(-1,room, rCapacities[room], eCapacities, pSolution) &&
                                    hardness == 0) {
                                        save_constraint = true;
                                    }
                            }
                            break;
                        }
                        case ::SAMEROOM_CONSTRAINT:
                        {
                            if (pSolution[i] != pSolution[param] && hardness == 0) {
                                save_constraint = true;
                            }
                            break;
                        }
                        case ::NOTSAMEROOM_CONSTRAINT:
                        {
                            if (pSolution[i] == pSolution[param] && hardness == 0) {
                                save_constraint = true;
                            }
                            break;
                        }
                        case ::NOTSHARING_CONSTRAINT:
                        {
                            bool sharing = false;
                            for(int k = 0 ; k < ::eTotal; k++) {
                                if(pSolution[i] == pSolution[k]) {
                                    sharing = true;
                                    break;
                                }
                            }

                            if (sharing && hardness == 0) {
                                save_constraint = true;
                            }
                            break;
                        }
                        case ::ADJACENCY_CONSTRAINT:
                        {
                            bool adjacent = false;
                            for(int k = 0; k < ::rTotal; k++) {
                                if (rAdjacency[pSolution[i]][k] == pSolution[param]) {
                                    adjacent = true;
                                    break;
                                }
                            }

                            if (!adjacent && hardness == 0) {
                                save_constraint = true;
                            }
                            break;
                        }
                        case ::NEARBY_CONSTRAINT:
                        {
                            bool nearby = false;
                            for(int k = 0; k < ::rTotal; k++) {
                                if (rProximity[pSolution[i]][k] == pSolution[param]) {
                                    nearby = true;
                                    break;
                                }
                            }

                            if (!nearby && hardness == 0) {
                                save_constraint = true;
                            }
                            break;
                        }
                        case ::AWAYFROM_CONSTRAINT:
                        {
                            bool nearby = false;
                            for(int k = 0; k < ::rTotal; k++) {
                                if (rProximity[pSolution[i]][k] == pSolution[param]) {
                                    nearby = true;
                                    break;
                                }
                            }

                            if (nearby && hardness == 0) {
                                save_constraint = true;
                            }
                            break;
                        }
                    }

                    if (save_constraint) {
                        vector<int> constraint;
                        // Guardar cID y cType
                        constraint.push_back(::eConstraints[i][j][0]);
                        constraint.push_back(::eConstraints[i][j][1]);
                        bsConstraints.insert(constraint);
                    }
                }
            }
        }
    }
    return bsConstraints;
}

void save_benchmark_checks(int checks) {
    ofstream file;
    string path = "./out/" + ::instance + "_benchmark_checks.csv";
    vector<char> constPath(path.begin(), path.end());
    constPath.push_back('\0');
    file.open(&constPath[0], ios::app);
    file << checks << endl;
    file.close();
}

void save_benchmark_timing(auto begin, auto end, float misused_space) {
    ofstream file;
    string path = "./out/" + ::instance + "_benchmark_timing.csv";
    vector<char> constPath(path.begin(), path.end());
    constPath.push_back('\0');
    file.open(&constPath[0], ios::app);
    auto diff = chrono::duration_cast<chrono::nanoseconds>(end - begin);
    file << diff.count() << "," << misused_space <<endl;
    file.close();
}

void save_benchmark_quality(float misused_space, float penalties) {
    ofstream file;
    string path = "./out/" + ::instance + "_benchmark_quality.csv";
    vector<char> constPath(path.begin(), path.end());
    constPath.push_back('\0');
    file.open(&constPath[0], ios::app);
    file << misused_space << "," << penalties << endl;
    file.close();
}

void save_solution(set< vector<int> >broken_constraints, int *cSolution, vector< vector<float> > stadistics, bool partial_flag) {
    ofstream file;
    stringstream ss;
    ss << ::partials;
    string path = partial_flag ? ("./out/partials/" + ::instance + "_partial_" + ss.str() + ".out") :
                  "./out/" + ::instance + ".out";
    vector<char> constPath(path.begin(), path.end());
    constPath.push_back('\0');
    file.open(&constPath[0]);
    file << broken_constraints.size();
    for(set< vector<int> >::iterator it=broken_constraints.begin(); it != broken_constraints.end(); ++it) {
        file << "\t" << (*it)[0];
    }
    file << endl;
    file << stadistics[0][1] << "\t" << stadistics[0][2] << "\t" << stadistics[0][3] << endl;
    file << endl;
    file << endl;
    for(unsigned int i = 1; i < stadistics.size(); i++) {
        int roomID = (int) stadistics[i][0];
        file << roomID << "\t" << stadistics[i][1] << "\t" ;
        file <<stadistics[i][2] << "\t" << (int) stadistics[i][3];
        for(int j = 0; j < ::eTotal; j++) {
            if(cSolution[j] == roomID) {
                file << "\t" << j;
            }
        }
        file << endl;

    }
    file.close();
}


vector< vector<float> > evaluate(set< vector<int> > constraints, float *eCapacities, float *rCapacities, int *optSolution, int *cSolution, int **domain) {
    /*
        Evalua la solución parcial calcula todas las estadisticas y las retorna.
        En el caso de que la solucion parcial sea mejor a la actual óptima, la reemplaza.
    */
    // Primero se eliminan las restricciones duplicadas
    float penalties = 0, misusedSpace = 0, requiredSpace = 0;
    float unused_space = 0, overused_space = 0, total_unused = 0, total_overused = 0;
    float room_entities = 0;
    vector< vector<float> > stadistics;


    // Obtener el total de penalizaciones por restricciones suaves quebrantadas
    for(set< vector<int> >::iterator it = constraints.begin(); it != constraints.end(); ++it) {
        penalties += ::cPenalties[(*it)[1]];
    }

    // Calcula el espacio mal utilizado
    for(int i = 0; i < ::rTotal; i++) {
        for(int j = 0; j < ::eTotal; j++) {
            if (cSolution[j] == i) {
                room_entities++;
                requiredSpace += eCapacities[j];
            }
        }

        // calcula el espacio mal utilizado considerando el maximo entre el
        // espacio sin usar y dos veces el sobreuso
        misusedSpace += rCapacities[i] >= requiredSpace ? (rCapacities[i] - requiredSpace) :
                        (2*(requiredSpace - rCapacities[i]));

        unused_space = (rCapacities[i] - requiredSpace >= 0 ? rCapacities[i] - requiredSpace : 0);
        overused_space = (requiredSpace - rCapacities[i] >= 0 ? requiredSpace - rCapacities[i] : 0);
        total_unused += unused_space;
        total_overused += overused_space;

        vector<float> room_stadistics;
        room_stadistics.push_back((float) i);
        room_stadistics.push_back(unused_space);
        room_stadistics.push_back(overused_space);
        room_stadistics.push_back(room_entities);

        stadistics.push_back(room_stadistics);

        requiredSpace = 0;
        room_entities = 0;
    }

    // Just for benchmark
    if(::benchmark && ::benchmark_type == 3) {
        save_benchmark_quality(misusedSpace, penalties);
    }

    misusedSpace += penalties;

    vector<float> general_stadistics;
    general_stadistics.push_back(misusedSpace);
    general_stadistics.push_back(total_unused + total_overused);
    general_stadistics.push_back(total_unused);
    general_stadistics.push_back(total_overused);

    stadistics.insert(stadistics.begin(), general_stadistics);

    // si el peso de la solucion parcial es superior al del actual óptimo
    if (misusedSpace < ::minEvaluation) {

        // benchmark timing
        if (::benchmark && ::benchmark_type == 1) {
            // Guarda el número de soluciones parciales y el tiempo
            save_benchmark_timing(::bench_time, chrono::high_resolution_clock::now(), misusedSpace);
        }

        // Se reemplaza el optimo actual por la solucion parcial
        copy(cSolution, cSolution + ::eTotal, optSolution);
        ::optimalBSConstraints = constraints;
        ::optimalStadistics = stadistics;
        ::minEvaluation = misusedSpace;
    }
    return stadistics;
}

list<int> getNeighbors(int next_entity, vector< vector<int> > eConstraints) {
    /*
        Retorna la lista de entidades vecinas(conectadas por restricciones) sin
        instanciar.
    */
    list<int> neighbors;
    bool added_next_entity = false;
    for(unsigned int i = 0; i < eConstraints.size(); i++) {
        if (eConstraints[i].size()) {
            if (eConstraints[i][1] != ::ALLOCATION_CONSTRAINT &&
                eConstraints[i][1] != ::NONALLOCATION_CONSTRAINT &&
                eConstraints[i][1] != ::NOTSHARING_CONSTRAINT &&
                ::solution[eConstraints[i][3]] == -1) {
                    if (eConstraints[i][3] == next_entity) added_next_entity = true;
                    neighbors.push_back(eConstraints[i][3]); // save eID
                }
        }
    }

    if (!added_next_entity) {
        neighbors.push_back(next_entity);
    }

    return neighbors;
}

bool check_forward(int entity, int next_entity, int room,float *eCapacities, float *rCapacities, vector< vector< vector<int> > > eConstraints, vector< vector< vector<int> > > rConstraints, int **domain)
{
    /*
        Setea los valores de dominio de las entidades vecinas (unidas por restricciones)
        al valor de la entidad en conflicto si el valor no es consistente con cualquier
        restricción dura. En el caso de que una entidad se quede sin valores válidos
        se setea dwo (domain wipe out) a true para indicar que el valor de la
        instanciacion no es válido y se sale de la función.
    */
    //cout << "ENTIDAD: " << entity << endl;
    list<int> neighbors = getNeighbors(next_entity, eConstraints[entity]);
    for(list<int>::iterator it=neighbors.begin(); it != neighbors.end(); ++it) {
        bool dwo = true;
        for(int i = 0; i < rTotal; i++) {
            if (domain[*it][i] != -1) {
                continue;
            }else{
                ::checks++;

                bool unary_past_consistent = check_constraints(*it, i, rCapacities[i], eCapacities, eConstraints[*it], rConstraints[i]);

                if (!unary_past_consistent) {
                    domain[*it][i] = entity;
                }
                // check constraint satisfaction between room and domain[*it][i]
                // if not valid set domain[*it][i] = -1
                for (unsigned int j = 0; j < eConstraints[entity].size(); j++) {
                    if (eConstraints[entity][j].size()) {
                        if (eConstraints[entity][j][3] == *it) {
                            if(eConstraints[entity][j][1] == ::SAMEROOM_CONSTRAINT) {
                                if (i != room && eConstraints[entity][j][2] == 1) {
                                    domain[*it][i] = entity;
                                    break;
                                }
                            }else if (eConstraints[entity][j][1] == ::NOTSAMEROOM_CONSTRAINT) {
                                if (i == room && eConstraints[entity][j][2] == 1) {
                                    domain[*it][i] = entity;
                                    break;
                                }
                            }else if (eConstraints[entity][j][1] ==  ::ADJACENCY_CONSTRAINT) {
                                bool adjacent = false;
                                for(unsigned int k=0; k < ::rAdjacency[room].size(); k++) {
                                    if (::rAdjacency[room][k] == i) {
                                        adjacent = true;
                                        break;
                                    }
                                }

                                if (!adjacent && eConstraints[entity][j][2] == 1) {
                                    domain[*it][i] = entity;
                                    break;
                                }
                            }else if (eConstraints[entity][j][1] == ::NEARBY_CONSTRAINT) {
                                bool nearby = false;
                                for(unsigned int k = 0; k < ::rProximity[room].size(); k++) {
                                    if (::rProximity[room][k] == i) {
                                        nearby = true;
                                        break;
                                    }
                                }
                                if (!nearby && eConstraints[entity][j][2] == 1) {
                                    domain[*it][i] = entity;
                                    break;
                                }
                            }else if (eConstraints[entity][j][1] == ::AWAYFROM_CONSTRAINT) {
                                bool nearby = false;
                                for(unsigned int k = 0; k < ::rProximity[room].size(); k++) {
                                    if (::rProximity[room][k] == i) {
                                        nearby = true;
                                        break;
                                    }
                                }
                                if (nearby && eConstraints[entity][j][2] == 1) {
                                    domain[*it][i] = entity;
                                    break;
                                }
                            }
                        } // end if
                    }
                } // end for restricciones
            } // end else
            if (domain[*(it)][i] == -1) {
                dwo = false;
            }
        } // end domain for
        if (dwo) {
            return false;
        }
    }
    return true;
}

void restore(int entity, int next_entity, vector< vector<int> > eConstraints, int **domain)
{
    /*
        Restaura todos los valores eliminados (seteados al valor de la entidad en conflicto)
        del dominio de las entidades vecinas (conectadas por restricciones) que fueron modificados
        antes de detectar un dwo (domain-wipe-out) regresa el dominio de dichas
        entidades al estado previo al filtrado de dominio.
    */

    list<int> neighbors = getNeighbors(next_entity, eConstraints);
    for(list<int>::iterator it=neighbors.begin(); it != neighbors.end(); ++it) {
        for(int i = 0; i < rTotal; i++) {
            if (domain[*it][i] == entity) {
                domain[*it][i] = -1;
            }
        }
    }
}

void forward_checking(int entity, list<int> *entities, float *eCapacities, float *rCapacities, int **domain)
{
    bool is_consistent = false;
    int next_entity = choose_entity(entity, entities);


    // Intenta probar cada valor del dominio de la entidad
    for(int i = 0; i < rTotal; i++) {
        // Saltar todos los valores inválidos
        if (domain[entity][i] != -1) continue;

        // Chequea restricciones con la variable como de asignación o capacidad de cuarto
        is_consistent = check_constraints(entity, i, rCapacities[i], eCapacities, eConstraints[entity], rConstraints[i]);
        if (is_consistent) {
            solution[entity] = i;
            // Si se está en la ultima entidad de la lista de instanciación
            // se obtiene una solución parcial
            if (entities->back() == entity) {
                ::partials++;

                if (::benchmark && ::benchmark_type == 2) {
                    save_benchmark_checks(::checks);
                    ::checks = 0;
                }

                set< vector<int> > broken_constraints = getBrokenConstraints(solution, eCapacities, rCapacities, domain);
                vector< vector<float> > stadistics = evaluate(broken_constraints, eCapacities, rCapacities, ::optimalSolution, solution, domain);

                cout << endl << endl;
                cout << "=========================================" << endl;
                cout << "            PARTIAL SOLUTION             " << endl;
                cout << "=========================================" << endl;
                for (int j = 0; j < eTotal; j++)  {
                    cout << solution[j] << ", ";
                }
                cout << endl;
                cout << "PESO:\t " << stadistics[0][0] << endl;
                cout << endl;

                if (::partials <= ::partials_quantity) {
                    // Guarda una solución parcial
                    save_solution(broken_constraints, solution, stadistics, true);
                }
            }else {
                // Filtra los dominios de las variables adyacentes
                if (check_forward(entity,next_entity,i,eCapacities, rCapacities, eConstraints, rConstraints, domain)) {
                    forward_checking(next_entity, entities, eCapacities, rCapacities, domain);
                }
                // restaura los valores de dominio de las variables vecinas que tenian conflicto con la instaciacion
                restore(entity,next_entity, eConstraints[entity], domain);
            }
        }
    }
    ::solution[entity] = -1;
}

// Funciones de ordenamiento para indicar el orden de instanciación

bool compare_max_connected(int first, int second) {
    return (eConstraints[first].size() > eConstraints[second].size());
}

bool compare_min_connected(int first, int second) {
    return (eConstraints[first].size() < eConstraints[second].size());
}

// Ayuda del programa
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
         << "\t-io, --instantiation-order\t\tOrden de instanciacion:\n"
         << "\t\t\t\t - 0 : secuencial (ej: 0,1,2,...,n)\n"
         << "\t\t\t\t - 1 : variable más conectada primero\n"
         << "\t\t\t\t - 2 : variable menos conectada primero\n"
         <<endl
         //<< "\t-b, --benchmark\t\t genera un archivo con tiempos\n"
         <<endl
         << "\t-pq, --partials-quantity\t el siguiente argumento indica \n"
         <<"\t\t\t\tla cantidad de soluciones parciales a\n"
         <<"\t\t\t\t guardar. (default: 1)\n"
         << "Ejemplo: \n"
         << "\t" << name << " -d nott_data -i nott1\n\n" ;
}
