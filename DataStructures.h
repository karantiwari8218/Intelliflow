#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H

#include "Vehicle.h"

struct QueueNode {
    Vehicle* vehicle;
    QueueNode* next;
};

class LaneQueue {
private:
    QueueNode* head;
    QueueNode* tail;
    int count;

public:
    LaneQueue();
    ~LaneQueue();

    void enqueue(Vehicle* v);
    Vehicle* dequeue();
    Vehicle* peek() const;
    bool isEmpty() const;
    int getSize() const;
    Vehicle* getAt(int index) const;
    void clear();
};

class EmergencyPriorityQueue {
private:
    Vehicle** heap;
    int capacity;
    int count;

    void siftUp(int index);
    void siftDown(int index);
    void swapNodes(int i, int j);
    void resize();

public:
    EmergencyPriorityQueue(int initCap = 32);
    ~EmergencyPriorityQueue();

    void push(Vehicle* v);
    Vehicle* pop();
    Vehicle* peek() const;
    bool isEmpty() const;
    int getSize() const;
    bool contains(Vehicle* v) const;
    bool remove(Vehicle* v);
    Vehicle* getAt(int index) const;
    void clear();
};

class ActiveVehicleList {
private:
    Vehicle** items;
    int capacity;
    int count;

    void resize();

public:
    ActiveVehicleList(int initCap = 64);
    ~ActiveVehicleList();

    void add(Vehicle* v);
    bool remove(Vehicle* v);
    Vehicle* get(int index) const;
    int getSize() const;
    bool isEmpty() const;
    void clear();
    Vehicle* operator[](int index) const;
};

#endif
