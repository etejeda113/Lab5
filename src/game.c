#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "lab5.h"

extern Node *g_root;
extern EditStack g_undo;
extern EditStack g_redo;
extern Hash g_index;
char *strdup(const char *s);


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

    Node *parent = NULL; // track parent node for learning phase
    int parentAnswer = -1; // record whether last answer was yes/no


    // Initialize stack
    FrameStack stack;
    fs_init(&stack);
    fs_push(&stack, g_root, -1);

    // flag to exit game
    int done = 0;

    // begin traversal
    while (!fs_empty(&stack) && !done) {
        Frame currentFrame = fs_pop(&stack);
        Node *cur = currentFrame.node;

        if (!cur) break;

        // Question node
        if (cur->isQuestion) {
            char prompt[256];
            snprintf(prompt, sizeof(prompt), "%s (y/n): ", cur->text);
            int answer = get_yes_no(6, 2, prompt);

            // track parent and which branch selected
            parent = cur;

            if(answer){
                parentAnswer = 1;
            }else{
                parentAnswer = 0;
            }

            // Push next node (yes/no) branch onto the stack
            Node *next = (answer == 1) ? cur->yes : cur->no;
            fs_push(&stack, next, answer);

            mvprintw(6, 2, "%-76s", ""); // clear input line
            refresh();
        } 

         // Leaf node (animal)

        else{
        char prompt[256];
        snprintf(prompt, sizeof(prompt), "Is it a %s? (y/n): ", cur->text);
        int correct = get_yes_no(6, 2, prompt);
        mvprintw(6, 2, "%-76s", ""); 

        // If guess is correct
        if (correct) {
            attron(COLOR_PAIR(3) | A_BOLD);
            mvprintw(8, 2, "Yay! I guessed it!");
            attroff(COLOR_PAIR(3) | A_BOLD);
            mvprintw(10, 2, "Press any key to return...");
            refresh();
            getch();
            done = 1;
        } 

        // LEARNING PHASE (Wrong Guess)        
        else {
            //ask for correct animal name
        char *newAnimal_in = get_input(8, 2, "What animal were you thinking of? ");
        if (!newAnimal_in || newAnimal_in[0] == '\0') {
            mvprintw(10, 2, "No animal provided. Press any key to return...");
            refresh();
            getch();
            break;
        }

        
        char *newAnimalCopy = strdup(newAnimal_in);
        if (!newAnimalCopy) {
            mvprintw(12, 2, "No animal provided. Press any key to return...");
            refresh();
            getch();
            break;
        }

        // ask for a question to distinguish your animal from current animal list
        char *prompt_q = get_input(9, 2, "Provide a (yes/no) question to distinguish it: ");
        if (!prompt_q || prompt_q[0] == '\0') {
            free(newAnimalCopy);
            mvprintw(11, 2, "No question provided. Press any key to return...");
            refresh();
            getch();
            break;
        }
        
        char *prompt_qCopy = strdup(prompt_q);
        if (!prompt_qCopy) {
            free(newAnimalCopy);
            mvprintw(12, 2, "No question provided. Press any key to return...");
            refresh();
            getch();
            break;
        }
        
        // ask what the correct ans is for the new animal
        int answeredYes = get_yes_no(10, 2, "For your animal, what is the answer? (y/n): ");

        // create new node for question and animal
        Node *qNode = create_question_node(prompt_qCopy);
        Node *ansNode = create_animal_node(newAnimalCopy);

        free (prompt_qCopy); // free copy
        free (newAnimalCopy); // free copy

        if (!qNode || !ansNode) {
            mvprintw(10, 2, "Error creating nodes. Press any key to return...");
            refresh();
            getch();
            if (qNode) free_tree(qNode);
            if (ansNode) free_tree(ansNode);
            break;
        }

        //link new nodes
        if (answeredYes) {
            qNode->yes = ansNode;
            qNode->no = cur;
        } else {
            qNode->yes = cur;
            qNode->no = ansNode;
        }

        if (!parent) { // update parent pointer 
            g_root = qNode;
        } else if (parentAnswer == 1) {
            parent->yes = qNode;
        } else {
            parent->no = qNode;
        }

        Edit e;
        e.type = EDIT_INSERT_SPLIT;
        e.parent = parent;
        e.wasYesChild = (parent == NULL) ? -1 : (parentAnswer ? 1 : 0);       
        e.oldLeaf = cur;
        e.newQuestion = qNode;
        e.newLeaf = ansNode;

        es_push(&g_undo, e);
        es_clear(&g_redo);

        attron(COLOR_PAIR(3) | A_BOLD);
        mvprintw(12, 2, "Thanks! I'll remember that.");
        attroff(COLOR_PAIR(3) | A_BOLD);
        mvprintw(14, 2, "Press any key to return...");
        refresh();
        getch();
    }
}
    }


    fs_free(&stack); // free mem
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

    if(es_empty(&g_undo)){
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
