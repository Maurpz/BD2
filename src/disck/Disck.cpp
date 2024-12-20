#include "./Disck.hpp"

struct FreeSpaceMap {
  uint16_t tam_sector;
};

struct EntryFreeSpaceMap {
  uint8_t part_free_block;
  
  EntryFreeSpaceMap(){
    part_free_block = 1;
  }
};

//todo: Disck 2

Disck::Disck(int numeroDePlatos, int numeroDePistas, int numeroDeBloques, int numeroDeSectores, int sizeSector,int  blocksReservados) {
  this->numeroDePlatos = numeroDePlatos;
  this->pistasPorSuperficie = numeroDePistas;
  this->bloquesPorPista = numeroDeBloques;
  this->sectoresPorBloque = numeroDeSectores;
  this->bytesPorSector = sizeSector;

  this->bloquesPorSuperficie = bloquesPorPista * pistasPorSuperficie;
  this->bloquesPorPlato = bloquesPorSuperficie * 2;
  this-> numeroTotalDeBloques = bloquesPorPlato * numeroDePlatos;
  this->bytesPorBloque = sectoresPorBloque * bytesPorSector;
  root = "data";

  this->blocksReservados = blocksReservados;
}


Disck::~Disck() {}

void Disck::createDirectories(){
  string origin = root+"/disck2";
  int totalSize = bloquesPorSuperficie * sectoresPorBloque * bytesPorSector ;

  char surface[totalSize];

  memset(surface, 0, totalSize);
  fs::create_directory(origin);

  for (int i = 0; i < numeroDePlatos; i++) {
    cout<<"plato: "<<i<<endl;
    string nameDirectory = origin+"/platter_";
    nameDirectory+=to_string(i+1);
    fs::create_directory(nameDirectory);
     
    for (int j = 1; j < 3; j++) {
      string nameFile = "/surface_";
      nameFile+=to_string(j);
      //createFile(nameDirectory,nameFile);
      ofstream file(nameDirectory+nameFile+".bin", ios::binary | ios::out);
      if (file.is_open()){
      file.write(surface, totalSize);
      file.close();
      cout<<"Se escribio correctamente"<<endl;
      }
      else {
        cerr<<"No se pudo escribir correctamente"<<endl;
      } 
    }
  }
  //*creando las superficies
  string nameFree = "/freeSpaceMap.bin";

  EntryFreeSpaceMap entrys[numeroTotalDeBloques];

  ofstream file(origin+nameFree, ios::binary | ios::out);
  if (file.is_open()){
    file.write(reinterpret_cast<char*>(&entrys), numeroTotalDeBloques*sizeof(EntryFreeSpaceMap));
    file.close();
    cout<<"Se escribio correctamente el bitmap"<<endl;
    }
  else {
    cerr<<"No se pudo escribir correctamente"<<endl;
  }
    

}

int Disck::getFreeBlock(int size){
  string dirname = "data/disck2/freeSpaceMap.bin";
  int numBlocks = size/bytesPorBloque;

  ifstream file(dirname, ios::binary | ios::in);
  if (!file.is_open()){
    cout<<"NO se pudo abrir el archivo de freSpace"<<endl;
    return -1;
  }


  EntryFreeSpaceMap temp;
  int i = 0;
  while (file.read(reinterpret_cast<char*>(&temp), sizeof(EntryFreeSpaceMap))) {
    cout<<i<<"\t"<<endl;
    if (i>=blocksReservados) {

      //* si el bloque esta libre entonces se retorna su numero
      if (int(temp.part_free_block) == 1) {
        file.close();
        cout<<"Bloque disponible : "<<i<<endl;
        return i;
      }
    }
    i++;
  }
  file.close();
  return -1;
}

pair<string,int> Disck::ubicacionDelBloque(int index) {
  //*hallamos la ruta del bloque
  string pathBlock = root+"/disck2";
  //fstream(root+"disck2/freeSpaceMap.bin", ios::binary | ios::in);
  int plato = (index/bloquesPorPlato)%numeroDePlatos;
  //cout<<"Nro plato : "<<plato<<endl;
  pathBlock+="/platter_"+to_string(plato+1);

  int superficie = (index/bloquesPorSuperficie)%2;

  //cout<<"Nro superficie: "<<superficie<<endl;
  pathBlock+="/surface_"+to_string(superficie+1)+".bin";

  //cout<<"Nro pista : "<<(index/bloquesPorPista)%pistasPorSuperficie<<endl;

  //* calculamos su offset
  int bytesPorSuperficie = bytesPorBloque * bloquesPorPista * pistasPorSuperficie;
  int offsetBlock = (index * bytesPorBloque)%bytesPorSuperficie;
  //cout<<"Su ubicacion "<<offsetBlock<<endl; 
  return make_pair(pathBlock, offsetBlock);


}

void Disck::writeToDisck(int numBloque, char * data, int size) {
  pair<string, int> pathAndOffset = ubicacionDelBloque(numBloque);
  cout<<"Escribiendo en "<<pathAndOffset.first<<" offset "<<pathAndOffset.second<<endl;
  fstream surface(pathAndOffset.first, ios::binary | ios::out | ios::in);
  if (!surface.is_open()) {
    cout<<"No se pudo encontrar la ruta : "<<pathAndOffset.first<<endl;
    return;
  }
  surface.seekp(pathAndOffset.second, ios::beg);
  surface.write(data,size);
  surface.close();
  cout<<"Se escribio con exito en la superficie"<<endl;
}

void Disck::updateStateBlock(int numBlock) {
  string dirname = "data/disck2/freeSpaceMap.bin";
  fstream freeSpace(dirname, ios::binary | ios::in | ios:: out);
  if(!freeSpace.is_open()) {
    cout<<"No se pude abrir el archivo"<<endl;
    return;
  }
  EntryFreeSpaceMap entryTemp;
  freeSpace.seekg(numBlock * sizeof(EntryFreeSpaceMap),ios::beg);
  freeSpace.read(reinterpret_cast<char*>(&entryTemp), sizeof(EntryFreeSpaceMap));
  cout<<"Disponibilidad del bloque a actualizar"<<(int)entryTemp.part_free_block<<endl;
  entryTemp.part_free_block = 0;
  freeSpace.seekp((numBlock * sizeof(EntryFreeSpaceMap)), ios::beg);
  freeSpace.write(reinterpret_cast<char*>(&entryTemp),sizeof(EntryFreeSpaceMap));
  cout<<"actualizacion correcta"<<endl;

}
