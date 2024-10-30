#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdint>
using namespace std;

//tamaño de la pagina
const int SIZE = 4096;
const int MAX_SIZE_CHAR = 21;

//cabecera de la pagina
struct Header {
  uint32_t elements;  //indica el numero de elementos que hay en una pagina
  uint16_t pd_lower;  //indica el limite inferios del espacio libre
  uint16_t pd_upper;  //indica el limite superios del espacio libre
};

//line pointer de cada registro
struct LinePointer {
  uint16_t offset;  //indica donde comienza el registro
  uint16_t length;  //indica el tamaño del registro
};

struct TupleHeader {
  uint16_t t_hoff;    //indica el inicio de los datos reales (tuple data)
  uint16_t t_bits;    //bitmap de valores nulos
};

//la descripcion de una tabla
struct PgClassRow {
  //deberia de tener su propi oid
  char relname[MAX_SIZE_CHAR];   //nombre de la tabla
  uint32_t relam;     //OID de la estructura del indice B-Tree , etc
  uint16_t relpages;  //numero relativo de pagainas de la tabla
  uint16_t reltuples; //numero relativo de tuplas o registros
  uint32_t relfilenode;//OID de la tabla, sera su nombre en el sistema de archivos
  char relkind;       // indica si es tabla r: regular, i: indice, v: vista
};

struct PgAttributeRow {
  uint32_t attrelid;  //OID de la tabla que describe
  char attname[MAX_SIZE_CHAR];   //nombre de la columnas "edad, id, nombre, ...etc"
  uint32_t atttypid;  //id del tipo de dato de la columna
  uint16_t attlen;    //longitud de la columna en bytes segun su tipo
  uint16_t attnum;    //posicion de la columna dentro de la tabla, comienza en 1 para el primero
  bool attnotnull;    //indica si la columna permite valores nulos true: si permite, false: no permite
  bool attisdropped;  //indica si la columna ha sido eliminada
  //deberia de implementarse el atttypmod que es para el limite de un varchar varchar(55)
};

struct PgType {
  uint32_t oid;   //identificador unico del tipo descrito
  char typname[MAX_SIZE_CHAR];  //nombre del tipo "int4, varchar"
  uint16_t typlen;  //longitud de bytes del tipo de dato
  char typcategory; //indica la categoria "N: es para tipos numeros, S: tipos de texto"
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
  fstream reader(fileName, ios::binary | ios::out | ios::in);
  if (! reader.is_open()) {
    cout<<"NO se pudo abrir"<<endl;
    return;
  }

  //crear un linePointer para el nuevo dato a insertar
  uint16_t new_pd_upper = h1->pd_upper - sizeof(value);
  LinePointer l_temp;
  l_temp.length = sizeof(value);
  l_temp.offset = new_pd_upper;

  //movemos el cabezal a su nueva dirreccion donde se escribira el registro
  reader.seekp(l_temp.offset,ios::beg);
  reader.write(reinterpret_cast<char*>(&value),l_temp.length);

  //una vez escrito debemos que agregar el line pointer
  //para ello usamos el pd_lower para agregar el linePointer
  reader.seekp(h1->pd_lower,ios::beg);
  reader.write(reinterpret_cast<char*>(&l_temp),sizeof(LinePointer));

  //seguido de esto modificamos los nuevos valores del pageHeader
  h1->pd_lower+=sizeof(LinePointer);
  h1->elements++;
  h1->pd_upper = new_pd_upper;

  //guardamos los cambios de la nueva cabecera
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
  uint16_t start = sizeof(Header) + (sizeof(LinePointer) * index);
  cout<<"Empieza a leer el linePointer desde :"<<start<<endl;
  //movemos el cabezal donde esta nuestro linepointerde nuestro registro
  reader.seekg(start,ios::beg);

  //recuperamos el linePointer correspondiente y movemos el cabezal al inicio del registro
  LinePointer l_temp;
  reader.read(reinterpret_cast<char*>(&l_temp),sizeof(LinePointer));
  reader.seekg(l_temp.offset,ios::beg);

  //recuperamos el registro
  int number = 0;
  reader.read(reinterpret_cast<char*>(&number),l_temp.length);
  cout<<"El numero leido es :"<<number<<endl;
  cout<<"Y el tenemos "<<h1->elements<<" elementos almacenados"<<endl;
  reader.close();
  delete h1;
}


void guardarTupla(string & fileName, PgAttributeRow * column,int value) {
  cout<<"\tinsertar n"<<endl;
  Header * h1 = readHeader(fileName);
  fstream reader(fileName, ios::binary | ios::out | ios::in);
  if (! reader.is_open()) {
    cout<<"NO se pudo abrir"<<endl;
    return;
  }

  //crear un linePointer para el nuevo dato a insertar
  uint16_t new_pd_upper = h1->pd_upper - sizeof(value);
  LinePointer l_temp;
  l_temp.length = sizeof(value);
  l_temp.offset = new_pd_upper;

  //movemos el cabezal a su nueva dirreccion donde se escribira el registro
  reader.seekp(l_temp.offset,ios::beg);
  reader.write(reinterpret_cast<char*>(&value),l_temp.length);

  //una vez escrito debemos que agregar el line pointer
  //para ello usamos el pd_lower para agregar el linePointer
  reader.seekp(h1->pd_lower,ios::beg);
  reader.write(reinterpret_cast<char*>(&l_temp),sizeof(LinePointer));

  //seguido de esto modificamos los nuevos valores del pageHeader
  h1->pd_lower+=sizeof(LinePointer);
  h1->elements++;
  h1->pd_upper = new_pd_upper;

  //guardamos los cambios de la nueva cabecera
  reader.seekp(0,ios::beg);
  reader.write(reinterpret_cast<char*>(h1),sizeof(Header));

  reader.close();
  delete h1;
}


int main () {
  string studentsTableName = "students";
  PgClassRow tableRow;
  uint16_t tam_nameTable = sizeof(sizeof(tableRow.relname) - 1);
  strncpy(tableRow.relname,studentsTableName.c_str(),tam_nameTable);
  tableRow.relname[tam_nameTable] = 0;
  tableRow.relpages = 1;
  tableRow.reltuples = 0;
  tableRow.relfilenode = 55555;
  tableRow.relkind = 'r';
  tableRow.relam = 0;


  //creamos 1 tipo (entero)
  PgType int4;
  int4.oid = 1234;
  int4.typcategory = 'N';
  int4.typlen = 4;
  string nameType = "INTEGER";
  strncpy(int4.typname, nameType.c_str(), sizeof(int4.typname) - 1);
  int4.typname[sizeof(int4.typname) - 1] = '\0';

  string nameColumn = "edad";
  PgAttributeRow columnTable;
  uint16_t tam_nameColumn = sizeof(columnTable.attname) - 1;
  strncpy(columnTable.attname,nameColumn.c_str(),tam_nameColumn);
  columnTable.attname[tam_nameColumn] = 0;
  columnTable.attrelid = tableRow.relfilenode;
  columnTable.atttypid = int4.oid;
  columnTable.attlen = int4.typlen;
  columnTable.attisdropped = false;
  columnTable.attnum = 1;
  columnTable.attnotnull = false;



  //---------------archivos----------------------
  string filename = "./bin/t01.bin";
  createHeader(filename);
  insertN(filename,11);
  insertN(filename,12);
  readN(filename,0);
  readN(filename,1);
  return 0;
}