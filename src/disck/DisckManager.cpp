#include "./DisckManager.hpp"
#include "./Disck.cpp"

DisckManager::DisckManager(int numeroDePlatos, int numeroDePistas, int numeroDeBloques, int numeroDeSectores, int sizeSector,int blocksReservados) {
  this->disck = make_unique<Disck> (numeroDePlatos, numeroDePistas, numeroDeBloques, numeroDeSectores, sizeSector, blocksReservados);
}


DisckManager::~DisckManager(){};

//todo: saveFIle
void DisckManager::saveFile(string nameFile, char * data, int size) {
  int numBlock = disck->getFreeBlock(size);
  disck->writeToDisck(numBlock, data, size);
  indexingFile(nameFile, numBlock, size);
  disck->updateStateBlock(numBlock);
  cout<<"Indexsacion correcta"<<endl;
}

void DisckManager::updateFile(int numBlock, char * data, int size) {
  disck->writeToDisck(numBlock, data, size);
}


void DisckManager::indexingFile(string nameFile, int numBlock, int dataSize) {
  pair<string, int> reservado = disck->ubicacionDelBloque(0);
  fstream files(reservado.first, ios::binary | ios::in | ios::out);
  if (!files.is_open()) {
    cout<<"NO se pudo abrir el directorio de archivos correctamente"<<endl;
    return;
  }
  int offset = 0;
  HeaderInfoFile temp;
  files.read(reinterpret_cast<char *>(&temp), sizeof(HeaderInfoFile));

  cout<<"offset"<<temp.offset<<" numFIles : "<<temp.numFiles<<endl;

  InfoFile indexTemp;
  strncpy(indexTemp.nameFile, nameFile.c_str(),24);
  indexTemp.nameFile[24] = 0;



  indexTemp.numBlock = numBlock;
  indexTemp.sizeData = dataSize;

  files.seekp(temp.offset);
  files.write(reinterpret_cast<char*>(&indexTemp),sizeof(InfoFile));
  temp.numFiles++;
  temp.offset+=sizeof(InfoFile);

  files.seekp(0);
  files.write(reinterpret_cast<char*>(&temp),sizeof(HeaderInfoFile));
  files.close();
}

void DisckManager::createDirectories() {
  disck->createDirectories();
  pair<string,int> path = disck->ubicacionDelBloque(0);

  fstream directory(path.first, ios::binary | ios::out | ios::in);
  if (!directory.is_open()) {
    cout<<"NOse pudoa brir el directorio"<<endl;
    return;  
  }

  HeaderInfoFile temp;
  temp.numFiles = 0;
  temp.offset = sizeof(HeaderInfoFile);

  directory.write(reinterpret_cast<char*>(&temp),sizeof(HeaderInfoFile));
  directory.close();
}

pair<int, int> DisckManager::findFile(string name) {
  cout<<"###########"<<endl;
  pair<string, int> pathDirectories = disck->ubicacionDelBloque(0);
  fstream files(pathDirectories.first, ios::binary | ios::in | ios::out);

  if (!files.is_open()) {
    cout<<"NO se pudo abrir directories"<<endl;
    return make_pair(-1, 0);
  }

  HeaderInfoFile h_temp;
  files.read(reinterpret_cast<char*>(&h_temp),sizeof(HeaderInfoFile));

  int offset = sizeof(HeaderInfoFile);
  // cout<<"offset : "<<h_temp.offset<<" numFIles : "<<h_temp.numFiles<<endl;
  InfoFile tempFile;

  for(int i = 0; i < h_temp.numFiles; i++) {
    files.read(reinterpret_cast<char*>(&tempFile), sizeof(InfoFile));
    offset+=sizeof(InfoFile);
    if (name == tempFile.nameFile) {
      // cout<<"Encontro el archivo esta guardado ene l bloque"<<tempFile.numBlock<<endl;
      return make_pair(tempFile.numBlock, tempFile.sizeData);
    }
    // cout<<"idFile : "<<tempFile.nameFile<<endl;
    // cout<<"numBlock : "<<tempFile.numBlock<<endl;
    // cout<<"sizeData : "<<tempFile.sizeData<<endl;
  }

  files.close();
  return make_pair(-1, 0);


}

unique_ptr<char[]> DisckManager::getBlockByNumber(int numBlock, int sizeData) {
  pair<string, int> path_and_offset = disck->ubicacionDelBloque(numBlock);
  fstream file(path_and_offset.first, ios::binary | ios::out | ios::in);
  if (!file.is_open()) {
    cout<<"NO se pudo abrir directories"<<endl;
    return nullptr;
  }
  unique_ptr<char[]> data = make_unique<char[]>(sizeData);
  file.seekg(path_and_offset.second, ios::beg);
  file.read(data.get(),sizeData);
  file.close();
  return move(data);

}