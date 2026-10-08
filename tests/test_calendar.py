#!/usr/bin/env python3
"""Calendar regressions plus the exact watch decoder under ASan/UBSan."""
import datetime as dt
import json
import pathlib
import subprocess
import sys
import tempfile
ROOT=pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from build_calendar import Catalog, COLORS
cal=json.loads((ROOT/'artifacts/calendar-days.json').read_text())['days']
catalog=Catalog(next((ROOT/'data/source').glob('*.epub')),json.loads((ROOT/'artifacts/readings.json').read_text()))
# Independently chosen celebrations exercise feast overrides and optional colors.
expected_colors = {
    '2026-10-08':'GREEN', '2026-10-15':'WHITE', '2026-11-29':'PURPLE',
    '2026-04-03':'RED', '2026-03-15':'ROSE', '2026-12-13':'ROSE',
    '2026-11-01':'WHITE', '2026-11-02':'PURPLE', '2026-06-13':'GREEN',
    '2024-04-08':'WHITE', '2024-03-25':'PURPLE', '2026-06-29':'RED',
}
for iso, color in expected_colors.items():
    assert cal[iso][0]['colors'][0] == color, iso
raw=(ROOT/'artifacts/calendar-raw.bin').read_bytes()
report=json.loads((ROOT/'artifacts/calendar-selection.json').read_text())
for index, (iso, celebrations) in enumerate(sorted(cal.items())):
    color=celebrations[0]['colors'][0] if celebrations[0]['colors'] else 'NONE'
    assert color!='NONE' or celebrations[0]['id']=='holy_saturday'
    assert raw[48+index*15+14] == COLORS.index(color), iso
    assert report[index]['date']==iso and report[index]['color']==color

def readings(iso):
    return catalog.resolve(dt.date.fromisoformat(iso),cal[iso][0])[0]
def pages(iso):
    return {r['page'] for r in readings(iso)}
# Dates chosen independently for season transitions, cycles and precedence.
assert pages('2026-10-08')=={1549}
assert cal['2026-10-07'][0]['id']=='our_lady_of_the_rosary'
assert pages('2026-10-07')=={1547} # Memorial retains weekday readings.
assert cal['2026-10-04'][0]['id']=='ordinary_time_27_sunday' # Sunday outranks Francis.
assert pages('2026-11-01')=={562} # All Saints outranks Ordinary Sunday.
assert pages('2024-04-08')=={1717} # Annunciation transferred beyond Easter octave.
assert pages('2024-03-25')=={1045} # Holy Monday, not Annunciation.
assert pages('2026-01-06')=={975}
assert pages('2026-01-07')=={982}
assert pages('2026-01-12')!={987} # After the Baptism, Ordinary Time resumes.
assert pages('2026-05-14')=={1104} # Ascension remains Thursday, cycle A.
assert pages('2026-06-04')=={1667} # Corpus Christi remains Thursday, cycle A.
assert cal['2026-11-29'][0]['sundayCycle']=='YEAR_B'
assert cal['2026-11-30'][0]['weekdayCycle']=='YEAR_1'
assert pages('2026-06-11')=={330} # Barnabas has a proper first reading.
assert pages('2026-10-02')=={515} # Guardian Angels has a proper Gospel.
assert pages('2026-05-25')=={1755} # Mary, Mother of the Church.
assert pages('2026-03-29')=={1042}
assert pages('2027-03-21')=={1042,1043}
assert pages('2028-04-09')=={1042,1044}
assert not readings('2026-04-04') # No daytime Mass on Holy Saturday.
assert pages('2026-04-05')=={1055}
assert next(r for r in readings('2026-04-03') if r['kind']==4)['citation']=='Philippians 2: 8–9'
assert {r['kind'] for r in readings('2026-11-02')}=={0,1,2,4,5}
assert 2 not in {r['kind'] for r in readings('2026-08-06')}
assert 2 in {r['kind'] for r in readings('2023-08-06')} # Lord's feast on Sunday.
assert catalog.mass(1241)[0]['citation']=='1 Peter 1: 10–16'
assert next(r for r in catalog.mass(1220) if r['kind']==2)['citation']=='2 Corinthians 1: 18–22'
assert cal['2026-06-13'][0]['rank']=='WEEKDAY'
assert {d['rank'] for d in cal['2026-06-13'][1:]}=={'OPTIONAL_MEMORIAL'}
assert cal['2025-06-28'][0]['rank']=='WEEKDAY'
assert cal['2027-06-05'][0]['rank']=='WEEKDAY'
assert 2 not in {r['kind'] for r in readings('2022-12-30')} # Holy Family on Friday.
assert 2 in {r['kind'] for r in readings('2026-12-27')}
def citation(iso, kind):
    return next(r['citation'] for r in readings(iso) if r['kind']==kind)
assert citation('2026-02-05',0)=='1 Kings 2: 1–4, 10–12'
assert citation('2026-02-26',0)=='Esther C: 12, 14–16, 23–25'
assert citation('2026-07-24',0)=='Jeremiah 3: 14–17'
assert citation('2026-10-11',5)=='Matthew 22: 1–14'
assert citation('2026-05-30',0)=='Jude 17, 20b–25'
assert citation('2026-07-03',5)=='John 20: 24–29'
assert citation('2026-10-28',5)=='Luke 6: 12–16'
assert len(next(r for r in readings('2026-10-28') if r['kind']==5)['refs'])==3
assert citation('2026-04-27',5)=='John 10: 11–18'
assert citation('2026-08-03',5)=='Matthew 14: 22–36'
assert citation('2026-08-04',5)=='Matthew 15: 1–2, 10–14'
assert citation('2026-03-23',5)=='John 8: 1–11'
from supplements import john_8
for iso in ('2022-04-04','2025-04-07','2028-04-03','2031-03-31','2034-03-27','2037-03-23'):
    gospel = next(r for r in readings(iso) if r['kind']==5)
    assert gospel['citation']=='John 8: 12–20'
    assert [catalog.segments[r] for r in gospel['refs']]==john_8()['paragraphs']
# Long-form alternatives end before Or / Short form; no duplicate variant body.
for page in (1042,1043,1044,1051,1055,958):
    for r in catalog.mass(page):
        texts=[catalog.segments[ref&0x7fff] for ref in r['refs']]
        assert not any(t=='Or:' or t.startswith('Short form') for t in texts)
assert len([r for r in catalog.mass(1055) if r['kind']==2])==1
print('Calendar precedence, transfers, cycles, readings and alternative-boundary regressions passed',flush=True)
with tempfile.TemporaryDirectory() as folder:
    folder=pathlib.Path(folder);src=ROOT/'src/c'
    flags=['-std=c99','-g','-O1','-Wall','-Wextra','-Wno-unused-parameter','-fsanitize=address,undefined','-I',str(src)]
    sources=[ROOT/'tests/calendar_test.c',src/'calendar.c',src/'pmr.c']
    sources += [src/'vendor/tinf'/f for f in ('tinflate.c','tinfzlib.c','adler32.c')]
    objects=[]
    for i,source in enumerate(sources):
        obj=folder/f'{i}.o'
        defines=['-DNDEBUG'] if 'vendor' in source.parts else []
        subprocess.run(['cc',*flags,*defines,'-c',str(source),'-o',str(obj)],check=True);objects.append(str(obj))
    subprocess.run(['cc','-fsanitize=address,undefined',*objects,'-o',str(folder/'test')],check=True)
    subprocess.run([str(folder/'test'),str(ROOT/'resources/calendar.bin'),str(ROOT/'artifacts/calendar-raw.bin'),str(ROOT/'resources/readings.pmr')],check=True)
