// Copyright Joyent, Inc. and other Node contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit
// persons to whom the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
// NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
// USE OR OTHER DEALINGS IN THE SOFTWARE.

'use strict';
const common = require('../common');
const fixtures = require('../common/fixtures');
const assert = require('assert');
const net = require('net');
const repl = require('repl');

common.globalCheck = false;
common.crashOnUnhandledRejection();

const message = 'Read, Eval, Print Loop';
const prompt_unix = 'node via Unix socket> ';
const prompt_tcp = 'node via TCP socket> ';

// absolute path to test/fixtures/a.js
const moduleFilename = fixtures.path('a');

// function for REPL to run
global.invoke_me = function(arg) {
  return `invoked ${arg}`;
};


// Helpers for describing the expected output:
const kArrow = /^ *\^+ *$/;  // Arrow of ^ pointing to syntax error location
const kSource = Symbol('kSource');  // Placeholder standing for input readback

async function runReplTests(socket, prompt, tests) {
  let lineBuffer = '';

  for (const { send, expect } of tests) {
    // expect can be a single line or multiple lines
    const expectedLines = Array.isArray(expect) ? expect : [ expect ];

    console.error('out:', JSON.stringify(send));
    socket.write(`${send}\n`);

    for (let expectedLine of expectedLines) {
      // special value: kSource refers to last sent source text
      if (expectedLine === kSource)
        expectedLine = send;

      while (!lineBuffer.includes('\n')) {
        lineBuffer += await event(socket, 'data');

        // Cut away the initial prompt
        while (lineBuffer.startsWith(prompt))
          lineBuffer = lineBuffer.substr(prompt.length);

        // Allow to match partial text if no newline was received, because
        // sending newlines from the REPL itself would be redundant
        // (e.g. in the `... ` multiline prompt: The user already pressed
        // enter for that, so the REPL shouldn't do it again!).
        if (lineBuffer === expectedLine && !expectedLine.includes('\n'))
          lineBuffer += '\n';
      }

      // Split off the current line.
      const newlineOffset = lineBuffer.indexOf('\n');
      let actualLine = lineBuffer.substr(0, newlineOffset);
      lineBuffer = lineBuffer.substr(newlineOffset + 1);

      // This might have been skipped in the loop above because the buffer
      // already contained a \n to begin with and the entire loop was skipped.
      while (actualLine.startsWith(prompt))
        actualLine = actualLine.substr(prompt.length);

      console.error('in:', JSON.stringify(actualLine));

      // Match a string directly, or a RegExp through .test().
      if (typeof expectedLine === 'string') {
        assert.strictEqual(actualLine, expectedLine);
      } else {
        assert(expectedLine.test(actualLine),
               `${actualLine} match ${expectedLine}`);
      }
    }
  }

  const remainder = socket.read();
  assert(remainder === '' || remainder === null);
}

const unixTests = [
  {
    send: '',
    expect: ''
  },
  {
    send: 'message',
    expect: `'${message}'`
  },
  {
    send: 'invoke_me(987)',
    expect: '\'invoked 987\''
  },
  {
    send: 'a = 12345',
    expect: '12345'
  },
  {
    send: '{a:1}',
    expect: '{ a: 1 }'
  }
];

const strictModeTests = [
  {
    send: 'ref = 1',
    expect: /^ReferenceError:\s/
  }
];

const errorTests = [
  // Uncaught error throws and prints out
  {
    send: 'throw new Error(\'test error\');',
    expect: /^Error: test error/
  },
  // Common syntax error is treated as multiline command
  {
    send: 'function test_func() {',
    expect: '... '
  },
  // You can recover with the .break command
  {
    send: '.break',
    expect: ''
  },
  // But passing the same string to eval() should throw
  {
    send: 'eval("function test_func() {")',
    expect: /^SyntaxError: /
  },
  // Can handle multiline template literals
  {
    send: '`io.js',
    expect: '... '
  },
  // Special REPL commands still available
  {
    send: '.break',
    expect: ''
  },
  // Template expressions
  {
    send: '`io.js ${"1.0"',
    expect: [
      kSource,
      kArrow,
      '',
      /^SyntaxError: /,
      ''
    ]
  },
  {
    send: '`io.js ${',
    expect: '... '
  },
  {
    send: '"1.0" + ".2"}`',
    expect: '\'io.js 1.0.2\''
  },
  // Dot prefix in multiline commands aren't treated as commands
  {
    send: '("a"',
    expect: '... '
  },
  {
    send: '.charAt(0))',
    expect: '\'a\''
  },
  // Floating point numbers are not interpreted as REPL commands.
  {
    send: '.1234',
    expect: '0.1234'
  },
  // Floating point expressions are not interpreted as REPL commands
  {
    send: '.1+.1',
    expect: '0.2'
  },
  // Can parse valid JSON
  {
    send: 'JSON.parse(\'{"valid": "json"}\');',
    expect: '{ valid: \'json\' }'
  },
  // invalid input to JSON.parse error is special case of syntax error,
  // should throw
  {
    send: 'JSON.parse(\'{invalid: \\\'json\\\'}\');',
    expect: [/^SyntaxError: /, '']
  },
  // end of input to JSON.parse error is special case of syntax error,
  // should throw
  {
    send: 'JSON.parse(\'066\');',
    expect: [/^SyntaxError: /, '']
  },
  // should throw
  {
    send: 'JSON.parse(\'{\');',
    expect: [/^SyntaxError: /, '']
  },
  // invalid RegExps are a special case of syntax error,
  // should throw
  {
    send: '/(/;',
    expect: /^SyntaxError: /
  },
  // invalid RegExp modifiers are a special case of syntax error,
  // should throw (GH-4012)
  {
    send: 'new RegExp("foo", "wrong modifier");',
    expect: [/^SyntaxError: /, '']
  },
  // strict mode syntax errors should be caught (GH-5178)
  {
    send: '(function() { "use strict"; return 0755; })()',
    expect: [
      kSource,
      kArrow,
      '',
      /^SyntaxError: /,
      ''
    ]
  },
  {
    send: '(function(a, a, b) { "use strict"; return a + b + c; })()',
    expect: [
      kSource,
      kArrow,
      '',
      /^SyntaxError: /,
      ''
    ]
  },
  {
    send: '(function() { "use strict"; with (this) {} })()',
    expect: [
      kSource,
      kArrow,
      '',
      /^SyntaxError: /,
      ''
    ]
  },
  {
    send: '(function() { "use strict"; var x; delete x; })()',
    expect: [
      kSource,
      kArrow,
      '',
      /^SyntaxError: /,
      ''
    ]
  },
  {
    send: '(function() { "use strict"; eval = 17; })()',
    expect: [
      kSource,
      kArrow,
      '',
      /^SyntaxError: /,
      ''
    ]
  },
  {
    send: '(function() { "use strict"; if (true) function f() { } })()',
    expect: [
      kSource,
      kArrow,
      '',
      /^SyntaxError: /,
      ''
    ]
  },
  // Named functions can be used:
  {
    send: 'function blah() { return 1; }',
    expect: 'undefined'
  },
  {
    send: 'blah()',
    expect: '1'
  },
  // Functions should not evaluate twice (#2773)
  {
    send: 'var I = [1,2,3,function() {}]; I.pop()',
    expect: '[Function]'
  },
  // Multiline object
  {
    send: '{ a: ',
    expect: '... '
  },
  {
    send: '1 }',
    expect: '{ a: 1 }'
  },
  // Multiline anonymous function with comment
  {
    send: '(function() {',
    expect: '... '
  },
  {
    send: '// blah',
    expect: '... '
  },
  {
    send: 'return 1;',
    expect: '... '
  },
  {
    send: '})()',
    expect: '1'
  },
  // Multiline function call
  {
    send: 'function f(){}; f(f(1,',
    expect: '... ',
    chakracore: 'https://github.com/Microsoft/ChakraCore/issues/767'
  },
  {
    send: '2)',
    expect: '... ',
    chakracore: 'skip'
  },
  {
    send: ')',
    expect: 'undefined',
    chakracore: 'skip'
  },
  // npm prompt error message
  {
    send: 'npm install foobar',
    expect: [
      'npm should be run outside of the node repl, in your normal shell.',
      '(Press Control-D to exit.)'
    ]
  },
  {
    send: '(function() {\n\nreturn 1;\n})()',
    expect: '... ... ... 1'
  },
  {
    send: '{\n\na: 1\n}',
    expect: '... ... ... { a: 1 }'
  },
  {
    send: 'url.format("http://google.com")',
    expect: '\'http://google.com/\''
  },
  {
    send: 'var path = 42; path',
    expect: '42'
  },
  // this makes sure that we don't print `undefined` when we actually print
  // the error message
  {
    send: '.invalid_repl_command',
    expect: 'Invalid REPL keyword'
  },
  // this makes sure that we don't crash when we use an inherited property as
  // a REPL command
  {
    send: '.toString',
    expect: 'Invalid REPL keyword'
  },
  // fail when we are not inside a String and a line continuation is used
  {
    send: '[] \\',
    expect: [
      kSource,
      kArrow,
      '',
      /^SyntaxError: /,
      ''
    ]
  },
  // do not fail when a String is created with line continuation
  {
    send: '\'the\\\nfourth\\\neye\'',
    expect: ['... ... \'thefourtheye\'']
  },
  // Don't fail when a partial String is created and line continuation is used
  // with whitespace characters at the end of the string. We are to ignore it.
  // This test is to make sure that we properly remove the whitespace
  // characters at the end of line, unlike the buggy `trimWhitespace` function
  {
    send: '  \t    .break  \t  ',
    expect: ''
  },
  // multiline strings preserve whitespace characters in them
  {
    send: '\'the \\\n   fourth\t\t\\\n  eye  \'',
    expect: '... ... \'the    fourth\\t\\t  eye  \''
  },
  // more than one multiline strings also should preserve whitespace chars
  {
    send: '\'the \\\n   fourth\' +  \'\t\t\\\n  eye  \'',
    expect: '... ... \'the    fourth\\t\\t  eye  \''
  },
  // using REPL commands within a string literal should still work
  {
    send: '\'\\\n.break',
    expect: '... ' + prompt_unix
  },
  // using REPL command "help" within a string literal should still work
  {
    send: '\'thefourth\\\n.help\neye\'',
    expect: [
      /\.break/,
      /\.clear/,
      /\.editor/,
      /\.exit/,
      /\.help/,
      /\.load/,
      /\.save/,
      /'thefourtheye'/
    ]
  },
  // empty lines in the REPL should be allowed
  {
    send: '\n\r\n\r\n',
    expect: ''
  },
  // empty lines in the string literals should not affect the string
  {
    send: '\'the\\\n\\\nfourtheye\'\n',
    expect: '... ... \'thefourtheye\''
  },
  // Regression test for https://github.com/nodejs/node/issues/597
  {
    send: '/(.)(.)(.)(.)(.)(.)(.)(.)(.)/.test(\'123456789\')\n',
    expect: 'true'
  },
  // the following test's result depends on the RegExp's match from the above
  {
    send: 'RegExp.$1\nRegExp.$2\nRegExp.$3\nRegExp.$4\nRegExp.$5\n' +
          'RegExp.$6\nRegExp.$7\nRegExp.$8\nRegExp.$9\n',
    expect: ['\'1\'', '\'2\'', '\'3\'', '\'4\'', '\'5\'', '\'6\'',
             '\'7\'', '\'8\'', '\'9\'']
  },
  // regression tests for https://github.com/nodejs/node/issues/2749
  {
    send: 'function x() {\nreturn \'\\n\';\n }',
    expect: '... ... undefined'
  },
  {
    send: 'function x() {\nreturn \'\\\\\';\n }',
    expect: '... ... undefined'
  },
  // regression tests for https://github.com/nodejs/node/issues/3421
  {
    send: 'function x() {\n//\'\n }',
    expect: '... ... undefined'
  },
  {
    send: 'function x() {\n//"\n }',
    expect: '... ... undefined'
  },
  {
    send: 'function x() {//\'\n }',
    expect: '... undefined'
  },
  {
    send: 'function x() {//"\n }',
    expect: '... undefined'
  },
  {
    send: 'function x() {\nvar i = "\'";\n }',
    expect: '... ... undefined'
  },
  {
    send: 'function x(/*optional*/) {}',
    expect: 'undefined'
  },
  {
    send: 'function x(/* // 5 */) {}',
    expect: 'undefined'
  },
  {
    send: '// /* 5 */',
    expect: 'undefined'
  },
  {
    send: '"//"',
    expect: '\'//\''
  },
  {
    send: '"data /*with*/ comment"',
    expect: '\'data /*with*/ comment\''
  },
  {
    send: 'function x(/*fn\'s optional params*/) {}',
    expect: 'undefined'
  },
  {
    send: '/* \'\n"\n\'"\'\n*/',
    expect: '... ... ... undefined'
  },
  // REPL should get a normal require() function, not one that allows
  // access to internal modules without the --expose_internals flag.
  {
    send: 'require("internal/repl")',
    expect: [
      /^Error: Cannot find module 'internal\/repl'/,
      /^    at .*/,
      /^    at .*/,
      /^    at .*/,
      /^    at .*/
    ]
  },
  // REPL should handle quotes within regexp literal in multiline mode
  {
    send: "function x(s) {\nreturn s.replace(/'/,'');\n}",
    expect: '... ... undefined'
  },
  {
    send: "function x(s) {\nreturn s.replace(/'/,'');\n}",
    expect: '... ... undefined'
  },
  {
    send: 'function x(s) {\nreturn s.replace(/"/,"");\n}',
    expect: '... ... undefined'
  },
  {
    send: 'function x(s) {\nreturn s.replace(/.*/,"");\n}',
    expect: '... ... undefined'
  },
  {
    send: '{ var x = 4; }',
    expect: 'undefined'
  },
  // Illegal token is not recoverable outside string literal, RegExp literal,
  // or block comment. https://github.com/nodejs/node/issues/3611
  {
    send: 'a = 3.5e',
    expect: [
      kSource,
      kArrow,
      '',
      /^SyntaxError: /,
      ''
    ]
  },
  // Mitigate https://github.com/nodejs/node/issues/548
  {
    send: 'function name(){ return "node"; };name()',
    expect: '\'node\''
  },
  {
    send: 'function name(){ return "nodejs"; };name()',
    expect: '\'nodejs\''
  },
  // Avoid emitting repl:line-number for SyntaxError
  {
    send: 'a = 3.5e',
    expect: [
      kSource,
      kArrow,
      '',
      /^SyntaxError: /,
      ''
    ]
  },
  // Avoid emitting stack trace
  {
    send: 'a = 3.5e',
    expect: [
      kSource,
      kArrow,
      '',
      /^SyntaxError: /,
      ''
    ]
  },

  // https://github.com/nodejs/node/issues/9850
  {
    send: 'function* foo() {}; foo().next();',
    expect: '{ value: undefined, done: true }'
  },

  {
    send: 'function *foo() {}; foo().next();',
    expect: '{ value: undefined, done: true }'
  },

  {
    send: 'function*foo() {}; foo().next();',
    expect: '{ value: undefined, done: true }'
  },

  {
    send: 'function * foo() {}; foo().next()',
    expect: '{ value: undefined, done: true }'
  },

  // https://github.com/nodejs/node/issues/9300
  {
    send: 'function foo() {\nvar bar = 1 / 1; // "/"\n}',
    expect: '... ... undefined'
  },

  {
    send: '(function() {\nreturn /foo/ / /bar/;\n}())',
    expect: '... ... NaN'
  },

  {
    send: '(function() {\nif (false) {} /bar"/;\n}())',
    expect: '... ... undefined'
  },

  // https://github.com/nodejs/node/issues/16483
  {
    send: 'new Proxy({x:42}, {get(){throw null}});',
    expect: 'Proxy [ { x: 42 }, { get: [Function: get] } ]'
  },
  {
    send: 'repl.writer.options.showProxy = false, new Proxy({x:42}, {});',
    expect: '{ x: 42 }'
  },

  // Newline within template string maintains whitespace.
  {
    send: '`foo \n`',
    expect: '... \'foo \\n\''
  },
  // Whitespace is not evaluated.
  {
    send: ' \t  \n',
    expect: 'undefined'
  },
  // Do not parse `...[]` as a REPL keyword
  {
    send: '...[]',
    expect: [
      kSource,
      kArrow,
      '',
      /^SyntaxError: /,
      ''
    ]
  },
  // bring back the repl to prompt
  {
    send: '.break',
    expect: ''
  },
  {
    send: 'console.log("Missing comma in arg list" process.version)',
    expect: [
      kSource,
      kArrow,
      '',
      /^SyntaxError: /,
      ''
    ]
  },
  {
    send: 'x = {\nfield\n{',
    expect: [
      '... ... {',
      kArrow,
      '',
      /^SyntaxError: /,
      ''
    ]
  },
  {
    send: '(2 + 3))',
    expect: [
      kSource,
      kArrow,
      '',
      /^SyntaxError: /,
      ''
    ]
  },
  {
    send: 'if (typeof process === "object"); {',
    expect: '... '
  },
  {
    send: 'console.log("process is defined");',
    expect: '... '
  },
  {
    send: '} else {',
    expect: [
      kSource,
      kArrow,
      '',
      /^SyntaxError: /,
      ''
    ]
  },
].filter((v) => !common.engineSpecificMessage(v));

const tcpTests = [
  {
    send: '',
    expect: ''
  },
  {
    send: 'invoke_me(333)',
    expect: '\'invoked 333\''
  },
  {
    send: 'a += 1',
    expect: '12346'
  },
  {
    send: `require(${JSON.stringify(moduleFilename)}).number`,
    expect: '42'
  }
];

(async function() {
  {
    const [ socket, replServer ] = await startUnixRepl();

    await runReplTests(socket, prompt_unix, unixTests);
    await runReplTests(socket, prompt_unix, errorTests);
    replServer.replMode = repl.REPL_MODE_STRICT;
    await runReplTests(socket, prompt_unix, strictModeTests);

    socket.end();
  }
  {
    const [ socket ] = await startTCPRepl();

    await runReplTests(socket, prompt_tcp, tcpTests);

    socket.end();
  }
})();

function startTCPRepl() {
  let resolveSocket, resolveReplServer;

  const server = net.createServer(common.mustCall((socket) => {
    assert.strictEqual(server, socket.server);

    socket.on('end', common.mustCall(() => {
      socket.end();
    }));

    resolveReplServer(repl.start(prompt_tcp, socket));
  }));

  server.listen(0, common.mustCall(() => {
    const client = net.createConnection(server.address().port);

    client.setEncoding('utf8');

    client.on('connect', common.mustCall(() => {
      assert.strictEqual(true, client.readable);
      assert.strictEqual(true, client.writable);

      resolveSocket(client);
    }));

    client.on('close', common.mustCall(() => {
      server.close();
    }));
  }));

  return Promise.all([
    new Promise((resolve) => resolveSocket = resolve),
    new Promise((resolve) => resolveReplServer = resolve)
  ]);
}

function startUnixRepl() {
  let resolveSocket, resolveReplServer;

  const server = net.createServer(common.mustCall((socket) => {
    assert.strictEqual(server, socket.server);

    socket.on('end', common.mustCall(() => {
      socket.end();
    }));

    const replServer = repl.start({
      prompt: prompt_unix,
      input: socket,
      output: socket,
      useGlobal: true
    });
    replServer.context.message = message;
    resolveReplServer(replServer);
  }));

  server.listen(common.PIPE, common.mustCall(() => {
    const client = net.createConnection(common.PIPE);

    client.setEncoding('utf8');

    client.on('connect', common.mustCall(() => {
      assert.strictEqual(true, client.readable);
      assert.strictEqual(true, client.writable);

      resolveSocket(client);
    }));

    client.on('close', common.mustCall(() => {
      server.close();
    }));
  }));

  return Promise.all([
    new Promise((resolve) => resolveSocket = resolve),
    new Promise((resolve) => resolveReplServer = resolve)
  ]);
}

function event(ee, eventName) {
  return new Promise((resolve) => {
    ee.once(eventName, common.mustCall(resolve));
  });
}
