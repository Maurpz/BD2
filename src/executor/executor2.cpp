#pragma once
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


//! INSERT INTO

void Executor::insertInto (string nameTable, vector<Col_Data> & rawData) {

  cout<<"******************************************************"<<endl;
  //cout<<"Insert Into iniciando ..."<<endl;
  unique_ptr<PgClassRow> table = refCatalog->getTable(nameTable);
  cout<<"NUmero de paginas de la tabla : "<<table->relpages<<endl;


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
  //*para insertar por defecto inserta en la ultima pagina creada se basa en el numero de paginas de una tabla
  Buffer * page = refBufferManager->loadPage(table->oid, (table->relpages)-1);





  if (page == nullptr) {
    cout<<"la pagina es nula no se puede trabajar sobre ella 1"<<endl;
  }
  else {
    cout<<"La pagina que se traera al buffer es la : "<<table->relpages-1<<endl;

  }

  // //*actualizamos el offset aunque tambien podriamos no hacerlo
  // memcpy(page->getData(), &hp, sizeof(Header));
  pair<int, int> status = PageManager::writeRegisterInBuffer(fields,numColumns, bitmap, page);

  if (status.first == 400) {
    cout<<"Necesita crear una nueva pagina"<<endl;
    refBufferManager->newPage(table->oid,table->relpages);
    page = refBufferManager->loadPage(table->oid,table->relpages);
    if (page == nullptr) {
    cout<<"la pagina es nula no se puede trabajar sobre ella 2"<<endl;
    }
    else {
      cout<<"2 La pagina que se traera al buffer es la : "<<table->relpages-1<<endl;
    }
    refCatalog->increaseNumPage(nameTable);
    status = PageManager::writeRegisterInBuffer(fields,numColumns, bitmap, page);
  }

  cout<<"Entrara a la coprovacion"<<endl;
  if (status.first == 200) { 
    cout<<"L escritura de la tupla fue un exito"<<endl;
    table = refCatalog->getTable(nameTable);
    refBufferManager->flushPage(table->oid, table->relpages-1);

    if (table->relam != 1111){
      cout<<"Tiene un indice asique hay que indexzacrlo"<<endl;
      unique_ptr<PgIndex> index = refCatalog->getIndex(table->relam);
      if (index != nullptr) {
        cout<<"Existe el indice"<<endl;
      }
      else {
        cout<<"NO exists"<<endl;
      }

      
      BPlusTree * tempTree = refBufferManager->getBTree(index->oid);

      CTID tempItem;
      tempItem.numBlock = table->relpages-1;
      tempItem.offsetTuple = status.second;

      for (auto & column : fields) {
        if (column.refColumn->attnum == index->columnIndex) {
          tempTree->set(stoi(column.data), tempItem);
          cout<<"Se inserto correctamente el item en el arbol "<<endl;
          break;
        }
      }
    }

    cout<<"Se escribio corectamente en el disco la pagina : "<<table->relpages-1<<endl;
  }

}


//todo ------------------------------------Select All--------------------------------

void Executor::selectAll(string nameTable) {
  unique_ptr<PgClassRow> table = refCatalog->getTable(nameTable);

  vector<PgAttributeRow> columns = refCatalog->getAllColumns(table->oid);

  //?podriamos ordenar las columnas aunque por defecto ya deberian de estarlo

  for (uint16_t numPage = 0; numPage < table->relpages; numPage++) {
    Buffer * page = refBufferManager->loadPage(table->oid, numPage);
    //*alistamos los recursos que usaremos
    int offset = 0;
    Header hp;
    LinePointer l_ptr;
    TupleHeader th;

    //*recuperamos el headerPage
    memcpy(&hp, page->getData() + offset, sizeof(Header));

    offset += sizeof(Header);
    cout<<"]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]}: "<<hp.pd_lower<<endl;


    for (int i = offset; i < hp.pd_lower; i += sizeof(LinePointer)) {
      cout<<"*******************************+: "<<i<<endl;
      memcpy(&l_ptr, page->getData() + i, sizeof(LinePointer));
      int tempOffset = l_ptr.offset;
      if (l_ptr.flags == LinePointerFlags::LP_DEAD) {
        //cout<<"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$4 el punetoa punta aun registro eliminado"<<endl;
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
    //cout<<"Terminamso de leer los datos"<<endl;
    cout<<endl;

  }

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
  cout<<"=============================================="<<endl;
  unique_ptr<PgClassRow> table = refCatalog->getTable(nameTable);
  vector<PgAttributeRow> columns = refCatalog->getAllColumns(table->oid);

  vector<Field_W_Condition> fields;

  for (PgAttributeRow & col : columns) {
    bool match = false;
    for (Condition& con : conditions) {
      if (col.attname == con.column) {
        //cout<<"\t################## "<<con.operator_<<endl;
        Field_W_Condition temp;
        temp.refColumn = &col;
        temp.operator_ = con.operator_;
        temp.value = con.value;
        //cout<<"\t################## QQQ  "<<temp.operator_<<endl;
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

  // //* definimos nuestros datos  usar
  // Header hp;
  // LinePointer l_ptr;
  // TupleHeader th;
  // int offset = 0;

  // memcpy(&hp, page->getData() +offset, sizeof(Header));
  // offset += sizeof(Header);


  //* verificamos si la condicion por la que se busca esta indexada
  unique_ptr<PgIndex> indexBT = refCatalog->getIndex(table->relam);
  if (indexBT != nullptr) {
    for (auto & column : fields) {
      if (column.refColumn->attnum == indexBT->columnIndex) {
        cout<<"&&&&&&&&&&&&&&&&&&&LLega por index scan"<<endl;

        BPlusTree * tree = refBufferManager->getBTree(indexBT->oid);
        CTID metaDataTuple =  tree->get(stoi(column.value));

        page = refBufferManager->loadPage(table->oid, metaDataTuple.numBlock); 
        int tempOffset = metaDataTuple.offsetTuple;
        TupleHeader th;


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
        return;
      }
    }
  }

  cout<<"&&&&&&&&&&&&&&&&&&&LLega por secuecnial scan"<<endl;

  page = refBufferManager->loadPage(table->oid,0);

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
        //cout<<"\t################## "<<con.operator_<<endl;
        temp.operator_ = con.operator_;
        temp.value = con.value;
        //cout<<"\t################## QQQ  "<<temp.operator_<<endl;
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



//! ---------------------- U P D A T E -----------------------



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

  //cout<<"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@qqupdate"<<endl;
  for (int i = offset; i < hp.pd_lower; i+=sizeof(LinePointer)) {
    //cout<<i<<endl;

    memcpy(&l_ptr,page->getData() + i, sizeof(LinePointer));
    //cout<<"el primer offset del pointer es :"<<l_ptr.offset<<endl;

    //todo: aqui deberia mos de revisar si el linepointer es valido o no
    vector <Field> fieldsTemp;
    fieldsTemp = fields;
    int tempBitmap = bitmap;
    int tempOffset = 0;
    tempOffset = l_ptr.offset;
    memcpy(&th, page->getData() + tempOffset, sizeof(TupleHeader));
    tempOffset+=sizeof(TupleHeader);
    bool allConditionsSuccessfully = true;
    //cout<<"tamaño del vector original  :"<<fields.size()<<endl;
    //cout<<"tamaño del vector copia "<<fieldsTemp.size()<<endl;

    for (int j = 0; j < fieldsTemp.size(); j++) {
      //cout<<"\t verifiacr si esta columna tiene condicional  "<<fieldsConditions[j].operator_<<fieldsConditions[j].value<<endl;
      //*si se encuentra alamcenado en la tupla 
      if (!PageManager::isBitNull(th.t_bits,fieldsTemp[j].refColumn->attnum)) {
        //cout<<"El bit es correcto"<<endl;
        //*ahora revisamos el tipo de dato string o int
        if (fieldsTemp[j].refColumn->attlen < 0) {
          //cout<<"es un string"<<endl;
          int size = 0;
          string data;
          memcpy(&size, page->getData() + tempOffset, sizeof(int));
          data.resize(size);
          //cout<<"El tamaño del string a recuperar"<<size<<endl;
          tempOffset+= sizeof(int);
          memcpy(data.data(), page->getData() + tempOffset, size);
          tempOffset += size;
          if (!checkCondition_S(data,fieldsConditions[j].operator_,fieldsConditions[j].value)) {
            //cout<<"NO cumple la condicion del string"<<endl;
            allConditionsSuccessfully = false;
            break;
          }
          //cout<<"Data :"<<data<<"  "<<endl;
          if (fields[j].isNull) {
            fieldsTemp[j].data = data;
            fieldsTemp[j].isNull = false;
            PageManager::changeBitMap(&tempBitmap,fieldsTemp[j].refColumn->attnum);
          }
        }
        else if (fieldsTemp[j].refColumn->attlen == 4){
          //cout<<"Es un numero"<<endl;
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
          //cout<<"numero :"<<number<<"  "<<endl;


        }
      }
      else {
        //cout<<"El no es corecto"<<endl;
        //*no se encuentra almacenado en la apgina la columna actual
      }
    }
    //*si todas las condiciones fueron verdaderas entonces actualizamos el valor


    for (Field & f : fieldsTemp) {
      //cout<<"contenido : "<<f.data<< "  estado : "<<f.isNull<<"  column : "<<f.refColumn->attname<<endl;
    }
    if (allConditionsSuccessfully) {
      l_ptr.offset = PageManager::writeTuple(fieldsTemp, numColumns, tempBitmap, page);
      //cout<<"el nuevo offset del pointer es :"<<l_ptr.offset<<endl;
      memcpy(page->getData() + i, &l_ptr, sizeof(LinePointer));
      if (isUnique) {
        //cout<<"Es una actualizacion unica solo se hara 1 vez"<<endl;
        break;
      }

    }

  }
  //cout<<"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@qqupdate"<<endl;
} 


//! ------------------ D E L E T E -------------------------------

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
  //cout<<"????????????????????????????????empezo  el delete"<<endl;


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
            //cout<<"NO cumple con la condicion en string"<<endl;
            allConditionsSuccessfully--;
            break;
          }
          //cout<<"DATA: "<<data<<"\t";
        }
        else if (field.refColumn->attlen == 4) {
          int number = 0;
          PageManager::copyFromBuffer(&number, page->getData(), tempOffset, sizeof(int));

          if (!checkCondition_N(number, field.operator_, stoi(field.value))) {
            //cout<<"No cumple conla condicion de int"<<endl;
            allConditionsSuccessfully--;
            break;
          }
          //cout<<"Number"<<number<<"\t";
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
        //cout<<"LA condicion es unica en su fila"<<endl;
        break;
      }
    }

  }
  //cout<<"????????????????????????????????Termino el delete"<<endl;



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