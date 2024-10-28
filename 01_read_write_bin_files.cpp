#include <iostream>
#include <fstream>
#include <cstring>
using namespace std;

void escribir (const char* nombre) {
  ofstream archivo("01.bin",ios::binary);
  if (!archivo) {
    cout<<"NO se pudo abrir el archivo para escribir"<<endl;
    return;
  }

  archivo.write(nombre,strlen(nombre));
  archivo.close();
  cout<<"sE escribio correctamente ene la rchivo "<<endl;
}


void leer() {
  ifstream archivo("01.bin",ios::binary);
  if (!archivo) {
    cout<<"NO se pudo abrir el archivo para escribir"<<endl;
    return;
  }

  archivo.seekg(0,ios::end);
  streamsize tamaño = archivo.tellg();
  archivo.seekg(0, ios::beg);

  char * buffer = new char[tamaño +1];
  archivo.read(buffer, tamaño);
  buffer[tamaño]=0;

  archivo.close();
  cout<<"Este es el contenido del archivo: "<<buffer<<endl;
  delete [] buffer;
}

int main () {
  const char* nombre = "Jose Mauricio";
  escribir(nombre);
  leer();
  return 0;
}