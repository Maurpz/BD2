#include "Executor.h"
#include "../storage/Page.cpp"
#include <algorithm>
using namespace std;

Executor::Executor(){}

void Executor::setReferenceToBufferManager(BufferManager * referenceBM) {
  this->refBufferManager = referenceBM;
}

void Executor::setReferenceToCatalog(Catalog * referenceCat) {
  this->refCatalog = referenceCat;
}

//*insert para 1 sola columna
void Executor::executeInsert1(string nameTable, string nameColumn, int data) {
  //todo: falta hacer comprovaciones de la existencia de tablas y columnas 
  //todo: falta hacer validadciones de tipos a datos que se insertaran 
  //todo: saneamiento de datos
  //!estamos suponiendo que sabemos que existe la tabla
  unique_ptr<PgClassRow> table =refCatalog->getTable(nameTable);
  unique_ptr<PgAttributeRow> column = refCatalog->getColumn(table->oid,nameColumn);

  //*pedir al buffer manager la pagina
  //*por defecto pido la primera pagina pero suponog que seria la ultima
  //*para poder escribir consultar eso
  //* por el momento solo trabajamos con el mejor de los casos

  //todo podriamos implementar un diccionario cuando busque las columna asi no buscamos por iteracion
  Buffer * page = refBufferManager->loadPage(table->oid,0);

  Field field;
  field.refColumn = column.get();
  field.data = data;
  field.isNull = false;

  vector<Field> fields;
  fields.push_back(field);


  int bitmapNull = 0;
  for (size_t i = 0; i < fields.size(); i++) {
    if (fields[i].isNull) {
      bitmapNull |= 1 << i;
    }
  }

  //*creamos un offset para el buffer
  //*despues podriamos añadirlo a la misma clase buffer quisa
  int offset = 0;

  //*escribimos en la pagina primero leyendo su contenido
  Header headPage;
  memcpy(&headPage,page->getData(),sizeof(Header));
  offset = headPage.pd_lower;
  cout<<"Recuperamos el header del pagina en el buffer"<<endl;
  cout<<"pd_lower : "<<headPage.pd_lower<<endl;
  cout<<"pd_upper : "<<headPage.pd_upper<<endl;
  cout<<"numero de tuplas : "<<headPage.elements<<endl;

  //*creamos el line pointer
  LinePointer pointer;
  int dataSize = 0;
  for (size_t i = 0; i < fields.size(); i++) {
    if (!fields[i].isNull) {
      if (field.refColumn->attlen>0) {
        dataSize +=fields[i].refColumn->attlen;
      }
      //* para calcular el tamaño total sin del tupla
      // else {
      //   dataSize+=sizeof(int);
      //   dataSize+=fields[i].data.
      // }
    }
  }
  dataSize+= sizeof(TupleHeader);

  cout<<"El tamaño de la data que se insertara es de :"<<dataSize<<endl;

  cout<<1<<endl;
  cout<<"offset"<<offset<<endl;
  //*creamos el tuple header
  TupleHeader tHeader;
  tHeader.t_attnum = fields.size();
  tHeader.t_length = dataSize;
  tHeader.t_bits = bitmapNull;

  cout<<2<<endl;
  cout<<"offset"<<offset<<endl;
  //*posicionamos el cabezal para la escritura
  pointer.offset = headPage.pd_upper - dataSize;
  pointer.length = dataSize;
  offset = pointer.offset;

  tHeader.t_hoff =offset + sizeof(TupleHeader);

  cout<<3<<endl;
  cout<<"offset"<<offset<<endl;
  cout<<"nose: "<<sizeof(page)<<endl;
  memcpy(page->getData() + offset, &tHeader, sizeof(TupleHeader));
  //*actualizamos el offset con la nueva escritura
  offset += sizeof(TupleHeader);
  cout<<4<<endl;
  cout<<"offset"<<offset<<endl;
  memcpy(page->getData() + offset, &data, sizeof(int));
  offset += sizeof(int);

  cout<<5<<endl;
  cout<<"offset"<<offset<<endl;
  //*termino de escribir la tupla ahora escribiremos el linePointer
  offset = headPage.pd_lower;
  memcpy(page->getData() + offset, &pointer, sizeof(LinePointer));
  offset+= sizeof(LinePointer);

  //*final mente actualizamos los valores de PageHeader
  headPage.pd_upper -= dataSize;
  headPage.pd_lower += sizeof(LinePointer);
  headPage.elements++;
  cout<<6<<endl;
  cout<<"offset"<<offset<<endl;
  offset = 0;
  memcpy(page->getData() + offset, &headPage, sizeof(Header));
  cout<<7<<endl;
  cout<<"offset"<<offset<<endl;
  page->setIsDirty(true);

  //todo: verificar que mas se debe de ahcer con el buffer marcar com sucio y que mas procede ?


}

//todo: debemos crear un metodo para leer estos datos que se insertan

// int createBitMapNull(vector<Field> & fields){
//   int bitmapNull = 0;
//   for (size_t i = 0; i < fields.size(); i++) {
//     if (fields[i].isNull) {
//       bitmapNull |= 1 << i;
//     }
//   }
//   return bitmapNull;
// }

void Executor::executeSelectAll1(string nameTable) {

  unique_ptr<PgClassRow> table = refCatalog->getTable(nameTable);
  unique_ptr<PgAttributeRow> column = refCatalog->getColumn(table->oid,"edad");

  //*pedimos la pagina para leer
  Buffer* page = refBufferManager->loadPage(table->relfilenode,0);

  Header headerPage;
  memcpy(&headerPage, page->getData(), sizeof(Header));

  cout<<"elementos: "<<headerPage.elements<<endl;
  cout<<"lower: "<<headerPage.pd_lower<<endl;
  cout<<"upper: "<<headerPage.pd_upper<<endl;

  int offset = 0;
  TupleHeader tempTupleHeader;
  LinePointer tempPointer;
  offset+=sizeof(Header);

  for (int i = offset; i < headerPage.pd_lower; i+=sizeof(LinePointer) ) {
    int tempOffSet = 0;
    memcpy(&tempPointer,page->getData() + i, sizeof(LinePointer));
    tempOffSet = tempPointer.offset;

    memcpy(&tempTupleHeader, page->getData() + tempOffSet, sizeof(TupleHeader));
    tempOffSet = tempTupleHeader.t_hoff;
    int data = 0;
    memcpy(&data, page->getData() + tempOffSet, sizeof(int));
    cout<<"El dato almacenado es : "<<data<<endl;
  }

}



//? metodos para 2 columnas (longitud fija)

void Executor::executeInsert2(string nameTable, string column1, int data1, string column2, int data2) {
  cout<<"#############################Insert 2 ##########################"<<endl;
  unique_ptr<PgClassRow> table = refCatalog->getTable(nameTable);
  int numColumns = refCatalog->getNumColumns(table->oid);

  unique_ptr<PgAttributeRow> pg_column1 = refCatalog->getColumn(table->oid, column1);
  unique_ptr<PgAttributeRow> pg_column2 = refCatalog->getColumn(table->oid, column2);

  Field  c1;
  c1.data = data1;
  c1.refColumn = pg_column1.get();

  Field c2;
  c2.data = data2;
  c2.refColumn = pg_column2.get();

  vector<Field> data_and_Columns;
  data_and_Columns.push_back(c1);
  data_and_Columns.push_back(c2);

  int bitmapNull = PageManager::generatebitMapNull(numColumns);
  cout<<"BIts genrados default :"<<bitmapNull<<endl;

  //* podriamos cambiar los bits en un iteracion anterior
  PageManager::changeBitMap(&bitmapNull, pg_column1->attnum);
  PageManager::changeBitMap(&bitmapNull, pg_column2->attnum);

  cout<<"BIts genrados cambiados :"<<bitmapNull<<endl;



  //*pedimos el page al bufferManager
  Buffer * page = refBufferManager->loadPage(table->oid, 0); //*pedimos la primera pagina


  //todo ---------------escribir en el archivo ----------------------
  //*recuperamos el header
  int offset = 0;
  Header headPage;
  memcpy(&headPage, page->getData() + offset, sizeof(Header));

  cout<<"header lower"<<headPage.pd_lower<<endl;
  cout<<"header upper"<<headPage.pd_upper<<endl;


  //todo ordenamos los datos para poder escribirlos
  sort(data_and_Columns.begin(),data_and_Columns.end(),[] (const Field & a, const Field &b) {
    return a.refColumn->attnum < b.refColumn->attnum;
  });
  cout<<"Terminamos de ordenar"<<endl;

  TupleHeader tupleheader;
  tupleheader.t_bits = bitmapNull;
  tupleheader.t_attnum = numColumns;

  int totalDataSize = 0;

  for (Field & column : data_and_Columns) {
    totalDataSize+= column.refColumn->attlen;
  }
  cout<<"Terminamos de calcular el tamaño total de la data"<<endl;
  totalDataSize+=sizeof(TupleHeader);
  tupleheader.t_length = totalDataSize;

  LinePointer pointer;
  pointer.length = totalDataSize;

  pointer.offset = headPage.pd_upper - totalDataSize;
  tupleheader.t_hoff = pointer.offset + sizeof(TupleHeader);

  offset = pointer.offset;
  //*escribimos el tupleHeader
  cout<<"11"<<endl;
  memcpy(page->getData() + offset, &tupleheader, sizeof(TupleHeader));
  offset += sizeof(TupleHeader);

  //*escribimos la data
  for (Field & column : data_and_Columns) {
    memcpy(page->getData() + offset, &column.data, column.refColumn->attlen);
    offset+= column.refColumn->attlen;
  }
  cout<<"12"<<endl;


  //*escribimos el linePointer

  offset = headPage.pd_lower;
  memcpy(page->getData() + offset, &pointer, sizeof(LinePointer));
  headPage.pd_lower += sizeof(LinePointer);
  headPage.pd_upper = pointer.offset;
  cout<<"13"<<endl;


  //*escribimos el pageHeader
  offset = 0;
  memcpy(page->getData(), &headPage, sizeof(Header));

  //* sincronizamos el buffer con los archivos
  cout<<"14"<<endl;

  //refBufferManager->flushPage(table->relfilenode,0);//!opcional aqui

}



void Executor::executeSelectAll2(string nameTable) {
  cout<<"###############################Select all###########333333"<<endl;
  unique_ptr<PgClassRow> table = refCatalog->getTable(nameTable);
  vector<PgAttributeRow> columns = refCatalog->getAllColumns(table->oid);

  Buffer * page = refBufferManager->loadPage(table->oid,0);

  //recuperamos el headerPage
  int offset = 0;
  Header hp;
  memcpy(&hp, page->getData() + offset, sizeof(Header));

  cout<<"lower : "<<hp.pd_lower<<endl;
  cout<<"upper : "<<hp.pd_upper<<endl;

  offset = sizeof(Header);
  LinePointer p_aux;

  TupleHeader t_aux;

  for (int i = offset; i < hp.pd_lower; i+=sizeof(LinePointer)) {
    memcpy(&p_aux, page->getData() + i, sizeof(LinePointer));
    cout<<"Pointer size"<<p_aux.length<<endl;
    cout<<"Pointer offset"<<p_aux.offset<<endl;


    int dataOffset = p_aux.offset;
    memcpy(&t_aux, page->getData() + dataOffset, sizeof(TupleHeader));
    cout<<"th elements"<<t_aux.t_attnum<<endl;
    cout<<"th offset data"<<t_aux.t_hoff<<endl;
    cout<<"th size"<<t_aux.t_length<<endl;
    cout<<"th bits"<<t_aux.t_bits<<endl;


    dataOffset = t_aux.t_hoff;
    cout<<" entraremos al bucle"<<endl;
    for (PgAttributeRow & c : columns) {
      if (!PageManager::isBitNull(t_aux.t_bits, c.attnum)) {
        int data = 0;
        memcpy(&data, page->getData() + dataOffset, c.attlen);
        dataOffset+=c.attlen;
        cout<<data<<"\t";
      }
    }
    cout<<endl;
  }

}


void Executor::executeInsertString(string nameTable, string nameColumn, string data) {
  unique_ptr<PgClassRow> table = refCatalog->getTable(nameTable);
  unique_ptr<PgAttributeRow> column = refCatalog->getColumn(table->oid, nameColumn);
  int numCols = refCatalog->getNumColumns(table->oid);


  Buffer * page = refBufferManager->loadPage(table->oid, 0);

  //todo: hacer una funcion que me retorne una instancia de HeaderPage

  int offset = 0;
  Header hp;
  memcpy(&hp, page->getData() + offset, sizeof(Header));

  //todo: hacer funciones de depuracion tambien
  cout<<"lower"<<hp.pd_lower<<endl;
  cout<<"upper"<<hp.pd_upper<<endl;

  offset = hp.pd_lower;
  LinePointer pt;
  TupleHeader th;
  th.t_attnum = numCols;
  int bitmap = PageManager::generatebitMapNull(numCols);
  PageManager::changeBitMap(&bitmap, column->attnum);

  int totalSize = 0;
  totalSize+=sizeof(TupleHeader);
  
  if (column->attlen<0) {
    totalSize+=sizeof(int);
    totalSize+= data.size();
  }

  th.t_length = totalSize;
  pt.length = totalSize;

  offset = hp.pd_upper - totalSize;
  pt.offset = hp.pd_upper - totalSize;
  th.t_bits = bitmap;
  th.t_hoff = pt.offset + sizeof(TupleHeader);


  memcpy(page->getData() + offset, &th, sizeof(TupleHeader));
  offset += sizeof(TupleHeader);

  if (column->attlen < 0) {
    int sizeString = data.size();
    memcpy(page->getData() + offset, &sizeString, sizeof(int));
    offset+=sizeof(int);
    memcpy(page->getData() + offset, data.c_str(), sizeString);
  }

  offset = hp.pd_lower;
  memcpy(page->getData() + offset, &pt, sizeof(LinePointer));
  offset = 0;
  hp.elements++;
  hp.pd_lower+=sizeof(LinePointer);
  hp.pd_upper = pt.offset;
  memcpy(page->getData() + offset, &hp, sizeof(Header));
  cout<<"@@@@@@@@@@@@@@@@@@@@@ guardo correctamente"<<endl;
}

void Executor::executeSelectAllStrings(string nameTable) {
  unique_ptr<PgClassRow> table = refCatalog->getTable(nameTable);
  vector<PgAttributeRow> columns = refCatalog->getAllColumns(table->oid);

  Buffer * page = refBufferManager->loadPage(table->oid, 0);

  int offset = 0;
  Header hp;
  memcpy(&hp, page->getData() + offset, sizeof(Header));
  offset += sizeof(Header);

  LinePointer pt_aux;
  TupleHeader th_aux;

  for (int i = offset; i < hp.pd_lower; i+= sizeof(LinePointer)) {
    memcpy(&pt_aux, page->getData() + i, sizeof(LinePointer));
    int tempOffset = pt_aux.offset;

    memcpy(&th_aux, page->getData() + tempOffset, sizeof(TupleHeader));
    tempOffset = th_aux.t_hoff;

    for (PgAttributeRow & c : columns) {
      if (!PageManager::isBitNull(th_aux.t_bits, c.attnum)) {
        int sizeStr = 0;
        string data;
        memcpy(&sizeStr, page->getData() + tempOffset, sizeof(int));
        tempOffset += sizeof(int);
        data.resize(sizeStr);
        memcpy(data.data(), page->getData() + tempOffset, sizeStr);
        tempOffset+= sizeStr;
        cout<<"String recuperado : "<<data<<endl;;

      }
    }
  }
}


void Executor::insertInto (string nameTable, vector<Col_Data> & rawData) {
  cout<<"Insert Into iniciando ..."<<endl;
  unique_ptr<PgClassRow> table = refCatalog->getTable(nameTable);
  int numColumns = refCatalog->getNumColumns(table->oid);
  vector<unique_ptr<PgAttributeRow>> refColumns;

  vector<Field> fields;

  int bitmap = PageManager::generatebitMapNull(numColumns);


  for (Col_Data & e : rawData) {
    refColumns.push_back(refCatalog->getColumn(table->oid, e.columnName));
    Field tempF;
    tempF.data = e.data;
    tempF.isNull = false;
    tempF.refColumn = refColumns.back().get();
    PageManager::changeBitMap(&bitmap, tempF.refColumn->attnum);
    fields.push_back(tempF);
  }

  sort(fields.begin(),fields.end(),[] (const Field & a, const Field &b) {
    return a.refColumn->attnum < b.refColumn->attnum;
  });
  Buffer * page = refBufferManager->loadPage(table->oid, 0);



  // //*escribir en el archivo
  // //*calcular el tamaño total de la tupla
  // int totalSize = 0;
  // totalSize += sizeof(TupleHeader);

  // for (Field & column : fields) {
  //   if (column.refColumn->attlen < 0) {
  //     totalSize += sizeof(int); //*sumamos el valor que indicara el tamaño del string
  //     totalSize += column.data.size();
  //   }
  //   else {
  //     totalSize += column.refColumn->attlen;
  //   }
  // }

  // //*recuperamos la pagina para escribir en ella
  // Buffer * page = refBufferManager->loadPage(table->oid, 0);

  // //*Preparamos las estrcuturas que usaremos
  // int offset = 0;
  // Header hp;
  // LinePointer l_ptr;
  // TupleHeader th;

  // //*recuperamos el headerPage
  // memcpy(&hp, page->getData() + offset, sizeof(Header));

  // //*calculamos donde deberia de empezar a escribir la tupla
  // offset = hp.pd_upper - totalSize;

  // //*primero llenamos los datos para el TupleHeader
  // th.t_attnum = numColumns; 
  // th.t_bits = bitmap;
  // th.t_length = totalSize;
  // th.t_hoff = offset + sizeof(TupleHeader);

  // //*escribimos en el buffer el tupleHeader
  // memcpy(page->getData() + offset, &th, sizeof(TupleHeader));
  // offset += sizeof(TupleHeader);

  // //*procedemos a escribir la data
  // for (Field & column : fields) {
  //   if (column.refColumn->attlen < 0) {
  //     int size = column.data.size();
  //     memcpy(page->getData() + offset, &size, sizeof(int));//*escribimos el tamaño
  //     offset+= sizeof(int);
  //     memcpy(page->getData() + offset, column.data.c_str(), size);
  //     offset += size;
  //   }
  //   else {
  //     int num = stoi(column.data);
  //     memcpy(page->getData() + offset, &num, sizeof(int));
  //     offset += sizeof(int);
  //   }
  // }

  // //*ahora definimos los valores del LinePointer y lo escribimos 
  // l_ptr.length = totalSize;
  // l_ptr.offset = hp.pd_upper - totalSize;

  // //*actualizamos el offset
  // offset = hp.pd_lower;

  // memcpy(page->getData() + offset, &l_ptr, sizeof(LinePointer));

  // //*finalmente actualizamos los datos del HeaderPage y lo escribimos
  // hp.elements++;
  // hp.pd_lower += sizeof(LinePointer);
  // hp.pd_upper -= totalSize;

  // //*actualizamos el offset aunque tambien podriamos no hacerlo
  // memcpy(page->getData(), &hp, sizeof(Header));
  PageManager::writeRegisterInBuffer(fields,numColumns, bitmap, page);

  cout<<"Inset Into terminada ..."<<endl;


}

//todo ------------------------------------Select All--------------------------------

void Executor::selectAll(string nameTable) {
  unique_ptr<PgClassRow> table = refCatalog->getTable(nameTable);

  vector<PgAttributeRow> columns = refCatalog->getAllColumns(table->oid);

  //?podriamos ordenar las columnas aunque por defecto ya deberian de estarlo

  //*recuperamos la pagina para poder leer los datos
  Buffer * page = refBufferManager->loadPage(table->oid, 0);

  //*alistamos los recursos que usaremos
  int offset = 0;
  Header hp;
  LinePointer l_ptr;
  TupleHeader th;

  //*recuperamos el headerPage
  memcpy(&hp, page->getData() + offset, sizeof(Header));

  offset += sizeof(Header);


  for (int i = offset; i < hp.pd_lower; i += sizeof(LinePointer)) {
    memcpy(&l_ptr, page->getData() + i, sizeof(LinePointer));
    int tempOffset = l_ptr.offset;
    if (l_ptr.flags == LinePointerFlags::LP_DEAD) {
      cout<<"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$4 el punetoa punta aun registro eliminado"<<endl;
      continue;
    }

    memcpy(&th, page->getData() + tempOffset, sizeof(TupleHeader));
    tempOffset += sizeof(TupleHeader);

    for (PgAttributeRow & c : columns) {
      if (!PageManager::isBitNull(th.t_bits,c.attnum)) {
        if (c.attlen < 0) {
          int size;
          string data;
          memcpy(&size, page->getData() + tempOffset, sizeof(int));
          tempOffset+= sizeof(int);

          data.resize(size);
          memcpy(data.data(), page->getData() + tempOffset, size);
          tempOffset += size;

          cout<<data<<"\t";
        }
        else {
          int data;
          memcpy(&data, page->getData() + tempOffset, c.attlen);
          tempOffset+=c.attlen;
          cout<<data<<"\t";
        }
      }
      else {
        cout<<"null\t";
      }
    }
    cout<<endl;

  }
  cout<<"Terminamso de leer los datos"<<endl;

}

//todo-----------------------------Select Custom--------------------------------

void Executor::selectCustom(string nametable, vector<string> & fields) {
  unique_ptr<PgClassRow> table = refCatalog->getTable(nametable);
  vector<PgAttributeRow> columns = refCatalog->getAllColumns(table->oid);

  vector<CustomField> customFields;

  for (PgAttributeRow & c : columns) {
    bool match = false;
    for (string field : fields) {
      if (c.attname == field) {
        customFields.emplace_back();
        customFields.back().refColumn = move(refCatalog->getColumn(table->oid, field));
        customFields.back().isSelected = true;
        match = true;
        break;
      }
    }
    if (!match) {
      customFields.emplace_back();
      customFields.back().refColumn = move(refCatalog->getColumn(table->oid, c.attname));
    }
  }

  //*recuperamos el buffer para leerlo
  Buffer * page = refBufferManager->loadPage(table->oid, 0);

  //*definimos nuestro variables auxiliares
  int offset = 0;
  Header hp;
  LinePointer l_ptr;
  TupleHeader th;

  //*recuperamos el header
  memcpy(&hp, page->getData() + offset, sizeof(Header));
  offset += sizeof(Header);

  for (int i = offset; i < hp.pd_lower; i+= sizeof(LinePointer)) {
    memcpy(&l_ptr, page->getData() + i, sizeof(LinePointer));

    int tempOffset = l_ptr.offset;

    //*recuperamos el tuple header
    memcpy(&th, page->getData() + tempOffset, sizeof(TupleHeader));
    tempOffset = th.t_hoff;

    for (CustomField & field : customFields) {
      
      if (field.isSelected ) {
        if (!PageManager::isBitNull(th.t_bits, field.refColumn->attnum)) {
          if (field.refColumn->attlen < 0) {
            int size;
            string data;
            memcpy(&size, page->getData() + tempOffset, sizeof(int));
            tempOffset+= sizeof(int);
            data.resize(size);
            memcpy(data.data(), page->getData() + tempOffset, size);
            tempOffset+= size;
            cout<<data<<"\t";

          }
          else {
            int num;
            memcpy(&num, page->getData() + tempOffset, field.refColumn->attlen);
            tempOffset+= field.refColumn->attlen;
            cout<<num<<"\t";
          }
        }
        else {
          cout<<"null\t";
        }
      }
      else if (!field.isSelected && !PageManager::isBitNull(th.t_bits, field.refColumn->attnum)) {
        if (field.refColumn->attlen < 0) {
          int size;
          memcpy(&size, page->getData() + tempOffset, sizeof(int));
          tempOffset+=sizeof(int);
          tempOffset+= size;
        }
        else {
          tempOffset+= field.refColumn->attlen;
        }
      }

    }
    cout<<endl;
  }

  cout<<"Terminamos el custom select"<<endl;
}

//todo--------------------------------Select All Where-----------------------------

void Executor::selectAllWhere(string nameTable, vector<Condition> & conditions) {
  unique_ptr<PgClassRow> table = refCatalog->getTable(nameTable);
  vector<PgAttributeRow> columns = refCatalog->getAllColumns(table->oid);

  vector<Field_W_Condition> fields;

  for (PgAttributeRow & col : columns) {
    bool match = false;
    for (Condition& con : conditions) {
      if (col.attname == con.column) {
        cout<<"\t################## "<<con.operator_<<endl;
        Field_W_Condition temp;
        temp.refColumn = &col;
        temp.operator_ = con.operator_;
        temp.value = con.value;
        cout<<"\t################## QQQ  "<<temp.operator_<<endl;
        fields.push_back(temp);
        match = true;
      }
    }
    if (!match) {
      Field_W_Condition temp;
      temp.refColumn = &col;
      temp.operator_ = '0';
      temp.value = '0';
      fields.push_back(temp);
    }

  }

  Buffer * page = refBufferManager->loadPage(table->oid,0);

  //* definimos nuestros datos  usar
  Header hp;
  LinePointer l_ptr;
  TupleHeader th;
  int offset = 0;

  memcpy(&hp, page->getData() +offset, sizeof(Header));
  offset += sizeof(Header);

  for (int i = offset; i < hp.pd_lower; i+=sizeof(LinePointer)) {
    int tempOffset = 0;
    memcpy(&l_ptr, page->getData() + i, sizeof(LinePointer));
    tempOffset = l_ptr.offset;

    memcpy(&th, page->getData() + tempOffset, sizeof(TupleHeader));
    tempOffset+= sizeof(TupleHeader);


    for (Field_W_Condition field : fields) {
      if (!PageManager::isBitNull(th.t_bits, field.refColumn->attnum)) {
        if (field.refColumn->attlen < 0) {
          int size;
          string data;
          memcpy(&size, page->getData() + tempOffset, sizeof(int));
          tempOffset+= sizeof(int);
          data.resize(size);
          memcpy(data.data(), page->getData() + tempOffset, size);
          tempOffset+= size;
          if (!checkCondition_S(data, field.operator_, field.value)) {
            //si no cumple break
            cout<<"NO cumple con la condicion string"<<endl;
            break;
          }
          cout<<data<<"\t";

        }
        else {
          int num;
          memcpy(&num, page->getData() + tempOffset, field.refColumn->attlen);
          tempOffset+= field.refColumn->attlen;
          if (!checkCondition_N(num, field.operator_, stoi(field.value))) {
            //si no cumple break
            cout<<"NO cumple con la condicion int"<<endl;
            break;
          }
          cout<<num<<"\t";
        }
      }
      else {
        cout<<"null\t";
      }
    }
    cout<<endl;
  } 
}

//todo  select with where  v2 
 vector<vector<string>> Executor::selectAllWherev2(string nameTable, vector<Condition> & conditions) {
  unique_ptr<PgClassRow> table = refCatalog->getTable(nameTable);
  vector<PgAttributeRow> columns = refCatalog->getAllColumns(table->oid);

  vector<Field_W_Condition> fields;

  for (PgAttributeRow & col : columns) {
    Field_W_Condition temp;
    temp.refColumn = &col;
    temp.operator_ = '0';
    temp.value = '0';

    for (Condition& con : conditions) {
      if (col.attname == con.column) {
        cout<<"\t################## "<<con.operator_<<endl;
        temp.operator_ = con.operator_;
        temp.value = con.value;
        cout<<"\t################## QQQ  "<<temp.operator_<<endl;
        break;
      }
    }
    fields.push_back(temp);
  }

  //*obtemos la pagina para hacer la busqueda y filtrado
  Buffer * page = refBufferManager->loadPage(table->oid,0);

  //* definimos la estrcutura que sera devuelta
  vector<vector<string>> result;

  //* definimos nuestros datos  usar
  Header hp;
  LinePointer l_ptr;
  TupleHeader th;
  int offset = 0;

  memcpy(&hp, page->getData() +offset, sizeof(Header));
  offset += sizeof(Header);

  //* inciamos a leer los linePointers
  for (int i = offset; i < hp.pd_lower; i+=sizeof(LinePointer)) {
    vector<string> tempRowValues;
    int tempOffset = 0;
    memcpy(&l_ptr, page->getData() + i, sizeof(LinePointer));
    tempOffset = l_ptr.offset;

    memcpy(&th, page->getData() + tempOffset, sizeof(TupleHeader));
    tempOffset+= sizeof(TupleHeader);
    int conditionsSuccessfully = 0;


    for (Field_W_Condition field : fields) {
      //*revisamos si el valor de la columna esta almacenada en la pagina
      if (!PageManager::isBitNull(th.t_bits, field.refColumn->attnum)) {
        if (field.refColumn->attlen < 0) {
          int size;
          string data;
          memcpy(&size, page->getData() + tempOffset, sizeof(int));
          tempOffset+= sizeof(int);
          data.resize(size);
          memcpy(data.data(), page->getData() + tempOffset, size);
          tempOffset+= size;
          if (!checkCondition_S(data, field.operator_, field.value)) {
            //si no cumple break
            cout<<"NO cumple con la condicion string"<<endl;
            conditionsSuccessfully--;
            break;
          }
          tempRowValues.push_back(data);
          cout<<data<<"\t";

        }
        else {
          int num;
          memcpy(&num, page->getData() + tempOffset, field.refColumn->attlen);
          tempOffset+= field.refColumn->attlen;
          if (!checkCondition_N(num, field.operator_, stoi(field.value))) {
            //si no cumple break
            cout<<"NO cumple con la condicion int"<<endl;
            conditionsSuccessfully--;
            break;
          }
          tempRowValues.push_back(to_string(num));
          cout<<num<<"\t";
        }
      }

      //* como no esta almacenada entonces  agregamos un null
      else {
        cout<<"null\t";
        tempRowValues.push_back("null");
      }
    }
    //*una vez terminado revisamos si cumnplio con todas las condiciones
    if (!(conditionsSuccessfully < 0)) {
      cout<<"Se agrega a la lista de resyult"<<endl;
      result.push_back(tempRowValues);
    }
    cout<<endl;
  }
  cout<<"iniciooooooooooooooooooooooooooo"<<endl;
  cout<<"EL atamano del result es :"<<result.size()<<endl;
  for (auto row : result) {
    for (string value : row) {
      cout<<value<<"\t";
    }
    cout<<endl;
  } 
  cout<<"Finalllllllllllll"<<endl;
  return result;
}



//todo update



void Executor::updateRegister(string nameTable, vector<pair<string,string>> columnsAndValues, vector<Condition> conditions, bool isUnique) {
  unique_ptr<PgClassRow> table = refCatalog->getTable(nameTable);
  vector<PgAttributeRow> columns = refCatalog->getAllColumns(table->oid);

  int numColumns = refCatalog->getNumColumns(table->oid);

  //* campos a actualizar
  vector<Field> fields;

  int bitmap = PageManager::generatebitMapNull(numColumns);

  for ( PgAttributeRow & column : columns) {
    Field temp;
    temp.refColumn = &column;

    for (auto & data : columnsAndValues) {
      if (column.attname == data.first) {
        temp.data = data.second;
        temp.isNull = false;
        PageManager::changeBitMap(&bitmap, column.attnum);
        break;
      }
    }
    temp.refColumn = &column;
    fields.push_back(temp);
  }

  //* campos para la revision de condicionales

  vector<Field_W_Condition> fieldsConditions;

  for (PgAttributeRow & col : columns) {
    Field_W_Condition temp;
    temp.refColumn = &col;
    temp.operator_ = '0';
    temp.value = '0';

    for (Condition& con : conditions) {
      if (col.attname == con.column) {
        temp.operator_ = con.operator_;
        temp.value = con.value;
        break;
      }
    }
    fieldsConditions.push_back(temp);
  }

  //*recupermaos la pagina del buffer
  Buffer * page = refBufferManager->loadPage(table->oid, 0);
  //*declaramos  nuestras estructuras auxiliares
  int offset = 0;
  Header hp;
  LinePointer l_ptr;
  TupleHeader th;

  memcpy(&hp, page->getData() + offset, sizeof(Header));
  offset+= sizeof(Header);

  cout<<"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@qqupdate"<<endl;
  for (int i = offset; i < hp.pd_lower; i+=sizeof(LinePointer)) {
    cout<<i<<endl;

    memcpy(&l_ptr,page->getData() + i, sizeof(LinePointer));
    cout<<"el primer offset del pointer es :"<<l_ptr.offset<<endl;

    //todo: aqui deberia mos de revisar si el linepointer es valido o no
    vector <Field> fieldsTemp;
    fieldsTemp = fields;
    int tempBitmap = bitmap;
    int tempOffset = 0;
    tempOffset = l_ptr.offset;
    memcpy(&th, page->getData() + tempOffset, sizeof(TupleHeader));
    tempOffset+=sizeof(TupleHeader);
    bool allConditionsSuccessfully = true;
    cout<<"tamaño del vector original  :"<<fields.size()<<endl;
    cout<<"tamaño del vector copia "<<fieldsTemp.size()<<endl;

    for (int j = 0; j < fieldsTemp.size(); j++) {
      cout<<"\t verifiacr si esta columna tiene condicional  "<<fieldsConditions[j].operator_<<fieldsConditions[j].value<<endl;
      //*si se encuentra alamcenado en la tupla 
      if (!PageManager::isBitNull(th.t_bits,fieldsTemp[j].refColumn->attnum)) {
        cout<<"El bit es correcto"<<endl;
        //*ahora revisamos el tipo de dato string o int
        if (fieldsTemp[j].refColumn->attlen < 0) {
          cout<<"es un string"<<endl;
          int size = 0;
          string data;
          memcpy(&size, page->getData() + tempOffset, sizeof(int));
          data.resize(size);
          cout<<"El tamaño del string a recuperar"<<size<<endl;
          tempOffset+= sizeof(int);
          memcpy(data.data(), page->getData() + tempOffset, size);
          tempOffset += size;
          if (!checkCondition_S(data,fieldsConditions[j].operator_,fieldsConditions[j].value)) {
            cout<<"NO cumple la condicion del string"<<endl;
            allConditionsSuccessfully = false;
            break;
          }
          cout<<"Data :"<<data<<"  "<<endl;
          if (fields[j].isNull) {
            fieldsTemp[j].data = data;
            fieldsTemp[j].isNull = false;
            PageManager::changeBitMap(&tempBitmap,fieldsTemp[j].refColumn->attnum);
          }
        }
        else if (fieldsTemp[j].refColumn->attlen == 4){
          cout<<"Es un numero"<<endl;
          //*si la dimension es un numero entero
          int number;
          memcpy(&number, page->getData() + tempOffset, sizeof(int));
          tempOffset+= sizeof(int);
          if (!checkCondition_N(number,fieldsConditions[j].operator_,stoi(fieldsConditions[j].value))) {
            allConditionsSuccessfully = false;
            break;
          }
          if (fields[j].isNull){
            fieldsTemp[j].data = to_string(number);
            fieldsTemp[j].isNull = false;
            PageManager::changeBitMap(&tempBitmap,fieldsTemp[j].refColumn->attnum);

          }
          cout<<"numero :"<<number<<"  "<<endl;


        }
      }
      else {
        cout<<"El no es corecto"<<endl;
        //*no se encuentra almacenado en la apgina la columna actual
      }
    }
    //*si todas las condiciones fueron verdaderas entonces actualizamos el valor


    for (Field & f : fieldsTemp) {
      cout<<"contenido : "<<f.data<< "  estado : "<<f.isNull<<"  column : "<<f.refColumn->attname<<endl;
    }
    if (allConditionsSuccessfully) {
      l_ptr.offset = PageManager::writeTuple(fieldsTemp, numColumns, tempBitmap, page);
      cout<<"el nuevo offset del pointer es :"<<l_ptr.offset<<endl;
      memcpy(page->getData() + i, &l_ptr, sizeof(LinePointer));
      if (isUnique) {
        cout<<"Es una actualizacion unica solo se hara 1 vez"<<endl;
        break;
      }

    }

  }
  cout<<"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@qqupdate"<<endl;
} 


//todo: delete register

void Executor::deleteRegister(string nameTable, vector<Condition> conditions, bool isUnique) {
  unique_ptr<PgClassRow> table = refCatalog->getTable(nameTable);
  vector<PgAttributeRow> columns = refCatalog->getAllColumns(table->oid);


  //* hacer un vector de condiciones
  vector<Field_W_Condition> fieldConditions;

  for (PgAttributeRow & col : columns) {
    Field_W_Condition temp;
    temp.operator_ = "0";
    temp.value = "0";
    temp.refColumn = &col;
    for (Condition & c : conditions) {
      if (c.column == col.attname) {
        temp.operator_ = c.operator_;
        temp.value = c.value;
        break;
      }
    }
    fieldConditions.push_back(temp);
  }

  //*pedimos la pagina al buffer la pagina del buffer
  //todo: averigiar si cuadno hace secuencial scan postgres pide todas las paginas que tiene una tabla o como hace si la pide 1 por 1  o todas de golpe e itera en ellas

  Buffer * page = refBufferManager->loadPage(table->oid, 0);

  //* ahora hacemos el scuencias scan

  //*definimos nuestras variables auxiliares
  Header hp;
  LinePointer l_ptr;
  TupleHeader th;
  int offset = 0;
  // memcpy(&hp, page->getData() + offset, sizeof(Header));
  // offset += sizeof(Header);
  //*recuperamos el headerPage
  PageManager::copyFromBuffer(&hp,page->getData(),offset,sizeof(Header));
  cout<<"????????????????????????????????empezo  el delete"<<endl;


  for (int i = offset; i < hp.pd_lower; i+= sizeof(LinePointer)) {
    int tempOffset = i;
    PageManager::copyFromBuffer(&l_ptr,page->getData(),tempOffset,sizeof(LinePointer));

    //* recuperamos el tupleHeader del primer registro
    tempOffset = l_ptr.offset;
    PageManager::copyFromBuffer(&th,page->getData(),tempOffset,sizeof(TupleHeader));

    int allConditionsSuccessfully = 0;

    for (Field_W_Condition & field : fieldConditions) {
      //*comprovamos que no sea exista un valor registrado para la columna actual
      if (!PageManager::isBitNull(th.t_bits,field.refColumn->attnum)) {
        //*ahora comprovamos su tamaño si es variable o fijo
        if (field.refColumn->attlen < 0) {
          //* es un string
          int sizeString = 0;
          string data;
          PageManager::copyFromBuffer(&sizeString,page->getData(),tempOffset,sizeof(int));
          data.resize(sizeString);
          PageManager::copyFromBuffer(data.data(),page->getData(),tempOffset,sizeString);
          if (!checkCondition_S(data,field.operator_,field.value)) {
            cout<<"NO cumple con la condicion en string"<<endl;
            allConditionsSuccessfully--;
            break;
          }
          cout<<"DATA: "<<data<<"\t";
        }
        else if (field.refColumn->attlen == 4) {
          int number = 0;
          PageManager::copyFromBuffer(&number, page->getData(), tempOffset, sizeof(int));

          if (!checkCondition_N(number, field.operator_, stoi(field.value))) {
            cout<<"No cumple conla condicion de int"<<endl;
            allConditionsSuccessfully--;
            break;
          }
          cout<<"Number"<<number<<"\t";
        }
      }
    }
    cout<<endl;
    if (allConditionsSuccessfully == 0) {
      l_ptr.flags = LinePointerFlags::LP_DEAD;
      PageManager::copyToBuffer(page->getData(), &l_ptr, i, sizeof(LinePointer));
      th.flags = TupleHeaderFlags::TH_DELETED;
      tempOffset = l_ptr.offset;
      PageManager::copyToBuffer(page->getData(), &th, tempOffset, sizeof(TupleHeader));
      if (isUnique) {
        cout<<"LA condicion es unica en su fila"<<endl;
        break;
      }
    }

  }
  cout<<"????????????????????????????????Termino el delete"<<endl;



}



bool Executor::checkCondition_S (string a, string operator_, string b) {
  if (operator_ == "=") return (a == b);
  return true;
}

bool Executor::checkCondition_N(int a, string operator_, int b) {
  if (operator_ == "=") return (a == b);
  else if (operator_ == ">") return (a > b);
  else if (operator_ == "<") return (a < b);
  else if (operator_ == "<=") return (a <= b);
  else if (operator_ == ">=") return (a >= b);
  else {
    return true;
  }
}