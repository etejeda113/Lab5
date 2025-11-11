#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "lab5.h"

extern Node *g_root;

#define MAGIC 0x41544C35  /* "ATL5" */
#define VERSION 1

typedef struct {
    Node *node;
    int id;
} NodeMapping;

// find ID
static int find_id(NodeMapping *map, int count, Node *ptr) {
    for (int i = 0; i < count; ++i) {
        if (map[i].node == ptr) return map[i].id;
    }
    return -1;
}
/* TODO 27: Implement save_tree
 * Save the tree to a binary file using BFS traversal
 * 
 * Binary format:
 * - Header: magic (4 bytes), version (4 bytes), nodeCount (4 bytes)
 * - For each node in BFS order:
 *   - isQuestion (1 byte)
 *   - textLen (4 bytes)
 *   - text (textLen bytes, no null terminator)
 *   - yesId (4 bytes, -1 if NULL)
 *   - noId (4 bytes, -1 if NULL)
 * 
 * Steps:
 * 1. Return 0 if g_root is NULL
 * 2. Open file for writing binary ("wb")
 * 3. Initialize queue and NodeMapping array
 * 4. Use BFS to assign IDs to all nodes:
 *    - Enqueue root with id=0
 *    - Store mapping[0] = {g_root, 0}
 *    - While queue not empty:
 *      - Dequeue node and id
 *      - If node has yes child: add to mappings, enqueue with new id
 *      - If node has no child: add to mappings, enqueue with new id
 * 5. Write header (magic, version, nodeCount)
 * 6. For each node in mapping order:
 *    - Write isQuestion, textLen, text bytes
 *    - Find yes child's id in mappings (or -1)
 *    - Find no child's id in mappings (or -1)
 *    - Write yesId, noId
 * 7. Clean up and return 1 on success
 */
int save_tree(const char *filename) {
    // TODO: Implement this function
    // This is complex - break it into smaller steps
    // You'll need to use the Queue functions you implemented
   
    //  * 1. Return 0 if g_root is NULL
    if(g_root == NULL) return 0;

    //  * 2. Open file for writing binary ("wb")
    FILE *fptr = fopen(filename, "wb");
    if(fptr == NULL) return 0;

    //  * 3. Initialize queue and NodeMapping array
    Queue q;
    q_init(&q);

    //  * 4. Use BFS to assign IDs to all nodes:

    // number of nodes in tree
    int treeCap =16;
    int treeCount =0;

    // allocate mem for mapping array
    NodeMapping *map = malloc(sizeof(NodeMapping)*treeCap);
    if(map == NULL){
        fclose(fptr);
        return 0;
    }

    if(treeCount>= treeCap){
        treeCap *= 2;
        NodeMapping *nbuffer = realloc(map,sizeof(NodeMapping)*treeCap);
        if(nbuffer==NULL){
            free(map);
            fclose(fptr);
            return 0;
        }
        map = nbuffer;
    }
    map[treeCount].node = g_root;
    map[treeCount].id = 0;
    treeCount++;

    q_enqueue(&q, g_root, 0);

    // assign ID to node
    while(!q_empty(&q)){
        Node *cur = NULL;
        int id = -1;
        q_dequeue(&q, &cur, &id);

    // If node has yes child: add to mappings, enqueue with new id

        if(cur->yes){
            int yesID = find_id(map, treeCount, cur->yes);
            if(yesID <0){
                yesID = treeCount;
                if(treeCount>=treeCap){
                    treeCap *=2;
                    NodeMapping *nbuf = realloc(map, sizeof(NodeMapping)*treeCap);
                        if(!nbuf){
                            free(map);
                            q_free(&q);
                            fclose(fptr);
                            return 0;
                        }
                        map = nbuf;
                    }
                    
                map[treeCount].node = cur->yes;
                map[treeCount].id =yesID;
                treeCount++;
                q_enqueue(&q, cur->yes, yesID);
            }
        }

    // If node has no child: add to mappings, enqueue with new id

        if(cur->no){
            int noID = find_id(map, treeCount, cur->no);
            if(noID < 0){
                // assign new ID
                noID = treeCount;
                if(treeCount>= treeCap){
                    treeCap *=2;
                    NodeMapping *nbuf = realloc(map, sizeof(NodeMapping)*treeCap);
                    if(!nbuf){
                            free(map);
                            q_free(&q);
                            fclose(fptr);
                            return 0;
                    }
                    map = nbuf;
                }
            map[treeCount].node = cur->no;
            map[treeCount].id = noID;
            treeCount++;
            q_enqueue(&q, cur->no, noID);

        }
    }
}
    q_free(&q);
    // * 5. Write header (magic, version, treeCount)

    uint32_t magic = MAGIC;
    uint32_t version = VERSION;
    uint32_t count = (uint32_t)treeCount;
    
    if (fwrite(&magic, sizeof(uint32_t), 1, fptr) != 1 ||
        fwrite(&version, sizeof(uint32_t), 1, fptr) != 1 ||
        fwrite(&count, sizeof(uint32_t), 1, fptr) != 1) {
            free(map);
            fclose(fptr);
            return 0;
        }


    //  *    - Write isQuestion, textLen, text bytes

    for(int i=0; i<treeCount; i++){
        Node *node = map[i].node; 

        uint8_t isQuestion= (uint8_t)(node->isQuestion ? 1 : 0);
        int32_t textLength = (int32_t)strlen(node->text);


    //Find yes child's id in mappings (or -1)

        int32_t yesChildID;
        int32_t noChildID;

    
        if(node->yes != NULL){
            yesChildID = (int32_t)find_id(map, treeCount, node->yes);
        }else{
            yesChildID = -1;
        }

        if(node->no != NULL){
            noChildID = (int32_t)find_id(map, treeCount, node->no);
        }else{
            noChildID = -1;
        }
        

    if(fwrite(&isQuestion, sizeof(int8_t), 1, fptr) != 1){
        free(map);
        fclose(fptr);
        return 0;
    }
    if(fwrite(&textLength, sizeof(int32_t), 1, fptr) != 1){
        free(map);
        fclose(fptr);
        return 0;
    }
    if(textLength > 0 && fwrite(node->text, 1, (size_t)textLength, fptr) != (size_t)textLength){
        free(map);
        fclose(fptr);
        return 0;
    }
    if(fwrite(&yesChildID, sizeof(int32_t), 1, fptr) != 1){
        free(map);
        fclose(fptr);
        return 0;
    }
    if(fwrite(&noChildID, sizeof(int32_t), 1, fptr) != 1){
        free(map);
        fclose(fptr);
        return 0;
    }
}

    free(map);
    fclose(fptr);
    return 1;
}

/* TODO 28: Implement load_tree
 * Load a tree from a binary file and reconstruct the structure
 * 
 * Steps:
 * 1. Open file for reading binary ("rb")
 * 2. Read and validate header (magic, version, count)
 * 3. Allocate arrays for nodes and child IDs:
 *    - Node **nodes = calloc(count, sizeof(Node*))
 *    - int32_t *yesIds = calloc(count, sizeof(int32_t))
 *    - int32_t *noIds = calloc(count, sizeof(int32_t))
 * 4. Read each node:
 *    - Read isQuestion, textLen
 *    - Validate textLen (e.g., < 10000)
 *    - Allocate and read text string (add null terminator!)
 *    - Read yesId, noId
 *    - Validate IDs are in range [-1, count)
 *    - Create Node and store in nodes[i]
 * 5. Link nodes using stored IDs:
 *    - For each node i:
 *      - If yesIds[i] >= 0: nodes[i]->yes = nodes[yesIds[i]]
 *      - If noIds[i] >= 0: nodes[i]->no = nodes[noIds[i]]
 * 6. Free old g_root if not NULL
 * 7. Set g_root = nodes[0]
 * 8. Clean up temporary arrays
 * 9. Return 1 on success
 * 
 * Error handling:
 * - If any read fails or validation fails, goto load_error
 * - In load_error: free all allocated memory and return 0
 */
int load_tree(const char *filename) {
    // TODO: Implement this function
    // This is the most complex function in the lab
    // Take it step by step and test incrementally

    // * 1. Open file for reading binary ("rb")

    
    FILE *fptr = fopen(filename, "rb");
    if(fptr == NULL) return 0;
    
    //  * 2. Read and validate header (magic, version, count)

    uint32_t magic;
    uint32_t version;
    uint32_t count;

    if(fread(&magic, sizeof(uint32_t), 1, fptr) != 1){
        fclose(fptr);
        return 0;
    }

    if(fread(&version, sizeof(uint32_t), 1, fptr) != 1){
        fclose(fptr);  
        return 0;  
    }

    if(fread(&count, sizeof(uint32_t), 1, fptr) != 1){
        fclose(fptr);
        return 0;

    }

    printf("DEBUG: magic=0x%X version=%u count=%u\n", magic, version, count);

    // validating
    
    if(magic != MAGIC || version != VERSION || count == 0 || count > 10000){
        fclose(fptr);
        return 0;
    }

    // * 3. Allocate arrays for nodes and child IDs
    Node **nodes = calloc(count, sizeof(Node*));
    int32_t *yesIds = calloc(count, sizeof(int32_t)*count);
    int32_t *noIds = calloc(count, sizeof(int32_t)*count);
    if (!nodes || !yesIds || !noIds) {
        free (nodes);
        free(yesIds);
        free (noIds);
        fclose(fptr);
        return 0;
    }

    
    //* 4. Read each node:

    //*    - Read isQuestion, textLen
    
    for (uint32_t i=0; i<count; i++){
        uint8_t isQuestion;
        uint32_t textLen;

        if(fread(&isQuestion, sizeof(uint8_t), 1, fptr) != 1){
            goto load_err;
        }
        if(fread(&textLen, sizeof(uint32_t), 1, fptr) != 1){
            goto load_err;
        }
        
    //*    - Validate textLen (e.g., < 10000)

    if (textLen > 10000){
        goto load_err;
    }

    //*    - Allocate and read text string (add null terminator!)

    char *text = malloc((size_t)+1);
    if(text == NULL){
        goto load_err;
    }

    if(textLen > 0){
        if(fread(text, 1, (size_t)textLen, fptr) != (size_t)textLen){
            free(text);
            goto load_err;
        }   
    }
    text[textLen] = '\0';

    //*    - Read yesId, noId

    int32_t yID = -1, noID =-1;
    if(fread(&yID, sizeof(int32_t), 1, fptr) != 1){
        free(text);
        goto load_err;
    }

    if(fread(&noID, sizeof(int32_t), 1, fptr) != 1){
        free(text);
        goto load_err;
    }

    //*    - Validate IDs are in range [-1, count)
    if(yID < -1 || yID >= (int32_t)count){        
        free(text);
        goto load_err;
    }
     if(noID < -1 || noID >= (int32_t)count){
        free(text);
        goto load_err;
    }

    //*    - Create Node and store in nodes[i]

    Node *node = malloc(sizeof(Node));

    if(node == NULL){
        free(text);
        goto load_err;
    }

    node->isQuestion = (isQuestion ? 1 : 0);
    node->text = text;
    node->yes = NULL;
    node->no = NULL;
    
    nodes[i] = node;
    yesIds[i] = yID;
    noIds[i] = noID;
}
//* 5. Link nodes using stored IDs:

//*    - For each node i:
// *      - If yesIds[i] >= 0: nodes[i]->yes = nodes[yesIds[i]]
// *      - If noIds[i] >= 0: nodes[i]->no = nodes[noIds[i]]

for(uint32_t i=0; i<count; i++){
    if(yesIds[i] >= 0){
        nodes[i]->yes = nodes[yesIds[i]];
    }
    if(noIds[i] >= 0){
        nodes[i]->no = nodes[noIds[i]];
    }

}

// * 6. Free old g_root if not NULL

// * 7. Set g_root = nodes[0]

if (g_root){
    free_tree(g_root);
}

g_root = nodes[0];

// * 8. Clean up temporary arrays

free(yesIds);
free(noIds);
free(nodes);

// * 9. Retrn 1 on success
fclose(fptr);
return 1;


// if there is an error free everythign
load_err:
    if(nodes){
        for(uint32_t i=0; i<count; i++){
            if(nodes[i]){
                free(nodes[i]->text);
                free(nodes[i]);
            }
        }
    }   
    free(nodes);
    free(yesIds);
    free(noIds);
    fclose(fptr);
    return 0;
}