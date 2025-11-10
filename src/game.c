#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "lab5.h"

extern Node *g_root;
extern EditStack g_undo;
extern EditStack g_redo;
extern Hash g_index;

/* TODO 31: Implement play_game
 * Main game loop using iterative traversal with a stack
 * 
 * Key requirements:
 * - Use FrameStack (NO recursion!)
 * - Push frames for each decision point
 * - Track parent and answer for learning
 * 
 * Steps:
 * 1. Initialize and display game UI
 * 2. Initialize FrameStack
 * 3. Push root frame with answeredYes = -1
 * 4. Set parent = NULL, parentAnswer = -1
 * 5. While stack not empty:
 *    a. Pop current frame
 *    b. If current node is a question:
 *       - Display question and get user's answer (y/n)
 *       - Set parent = current node
 *       - Set parentAnswer = answer
 *       - Push appropriate child (yes or no) onto stack
 *    c. If current node is a leaf (animal):
 *       - Ask "Is it a [animal]?"
 *       - If correct: celebrate and break
 *       - If wrong: LEARNING PHASE
 *         i. Get correct animal name from user
 *         ii. Get distinguishing question
 *         iii. Get answer for new animal (y/n for the question)
 *         iv. Create new question node and new animal node
 *         v. Link them: if newAnswer is yes, newQuestion->yes = newAnimal
 *         vi. Update parent pointer (or g_root if parent is NULL)
 *         vii. Create Edit record and push to g_undo
 *         viii. Clear g_redo stack
 *         ix. Update g_index with canonicalized question
 * 6. Free stack
 */

void play_game() {
    clear();
    attron(COLOR_PAIR(5) | A_BOLD);
    mvprintw(0, 0, "%-80s", " Playing 20 Questions");
    attroff(COLOR_PAIR(5) | A_BOLD);
    
    mvprintw(2, 2, "Think of an animal, and I'll try to guess it!");
    mvprintw(3, 2, "Press any key to start...");
    refresh();
    getch();

    /*return 
    // TODO: Implement the game loop
    // Initialize FrameStack
    // Push root
    // Loop until stack empty or guess is correct
    // Handle question nodes and leaf nodes differently
    
    FrameStack stack;
    fs_init(&stack);
    
    //   * 3. Push root frame with answeredYes = -1

    Frame frame;
    frame.node = g_root;
    frame.answeredYes = -1;

    //  * 4. Set parent = NULL, parentAnswer = -1

    Node *parent = NULL;
    int parentAnswer = -1;

    fs_push(&stack, g_root, -1);

    // 5. While stack not empty:

    //*    a. Pop current frame

    while(!fs_empty(&stack)){
        Frame current = fs_pop(&stack);
        Node *node = current.node;


    /*    b. If current node is a question:
    *       - Display question and get user's answer (y/n)
    *       - Set parent = current node
    *       - Set parentAnswer = answer
    *       - Push appropriate child (yes or no) onto stack

   
    if(node->isQuestion == 1){
        char answer;
        while(1){
            printf("%s (y/n)?", node->text);
            refresh();
            answer = getch();
        
        if(answer == 'y' || answer == 'Y' || answer == 'n' || answer == 'N') break;
        }

        parent = node;
        if(answer == 'y' || answer == 'Y'){
            parentAnswer = 1; 
        }
        else{
            parentAnswer = 0;
        }

        if(parentAnswer){
            fs_push(&stack, node->yes, 1);
        }

        else{
            fs_push(&stack, node->no, 0);
        }
*/
        /*    c. If current node is a leaf (animal):
 *       - Ask "Is it a [animal]?"
 *       - If correct: celebrate and break
 *       - If wrong: LEARNING PHASE
 *         i. Get correct animal name from user
 *         ii. Get distinguishing question
 *         iii. Get answer for new animal (y/n for the question)
 *         iv. Create new question node and new animal node
 *         v. Link them: if newAnswer is yes, newQuestion->yes = newAnimal
 *         vi. Update parent pointer (or g_root if parent is NULL)
 *         vii. Create Edit record and push to g_undo
 *         viii. Clear g_redo stack
 *         ix. Update g_index with canonicalized question
*/
    
    // TODO: Your implementation here
    }

/* TODO 32: Implement undo_last_edit
 * Undo the most recent tree modification
 * 
 * Steps:
 * 1. Check if g_undo stack is empty, return 0 if so
 * 2. Pop edit from g_undo
 * 3. Restore the tree structure:
 *    - If edit.parent is NULL:
 *      - Set g_root = edit.oldLeaf
 *    - Else if edit.wasYesChild:
 *      - Set edit.parent->yes = edit.oldLeaf
 *    - Else:
 *      - Set edit.parent->no = edit.oldLeaf
 * 4. Push edit to g_redo stack
 * 5. Return 1
 * 
 * Note: We don't free newQuestion/newLeaf because they might be redone
 */
int undo_last_edit() {
    // TODO: Implement this function

    //  * 1. Check if g_undo stack is empty, return 0 if so

    if(es_empty(&g_undo) == 1){
        return 0;
    }

    //  * 2. Pop edit from g_undo

    Edit edit = es_pop(&g_undo);

    if(edit.parent == NULL){
        g_root = edit.oldLeaf;
    }
    else if(edit.wasYesChild == 1){
        edit.parent->yes = edit.oldLeaf;
    }
    else{
        edit.parent->no = edit.oldLeaf;
    }
    
    es_push (&g_redo, edit);

    return 1;
}

/* TODO 33: Implement redo_last_edit
 * Redo a previously undone edit
 * 
 * Steps:
 * 1. Check if g_redo stack is empty, return 0 if so
 * 2. Pop edit from g_redo
 * 3. Reapply the tree modification:
 *    - If edit.parent is NULL:
 *      - Set g_root = edit.newQuestion
 *    - Else if edit.wasYesChild:
 *      - Set edit.parent->yes = edit.newQuestion
 *    - Else:
 *      - Set edit.parent->no = edit.newQuestion
 * 4. Push edit back to g_undo stack
 * 5. Return 1
 */
int redo_last_edit() {
    // TODO: Implement this function
    if(es_empty(&g_redo) == 1){
        return 0;
    }

    Edit edit = es_pop(&g_redo);

    if(edit.parent == NULL){
        g_root = edit.newQuestion;
    }
    else if(edit.wasYesChild==1){
        edit.parent->yes = edit.newQuestion;
    }
    else{
        edit.parent->no = edit.newQuestion;
    }
    es_push(&g_undo, edit);
    return 1;
}
