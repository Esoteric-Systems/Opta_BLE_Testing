#include <Arduino.h>
#include <ArduinoBLE.h>

#include "mbed.h"
extern "C" {
  #include "mbed_stats.h"
}

BLEService TestService("fea97fa7-d95a-7d95-ad0d-f2e900000000");
BLEIntCharacteristic TestCharacteristic("fea97fa7-d95a-7d95-ad0d-f2e900000146", BLERead | BLEWrite | BLENotify);
int value = 500;
unsigned long lastCharacteristicScan = 0;

// memory printing
void print_memory_stats() {
  mbed_stats_heap_t heap;
  mbed_stats_heap_get(&heap);

  char temp[200];
  sprintf(temp, "Heap Info:\n  Reserved for Heap: %u\n  Total Bytes Ever Allocated: %u\n  Current: %u\n  Allocations: %u\n  Failed: %u\n  ", heap.reserved_size, heap.current_size, heap.total_size, heap.alloc_cnt, heap.alloc_fail_cnt);
  Serial.println(temp);
}

// BLE connect callback
void onBLEConnect(BLEDevice central) {
  Serial.print("Connected to Central: ");
  Serial.println(central.address());
  print_memory_stats();
}

// BLE disconnect callback
void onBLEDisconnect(BLEDevice central) {
  Serial.print("Disconnected from Central: ");
  Serial.println(central.address());
  print_memory_stats();
}

// function to encode data into advertisement data
void setAdvertisement() {
  BLEAdvertisingData BLEAdvertisement;
  BLEAdvertisement.clear();
  BLEAdvertisement.setAdvertisedService(TestService);
  BLE.setAdvertisingData(BLEAdvertisement);
  if (!BLE.advertise()) {
    Serial.println("Failed to advertise");
  }
}

// initialize services and their characteristics
void initializeServicesandCharacteristics() {
  TestService.addCharacteristic(TestCharacteristic);
  BLE.addService(TestService);

  TestCharacteristic.writeValue(value);
}

void characteristicHandling() {
  if (millis() - lastCharacteristicScan > 1000) {
    if (TestCharacteristic.written()) {
    value = TestCharacteristic.value();
    }
  }
  lastCharacteristicScan = millis();
}

// main bluetooth updates
void handleBLEConnection() {
  // check for events
  BLE.poll();

  characteristicHandling();
}

// initialize BLE functionality
void initializeBLE() {
  // try setting up BLE until successful
  while (!BLE.begin()) {
    Serial.println("Failed to Start BLE");
  };

  // set up services, characteristics, and their values
  initializeServicesandCharacteristics();

  // set up callbacks
  BLE.setEventHandler(BLEConnected, onBLEConnect);
  BLE.setEventHandler(BLEDisconnected, onBLEDisconnect);

  // initialize advertisement data and start advertising
  setAdvertisement();
}

void setup() {
  initializeBLE();
  Serial.println("BLE Initialized");
}

void loop() {
  handleBLEConnection();
}