#include <iostream>
using namespace std;

void string_to_arrayChar(char* destinity, string origin){
  int i = 0;
  for(i;i<origin.length();i++){
    destinity[i] = origin[i];
  }
  destinity[i] = 0;

}