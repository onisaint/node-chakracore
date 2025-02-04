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

const {
  byteLengthUtf8,
  copy: _copy,
  compare: _compare,
  compareOffset,
  createFromString,
  fill: bindingFill,
  indexOfBuffer,
  indexOfNumber,
  indexOfString,
  swap16: _swap16,
  swap32: _swap32,
  swap64: _swap64,
  kMaxLength,
  kStringMaxLength
} = process.binding('buffer');
// We cannot use internalBinding unconditionally here because of the way
// that test/parallel/test-buffer-bindingobj-no-zerofill.js is written.
let isAnyArrayBuffer;
try {
  const { internalBinding } = require('internal/bootstrap_loaders');
  isAnyArrayBuffer = internalBinding('types').isAnyArrayBuffer;
} catch (e) {
  isAnyArrayBuffer = require('util').types.isAnyArrayBuffer;
}
const {
  customInspectSymbol,
  normalizeEncoding,
  kIsEncodingSymbol
} = require('internal/util');
const {
  isArrayBufferView,
  isUint8Array
} = require('internal/util/types');
const {
  pendingDeprecation
} = process.binding('config');
const {
  ERR_BUFFER_OUT_OF_BOUNDS,
  ERR_INDEX_OUT_OF_RANGE,
  ERR_INVALID_ARG_TYPE,
  ERR_INVALID_ARG_VALUE,
  ERR_INVALID_BUFFER_SIZE,
  ERR_INVALID_OPT_VALUE,
  ERR_NO_LONGER_SUPPORTED,
  ERR_UNKNOWN_ENCODING
} = require('internal/errors').codes;

const internalBuffer = require('internal/buffer');

const { setupBufferJS } = internalBuffer;

const bindingObj = {};

class FastBuffer extends Uint8Array {
  constructor(arg1, arg2, arg3) {
    super(arg1, arg2, arg3);
  }
}
FastBuffer.prototype.constructor = Buffer;
internalBuffer.FastBuffer = FastBuffer;

Buffer.prototype = FastBuffer.prototype;

for (const [name, method] of Object.entries(internalBuffer.readWrites)) {
  Buffer.prototype[name] = method;
}

const constants = Object.defineProperties({}, {
  MAX_LENGTH: {
    value: kMaxLength,
    writable: false,
    enumerable: true
  },
  MAX_STRING_LENGTH: {
    value: kStringMaxLength,
    writable: false,
    enumerable: true
  }
});

Buffer.poolSize = 8 * 1024;
var poolSize, poolOffset, allocPool;

setupBufferJS(Buffer.prototype, bindingObj);

// |zeroFill| can be undefined when running inside an isolate where we
// do not own the ArrayBuffer allocator.  Zero fill is always on in that case.
const zeroFill = bindingObj.zeroFill || [0];

function createUnsafeBuffer(size) {
  return new FastBuffer(createUnsafeArrayBuffer(size));
}

function createUnsafeArrayBuffer(size) {
  zeroFill[0] = 0;
  try {
    return new ArrayBuffer(size);
  } finally {
    zeroFill[0] = 1;
  }
}

function createPool() {
  poolSize = Buffer.poolSize;
  allocPool = createUnsafeArrayBuffer(poolSize);
  poolOffset = 0;
}
createPool();

function alignPool() {
  // Ensure aligned slices
  if (poolOffset & 0x7) {
    poolOffset |= 0x7;
    poolOffset++;
  }
}

// Mark a constructor not supporting @@species
function markNoSpeciesConstructor(constructor) {
  if (Symbol.species) {
    Object.defineProperty(constructor, Symbol.species, {
      get: function() { return undefined; },
      configurable: true,
    });
  }
}

var bufferWarn = true;
const bufferWarning = 'The Buffer() and new Buffer() constructors are not ' +
                      'recommended for use due to security and usability ' +
                      'concerns. Please use the new Buffer.alloc(), ' +
                      'Buffer.allocUnsafe(), or Buffer.from() construction ' +
                      'methods instead.';

function showFlaggedDeprecation() {
  if (bufferWarn) {
    // This is a *pending* deprecation warning. It is not emitted by
    // default unless the --pending-deprecation command-line flag is
    // used or the NODE_PENDING_DEPRECATION=1 env var is set.
    process.emitWarning(bufferWarning, 'DeprecationWarning', 'DEP0005');
    bufferWarn = false;
  }
}

const doFlaggedDeprecation =
  pendingDeprecation ?
    showFlaggedDeprecation :
    function() {};

/**
 * The Buffer() constructor is deprecated in documentation and should not be
 * used moving forward. Rather, developers should use one of the three new
 * factory APIs: Buffer.from(), Buffer.allocUnsafe() or Buffer.alloc() based on
 * their specific needs. There is no runtime deprecation because of the extent
 * to which the Buffer constructor is used in the ecosystem currently -- a
 * runtime deprecation would introduce too much breakage at this time. It's not
 * likely that the Buffer constructors would ever actually be removed.
 * Deprecation Code: DEP0005
 **/
function Buffer(arg, encodingOrOffset, length) {
  doFlaggedDeprecation();
  // Common case.
  if (typeof arg === 'number') {
    if (typeof encodingOrOffset === 'string') {
      throw new ERR_INVALID_ARG_TYPE('string', 'string', arg);
    }
    return Buffer.alloc(arg);
  }
  return Buffer.from(arg, encodingOrOffset, length);
}

Object.defineProperty(Buffer, Symbol.species, {
  enumerable: false,
  configurable: true,
  get() { return FastBuffer; }
});

/**
 * Functionally equivalent to Buffer(arg, encoding) but throws a TypeError
 * if value is a number.
 * Buffer.from(str[, encoding])
 * Buffer.from(array)
 * Buffer.from(buffer)
 * Buffer.from(arrayBuffer[, byteOffset[, length]])
 **/
Buffer.from = function from(value, encodingOrOffset, length) {
  if (typeof value === 'string')
    return fromString(value, encodingOrOffset);

  if (isAnyArrayBuffer(value))
    return fromArrayBuffer(value, encodingOrOffset, length);

  if (value === null || value === undefined) {
    throw new ERR_INVALID_ARG_TYPE(
      'first argument',
      ['string', 'Buffer', 'ArrayBuffer', 'Array', 'Array-like Object'],
      value
    );
  }

  if (typeof value === 'number') {
    throw new ERR_INVALID_ARG_TYPE('value', 'not number', value);
  }

  const valueOf = value.valueOf && value.valueOf();
  if (valueOf !== null && valueOf !== undefined && valueOf !== value)
    return Buffer.from(valueOf, encodingOrOffset, length);

  var b = fromObject(value);
  if (b)
    return b;

  if (typeof value[Symbol.toPrimitive] === 'function') {
    return Buffer.from(value[Symbol.toPrimitive]('string'),
                       encodingOrOffset,
                       length);
  }

  throw new ERR_INVALID_ARG_TYPE(
    'first argument',
    ['string', 'Buffer', 'ArrayBuffer', 'Array', 'Array-like Object'],
    value
  );
};

Object.setPrototypeOf(Buffer, Uint8Array);
markNoSpeciesConstructor(Buffer);

// The 'assertSize' method will remove itself from the callstack when an error
// occurs. This is done simply to keep the internal details of the
// implementation from bleeding out to users.
function assertSize(size) {
  let err = null;

  if (typeof size !== 'number') {
    err = new ERR_INVALID_ARG_TYPE('size', 'number', size);
  } else if (size < 0 || size > kMaxLength) {
    err = new ERR_INVALID_OPT_VALUE.RangeError('size', size);
  }

  if (err !== null) {
    Error.captureStackTrace(err, assertSize);
    throw err;
  }
}

/**
 * Creates a new filled Buffer instance.
 * alloc(size[, fill[, encoding]])
 **/
Buffer.alloc = function alloc(size, fill, encoding) {
  assertSize(size);
  if (fill !== undefined && size > 0) {
    return _fill(createUnsafeBuffer(size), fill, encoding);
  }
  return new FastBuffer(size);
};

/**
 * Equivalent to Buffer(num), by default creates a non-zero-filled Buffer
 * instance. If `--zero-fill-buffers` is set, will zero-fill the buffer.
 **/
Buffer.allocUnsafe = function allocUnsafe(size) {
  assertSize(size);
  return allocate(size);
};

/**
 * Equivalent to SlowBuffer(num), by default creates a non-zero-filled
 * Buffer instance that is not allocated off the pre-initialized pool.
 * If `--zero-fill-buffers` is set, will zero-fill the buffer.
 **/
Buffer.allocUnsafeSlow = function allocUnsafeSlow(size) {
  assertSize(size);
  return createUnsafeBuffer(size);
};

// If --zero-fill-buffers command line argument is set, a zero-filled
// buffer is returned.
function SlowBuffer(length) {
  // eslint-disable-next-line eqeqeq
  if (+length != length)
    length = 0;
  assertSize(+length);
  return createUnsafeBuffer(+length);
}

Object.setPrototypeOf(SlowBuffer.prototype, Uint8Array.prototype);
Object.setPrototypeOf(SlowBuffer, Uint8Array);
markNoSpeciesConstructor(SlowBuffer);

function allocate(size) {
  if (size <= 0) {
    return new FastBuffer();
  }
  if (size < (Buffer.poolSize >>> 1)) {
    if (size > (poolSize - poolOffset))
      createPool();
    var b = new FastBuffer(allocPool, poolOffset, size);
    poolOffset += size;
    alignPool();
    return b;
  } else {
    return createUnsafeBuffer(size);
  }
}

function fromString(string, encoding) {
  var length;
  if (typeof encoding !== 'string' || encoding.length === 0) {
    if (string.length === 0)
      return new FastBuffer();
    encoding = 'utf8';
    length = byteLengthUtf8(string);
  } else {
    length = byteLength(string, encoding, true);
    if (length === -1)
      throw new ERR_UNKNOWN_ENCODING(encoding);
    if (string.length === 0)
      return new FastBuffer();
  }

  if (length >= (Buffer.poolSize >>> 1))
    return createFromString(string, encoding);

  if (length > (poolSize - poolOffset))
    createPool();
  var b = new FastBuffer(allocPool, poolOffset, length);
  const actual = b.write(string, encoding);
  if (actual !== length) {
    // byteLength() may overestimate. That's a rare case, though.
    b = new FastBuffer(allocPool, poolOffset, actual);
  }
  poolOffset += actual;
  alignPool();
  return b;
}

function fromArrayLike(obj) {
  const length = obj.length;
  const b = allocate(length);
  for (var i = 0; i < length; i++)
    b[i] = obj[i];
  return b;
}

function fromArrayBuffer(obj, byteOffset, length) {
  // convert byteOffset to integer
  if (byteOffset === undefined) {
    byteOffset = 0;
  } else {
    byteOffset = +byteOffset;
    if (Number.isNaN(byteOffset))
      byteOffset = 0;
  }

  const maxLength = obj.byteLength - byteOffset;

  if (maxLength < 0)
    throw new ERR_BUFFER_OUT_OF_BOUNDS('offset');

  if (length === undefined) {
    length = maxLength;
  } else {
    // Convert length to non-negative integer.
    length = +length;
    if (length > 0) {
      if (length > maxLength)
        throw new ERR_BUFFER_OUT_OF_BOUNDS('length');
    } else {
      length = 0;
    }
  }

  return new FastBuffer(obj, byteOffset, length);
}

function fromObject(obj) {
  if (isUint8Array(obj)) {
    const b = allocate(obj.length);

    if (b.length === 0)
      return b;

    _copy(obj, b, 0, 0, obj.length);
    return b;
  }

  if (obj.length !== undefined || isAnyArrayBuffer(obj.buffer)) {
    if (typeof obj.length !== 'number') {
      return new FastBuffer();
    }
    return fromArrayLike(obj);
  }

  if (obj.type === 'Buffer' && Array.isArray(obj.data)) {
    return fromArrayLike(obj.data);
  }
}

// Static methods

Buffer.isBuffer = function isBuffer(b) {
  return b instanceof Buffer;
};

Buffer.compare = function compare(a, b) {
  if (!isUint8Array(a) || !isUint8Array(b)) {
    throw new ERR_INVALID_ARG_TYPE(['buf1', 'buf2'], ['Buffer', 'Uint8Array']);
  }

  if (a === b) {
    return 0;
  }

  return _compare(a, b);
};

Buffer.isEncoding = function isEncoding(encoding) {
  return typeof encoding === 'string' && encoding.length !== 0 &&
         normalizeEncoding(encoding) !== undefined;
};
Buffer[kIsEncodingSymbol] = Buffer.isEncoding;

Buffer.concat = function concat(list, length) {
  var i;
  if (!Array.isArray(list)) {
    throw new ERR_INVALID_ARG_TYPE('list', ['Array', 'Buffer', 'Uint8Array']);
  }

  if (list.length === 0)
    return new FastBuffer();

  if (length === undefined) {
    length = 0;
    for (i = 0; i < list.length; i++)
      length += list[i].length;
  } else {
    length = length >>> 0;
  }

  var buffer = Buffer.allocUnsafe(length);
  var pos = 0;
  for (i = 0; i < list.length; i++) {
    var buf = list[i];
    if (!isUint8Array(buf)) {
      throw new ERR_INVALID_ARG_TYPE('list', ['Array', 'Buffer', 'Uint8Array']);
    }
    _copy(buf, buffer, pos);
    pos += buf.length;
  }

  // Note: `length` is always equal to `buffer.length` at this point
  if (pos < length) {
    // Zero-fill the remaining bytes if the specified `length` was more than
    // the actual total length, i.e. if we have some remaining allocated bytes
    // there were not initialized.
    buffer.fill(0, pos, length);
  }

  return buffer;
};

function base64ByteLength(str, bytes) {
  // Handle padding
  if (str.charCodeAt(bytes - 1) === 0x3D)
    bytes--;
  if (bytes > 1 && str.charCodeAt(bytes - 1) === 0x3D)
    bytes--;

  // Base64 ratio: 3/4
  return (bytes * 3) >>> 2;
}

function byteLength(string, encoding) {
  if (typeof string !== 'string') {
    if (isArrayBufferView(string) || isAnyArrayBuffer(string)) {
      return string.byteLength;
    }

    throw new ERR_INVALID_ARG_TYPE(
      'string', ['string', 'Buffer', 'ArrayBuffer'], string
    );
  }

  const len = string.length;
  const mustMatch = (arguments.length > 2 && arguments[2] === true);
  if (!mustMatch && len === 0)
    return 0;

  if (!encoding)
    return (mustMatch ? -1 : byteLengthUtf8(string));

  encoding += '';
  switch (encoding.length) {
    case 4:
      if (encoding === 'utf8') return byteLengthUtf8(string);
      if (encoding === 'ucs2') return len * 2;
      encoding = encoding.toLowerCase();
      if (encoding === 'utf8') return byteLengthUtf8(string);
      if (encoding === 'ucs2') return len * 2;
      break;
    case 5:
      if (encoding === 'utf-8') return byteLengthUtf8(string);
      if (encoding === 'ascii') return len;
      if (encoding === 'ucs-2') return len * 2;
      encoding = encoding.toLowerCase();
      if (encoding === 'utf-8') return byteLengthUtf8(string);
      if (encoding === 'ascii') return len;
      if (encoding === 'ucs-2') return len * 2;
      break;
    case 7:
      if (encoding === 'utf16le' || encoding.toLowerCase() === 'utf16le')
        return len * 2;
      break;
    case 8:
      if (encoding === 'utf-16le' || encoding.toLowerCase() === 'utf-16le')
        return len * 2;
      break;
    case 6:
      if (encoding === 'latin1' || encoding === 'binary') return len;
      if (encoding === 'base64') return base64ByteLength(string, len);
      encoding = encoding.toLowerCase();
      if (encoding === 'latin1' || encoding === 'binary') return len;
      if (encoding === 'base64') return base64ByteLength(string, len);
      break;
    case 3:
      if (encoding === 'hex' || encoding.toLowerCase() === 'hex')
        return len >>> 1;
      break;
  }
  return (mustMatch ? -1 : byteLengthUtf8(string));
}

Buffer.byteLength = byteLength;

// For backwards compatibility.
Object.defineProperty(Buffer.prototype, 'parent', {
  enumerable: true,
  get() {
    if (!(this instanceof Buffer))
      return undefined;
    return this.buffer;
  }
});
Object.defineProperty(Buffer.prototype, 'offset', {
  enumerable: true,
  get() {
    if (!(this instanceof Buffer))
      return undefined;
    return this.byteOffset;
  }
});

function stringSlice(buf, encoding, start, end) {
  if (encoding === undefined) return buf.utf8Slice(start, end);
  encoding += '';
  switch (encoding.length) {
    case 4:
      if (encoding === 'utf8') return buf.utf8Slice(start, end);
      if (encoding === 'ucs2') return buf.ucs2Slice(start, end);
      encoding = encoding.toLowerCase();
      if (encoding === 'utf8') return buf.utf8Slice(start, end);
      if (encoding === 'ucs2') return buf.ucs2Slice(start, end);
      break;
    case 5:
      if (encoding === 'utf-8') return buf.utf8Slice(start, end);
      if (encoding === 'ascii') return buf.asciiSlice(start, end);
      if (encoding === 'ucs-2') return buf.ucs2Slice(start, end);
      encoding = encoding.toLowerCase();
      if (encoding === 'utf-8') return buf.utf8Slice(start, end);
      if (encoding === 'ascii') return buf.asciiSlice(start, end);
      if (encoding === 'ucs-2') return buf.ucs2Slice(start, end);
      break;
    case 6:
      if (encoding === 'latin1' || encoding === 'binary')
        return buf.latin1Slice(start, end);
      if (encoding === 'base64') return buf.base64Slice(start, end);
      encoding = encoding.toLowerCase();
      if (encoding === 'latin1' || encoding === 'binary')
        return buf.latin1Slice(start, end);
      if (encoding === 'base64') return buf.base64Slice(start, end);
      break;
    case 3:
      if (encoding === 'hex' || encoding.toLowerCase() === 'hex')
        return buf.hexSlice(start, end);
      break;
    case 7:
      if (encoding === 'utf16le' || encoding.toLowerCase() === 'utf16le')
        return buf.ucs2Slice(start, end);
      break;
    case 8:
      if (encoding === 'utf-16le' || encoding.toLowerCase() === 'utf-16le')
        return buf.ucs2Slice(start, end);
      break;
  }
  throw new ERR_UNKNOWN_ENCODING(encoding);
}

Buffer.prototype.copy =
  function copy(target, targetStart, sourceStart, sourceEnd) {
    return _copy(this, target, targetStart, sourceStart, sourceEnd);
  };

// No need to verify that "buf.length <= MAX_UINT32" since it's a read-only
// property of a typed array.
// This behaves neither like String nor Uint8Array in that we set start/end
// to their upper/lower bounds if the value passed is out of range.
Buffer.prototype.toString = function toString(encoding, start, end) {
  if (arguments.length === 0) {
    return this.utf8Slice(0, this.length);
  }

  const len = this.length;
  if (len === 0)
    return '';

  if (!start || start < 0)
    start = 0;
  else if (start >= len)
    return '';

  if (end === undefined || end > len)
    end = len;
  else if (end <= 0)
    return '';

  start |= 0;
  end |= 0;

  if (end <= start)
    return '';
  return stringSlice(this, encoding, start, end);
};

Buffer.prototype.equals = function equals(b) {
  if (!isUint8Array(b)) {
    throw new ERR_INVALID_ARG_TYPE('otherBuffer', ['Buffer', 'Uint8Array'], b);
  }
  if (this === b)
    return true;

  return _compare(this, b) === 0;
};

// Override how buffers are presented by util.inspect().
Buffer.prototype[customInspectSymbol] = function inspect() {
  var str = '';
  var max = exports.INSPECT_MAX_BYTES;
  str = this.toString('hex', 0, max).replace(/(.{2})/g, '$1 ').trim();
  if (this.length > max)
    str += ' ... ';
  return `<${this.constructor.name} ${str}>`;
};
Buffer.prototype.inspect = Buffer.prototype[customInspectSymbol];

Buffer.prototype.compare = function compare(target,
                                            start,
                                            end,
                                            thisStart,
                                            thisEnd) {
  if (!isUint8Array(target)) {
    throw new ERR_INVALID_ARG_TYPE('target', ['Buffer', 'Uint8Array'], target);
  }
  if (arguments.length === 1)
    return _compare(this, target);

  if (start === undefined)
    start = 0;
  else if (start < 0)
    throw new ERR_INDEX_OUT_OF_RANGE();
  else
    start >>>= 0;

  if (end === undefined)
    end = target.length;
  else if (end > target.length)
    throw new ERR_INDEX_OUT_OF_RANGE();
  else
    end >>>= 0;

  if (thisStart === undefined)
    thisStart = 0;
  else if (thisStart < 0)
    throw new ERR_INDEX_OUT_OF_RANGE();
  else
    thisStart >>>= 0;

  if (thisEnd === undefined)
    thisEnd = this.length;
  else if (thisEnd > this.length)
    throw new ERR_INDEX_OUT_OF_RANGE();
  else
    thisEnd >>>= 0;

  if (thisStart >= thisEnd)
    return (start >= end ? 0 : -1);
  else if (start >= end)
    return 1;

  return compareOffset(this, target, start, thisStart, end, thisEnd);
};

// Finds either the first index of `val` in `buffer` at offset >= `byteOffset`,
// OR the last index of `val` in `buffer` at offset <= `byteOffset`.
//
// Arguments:
// - buffer - a Buffer to search
// - val - a string, Buffer, or number
// - byteOffset - an index into `buffer`; will be clamped to an int32
// - encoding - an optional encoding, relevant if val is a string
// - dir - true for indexOf, false for lastIndexOf
function bidirectionalIndexOf(buffer, val, byteOffset, encoding, dir) {
  if (typeof byteOffset === 'string') {
    encoding = byteOffset;
    byteOffset = undefined;
  } else if (byteOffset > 0x7fffffff) {
    byteOffset = 0x7fffffff;
  } else if (byteOffset < -0x80000000) {
    byteOffset = -0x80000000;
  }
  // Coerce to Number. Values like null and [] become 0.
  byteOffset = +byteOffset;
  // If the offset is undefined, "foo", {}, coerces to NaN, search whole buffer.
  if (Number.isNaN(byteOffset)) {
    byteOffset = dir ? 0 : buffer.length;
  }
  dir = !!dir;  // Cast to bool.

  if (typeof val === 'string') {
    if (encoding === undefined) {
      return indexOfString(buffer, val, byteOffset, encoding, dir);
    }
    return slowIndexOf(buffer, val, byteOffset, encoding, dir);
  } else if (isUint8Array(val)) {
    return indexOfBuffer(buffer, val, byteOffset, encoding, dir);
  } else if (typeof val === 'number') {
    return indexOfNumber(buffer, val, byteOffset, dir);
  }

  throw new ERR_INVALID_ARG_TYPE(
    'value', ['string', 'Buffer', 'Uint8Array'], val
  );
}

function slowIndexOf(buffer, val, byteOffset, encoding, dir) {
  var loweredCase = false;
  for (;;) {
    switch (encoding) {
      case 'utf8':
      case 'utf-8':
      case 'ucs2':
      case 'ucs-2':
      case 'utf16le':
      case 'utf-16le':
      case 'latin1':
      case 'binary':
        return indexOfString(buffer, val, byteOffset, encoding, dir);

      case 'base64':
      case 'ascii':
      case 'hex':
        return indexOfBuffer(
          buffer, Buffer.from(val, encoding), byteOffset, encoding, dir);

      default:
        if (loweredCase) {
          throw new ERR_UNKNOWN_ENCODING(encoding);
        }

        encoding = ('' + encoding).toLowerCase();
        loweredCase = true;
    }
  }
}

Buffer.prototype.indexOf = function indexOf(val, byteOffset, encoding) {
  return bidirectionalIndexOf(this, val, byteOffset, encoding, true);
};

Buffer.prototype.lastIndexOf = function lastIndexOf(val, byteOffset, encoding) {
  return bidirectionalIndexOf(this, val, byteOffset, encoding, false);
};

Buffer.prototype.includes = function includes(val, byteOffset, encoding) {
  return this.indexOf(val, byteOffset, encoding) !== -1;
};

// Usage:
//    buffer.fill(number[, offset[, end]])
//    buffer.fill(buffer[, offset[, end]])
//    buffer.fill(string[, offset[, end]][, encoding])
Buffer.prototype.fill = function fill(val, start, end, encoding) {
  return _fill(this, val, start, end, encoding);
};

function _fill(buf, val, start, end, encoding) {
  if (typeof val === 'string') {
    if (start === undefined || typeof start === 'string') {
      encoding = start;
      start = 0;
      end = buf.length;
    } else if (typeof end === 'string') {
      encoding = end;
      end = buf.length;
    }

    const normalizedEncoding = normalizeEncoding(encoding);
    if (normalizedEncoding === undefined) {
      if (typeof encoding !== 'string') {
        throw new ERR_INVALID_ARG_TYPE('encoding', 'string', encoding);
      }
      throw new ERR_UNKNOWN_ENCODING(encoding);
    }

    if (val.length === 0) {
      // If val === '' default to zero.
      val = 0;
    } else if (val.length === 1) {
      // Fast path: If `val` fits into a single byte, use that numeric value.
      if (normalizedEncoding === 'utf8') {
        const code = val.charCodeAt(0);
        if (code < 128) {
          val = code;
        }
      } else if (normalizedEncoding === 'latin1') {
        val = val.charCodeAt(0);
      }
    }
  } else {
    encoding = undefined;
  }

  if (start === undefined) {
    start = 0;
    end = buf.length;
  } else {
    // Invalid ranges are not set to a default, so can range check early.
    if (end === undefined) {
      if (start < 0)
        throw new ERR_INDEX_OUT_OF_RANGE();
      end = buf.length;
    } else {
      if (start < 0 || end > buf.length || end < 0)
        throw new ERR_INDEX_OUT_OF_RANGE();
      end = end >>> 0;
    }
    start = start >>> 0;
    if (start >= end)
      return buf;
  }

  const res = bindingFill(buf, val, start, end, encoding);
  if (res < 0) {
    if (res === -1)
      throw new ERR_INVALID_ARG_VALUE('value', val);
    throw new ERR_INDEX_OUT_OF_RANGE();
  }

  return buf;
}

Buffer.prototype.write = function write(string, offset, length, encoding) {
  // Buffer#write(string);
  if (offset === undefined) {
    return this.utf8Write(string, 0, this.length);

  // Buffer#write(string, encoding)
  } else if (length === undefined && typeof offset === 'string') {
    encoding = offset;
    length = this.length;
    offset = 0;

  // Buffer#write(string, offset[, length][, encoding])
  } else if (isFinite(offset)) {
    offset = offset >>> 0;
    if (isFinite(length)) {
      length = length >>> 0;
    } else {
      encoding = length;
      length = undefined;
    }

    var remaining = this.length - offset;
    if (length === undefined || length > remaining)
      length = remaining;

    if (string.length > 0 && (length < 0 || offset < 0))
      throw new ERR_BUFFER_OUT_OF_BOUNDS('length', true);
  } else {
    // if someone is still calling the obsolete form of write(), tell them.
    // we don't want eg buf.write("foo", "utf8", 10) to silently turn into
    // buf.write("foo", "utf8"), so we can't ignore extra args
    throw new ERR_NO_LONGER_SUPPORTED(
      'Buffer.write(string, encoding, offset[, length])'
    );
  }

  if (!encoding) return this.utf8Write(string, offset, length);

  encoding += '';
  switch (encoding.length) {
    case 4:
      if (encoding === 'utf8') return this.utf8Write(string, offset, length);
      if (encoding === 'ucs2') return this.ucs2Write(string, offset, length);
      encoding = encoding.toLowerCase();
      if (encoding === 'utf8') return this.utf8Write(string, offset, length);
      if (encoding === 'ucs2') return this.ucs2Write(string, offset, length);
      break;
    case 5:
      if (encoding === 'utf-8') return this.utf8Write(string, offset, length);
      if (encoding === 'ascii') return this.asciiWrite(string, offset, length);
      if (encoding === 'ucs-2') return this.ucs2Write(string, offset, length);
      encoding = encoding.toLowerCase();
      if (encoding === 'utf-8') return this.utf8Write(string, offset, length);
      if (encoding === 'ascii') return this.asciiWrite(string, offset, length);
      if (encoding === 'ucs-2') return this.ucs2Write(string, offset, length);
      break;
    case 7:
      if (encoding === 'utf16le' || encoding.toLowerCase() === 'utf16le')
        return this.ucs2Write(string, offset, length);
      break;
    case 8:
      if (encoding === 'utf-16le' || encoding.toLowerCase() === 'utf-16le')
        return this.ucs2Write(string, offset, length);
      break;
    case 6:
      if (encoding === 'latin1' || encoding === 'binary')
        return this.latin1Write(string, offset, length);
      if (encoding === 'base64')
        return this.base64Write(string, offset, length);
      encoding = encoding.toLowerCase();
      if (encoding === 'latin1' || encoding === 'binary')
        return this.latin1Write(string, offset, length);
      if (encoding === 'base64')
        return this.base64Write(string, offset, length);
      break;
    case 3:
      if (encoding === 'hex' || encoding.toLowerCase() === 'hex')
        return this.hexWrite(string, offset, length);
      break;
  }
  throw new ERR_UNKNOWN_ENCODING(encoding);
};

Buffer.prototype.toJSON = function toJSON() {
  if (this.length > 0) {
    const data = new Array(this.length);
    for (var i = 0; i < this.length; ++i)
      data[i] = this[i];
    return { type: 'Buffer', data };
  } else {
    return { type: 'Buffer', data: [] };
  }
};

function adjustOffset(offset, length) {
  // Use Math.trunc() to convert offset to an integer value that can be larger
  // than an Int32. Hence, don't use offset | 0 or similar techniques.
  offset = Math.trunc(offset);
  if (offset === 0) {
    return 0;
  }
  if (offset < 0) {
    offset += length;
    return offset > 0 ? offset : 0;
  }
  if (offset < length) {
    return offset;
  }
  return Number.isNaN(offset) ? 0 : length;
}

Buffer.prototype.slice = function slice(start, end) {
  const srcLength = this.length;
  start = adjustOffset(start, srcLength);
  end = end !== undefined ? adjustOffset(end, srcLength) : srcLength;
  const newLength = end > start ? end - start : 0;
  return new FastBuffer(this.buffer, this.byteOffset + start, newLength);
};

function swap(b, n, m) {
  const i = b[n];
  b[n] = b[m];
  b[m] = i;
}

Buffer.prototype.swap16 = function swap16() {
  // For Buffer.length < 128, it's generally faster to
  // do the swap in javascript. For larger buffers,
  // dropping down to the native code is faster.
  const len = this.length;
  if (len % 2 !== 0)
    throw new ERR_INVALID_BUFFER_SIZE('16-bits');
  if (len < 128) {
    for (var i = 0; i < len; i += 2)
      swap(this, i, i + 1);
    return this;
  }
  return _swap16(this);
};

Buffer.prototype.swap32 = function swap32() {
  // For Buffer.length < 192, it's generally faster to
  // do the swap in javascript. For larger buffers,
  // dropping down to the native code is faster.
  const len = this.length;
  if (len % 4 !== 0)
    throw new ERR_INVALID_BUFFER_SIZE('32-bits');
  if (len < 192) {
    for (var i = 0; i < len; i += 4) {
      swap(this, i, i + 3);
      swap(this, i + 1, i + 2);
    }
    return this;
  }
  return _swap32(this);
};

Buffer.prototype.swap64 = function swap64() {
  // For Buffer.length < 192, it's generally faster to
  // do the swap in javascript. For larger buffers,
  // dropping down to the native code is faster.
  const len = this.length;
  if (len % 8 !== 0)
    throw new ERR_INVALID_BUFFER_SIZE('64-bits');
  if (len < 192) {
    for (var i = 0; i < len; i += 8) {
      swap(this, i, i + 7);
      swap(this, i + 1, i + 6);
      swap(this, i + 2, i + 5);
      swap(this, i + 3, i + 4);
    }
    return this;
  }
  return _swap64(this);
};

Buffer.prototype.toLocaleString = Buffer.prototype.toString;

let transcode;
if (process.binding('config').hasIntl) {
  const {
    icuErrName,
    transcode: _transcode
  } = process.binding('icu');

  // Transcodes the Buffer from one encoding to another, returning a new
  // Buffer instance.
  transcode = function transcode(source, fromEncoding, toEncoding) {
    if (!isUint8Array(source)) {
      throw new ERR_INVALID_ARG_TYPE('source',
                                     ['Buffer', 'Uint8Array'], source);
    }
    if (source.length === 0) return Buffer.alloc(0);

    fromEncoding = normalizeEncoding(fromEncoding) || fromEncoding;
    toEncoding = normalizeEncoding(toEncoding) || toEncoding;
    const result = _transcode(source, fromEncoding, toEncoding);
    if (typeof result !== 'number')
      return result;

    const code = icuErrName(result);
    const err = new Error(`Unable to transcode Buffer [${code}]`);
    err.code = code;
    err.errno = result;
    throw err;
  };
}

module.exports = exports = {
  Buffer,
  SlowBuffer,
  transcode,
  INSPECT_MAX_BYTES: 50,

  // Legacy
  kMaxLength,
  kStringMaxLength
};

Object.defineProperty(exports, 'constants', {
  configurable: false,
  enumerable: true,
  value: constants
});
