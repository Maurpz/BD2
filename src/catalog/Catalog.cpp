#include "Catalog.h"
#include "../storage/Storage.cpp"
#include "../tools/string_to_arrayChar.cpp"

Catalog::Catalog() {
  char nameI [] = {'I','N','T','E','G','E','R',0};
  char nameV [] = {'v','A','R','C','H','A','R',0};
  types["INT"] = make_unique<PgType>(121, nameI, 4, 'N');
  types["VARCHAR"] = make_unique<PgType>(5215, nameV, -1, 'S');

  r_and_w = make_unique<fstream>();
}

bool Catalog::createTable(string tableName) {
  PgClassRow tempClassRow;
  int oid = Storage::generateOID();
  tempClassRow.oid = oid;

  //asignamos el nombre de la tabla
  // int i = 0;
  // for(i;i<tableName.length();i++){
  //   tempClassRow.relname[i] = tableName[i];
  // }
  // tempClassRow.relname[i] = 0;
  string_to_arrayChar(tempClassRow.relname,tableName);
  //---------------------

  tempClassRow.relam = 1111;
  tempClassRow.relpages = 1;
  tempClassRow.reltuples = 0;
  tempClassRow.relkind = 'r';
  tempClassRow.relfilenode = oid;

  string dataBaseName = "./db_storage/data/testDB/";
  string dirName = "pg_class.bin";

  r_and_w->open(dataBaseName+dirName, ios::app | ios::binary | ios::out | ios::in);
  if (! r_and_w->is_open()) {
    cout<<"NO se pudo abrir"<<endl;
    return false;
  }
  r_and_w->write(reinterpret_cast<char*>(&tempClassRow),sizeof(PgClassRow));
  r_and_w->close();
  return true;
}

unique_ptr<PgClassRow> Catalog::getTable(string tableName) {
  char tempNameArray[MAX_SIZE_CHAR];
  int lengthName = tableName.length();
  strncpy(tempNameArray,tableName.c_str(),lengthName+1);
  tempNameArray[lengthName] = 0;

  string dataBaseName = "./db_storage/data/testDB/";
  string dirName = "pg_class.bin";

  r_and_w->open(dataBaseName+dirName, ios::binary | ios::out | ios::in);

  if (! r_and_w->is_open()) {
    cout<<"NO se pudo abrir"<<endl;
    return nullptr;
  }
  unique_ptr<PgClassRow> result = make_unique<PgClassRow>();

  while(r_and_w->read(reinterpret_cast<char*>(result.get()),sizeof(PgClassRow))) {
    if (result->relname == tableName) {
      cout<<"clase tabla encontrada : "<<result->relname<<endl;
      r_and_w->close();
      return move(result);
    }
    cout << "OID: " << result->oid <<endl;
    cout << "Nombre: " << result->relname <<endl;
    cout << "Páginas: " << result->relpages <<endl;
    cout << "Tuplas: " << result->reltuples <<endl;
    cout << "--------------------------" <<endl;
  }
  r_and_w->close();
  return nullptr;
}

void Catalog::createColumn(int tableOID, string nameColumn, string typeName,int index,bool permitNull) {
  PgAttributeRow columnTemp;
  columnTemp.attrelid = tableOID;
  columnTemp.attisdropped = false;

  //pasar el nombre  de string a char[]
  string_to_arrayChar(columnTemp.attname, nameColumn);

  //TODO: agregar validacion antes de llamar al type 
  //Todo: atributos etc  verificar si existe antes de llamar al get
  PgType * typeColumn = getType(typeName);
  columnTemp.attlen = typeColumn->typlen;
  columnTemp.atttypid = typeColumn->oid;
  columnTemp.attnum = index;
  columnTemp.attnotnull = permitNull;

  string dataBaseName = "./db_storage/data/testDB/";
  string dirName = "pg_attribute.bin";

  r_and_w->open(dataBaseName+dirName, ios::binary | ios::out | ios::app | ios::in);
  if (!r_and_w->is_open()) {
    cout<<"NO se pudo abrir el archivo"<<endl;
    return;
  }
  r_and_w->write(reinterpret_cast<char*>(&columnTemp),sizeof(PgAttributeRow));
  if(!r_and_w->good()) {
    cout<<"Quisa no se escribio correctamente enela rchivo"<<endl;
  }
  r_and_w->close();

}

PgType * Catalog::getType(string typeName) {
  if (types.find(typeName) != types.end()) {
    return types[typeName].get();
  }
  return nullptr;
}

unique_ptr<PgAttributeRow> Catalog::getColumn(int tableOID, string columnName) {
  cout<<"dentro de getColumn"<<endl;
  string dataBaseName = "./db_storage/data/testDB/";
  string dirName = "pg_attribute.bin";
  r_and_w->open(dataBaseName+dirName, ios::binary | ios::in);

  if (!r_and_w->is_open()) {
    cout<<"NO se pudo abrir el archivo"<<endl;
    return nullptr;
  }

  unique_ptr<PgAttributeRow> foundColumn = make_unique<PgAttributeRow>();

  while (r_and_w->read(reinterpret_cast<char*>(foundColumn.get()),sizeof(PgAttributeRow))) {
    if (foundColumn->attname==columnName && foundColumn->attrelid==tableOID) {
      cout<<"Se encontro la columna"<<endl;
      r_and_w->close();
      return move(foundColumn);
    }
    cout<<"nombre : "<<foundColumn->attname<<endl;
    cout<<"oid tabla : "<<foundColumn->attrelid<<endl;
    cout<<"len : "<<foundColumn->attlen<<endl;
    cout<<"posicion : "<<foundColumn->attnum<<endl;
    cout<<"-------------------"<<endl;
  }

  r_and_w->close();
  return nullptr;

}