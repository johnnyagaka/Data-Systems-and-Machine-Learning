/*
 * 
 * You will need to write your B+Tree almost entirely from scratch. 
 * 
 * B+Trees are dynamically balanced tree structures that provides efficient support for insertion, deletion, equality, and range searches. 
 * The internal nodes of the tree direct the search and the leaf nodes hold the base data..
 * 
 * For a basic rundown on B+Trees, we will refer to parts of Chapter 10 of the textbook Ramikrishnan-Gehrke 
 * (all chapters and page numbers in this assignment prompt refer to the 3rd edition of the textbook).
 *
 * Read Chapter 10 which is on Tree Indexing in general. In particular, focus on Chapter 10.3 on B+Tree.
 */
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

#ifndef BTREE_H
#define BTREE_H

#include "data_types.h"
#include "query.h"


/* 
Designing your C Structs for B+Tree nodes (Chapter 10.3.1)
How will you represent a B+Tree as a C Struct (or series of C structs that work together)? There are many valid ways to do this part of your design, and we leave it open to you to try and tune this as you progress through the project.
How will you account for a B+Tree node being an internal node or a leaf node? Will you have a single node type that can conditionally be either of the two types, or will you have two distinct struct types?
How many children does each internal node have? This is called the fanout of the B+Tree.
What is the maximum size for a leaf node? How about for an internal node?
What is the minimum threshold of content for a node, before it has to be part of a rebalancing?
*/
#include<bits/stdc++.h>
//using namespace std;

#define FANOUT 50


// noOfChildNodes = numberOfNodes + 1
int noOfChildNodes;

struct Node{
    //number of nodes
    int tNodes;
    //for parent Node and index
    Node *parentNode;
    //values
    int value[FANOUT];
    //child Nodes
    Node *childNode[FANOUT];
    //Initializing a Node using a constructor
    Node(){ 
        tNodes = 0;
        parentNode = NULL;
        for(int i=0; i<FANOUT; i++){
            value[i] = INT_MAX;
            childNode[i] = NULL;
        }
    }
};
//This line declares the root node of the b+ tree
Node *rootNode = new Node();

//function to split the leaf nodes
void splitLeaf(Node *curNode){
    int x, i, j;

    //split the greater half to the left when numberOfPointer is odd
    //else split equal equal when numberOfPointer is even
    if(noOfChildNodes%2)
        x = (noOfChildNodes+1)/2;
    else x = noOfChildNodes/2;

    //we don't declare another Node for leftNode, rather re-use curNode as leftNode and
    //take away the right half values to the rightNode
    Node *rightNode = new Node();

    //so leftNode has x number of nodes
    curNode->tNodes = x;
    //and rightNode has noOfChildNodes-x
    rightNode->tNodes = noOfChildNodes-x;
    //so both of them have their common parent [even parent may be null, so both of them will have null parent]
    rightNode->parentNode = curNode->parentNode;

    for(i=x, j=0; i<noOfChildNodes; i++, j++){
        //take the right-half values from curNodes and put in the rightNode
        rightNode->value[j] = curNode->value[i];
        //and erase right-half values from curNode to make it real leftNode
        //so that it does not contain all values only contains left-half values
        curNode->value[i] = INT_MAX;
    }
    //for splitting the leaf Nodes we copy the first item from the rightNode to their parentNode
    //and val contains that value
    int val = rightNode->value[0];

    //if the leaf itself is a parent then
    if(curNode->parentNode==NULL){
        //it has null parent, so create a new parent
        Node *parentNode = new Node();
        //and new parent should have a null parent
        parentNode->parentNode = NULL;
        //new parent will have only one member
        parentNode->tNodes=1;
        //and that member is val
        parentNode->value[0] = val;
        //so the parent has two child, so assign them (don't forget curNode is actually the leftNode)
        parentNode->childNode[0] = curNode;
        parentNode->childNode[1] = rightNode;
        //their parent of the left and right Nodes is no longer null, so assign their parent
        curNode->parentNode = rightNode->parentNode = parentNode;
        //from now on this parentNode is the rootNode
        rootNode = parentNode;
        return;
    }
    else{   //if the splitted leaf Node is not rootNode then

        // we have to put the val and assign the rightNode to the right place in the parentNode
        // so we go to the parentNode and from now we consider the curNode as the parentNode of the splitted Node

        curNode = curNode->parentNode;

        //for the sake of insertNodeion sort to put the rightNode and val in the exact position
        //of th parentNode [here curNode] take a new child Node and assign rightNode to it
        Node *newChildNode = new Node();
        newChildNode = rightNode;

        //simple insertion sort to put val at the exact position of values[] in the parentNode [here curNode]

        for(i=0; i<=curNode->tNodes; i++){
            if(val < curNode->value[i]){
                swap(curNode->value[i], val);
            }
        }

        //after putting val number of nodes gets increase by one
        curNode->tNodes++;

         //simple insertNodeion sort to put rightNode at the exact position
         //of childNode[] in the parentNode [here curNode]

        for(i=0; i<curNode->tNodes; i++){
            if(newChildNode->value[0] < curNode->childNode[i]->value[0]){
                swap(curNode->childNode[i], newChildNode);
            }
        }
        curNode->childNode[i] = newChildNode;

        //we reordered some Nodes and pointers, so for the sake of safety
        //all childNodes' should have their parent updated
        for(i=0;curNode->childNode[i]!=NULL;i++){
            curNode->childNode[i]->parentNode = curNode;
        }
    }

}

//function to split the non leaf nodes
void splitNonLeaf(Node *curNode){
    int x, i, j;

    //split the less half to the left when numberOfPointer is odd
    //else split equal equal when numberOfPointer is even.  n/2 does it nicely for us

    x = noOfChildNodes/2;

    //declare rightNode and we will use curNode as the leftNode
    Node *rightNode = new Node();

    //so leftNode has x number of nodes
    curNode->tNodes = x;
    //rightNode has noOfChildNodes-x-1 children, because we won't copy and paste
    //rather delete and paste the first item of the rightNode
    rightNode->tNodes = noOfChildNodes-x-1;
    //both children have their common parent
    rightNode->parentNode = curNode->parentNode;


    for(i=x, j=0; i<=noOfChildNodes; i++, j++){
        //copy the right-half members to the rightNode
        rightNode->value[j] = curNode->value[i];
        //and also copy their children
        rightNode->childNode[j] = curNode->childNode[i];
        //erase the right-half values from curNode to make it perfect leftNode
        //which won't contain only left-half values and their children
        curNode->value[i] = INT_MAX;
        //erase all the right-half childNodes from curNode except the x one
        //because if left child has 3 nodes then it should have 4 childNodes, so don't delete that child
        if(i!=x)curNode->childNode[i] = NULL;
    }

    //we will take a copy of the first item of the rightNode
    //as we will delete that item later from the list
    int val = rightNode->value[0];
    //just right-shift value[] and childNode[] by one from rightNode
    //to have no repeat of the first item for non-leaf Node
    memcpy(&rightNode->value, &rightNode->value[1], sizeof(int)*(rightNode->tNodes+1));
    memcpy(&rightNode->childNode, &rightNode->childNode[1], sizeof(rootNode)*(rightNode->tNodes+1));

    //we reordered some values and positions so don't forget
    //to assign the children's exact parent

    for(i=0;curNode->childNode[i]!=NULL;i++){
        curNode->childNode[i]->parentNode = curNode;
    }
    for(i=0;rightNode->childNode[i]!=NULL;i++){
        rightNode->childNode[i]->parentNode = rightNode;
    }

    //if the splitted Node itself a parent
    if(curNode->parentNode==NULL){
        //create a new parent
        Node *parentNode = new Node();
        //parent should have a null parent
        parentNode->parentNode = NULL;
        //parent will have only one node
        parentNode->tNodes=1;
        //the only value is the val
        parentNode->value[0] = val;
        //it has two children, leftNode and rightNode
        parentNode->childNode[0] = curNode;
        parentNode->childNode[1] = rightNode;

        //and both rightNode and leftNode has no longer null parent, they have their new parent
        curNode->parentNode = rightNode->parentNode = parentNode;

        //from now on this new parent is the root parent
        rootNode = parentNode;
        return;
    }
    else{   //if the splitted leaf Node is not rootNode then

        // we have to put the val and assign the rightNode to the right place in the parentNode
        // so we go to the parentNode and from now we consider the curNode as the parentNode of the splitted Node
        curNode = curNode->parentNode;

        //for the sake of insertNodeion sort to put the rightNode and val in the exact position
        //of th parentNode [here curNode] take a new child Node and assign rightNode to it

        Node *newChildNode = new Node();
        newChildNode = rightNode;

        //simple insertion sort to put val at the exact position of values[] in the parentNode [here curNode]


        for(i=0; i<=curNode->tNodes; i++){
            if(val < curNode->value[i]){
                swap(curNode->value[i], val);
            }
        }

        //after putting val number of nodes gets increase by one
        curNode->tNodes++;

        //simple insertNodeion sort to put rightNode at the exact position
         //of childNode[] in the parentNode [here curNode]

        for(i=0; i<curNode->tNodes; i++){
            if(newChildNode->value[0] < curNode->childNode[i]->value[0]){
                swap(curNode->childNode[i], newChildNode);
            }
        }
        curNode->childNode[i] = newChildNode;

        //we reordered some Nodes and pointers, so for the sake of safety
        //all childNodes' should have their parent updated
         for(i=0;curNode->childNode[i]!=NULL;i++){
            curNode->childNode[i]->parentNode = curNode;
        }
    }

}

void insertNode(Node *curNode, int val){

    for(int i=0; i<=curNode->tNodes; i++){
        if(val < curNode->value[i] && curNode->childNode[i]!=NULL){
            insertNode(curNode->childNode[i], val);
            if(curNode->tNodes==noOfChildNodes)
                splitNonLeaf(curNode);
            return;
        }
        else if(val < curNode->value[i] && curNode->childNode[i]==NULL){
            swap(curNode->value[i], val);
            //swap(curNode->childNode[i], newChildNode);
            if(i==curNode->tNodes){
                    curNode->tNodes++;
                    break;
            }
        }
    }

    if(curNode->tNodes==noOfChildNodes){

            splitLeaf(curNode);
    }
}


void redistributeNode(Node *leftNode, Node *rightNode, bool isLeaf, int posOfLeftNode, int whichOneisCurNode){

    //re-distribution will affect the first value of the rightNode, so remember it
    //for later replacement of the copy of this value somewhere in ancestor Node
    int PrevRightFirstVal = rightNode->value[0];


    if(whichOneisCurNode==0){ //leftNode is curNode

        //if the Nodes are not leaf node
        if(!isLeaf){
            //bring down the value from which it is left child in parentNode
            leftNode->value[leftNode->tNodes] = leftNode->parentNode->value[posOfLeftNode];
            //the right child of newly added value should be the left child of first value of rightNode
            leftNode->childNode[leftNode->tNodes+1] = rightNode->childNode[0];
            //increase leftNode's number of nodes by one
            leftNode->tNodes++;
            //send up a the first value of the rightNode to the parentNode
            leftNode->parentNode->value[posOfLeftNode] = rightNode->value[0];
            //shift left by one in rightNode
            memcpy(&rightNode->value[0], &rightNode->value[1], sizeof(int)*(rightNode->tNodes+1));
            memcpy(&rightNode->childNode[0], &rightNode->childNode[1], sizeof(rootNode)*(rightNode->tNodes+1));
            rightNode->tNodes--;

        }else{
            //borrow the first value of rightNode to the last position of leftNode
            leftNode->value[leftNode->tNodes] = rightNode->value[0];
            leftNode->tNodes++;
            //shift by one node to left of the rightNode
            memcpy(&rightNode->value[0], &rightNode->value[1], sizeof(int)*(rightNode->tNodes+1));
            //decrease number of nodes by one
            rightNode->tNodes--;

            leftNode->parentNode->value[posOfLeftNode] = rightNode->value[0];
        }



    }else{ //rightNode is curNode

        if(!isLeaf){

            //shift right by one in rightNode so that first position becomes free
            memcpy(&rightNode->value[1], &rightNode->value[0], sizeof(int)*(rightNode->tNodes+1));
            memcpy(&rightNode->childNode[1], &rightNode->childNode[0], sizeof(rootNode)*(rightNode->tNodes+1));
            //bring down the value from which it is left child in parentNode to first pos of rightNode
            rightNode->value[0] = leftNode->parentNode->value[posOfLeftNode];
            //and the left child of the newly first value of right child will be the last child of leftNode
            rightNode->childNode[0] = leftNode->childNode[leftNode->tNodes];

            rightNode->tNodes++;

            //send up a the last value of the leftNode to the parentNode
            leftNode->parentNode->value[posOfLeftNode] = leftNode->value[leftNode->tNodes-1];
            //erase the last element and pointer of leftNode
            leftNode->value[leftNode->tNodes-1] = INT_MAX;
            leftNode->childNode[leftNode->tNodes] = NULL;
            leftNode->tNodes--;

        }else{

            //shift by one node to right of the rightNode so that we can free the first position
            memcpy(&rightNode->value[1], &rightNode->value[0], sizeof(int)*(rightNode->tNodes+1));
            //borrow the last value of leftNode to the first position of rightNode
            rightNode->value[0] = leftNode->value[leftNode->tNodes-1];
            //increase number of nodes by one
            rightNode->tNodes++;

            leftNode->value[leftNode->tNodes-1] = INT_MAX;
            leftNode->tNodes--;

            leftNode->parentNode->value[posOfLeftNode] = rightNode->value[0];
        }
    }
}

void mergeNode(Node *leftNode, Node *rightNode, bool isLeaf, int posOfRightNode){

    //cout << "leftNode " << leftNode->value[0] << " rightNode " << rightNode->value[0] << endl;
    //cout << "size " << leftNode->tNodes << " size " << rightNode->tNodes << endl;
    if(!isLeaf){

        leftNode->value[leftNode->tNodes] = leftNode->parentNode->value[posOfRightNode-1];
        leftNode->tNodes++;
    }

    memcpy(&leftNode->value[leftNode->tNodes], &rightNode->value[0], sizeof(int)*(rightNode->tNodes+1));
    memcpy(&leftNode->childNode[leftNode->tNodes], &rightNode->childNode[0], sizeof(rootNode)*(rightNode->tNodes+1));


    leftNode->tNodes += rightNode->tNodes;


    //cout << "before: " << leftNode->parentNode->value[1] << endl;
    memcpy(&leftNode->parentNode->value[posOfRightNode-1], &leftNode->parentNode->value[posOfRightNode], sizeof(int)*(leftNode->parentNode->tNodes+1));
    memcpy(&leftNode->parentNode->childNode[posOfRightNode], &leftNode->parentNode->childNode[posOfRightNode+1], sizeof(rootNode)*(leftNode->parentNode->tNodes+1));
    //cout << "after merging " << leftNode->parentNode->childNode[posOfRightNode-2]->value[0] << " and ";// << leftNode->parentNode->childNode[posOfRightNode]->value[0] << endl;
    leftNode->parentNode->tNodes--;

    //we reordered some Nodes and pointers, so for the sake of safety
    //all childNodes' should have their parent updated
     for(int i=0;leftNode->childNode[i]!=NULL;i++){
        leftNode->childNode[i]->parentNode = leftNode;
    }



}
bool dataFound;
void deleteNode(Node *curNode, int val, int curNodePosition){

     //to check if the current Node is a leaf or not
     bool isLeaf;
     if(curNode->childNode[0]==NULL)
        isLeaf = true;
     else isLeaf = false;

    //left most value could be changed due to merge or re-distribution later,
    //so keep it to replace it's copy from it's ancestor
    int prevLeftMostVal = curNode->value[0];



     for(int i=0;dataFound==false &&  i<=curNode->tNodes; i++){
        if(val < curNode->value[i] && curNode->childNode[i] != NULL){
            deleteNode(curNode->childNode[i], val, i);

        }
        //if we could find the target value at any leafNode then
        else if(val == curNode->value[i] && curNode->childNode[i] == NULL){

            //delete the node by shifting all values and pointers  by one to the left
            memcpy(&curNode->value[i], &curNode->value[i+1], sizeof(int)*(curNode->tNodes+1));
            //decrease number of nodes by one
            curNode->tNodes--;
            dataFound = true;
            break;
        }
     }

    //if the root is the only leaf
     if(curNode->parentNode == NULL && curNode->childNode[0] == NULL){
        return;
     }


  //if the curNode is rootNode and it has one pointers only
    if(curNode->parentNode==NULL && curNode->childNode[0] != NULL && curNode->tNodes == 0){
        rootNode = curNode->childNode[0];
        rootNode->parentNode = NULL;
        return;
    }


    //now check if the curNode has less than half of the number of maximum node
    //cout << curNodePosition << endl;
    //if less than half we will try to re-distribute first

    //cout << curNode->childNode[0]->value[0] << " "<< curNodePosition << endl;
    if(isLeaf && curNode->parentNode!=NULL){

        if(curNodePosition==0){
            Node *rightNode = new Node();
            rightNode = curNode->parentNode->childNode[1];

            //if we the right one has more than half nodes of maximum capacity than re-distribute
            if(rightNode!=NULL && rightNode->tNodes > (noOfChildNodes+1)/2){

                    redistributeNode(curNode, rightNode, isLeaf, 0, 0);
            }
            //else there is nothing to re-distribute, so we can merge them
            else if (rightNode!=NULL && curNode->tNodes+rightNode->tNodes < noOfChildNodes){

                    mergeNode(curNode, rightNode, isLeaf, 1);
            }
        }

        else{


            Node *leftNode = new Node();
            Node *rightNode = new Node();


            leftNode = curNode->parentNode->childNode[curNodePosition-1];

            rightNode = curNode->parentNode->childNode[curNodePosition+1];


            //if we see that left one has more than half nodes of maximum capacity then try to re-distribute
            if(leftNode!=NULL && leftNode->tNodes > (noOfChildNodes+1)/2){
                 redistributeNode(leftNode, curNode, isLeaf, curNodePosition-1, 1);
            }
            else if(rightNode!=NULL && rightNode->tNodes > (noOfChildNodes+1)/2){
                redistributeNode(curNode, rightNode, isLeaf, curNodePosition, 0);
            }
            else if (leftNode!=NULL && curNode->tNodes+leftNode->tNodes < noOfChildNodes){
                mergeNode(leftNode, curNode, isLeaf, curNodePosition);
            }
            else if (rightNode!=NULL && curNode->tNodes+rightNode->tNodes < noOfChildNodes){
               mergeNode(curNode, rightNode, isLeaf, curNodePosition+1);
            }
        }
    }
    else if(!isLeaf && curNode->parentNode!=NULL){

        if(curNodePosition==0){
            Node *rightNode = new Node();
            rightNode = curNode->parentNode->childNode[1];

            //if we see the right one has more than half nodes of maximum capacity than re-distribute
            if( rightNode!=NULL && rightNode->tNodes-1 >= ceil((noOfChildNodes-1)/2) ){
                redistributeNode(curNode, rightNode, isLeaf, 0, 0);
            }
            //else there is nothing to re-distribute, so we can merge them
            else if (rightNode!=NULL && curNode->tNodes+rightNode->tNodes < noOfChildNodes - 1){
                mergeNode(curNode, rightNode, isLeaf, 1);
            }
        }
        //for any other case we can safely take the left one to try for re-distribution
        else{


            Node *leftNode = new Node();
            Node *rightNode = new Node();


            leftNode = curNode->parentNode->childNode[curNodePosition-1];

            rightNode = curNode->parentNode->childNode[curNodePosition+1];


            //if we see that left one has more than half nodes of maximum capacity then try to re-distribute
            if( leftNode!=NULL && leftNode->tNodes-1 >= ceil((noOfChildNodes-1)/2)){
                 redistributeNode(leftNode, curNode, isLeaf, curNodePosition-1, 1);
            }
            else if(rightNode!=NULL && rightNode->tNodes-1 >=  ceil((noOfChildNodes-1)/2)){
                redistributeNode(curNode, rightNode, isLeaf, curNodePosition, 0);
            }
            //else there is nothing to re-distribute, so we merge them

            else if ( leftNode!=NULL && curNode->tNodes+leftNode->tNodes < noOfChildNodes-1) {
                mergeNode(leftNode, curNode, isLeaf, curNodePosition);
            }
            else if ( rightNode!=NULL && curNode->tNodes+rightNode->tNodes < noOfChildNodes-1){
               mergeNode(curNode, rightNode, isLeaf, curNodePosition+1);
            }
        }

    }



    //delete the duplicate if any in the ancestor Node
    Node *tempNode = new Node();
    tempNode = curNode->parentNode;
    while(tempNode!=NULL){
            for(int i=0; i<tempNode->tNodes;i++){
                if(tempNode->value[i]==prevLeftMostVal){
                    tempNode->value[i] = curNode->value[0];
                    break;
                }
        }
        tempNode = tempNode->parentNode;
    }

}

void print(vector < Node* > Nodes){
    vector < Node* > newNodes;
    for(int i=0; i<Nodes.size(); i++){ //for every Node
        Node *curNode = Nodes[i];

        cout <<"[|";
        int j;
        for(j=0; j<curNode->tNodes; j++){  //traverse the childNodes, print values and save all the childNodes
            cout << curNode->value[j] << "|";
            if(curNode->childNode[j]!=NULL)
            newNodes.push_back(curNode->childNode[j]);
        }
        if(curNode->value[j]==INT_MAX && curNode->childNode[j]!=NULL)
            newNodes.push_back(curNode->childNode[j]);

        cout << "]  ";
    }

    if(newNodes.size()==0){ //if there is no childNode Node left to send out then just the end of the recursion

        puts("");
        puts("");
        Nodes.clear();
        //exit(0);
    }
    else {                    //else send the childNodes to the recursion to continue to the more depth
        puts("");
        puts("");
        Nodes.clear();
        print(newNodes);
    }
}

int main(){
    int num[50];// = {1,4,7,10,17,21,31,25,19,20,28,42};
    // int num[] = {5,10,15,20,25,28,30,50,55,60,65,70,75,80,85,90,95};


    printf("Pointers: ");
    scanf("%d", &noOfChildNodes);

    FILE *p;
    p = fopen("input.txt", "r");
    freopen("output.txt", "w", stdout);


    vector < Node* > Nodes;


    int ch;
    int i = 0;
    int totalValues = 0;
    //input from file
    printf("1.Insert a value\n2.Print the tree\n3.Delete a value\n");
    while(fscanf(p, "%d", &ch)!=EOF){

        if(ch==1){
            fscanf(p, "%d", &num[i]);
            printf("**** Insert %d ****\n\n", num[i]);
            insertNode(rootNode, num[i]);
            i++;
            totalValues++;

        }else if(ch==2){
            Nodes.clear();
            Nodes.push_back(rootNode);
            print(Nodes);
            puts("");

        }else if(ch==3) {
            int val;
            fscanf(p, "%d", &val);
            if(totalValues==0) {
                printf("Sorry! There is no more data to be deleted!!");
                continue;

            }
            printf("---- Delete %d ----\n\n", val);
            dataFound = false;
            deleteNode(rootNode, val, 0);
            totalValues--;
        }
    }
    //input manually
     while(scanf("%d", &ch)!=EOF){


        if(ch==1){
            scanf("%d", &num[i]);
            printf("**** Insert %d ****\n\n", num[i]);
            insertNode(rootNode, num[i]);
            i++;
            totalValues++;

        }else if(ch==2){
            Nodes.clear();
            Nodes.push_back(rootNode);
            print(Nodes);
            puts("");

        }else if(ch==3) {
            int val;
            scanf("%d", &val);
            if(totalValues==0) {
                printf("Sorry! There is no more data to be deleted!!");
                continue;

            }
            printf("---- Delete %d ----\n\n", val);
            dataFound = false;
            deleteNode(rootNode, val, 0);
            totalValues--;
        }
    }


    return 0;
}

// TODO: here you will need to define a B+Tree node(s) struct(s)


/* The following are methods that can be invoked on B+Tree node(s).
 * Hint: You may want to review different design patterns for passing structs into C functions.
 */

/* FIND (Chapter 10.4)
This is an equality search for an entry whose key matches the target key exactly.
How many nodes need to be accessed during an equality search for a key, within the B+Tree? 
*/

// TODO: here you will need to define FIND/SEARCH related method(s) of finding key-values in your B+Tree.


/* INSERT (Chapter 10.5)
How does inserting an entry into the tree differ from finding an entry in the tree?
When you insert a key-value pair into the tree, what happens if there is no space in the leaf node? What is the overflow handling algorithm?
For Splitting B+Tree Nodes (Chapter 10.8.3)
*/

// TODO: here you will need to define INSERT related method(s) of adding key-values in your B+Tree.


/* BULK LOAD (Chapter 10.8.2)
Bulk Load is a special operation to build a B+Tree from scratch, from the bottom up, when beginning with an already known dataset.
Why might you use Bulk Load instead of a series of inserts for populating a B+Tree? Compare the cost of a Bulk Load of N data entries versus that of an insertion of N data entries? What are the tradeoffs?
*/

// TODO: here you will need to define BULK LOAD related method(s) of initially adding all at once some key-values to your B+Tree.
// BULK LOAD only can happen at the start of a workload


/*RANGE (GRADUATE CREDIT)
Scans are range searches for entries whose keys fall between a low key and high key.
Consider how many nodes need to be accessed during a range search for keys, within the B+Tree?
Can you describe two different methods to return the qualifying keys for a range search? 
(Hint: how does the algorithm of a range search compare to an equality search? What are their similarities, what is different?)
Can you describe a generic cost expression for Scan, measured in number of random accesses, with respect to the depth of the tree?
*/

// TODO GRADUATE: here you will need to define RANGE for finding qualifying keys and values that fall in a key range.



#endif