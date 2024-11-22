using namespace std;

#ifndef CATALOG_H
#define CATALOG_H

const int MAX_SIZE_CHAR = 21;
#include <vector>
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
  int16_t attlen;    //longitud de la columna en bytes segun su tipo
  uint16_t attnum;    //posicion de la columna dentro de la tabla, comienza en 1 para el primero
  bool attnotnull;    //indica si la columna permite valores nulos true: si permite, false: no permite
  bool attisdropped;  //indica si la columna ha sido eliminada
  //deberia de implementarse el atttypmod que es para el limite de un varchar varchar(55)
};

struct PgType {
  uint32_t oid;   //identificador unico del tipo descrito
  char typname[MAX_SIZE_CHAR];  //nombre del tipo "int4, varchar"
  int16_t typlen;  //longitud de bytes del tipo de dato
  char typcategory; //indica la categoria "N: es para tipos numeros, S: tipos de texto"

  PgType(uint32_t id, char name[], int16_t len, char category)
    :oid(id), typlen(len), typcategory(category) {
      strncpy(typname, name, MAX_SIZE_CHAR - 1);
        typname[MAX_SIZE_CHAR - 1] = '\0';{}
    }
};

//?Estructuras tenporales
//*son de prueva testeo

struct Col_Data {
  string columnName;
  string data;
};

//field para solo tipo int
// struct Field {
//   PgAttributeRow * refColumn;
//   int data;
//   bool isNull;

//   Field():isNull(true){}
// };

//*guardar datos
//field para tipo de string e int
struct Field {
  PgAttributeRow * refColumn;
  string data;
  bool isNull;

  Field():isNull(true){}
};

//*leer columnas especificas 
struct CustomField {
  unique_ptr<PgAttributeRow> refColumn;
  bool isSelected;

  CustomField(): isSelected(false){}
};

//* con un where
struct Condition {
  string column;
  string operator_;
  string value;
};

struct Field_W_Condition {
  //unique_ptr<PgAttributeRow> refColumn;
  PgAttributeRow * refColumn;
  string operator_;
  string value;
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

    //*@params create column
    //int tableOID, string nameColumn, string typeName,int index,bool permitNull
    void createColumn(int,string,string,int,bool);
    void createType();

    bool tableExists(string tableName);//
    bool columnExists(string columnName);
    bool typeExists(string typeName);

    unique_ptr<PgClassRow> getTable(string tableName);
    unique_ptr<PgAttributeRow> getColumn(int tableOID, string columnName);
    vector<PgAttributeRow> getAllColumns(int tableOID);
    int getNumColumns(int tableOID);
    PgType * getType(string typeName);

    //TODO: metodos para crear indices
};


#endif