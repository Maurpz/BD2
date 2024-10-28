#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdint>
using namespace std;

const int SIZE = 4096;

struct Header {
  uint32_t elements;
  uint16_t pd_lower;
  uint16_t pd_upper;
};

struct LinePointer {
  uint16_t offset;
};

void createHeader (const string & fileName) {
  char page[SIZE];
  memset(page, 0, SIZE);

  Header h1;
  h1.elements = 0;
  h1.pd_lower = sizeof(Header);
  h1.pd_upper = SIZE;

  memcpy(page, &h1, sizeof(Header));

  ofstream writer(fileName, ios::binary);
  if (! writer.is_open()) {
    cout<<"NO se pudo abrir"<<endl;
    return;
  }
  writer.write(page, SIZE);
  writer.close();
}

Header * readHeader (const string & fileName) {
  cout<<"\tLeer header"<<endl;
  ifstream reader(fileName, ios::binary);
  if (! reader.is_open()) {
    cout<<"NO se pudo abrir y leer el header"<<endl;
    return NULL;
  }

  Header * h1 = new Header;
  reader.read(reinterpret_cast<char*>(h1),sizeof(Header));
  cout<<"R->elements: "<<h1->elements<<endl;
  cout<<"R->lower: "<<h1->pd_lower<<endl;
  cout<<"R->upper: "<<h1->pd_upper<<endl;
  reader.close();
  return h1;
}


void insertN(const string & fileName, int value) {
  cout<<"\tinsertar n"<<endl;
  Header * h1 = readHeader(fileName);
  ofstream reader(fileName, ios::binary);
  if (! reader.is_open()) {
    cout<<"NO se pudo abrir"<<endl;
    return;
  }

  reader.seekp(h1->pd_lower,ios::beg);
  reader.write(reinterpret_cast<char*>(&value),sizeof(value));
  h1->pd_lower+=sizeof(value);
  h1->elements++;
  reader.seekp(0,ios::beg);
  reader.write(reinterpret_cast<char*>(h1),sizeof(Header));
  reader.close();
  delete h1;
}

void readN(const string & fileName, int index) {
  cout<<"\tLeer N"<<endl;
  ifstream reader(fileName, ios::binary);
  if (! reader.is_open()) {
    cout<<"NO se pudo abrir"<<endl;
    return;
  }
  Header * h1 = readHeader(fileName);

  reader.seekg(sizeof(Header),ios::beg);
  int number = 0;
  reader.read(reinterpret_cast<char*>(&number),sizeof(int));
  cout<<"El numero leido es :"<<number<<endl;
  cout<<"Y el tenemos "<<h1->elements<<" elementos almacenados";
  reader.close();
  delete h1;
}

int main () {
  string filename = "./bin/t01.bin";
  createHeader(filename);
  insertN(filename,44);
  readN(filename,33);
  return 0;
}