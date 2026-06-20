const test = require('node:test');
const assert = require('node:assert/strict');

const {
  parseYamlConfig,
  toAppMessageDict,
  buildTuplePayload
} = require('../../scripts/emu-apply-config');

test('parseYamlConfig reads yaml fields', () => {
  const config = parseYamlConfig([
    'language: 9',
    'offset: 180',
    'strict_hour_phrases: true'
  ].join('\n'));

  assert.equal(config.language, 9);
  assert.equal(config.offset, 180);
  assert.equal(config.strict_hour_phrases, true);
});

test('toAppMessageDict maps yaml values to message keys', () => {
  const dict = toAppMessageDict({
    language: 11,
    offset: 150,
    gesture: 4,
    bt_notification: 1,
    strict_hour_phrases: false,
    weather_code: 0,
    weather_temp_f: 72
  });

  assert.deepEqual(dict, {
    KEY_LANGUAGE: 11,
    KEY_OFFSET: 150,
    KEY_GESTURE: 4,
    KEY_BT_NOTIFICATION: 1,
    KEY_STRICT_HOUR_PHRASES: 0,
    KEY_WEATHER_CODE: 0,
    KEY_WEATHER_TEMP_F: 72
  });
});

test('buildTuplePayload encodes uint payload bytes with message IDs', () => {
  const tuples = buildTuplePayload(
    {
      KEY_LANGUAGE: 11,
      KEY_OFFSET: 180
    },
    {
      KEY_LANGUAGE: 4,
      KEY_OFFSET: 5
    }
  );

  assert.deepEqual(tuples, [
    { key: 4, type: 2, data: [11] },
    { key: 5, type: 2, data: [180] }
  ]);
});
