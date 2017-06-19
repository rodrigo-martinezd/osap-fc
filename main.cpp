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

    // Read file

    

    // Represent instance

    return 0;
}
