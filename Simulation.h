#ifndef SIMULATION_H
#define SIMULATION_H

#include <windows.h>
#include "Vehicle.h"
#include "DataStructures.h"
#include "TrafficLight.h"

class Simulation {
private:
    int canvasWidth;
    int canvasHeight;
    int intersectionX;
    int intersectionY;
    int roadWidth;

    LaneQueue laneQueues[4];
    EmergencyPriorityQueue emergencyQueue;
    ActiveVehicleList activeVehicles;
    TrafficLightController trafficController;

    int totalSpawned;
    int totalPassed;
    int emergencyCleared;
    float simTime;
    bool isPaused;
    int nextVehicleId;

    float autoSpawnTimer;
    float emergencySpawnTimer;
    float emergencySafetyTimer;
    int roundRobinDir;

    void updateVehiclePositions(float dt);
    void checkLaneHeadway();
    void checkStopLines();
    void checkEmergencyStatus();
    void checkOvertakeAndLaneChange();
    void processLaneQueues();
    void cleanupDepartedVehicles();
    void handleAutoSpawn(float dt);
    void seedInitialTraffic();

    bool isEntranceClear(Direction dir, int subLane) const;
    float getStopLine(Direction dir) const;
    bool hasCrossedStopLine(const Vehicle* v) const;
    bool isInsideIntersection(const Vehicle* v) const;
    bool isIntersectionClearFor(Direction dir) const;
    float getLaneCenterX(Direction dir, int subLane) const;
    float getLaneCenterY(Direction dir, int subLane) const;

    void drawRoads(HDC hdc, int width, int height);

public:
    Simulation();
    ~Simulation();

    void initialize(int width, int height);
    void update(float dt);
    void draw(HDC hdc, int fullWidth, int fullHeight);

    void spawnVehicle(Direction dir, int subLane, VehicleType type);
    void spawnRandomCar(Direction dir, int subLane);
    void spawnAmbulance(Direction dir, int subLane);
    void spawnFireTruck(Direction dir, int subLane);
    void spawnPolice(Direction dir, int subLane);

    void togglePause();
    void reset();

    bool getIsPaused() const;
    int getTotalSpawned() const;
    int getTotalPassed() const;
    int getEmergencyCleared() const;
    int getActiveCount() const;
    int getTotalVehicleCount() const;

    const TrafficLightController& getTrafficController() const;
    const EmergencyPriorityQueue& getEmergencyQueue() const;
};

#endif
