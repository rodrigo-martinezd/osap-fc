#include <iostream>
#include <vector>
#include <list>
#include <fstream>
#include <sstream>

using namespace std;

int eTotal = 0;
int rTotal = 0;
int cTotal = 0;
int *solution;
int *optimalSolution;
vector< vector< vector<int> > > eConstraints;
vector< vector< vector<int> > > rConstraints;
vector< vector<int> > rAdjacency;
vector< vector<int> > rProximity;

const int cPenalties[10] = {20, 10, 10, 10, 10, 10, 50, 10, 10, 10};
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

void show_usage(string name);
bool compare_max_connected(int first, int second);
bool compare_min_connected(int first, int second);
int choose_entity(list<int> *entities);
vector< vector<int> > check_constraints(int room, float roomCapacity, float *eCapacities, vector< vector<int> > eConstraints, vector< vector<int> > rConstraints);
void build_output_file(vector< vector<int> > brokenSConstraints, int *cSolution);
void evaluate(vector< vector<int> > brokenSConstraints, int *optSolution, int *cSolution);
list<int> getNeighbors(vector< vector<int> > eConstraints);
void restore(int entity, vector< vector<int> > eConstraints, int **domain);
bool check_forward(int entity, int room, vector< vector<int> > eConstraints, int **domain);
void forward_checking(int entity, list<int> *entities, float *eCapacities, float *rCapacities, int **domain);



int main(int argc, char* argv[])
{
    string dataset, instance;
    ifstream file;
    int iOrder = 0;
    float *eCapacities;
    float *rCapacities;
    int **domain;
    list<int> entities;
    vector < vector<int> > brokenSCList; // Save cID and cType


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
                            domain[i][j] = -1;
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

    if (iOrder == 1) {
        // order list by criterion given
        entities.sort(compare_max_connected);
    }else if (iOrder == 2) {
        entities.sort(compare_min_connected);
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

    cout << endl;
    cout << endl;
    cout << "++++++++++++++++++++++++++++++++++++++++++++++++++++++" <<endl;
    cout << "iOrder: " << iOrder << endl;

    cout << "orden de variables" << endl;
    for(list<int>::iterator it=entities.begin(); it!=entities.end(); ++it) {
        cout << *it << ",";
    }
    cout << endl;

    int startEntity = choose_entity(&entities);
    cout << "entity selected: " << startEntity << endl;
    cout << "entidades restantes" << endl;
    for(list<int>::iterator it=entities.begin(); it!=entities.end(); ++it) {
        cout << *it << ",";
    }
    cout << endl;
    cout << "=========================" << endl;
    cout << "     antes de filtrar " << endl;
    cout << "=========================" << endl;
    list<int> neighbors = getNeighbors(eConstraints[0]);
    for(list<int>::iterator it=neighbors.begin(); it != neighbors.end();++it) {
        cout << "dominio de : " << *it << endl;
        for (int j = 0; j < rTotal; j++) {
            cout << domain[*it][j] << ",";
        }
        cout << endl;
    }

    neighbors.clear();
    if (check_forward(0, 0, eConstraints[0], domain)){
        cout << "=========================" << endl;
        cout << "     despues de filtrar " << endl;
        cout << "=========================" << endl;
        list<int> neighbors = getNeighbors(eConstraints[0]);
        for(list<int>::iterator it=neighbors.begin(); it != neighbors.end();++it) {
            cout << "dominio de : " << *it << endl;
            for (int j = 0; j < rTotal; j++) {
                cout << domain[*it][j] << ",";
            }
            cout << endl;
        }
    }else{
        cout << "can\'t assign" << endl;
    }

    solution[0] = 0;
    vector< vector<int> > constraints = check_constraints(0, rCapacities[0], eCapacities, eConstraints[4], rConstraints[0]);
    for(unsigned int i = 0; i < constraints.size(); i++) {
        if (i == 0) {
            cout << "is valid: " << (constraints[0][0] == 1) << endl;
        }else{
            cout << "cID: " << constraints[i][0] << "\tcType: " << constraints[i][1] << endl;
        }
    }


    restore(0, eConstraints[0], domain);

    cout << "=========================" << endl;
    cout << "     despues de restaurar " << endl;
    cout << "=========================" << endl;
    list<int> neighbors2 = getNeighbors(eConstraints[0]);
    for(list<int>::iterator it=neighbors2.begin(); it != neighbors2.end();++it) {
        cout << "dominio de : " << *it << endl;
        for (int j = 0; j < rTotal; j++) {
            cout << domain[*it][j] << ",";
        }
        cout << endl;
    }
    //forward_checking(startEntity, &entities, eCapacities, rCapacities, domain);
    // show optimal solution

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

int choose_entity(list<int> *entities){
    /*
        extrae siempre el primer elemento de la lista de entidades ya que la
        lista fue previamente ordenada según el criterio indicado al compilar
    */
    int entitySelected = entities->front();
    entities->pop_front();
    return entitySelected;
}

bool check_room_overflow(int room, float roomCapacity, float *eCapacities) {
    /*
        verifica si la capacidad de un cuarto fue superada
    */
    float sum = 0;
    for (int i = 0; i < eTotal; i++) {
        if (::solution[i] != -1 && ::solution[i] == room) {
            sum += eCapacities[i];
        }
    }

    return roomCapacity < sum; // check overflow
}

vector< vector<int> > check_constraints(int room, float roomCapacity, float *eCapacities, vector< vector<int> > eConstraints, vector< vector<int> > rConstraints) {
    /*
        Chequea si el valor de instanciación (room) es válido también Retorna
        la lista de restricciones suaves quebrantadas
    */
    vector< vector<int> > brokenSConstraints(1, vector<int>());
    // La primera celda del primer vector indica si el valor de "room" es válido
    // Los otros vectores guardan datos de restricciones suaves quebrantadas (cID, cType)
    brokenSConstraints[0].push_back(1);

    for(unsigned int i=0; i < eConstraints.size(); i++ ) {
        int entity = eConstraints[i][3];
        bool save_softconstraints = false;

        /*
            hace un chequeo exhaustivo para las restricciones ALLOCATION_CONSTRAINT,
            NONALLOCATION_CONSTRAINT, NOTSHARING_CONSTRAINT.

            mientras que para el resto asume que los valores inválidos fueron filtrados
            por lo tanto el chequeo solo es para guardar las restricciones suaves
            quebrantadas
        */

        if (eConstraints[i].size()) {
            if ((eConstraints[i][1] == ::ALLOCATION_CONSTRAINT && eConstraints[i][3] != room) ||
                (eConstraints[i][1] == ::NONALLOCATION_CONSTRAINT && eConstraints[i][3] == room)) {

                // check cHardness
                if (eConstraints[i][2] == 1) {
                    brokenSConstraints[0][0] = 0;
                    break;
                }else{
                    save_softconstraints = true;
                }
            }else if (eConstraints[i][1] == ::NOTSHARING_CONSTRAINT) {
                // la entidad no puede compartir cuarto con ninguna otra
                bool room_already_used = false;
                int room_uses = 0;
                for(int j = 0; j < eTotal; j++) {
                    if (::solution[j] == room) room_uses++;
                    if (room_uses > 1) {
                        room_already_used = true;
                        break;
                    }
                }

                if (room_already_used) {
                    if (eConstraints[i][2] == 1) {
                        brokenSConstraints[0][0] = 0;
                        break;
                    }else {
                        save_softconstraints = true;
                    }
                }
            }else if(eConstraints[i][1] == ::SAMEROOM_CONSTRAINT && ::solution[entity] != -1) {
                if (::solution[entity] != room && eConstraints[i][2] == 0) {
                    save_softconstraints = true;
                }
            }else if (eConstraints[i][1] == ::NOTSAMEROOM_CONSTRAINT && ::solution[entity] != -1) {
                if (::solution[entity] == room && eConstraints[i][2] == 0) {
                    save_softconstraints = true;
                }
            }else if (eConstraints[i][1] ==  ::ADJACENCY_CONSTRAINT && ::solution[entity] != -1) {
                bool adjacent = false;
                for(unsigned int k=0; k < ::rAdjacency[room].size(); k++) {
                    if (::rAdjacency[room][k] == ::solution[entity]) {
                        adjacent = true;
                        break;
                    }
                }

                if (!adjacent && eConstraints[i][2] == 0) {
                    save_softconstraints = true;
                }
            }else if (eConstraints[i][1] == ::NEARBY_CONSTRAINT && ::solution[entity] != -1) {
                bool nearby = false;
                for(unsigned int k = 0; k < ::rProximity[room].size(); k++) {
                    if (::rProximity[room][k] == ::solution[entity]) {
                        nearby = true;
                        break;
                    }
                }
                if (!nearby && eConstraints[i][2] == 0) {
                    save_softconstraints = true;
                }
            }else if (eConstraints[i][1] == ::AWAYFROM_CONSTRAINT && ::solution[entity] != -1) {
                bool nearby = false;
                for(unsigned int k = 0; k < ::rProximity[room].size(); k++) {
                    if (::rProximity[room][k] == ::solution[entity]) {
                        nearby = true;
                        break;
                    }
                }
                if (nearby && eConstraints[i][2] == 0) {
                    save_softconstraints = true;
                }
            }

            if (save_softconstraints) {
                vector<int> sConstraints;
                // Save cID and cType
                sConstraints.push_back(eConstraints[i][0]);
                sConstraints.push_back(eConstraints[i][1]);
                brokenSConstraints.push_back(sConstraints);
            }
        }
    }

    if (brokenSConstraints[0][0] == 1 && rConstraints.size() && rConstraints[0].size()) {
        bool is_room_overflow = check_room_overflow(room, roomCapacity, eCapacities);
        if (rConstraints[0][1] && is_room_overflow) {
            brokenSConstraints[0][0] = 0;
        }else if (is_room_overflow){
            // softconstraint
            vector<int> sConstraints;
            sConstraints.push_back(rConstraints[0][0]);
            sConstraints.push_back(rConstraints[0][1]);
            brokenSConstraints.push_back(sConstraints);
        }
    }

    return brokenSConstraints;
}

void build_output_file(vector< vector<int> > brokenSConstraints, int *cSolution) {

}

void evaluate(vector< vector<int> > brokenSConstraints, int *optSolution, int *cSolution) {

}

list<int> getNeighbors(vector< vector<int> > eConstraints) {
    /*
        Retorna la lista de entidades vecinas(conectadas por restricciones) sin
        instanciar.
    */
    list<int> neighbors;
    for(unsigned int i = 0; i < eConstraints.size(); i++) {
        if (eConstraints[i][1] != ::ALLOCATION_CONSTRAINT &&
            eConstraints[i][1] != ::NONALLOCATION_CONSTRAINT &&
            eConstraints[i][1] != ::NOTSHARING_CONSTRAINT &&
            ::solution[eConstraints[i][3]] == -1) {
                neighbors.push_back(eConstraints[i][3]); // save eID
            }
    }

    return neighbors;
}

bool check_forward(int entity, int room, vector< vector<int> > eConstraints, int **domain)
{
    /*
        Setea los valores de dominio de las entidades vecinas (unidas por restricciones)
        al valor de la entidad en conflicto si el valor no es consistente con cualquier
        restricción dura. En el caso de que una entidad se quede sin valores válidos
        se setea dwo (domain wipe out) a true para indicar que el valor de la
        instanciacion no es válido y se sale de la función.
    */
    list<int> neighbors = getNeighbors(eConstraints);
    for(list<int>::iterator it=neighbors.begin(); it != neighbors.end(); ++it) {
        bool dwo = true;
        for(int i = 0; i < rTotal; i++) {
            if (domain[*it][i] != -1) continue;
            else {
                // check constraint satisfaction between room and domain[*it][i]
                // if not valid set domain[*it][i] = -1
                for (unsigned int j = 0; j < eConstraints.size(); j++) {
                    if (eConstraints[j][3] == *it) {
                        if(eConstraints[j][1] == ::SAMEROOM_CONSTRAINT) {
                            if (i != room && eConstraints[j][2] == 1) {
                                domain[*it][i] = entity;
                                break;
                            }
                        }

                        if (eConstraints[j][1] == ::NOTSAMEROOM_CONSTRAINT) {
                            if (i == room && eConstraints[j][2] == 1) {
                                domain[*it][i] = entity;
                                break;
                            }
                        }

                        if (eConstraints[j][1] ==  ::ADJACENCY_CONSTRAINT) {
                            bool adjacent = false;
                            for(unsigned int k=0; k < ::rAdjacency[room].size(); k++) {
                                if (::rAdjacency[room][k] == i) {
                                    adjacent = true;
                                    break;
                                }
                            }

                            if (!adjacent && eConstraints[j][2] == 1) {
                                domain[*it][i] = entity;
                                break;
                            }
                        }

                        if (eConstraints[j][1] == ::NEARBY_CONSTRAINT) {
                            bool nearby = false;
                            for(unsigned int k = 0; k < ::rProximity[room].size(); k++) {
                                if (::rProximity[room][k] == i) {
                                    nearby = true;
                                    break;
                                }
                            }
                            if (!nearby && eConstraints[j][2] == 1) {
                                domain[*it][i] = entity;
                                break;
                            }
                        }

                        if (eConstraints[j][1] == ::AWAYFROM_CONSTRAINT) {
                            bool nearby = false;
                            for(unsigned int k = 0; k < ::rProximity[room].size(); k++) {
                                if (::rProximity[room][k] == i) {
                                    nearby = true;
                                    break;
                                }
                            }
                            if (nearby && eConstraints[j][2] == 1) {
                                domain[*it][i] = entity;
                                break;
                            }
                        }
                    }
                }
            }
            if (domain[*it][i] == -1) dwo = false;
        }
        if (dwo) return false;
    }

    return true;
}

void restore(int entity, vector< vector<int> > eConstraints, int **domain)
{
    /*
        Restaura todos los valores eliminados (seteados al valor de la entidad en conflicto)
        del dominio de las entidades vecinas (conectadas por restricciones) que fueron modificados
        antes de detectar un dwo (domain-wipe-out) regresa el dominio de dichas
        entidades al estado previo al filtrado de dominio.
    */

    list<int> neighbors = getNeighbors(eConstraints);
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
    // probar cada valor del dominio de la entidades
    /*float room = -1;
    vector< vector<int> > checked_constraints;
    for(int i = 0; i < rTotal; i++) {
        // Saltar todos los valores inválidos
        room = domain[entity][i];
        if (room == -1) continue;
        else {
            solution[entity] = room;
        }

        // Chequea restricciones con la variable como de asignación o capacidad de cuarto
        checked_constraints = check_constraints(room, rCapacities[room], eCapacities, eConstraints, rConstraints);
        if (checked_constraints[0][0]) {
            // add soft constraints to global variable
            if (entities->empty()) {
                // partial solution
                build_output_file(solution);
                evaluate(optimalSolution, solution);
            }else {
                // setea a -1 los valores de dominio de las variables vecinas que tiene conflicto con la instanciacion
                if (check_forward(room, eConstraints[entity], domain)) {
                    forward_checking(choose_entity(entities), entities, eCapacities, rCapacities, domain);
                }
                // restaura los valores de dominio de las variables vecinas que tenian conflicto con la instaciacion
                restore(i, eConstraints[entity], domain);
            }
        }
    }*/
}

bool compare_max_connected(int first, int second) {
    return (eConstraints[first].size() > eConstraints[second].size());
}

bool compare_min_connected(int first, int second) {
    return (eConstraints[first].size() < eConstraints[second].size());
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
         << "\t-io, --instantiation-order\t\tOrden de instanciacion:\n"
         << "\t\t\t\t - 0 : secuencial (ej: 0,1,2,...,n)\n"
         << "\t\t\t\t - 1 : variable más conectada\n"
         << "\t\t\t\t - 2 : variable menos conectada\n"
         << "Ejemplo: \n"
         << "\t" << name << " -d nott_data -i nott1\n\n" ;
}
