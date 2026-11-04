"""Compare saved deterministic measurements; fail on audibility regressions."""
import csv
import math
from pathlib import Path

root = Path(__file__).resolve().parents[1] / 'docs/validation/audibility'
def read(name):
    with (root / name).open() as stream:
        rows = list(csv.DictReader(stream))
    assert len(rows) == 200, '100 presets, two polarities required'
    result = {(r['preset'], r['mode']): r for r in rows}
    assert len(result) == 200, 'duplicate measurement'
    return result

before, after = read('before.csv'), read('after.csv')
assert before.keys() == after.keys()
for key, row in after.items():
    for column in ('wet_gain_db', 'insert_wet_gain_db', 'peak', 'correlation'):
        assert math.isfinite(float(row[column])), (key, column)
    assert float(row['insert_wet_gain_db']) > -35, key
    # Headroom check applies only to this documented 0.1-peak noise stimulus.
    assert 0 < float(row['peak']) < 1, key
    if key[1] == 'mono':
        delta = float(row['wet_gain_db']) - float(before[key]['wet_gain_db'])
        assert abs(delta - 20 * math.log10(3)) < .001, (key, delta)
    else:
        assert float(before[key]['peak']) == 0, key
        mono = after[(key[0], 'mono')]
        assert abs(float(row['wet_gain_db']) - float(mono['wet_gain_db'])) < 1, key
print('PASS: 100 presets, finite mono/side returns, +9.54 dB mono trim, '
      'side energy within 1 dB of mono; stimulus headroom and insert contribution.')
