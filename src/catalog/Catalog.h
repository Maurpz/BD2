using namespace std;

#ifndef CATALOG_H
#define CATALOG_H

const int MAX_SIZE_CHAR = 21;
#include <cstdint>
#include <map>
#include <memory>
#include <iostream>
#include <cstring>
#include <fstream>


//la descripcion de una tabla
struct PgClassRow {
  uint32_t oid;       // oid de la tabla
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

  PgType(uint32_t id, char name[], uint16_t len, char category)
    :oid(id), typlen(len), typcategory(category) {
      strncpy(typname, name, MAX_SIZE_CHAR - 1);
        typname[MAX_SIZE_CHAR - 1] = '\0';{}
    }
};

class Catalog {
  private:
    string nameBD;
    map<string,unique_ptr<PgType>> types;
    unique_ptr<fstream> r_and_w;
  public:
    Catalog();

    int findTable(string tableName);
    bool createTable(string tableName);
    void createAttribute();
    void createType();

    void getOidByTableName(string nameTable);
};


#endif