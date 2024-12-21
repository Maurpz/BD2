#include <stdexcept>
#include "bplustree.cpp"
#include "../disck/DisckManager.hpp"
const int PAGE_SIZE_b=4512;

struct Page {
    bool isLeaf;
    vector<int> keys;
    vector<CTID> values;           // Solo en nodos hoja
    vector<int> childrenIds;       // IDs de los nodos hijos (en nodos internos)
    int nextPageId;                // ID de la siguiente hoja (para hojas)
    int prevPageId;                // ID de la hoja anterior (para hojas)
};


int getNodeId(const Node* node) {
    static std::unordered_map<const Node*, int> nodeIds;
    static int nextId = 0;

    if (nodeIds.find(node) == nodeIds.end()) {
        nodeIds[node] = nextId++;
    }
    return nodeIds[node];
}




// Asegúrate de incluir las definiciones de BPlusTree, Node y CTID aquí

// Función para serializar un nodo a un buffer gestionado por unique_ptr
std::unique_ptr<char[]> serializeNodeToBuffer(const Node& node, int treeId, int nodeIndex) {
    auto buffer = std::make_unique<char[]>(PAGE_SIZE_b);
    memset(buffer.get(), 0, PAGE_SIZE_b); // Inicializar con ceros

    int offset = 0;

    // Agregar el ID del árbol y el índice del nodo como metadata (opcional)
    memcpy(buffer.get() + offset, &treeId, sizeof(int));
    offset += sizeof(int);

    memcpy(buffer.get() + offset, &nodeIndex, sizeof(int));
    offset += sizeof(int);

    // Escribir si es hoja
    buffer[offset] = node.isLeaf;
    offset += 1;

    // Escribir cantidad de claves
    int numKeys = node.keys.size();
    memcpy(buffer.get() + offset, &numKeys, sizeof(int));
    offset += sizeof(int);

    // Escribir claves
    memcpy(buffer.get() + offset, node.keys.data(), numKeys * sizeof(int));
    offset += numKeys * sizeof(int);

    if (node.isLeaf) {
        // Escribir valores (CTIDs)
        memcpy(buffer.get() + offset, node.values.data(), numKeys * sizeof(CTID));
        offset += numKeys * sizeof(CTID);

        // Escribir punteros a hojas vecinas (IDs de las hojas prev y next)
        int prevId = node.prev ? node.prev->keys.empty() ? -1 : node.prev->keys[0] : -1; // Ajusta según tu lógica
        int nextId = node.next ? node.next->keys.empty() ? -1 : node.next->keys[0] : -1; // Ajusta según tu lógica
        memcpy(buffer.get() + offset, &prevId, sizeof(int));
        offset += sizeof(int);
        memcpy(buffer.get() + offset, &nextId, sizeof(int));
        offset += sizeof(int);
    } else {
        // Escribir hijos (IDs de los nodos hijos)
        for (const Node* child : node.children) {
            // Aquí asumimos que el índice del hijo ya ha sido asignado previamente
            // y que tienes una forma de obtener el nodeIndex del hijo
            // Para simplificar, podrías pasar un mapa o gestionar los índices fuera
            // de esta función. Aquí usaremos un placeholder.
            int childIndex = -1; // Debes reemplazar esto con el índice real del hijo
            memcpy(buffer.get() + offset, &childIndex, sizeof(int));
            offset += sizeof(int);
        }
    }

    return buffer; // Retornar el buffer gestionado
}

// Función para serializar y guardar todo el árbol B+ en disco
void serializeAndSaveBPlusTree(const BPlusTree& tree, int treeId, DisckManager * diskManager) {
    if (tree.root == nullptr) {
        throw std::invalid_argument("El árbol B+ está vacío.");
    }

    std::queue<Node*> nodeQueue; // Cola para BFS
    nodeQueue.push(tree.root);

    int nodeIndex = 0; // Índice para nombrar los nodos
    std::unordered_map<const Node*, int> nodeIndices; // Mapa para mantener el índice de cada nodo
    nodeIndices[tree.root] = nodeIndex;

    while (!nodeQueue.empty()) {
        Node* currentNode = nodeQueue.front();
        nodeQueue.pop();

        // Obtener el índice del nodo actual
        int currentIndex = nodeIndices[currentNode];

        // Generar el nombre del archivo
        std::string nodeName = (currentIndex == 0) ? std::to_string(treeId) :
                               std::to_string(treeId) + "_" + std::to_string(currentIndex - 1);

        // Serializar el nodo
        std::unique_ptr<char[]> buffer = serializeNodeToBuffer(*currentNode, treeId, currentIndex);

        // Guardar el buffer en disco usando DiskManager
        diskManager->saveFile(nodeName, buffer.get(), PAGE_SIZE_b);

        // Si no es hoja, agregar los hijos a la cola
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
