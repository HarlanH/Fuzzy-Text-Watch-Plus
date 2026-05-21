const test = require('node:test');
const assert = require('node:assert/strict');

const {
  buildConfigUrl,
  configDataToDict,
  hasColorPlatform,
  hexColorToARGB2222,
  parseIcsEvents,
  computeMeetingStatus,
  normalizeCalendarUrl,
  getCalendarUrls,
  millisUntilNextBoundary,
  mergeEventsIntoCache,
  buildWeatherAppMessage,
  buildWeatherRequestUrl
} = require('../../src/js/pebble-js-app');

test('buildConfigUrl returns embedded data URL', () => {
  const url = buildConfigUrl(false);
  assert.match(url, /^data:text\/html;charset=utf-8,/);
  assert.match(decodeURIComponent(url), /pebblejs:\/\/close#/);
});

test('buildConfigUrl includes French and Japanese language options', () => {
  const html = decodeURIComponent(buildConfigUrl(false));
  assert.match(html, /<option value='9'>Français<\/option>/);
  assert.match(html, /<option value='10'>日本語 \(romaji\)<\/option>/);
  assert.match(html, /<option value='2'>English \(US\)<\/option>/);
  assert.match(html, /<option value='11'>English \(UK\)<\/option>/);
});

test('hasColorPlatform recognizes known color platforms', () => {
  assert.equal(hasColorPlatform('basalt'), true);
  assert.equal(hasColorPlatform('chalk'), true);
  assert.equal(hasColorPlatform('aplite'), false);
});

test('hexColorToARGB2222 converts valid color strings', () => {
  assert.equal(hexColorToARGB2222('0xFFFFFF'), 255);
  assert.equal(hexColorToARGB2222('0x000000'), 192);
  assert.equal(hexColorToARGB2222('#ffffff'), 0);
});

test('configDataToDict maps app settings keys', () => {
  const dict = configDataToDict({
    inverse_colors: true,
    background_color: '0x000000',
    regular_color: '0xFFFFFF',
    bold_color: '0xABCDEF',
    language: '2',
    offset: '180',
    gesture: '4',
    bt_notification: '2'
  });

  assert.equal(dict.KEY_INVERSE, 1);
  assert.equal(dict.KEY_BACKGROUND, 192);
  assert.equal(dict.KEY_REGULAR_TEXT, 255);
  assert.equal(dict.KEY_BOLD_TEXT, 239);
  assert.equal(dict.KEY_LANGUAGE, 2);
  assert.equal(dict.KEY_OFFSET, 180);
  assert.equal(dict.KEY_GESTURE, 4);
  assert.equal(dict.KEY_BT_NOTIFICATION, 2);
  assert.equal(dict.KEY_STRICT_HOUR_PHRASES, 1);
});

test('configDataToDict maps strict hour phrases off', () => {
  const dict = configDataToDict({ strict_hour_phrases: false });
  assert.equal(dict.KEY_STRICT_HOUR_PHRASES, 0);
});

test('configDataToDict supports new language identifiers', () => {
  const french = configDataToDict({ language: '9' });
  const japanese = configDataToDict({ language: '10' });
  const uk = configDataToDict({ language: '11' });
  assert.equal(french.KEY_LANGUAGE, 9);
  assert.equal(japanese.KEY_LANGUAGE, 10);
  assert.equal(uk.KEY_LANGUAGE, 11);
});

test('parseIcsEvents ignores transparent and free events', () => {
  const ics = [
    'BEGIN:VCALENDAR',
    'BEGIN:VEVENT',
    'DTSTART:20260428T160000Z',
    'DTEND:20260428T170000Z',
    'TRANSP:TRANSPARENT',
    'END:VEVENT',
    'BEGIN:VEVENT',
    'DTSTART:20260428T180000Z',
    'DTEND:20260428T190000Z',
    'X-MICROSOFT-CDO-BUSYSTATUS:FREE',
    'END:VEVENT',
    'BEGIN:VEVENT',
    'DTSTART:20260428T200000Z',
    'DTEND:20260428T210000Z',
    'END:VEVENT',
    'END:VCALENDAR'
  ].join('\r\n');

  const events = parseIcsEvents(ics);
  assert.equal(events.length, 1);
  assert.equal(events[0].start.getTime(), Date.UTC(2026, 3, 28, 20, 0, 0));
});

test('computeMeetingStatus returns NOW when in-progress busy event exists', () => {
  var now = new Date(Date.UTC(2026, 3, 28, 20, 5, 0));
  var events = [
    { start: new Date(Date.UTC(2026, 3, 28, 20, 0, 0)), end: new Date(Date.UTC(2026, 3, 28, 20, 30, 0)) }
  ];
  assert.equal(computeMeetingStatus(events, now, 10), 2);
});

test('computeMeetingStatus returns SOON for next busy event within 10 minutes', () => {
  var now = new Date(Date.UTC(2026, 3, 28, 19, 52, 0));
  var events = [
    { start: new Date(Date.UTC(2026, 3, 28, 20, 0, 0)), end: new Date(Date.UTC(2026, 3, 28, 20, 30, 0)) }
  ];
  assert.equal(computeMeetingStatus(events, now, 10), 1);
});

test('normalizeCalendarUrl converts webcal scheme to https', () => {
  assert.equal(
    normalizeCalendarUrl('webcal://calendar.google.com/calendar/ical/foo/basic.ics'),
    'https://calendar.google.com/calendar/ical/foo/basic.ics'
  );
});

test('getCalendarUrls returns up to three normalized calendar URLs', () => {
  const urls = getCalendarUrls({
    calendar_ics_1: ' webcal://calendar.google.com/calendar/ical/one/basic.ics ',
    calendar_ics_2: '',
    calendar_ics_3: 'https://calendar.google.com/calendar/ical/three/basic.ics'
  });

  assert.deepEqual(urls, [
    'https://calendar.google.com/calendar/ical/one/basic.ics',
    'https://calendar.google.com/calendar/ical/three/basic.ics'
  ]);
});

test('millisUntilNextBoundary aligns to five-minute marks', () => {
  var now = new Date(Date.UTC(2026, 3, 28, 16, 49, 10, 500));
  assert.equal(millisUntilNextBoundary(now, 5), 49500);
});

test('mergeEventsIntoCache drops duplicates across calendars', () => {
  var cached = [
    { start: new Date(Date.UTC(2026, 3, 28, 16, 0, 0)), end: new Date(Date.UTC(2026, 3, 28, 16, 30, 0)) }
  ];
  var incoming = [
    { start: new Date(Date.UTC(2026, 3, 28, 16, 0, 0)), end: new Date(Date.UTC(2026, 3, 28, 16, 30, 0)) },
    { start: new Date(Date.UTC(2026, 3, 28, 17, 0, 0)), end: new Date(Date.UTC(2026, 3, 28, 17, 30, 0)) }
  ];
  var merged = mergeEventsIntoCache(cached, incoming);
  assert.equal(merged.length, 2);
});

test('buildWeatherAppMessage maps Open-Meteo current to int tuples (°F)', () => {
  assert.deepEqual(buildWeatherAppMessage({ weather_code: 2, temperature_2m: 53.2 }), {
    11: 2,
    12: 53
  });
  assert.deepEqual(buildWeatherAppMessage({ weather_code: 63, temperature_2m: 58.8 }), {
    11: 63,
    12: 59
  });
  assert.equal(buildWeatherAppMessage(null), null);
  assert.equal(buildWeatherAppMessage({}), null);
});

test('buildWeatherRequestUrl uses provided coordinates and has fallback', () => {
  const brooklynUrl = buildWeatherRequestUrl({ latitude: 40.6782, longitude: -73.9442 });
  assert.match(brooklynUrl, /latitude=40\.6782/);
  assert.match(brooklynUrl, /longitude=-73\.9442/);
  assert.match(brooklynUrl, /temperature_unit=fahrenheit/);
  assert.match(brooklynUrl, /current=temperature_2m,weather_code/);

  const fallbackUrl = buildWeatherRequestUrl();
  assert.match(fallbackUrl, /latitude=37\.7749/);
  assert.match(fallbackUrl, /longitude=-122\.4194/);
});
