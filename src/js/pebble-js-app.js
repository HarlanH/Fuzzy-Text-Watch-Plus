// Watchface config version
var version = 44;
// Pebble injects message keys at build time; Node tests use package.json ids.
var K_WEATHER_CODE = typeof KEY_WEATHER_CODE !== 'undefined' ? KEY_WEATHER_CODE : 11;
var K_WEATHER_TEMP_F = typeof KEY_WEATHER_TEMP_F !== 'undefined' ? KEY_WEATHER_TEMP_F : 12;
var SETTINGS_STORAGE_KEY = 'almost_five_settings';
var CALENDAR_POLL_INTERVAL_MS = 300000;
var WEATHER_POLL_INTERVAL_MS = 1800000;
var DEFAULT_WEATHER_LAT = 37.7749;
var DEFAULT_WEATHER_LON = -122.4194;
var MEETING_STATUS_NONE = 0;
var MEETING_STATUS_SOON = 1;
var MEETING_STATUS_NOW = 2;
var lastMeetingStatus = null;
var lastWeatherSig = null;
var pollTimer = null;
var statusTimer = null;
var weatherTimer = null;
var cachedEvents = [];

function buildConfigUrl(hasColorScreen, initialSettings) {
  var initialSettingsJson = JSON.stringify(initialSettings || {});
  var html = "<!doctype html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'>" +
    "<title>Almost Five Settings</title>" +
    "<style>body{font-family:Helvetica,Arial,sans-serif;padding:16px;background:#f6f7f8;color:#1f2328}h3{margin-top:0}" +
    "label{display:block;margin:10px 0 6px;font-size:14px}input,select{width:100%;padding:10px;border:1px solid #c8ccd1;border-radius:6px;box-sizing:border-box}" +
    ".row{margin-bottom:10px}.muted{font-size:12px;color:#5a6169}button{margin-top:16px;padding:10px 12px;border:0;border-radius:6px;background:#0b57d0;color:#fff;font-size:15px;width:100%}" +
    ".hidden{display:none}</style></head><body>" +
    "<h3>Almost Five Settings</h3>" +
    "<div class='row'><label>Language</label><select id='language'>" +
    "<option value='1'>Svenska</option><option value='2'>English (US)</option><option value='11'>English (UK)</option><option value='3'>Norsk</option><option value='4'>Nederlands</option>" +
    "<option value='5'>Italiano</option><option value='6'>Español</option><option value='7'>Deutsch (O)</option><option value='8'>Deutsch (w)</option>" +
    "<option value='9'>Français</option><option value='10'>日本語 (romaji)</option></select></div>" +
    "<div id='bwRow' class='row hidden'><label><input id='inverse' type='checkbox'> Inverse colors</label></div>" +
    "<div id='colorRows' class='hidden'>" +
    "<div class='row'><label>Background color (0xRRGGBB)</label><input id='background' type='text' value='0x000000'></div>" +
    "<div class='row'><label>Regular text color (0xRRGGBB)</label><input id='regular' type='text' value='0xFFFFFF'></div>" +
    "<div class='row'><label>Bold text color (0xRRGGBB)</label><input id='bold' type='text' value='0xFFFFFF'></div>" +
    "</div>" +
    "<div class='row'><label>Time offset, minutes</label><select id='offset'>" +
    "<option value='0'>0:00</option><option value='60'>1:00</option><option value='120'>2:00</option><option value='150'>2:30</option>" +
    "<option value='180'>3:00</option><option value='240'>4:00</option><option value='300'>5:00</option></select></div>" +
    "<div class='row'><label>Enable backlight on gesture</label><select id='gesture'>" +
    "<option value='0'>Off</option><option value='2'>Flick wrist</option><option value='3'>Shake up/down</option><option value='1'>Boxing move</option><option value='4'>Any shake</option></select></div>" +
    "<div class='row'><label>Connection lost</label><select id='bt'>" +
    "<option value='0'>Off</option><option value='1'>Message only</option><option value='2'>Buzz and message</option></select></div>" +
    "<div class='row'><label><input id='strict_hour' type='checkbox' checked> Wait for real :00 and :30</label></div>" +
    "<p class='muted'>When on, “o'clock” and “half past” only appear after the real clock reaches that time (time offset can no longer pull those phrases early).</p>" +
    "<div class='row'><label>Google Calendar iCal URL 1</label><input id='calendar_ics_1' type='text' placeholder='https://.../basic.ics'></div>" +
    "<div class='row'><label>Google Calendar iCal URL 2</label><input id='calendar_ics_2' type='text' placeholder='https://.../basic.ics'></div>" +
    "<div class='row'><label>Google Calendar iCal URL 3</label><input id='calendar_ics_3' type='text' placeholder='https://.../basic.ics'></div>" +
    "<button id='test_fetch' type='button' style='background:#5f6368;margin-top:8px'>Test calendar fetch</button>" +
    "<p id='test_result' class='muted'></p>" +
    "<button id='save'>Save</button><p class='muted'>Version: " + version + "</p>" +
    "<script>(function(){var hasColor=" + (hasColorScreen ? "true" : "false") + ";" +
    "var initialSettings=" + initialSettingsJson + ";" +
    "var defaults={inverse_colors:false,background_color:'0x000000',regular_color:'0xFFFFFF',bold_color:'0xFFFFFF',language:'2',offset:'180',gesture:'4',bt_notification:'2',strict_hour_phrases:true,calendar_ics_1:'',calendar_ics_2:'',calendar_ics_3:''};" +
    "function normUrl(url){var trimmed=String(url||'').trim();if(trimmed.indexOf('webcal://')===0){return 'https://'+trimmed.substring('webcal://'.length);}return trimmed;}" +
    "function load(){try{var raw=localStorage.getItem('" + SETTINGS_STORAGE_KEY + "');if(raw){return Object.assign({},defaults,initialSettings,JSON.parse(raw));}}catch(e){}return Object.assign({},defaults,initialSettings);}" +
    "function save(v){try{localStorage.setItem('" + SETTINGS_STORAGE_KEY + "',JSON.stringify(v));}catch(e){}}" +
    "var state=load();" +
    "if(hasColor){document.getElementById('colorRows').classList.remove('hidden');}else{document.getElementById('bwRow').classList.remove('hidden');}" +
    "document.getElementById('inverse').checked=!!state.inverse_colors;" +
    "document.getElementById('background').value=state.background_color||'0x000000';" +
    "document.getElementById('regular').value=state.regular_color||'0xFFFFFF';" +
    "document.getElementById('bold').value=state.bold_color||'0xFFFFFF';" +
    "document.getElementById('language').value=String(state.language||'2');" +
    "document.getElementById('offset').value=String(state.offset||'180');" +
    "document.getElementById('gesture').value=String(state.gesture||'4');" +
    "document.getElementById('bt').value=String(state.bt_notification||'2');" +
    "document.getElementById('strict_hour').checked=state.strict_hour_phrases!==false;" +
    "document.getElementById('calendar_ics_1').value=String(state.calendar_ics_1||state.calendar_ics||'');" +
    "document.getElementById('calendar_ics_2').value=String(state.calendar_ics_2||'');" +
    "document.getElementById('calendar_ics_3').value=String(state.calendar_ics_3||'');" +
    "document.getElementById('test_fetch').addEventListener('click',function(){var out={inverse_colors:document.getElementById('inverse').checked,background_color:document.getElementById('background').value,regular_color:document.getElementById('regular').value,bold_color:document.getElementById('bold').value,language:document.getElementById('language').value,offset:document.getElementById('offset').value,gesture:document.getElementById('gesture').value,bt_notification:document.getElementById('bt').value,strict_hour_phrases:document.getElementById('strict_hour').checked,calendar_ics_1:normUrl(document.getElementById('calendar_ics_1').value),calendar_ics_2:normUrl(document.getElementById('calendar_ics_2').value),calendar_ics_3:normUrl(document.getElementById('calendar_ics_3').value),test_fetch:'1'};save(out);document.location='pebblejs://close#'+encodeURIComponent(JSON.stringify(out));});" +
    "document.getElementById('save').addEventListener('click',function(){var out={inverse_colors:document.getElementById('inverse').checked,background_color:document.getElementById('background').value,regular_color:document.getElementById('regular').value,bold_color:document.getElementById('bold').value,language:document.getElementById('language').value,offset:document.getElementById('offset').value,gesture:document.getElementById('gesture').value,bt_notification:document.getElementById('bt').value,strict_hour_phrases:document.getElementById('strict_hour').checked,calendar_ics_1:normUrl(document.getElementById('calendar_ics_1').value),calendar_ics_2:normUrl(document.getElementById('calendar_ics_2').value),calendar_ics_3:normUrl(document.getElementById('calendar_ics_3').value)};save(out);document.location='pebblejs://close#'+encodeURIComponent(JSON.stringify(out));});})();</script>" +
    "</body></html>";

  return 'data:text/html;charset=utf-8,' + encodeURIComponent(html);
}

function configDataToDict(configData) {
  var dict = {};
  dict.KEY_INVERSE = configData.inverse_colors ? 1 : 0;

  var argb = hexColorToARGB2222(configData.background_color);
  if (argb > 0) dict.KEY_BACKGROUND = argb;

  argb = hexColorToARGB2222(configData.regular_color);
  if (argb > 0) dict.KEY_REGULAR_TEXT = argb;

  argb = hexColorToARGB2222(configData.bold_color);
  if (argb > 0) dict.KEY_BOLD_TEXT = argb;

  var lang = configData.language;
  if (lang > 0) dict.KEY_LANGUAGE = parseInt(lang, 10);

  var offset = configData.offset;
  if (offset !== undefined) dict.KEY_OFFSET = parseInt(offset, 10);

  var gesture = configData.gesture;
  if (gesture !== undefined) dict.KEY_GESTURE = parseInt(gesture, 10);

  var btNotification = configData.bt_notification;
  if (btNotification !== undefined) dict.KEY_BT_NOTIFICATION = parseInt(btNotification, 10);

  dict.KEY_STRICT_HOUR_PHRASES = configData.strict_hour_phrases !== false ? 1 : 0;

  return dict;
}

function hexColorToARGB2222(color) {
  if (color && color.length === 8) { // Expect "0xRRGGBB"
    var r = parseInt(color.substring(2, 3), 16) >> 2;
    var g = parseInt(color.substring(4, 5), 16) >> 2;
    var b = parseInt(color.substring(6, 7), 16) >> 2;

    var col = 3; // alpha
    col = (col << 2) + r;
    col = (col << 2) + g;
    col = (col << 2) + b;
    return col;
  }

  return 0;
}

function hasColorPlatform(platform) {
  return platform.indexOf('basalt') >= 0
    || platform.indexOf('chalk') >= 0
    || platform.indexOf('emery') >= 0
    || platform.indexOf('diorite') >= 0;
}

function hasColor() {
  if (Pebble.getActiveWatchInfo) {
    var platform = Pebble.getActiveWatchInfo().platform;
    return hasColorPlatform(platform);
  }

  return false;
}

function loadStoredSettings() {
  var defaults = {
    calendar_ics_1: '',
    calendar_ics_2: '',
    calendar_ics_3: ''
  };

  try {
    var raw = localStorage.getItem(SETTINGS_STORAGE_KEY);
    if (raw) {
      return Object.assign({}, defaults, JSON.parse(raw));
    }
  } catch (err) {
    console.log('Failed to read settings: ' + err);
  }

  return defaults;
}

function saveStoredSettings(settings) {
  try {
    localStorage.setItem(SETTINGS_STORAGE_KEY, JSON.stringify(settings));
  } catch (err) {
    console.log('Failed to save settings: ' + err);
  }
}

function normalizeCalendarUrl(url) {
  var trimmed = (url || '').trim();
  if (trimmed.indexOf('webcal://') === 0) {
    return 'https://' + trimmed.substring('webcal://'.length);
  }
  return trimmed;
}

function getCalendarUrls(settings) {
  var urls = [];
  var candidates = [
    settings.calendar_ics_1 || settings.calendar_ics || '',
    settings.calendar_ics_2 || '',
    settings.calendar_ics_3 || ''
  ];
  for (var i = 0; i < candidates.length; i++) {
    var normalized = normalizeCalendarUrl(candidates[i]);
    if (normalized) {
      urls.push(normalized);
    }
  }
  return urls;
}

function parseIcsDate(value) {
  if (!value) {
    return null;
  }

  var utc = /^(\d{4})(\d{2})(\d{2})T(\d{2})(\d{2})(\d{2})Z$/.exec(value);
  if (utc) {
    return new Date(Date.UTC(
      parseInt(utc[1], 10),
      parseInt(utc[2], 10) - 1,
      parseInt(utc[3], 10),
      parseInt(utc[4], 10),
      parseInt(utc[5], 10),
      parseInt(utc[6], 10)
    ));
  }

  return null;
}

function unfoldIcsLines(icsText) {
  var lines = icsText.split(/\r?\n/);
  var unfolded = [];
  for (var i = 0; i < lines.length; i++) {
    var line = lines[i];
    if ((line.charAt(0) === ' ' || line.charAt(0) === '\t') && unfolded.length > 0) {
      unfolded[unfolded.length - 1] += line.substring(1);
    } else {
      unfolded.push(line);
    }
  }
  return unfolded;
}

function isBusyEvent(event) {
  var transp = event.transp || '';
  if (transp.toUpperCase() === 'TRANSPARENT') {
    return false;
  }

  var busyStatus = event.busyStatus || '';
  if (busyStatus.toUpperCase() === 'FREE') {
    return false;
  }

  return true;
}

function parseIcsEvents(icsText) {
  if (!icsText) {
    return [];
  }

  var lines = unfoldIcsLines(icsText);
  var events = [];
  var event = null;

  for (var i = 0; i < lines.length; i++) {
    var line = lines[i];
    if (line === 'BEGIN:VEVENT') {
      event = {};
      continue;
    }
    if (line === 'END:VEVENT') {
      if (event && event.start && event.end && isBusyEvent(event)) {
        events.push({
          start: event.start,
          end: event.end
        });
      }
      event = null;
      continue;
    }
    if (!event) {
      continue;
    }

    var dtStart = /^DTSTART(?:;[^:]*)?:(.+)$/.exec(line);
    if (dtStart) {
      event.start = parseIcsDate(dtStart[1]);
      continue;
    }

    var dtEnd = /^DTEND(?:;[^:]*)?:(.+)$/.exec(line);
    if (dtEnd) {
      event.end = parseIcsDate(dtEnd[1]);
      continue;
    }

    var transp = /^TRANSP:(.+)$/.exec(line);
    if (transp) {
      event.transp = transp[1];
      continue;
    }

    var busyStatus = /^X-MICROSOFT-CDO-BUSYSTATUS:(.+)$/.exec(line);
    if (busyStatus) {
      event.busyStatus = busyStatus[1];
    }
  }

  return events;
}

function computeMeetingStatus(events, now, soonMinutes) {
  var nowMs = now.getTime();
  var soonMs = soonMinutes * 60 * 1000;

  for (var i = 0; i < events.length; i++) {
    var startMs = events[i].start.getTime();
    var endMs = events[i].end.getTime();
    if (nowMs >= startMs && nowMs < endMs) {
      return MEETING_STATUS_NOW;
    }
  }

  var closestStartMs = null;
  for (var j = 0; j < events.length; j++) {
    var candidateMs = events[j].start.getTime();
    if (candidateMs > nowMs && (closestStartMs === null || candidateMs < closestStartMs)) {
      closestStartMs = candidateMs;
    }
  }

  if (closestStartMs !== null && closestStartMs - nowMs <= soonMs) {
    return MEETING_STATUS_SOON;
  }

  return MEETING_STATUS_NONE;
}

function mergeEventsIntoCache(existing, incoming) {
  var seen = {};
  var merged = [];
  var all = (existing || []).concat(incoming || []);
  for (var i = 0; i < all.length; i++) {
    var event = all[i];
    var key = event.start.getTime() + '-' + event.end.getTime();
    if (!seen[key]) {
      seen[key] = true;
      merged.push(event);
    }
  }
  return merged;
}

/** Build AppMessage dict for watch-localized weather (Open-Meteo current object). Always °F on the wire. */
function buildWeatherAppMessage(current) {
  if (!current) {
    return null;
  }

  var code = current.weather_code;
  if (typeof code !== 'number') {
    code = current.weathercode;
  }

  var tempF = current.temperature_2m;
  if (typeof tempF !== 'number') {
    tempF = current.temperature;
  }

  if (typeof code !== 'number' || typeof tempF !== 'number') {
    return null;
  }

  var msg = {};
  msg[K_WEATHER_CODE] = Math.round(code);
  msg[K_WEATHER_TEMP_F] = Math.round(tempF);
  return msg;
}

function sendWeatherAppMessage(dict) {
  if (!dict) {
    dict = {};
    dict[K_WEATHER_CODE] = -1;
    dict[K_WEATHER_TEMP_F] = 0;
  }

  var sig = dict[K_WEATHER_CODE] + ':' + dict[K_WEATHER_TEMP_F];
  if (lastWeatherSig === sig) {
    return;
  }

  Pebble.sendAppMessage(dict, function() {
    lastWeatherSig = sig;
  }, function(err) {
    console.log('Weather AppMessage send failed: ' + JSON.stringify(err));
  });
}

function buildWeatherRequestUrl(coords) {
  var latitude = DEFAULT_WEATHER_LAT;
  var longitude = DEFAULT_WEATHER_LON;
  if (coords && typeof coords.latitude === 'number' && typeof coords.longitude === 'number') {
    latitude = coords.latitude;
    longitude = coords.longitude;
  }

  return 'https://api.open-meteo.com/v1/forecast?latitude=' + encodeURIComponent(latitude) +
    '&longitude=' + encodeURIComponent(longitude) +
    '&current=temperature_2m,weather_code&temperature_unit=fahrenheit';
}

function getWeatherCoordinates(callback) {
  if (typeof navigator === 'undefined' || !navigator.geolocation || !navigator.geolocation.getCurrentPosition) {
    callback(null);
    return;
  }

  navigator.geolocation.getCurrentPosition(function(pos) {
    if (!pos || !pos.coords) {
      callback(null);
      return;
    }
    callback({
      latitude: pos.coords.latitude,
      longitude: pos.coords.longitude
    });
  }, function(err) {
    console.log('Weather geolocation unavailable: ' + JSON.stringify(err));
    callback(null);
  }, {
    enableHighAccuracy: false,
    timeout: 10000,
    maximumAge: 15 * 60 * 1000
  });
}

function refreshWeatherPhrase() {
  getWeatherCoordinates(function(coords) {
    var url = buildWeatherRequestUrl(coords);
    var req = new XMLHttpRequest();
    req.onload = function() {
      if (req.status < 200 || req.status >= 300) {
        console.log('Weather request failed with status ' + req.status);
        sendWeatherAppMessage(null);
        return;
      }

      try {
        var payload = JSON.parse(req.responseText);
        var msg = buildWeatherAppMessage(payload.current);
        sendWeatherAppMessage(msg || null);
      } catch (err) {
        console.log('Weather parse failed: ' + err);
        sendWeatherAppMessage(null);
      }
    };

    req.onerror = function() {
      console.log('Weather request network error');
      sendWeatherAppMessage(null);
    };

    req.open('GET', url, true);
    req.send();
  });
}

function scheduleWeatherPolling() {
  if (weatherTimer) {
    clearTimeout(weatherTimer);
  }

  weatherTimer = setTimeout(function() {
    refreshWeatherPhrase();
    scheduleWeatherPolling();
  }, WEATHER_POLL_INTERVAL_MS);
}

function millisUntilNextBoundary(now, minutes) {
  var intervalMs = minutes * 60 * 1000;
  var nowMs = now.getTime();
  var nextMs = Math.ceil(nowMs / intervalMs) * intervalMs;
  return nextMs - nowMs;
}

function evaluateAndSendStatus() {
  var status = computeMeetingStatus(cachedEvents, new Date(), 10);
  sendMeetingStatus(status);
}

function sendMeetingStatus(status) {
  if (lastMeetingStatus === status) {
    return;
  }

  var dict = { KEY_MEETING_STATUS: status };
  Pebble.sendAppMessage(dict, function() {
    lastMeetingStatus = status;
  }, function(err) {
    console.log('Meeting status send failed: ' + JSON.stringify(err));
  });
}

function refreshMeetingStatus() {
  var settings = loadStoredSettings();
  var icsUrls = getCalendarUrls(settings);

  if (icsUrls.length === 0) {
    sendMeetingStatus(MEETING_STATUS_NONE);
    return;
  }

  var pending = icsUrls.length;
  var fetchedEvents = [];

  function handleDone() {
    pending--;
    if (pending > 0) {
      return;
    }
    cachedEvents = mergeEventsIntoCache([], fetchedEvents);
    evaluateAndSendStatus();
  }

  for (var i = 0; i < icsUrls.length; i++) {
    (function(url) {
      var req = new XMLHttpRequest();
      req.onload = function() {
        if (req.status >= 200 && req.status < 300) {
          fetchedEvents = mergeEventsIntoCache(fetchedEvents, parseIcsEvents(req.responseText));
        } else {
          console.log('ICS request failed with status ' + req.status + ' for ' + url);
        }
        handleDone();
      };
      req.onerror = function() {
        console.log('ICS request network error for ' + url);
        handleDone();
      };
      req.open('GET', url, true);
      req.send();
    })(icsUrls[i]);
  }
}

function testCalendarFetch(settings) {
  var urls = getCalendarUrls(settings);
  if (urls.length === 0) {
    Pebble.showSimpleNotificationOnPebble('Calendar Test', 'Enter an iCal URL first.');
    return;
  }

  var pending = urls.length;
  var okCount = 0;
  var failCount = 0;

  function finish() {
    pending--;
    if (pending > 0) {
      return;
    }
    if (okCount > 0 && failCount === 0) {
      Pebble.showSimpleNotificationOnPebble('Calendar Test', 'Success: ' + okCount + ' feed(s) fetched.');
    } else if (okCount > 0) {
      Pebble.showSimpleNotificationOnPebble('Calendar Test', 'Partial success: ' + okCount + ' ok, ' + failCount + ' failed.');
    } else {
      Pebble.showSimpleNotificationOnPebble('Calendar Test', 'Fetch failed for all calendars.');
    }
  }

  for (var i = 0; i < urls.length; i++) {
    (function(url) {
      var req = new XMLHttpRequest();
      req.onload = function() {
        if (req.status >= 200 && req.status < 300 && req.responseText.indexOf('BEGIN:VCALENDAR') >= 0) {
          okCount++;
        } else {
          failCount++;
        }
        finish();
      };
      req.onerror = function() {
        failCount++;
        finish();
      };
      req.open('GET', url, true);
      req.send();
    })(urls[i]);
  }
}

function scheduleAlignedPolling() {
  if (pollTimer) {
    clearTimeout(pollTimer);
  }

  var delayMs = millisUntilNextBoundary(new Date(), 5);
  pollTimer = setTimeout(function() {
    refreshMeetingStatus();
    scheduleAlignedPolling();
  }, delayMs);
}

function scheduleMinuteStatusChecks() {
  if (statusTimer) {
    clearTimeout(statusTimer);
  }

  var delayMs = millisUntilNextBoundary(new Date(), 1);
  statusTimer = setTimeout(function() {
    evaluateAndSendStatus();
    scheduleMinuteStatusChecks();
  }, delayMs);
}

function startMeetingScheduling() {
  refreshMeetingStatus();
  scheduleAlignedPolling();
  scheduleMinuteStatusChecks();
}

function startWeatherScheduling() {
  refreshWeatherPhrase();
  scheduleWeatherPolling();
}

if (typeof Pebble !== 'undefined') {
  Pebble.addEventListener('ready', function() {
    console.log('PebbleKit JS ready!');
    startMeetingScheduling();
    startWeatherScheduling();
  });

  Pebble.addEventListener('showConfiguration', function() {
    var url = buildConfigUrl(hasColor(), loadStoredSettings());
    console.log('Showing configuration page');
    Pebble.openURL(url);
  });

  Pebble.addEventListener('webviewclosed', function(e) {
    if (!e || !e.response) {
      return;
    }

    var configData = JSON.parse(decodeURIComponent(e.response));
    console.log('Configuration page returned: ' + JSON.stringify(configData));
    configData.calendar_ics_1 = normalizeCalendarUrl(configData.calendar_ics_1 || configData.calendar_ics || '');
    configData.calendar_ics_2 = normalizeCalendarUrl(configData.calendar_ics_2 || '');
    configData.calendar_ics_3 = normalizeCalendarUrl(configData.calendar_ics_3 || '');
    delete configData.calendar_ics;
    saveStoredSettings(configData);
    refreshMeetingStatus();

    if (configData.test_fetch === '1') {
      testCalendarFetch(configData);
      return;
    }

    var dict = configDataToDict(configData);
    Pebble.sendAppMessage(dict, function() {
      console.log('Send successful: ' + JSON.stringify(dict));
    }, function(err) {
      console.log('Send failed! ' + JSON.stringify(err) + ', ' + JSON.stringify(dict));
    });
  });
}

if (typeof module !== 'undefined' && module.exports) {
  module.exports = {
    buildConfigUrl: buildConfigUrl,
    configDataToDict: configDataToDict,
    hasColorPlatform: hasColorPlatform,
    hexColorToARGB2222: hexColorToARGB2222,
    parseIcsEvents: parseIcsEvents,
    computeMeetingStatus: computeMeetingStatus,
    normalizeCalendarUrl: normalizeCalendarUrl,
    getCalendarUrls: getCalendarUrls,
    millisUntilNextBoundary: millisUntilNextBoundary,
    mergeEventsIntoCache: mergeEventsIntoCache,
    buildWeatherAppMessage: buildWeatherAppMessage,
    buildWeatherRequestUrl: buildWeatherRequestUrl
  };
}
