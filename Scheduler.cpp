

#include <iostream>
#include <vector>
#include <queue>

using namespace std;


//Step 1 Create a input table
// We will need to read in the inputs and then stores them into a vector
// First lets make a struct to store the input
// The struct just needs the time and type
struct input{
    string operation;
    int parameter;
};


//Step 2 Create a process table
// We need to crate a process table that hold all of the 
struct processInfo{
    int processID; // We let us know which process it is. Ex p1, p2, p3 ... pn
    int firstLine; // Were the process begins
    int lastLine; // Were the process ends
    int currentLine; // Were the process currently is
    int state; // Ready - 1, Running - 2, Blocked - 3
};

//Step 3
// Create the process structure
struct process{
    int processID;
    float time;
    string instruction;
    int logicalReads;
    int physicalWrites;
    int physicalReads;
    int bufferSize;
    process() : logicalReads(0), physicalWrites(0), physicalReads(0), bufferSize(0) {}
};

//Step 4 setting up the main queue
struct Node{
    process data;
    Node *next;
};

class PrioQueue{
private:
    Node* front;
    Node* rear;
public:
    // basic contructor
    PrioQueue(){
        front = nullptr;
        rear = nullptr;
    };
    // destructor
    ~PrioQueue(){
        Node* p = front;
        while (front){
            front = front->next;
            delete p;
            p = front;
        }
    };
    // how we will add process to the main queue
    void enqueue(process x){
    Node* t = new Node;
    if (t == nullptr){
        cout << "Queue Overflow" << endl;
    } 
    else {
        t->data = x;
        t->next = nullptr;
        if (front == nullptr){ // if the queue is empty
            front = t;
            rear = t;
        }
        else if(t->data.time < front->data.time){ // when we are inserting at the begin
            t->next = front;
            front = t;
        }
        else {
            Node* p = front; // when we need to indert in the middle or end
            while(p->next && (t->data.time > p->next->data.time)){ // traversing to find the right spot
                p = p->next;
            }
            if(p->next == nullptr){ // Inserting at the end
                    rear->next = t;
                    rear = t;
            }
            else if(p->next->data.time == t->data.time){ // how to deal with when the values are equal
                t->next = p->next->next;
                p->next->next = t;
            }
            else{ // how to deal with inbetween insertion
                
                t->next = p->next;
                p->next = t;
            }

            }
        }
    };
    
    // how we will remove process from the main queue
    void dequeue() {
        process x;
        Node* p;
        if (isEmpty()){
            cout << "Queue Underflow" << endl;
        } else {
            p = front;
            front = front->next;
            x = p->data;
            delete p;
        }
    };
    // empty check
    bool isEmpty(){
        if (front == nullptr){
            return true;
        }
        return false;
    };
    // If we want to see what in the main queue
    void display(){
        Node *p = front;
        while(p){
            //cout << "THis is the id: " << p->data.processID << " This is the process time: " << p->data.time << ". This is the process type: " << p->data.instruction << endl;
            p=p->next;
        }
        //cout << endl;
    };

    process top(){
        return front->data;
    }

};

// end of step 4 and mainqueue

//bufferfunction
float bigbuffer(float necessaryBytes, int currentBSIZE, int BSIZE){
    int finalBufferSize;

    if(necessaryBytes <= currentBSIZE){ // when we have enough bytes
        finalBufferSize = currentBSIZE - necessaryBytes;
    }
    else{ // when we dont have enought bytes
        int bytesMiss = necessaryBytes - currentBSIZE;
        int bytesBrought;
        if(bytesMiss % BSIZE ==0){
            bytesBrought = bytesMiss;
        }
        else{
            bytesBrought = (bytesMiss / BSIZE + 1) * BSIZE;
        }
        finalBufferSize = bytesBrought - bytesMiss;
    }

    return finalBufferSize;
}

//Step 5 adding the necessary global variables
queue<process> readyQueue;
queue<process> sQueue;
bool CPU = 0;
bool SSD = 0;
float clockTime;
float bsize;
PrioQueue mainPriorityQueue;
int runningProcess = 0;

//Step 5 is complete

//Step 6 adding the arrival function and core request function
void arrival(process& currentProcess, vector<input>& inputT, vector<processInfo>& pTable){
    pTable[currentProcess.processID].currentLine = pTable[currentProcess.processID].firstLine + 1; // we increment the current line
    currentProcess.instruction = inputT[pTable[currentProcess.processID].currentLine].operation; // we update to the new command
    currentProcess.time = inputT[pTable[currentProcess.processID].currentLine].parameter;// updating the time
};

// The arrival seems to be working now lets move on to the core request function
void coreRequest(process& currentProcess, vector<input>& inputT, vector<processInfo>& pTable){
    if(CPU == 0){
        CPU=1;
        pTable[currentProcess.processID].state = 2; // we set the state to running =2 
        // fine the completion time of the process
        currentProcess.time = clockTime + currentProcess.time;
        mainPriorityQueue.enqueue(currentProcess);
    }
    else{
        pTable[currentProcess.processID].state = 1; // if the cpu is full set the status to ready = 1
        // push the process into the ready que
        readyQueue.push(currentProcess);
    }
    
};

//Step 6 is complete both funciton are working

//Step7 Introducing the core completion function
//Step 8 setting up a ssd request *** in progress
void SSDRequest(process& currentProcess, vector<input>& inputT, vector<processInfo>& pTable){
    if(SSD == 0){
        pTable[currentProcess.processID].state = 3;
        if(currentProcess.instruction == "WRITE"){
            currentProcess.physicalWrites +=1;
            currentProcess.time = clockTime + 0.1;
            mainPriorityQueue.enqueue(currentProcess);
        }
        else if(currentProcess.instruction == "READ"){
            if(currentProcess.time > currentProcess.bufferSize){
                //cout << "in the SSDREquest current buffer amount needed = " << currentProcess.time << endl;
                currentProcess.bufferSize = bigbuffer(currentProcess.time,currentProcess.bufferSize,inputT[0].parameter);
                currentProcess.physicalReads +=1;
                currentProcess.time = clockTime + 0.1;
                mainPriorityQueue.enqueue(currentProcess);
                //cout << "this is the buffer size in the end: " << currentProcess.bufferSize << endl;
            }
            else{
                currentProcess.bufferSize = bigbuffer(currentProcess.time,currentProcess.bufferSize,inputT[0].parameter);
                currentProcess.logicalReads +=1;
                currentProcess.time = clockTime; // new edit ***
                mainPriorityQueue.enqueue(currentProcess);
            }
        
        }
        else{
            sQueue.push(currentProcess);
        }
    }
}
void coreCompletion(process& currentProcess, vector<input>& inputT, vector<processInfo>& pTable){
    CPU = 0;
    if(!readyQueue.empty()){
        process top = readyQueue.front();
        readyQueue.pop();
        coreRequest(top,inputT,pTable);
    }
    if(pTable[currentProcess.processID].currentLine == pTable[currentProcess.processID].lastLine){
        //Process 0 terminates at t = 310ms.
        //It performed 0 physical read(s), 0 logical read(s), and 1 physical write(s).
        cout << endl;
        cout << "Process " << currentProcess.processID << " terminated at t = " << clockTime << "ms." << endl;
        cout << "It perfomed " << currentProcess.physicalReads << " physical read(s), " << currentProcess.logicalReads <<
        " logical read(s), and " << currentProcess.physicalWrites << " physical write(s)." << endl;
        cout << endl;
        cout << "Process states:" << endl << "--------------" << endl;
        cout << currentProcess.processID << " Terminated" << endl;
        pTable[currentProcess.processID].state = -1;
        for(int i = 0; i < pTable.size(); i++){
            if(pTable[i].state == 1){
                cout << pTable[i].processID << " Ready" << endl;
            }
            else if(pTable[i].state == 2){
                cout << pTable[i].processID << " Running" << endl;
            }
        }
    }
    else{
        pTable[currentProcess.processID].currentLine = pTable[currentProcess.processID].currentLine +1;
        currentProcess.instruction = inputT[pTable[currentProcess.processID].currentLine].operation;
        currentProcess.time = inputT[pTable[currentProcess.processID].currentLine].parameter;
        //cout << "this is the currentProcess: inside the core completion event: " << currentProcess.instruction << endl;
        if((currentProcess.instruction == "READ") || (currentProcess.instruction == "WRITE")){
            SSDRequest(currentProcess,inputT,pTable);
        }
        if(currentProcess.instruction == "INPUT" || currentProcess.instruction == "DISPLAY"){
            currentProcess.time += clockTime;
            mainPriorityQueue.enqueue(currentProcess);
        }
    }
}


//Step 9 core completion func
void ssdCompletion(process& currentProcess, vector<input>& inputT, vector<processInfo>& pTable){
    SSD = 0;
    if(!sQueue.empty()){
        process top = sQueue.front();
        sQueue.pop();
        SSDRequest(top,inputT,pTable);
    }
    else{
        //cout << "im in the ssdCompletion else statments" << endl;
        pTable[currentProcess.processID].currentLine = pTable[currentProcess.processID].currentLine +1;
        currentProcess.instruction = inputT[pTable[currentProcess.processID].currentLine].operation;
        currentProcess.time = inputT[pTable[currentProcess.processID].currentLine].parameter;
        //cout << "this is the current line: " << pTable[currentProcess.processID].currentLine << " and this is the instruction " << currentProcess.instruction << " and this is the instruction's time: " << currentProcess.time << endl;
        coreRequest(currentProcess,inputT,pTable);
    }
}

int main(){
    //Step 1 reading input
    input line; // the line to parse and save for the input table
    vector<input> inputTable; // where we will be storing the input and creating our input table

    while(cin >> line.operation){
        cin >> line.parameter;
        inputTable.push_back(line);
    }
    //This is for checking the ouputs
    for(int i=0; i < inputTable.size(); i++){
        //cout << i << " ProcessType: " << inputTable[i].operation << " Process Parameter: " << inputTable[i].parameter << endl;
    }
    // Everything looks in order
    

    //Step 2 creating a process table
    // We need to figure out the processID, startLine, endLine, and so on
    // We will create a vector of type processInfo(The user defined struct) and that will act as our process table
    vector<processInfo> processTable;
    int processToken = -1; //This token keeps track of which process is which
    processInfo currentProcessBeingRead; // this is where we will store the current process being read for the processTable
    for(int i=1; i < inputTable.size(); i++){
        if(inputTable[i].operation == "START"){ // Finding where the process starts and ends
            processToken++; // we increment the token to 0 for the first process
            currentProcessBeingRead.processID = processToken; // load in the current attributes for the process
            currentProcessBeingRead.firstLine = i;
            currentProcessBeingRead.currentLine = 0;
            currentProcessBeingRead.state = 0;
            processTable.push_back(currentProcessBeingRead); 
            if(i>1){ //handling the case of getting the last line
                //We go to the previous process lastLine and subtract 1 from the current line 
                processTable[processToken-1].lastLine = i-1;
            }
        }
    }
    processTable[processTable.size()-1].lastLine = inputTable.size()-1; // add the last line for the final process of the table
    runningProcess = processToken;
    //testing to make sure the output looks good
    //cout << "processID FirstLine LastLine Current Line State" << endl;
    // for(int i = 0; i < processTable.size(); i++){
    //     //cout << processTable[i].processID << " " << processTable[i].firstLine
    //     << " " << processTable[i].lastLine << " " << processTable[i].currentLine << " " << processTable[i].state <<endl;
    // }

    //Step 2 is complete

    //Step 3 creation of a process
    
    //Step 3 was completed (creation of the process struct)


    //Step 4 Testing the main queue
    
    for (int i = 0; i < processTable.size(); ++i) {
        process newProcess;
        newProcess.processID = processTable[i].processID; // capturing the id
        newProcess.time = inputTable[processTable[i].firstLine].parameter; // the process arrival time
        newProcess.instruction = inputTable[processTable[i].firstLine].operation; // the process type
        mainPriorityQueue.enqueue(newProcess); // storing it in the main queue
    }

    mainPriorityQueue.display();
    //Step 4 testing is complete
    
    //Step 5 adding the global variables is compelete

    //Step 6 adding the start function, core request function, and testing the while loop
    // cout << "Testing in the main queue while loop" << endl;
    while(!mainPriorityQueue.isEmpty()){
        process savedProcess = mainPriorityQueue.top(); // saving the top of the que
        // for testing purposes
        //cout << "curent information below " << endl;
        mainPriorityQueue.display();
        mainPriorityQueue.dequeue(); // pop the process after saving it
        //cout << "This is the clock time before : " << clockTime << endl;
        clockTime = savedProcess.time; // update the clock time
        //cout << "This is the clock time after : " << clockTime << endl;
        if(savedProcess.instruction == "START"){ 
            // creating the arrival function
            arrival(savedProcess,inputTable,processTable);
            coreRequest(savedProcess,inputTable,processTable);
        }
        else if(savedProcess.instruction == "CORE" && (savedProcess.time <= clockTime)){ // Part 7 adding the core completion event
            coreCompletion(savedProcess,inputTable,processTable);
        }
        else if(((savedProcess.instruction == "WRITE") ||(savedProcess.instruction == "READ")) && (savedProcess.time <= clockTime)){ // Part 7 adding the core completion event
            //cout << "entering the write completion event" << endl;
            ssdCompletion(savedProcess,inputTable,processTable);
        }
        else if(savedProcess.instruction == "DISPLAY" || savedProcess.instruction == "INPUT"){
            processTable[savedProcess.processID].currentLine = processTable[savedProcess.processID].currentLine +1;
            savedProcess.instruction = inputTable[processTable[savedProcess.processID].currentLine].operation;
            savedProcess.time = inputTable[processTable[savedProcess.processID].currentLine].parameter;
            coreRequest(savedProcess,inputTable,processTable);
        }

        
    }
    
    // cout << "processID FirstLine LastLine Current Line State" << endl;
    // for(int i = 0; i < processTable.size(); i++){
    //     cout << processTable[i].processID << " " << processTable[i].firstLine
    //     << " " << processTable[i].lastLine << " " << processTable[i].currentLine << " " << processTable[i].state <<endl;
    // }

    // cout << "This is what is in the ready queue. " << readyQueue.front().processID << " This is its time " << readyQueue.front().time << endl;
    // cout << "This is the clock time: " << clockTime << endl;
    // cout << "This is the CPU status: " << CPU << endl;
    // Step 6 testing is concluded 

    return 0;
}
