// // Función para deserializar un nodo desde un buffer
// TempNode deserializeNodeFromBuffer(const char* buffer, int nodeIndex) {
//     TempNode tempNode;
//     tempNode.nodeIndex = nodeIndex;
//     tempNode.isLeaf = buffer[8] ? true : false; // treeId (4 bytes) + nodeIndex (4 bytes) + isLeaf (1 byte)
//     int offset = 9;

//     // Leer cantidad de claves
//     int numKeys;
//     memcpy(&numKeys, buffer + offset, sizeof(int));
//     offset += sizeof(int);
//     tempNode.node->keys.resize(numKeys);
//     memcpy(tempNode.node->keys.data(), buffer + offset, numKeys * sizeof(int));
//     offset += numKeys * sizeof(int);

//     // Crear el nodo
//     tempNode.node = new Node(nullptr, tempNode.isLeaf);
//     tempNode.node->keys.assign(tempNode.node->keys.begin(), tempNode.node->keys.end());

//     if (tempNode.isLeaf) {
//         // Leer valores (CTIDs)
//         tempNode.node->values.resize(numKeys);
//         memcpy(tempNode.node->values.data(), buffer + offset, numKeys * sizeof(CTID));
//         offset += numKeys * sizeof(CTID);

//         // Leer prevId y nextId
//         memcpy(&tempNode.prevId, buffer + offset, sizeof(int));
//         offset += sizeof(int);
//         memcpy(&tempNode.nextId, buffer + offset, sizeof(int));
//         offset += sizeof(int);
//     } else {
//         // Leer childIndices
//         tempNode.childIndices.resize(numKeys + 1);
//         for(int i = 0; i < numKeys + 1; ++i) {
//             memcpy(&tempNode.childIndices[i], buffer + offset, sizeof(int));
//             offset += sizeof(int);
//         }
//     }

//     return tempNode;
// }





/*
#include "bplustree.cpp"
#include "../disck/DisckManager.cpp"



// Constante para el tamaño de página
const int PAGE_SIZE =  1024 ;

// Estructura para almacenar datos temporales durante deserialización
struct TempNode {
    int nodeIndex;
    Node* node;
    bool isLeaf;
    std::vector<int> childIndices; // para nodos internos
    int prevId; // para nodos hoja
    int nextId; // para nodos hoja
};

// Función para generar nombre de nodo
string generateNodeName(const string& treeName, int treeId, int nodeIndex) {
    if (nodeIndex == 0) {
        return treeName + "_" + to_string(treeId);
    } else {
        return treeName + "_" + to_string(treeId) + "_" + to_string(nodeIndex - 1);
    }
}









TempNode deserializeNodeFromBuffer(const char* buffer, int nodeIndex) {
    TempNode tempNode;
    tempNode.nodeIndex = nodeIndex;

    bool isLeaf = buffer[8] ? true : false; // se lee isLeaf correctamente
    tempNode.isLeaf = isLeaf;
    int offset = 9;

    // Leer cantidad de claves
    int numKeys;
    memcpy(&numKeys, buffer + offset, sizeof(int));
    offset += sizeof(int);

    // Crear el nodo antes de usarlo
    tempNode.node = new Node(nullptr, isLeaf);

    // Leer las claves
    tempNode.node->keys.resize(numKeys);
    memcpy(tempNode.node->keys.data(), buffer + offset, numKeys * sizeof(int));
    offset += numKeys * sizeof(int);

    if (tempNode.isLeaf) {
        // Leer valores (CTIDs)
        tempNode.node->values.resize(numKeys);
        memcpy(tempNode.node->values.data(), buffer + offset, numKeys * sizeof(CTID));
        offset += numKeys * sizeof(CTID);

        // Leer prevId y nextId
        memcpy(&tempNode.prevId, buffer + offset, sizeof(int));
        offset += sizeof(int);
        memcpy(&tempNode.nextId, buffer + offset, sizeof(int));
        offset += sizeof(int);
    } else {
        // Leer childIndices
        tempNode.childIndices.resize(numKeys + 1);
        for (int i = 0; i < numKeys + 1; ++i) {
            memcpy(&tempNode.childIndices[i], buffer + offset, sizeof(int));
            offset += sizeof(int);
        }
    }

    return tempNode;
}


// Función para serializar y guardar todo el árbol B+ en disco
void serializeAndSaveBPlusTree(const BPlusTree& tree, int treeId, DisckManager * diskManager) {
    if (tree.root == nullptr) {
        throw invalid_argument("El árbol B+ está vacío.");
    }

    queue<Node*> nodeQueue; // Cola para BFS
    nodeQueue.push(tree.root);

    int nodeIndex = 0; // Índice para nombrar los nodos
    unordered_map<const Node*, int> nodeIndices; // Mapa para mantener el índice de cada nodo
    nodeIndices[tree.root] = nodeIndex;

    while (!nodeQueue.empty()) {
        Node* currentNode = nodeQueue.front();
        nodeQueue.pop();

        // Obtener el índice del nodo actual
        int currentIndex = nodeIndices[currentNode];

        // Generar el nombre del archivo
        string nodeName = generateNodeName("myTree", treeId, currentIndex); // Asumiendo treeName = "myTree"

        // Crear un buffer y serializar el nodo
        auto buffer = make_unique<char[]>(PAGE_SIZE);
        memset(buffer.get(), 0, PAGE_SIZE); // Inicializar con ceros

        int offset = 0;

        // Agregar el ID del árbol y el índice del nodo como metadata (opcional)
        memcpy(buffer.get() + offset, &treeId, sizeof(int));
        offset += sizeof(int);

        memcpy(buffer.get() + offset, &currentIndex, sizeof(int));
        offset += sizeof(int);

        // Escribir si es hoja
        buffer[offset] = currentNode->isLeaf ? 1 : 0;
        offset += 1;

        // Escribir cantidad de claves
        int numKeys = currentNode->keys.size();
        memcpy(buffer.get() + offset, &numKeys, sizeof(int));
        offset += sizeof(int);

        // Escribir claves
        memcpy(buffer.get() + offset, currentNode->keys.data(), numKeys * sizeof(int));
        offset += numKeys * sizeof(int);

        if (currentNode->isLeaf) {
            // Escribir valores (CTIDs)
            memcpy(buffer.get() + offset, currentNode->values.data(), numKeys * sizeof(CTID));
            offset += numKeys * sizeof(CTID);

            // Escribir punteros a hojas vecinas (IDs de las hojas prev y next)
            int prevId = (currentNode->prev && nodeIndices.find(currentNode->prev) != nodeIndices.end()) ? nodeIndices[currentNode->prev] : -1;
            int nextId = (currentNode->next && nodeIndices.find(currentNode->next) != nodeIndices.end()) ? nodeIndices[currentNode->next] : -1;
            memcpy(buffer.get() + offset, &prevId, sizeof(int));
            offset += sizeof(int);
            memcpy(buffer.get() + offset, &nextId, sizeof(int));
            offset += sizeof(int);
        } else {
            // Escribir hijos (IDs de los nodos hijos)
            for (const Node* child : currentNode->children) {
                int childIndex = (child && nodeIndices.find(child) != nodeIndices.end()) ? nodeIndices[child] : -1;
                memcpy(buffer.get() + offset, &childIndex, sizeof(int));
                offset += sizeof(int);
            }
        }

        // Guardar el buffer en disco usando DiskManager
        diskManager->saveFile(nodeName, buffer.get(), PAGE_SIZE);

        // Si no es hoja, agregar sus hijos a la cola
        if (!currentNode->isLeaf) {
            for (Node* child : currentNode->children) {
                if (child != nullptr && nodeIndices.find(child) == nodeIndices.end()) {
                    nodeQueue.push(child);
                    nodeIndex++;
                    nodeIndices[child] = nodeIndex;
                }
            }
        }
    }
}

// Función para deserializar y reconstruir el árbol B+ desde disco
unique_ptr<BPlusTree> deserializeBPlusTree(const string& treeName, int treeId, DisckManager * diskManager) {
    unique_ptr<BPlusTree> tree = make_unique<BPlusTree>(3); // Debe usar el mismo maxCapacity usado en serialización

    unordered_map<int, Node*> nodeMap; // Mapa de nodeIndex a Node*
    unordered_map<int, vector<int>> nodeChildrenMap; // nodeIndex -> childIndices
    unordered_map<int, pair<int, int>> leafNeighbors; // nodeIndex -> (prevId, nextId)

    queue<int> nodeQueue; // Cola para BFS con nodeIndices
    nodeQueue.push(0); // root nodeIndex = 0

    while (!nodeQueue.empty()) {
        int currentIndex = nodeQueue.front();
        nodeQueue.pop();

        // Generar el nombre del archivo
        string nodeName = generateNodeName(treeName, treeId, currentIndex);

        // Encontrar el archivo correspondiente al nodo
        pair<int, int> fileInfo;
        try {
            fileInfo = diskManager->findFile(nodeName);
        } catch(const exception& e) {
            throw runtime_error("Error al encontrar archivo " + nodeName + ": " + e.what());
        }

        int blockNumber = fileInfo.first;
        int sizeData = fileInfo.second;

        // Obtener el buffer del nodo desde el bloque del disco
        unique_ptr<char[]> buffer;
        try {
            buffer = diskManager->getBlockByNumber(blockNumber, sizeData);
        } catch(const exception& e) {
            throw runtime_error("Error al obtener bloque para " + nodeName + ": " + e.what());
        }

        // Deserializar el nodo
        int offset = 0;

        // Leer el ID del árbol y el índice del nodo (opcional, ignorados en reconstrucción)
        int serializedTreeId;
        memcpy(&serializedTreeId, buffer.get() + offset, sizeof(int));
        offset += sizeof(int);

        int serializedNodeIndex;
        memcpy(&serializedNodeIndex, buffer.get() + offset, sizeof(int));
        offset += sizeof(int);

        // Leer si es hoja
        bool isLeaf = buffer[offset] ? true : false;
        offset += 1;

        // Leer cantidad de claves
        int numKeys;
        memcpy(&numKeys, buffer.get() + offset, sizeof(int));
        offset += sizeof(int);

        // Leer claves
        vector<int> keys(numKeys);
        memcpy(keys.data(), buffer.get() + offset, numKeys * sizeof(int));
        offset += numKeys * sizeof(int);

        // Crear el nodo
        Node* node = new Node(nullptr, isLeaf);
        node->keys = keys;

        if (isLeaf) {
            // Leer valores (CTIDs)
            node->values.resize(numKeys);
            memcpy(node->values.data(), buffer.get() + offset, numKeys * sizeof(CTID));
            offset += numKeys * sizeof(CTID);

            // Leer prevId y nextId
            int prevId, nextId;
            memcpy(&prevId, buffer.get() + offset, sizeof(int));
            offset += sizeof(int);
            memcpy(&nextId, buffer.get() + offset, sizeof(int));
            offset += sizeof(int);

            // Guardar en nodeMap
            nodeMap[currentIndex] = node;

            // Guardar vecinos para asignar después
            leafNeighbors[currentIndex] = {prevId, nextId};
        } else {
            // Leer childIndices
            vector<int> childIndices(numKeys + 1);
            for(int i = 0; i < numKeys + 1; ++i) {
                memcpy(&childIndices[i], buffer.get() + offset, sizeof(int));
                offset += sizeof(int);
            }
            nodeChildrenMap[currentIndex] = childIndices;

            // Guardar en nodeMap
            nodeMap[currentIndex] = node;

            // Agregar hijos a la cola
            for(int i = 0; i < childIndices.size(); ++i) {
                int childIndex = childIndices[i];
                if(childIndex != -1 && nodeMap.find(childIndex) == nodeMap.end()) {
                    nodeQueue.push(childIndex);
                }
            }
        }

        // Asignar la raíz si es la raíz
        if(currentIndex == 0) {
            tree->root = node;
        }
    }

    // Asignar hijos para nodos internos
    for(auto& [nodeIndex, childIndices] : nodeChildrenMap) {
        Node* parent = nodeMap[nodeIndex];
        for(int i = 0; i < childIndices.size(); ++i) {
            int childIndex = childIndices[i];
            if(childIndex != -1) {
                parent->children[i] = nodeMap[childIndex];
                nodeMap[childIndex]->parent = parent;
            }
        }
    }

    // Asignar prev y next para nodos hoja
    for(auto& [nodeIndex, neighbors] : leafNeighbors) {
        int prevId = neighbors.first;
        int nextId = neighbors.second;
        Node* node = nodeMap[nodeIndex];
        if(prevId != -1 && nodeMap.find(prevId) != nodeMap.end()) {
            node->prev = nodeMap[prevId];
        }
        if(nextId != -1 && nodeMap.find(nextId) != nodeMap.end()) {
            node->next = nodeMap[nextId];
        }
    }

    return move(tree);
};













// Función principal para demostrar el uso
int main() {
    // Crear una instancia de DiskManager
    DisckManager diskManager(2,4,4,4,512,2);

    // Crear un árbol B+ y llenarlo con datos
    BPlusTree tree(3); // Capacidad máxima de 3 claves por nodo
    int treeId = 199838; // Identificador único para este árbol

    // Insertar datos en el árbol
    CTID a1 = {5, 2222};
    CTID a2 = {7, 2222};
    CTID a3 = {9, 3333};
    CTID a4 = {11, 4444};

    tree.set(44, a1);
    tree.set(4, a2);
    tree.set(22, a3);
    tree.set(15, a4);
    tree.print();
    cout<<"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"<<endl;


    // Serializar y guardar el árbol completo
    try {
        serializeAndSaveBPlusTree(tree, treeId, &diskManager);
        cout << "Árbol B+ serializado y guardado exitosamente.\n";
    } catch (const exception& e) {
        cerr << "Error al serializar el árbol: " << e.what() << "\n";
    }

    cout<<"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"<<endl;

    // Deserializar y reconstruir el árbol desde disco
    try {
        unique_ptr<BPlusTree> reconstructedTree = deserializeBPlusTree("myTree", treeId, &diskManager);
        cout << "Árbol B+ reconstruido exitosamente.\n";

        // (Opcional) Imprimir el árbol para verificar
        reconstructedTree->print();
    } catch (const exception& e) {
        cerr << "Error al reconstruir el árbol: " << e.what() << "\n";
    }

    return 0;
}


*/

#include "bplustree.cpp"
#include "../disck/DisckManager.cpp"
using namespace std;

// Constante para el tamaño de página
const int PAGE_SIZE =  1024 ;

// Variable global para la capacidad del B+ Tree
const int GLOBAL_MAX_CAPACITY = 3; // *** Nueva variable global

// Estructura para almacenar datos temporales durante deserialización
struct TempNode {
    int nodeIndex;
    Node* node;
    bool isLeaf;
    vector<int> childIndices; // para nodos internos
    int prevId; // para nodos hoja
    int nextId; // para nodos hoja
};

// Función para generar nombre de nodo
string generateNodeName(const string& treeName, int treeId, int nodeIndex) {
    if (nodeIndex == 0) {
        return treeName + "_" + to_string(treeId);
    } else {
        return treeName + "_" + to_string(treeId) + "_" + to_string(nodeIndex - 1);
    }
}

// Función para deserializar un nodo desde un buffer
TempNode deserializeNodeFromBuffer(const char* buffer, int nodeIndex) {
    TempNode tempNode;
    tempNode.nodeIndex = nodeIndex;

    int offset = 0;
    int serializedTreeId;
    memcpy(&serializedTreeId, buffer + offset, sizeof(int));
    offset += sizeof(int);

    int serializedNodeIndex;
    memcpy(&serializedNodeIndex, buffer + offset, sizeof(int));
    offset += sizeof(int);

    bool isLeaf = buffer[offset] ? true : false;
    offset += 1;

    tempNode.isLeaf = isLeaf;

    // Leer cantidad de claves
    int numKeys;
    memcpy(&numKeys, buffer + offset, sizeof(int));
    offset += sizeof(int);

    // Crear el nodo antes de usarlo
    tempNode.node = new Node(nullptr, isLeaf);

    // Leer las claves
    tempNode.node->keys.resize(numKeys);
    memcpy(tempNode.node->keys.data(), buffer + offset, numKeys * sizeof(int));
    offset += numKeys * sizeof(int);

    if (tempNode.isLeaf) {
        // Leer valores (CTIDs)
        tempNode.node->values.resize(numKeys);
        memcpy(tempNode.node->values.data(), buffer + offset, numKeys * sizeof(CTID));
        offset += numKeys * sizeof(CTID);

        // Leer prevId y nextId
        memcpy(&tempNode.prevId, buffer + offset, sizeof(int));
        offset += sizeof(int);
        memcpy(&tempNode.nextId, buffer + offset, sizeof(int));
        offset += sizeof(int);
    } else {
        // Leer childIndices
        tempNode.childIndices.resize(numKeys + 1);
        for (int i = 0; i < numKeys + 1; ++i) {
            memcpy(&tempNode.childIndices[i], buffer + offset, sizeof(int));
            offset += sizeof(int);
        }
    }

    return tempNode;
}


// Función para serializar y guardar todo el árbol B+ en disco
void serializeAndSaveBPlusTree(const BPlusTree& tree, int treeId, DisckManager * diskManager) {
    if (tree.root == nullptr) {
        throw invalid_argument("El árbol B+ está vacío.");
    }

    queue<Node*> nodeQueue; // Cola para BFS
    nodeQueue.push(tree.root);

    int nodeIndex = 0; // Índice para nombrar los nodos
    unordered_map<const Node*, int> nodeIndices; // Mapa para mantener el índice de cada nodo
    nodeIndices[tree.root] = nodeIndex;

    while (!nodeQueue.empty()) {
        Node* currentNode = nodeQueue.front();
        nodeQueue.pop();

        // Obtener el índice del nodo actual
        int currentIndex = nodeIndices[currentNode];

        // Generar el nombre del archivo
        string nodeName = generateNodeName("myTree", treeId, currentIndex); // Asumiendo treeName = "myTree"

        // Crear un buffer y serializar el nodo
        auto buffer = make_unique<char[]>(PAGE_SIZE);
        memset(buffer.get(), 0, PAGE_SIZE); // Inicializar con ceros

        int offset = 0;

        // Agregar el ID del árbol y el índice del nodo como metadata (opcional)
        memcpy(buffer.get() + offset, &treeId, sizeof(int));
        offset += sizeof(int);

        memcpy(buffer.get() + offset, &currentIndex, sizeof(int));
        offset += sizeof(int);

        // Escribir si es hoja
        buffer[offset] = currentNode->isLeaf ? 1 : 0;
        offset += 1;

        // Escribir cantidad de claves
        int numKeys = (int)currentNode->keys.size();
        memcpy(buffer.get() + offset, &numKeys, sizeof(int));
        offset += sizeof(int);

        // Escribir claves
        memcpy(buffer.get() + offset, currentNode->keys.data(), numKeys * sizeof(int));
        offset += numKeys * sizeof(int);

        if (currentNode->isLeaf) {
            // Escribir valores (CTIDs)
            memcpy(buffer.get() + offset, currentNode->values.data(), numKeys * sizeof(CTID));
            offset += numKeys * sizeof(CTID);

            // Escribir punteros a hojas vecinas (IDs de las hojas prev y next)
            int prevId = (currentNode->prev && nodeIndices.find(currentNode->prev) != nodeIndices.end()) ? nodeIndices[currentNode->prev] : -1;
            int nextId = (currentNode->next && nodeIndices.find(currentNode->next) != nodeIndices.end()) ? nodeIndices[currentNode->next] : -1;
            memcpy(buffer.get() + offset, &prevId, sizeof(int));
            offset += sizeof(int);
            memcpy(buffer.get() + offset, &nextId, sizeof(int));
            offset += sizeof(int);
        } else {
            // Escribir hijos (IDs de los nodos hijos)
            for (const Node* child : currentNode->children) {
                int childIndex = (child && nodeIndices.find(child) != nodeIndices.end()) ? nodeIndices[child] : -1;
                memcpy(buffer.get() + offset, &childIndex, sizeof(int));
                offset += sizeof(int);
            }
        }

        // Guardar el buffer en disco usando DiskManager
        diskManager->saveFile(nodeName, buffer.get(), PAGE_SIZE);

        // Si no es hoja, agregar sus hijos a la cola
        if (!currentNode->isLeaf) {
            for (Node* child : currentNode->children) {
                if (child != nullptr && nodeIndices.find(child) == nodeIndices.end()) {
                    nodeQueue.push(child);
                    nodeIndex++;
                    nodeIndices[child] = nodeIndex;
                }
            }
        }
    }
}

// Función para deserializar y reconstruir el árbol B+ desde disco
unique_ptr<BPlusTree> deserializeBPlusTree(const string& treeName, int treeId, DisckManager * diskManager) {
    // Usar la variable global para la capacidad
    unique_ptr<BPlusTree> tree = make_unique<BPlusTree>(GLOBAL_MAX_CAPACITY); 

    unordered_map<int, Node*> nodeMap; // Mapa de nodeIndex a Node*
    unordered_map<int, vector<int>> nodeChildrenMap; // nodeIndex -> childIndices
    unordered_map<int, pair<int, int>> leafNeighbors; // nodeIndex -> (prevId, nextId)

    queue<int> nodeQueue; // Cola para BFS con nodeIndices
    nodeQueue.push(0); // root nodeIndex = 0

    while (!nodeQueue.empty()) {
        int currentIndex = nodeQueue.front();
        nodeQueue.pop();

        // Generar el nombre del archivo
        string nodeName = generateNodeName(treeName, treeId, currentIndex);

        // Encontrar el archivo correspondiente al nodo
        pair<int, int> fileInfo;
        try {
            fileInfo = diskManager->findFile(nodeName);
        } catch(const exception& e) {
            throw runtime_error("Error al encontrar archivo " + nodeName + ": " + string(e.what()));
        }

        int blockNumber = fileInfo.first;
        int sizeData = fileInfo.second;

        // Obtener el buffer del nodo desde el bloque del disco
        unique_ptr<char[]> buffer;
        try {
            buffer = diskManager->getBlockByNumber(blockNumber, sizeData);
        } catch(const exception& e) {
            throw runtime_error("Error al obtener bloque para " + nodeName + ": " + string(e.what()));
        }

        // *** Aquí usamos la función deserializeNodeFromBuffer en lugar de deserializar inline
        TempNode tempNode = deserializeNodeFromBuffer(buffer.get(), currentIndex);

        Node* node = tempNode.node;

        if (tempNode.isLeaf) {
            // Es hoja
            nodeMap[currentIndex] = node;
            leafNeighbors[currentIndex] = {tempNode.prevId, tempNode.nextId};
        } else {
            // Es interno
            nodeMap[currentIndex] = node;
            nodeChildrenMap[currentIndex] = tempNode.childIndices;

            // Agregar hijos a la cola
            for (int childIndex : tempNode.childIndices) {
                if (childIndex != -1 && nodeMap.find(childIndex) == nodeMap.end()) {
                    nodeQueue.push(childIndex);
                }
            }
        }

        // Asignar la raíz si es la raíz
        if (currentIndex == 0) {
            tree->root = node;
        }
    }

    // Asignar hijos para nodos internos
    for (auto& [nodeIndex, childIndices] : nodeChildrenMap) {
        Node* parent = nodeMap[nodeIndex];
        parent->children.resize(childIndices.size());
        for (size_t i = 0; i < childIndices.size(); ++i) {
            int childIndex = childIndices[i];
            if (childIndex != -1) {
                parent->children[i] = nodeMap[childIndex];
                nodeMap[childIndex]->parent = parent;
            }
        }
    }

    // Asignar prev y next para nodos hoja
    for (auto& [nodeIndex, neighbors] : leafNeighbors) {
        int prevId = neighbors.first;
        int nextId = neighbors.second;
        Node* node = nodeMap[nodeIndex];
        if (prevId != -1 && nodeMap.find(prevId) != nodeMap.end()) {
            node->prev = nodeMap[prevId];
        }
        if (nextId != -1 && nodeMap.find(nextId) != nodeMap.end()) {
            node->next = nodeMap[nextId];
        }
    }

    return move(tree);
}

// Función principal para demostrar el uso
int main() {
    // Crear una instancia de DiskManager
    DisckManager diskManager(2,4,4,4,512,2);

    // Crear un árbol B+ y llenarlo con datos
    BPlusTree tree(GLOBAL_MAX_CAPACITY); // Usar la variable global
    int treeId = 238119; // Identificador único para este árbol

    // Insertar datos en el árbol
    CTID a1 = {5, 2222};
    CTID a2 = {7, 2222};
    CTID a3 = {9, 3333};
    CTID a4 = {11, 4444};

    tree.set(44, a1);
    tree.set(4, a2);
    tree.set(22, a3);
    tree.set(15, a4);
    tree.print();

    
    cout<<"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"<<endl;


    // Serializar y guardar el árbol completo
    try {
        serializeAndSaveBPlusTree(tree, treeId, &diskManager);
        cout << "Árbol B+ serializado y guardado exitosamente.\n";
    } catch (const exception& e) {
        cerr << "Error al serializar el árbol: " << e.what() << "\n";
    }

    cout<<"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"<<endl;

    // Deserializar y reconstruir el árbol desde disco
    try {
        unique_ptr<BPlusTree> reconstructedTree = deserializeBPlusTree("myTree", treeId, &diskManager);
        cout << "Árbol B+ reconstruido exitosamente.\n";

        // (Opcional) Imprimir el árbol para verificar
        reconstructedTree->print();
    } catch (const exception& e) {
        cerr << "Error al reconstruir el árbol: " << e.what() << "\n";
    }

    return 0;
}
