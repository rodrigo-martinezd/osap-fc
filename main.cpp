#include <iostream>
#include <cstdlib>

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
    float *eCapacity;
    float *rCapacity;
    int **solution;
    int *sUConstraints;
    int *domain;
    vector< vector<int> > rAdjacencyList;
    vector< vector<int> > rProximityList;
    vector< vector<int> > eConstraints;
    vector< vector<int> > rConstraints;
    const int[10] cPenalties = [20, 10, 10, 10, 10, 10, 50, 10, 10, 10];
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

    // Reading and representation process



    // Solution search process

    return 0;
}
