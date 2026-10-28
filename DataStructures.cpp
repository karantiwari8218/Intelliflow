#include "DataStructures.h"

LaneQueue::LaneQueue() : head(NULL), tail(NULL), count(0) {
}

LaneQueue::~LaneQueue() {
    clear();
}

void LaneQueue::enqueue(Vehicle* v) {
    if (!v) return;
    QueueNode* node = new QueueNode;
    node->vehicle = v;
    node->next = NULL;

    if (!tail) {
        head = tail = node;
    } else {
        tail->next = node;
        tail = node;
    }
    count++;
}

Vehicle* LaneQueue::dequeue() {
    if (!head) return NULL;
    QueueNode* temp = head;
    Vehicle* v = temp->vehicle;
    head = head->next;
    if (!head) {
        tail = NULL;
    }
    delete temp;
    count--;
    return v;
}

Vehicle* LaneQueue::peek() const {
    if (!head) return NULL;
    return head->vehicle;
}

bool LaneQueue::isEmpty() const {
    return count == 0;
}

int LaneQueue::getSize() const {
    return count;
}

Vehicle* LaneQueue::getAt(int index) const {
    if (index < 0 || index >= count) return NULL;
    QueueNode* curr = head;
    for (int i = 0; i < index && curr; ++i) {
        curr = curr->next;
    }
    return curr ? curr->vehicle : NULL;
}

void LaneQueue::clear() {
    while (head) {
        QueueNode* temp = head;
        head = head->next;
        delete temp;
    }
    tail = NULL;
    count = 0;
}

EmergencyPriorityQueue::EmergencyPriorityQueue(int initCap)
    : capacity(initCap), count(0) {
    if (capacity < 4) capacity = 4;
    heap = new Vehicle*[capacity];
}

EmergencyPriorityQueue::~EmergencyPriorityQueue() {
    delete[] heap;
}

void EmergencyPriorityQueue::swapNodes(int i, int j) {
    Vehicle* temp = heap[i];
    heap[i] = heap[j];
    heap[j] = temp;
}

void EmergencyPriorityQueue::resize() {
    int newCap = capacity * 2;
    Vehicle** newHeap = new Vehicle*[newCap];
    for (int i = 0; i < count; ++i) {
        newHeap[i] = heap[i];
    }
    delete[] heap;
    heap = newHeap;
    capacity = newCap;
}

void EmergencyPriorityQueue::siftUp(int index) {
    while (index > 0) {
        int parent = (index - 1) / 2;
        if (heap[index]->getPriority() > heap[parent]->getPriority()) {
            swapNodes(index, parent);
            index = parent;
        } else {
            break;
        }
    }
}

void EmergencyPriorityQueue::siftDown(int index) {
    while ((2 * index + 1) < count) {
        int left = 2 * index + 1;
        int right = 2 * index + 2;
        int best = index;

        if (heap[left]->getPriority() > heap[best]->getPriority()) {
            best = left;
        }
        if (right < count && heap[right]->getPriority() > heap[best]->getPriority()) {
            best = right;
        }

        if (best != index) {
            swapNodes(index, best);
            index = best;
        } else {
            break;
        }
    }
}

void EmergencyPriorityQueue::push(Vehicle* v) {
    if (!v) return;
    if (count >= capacity) {
        resize();
    }
    heap[count] = v;
    siftUp(count);
    count++;
}

Vehicle* EmergencyPriorityQueue::pop() {
    if (count == 0) return NULL;
    Vehicle* top = heap[0];
    count--;
    if (count > 0) {
        heap[0] = heap[count];
        siftDown(0);
    }
    return top;
}

Vehicle* EmergencyPriorityQueue::peek() const {
    if (count == 0) return NULL;
    return heap[0];
}

bool EmergencyPriorityQueue::isEmpty() const {
    return count == 0;
}

int EmergencyPriorityQueue::getSize() const {
    return count;
}

bool EmergencyPriorityQueue::contains(Vehicle* v) const {
    for (int i = 0; i < count; ++i) {
        if (heap[i] == v) return true;
    }
    return false;
}

bool EmergencyPriorityQueue::remove(Vehicle* v) {
    int idx = -1;
    for (int i = 0; i < count; ++i) {
        if (heap[i] == v) {
            idx = i;
            break;
        }
    }
    if (idx == -1) return false;

    count--;
    if (idx == count) {
        return true;
    }

    heap[idx] = heap[count];
    siftDown(idx);
    siftUp(idx);
    return true;
}

Vehicle* EmergencyPriorityQueue::getAt(int index) const {
    if (index < 0 || index >= count) return NULL;
    return heap[index];
}

void EmergencyPriorityQueue::clear() {
    count = 0;
}

ActiveVehicleList::ActiveVehicleList(int initCap)
    : capacity(initCap), count(0) {
    if (capacity < 8) capacity = 8;
    items = new Vehicle*[capacity];
}

ActiveVehicleList::~ActiveVehicleList() {
    delete[] items;
}

void ActiveVehicleList::resize() {
    int newCap = capacity * 2;
    Vehicle** newItems = new Vehicle*[newCap];
    for (int i = 0; i < count; ++i) {
        newItems[i] = items[i];
    }
    delete[] items;
    items = newItems;
    capacity = newCap;
}

void ActiveVehicleList::add(Vehicle* v) {
    if (!v) return;
    if (count >= capacity) {
        resize();
    }
    items[count++] = v;
}

bool ActiveVehicleList::remove(Vehicle* v) {
    int idx = -1;
    for (int i = 0; i < count; ++i) {
        if (items[i] == v) {
            idx = i;
            break;
        }
    }
    if (idx == -1) return false;

    for (int i = idx; i < count - 1; ++i) {
        items[i] = items[i + 1];
    }
    count--;
    return true;
}

Vehicle* ActiveVehicleList::get(int index) const {
    if (index < 0 || index >= count) return NULL;
    return items[index];
}

int ActiveVehicleList::getSize() const {
    return count;
}

bool ActiveVehicleList::isEmpty() const {
    return count == 0;
}

void ActiveVehicleList::clear() {
    count = 0;
}

Vehicle* ActiveVehicleList::operator[](int index) const {
    return get(index);
}

