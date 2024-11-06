using namespace std;
#include <iostream>
#ifndef STORAGE_H
#define STORAGE_H

class Storage {
  private:
    static int OID;
    string nameDB;
    string hostName;
  public:
    Storage();
    static int generateOID();

    

};
#endif