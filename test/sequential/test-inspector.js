// Flags: --expose-internals
'use strict';
const common = require('../common');

common.skipIfInspectorDisabled();

const assert = require('assert');
const { NodeInstance } = require('../common/inspector-helper.js');

function checkListResponse(response) {
  assert.strictEqual(1, response.length);
  assert.ok(response[0].devtoolsFrontendUrl);
  assert.ok(
    /ws:\/\/127\.0\.0\.1:\d+\/[0-9A-Fa-f]{8}-/
      .test(response[0].webSocketDebuggerUrl));
}

function checkVersion(response) {
  assert.ok(response);
  const expected = {
    'Browser': `node.js/${process.version}`,
    'Protocol-Version': '1.1',
  };
  assert.strictEqual(JSON.stringify(response),
                     JSON.stringify(expected));
}

function checkBadPath(err) {
  assert(err instanceof SyntaxError, 'Expected SyntaxError');
  assert(
    common.engineSpecificMessage({
      v8: /Unexpected token/,
      chakracore: /JSON\.parse Error: Invalid character at position:1/
    }).test(err.message),
    'Unexpected message: ' + err.message);
  assert(/WebSockets request was expected/.test(err.body),
         'Unexpected response: ' + err.body);
}

function checkException(message) {
  assert.strictEqual(message.exceptionDetails, undefined,
                     'An exception occurred during execution');
}

function assertNoUrlsWhileConnected(response) {
  assert.strictEqual(1, response.length);
  assert.ok(!response[0].hasOwnProperty('devtoolsFrontendUrl'));
  assert.ok(!response[0].hasOwnProperty('webSocketDebuggerUrl'));
}

function assertScopeValues({ result }, expected) {
  const unmatched = new Set(Object.keys(expected));
  for (const actual of result) {
    const value = expected[actual.name];
    if (value) {
      assert.strictEqual(value, actual.value.value);
      unmatched.delete(actual.name);
    }
  }
  if (unmatched.size)
    assert.fail(Array.from(unmatched.values()));
}

async function testBreakpointOnStart(session) {
  console.log('[test]',
              'Verifying debugger stops on start (--inspect-brk option)');
  const commands = [
    { 'method': 'Runtime.enable' },
    { 'method': 'Debugger.enable' },
    { 'method': 'Debugger.setPauseOnExceptions',
      'params': { 'state': 'none' } },
    { 'method': 'Debugger.setAsyncCallStackDepth',
      'params': { 'maxDepth': 0 } }
  ];

  if (!common.isChakraEngine) {
    commands.push(
      { 'method': 'Profiler.enable' },
      { 'method': 'Profiler.setSamplingInterval',
        'params': { 'interval': 100 } });
  }

  commands.push(
    { 'method': 'Debugger.setBlackboxPatterns',
      'params': { 'patterns': [] } },
    { 'method': 'Runtime.runIfWaitingForDebugger' });

  await session.send(commands);
  await session.waitForBreakOnLine(0, session.scriptPath());
}

async function testBreakpoint(session) {
  console.log('[test]', 'Setting a breakpoint and verifying it is hit');
  const commands = [
    { 'method': 'Debugger.setBreakpointByUrl',
      'params': { 'lineNumber': 5,
                  'url': session.scriptPath(),
                  'columnNumber': 0,
                  'condition': ''
      }
    },
    { 'method': 'Debugger.resume' },
  ];
  await session.send(commands);
  const { scriptSource } = await session.send({
    'method': 'Debugger.getScriptSource',
    'params': { 'scriptId': session.mainScriptId } });
  assert(scriptSource && (scriptSource.includes(session.script())),
         `Script source is wrong: ${scriptSource}`);

  await session.waitForConsoleOutput('log', ['A message', 5]);
  const paused = await session.waitForBreakOnLine(5, session.scriptPath());
  const scopeId = paused.params.callFrames[0].scopeChain[0].object.objectId;

  console.log('[test]', 'Verify we can read current application state');
  const response = await session.send({
    'method': 'Runtime.getProperties',
    'params': {
      'objectId': scopeId,
      'ownProperties': false,
      'accessorPropertiesOnly': false,
      'generatePreview': true
    }
  });
  assertScopeValues(response, { t: 1001, k: 1 });

  let { result } = await session.send({
    'method': 'Debugger.evaluateOnCallFrame', 'params': {
      'callFrameId': '{"ordinal":0,"injectedScriptId":1}',
      'expression': 'k + t',
      'objectGroup': 'console',
      'includeCommandLineAPI': true,
      'silent': false,
      'returnByValue': false,
      'generatePreview': true
    }
  });

  assert.strictEqual(1002, result.value);

  result = (await session.send({
    'method': 'Runtime.evaluate', 'params': {
      'expression': '5 * 5'
    }
  })).result;
  assert.strictEqual(25, result.value);
}

async function testI18NCharacters(session) {
  console.log('[test]', 'Verify sending and receiving UTF8 characters');
  const chars = 'טֶ字и';
  session.send({
    'method': 'Debugger.evaluateOnCallFrame', 'params': {
      'callFrameId': '{"ordinal":0,"injectedScriptId":1}',
      'expression': `console.log("${chars}")`,
      'objectGroup': 'console',
      'includeCommandLineAPI': true,
      'silent': false,
      'returnByValue': false,
      'generatePreview': true
    }
  });
  await session.waitForConsoleOutput('log', [chars]);
}

async function testCommandLineAPI(session) {
  const testModulePath = require.resolve('../fixtures/empty.js');
  const testModuleStr = JSON.stringify(testModulePath);
  const printAModulePath = require.resolve('../fixtures/printA.js');
  const printAModuleStr = JSON.stringify(printAModulePath);
  const printBModulePath = require.resolve('../fixtures/printB.js');
  const printBModuleStr = JSON.stringify(printBModulePath);

  // we can use `require` outside of a callframe with require in scope
  let result = await session.send(
    {
      'method': 'Runtime.evaluate', 'params': {
        'expression': 'typeof require("fs").readFile === "function"',
        'includeCommandLineAPI': true
      }
    });
  checkException(result);
  assert.strictEqual(result.result.value, true);

  // the global require has the same properties as a normal `require`
  result = await session.send(
    {
      'method': 'Runtime.evaluate', 'params': {
        'expression': [
          'typeof require.resolve === "function"',
          'typeof require.extensions === "object"',
          'typeof require.cache === "object"'
        ].join(' && '),
        'includeCommandLineAPI': true
      }
    });
  checkException(result);
  assert.strictEqual(result.result.value, true);
  // `require` twice returns the same value
  result = await session.send(
    {
      'method': 'Runtime.evaluate', 'params': {
        // 1. We require the same module twice
        // 2. We mutate the exports so we can compare it later on
        'expression': `
          Object.assign(
            require(${testModuleStr}),
            { old: 'yes' }
          ) === require(${testModuleStr})`,
        'includeCommandLineAPI': true
      }
    });
  checkException(result);
  assert.strictEqual(result.result.value, true);
  // after require the module appears in require.cache
  result = await session.send(
    {
      'method': 'Runtime.evaluate', 'params': {
        'expression': `JSON.stringify(
          require.cache[${testModuleStr}].exports
        )`,
        'includeCommandLineAPI': true
      }
    });
  checkException(result);
  assert.deepStrictEqual(JSON.parse(result.result.value),
                         { old: 'yes' });
  // remove module from require.cache
  result = await session.send(
    {
      'method': 'Runtime.evaluate', 'params': {
        'expression': `delete require.cache[${testModuleStr}]`,
        'includeCommandLineAPI': true
      }
    });
  checkException(result);
  assert.strictEqual(result.result.value, true);
  // require again, should get fresh (empty) exports
  result = await session.send(
    {
      'method': 'Runtime.evaluate', 'params': {
        'expression': `JSON.stringify(require(${testModuleStr}))`,
        'includeCommandLineAPI': true
      }
    });
  checkException(result);
  assert.deepStrictEqual(JSON.parse(result.result.value), {});
  // require 2nd module, exports an empty object
  result = await session.send(
    {
      'method': 'Runtime.evaluate', 'params': {
        'expression': `JSON.stringify(require(${printAModuleStr}))`,
        'includeCommandLineAPI': true
      }
    });
  checkException(result);
  assert.deepStrictEqual(JSON.parse(result.result.value), {});
  // both modules end up with the same module.parent
  result = await session.send(
    {
      'method': 'Runtime.evaluate', 'params': {
        'expression': `JSON.stringify({
          parentsEqual:
            require.cache[${testModuleStr}].parent ===
            require.cache[${printAModuleStr}].parent,
          parentId: require.cache[${testModuleStr}].parent.id,
        })`,
        'includeCommandLineAPI': true
      }
    });
  checkException(result);
  assert.deepStrictEqual(JSON.parse(result.result.value), {
    parentsEqual: true,
    parentId: '<inspector console>'
  });
  // the `require` in the module shadows the command line API's `require`
  result = await session.send(
    {
      'method': 'Debugger.evaluateOnCallFrame', 'params': {
        'callFrameId': '{"ordinal":0,"injectedScriptId":1}',
        'expression': `(
          require(${printBModuleStr}),
          require.cache[${printBModuleStr}].parent.id
        )`,
        'includeCommandLineAPI': true
      }
    });
  checkException(result);
  assert.notStrictEqual(result.result.value,
                        '<inspector console>');
}

async function runTest() {
  const child = new NodeInstance();
  checkListResponse(await child.httpGet(null, '/json'));
  checkListResponse(await child.httpGet(null, '/json/list'));
  checkVersion(await child.httpGet(null, '/json/version'));

  await child.httpGet(null, '/json/activate').catch(checkBadPath);
  await child.httpGet(null, '/json/activate/boom').catch(checkBadPath);
  await child.httpGet(null, '/json/badpath').catch(checkBadPath);

  const session = await child.connectInspectorSession();
  assertNoUrlsWhileConnected(await child.httpGet(null, '/json/list'));
  await testBreakpointOnStart(session);
  await testBreakpoint(session);
  await testI18NCharacters(session);
  await testCommandLineAPI(session);
  await session.runToCompletion();
  assert.strictEqual(55, (await child.expectShutdown()).exitCode);
}

common.crashOnUnhandledRejection();

runTest();
