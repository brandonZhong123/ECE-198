# Initial capture interpretation

Analyzed the three labelled R3 files in RepBaselineLogger/captures. Statistics use raw 10-bit ADC values; standard deviation is population standard deviation. analyze_captures.py reproduces these values.

| Recording | Duration | X mean | X standard deviation | Z mean | Z standard deviation |
|---|---:|---:|---:|---:|---:|
| Rest Trial 1 | 13.621 s | 368.37 | 0.95 | 352.40 | 0.86 |
| Good Trial 2, labelled 10 reps | 39.040 s | 467.72 | 6.75 | 142.27 | 0.93 |
| Partial Trial 1, labelled 10 reps | 32.980 s | 476.34 | 1.15 | 142.62 | 0.79 |

The counted good recording contains excursions on X; the partial recording is much closer to sensor noise. The first, uncounted good recording had much stronger repeated X swings. The large mean change between the rest capture and counted recordings prevents directly reusing the original rest mean as a baseline. The cause cannot be identified from these CSVs alone: mounting/orientation, wiring, supply, or handling changes need checking. A2/Y is intentionally excluded from this prototype because of the shield potentiometer conflict.

## Exact detector replay

Compiled replay.cpp with g++ -std=c++11 -Wall -Wextra and ran built-in detector checks successfully. The replay shares RepDetector.h with the Arduino sketch. It downsamples to intervals of at least 50 ms, estimates a baseline from the first 2 seconds of each file, and uses a provisional full threshold of 15 counts.

| Capture | Full events | Partial events | Rejected events |
|---|---:|---:|---:|
| Rest Trial 1 | 0 | 0 | 0 |
| Good Trial 2, labelled 10 reps | 7 | 0 | 2 |
| Partial Trial 1, labelled 10 reps | 0 | 0 | 1 |

Rejected events include brief departures that return too quickly or movements failing to return within 10 seconds. These results do NOT validate a 10-rep counter or a full/partial classifier. Raising/lowering thresholds to force the observed labels would not establish generalization. Live teaching is required, followed by a fresh counted validation capture in a consistent mounting position.

The prototype deliberately uses a 70%-of-teaching-median amplitude rule as an exploratory range proxy. It cannot determine clinical form correctness. Future validation must include full, partial, rest, and non-rep movements not used for teaching. Arduino compilation, actual sample timing, and screen operation have not been verified on hardware here.
