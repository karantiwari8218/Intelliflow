#include "Simulation.h"
#include <cstdio>
#include <cmath>

Simulation::Simulation()
    : canvasWidth(1000), canvasHeight(800),
      intersectionX(500), intersectionY(400), roadWidth(160),
      totalSpawned(0), totalPassed(0), emergencyCleared(0),
      simTime(0.0f), isPaused(false), nextVehicleId(1),
      autoSpawnTimer(0.0f), emergencySpawnTimer(0.0f),
      emergencySafetyTimer(0.0f), roundRobinDir(0) {
    trafficController.initialize(intersectionX, intersectionY, roadWidth);
    seedInitialTraffic();
}

Simulation::~Simulation() {
    reset();
}

void Simulation::initialize(int width, int height) {
    canvasWidth = width;
    canvasHeight = height;
    intersectionX = width / 2;
    intersectionY = height / 2;
    roadWidth = 160;
    trafficController.initialize(intersectionX, intersectionY, roadWidth);
    reset();
}

int Simulation::getTotalVehicleCount() const {
    int total = activeVehicles.getSize();
    for (int i = 0; i < 4; ++i) {
        total += laneQueues[i].getSize();
    }
    return total;
}

float Simulation::getLaneCenterX(Direction dir, int subLane) const {
    if (dir == DIR_SOUTH) {
        return (subLane == 0) ? (float)(intersectionX - 60) : (float)(intersectionX - 22);
    } else if (dir == DIR_NORTH) {
        return (subLane == 1) ? (float)(intersectionX + 22) : (float)(intersectionX + 60);
    }
    return (float)intersectionX;
}

float Simulation::getLaneCenterY(Direction dir, int subLane) const {
    if (dir == DIR_EAST) {
        return (subLane == 1) ? (float)(intersectionY + 22) : (float)(intersectionY + 60);
    } else if (dir == DIR_WEST) {
        return (subLane == 0) ? (float)(intersectionY - 60) : (float)(intersectionY - 22);
    }
    return (float)intersectionY;
}

float Simulation::getStopLine(Direction dir) const {
    int halfRoad = roadWidth / 2;
    if (dir == DIR_SOUTH) return (float)(intersectionY - halfRoad - 15);
    if (dir == DIR_NORTH) return (float)(intersectionY + halfRoad + 15);
    if (dir == DIR_EAST)  return (float)(intersectionX - halfRoad - 15);
    return (float)(intersectionX + halfRoad + 15);
}

bool Simulation::hasCrossedStopLine(const Vehicle* v) const {
    Direction dir = v->getDirection();
    float stop = getStopLine(dir);
    if (dir == DIR_SOUTH) return v->getY() >= stop;
    if (dir == DIR_NORTH) return v->getY() <= stop;
    if (dir == DIR_EAST)  return v->getX() >= stop;
    return v->getX() <= stop;
}

bool Simulation::isInsideIntersection(const Vehicle* v) const {
    if (v->isCleared()) return false;
    int halfRoad = roadWidth / 2;
    float vx = v->getDrawX();
    float vy = v->getDrawY();
    return (vx >= (intersectionX - halfRoad - 10) && vx <= (intersectionX + halfRoad + 10) &&
            vy >= (intersectionY - halfRoad - 10) && vy <= (intersectionY + halfRoad + 10));
}

bool Simulation::isIntersectionClearFor(Direction dir) const {
    for (int i = 0; i < activeVehicles.getSize(); ++i) {
        Vehicle* other = activeVehicles[i];
        if (other->isCleared()) continue;

        Direction od = other->getDirection();
        bool isPerp = false;
        if ((dir == DIR_NORTH || dir == DIR_SOUTH) && (od == DIR_EAST || od == DIR_WEST)) {
            isPerp = true;
        } else if ((dir == DIR_EAST || dir == DIR_WEST) && (od == DIR_NORTH || od == DIR_SOUTH)) {
            isPerp = true;
        }

        if (isPerp && isInsideIntersection(other)) {
            return false;
        }
    }
    return true;
}

bool Simulation::isEntranceClear(Direction dir, int subLane) const {
    float minSafeDist = 75.0f;
    for (int i = 0; i < activeVehicles.getSize(); ++i) {
        Vehicle* v = activeVehicles[i];
        if (v->getDirection() != dir || v->getSubLane() != subLane) continue;

        if (dir == DIR_SOUTH && v->getY() < minSafeDist) return false;
        if (dir == DIR_NORTH && v->getY() > (canvasHeight - minSafeDist)) return false;
        if (dir == DIR_EAST && v->getX() < minSafeDist) return false;
        if (dir == DIR_WEST && v->getX() > (canvasWidth - minSafeDist)) return false;
    }
    return true;
}

void Simulation::spawnVehicle(Direction dir, int subLane, VehicleType type) {
    if (getTotalVehicleCount() >= 20) return;

    Vehicle* v = NULL;
    int vid = nextVehicleId++;

    float startX = 0.0f;
    float startY = 0.0f;

    if (dir == DIR_SOUTH) {
        startX = getLaneCenterX(DIR_SOUTH, subLane);
        startY = -50.0f;
    } else if (dir == DIR_NORTH) {
        startX = getLaneCenterX(DIR_NORTH, subLane);
        startY = (float)(canvasHeight + 50);
    } else if (dir == DIR_EAST) {
        startX = -50.0f;
        startY = getLaneCenterY(DIR_EAST, subLane);
    } else if (dir == DIR_WEST) {
        startX = (float)(canvasWidth + 50);
        startY = getLaneCenterY(DIR_WEST, subLane);
    }

    if (type == TYPE_CAR) {
        COLORREF colors[] = { RGB(50, 120, 215), RGB(200, 50, 50), RGB(50, 170, 75), RGB(180, 180, 190), RGB(230, 160, 30) };
        COLORREF chosen = colors[vid % 5];
        float spdVar = (float)((vid * 7) % 30);
        v = new Car(vid, dir, subLane, startX, startY, chosen, spdVar);
    } else if (type == TYPE_BUS) {
        float spdVar = (float)((vid * 5) % 18);
        v = new Bus(vid, dir, subLane, startX, startY, spdVar);
    } else if (type == TYPE_AMBULANCE) {
        v = new Ambulance(vid, dir, subLane, startX, startY);
    } else if (type == TYPE_FIRE_TRUCK) {
        v = new FireTruck(vid, dir, subLane, startX, startY);
    } else if (type == TYPE_POLICE) {
        v = new PoliceCar(vid, dir, subLane, startX, startY);
    }

    if (v) {
        totalSpawned++;
        if (v->isEmergency()) {
            emergencyQueue.push(v);
        }
        laneQueues[dir].enqueue(v);
    }
}

void Simulation::spawnRandomCar(Direction dir, int subLane) {
    if (rand() % 4 == 0) {
        spawnVehicle(dir, subLane, TYPE_BUS);
    } else {
        spawnVehicle(dir, subLane, TYPE_CAR);
    }
}

void Simulation::spawnAmbulance(Direction dir, int subLane) {
    spawnVehicle(dir, subLane, TYPE_AMBULANCE);
}

void Simulation::spawnFireTruck(Direction dir, int subLane) {
    spawnVehicle(dir, subLane, TYPE_FIRE_TRUCK);
}

void Simulation::spawnPolice(Direction dir, int subLane) {
    spawnVehicle(dir, subLane, TYPE_POLICE);
}

void Simulation::seedInitialTraffic() {
    float s0 = getLaneCenterX(DIR_SOUTH, 0);
    float s1 = getLaneCenterX(DIR_SOUTH, 1);
    Vehicle* v1 = new Car(nextVehicleId++, DIR_SOUTH, 0, s0, 230.0f, RGB(50, 120, 215), 15.0f);
    Vehicle* v2 = new Car(nextVehicleId++, DIR_SOUTH, 0, s0, 130.0f, RGB(200, 50, 50), 22.0f);
    Vehicle* v3 = new Ambulance(nextVehicleId++, DIR_SOUTH, 1, s1, 60.0f);
    activeVehicles.add(v1);
    activeVehicles.add(v2);
    activeVehicles.add(v3);
    emergencyQueue.push(v3);
    spawnRandomCar(DIR_SOUTH, 0);

    float n0 = getLaneCenterX(DIR_NORTH, 0);
    float n1 = getLaneCenterX(DIR_NORTH, 1);
    Vehicle* v4 = new Car(nextVehicleId++, DIR_NORTH, 0, n0, 570.0f, RGB(50, 170, 75), 18.0f);
    Vehicle* v5 = new Bus(nextVehicleId++, DIR_NORTH, 0, n0, 680.0f, 10.0f);
    Vehicle* v6 = new PoliceCar(nextVehicleId++, DIR_NORTH, 1, n1, 720.0f);
    activeVehicles.add(v4);
    activeVehicles.add(v5);
    activeVehicles.add(v6);
    emergencyQueue.push(v6);
    spawnRandomCar(DIR_NORTH, 1);

    float e0 = getLaneCenterY(DIR_EAST, 0);
    float e1 = getLaneCenterY(DIR_EAST, 1);
    Vehicle* v7 = new Car(nextVehicleId++, DIR_EAST, 0, 230.0f, e0, RGB(180, 180, 190), 12.0f);
    Vehicle* v8 = new Car(nextVehicleId++, DIR_EAST, 0, 130.0f, e0, RGB(230, 160, 30), 25.0f);
    Vehicle* v9 = new FireTruck(nextVehicleId++, DIR_EAST, 1, 60.0f, e1);
    activeVehicles.add(v7);
    activeVehicles.add(v8);
    activeVehicles.add(v9);
    emergencyQueue.push(v9);
    spawnRandomCar(DIR_EAST, 0);

    float w0 = getLaneCenterY(DIR_WEST, 0);
    float w1 = getLaneCenterY(DIR_WEST, 1);
    Vehicle* v10 = new Car(nextVehicleId++, DIR_WEST, 0, 570.0f, w0, RGB(200, 50, 50), 16.0f);
    Vehicle* v11 = new Car(nextVehicleId++, DIR_WEST, 0, 680.0f, w0, RGB(50, 120, 215), 20.0f);
    Vehicle* v12 = new Car(nextVehicleId++, DIR_WEST, 1, 720.0f, w1, RGB(50, 170, 75), 28.0f);
    activeVehicles.add(v10);
    activeVehicles.add(v11);
    activeVehicles.add(v12);
    spawnAmbulance(DIR_WEST, 1);

    totalSpawned = 16;
}

void Simulation::processLaneQueues() {
    for (int d = 0; d < 4; ++d) {
        Direction dir = (Direction)d;
        if (!laneQueues[dir].isEmpty()) {
            Vehicle* peekV = laneQueues[dir].peek();
            if (peekV && isEntranceClear(dir, peekV->getSubLane())) {
                Vehicle* v = laneQueues[dir].dequeue();
                activeVehicles.add(v);
            }
        }
    }
}

void Simulation::handleAutoSpawn(float dt) {
    int total = getTotalVehicleCount();

    if (total < 20) {
        autoSpawnTimer += dt;
        emergencySpawnTimer += dt;

        float targetInterval = (total < 15) ? 0.35f : 1.3f;

        if (emergencySpawnTimer >= 4.5f && total < 20) {
            emergencySpawnTimer = 0.0f;
            int r = rand() % 3;
            Direction dir = (Direction)(rand() % 4);
            int subLane = 1;
            if (r == 0) spawnAmbulance(dir, subLane);
            else if (r == 1) spawnFireTruck(dir, subLane);
            else spawnPolice(dir, subLane);
            autoSpawnTimer = 0.0f;
        } else if (autoSpawnTimer >= targetInterval) {
            autoSpawnTimer = 0.0f;

            int bestDir = 0;
            int minCount = 999;
            for (int i = 0; i < 4; ++i) {
                int c = laneQueues[i].getSize();
                for (int j = 0; j < activeVehicles.getSize(); ++j) {
                    if (activeVehicles[j]->getDirection() == (Direction)i) c++;
                }
                if (c < minCount) {
                    minCount = c;
                    bestDir = i;
                }
            }

            int subLane = (rand() % 3 == 0) ? 1 : 0;
            spawnRandomCar((Direction)bestDir, subLane);
        }
    }
}

void Simulation::checkEmergencyStatus() {
    while (!emergencyQueue.isEmpty()) {
        Vehicle* top = emergencyQueue.peek();
        if (top->isCleared()) {
            emergencyQueue.pop();
            emergencyCleared++;
        } else {
            break;
        }
    }

    Vehicle* activeTarget = NULL;
    for (int i = 0; i < emergencyQueue.getSize(); ++i) {
        Vehicle* ev = emergencyQueue.getAt(i);
        if (!ev->isCleared()) {
            bool onRoad = false;
            for (int j = 0; j < activeVehicles.getSize(); ++j) {
                if (activeVehicles[j] == ev) {
                    onRoad = true;
                    break;
                }
            }
            if (onRoad) {
                activeTarget = ev;
                break;
            }
        }
    }

    if (activeTarget) {
        trafficController.triggerEmergency(activeTarget->getDirection());
        emergencySafetyTimer += 0.016f;
        if (emergencySafetyTimer > 8.0f) {
            emergencySafetyTimer = 0.0f;
            trafficController.clearEmergency();
        }
    } else {
        emergencySafetyTimer = 0.0f;
        if (trafficController.isEmergencyActive()) {
            trafficController.clearEmergency();
        }
    }
}

void Simulation::checkOvertakeAndLaneChange() {
    for (int i = 0; i < activeVehicles.getSize(); ++i) {
        Vehicle* curr = activeVehicles[i];
        if (curr->isChangingLane() || hasCrossedStopLine(curr)) continue;

        Direction dir = curr->getDirection();
        int curSub = curr->getSubLane();

        if (!curr->isEmergency()) {
            Vehicle* evBehind = NULL;
            float minEvDist = 999.0f;
            for (int j = 0; j < activeVehicles.getSize(); ++j) {
                Vehicle* ev = activeVehicles[j];
                if (!ev->isEmergency() || ev->getDirection() != dir || ev->isCleared()) continue;

                float dist = -1.0f;
                if (dir == DIR_SOUTH && ev->getY() < curr->getY()) dist = curr->getY() - ev->getY();
                else if (dir == DIR_NORTH && ev->getY() > curr->getY()) dist = ev->getY() - curr->getY();
                else if (dir == DIR_EAST && ev->getX() < curr->getX()) dist = curr->getX() - ev->getX();
                else if (dir == DIR_WEST && ev->getX() > curr->getX()) dist = ev->getX() - curr->getX();

                if (dist > 0.0f && dist < 210.0f && dist < minEvDist) {
                    minEvDist = dist;
                    evBehind = ev;
                }
            }

            if (evBehind) {
                if (curSub == 1 && curr->getLaneChangeCooldown() <= 0.0f) {
                    Vehicle* neighborIn0 = NULL;
                    float minNeighborGap = 999.0f;
                    for (int j = 0; j < activeVehicles.getSize(); ++j) {
                        Vehicle* nb = activeVehicles[j];
                        if (nb == curr || nb->getDirection() != dir || nb->getSubLane() != 0) continue;

                        float longDist = 999.0f;
                        if (dir == DIR_SOUTH || dir == DIR_NORTH) {
                            longDist = (float)fabs(nb->getY() - curr->getY());
                        } else {
                            longDist = (float)fabs(nb->getX() - curr->getX());
                        }

                        if (longDist < minNeighborGap) {
                            minNeighborGap = longDist;
                            neighborIn0 = nb;
                        }
                    }

                    if (!neighborIn0 || minNeighborGap >= 65.0f) {
                        float targetOffset = 0.0f;
                        if (dir == DIR_NORTH || dir == DIR_SOUTH) {
                            targetOffset = getLaneCenterX(dir, 0) - curr->getX();
                        } else {
                            targetOffset = getLaneCenterY(dir, 0) - curr->getY();
                        }
                        curr->changeToSubLane(0, targetOffset);
                        curr->setYielding(false);
                    } else {
                        float relPos = 0.0f;
                        if (dir == DIR_SOUTH) relPos = curr->getY() - neighborIn0->getY();
                        else if (dir == DIR_NORTH) relPos = neighborIn0->getY() - curr->getY();
                        else if (dir == DIR_EAST)  relPos = curr->getX() - neighborIn0->getX();
                        else if (dir == DIR_WEST)  relPos = neighborIn0->getX() - curr->getX();

                        if (relPos > 0.0f) {
                            curr->setSpeed(curr->getMaxSpeed() * 1.25f);
                        } else {
                            curr->setSpeed(curr->getMaxSpeed() * 0.50f);
                        }
                        curr->setYielding(true);
                    }
                } else if (curSub == 0) {
                    curr->setYielding(true);
                }
            } else {
                curr->setYielding(false);
            }
        } else {
            if (curr->getLaneChangeCooldown() <= 0.0f) {
                float distToLeadCur = 999.0f;
                for (int j = 0; j < activeVehicles.getSize(); ++j) {
                    Vehicle* other = activeVehicles[j];
                    if (other == curr || other->getDirection() != dir || other->getSubLane() != curSub) continue;

                    float dist = -1.0f;
                    if (dir == DIR_SOUTH && other->getY() > curr->getY()) dist = other->getY() - curr->getY();
                    else if (dir == DIR_NORTH && other->getY() < curr->getY()) dist = curr->getY() - other->getY();
                    else if (dir == DIR_EAST && other->getX() > curr->getX()) dist = other->getX() - curr->getX();
                    else if (dir == DIR_WEST && other->getX() < curr->getX()) dist = curr->getX() - other->getX();

                    if (dist > 0.0f && dist < distToLeadCur) {
                        distToLeadCur = dist;
                    }
                }

                if (distToLeadCur < 85.0f) {
                    int targetSub = 1 - curSub;
                    float distToLeadTarget = 999.0f;
                    bool targetClearOfRear = true;

                    for (int j = 0; j < activeVehicles.getSize(); ++j) {
                        Vehicle* other = activeVehicles[j];
                        if (other == curr || other->getDirection() != dir || other->getSubLane() != targetSub) continue;

                        float longDist = 999.0f;
                        if (dir == DIR_SOUTH || dir == DIR_NORTH) {
                            longDist = (float)fabs(other->getY() - curr->getY());
                        } else {
                            longDist = (float)fabs(other->getX() - curr->getX());
                        }

                        if (longDist < 60.0f) {
                            targetClearOfRear = false;
                            break;
                        }

                        float forwardDist = -1.0f;
                        if (dir == DIR_SOUTH && other->getY() > curr->getY()) forwardDist = other->getY() - curr->getY();
                        else if (dir == DIR_NORTH && other->getY() < curr->getY()) forwardDist = curr->getY() - other->getY();
                        else if (dir == DIR_EAST && other->getX() > curr->getX()) forwardDist = other->getX() - curr->getX();
                        else if (dir == DIR_WEST && other->getX() < curr->getX()) forwardDist = curr->getX() - other->getX();

                        if (forwardDist > 0.0f && forwardDist < distToLeadTarget) {
                            distToLeadTarget = forwardDist;
                        }
                    }

                    if (targetClearOfRear && distToLeadTarget > (distToLeadCur + 80.0f)) {
                        float targetOffset = 0.0f;
                        if (dir == DIR_NORTH || dir == DIR_SOUTH) {
                            targetOffset = getLaneCenterX(dir, targetSub) - curr->getX();
                        } else {
                            targetOffset = getLaneCenterY(dir, targetSub) - curr->getY();
                        }
                        curr->changeToSubLane(targetSub, targetOffset);
                    }
                }
            }
        }
    }
}

void Simulation::checkStopLines() {
    for (int i = 0; i < activeVehicles.getSize(); ++i) {
        Vehicle* v = activeVehicles[i];
        Direction dir = v->getDirection();
        LightColor light = trafficController.getLightColor(dir);
        float stopLine = getStopLine(dir);
        bool crossed = hasCrossedStopLine(v);

        if (!crossed) {
            float dist = 0.0f;
            if (dir == DIR_SOUTH) dist = stopLine - v->getY();
            else if (dir == DIR_NORTH) dist = v->getY() - stopLine;
            else if (dir == DIR_EAST)  dist = stopLine - v->getX();
            else if (dir == DIR_WEST)  dist = v->getX() - stopLine;

            if (dist > 0.0f && dist < 85.0f) {
                if (light == LIGHT_RED) {
                    v->setStopped(true);
                } else if (light == LIGHT_YELLOW) {
                    if (dist > 35.0f) {
                        v->setStopped(true);
                    } else {
                        v->setStopped(!isIntersectionClearFor(dir));
                    }
                } else {
                    if (dist < 30.0f && !isIntersectionClearFor(dir)) {
                        v->setStopped(true);
                    } else {
                        v->setStopped(false);
                    }
                }
            } else {
                v->setStopped(false);
            }
        } else {
            v->setStopped(false);

            int halfRoad = roadWidth / 2;
            if (!v->isCleared()) {
                if (dir == DIR_SOUTH && v->getY() > (intersectionY + halfRoad)) {
                    v->setCleared(true);
                } else if (dir == DIR_NORTH && v->getY() < (intersectionY - halfRoad)) {
                    v->setCleared(true);
                } else if (dir == DIR_EAST && v->getX() > (intersectionX + halfRoad)) {
                    v->setCleared(true);
                } else if (dir == DIR_WEST && v->getX() < (intersectionX - halfRoad)) {
                    v->setCleared(true);
                }
            }
        }
    }
}

void Simulation::checkLaneHeadway() {
    for (int i = 0; i < activeVehicles.getSize(); ++i) {
        Vehicle* curr = activeVehicles[i];
        Direction dir = curr->getDirection();
        int sub = curr->getSubLane();

        Vehicle* lead = NULL;
        float minGap = 9999.0f;

        for (int j = 0; j < activeVehicles.getSize(); ++j) {
            if (i == j) continue;
            Vehicle* other = activeVehicles[j];
            if (other->getDirection() != dir || other->getSubLane() != sub) continue;

            float gap = -1.0f;
            if (dir == DIR_SOUTH && other->getY() > curr->getY()) {
                gap = other->getY() - curr->getY();
            } else if (dir == DIR_NORTH && other->getY() < curr->getY()) {
                gap = curr->getY() - other->getY();
            } else if (dir == DIR_EAST && other->getX() > curr->getX()) {
                gap = other->getX() - curr->getX();
            } else if (dir == DIR_WEST && other->getX() < curr->getX()) {
                gap = curr->getX() - other->getX();
            }

            if (gap > 0.0f && gap < minGap) {
                minGap = gap;
                lead = other;
            }
        }

        if (lead) {
            float safeDist = (float)(curr->getHeight() > curr->getWidth() ? curr->getHeight() : curr->getWidth()) + 20.0f;
            if (minGap < safeDist) {
                curr->setStopped(true);
                if (lead->getSpeed() < curr->getSpeed()) {
                    curr->setSpeed(lead->getSpeed());
                }
            }
        }
    }
}

void Simulation::updateVehiclePositions(float dt) {
    for (int i = 0; i < activeVehicles.getSize(); ++i) {
        activeVehicles[i]->update(dt);
    }
}

void Simulation::cleanupDepartedVehicles() {
    for (int i = activeVehicles.getSize() - 1; i >= 0; --i) {
        Vehicle* v = activeVehicles[i];
        bool departed = false;

        if (v->getDirection() == DIR_SOUTH && v->getY() > (canvasHeight + 60)) departed = true;
        else if (v->getDirection() == DIR_NORTH && v->getY() < -60) departed = true;
        else if (v->getDirection() == DIR_EAST && v->getX() > (canvasWidth + 60)) departed = true;
        else if (v->getDirection() == DIR_WEST && v->getX() < -60) departed = true;

        if (departed) {
            totalPassed++;
            activeVehicles.remove(v);
            delete v;
        }
    }
}

void Simulation::update(float dt) {
    if (isPaused) return;

    simTime += dt;
    handleAutoSpawn(dt);
    processLaneQueues();
    checkOvertakeAndLaneChange();
    checkEmergencyStatus();
    trafficController.update(dt);
    checkStopLines();
    checkLaneHeadway();
    updateVehiclePositions(dt);
    cleanupDepartedVehicles();
}

void Simulation::togglePause() {
    isPaused = !isPaused;
}

void Simulation::reset() {
    for (int i = 0; i < activeVehicles.getSize(); ++i) {
        delete activeVehicles[i];
    }
    activeVehicles.clear();

    for (int d = 0; d < 4; ++d) {
        while (!laneQueues[d].isEmpty()) {
            delete laneQueues[d].dequeue();
        }
    }

    emergencyQueue.clear();
    trafficController.clearEmergency();
    totalSpawned = 0;
    totalPassed = 0;
    emergencyCleared = 0;
    simTime = 0.0f;
    isPaused = false;
    nextVehicleId = 1;
    autoSpawnTimer = 0.0f;
    emergencySpawnTimer = 0.0f;
    emergencySafetyTimer = 0.0f;
    roundRobinDir = 0;

    seedInitialTraffic();
}

bool Simulation::getIsPaused() const {
    return isPaused;
}

int Simulation::getTotalSpawned() const {
    return totalSpawned;
}

int Simulation::getTotalPassed() const {
    return totalPassed;
}

int Simulation::getEmergencyCleared() const {
    return emergencyCleared;
}

int Simulation::getActiveCount() const {
    return activeVehicles.getSize();
}

const TrafficLightController& Simulation::getTrafficController() const {
    return trafficController;
}

const EmergencyPriorityQueue& Simulation::getEmergencyQueue() const {
    return emergencyQueue;
}

void Simulation::drawRoads(HDC hdc, int width, int height) {
    HBRUSH grassBrush = CreateSolidBrush(RGB(32, 48, 36));
    RECT bgRect = { 0, 0, width, height };
    FillRect(hdc, &bgRect, grassBrush);
    DeleteObject(grassBrush);

    int halfRoad = roadWidth / 2;
    int x1 = intersectionX - halfRoad;
    int x2 = intersectionX + halfRoad;
    int y1 = intersectionY - halfRoad;
    int y2 = intersectionY + halfRoad;

    int sw = 12;
    HBRUSH sidewalkBrush = CreateSolidBrush(RGB(170, 175, 180));
    RECT sw1 = { x1 - sw, 0, x1, y1 - sw };
    RECT sw2 = { x2, 0, x2 + sw, y1 - sw };
    RECT sw3 = { x1 - sw, y2 + sw, x1, height };
    RECT sw4 = { x2, y2 + sw, x2 + sw, height };
    RECT sw5 = { 0, y1 - sw, x1 - sw, y1 };
    RECT sw6 = { 0, y2, x1 - sw, y2 + sw };
    RECT sw7 = { x2 + sw, y1 - sw, width, y1 };
    RECT sw8 = { x2 + sw, y2, width, y2 + sw };

    FillRect(hdc, &sw1, sidewalkBrush);
    FillRect(hdc, &sw2, sidewalkBrush);
    FillRect(hdc, &sw3, sidewalkBrush);
    FillRect(hdc, &sw4, sidewalkBrush);
    FillRect(hdc, &sw5, sidewalkBrush);
    FillRect(hdc, &sw6, sidewalkBrush);
    FillRect(hdc, &sw7, sidewalkBrush);
    FillRect(hdc, &sw8, sidewalkBrush);
    DeleteObject(sidewalkBrush);

    HBRUSH roadBrush = CreateSolidBrush(RGB(42, 44, 50));
    RECT vertRoad = { x1, 0, x2, height };
    RECT horizRoad = { 0, y1, width, y2 };
    FillRect(hdc, &vertRoad, roadBrush);
    FillRect(hdc, &horizRoad, roadBrush);
    DeleteObject(roadBrush);

    HBRUSH zebraBrush = CreateSolidBrush(RGB(240, 240, 245));
    for (int i = 0; i < 7; ++i) {
        int barW = 12;
        int step = 22;
        RECT zNorth = { x1 + 6 + i * step, y1 - 24, x1 + 6 + i * step + barW, y1 - 6 };
        RECT zSouth = { x1 + 6 + i * step, y2 + 6, x1 + 6 + i * step + barW, y2 + 24 };
        FillRect(hdc, &zNorth, zebraBrush);
        FillRect(hdc, &zSouth, zebraBrush);
    }
    for (int i = 0; i < 7; ++i) {
        int barH = 12;
        int step = 22;
        RECT zWest = { x1 - 24, y1 + 6 + i * step, x1 - 6, y1 + 6 + i * step + barH };
        RECT zEast = { x2 + 6, y1 + 6 + i * step, x2 + 24, y1 + 6 + i * step + barH };
        FillRect(hdc, &zWest, zebraBrush);
        FillRect(hdc, &zEast, zebraBrush);
    }
    DeleteObject(zebraBrush);

    HPEN stopPen = CreatePen(PS_SOLID, 4, RGB(250, 250, 250));
    HGDIOBJ oldPen = SelectObject(hdc, stopPen);
    MoveToEx(hdc, x1, y1 - 28, NULL);
    LineTo(hdc, intersectionX, y1 - 28);
    MoveToEx(hdc, intersectionX, y2 + 28, NULL);
    LineTo(hdc, x2, y2 + 28);
    MoveToEx(hdc, x1 - 28, intersectionY, NULL);
    LineTo(hdc, x1 - 28, y2);
    MoveToEx(hdc, x2 + 28, y1, NULL);
    LineTo(hdc, x2 + 28, intersectionY);

    HBRUSH medianBrush = CreateSolidBrush(RGB(65, 95, 65));
    HPEN medianPen = CreatePen(PS_SOLID, 1, RGB(180, 180, 185));
    SelectObject(hdc, medianBrush);
    SelectObject(hdc, medianPen);

    Rectangle(hdc, intersectionX - 4, 0, intersectionX + 4, y1 - 32);
    Rectangle(hdc, intersectionX - 4, y2 + 32, intersectionX + 4, height);
    Rectangle(hdc, 0, intersectionY - 4, x1 - 32, intersectionY + 4);
    Rectangle(hdc, x2 + 32, intersectionY - 4, width, intersectionY + 4);
    DeleteObject(medianBrush);
    DeleteObject(medianPen);

    HPEN laneDash = CreatePen(PS_DASH, 1, RGB(230, 230, 235));
    SelectObject(hdc, laneDash);

    int sLane0X = (int)getLaneCenterX(DIR_SOUTH, 0) + 19;
    int sLane1X = (int)getLaneCenterX(DIR_NORTH, 0) - 19;
    MoveToEx(hdc, sLane0X, 0, NULL);
    LineTo(hdc, sLane0X, y1 - 32);
    MoveToEx(hdc, sLane0X, y2 + 32, NULL);
    LineTo(hdc, sLane0X, height);
    MoveToEx(hdc, sLane1X, 0, NULL);
    LineTo(hdc, sLane1X, y1 - 32);
    MoveToEx(hdc, sLane1X, y2 + 32, NULL);
    LineTo(hdc, sLane1X, height);

    int eLane0Y = (int)getLaneCenterY(DIR_EAST, 0) - 19;
    int eLane1Y = (int)getLaneCenterY(DIR_WEST, 0) + 19;
    MoveToEx(hdc, 0, eLane0Y, NULL);
    LineTo(hdc, x1 - 32, eLane0Y);
    MoveToEx(hdc, x2 + 32, eLane0Y, NULL);
    LineTo(hdc, width, eLane0Y);
    MoveToEx(hdc, 0, eLane1Y, NULL);
    LineTo(hdc, x1 - 32, eLane1Y);
    MoveToEx(hdc, x2 + 32, eLane1Y, NULL);
    LineTo(hdc, width, eLane1Y);

    SelectObject(hdc, oldPen);
    DeleteObject(stopPen);
    DeleteObject(laneDash);
}

void Simulation::draw(HDC hdc, int fullWidth, int fullHeight) {
    drawRoads(hdc, fullWidth, fullHeight);
    trafficController.draw(hdc);

    for (int i = 0; i < activeVehicles.getSize(); ++i) {
        activeVehicles[i]->draw(hdc);
    }
}
