#include "../../RepTracker/RepCounter.h"
#include <cassert>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <map>
#include <vector>
void checkLegRaises() {
  RepCounter counter;
  uint32_t time = 0;
  auto hold = [&](float y, int samples) {
    for (int i = 0; i < samples; ++i, time += 50) {
      counter.update(y, time, LEG_RAISE_PROFILE);
    }
  };
  hold(340, 40);
  assert(counter.reps == 0);
  hold(410, 20);
  hold(320, 20);
  assert(counter.reps == 0);
  hold(410, 70);
  assert(counter.reps == 1);
  hold(370, 20);
  hold(410, 20);
  assert(counter.reps == 1);
  hold(320, 220);
  hold(410, 20);
  assert(counter.reps == 1);
  counter.reset();
  hold(410, 20);
  hold(0, 1);
  hold(410, 20);
  assert(counter.reps == 0);
}

void checkBicepCurls() {
  RepCounter counter;
  uint32_t time = 0;
  auto hold = [&](float z, int samples) {
    for (int i = 0; i < samples; ++i, time += 50) {
      counter.update(z, time, BICEP_CURL_PROFILE);
    }
  };
  hold(285, 20);
  hold(540, 6); // A 300 ms spike like the one before the curl set.
  hold(285, 20);
  assert(counter.reps == 0);
  hold(410, 20);
  assert(counter.reps == 0);
  hold(285, 20);
  assert(counter.reps == 1);
  counter.reset();
  hold(410, 20);
  hold(285, 20);
  assert(counter.reps == 0);
}

bool replay(const char* path, const RepProfile& profile, size_t axisColumn,
            size_t referenceColumn = 0) {
  std::ifstream file(path);
  if (!file) return false;
  std::string line;
  std::getline(file, line); // CSV header
  std::map<std::string, RepCounter> counters;
  std::map<std::string, uint32_t> lastSample;
  while (std::getline(file, line)) {
    std::stringstream row(line);
    std::string cell;
    std::vector<std::string> columns;
    while (std::getline(row, cell, ',')) columns.push_back(cell);
    if (columns.size() != 8) continue;
    const std::string trial = columns[0] + "," + columns[1];
    const uint32_t now = std::stoul(columns[3]);
    if (lastSample.count(trial) && now - lastSample[trial] < 50) continue;
    lastSample[trial] = now;
    RepCounter& counter = counters[trial];
    float sample = std::stof(columns[axisColumn]);
    if (referenceColumn != 0) sample -= std::stof(columns[referenceColumn]);
    if (counter.update(sample * 0.25f, now, profile)) {
      std::cout << trial << " rep " << counter.reps << " at " << now << " ms\n";
    }
  }
  for (const auto& trial : counters) {
    std::cout << "Trial " << trial.first << ": " << trial.second.reps << " reps\n";
  }
  return true;
}

int main(int argc, char** argv) {
  checkLegRaises();
  checkBicepCurls();
  std::cout << "Profile counter checks passed\n";
  if (argc > 1 && !replay(argv[1], LEG_RAISE_PROFILE, 5)) return 1;
  if (argc > 2 && !replay(argv[2], BICEP_CURL_PROFILE, 6)) return 1;
  if (argc > 3 && !replay(argv[3], LAT_RAISE_PROFILE, 6, 5)) return 1;
}
