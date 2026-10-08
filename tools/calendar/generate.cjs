// Build-time only: the watch receives an offline calendar, never JavaScript.
const fs = require('node:fs');
const path = require('node:path');
const { Romcal } = require('romcal');
const { GeneralRoman_En } = require('@romcal/calendar.general-roman');
const first = Number(process.argv[2] || 2020);
const last = Number(process.argv[3] || 2037);
const dest = path.resolve(__dirname, '../../artifacts/calendar-days.json');
const pick = (d) => ({
  id: d.id, name: d.name, rank: d.rank, dateDef: d.dateDef,
  season: d.seasons[0], week: d.calendar.weekOfSeason, colors: d.colors,
  sundayCycle: d.cycles.sundayCycle, weekdayCycle: d.cycles.weekdayCycle,
  weekday: d.weekday ? { id: d.weekday.id, dateDef: d.weekday.dateDef } : null,
});
(async () => {
  const r = new Romcal({ localizedCalendar: GeneralRoman_En, scope: 'gregorian',
    epiphanyOnSunday: false, ascensionOnSunday: false, corpusChristiOnSunday: false,
    outputOptions: { calculateProperties: true } });
  const days = {};
  for (let year = first; year <= last; year++) {
    const calendar = await r.generateCalendar(year);
    for (const [date, celebrations] of Object.entries(calendar)) {
      const heart = celebrations.find(d => d.id === 'immaculate_heart_of_mary' && d.rank === 'MEMORIAL');
      // CDW notification, 8 Dec 1998: collision with another obligatory
      // memorial makes BOTH optional. romcal dev.140 retains both as mandatory.
      if (heart && celebrations.some(d => d.id !== heart.id && d.rank === 'MEMORIAL')) {
        if (!heart.weekday || heart.weekday.rank !== 'WEEKDAY') throw new Error(`Missing weekday for ${date}`);
        days[date] = [pick(heart.weekday), ...celebrations.map(d => ({
          ...pick(d), rank: d.rank === 'MEMORIAL' ? 'OPTIONAL_MEMORIAL' : d.rank,
        }))];
      } else days[date] = celebrations.map(pick);
    }
  }
  fs.mkdirSync(path.dirname(dest), {recursive: true});
  fs.writeFileSync(dest, JSON.stringify({version: r.getVersion(), first, last, calendar: 'General Roman', days}));
  console.log(`Generated ${Object.keys(days).length} days (${first}–${last}): ${dest}`);
})().catch(e => {console.error(e);process.exit(1);});
