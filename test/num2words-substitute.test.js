'use strict';

const { describe, it } = require('node:test');
const assert = require('node:assert/strict');

/** Mirror of substitute_hour_in_phrase in src/num2words.c */
function substituteHourInPhrase(phrase, hour, varMarker) {
  const idx = phrase.indexOf(varMarker);
  if (idx === -1) {
    throw new Error(`marker ${varMarker} not in phrase`);
  }
  const prefix = phrase.slice(0, idx);
  const suffix = phrase.slice(idx + 2);
  return prefix + hour + suffix;
}

describe('hour substitution preserves display asterisk', () => {
  it('twenty til *$2', () => {
    const phrase = 'twenty til *$2 ';
    const out = substituteHourInPhrase(phrase, 'five', '$2');
    assert.equal(out, 'twenty til *five ');
  });

  it('*$1 o\'clock', () => {
    const phrase = '*$1 o\'clock ';
    const out = substituteHourInPhrase(phrase, 'twelve', '$1');
    assert.equal(out, '*twelve o\'clock ');
  });

  it('almost *$2', () => {
    const phrase = 'almost *$2 ';
    const out = substituteHourInPhrase(phrase, 'six', '$2');
    assert.equal(out, 'almost *six ');
  });
});
