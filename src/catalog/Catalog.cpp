#include "Catalog.h"
#include "../storage/Storage.cpp"
#include "../tools/string_to_arrayChar.cpp"
#include "../disck/DisckManager.cpp"
#include <vector>

Catalog::Catalog(DisckManager * refDisckManager) {
  char nameI [] = {'I','N','T','E','G','E','R',0};
  char nameV [] = {'v','A','R','C','H','A','R',0};
  types["INT"] = make_unique<PgType>(121, nameI, 4, 'N');
  types["VARCHAR"] = make_unique<PgType>(5215, nameV, -1, 'S');

  r_and_w = make_unique<fstream>();
  disckMg = refDisckManager;
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

  //string dataBaseName = "./db_storage/data/testDB/";
  string nameFile = "pg_class";

  //r_and_w->open(dataBaseName+dirName, ios::app | ios::binary | ios::out | ios::in);
  // if (! r_and_w->is_open()) {
  //   cout<<"NO se pudo abrir"<<endl;
  //   return false;
  // }
  // r_and_w->write(reinterpret_cast<char*>(&tempClassRow),sizeof(PgClassRow));
  // r_and_w->close();
  pair<int, int> fileFound = disckMg->findFile(nameFile);
  //* si encontro el archivo guardado
  if (fileFound.first > 0) {
    cout<<"El file pg_class Existe en el disco lo actualizaremos"<<endl;

    int offset = 0;
    unique_ptr<char []> data = disckMg->getBlockByNumber(fileFound.first, fileFound.second);
    Pg_Header pg_h;
    memcpy(&pg_h ,data.get(), sizeof(Pg_Header));
    // cout<<"offset pg "<<pg_h.offset<<endl;
    // cout<<"elements pg "<<pg_h.elements<<endl;
    offset = pg_h.offset;
    
    memcpy(data.get() + offset, &tempClassRow, sizeof(PgClassRow));
    pg_h.elements++;
    pg_h.offset+=sizeof(PgClassRow);
    memcpy(data.get(), &pg_h, sizeof(Pg_Header));
    //*actualizar el file
    disckMg->updateFile(fileFound.first, data.get(), fileFound.second);
    cout<<"Terminamos de actualizar el pg_class"<<endl;


  }
  else { //*si no existe ela archivo guardado
  //!tamaño por defecto del archivo
  int sizeFile = 2048;
  unique_ptr<char []> newData = make_unique<char []>(sizeFile);
  int offset = 0;
  Pg_Header pg_h;
  pg_h.elements = 1;
  pg_h.offset = sizeof(Pg_Header);
  pg_h.offset += sizeof(PgClassRow);

  pg_h.size = sizeFile;
  offset+= sizeof(Pg_Header);



  memcpy(newData.get(), &pg_h, sizeof(Pg_Header));

  memcpy(newData.get() + offset, &tempClassRow, sizeof(PgClassRow));

  disckMg->saveFile(nameFile, newData.get(), sizeFile);

  cout<<"Terminamos de guardar el dato en el disco"<<endl;

  }
  cout<<"Terminao la funcion create file"<<endl;

  return true;
}


//todo: getTable
unique_ptr<PgClassRow> Catalog::getTable(string tableName) {
  char tempNameArray[MAX_SIZE_CHAR];
  int lengthName = tableName.length();
  strncpy(tempNameArray,tableName.c_str(),lengthName+1);
  tempNameArray[lengthName] = 0;

  //string dataBaseName = "./db_storage/data/testDB/";
  string fileName = "pg_class";
  pair<int,int> foundFile = disckMg->findFile(fileName);

  if (foundFile.first < 0) {
    return nullptr;
  }
  /*
  r_and_w->open(dataBaseName+dirName, ios::binary | ios::out | ios::in);

  if (! r_and_w->is_open()) {
    cout<<"NO se pudo abrir"<<endl;
    return nullptr;
  }
  */
  unique_ptr<PgClassRow> result = make_unique<PgClassRow>();

  unique_ptr<char[]> data = disckMg->getBlockByNumber(foundFile.first, foundFile.second);

  Pg_Header pg_h;
  memcpy(&pg_h, data.get(), sizeof(Pg_Header));

  for (int i = sizeof(Pg_Header); i< pg_h.offset; i+= sizeof(PgClassRow)) {
    memcpy(result.get(), data.get() + i, sizeof(PgClassRow));
    if (result->relname == tableName) {
      // cout<<"clase tabla encontrada : "<<result->relname<<endl;
      // cout << "OID: " << result->oid <<endl;
      // cout << "Nombre: " << result->relname <<endl;
      // cout << "Páginas: " << result->relpages <<endl;
      // cout << "Tuplas: " << result->reltuples <<endl;
      // cout << "--------------------------" <<endl;
      return move(result);
    }
  }

  // while(r_and_w->read(reinterpret_cast<char*>(result.get()),sizeof(PgClassRow))) {
  //   if (result->relname == tableName) {
  //     //cout<<"clase tabla encontrada : "<<result->relname<<endl;
  //     r_and_w->close();
  //     // cout << "OID: " << result->oid <<endl;
  //     // cout << "Nombre: " << result->relname <<endl;
  //     // cout << "Páginas: " << result->relpages <<endl;
  //     // cout << "Tuplas: " << result->reltuples <<endl;
  //     // cout << "--------------------------" <<endl;
  //     return move(result);
  //   }

  // }
  //r_and_w->close();
  return nullptr;
}

//todo create Column

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

  //string dataBaseName = "./db_storage/data/testDB/";
  string dirName = "pg_attribute";

  //?busca un el archivo en el disco
  pair<int,int> fileFound = disckMg->findFile(dirName);

  //* SI EL FILE NO EXISTE LO CREAMOS Y GUARDAMOS LA PRIMERA COLUMNA
  if (fileFound.first < 0) {
    int sizeFile = 2048;
    unique_ptr<char []> newData = make_unique<char []>(sizeFile);
    int offset = 0;
    Pg_Header pg_h;
    pg_h.elements = 0;
    pg_h.size = sizeFile;
    pg_h.offset = sizeof(Pg_Header);
    
    memcpy(newData.get(), &pg_h, sizeof(Pg_Header));
    memcpy(newData.get() + pg_h.offset, &columnTemp, sizeof(PgAttributeRow));
    pg_h.elements++;
    pg_h.offset+=sizeof(PgAttributeRow);
    memcpy(newData.get(), &pg_h, sizeof(Pg_Header));
    disckMg->saveFile(dirName, newData.get(), sizeFile);
    cout<<"Creamos correctamente el PgAtributte"<<endl;
  }
  else {
    unique_ptr<char[]> data = disckMg->getBlockByNumber(fileFound.first, fileFound.second);
    Pg_Header pg_h;
    memcpy(&pg_h, data.get(), sizeof(Pg_Header));
    memcpy(data.get()+ pg_h.offset, &columnTemp, sizeof(PgAttributeRow));
    pg_h.offset+=sizeof(PgAttributeRow);
    pg_h.elements++;
    memcpy(data.get(), &pg_h, sizeof(Pg_Header));
    disckMg->updateFile(fileFound.first, data.get(), fileFound.second);
    cout<<"Guardamos correctamente el PgAtributte"<<endl;

  }


  // r_and_w->open(dataBaseName+dirName, ios::binary | ios::out | ios::app | ios::in);
  // if (!r_and_w->is_open()) {
  //   cout<<"NO se pudo abrir el archivo"<<endl;
  //   return;
  // }
  // r_and_w->write(reinterpret_cast<char*>(&columnTemp),sizeof(PgAttributeRow));
  // if(!r_and_w->good()) {
  //   cout<<"Quisa no se escribio correctamente enela rchivo"<<endl;
  // }
  // r_and_w->close();

}

PgType * Catalog::getType(string typeName) {
  if (types.find(typeName) != types.end()) {
    return types[typeName].get();
  }
  return nullptr;
}





unique_ptr<PgAttributeRow> Catalog::getColumn(int tableOID, string columnName) {
  //cout<<"---------dentro de getColumn-------------"<<endl;
  //string dataBaseName = "./db_storage/data/testDB/";
  string dirName = "pg_attribute";

  pair<int,int> fileFound = disckMg->findFile(dirName);
  if (fileFound.first > 0) {
    unique_ptr<PgAttributeRow> foundColumn = make_unique<PgAttributeRow>(); 
    unique_ptr<char []> data = disckMg->getBlockByNumber(fileFound.first, fileFound.second);
    Pg_Header pg_h;
    memcpy(&pg_h, data.get(), sizeof(Pg_Header));
    for (int i = sizeof(Pg_Header); i < pg_h.offset; i += sizeof(PgAttributeRow)) {
      memcpy(foundColumn.get(), data.get() + i, sizeof(PgAttributeRow));
      // cout<<"-------------------"<<endl;
      // cout<<"nombre : "<<foundColumn->attname<<endl;
      // cout<<"oid tabla : "<<foundColumn->attrelid<<endl;
      // cout<<"len : "<<foundColumn->attlen<<endl;
      // cout<<"posicion : "<<foundColumn->attnum<<endl;
      if (foundColumn->attname==columnName && foundColumn->attrelid==tableOID) {
        return move(foundColumn);
      }
    }
    cout<<"Termino la ejecucion del for del getColumnn"<<endl;

  }
  cout<<"NO se encuentra la columna deseada"<<endl;
  return nullptr;
  

  // r_and_w->open(dataBaseName+dirName, ios::binary | ios::in);

  // if (!r_and_w->is_open()) {
  //   cout<<"NO se pudo abrir el archivo"<<endl;
  //   return nullptr;
  // }


  // while (r_and_w->read(reinterpret_cast<char*>(foundColumn.get()),sizeof(PgAttributeRow))) {
  //   if (foundColumn->attname==columnName && foundColumn->attrelid==tableOID) {
  //     //cout<<"Se encontro la columna"<<endl;
  //     r_and_w->close();
  //     // cout<<"nombre : "<<foundColumn->attname<<endl;
  //     // cout<<"oid tabla : "<<foundColumn->attrelid<<endl;
  //     // cout<<"len : "<<foundColumn->attlen<<endl;
  //     // cout<<"posicion : "<<foundColumn->attnum<<endl;
  //     // cout<<"-------------------"<<endl;
  //     return move(foundColumn);
  //   }
  // }

  // r_and_w->close();
  // return nullptr;

}






int Catalog::getNumColumns(int tableOID) {
  //cout<<"----------getNumColumns---------"<<endl;
  //string dataBaseName = "./db_storage/data/testDB/";
  string dirName = "pg_attribute";
  int count = 0;
  pair<int, int> fileFound = disckMg->findFile(dirName);
  if (fileFound.first > 0) {
    unique_ptr<char []> data = disckMg->getBlockByNumber(fileFound.first, fileFound.second);
    Pg_Header pg_h;
    PgAttributeRow columnTemp;
    memcpy(&pg_h, data.get(), sizeof(Pg_Header));
    for (int i = sizeof(Pg_Header); i < pg_h.offset; i += sizeof(PgAttributeRow)) {
      memcpy(&columnTemp, data.get() + i, sizeof(PgAttributeRow));
      if (columnTemp.attrelid == tableOID) {
        count++;
      }
    }
  }
  // r_and_w->open(dataBaseName+dirName, ios::binary | ios::in);

  // if (!r_and_w->is_open()) {
  //   cout<<"NO se pudo abrir el archivo"<<endl;
  //   return -1;
  // }

  // unique_ptr<PgAttributeRow> foundColumn = make_unique<PgAttributeRow>();
  // int count = 0;

  // while (r_and_w->read(reinterpret_cast<char*>(foundColumn.get()),sizeof(PgAttributeRow))) {
  //   if (foundColumn->attrelid==tableOID) {
  //     count++;
  //   }
  // }
  // r_and_w->close();
  // //cout<<"numero de columas :"<<count<<endl;
  return count;
}





vector<PgAttributeRow> Catalog::getAllColumns(int tableOID) {
  vector<PgAttributeRow> res;
  //cout<<"------------getAllColumns----------------"<<endl;
  //string dataBaseName = "./db_storage/data/testDB/";
  string dirName = "pg_attribute";
  pair<int, int> fileFound = disckMg->findFile(dirName);

  if (fileFound.first > 0) {
    unique_ptr<char []> data = disckMg->getBlockByNumber(fileFound.first, fileFound.second);
    Pg_Header pg_h;
    PgAttributeRow column;

    memcpy(&pg_h, data.get(), sizeof(Pg_Header));

    for (int i = sizeof(Pg_Header); i < pg_h.offset; i += sizeof(PgAttributeRow)) {
      memcpy(&column, data.get() + i, sizeof(PgAttributeRow));
      if (column.attrelid == tableOID) {
        res.push_back(column);
      }
    }
  }
  // r_and_w->open(dataBaseName+dirName, ios::binary | ios::in);

  // if (!r_and_w->is_open()) {
  //   cout<<"NO se pudo abrir el archivo"<<endl;
  //   return res;
  // }
  // PgAttributeRow column;
  // int count = 0;

  // while (r_and_w->read(reinterpret_cast<char*>(&column),sizeof(PgAttributeRow))) {
  //   //cout<<"iteraciones: "<<count<<endl;
  //   if (column.attrelid == tableOID) {
  //     res.push_back(column);
  //     // cout<<"Se encontro la columna"<<endl;
  //     // cout<<"nombre : "<<column.attname<<endl;
  //     // cout<<"oid tabla : "<<column.attrelid<<endl;
  //     // cout<<"len : "<<column.attlen<<endl;
  //     // cout<<"posicion : "<<column.attnum<<endl;
  //     // cout<<"-------------------"<<endl;
  //   }
  //   count++;
  // }
  // r_and_w->close();
  return res;
}

void Catalog::increaseNumPage(string nameTable) {
  string nameFile = "pg_class";
  pair<int, int> fileFound = disckMg->findFile(nameFile);
  unique_ptr<char []> pg_class = disckMg->getBlockByNumber(fileFound.first, fileFound.second);

  Pg_Header pg_h;
  PgClassRow pg_clsrow;

  memcpy(&pg_h, pg_class.get(), sizeof(Pg_Header));

  for (int i = sizeof(Pg_Header); i < pg_h.offset; i += sizeof(PgClassRow)) {
    memcpy(&pg_clsrow, pg_class.get() + i, sizeof(PgClassRow));
    if (pg_clsrow.relname == nameTable) {
      pg_clsrow.relpages++;

      memcpy(pg_class.get() + i, &pg_clsrow, sizeof(PgClassRow));
      disckMg->updateFile(fileFound.first, pg_class.get(), fileFound.second);
      cout<<"La actualizacion del numero de paginas por tabla fue correcta"<<endl;
      return;
    }
  }
  cout<<"No se encontro la tabla para actualizar o no se pudo actualizar el nuemro de paginas por tabla"<<endl;

}


void Catalog::createIndex(string nameTable, string columnName){
  unique_ptr<PgClassRow> table = this->getTable(nameTable);
  unique_ptr<PgAttributeRow> column = this->getColumn(table->oid, columnName);

  int oid = Storage::generateOID();
  PgIndex indexTemp;
  indexTemp.oid = oid;
  indexTemp.columnIndex = column->attnum;

  //TODO -------------------------
  string nameFile = "pg_index";

  pair<int, int> fileFound = disckMg->findFile(nameFile);
  //* si encontro el archivo guardado
  if (fileFound.first > 0) {
    cout<<"El file pg_class Existe en el disco lo actualizaremos index"<<endl;

    int offset = 0;
    unique_ptr<char []> data = disckMg->getBlockByNumber(fileFound.first, fileFound.second);
    Pg_Header pg_h;
    memcpy(&pg_h ,data.get(), sizeof(Pg_Header));
    // cout<<"offset pg "<<pg_h.offset<<endl;
    // cout<<"elements pg "<<pg_h.elements<<endl;
    offset = pg_h.offset;
    
    memcpy(data.get() + offset, &indexTemp, sizeof(PgIndex));
    pg_h.elements++;
    pg_h.offset+=sizeof(PgIndex);
    memcpy(data.get(), &pg_h, sizeof(Pg_Header));
    //*actualizar el file
    disckMg->updateFile(fileFound.first, data.get(), fileFound.second);
    cout<<"Terminamos de actualizar el pg_index"<<endl;


  }
  else { //*si no existe ela archivo guardado
  //!tamaño por defecto del archivo
  int sizeFile = 2048;
  unique_ptr<char []> newData = make_unique<char []>(sizeFile);
  int offset = 0;
  Pg_Header pg_h;
  pg_h.elements = 1;
  pg_h.offset = sizeof(Pg_Header);
  pg_h.offset += sizeof(PgIndex);

  pg_h.size = sizeFile;
  offset+= sizeof(Pg_Header);



  memcpy(newData.get(), &pg_h, sizeof(Pg_Header));

  memcpy(newData.get() + offset, &indexTemp, sizeof(PgIndex));

  disckMg->saveFile(nameFile, newData.get(), sizeFile);

  cout<<"Terminamos de guardar el dato en el disco index"<<endl;

  }

  //!actualizamos el indice relacionado con la pagina

  nameFile = "pg_class";
  pair<int, int> fileFound2 = disckMg->findFile(nameFile);
  unique_ptr<char []> pg_class = disckMg->getBlockByNumber(fileFound2.first, fileFound2.second);

  Pg_Header pg_h;
  PgClassRow pg_clsrow;

  memcpy(&pg_h, pg_class.get(), sizeof(Pg_Header));

  for (int i = sizeof(Pg_Header); i < pg_h.offset; i += sizeof(PgClassRow)) {
    memcpy(&pg_clsrow, pg_class.get() + i, sizeof(PgClassRow));
    if (pg_clsrow.relname == nameTable) {
      pg_clsrow.relam = oid;

      memcpy(pg_class.get() + i, &pg_clsrow, sizeof(PgClassRow));
      disckMg->updateFile(fileFound2.first, pg_class.get(), fileFound2.second);
      cout<<"La actualizacion del numero de paginas por tabla fue correcta"<<endl;
      return;
    }
  }
  cout<<"No se encontro la tabla para actualizar o no se pudo actualizar el nuemro de paginas por tabla"<<endl;

  cout<<"Terminao la funcion create file index"<<endl;

}




unique_ptr<PgIndex> Catalog::getIndex(int indexOID) {
  string fileName = "pg_index";
  pair<int,int> foundFile = disckMg->findFile(fileName);

  if (foundFile.first < 0) {
    return nullptr;
  }
  unique_ptr<PgIndex> result = make_unique<PgIndex>();

  unique_ptr<char[]> data = disckMg->getBlockByNumber(foundFile.first, foundFile.second);

  Pg_Header pg_h;
  memcpy(&pg_h, data.get(), sizeof(Pg_Header));

  for (int i = sizeof(Pg_Header); i< pg_h.offset; i+= sizeof(PgIndex)) {
    memcpy(result.get(), data.get() + i, sizeof(PgIndex));
    if (result->oid == indexOID) {
      return move(result);
    }
  }

  return nullptr;


}
