#include <iostream>
#include <vector>

namespace fc
{
  //bool isSearchFinish(int **domain);
  void ForwardChecking(eConstraints, rConstraints, domain);
  int selectVariable(int entity, std::vector<int> entityList, std::vector<int> selectedEntities, int criterion);
  vector< vector<int> > checkForward(int entity, int entityValue, std::vector< std::vector<int> > eConstraints, std::vector< std::vector<int> > rConstraints);
  //void checkAndReduce(int entity, int entityValue, std::vector< std::vector<int> > eConstraints, int **domain);
  int evaluate();
  void restore(int eFatherValue, int eValue, std::vector< std::vector<int> > eConstraints, int *eDomain);
}
