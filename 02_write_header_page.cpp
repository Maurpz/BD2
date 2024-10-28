#include <iostream>
#include <fstream>
#include <cstring>
#include "Strucs.h"
using namespace std; 


void writeToHeaderPage (const string & filename) {
  char page[PAGE_SIZE];

  memset(page, 0,  PAGE_SIZE);

  PageHeader header;
  header.pd_lsn = 1;
  header.pd_lower = sizeof(PageHeader);
  header.pd_upper = PAGE_SIZE;

  memcpy(page, &header, sizeof(PageHeader));

  ofstream file(filename, ios::binary);
  if (file.is_open()){
    file.write(page, PAGE_SIZE);
    file.close();
    cout<<"Se escribio correctamente"<<endl;
  }
  else {
    cerr<<"No se pudo escribir correctamente"<<endl;
  } 
}

int main () {
  writeToHeaderPage("./bin/02.bin");
  return 0;
}