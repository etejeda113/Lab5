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

    // TODO: Implement the game loop

    // Initialize FrameStack
    // Push root
    // Loop until stack empty or guess is correct
    // Handle question nodes and leaf nodes differently
    
    FrameStack stack;
    fs_init(&stack);
    
    //   * 3. Push root frame with answeredYes = -1
    //  * 4. Set parent = NULL, parentAnswer = -1

    fs_push(&stack, g_root, -1);

    Node *parent = NULL;
    int parentAnswer = -1;
    int done =0;

    // 5. While stack not empty:

    //*    a. Pop current frame

    while(!fs_empty(&stack) && !done){
        Frame current = fs_pop(&stack);
        Node *cur = current.node;
   
        if(cur == NULL){
            break;
        }

        if(cur->isQuestion == 1){
        char prompt[256];
        snprintf(prompt, sizeof(prompt), "%s (y/n): ", cur->text);
        int answer = get_yes_no(6, 2, prompt);

        parent = cur;
        parentAnswer = answer;

        Node *next = (answer == 1) ? cur->yes : cur->no;
        fs_push(&stack, next, answer);

        mvprintw(6, 2, "%-76s", "");
        refresh();
        continue;
        }

        char prompt [256];
        snprintf(prompt, sizeof(prompt), "Is it a %s? (y/n): ", cur->text);
        int correct = get_yes_no(6, 2, prompt);
        mvprintw(6, 2, "%-76s", ""); 
        refresh();

        if(correct == 1){
            attron(COLOR_PAIR(3) | A_BOLD);
            mvprintw(8, 2, "Yay! I guessed it!");
            attron(COLOR_PAIR(3) | A_BOLD);
            mvprintw(10, 2, "Press any key to return...");
            refresh();
            getch();
            done = -1;
            break;
        }

char *newAnimal = get_input(8, 2, "What animal were you thinking of? ");
if(!newAnimal || newAnimal[0] == '\0'){
    mvprintw(10, 2, "No animal provided. Press any key to return...");
    refresh();
    getch();
    break;
}

// Clear line 8 after getting input
mvprintw(8, 2, "%-76s", "");
refresh();

char prompt_q[256];
snprintf(prompt_q, sizeof(prompt_q), "Provide a question to distinguish %s: ", newAnimal);
char *distQ_in = get_input(8, 2, prompt_q);  // Use line 8 again
if(!distQ_in || distQ_in[0] == '\0'){
    mvprintw(10, 2, "No question provided. Press any key to return...");
    refresh();
    getch();
    break;
}

// Clear line 8 again before the next prompt
mvprintw(8, 2, "%-76s", "");
refresh();

char answer_prompt[256];
snprintf(answer_prompt, sizeof(answer_prompt), "For %s, is the answer yes? (y/n): ", newAnimal);
int answeredYes = get_yes_no(8, 2, answer_prompt);  // Use line 8 for consistency

// Clear the prompt line after getting the answer
mvprintw(8, 2, "%-76s", "");
refresh();

Node *qNode = create_question_node(distQ_in);
Node *ansNode = create_animal_node(newAnimal);

if(!qNode || !ansNode){
    if(qNode) free_tree(qNode);
    if(ansNode) free_tree(ansNode);
    break;
}

if(answeredYes){
    qNode->yes = ansNode;
    qNode->no = cur;
}else{
    qNode->yes = cur;
    qNode->no = ansNode;
}

if(parent == NULL){
    g_root = qNode;
}else if(parentAnswer == 1){
    parent->yes = qNode;
}else{
    parent->no = qNode;
}

Edit e;
e.type = EDIT_INSERT_SPLIT;
e.parent = parent;
e.wasYesChild = parent ? parentAnswer : -1;
e.oldLeaf = cur;
e.newQuestion = qNode;
e.newLeaf = ansNode;

es_push(&g_undo, e);
es_clear(&g_redo);

attron(COLOR_PAIR(3) | A_BOLD);
mvprintw(10, 2, "Thanks! I'll remember that.");
attroff(COLOR_PAIR(3) | A_BOLD);
mvprintw(12, 2, "Press any key to return...");
refresh();
getch();
done = 1;
}
fs_free(&stack);
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
