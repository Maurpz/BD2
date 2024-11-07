#ifndef BUFFER_H
#define BUFFER_H


const int PAGE_SIZE = 4096;
class Buffer {
  private:
    char * data;
    int fileNodeOID;
    int pageNum;
    bool dirty;
    int pinCount;
  public:
    Buffer(int fileNodeOID, int pageNum);
    ~Buffer();

    char * getData();
    int getFileNodeOID();
    void setIsDirty(bool);
    bool isDirty();
    void setData(char[]);//!posiblemente eliminar
};
#endif