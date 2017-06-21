#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>

using namespace std;

static void show_usage(string name)
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
         << "Ejemplo: \n"
         << "\t" << name << " -d nott_data -i nott1\n\n" ;
}

int main(int argc, char* argv[])
{
    string dataset, instance;
    ifstream file;
    int eTotal = 0;
    int rTotal = 0;
    int cTotal = 0;
    float *eCapacities;
    float *rCapacities;
    int *solution;
    int *domain;
    vector<int> brokenSCList;
    vector< vector<int> > rAdjacency;
    vector< vector<int> > rProximity;
    vector< vector<int> > eConstraints;
    vector< vector<int> > rConstraints;
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

            if(arg == "-d" ||  arg == "--dataset")
            {
                if (i + 1 < argc) dataset = (string) argv[++i];
                else
                {
                    show_usage(argv[0]);
                    return 1;
                }
            }
            else if (arg == "-i" || arg == "--instance")
            {
                if (i + 1 < argc) instance = (string) argv[++i];
                else
                {
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
                    domain = new int[rTotal];
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
                            cout << value << ", " << endl;
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
        for(int j = 0; j < rAdjacency[i].size(); j++) {
            cout << rAdjacency[i][j] << ", " ;
        }
        cout << endl;
        cout << "Lista de proximidad lista de: " << i << endl;
        for(int j = 0; j < rProximity[i].size(); j++) {
            cout << rProximity[i][j] << ", " ;
        }
        cout << endl;
    }
    cout << "Entities: " << eTotal << "\tRooms: " << rTotal << "\tConstraints: "<< cTotal <<endl;

    /*
    choose criterion of stop
    choose first variable to instantiate
    start process
    clone domain
    take first not choosen value of clone domain
    */
    return 0;
}
