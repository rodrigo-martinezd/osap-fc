#include <iostream>
#include <vector>
#include "fc.h"

using namespace std;

namespace FC
{
  /* pseudo algorithm */

  /*

    choose variable and remove or save to a vector to not repeat
    choose a value from domain and remove that value from domain
    check constraints
    filter todos los futuros dominios de las entidades conectadas a la variable
    repeat

  */

  int chooseConnectedVariable(int variable, vector< vector< vector<int> > > eConstraints, bool criterion)
  {
    return 0;
  }

  int chooseStartVariable(int eTotal, vector< vector< vector< int > > > eConstraints, bool criterion)
  {
    return 0;
  }

  void filterDomain(int variable, int **domain)
  {

  }

  void resetDomain(int variable, int **domain)
  {

  }

  void backTracking()
  {

  }

  int* checkConstraints(int a[])
  {
    return a;
  }

  float evaluateSolution(int *solution)
  {
    return 0.0;
  }

  void forward_checking()
  {
    //escoger variable
    // escoger valor
    // verificar que no existan inconsistencias con ese valor y la variable
    // ejecutar check_and_reduce para revisar inconsistencias con vecinos y filtrar
  }

  void check_and_reduce()
  {

  }


}
