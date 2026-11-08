"""Report raw R3 statistics with only the Python standard library."""
import csv
from pathlib import Path
from statistics import mean, pstdev

root = Path(__file__).resolve().parent.parent / 'RepBaselineLogger' / 'captures'
for name in ('R3 Rest Position Trial 1.csv', 'R3 Good Reps Trial 2 - 10 reps.csv',
             'R3 Partial Reps Trial 1 - 10 reps.csv'):
    rows = list(csv.DictReader((root / name).open(encoding='utf-8-sig')))
    print(name, 'samples:', len(rows), 'duration_s:',
          (int(rows[-1]['t_ms']) - int(rows[0]['t_ms'])) / 1000)
    for key in ('x_raw', 'y_raw', 'z_raw'):
        values = [int(row[key]) for row in rows]
        print(key, 'min:', min(values), 'max:', max(values),
              'mean:', round(mean(values), 2), 'std:', round(pstdev(values), 2))
