#!/usr/bin/env node

const fs = require('node:fs');
const path = require('node:path');
const { spawnSync } = require('node:child_process');
const YAML = require('yaml');

function parseYamlConfig(yamlText) {
  const parsed = YAML.parse(yamlText);
  if (!parsed || typeof parsed !== 'object' || Array.isArray(parsed)) {
    throw new Error('Config YAML must be a mapping/object at top level.');
  }
  return parsed;
}

function toNumber(value, keyName) {
  const numeric = Number(value);
  if (!Number.isFinite(numeric)) {
    throw new Error(`Expected numeric value for "${keyName}", got: ${value}`);
  }
  return numeric;
}

function boolToBinary(value) {
  return value ? 1 : 0;
}

function hexColorToARGB2222(color, keyName) {
  if (typeof color !== 'string' || !/^0x[0-9A-Fa-f]{6}$/.test(color)) {
    throw new Error(`Expected ${keyName} in format 0xRRGGBB, got: ${color}`);
  }
  const r = parseInt(color.substring(2, 3), 16) >> 2;
  const g = parseInt(color.substring(4, 5), 16) >> 2;
  const b = parseInt(color.substring(6, 7), 16) >> 2;
  return (3 << 6) + (r << 4) + (g << 2) + b;
}

function toAppMessageDict(config) {
  const dict = {};
  if (config.language !== undefined) dict.KEY_LANGUAGE = toNumber(config.language, 'language');
  if (config.offset !== undefined) dict.KEY_OFFSET = toNumber(config.offset, 'offset');
  if (config.gesture !== undefined) dict.KEY_GESTURE = toNumber(config.gesture, 'gesture');
  if (config.bt_notification !== undefined) dict.KEY_BT_NOTIFICATION = toNumber(config.bt_notification, 'bt_notification');
  if (config.strict_hour_phrases !== undefined) dict.KEY_STRICT_HOUR_PHRASES = boolToBinary(Boolean(config.strict_hour_phrases));
  if (config.weather_code !== undefined) dict.KEY_WEATHER_CODE = toNumber(config.weather_code, 'weather_code');
  if (config.weather_temp_f !== undefined) dict.KEY_WEATHER_TEMP_F = toNumber(config.weather_temp_f, 'weather_temp_f');
  if (config.inverse_colors !== undefined) dict.KEY_INVERSE = boolToBinary(Boolean(config.inverse_colors));
  if (config.background_color !== undefined) dict.KEY_BACKGROUND = hexColorToARGB2222(config.background_color, 'background_color');
  if (config.regular_color !== undefined) dict.KEY_REGULAR_TEXT = hexColorToARGB2222(config.regular_color, 'regular_color');
  if (config.bold_color !== undefined) dict.KEY_BOLD_TEXT = hexColorToARGB2222(config.bold_color, 'bold_color');
  return dict;
}

function encodeUintData(value, keyName) {
  if (!Number.isInteger(value) || value < 0) {
    throw new Error(`Expected non-negative integer for "${keyName}", got: ${value}`);
  }
  if (value <= 0xff) return [value];
  if (value <= 0xffff) return [value & 0xff, (value >> 8) & 0xff];
  if (value <= 0xffffffff) return [value & 0xff, (value >> 8) & 0xff, (value >> 16) & 0xff, (value >> 24) & 0xff];
  throw new Error(`Value for "${keyName}" is too large: ${value}`);
}

function buildTuplePayload(dict, messageKeys) {
  const tuples = [];
  for (const [name, value] of Object.entries(dict)) {
    const key = messageKeys[name];
    if (key === undefined) {
      throw new Error(`Message key "${name}" is missing from package.json pebble.messageKeys.`);
    }
    tuples.push({
      key,
      type: 2, // Uint
      data: encodeUintData(value, name)
    });
  }
  return tuples;
}

function parseArgs(argv) {
  const args = { file: 'dev-config.yml', emulator: null, qemu: null };
  for (let i = 0; i < argv.length; i++) {
    if (argv[i] === '--file') args.file = argv[++i];
    else if (argv[i] === '--emulator') args.emulator = argv[++i];
    else if (argv[i] === '--qemu') args.qemu = argv[++i] || 'localhost';
    else if (argv[i] === '--help' || argv[i] === '-h') args.help = true;
    else throw new Error(`Unknown argument: ${argv[i]}`);
  }
  if (args.emulator && args.qemu) {
    throw new Error('Use either --emulator or --qemu, not both.');
  }
  return args;
}

function printHelp() {
  console.log('Usage: node scripts/emu-apply-config.js [--file dev-config.yml] [--emulator basalt | --qemu localhost]');
}

function runCli() {
  const args = parseArgs(process.argv.slice(2));
  if (args.help) {
    printHelp();
    return;
  }

  const yamlText = fs.readFileSync(path.resolve(args.file), 'utf8');
  const config = parseYamlConfig(yamlText);
  const dict = toAppMessageDict(config);
  const packageJson = JSON.parse(fs.readFileSync(path.resolve('package.json'), 'utf8'));
  const messageKeys = packageJson.pebble && packageJson.pebble.messageKeys;
  if (!messageKeys) {
    throw new Error('Could not read pebble.messageKeys from package.json');
  }
  const tuplePayload = buildTuplePayload(dict, messageKeys);

  const replArgs = ['repl'];
  if (args.emulator) {
    replArgs.push('--emulator', args.emulator);
  }
  if (args.qemu) {
    replArgs.push('--qemu', args.qemu);
  }
  const appUuid = packageJson.pebble && packageJson.pebble.uuid;
  if (!appUuid) {
    throw new Error('Could not read pebble.uuid from package.json');
  }

  const pythonScript = [
    'import json, uuid',
    `app_uuid = uuid.UUID('${appUuid}')`,
    `tuple_payload = json.loads(${JSON.stringify(JSON.stringify(tuplePayload))})`,
    "tuples = [protocol.AppMessageTuple(key=item['key'], type=item['type'], length=len(bytes(item['data'])), data=bytes(item['data'])) for item in tuple_payload]",
    "msg = protocol.AppMessage(command=0x01, transaction_id=1, data=protocol.AppMessagePush(uuid=app_uuid, count=len(tuples), dictionary=tuples))",
    'pebble.send_packet(msg)',
    "print('Sent AppMessage tuples:', len(tuples))",
    'exit()',
    ''
  ].join('\n');

  const result = spawnSync('pebble', replArgs, { input: pythonScript, stdio: ['pipe', 'inherit', 'inherit'] });
  if (result.error) throw result.error;
  if (typeof result.status === 'number' && result.status !== 0) process.exit(result.status);
}

if (require.main === module) {
  try {
    runCli();
  } catch (err) {
    console.error(err.message);
    process.exit(1);
  }
}

module.exports = {
  parseYamlConfig,
  toAppMessageDict,
  buildTuplePayload
};
