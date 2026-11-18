# Initial capture interpretation

Analyzed the three labelled R3 files in RepBaselineLogger/captures. Statistics use raw 10-bit ADC values; standard deviation is population standard deviation. analyze_captures.py reproduces these values.

| Recording | Duration | X mean | X standard deviation | Z mean | Z standard deviation |
|---|---:|---:|---:|---:|---:|
| Rest Trial 1 | 13.621 s | 368.37 | 0.95 | 352.40 | 0.86 |
| Good Trial 2, labelled 10 reps | 39.040 s | 467.72 | 6.75 | 142.27 | 0.93 |
| Partial Trial 1, labelled 10 reps | 32.980 s | 476.34 | 1.15 | 142.62 | 0.79 |

The counted good recording contains excursions on X; the partial recording is much closer to sensor noise. The first, uncounted good recording had much stronger repeated X swings. The large mean change between the rest capture and counted recordings prevents directly reusing the original rest mean as a baseline. The cause cannot be identified from these CSVs alone: mounting/orientation, wiring, supply, or handling changes need checking. A2/Y is intentionally excluded from this prototype because of the shield potentiometer conflict.

## Exact detector replay

Compiled tests/rep_tracker/replay.cpp with g++ -std=c++11 -Wall -Wextra and ran built-in detector checks successfully. The replay shares RepDetector.h with the Arduino sketch. It downsamples to intervals of at least 50 ms, estimates a baseline from the first 2 seconds of each file, and uses a provisional full threshold of 15 counts.

| Capture | Full events | Partial events | Rejected events |
|---|---:|---:|---:|
| Rest Trial 1 | 0 | 0 | 0 |
| Good Trial 2, labelled 10 reps | 7 | 0 | 2 |
| Partial Trial 1, labelled 10 reps | 0 | 0 | 1 |

Rejected events include brief departures that return too quickly or movements failing to return within 10 seconds. These results do NOT validate a 10-rep counter or a full/partial classifier. Raising/lowering thresholds to force the observed labels would not establish generalization. Live teaching is required, followed by a fresh counted validation capture in a consistent mounting position.

The prototype deliberately uses a 70%-of-teaching-median amplitude rule as an exploratory range proxy. It cannot determine clinical form correctness. Future validation must include full, partial, rest, and non-rep movements not used for teaching. Arduino compilation, actual sample timing, and screen operation have not been verified on hardware here.

## November 9 update: confirmed 8-rep R4 capture

The combined leg-raises-20261109-204841-037.csv has 1035 rest samples over 20.68 seconds (trial 2) and 1761 good-rep samples over 35.20 seconds (trial 3). User confirmed 8 actual reps after a signal-based estimate of 8. The strongest repeated signal is now Y/A1; X contains rail-like outliers and is not used. Rest Y and initial good-trial Y differ, so stable placement remains important.

The Y-axis profile in `RepCounter.h` uses thresholds of 1400 for movement and 1550 for return, in R4 counts. The first two seconds of the exercise trial have a Y median of 1646.5. The separate rest trials have lower medians (about 1421 and 1398), so the arming threshold uses the starting pose during the exercise trial. Replay detects zero reps in both rest trials and eight in the exercise trial. Count times in seconds are 8.34, 12.36, 16.50, 20.10, 24.90, 27.96, 31.08, and 35.10.

## November 18 update — bicep curls

The [curl capture](../RepBaselineLogger/captures/bicep-curls-20261118-161457-348.csv) has 1566 samples over 31.30 seconds. The Z-axis median during the first three seconds is 1138 in R4 counts. Median readings near the tops of the sustained lifts range from about 1627 to 1671. The curl profile arms and counts a return at or below 1300, and confirms a lift at or above 1500 for at least 450 ms. The movement dwell rejects a brief spike near 3.7 seconds.

Replay detects seven completed curls at 7.86, 11.52, 14.70, 17.64, 20.52, 23.82, and 27.30 seconds. The eighth lift stays up through the end of the capture. These recordings informed the thresholds. Independently counted sets are still needed to validate accuracy.

## November 18 update — lat raises

The [lat-raise capture](../RepBaselineLogger/captures/lat-raises-20261118-162832-960.csv) has 2267 samples over 45.32 seconds. Several large spikes appear on both Y and Z. Subtracting Y from Z reduces the shared spikes and shows twelve clear excursions. The median difference during the first three seconds is −160.5 R4 counts. The profile confirms a raise when the difference is at least 130 for 250 ms and counts a rep when it returns to −100 or below for 100 ms.

Replay detects twelve returns at 6.54, 10.32, 14.46, 17.70, 21.24, 24.42, 27.90, 31.20, 34.86, 37.92, 41.28, and 45.00 seconds. This capture was used to select the thresholds.
