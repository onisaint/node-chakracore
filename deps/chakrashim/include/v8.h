// Copyright 2012 the V8 project authors. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of Google Inc. nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef DEPS_CHAKRASHIM_INCLUDE_V8_H_
#define DEPS_CHAKRASHIM_INCLUDE_V8_H_

// Stops windows.h from including winsock.h (conflicting with winsock2.h).
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif

#if defined(_M_ARM)
#define CHAKRA_MIN_WIN32_WINNT _WIN32_WINNT_WIN10
#define CHAKRA_MIN_WIN32_WINNT_STR "_WIN32_WINNT_WIN10"
#else
#define CHAKRA_MIN_WIN32_WINNT _WIN32_WINNT_WIN7
#define CHAKRA_MIN_WIN32_WINNT_STR "_WIN32_WINNT_WIN7"
#endif

#if defined(_WIN32_WINNT) && (_WIN32_WINNT < CHAKRA_MIN_WIN32_WINNT)
#pragma message("warning: chakrashim requires minimum " \
                CHAKRA_MIN_WIN32_WINNT_STR \
                ". Redefine _WIN32_WINNT to " \
                CHAKRA_MIN_WIN32_WINNT_STR ".")
#undef _WIN32_WINNT
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT CHAKRA_MIN_WIN32_WINNT
#endif

#ifndef USE_EDGEMODE_JSRT
#define USE_EDGEMODE_JSRT     // Only works with edge JSRT
#endif

#if !defined(OSX_SDK_TR1) && defined(__APPLE__)
#include <AvailabilityMacros.h>
#if __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < MAC_OS_X_VERSION_10_9
#define OSX_SDK_TR1
#endif
#endif

#include <memory>
#include "ChakraCore.h"
#include "v8-version.h"
#include "v8config.h"

#ifdef _WIN32
#ifdef BUILDING_CHAKRASHIM
#define V8_EXPORT __declspec(dllexport)
#else
#define V8_EXPORT __declspec(dllimport)
#endif
#else  // !_WIN32
#ifdef BUILDING_CHAKRASHIM
#define V8_EXPORT __attribute__ ((visibility("default")))
#else
#define V8_EXPORT
#endif
#endif

#define TYPE_CHECK(T, S)                                       \
  while (false) {                                              \
    *(static_cast<T* volatile*>(0)) = static_cast<S*>(0);      \
  }

// Used to allow n-api constructs access to shim internals
// This is a temporary workaround and should go away once
// node-core has less of a dependency on the shim
namespace uvimpl {
class Work;
};

namespace v8 {
class PropertyDescriptor;
}

namespace jsrt {
class IsolateShim;

JsErrorCode CreateV8PropertyDescriptor(JsValueRef descriptor,
  v8::PropertyDescriptor* result);
}

namespace v8 {

class AccessorSignature;
class Array;
class Value;
class External;
class Primitive;
class Boolean;
class BooleanObject;
class Context;
class CpuProfiler;
class EscapableHandleScope;
class Function;
class FunctionTemplate;
class HeapProfiler;
class Int32;
class Integer;
class Isolate;
class Module;
class Name;
class Number;
class NumberObject;
class Object;
class ObjectTemplate;
class Platform;
class ResourceConstraints;
class RegExp;
class Promise;
class PropertyDescriptor;
class Proxy;
class Script;
class ScriptOrModule;
class Signature;
class StartupData;
class StackFrame;
class StackTrace;
class String;
class StringObject;
class Uint32;
template <class T> class Local;
template <class T> class Maybe;
template <class T> class MaybeLocal;
template <class T> class NonCopyablePersistentTraits;
template <class T> class PersistentBase;
template <class T, class M = NonCopyablePersistentTraits<T> > class Persistent;
template <typename T> class FunctionCallbackInfo;
template <typename T> class PropertyCallbackInfo;

class JitCodeEvent;
class RetainedObjectInfo;

enum PropertyAttribute {
  None = 0,
  ReadOnly = 1 << 0,
  DontEnum = 1 << 1,
  DontDelete = 1 << 2,
};

enum ExternalArrayType {
  kExternalInt8Array = 1,
  kExternalUint8Array,
  kExternalInt16Array,
  kExternalUint16Array,
  kExternalInt32Array,
  kExternalUint32Array,
  kExternalFloat32Array,
  kExternalFloat64Array,
  kExternalUint8ClampedArray,

  // Legacy constant names
  kExternalByteArray = kExternalInt8Array,
  kExternalUnsignedByteArray = kExternalUint8Array,
  kExternalShortArray = kExternalInt16Array,
  kExternalUnsignedShortArray = kExternalUint16Array,
  kExternalIntArray = kExternalInt32Array,
  kExternalUnsignedIntArray = kExternalUint32Array,
  kExternalFloatArray = kExternalFloat32Array,
  kExternalDoubleArray = kExternalFloat64Array,
  kExternalPixelArray = kExternalUint8ClampedArray
};

enum AccessControl {
  DEFAULT = 0,
  ALL_CAN_READ = 1,
  ALL_CAN_WRITE = 1 << 1,
  PROHIBITS_OVERWRITING = 1 << 2,
};

enum JitCodeEventOptions {
  kJitCodeEventDefault = 0,
  kJitCodeEventEnumExisting = 1,
};

enum class IntegrityLevel { kFrozen, kSealed };

enum class MicrotasksPolicy { kExplicit, kScoped, kAuto };

typedef void (*AccessorGetterCallback)(
  Local<String> property,
  const PropertyCallbackInfo<Value>& info);
typedef void (*AccessorNameGetterCallback)(
  Local<Name> property,
  const PropertyCallbackInfo<Value>& info);

typedef void (*AccessorSetterCallback)(
  Local<String> property,
  Local<Value> value,
  const PropertyCallbackInfo<void>& info);
typedef void (*AccessorNameSetterCallback)(
  Local<Name> property,
  Local<Value> value,
  const PropertyCallbackInfo<void>& info);

typedef void (*NamedPropertyGetterCallback)(
  Local<String> property, const PropertyCallbackInfo<Value>& info);
typedef void (*NamedPropertySetterCallback)(
  Local<String> property,
  Local<Value> value,
  const PropertyCallbackInfo<Value>& info);
typedef void (*NamedPropertyQueryCallback)(
  Local<String> property, const PropertyCallbackInfo<Integer>& info);
typedef void (*NamedPropertyDeleterCallback)(
  Local<String> property, const PropertyCallbackInfo<Boolean>& info);
typedef void (*NamedPropertyEnumeratorCallback)(
  const PropertyCallbackInfo<Array>& info);
typedef void (*NamedPropertyDescriptorCallback)(
  Local<Name> property, const PropertyCallbackInfo<Value>& info);
typedef void (*NamedPropertyDefinerCallback)(
  Local<Name> property, const PropertyDescriptor& desc,
  const PropertyCallbackInfo<Value>& info);

typedef void (*GenericNamedPropertyGetterCallback)(
  Local<Name> property, const PropertyCallbackInfo<Value>& info);
typedef void (*GenericNamedPropertySetterCallback)(
  Local<Name> property, Local<Value> value,
  const PropertyCallbackInfo<Value>& info);
typedef void (*GenericNamedPropertyQueryCallback)(
  Local<Name> property, const PropertyCallbackInfo<Integer>& info);
typedef void (*GenericNamedPropertyDeleterCallback)(
  Local<Name> property, const PropertyCallbackInfo<Boolean>& info);
typedef void (*GenericNamedPropertyEnumeratorCallback)(
  const PropertyCallbackInfo<Array>& info);
typedef void (*GenericNamedPropertyDescriptorCallback)(
  Local<Name> property, const PropertyCallbackInfo<Value>& info);
typedef void (*GenericNamedPropertyDefinerCallback)(
  Local<Name> property, const PropertyDescriptor& desc,
  const PropertyCallbackInfo<Value>& info);

typedef void (*IndexedPropertyGetterCallback)(
  uint32_t index, const PropertyCallbackInfo<Value>& info);
typedef void (*IndexedPropertySetterCallback)(
  uint32_t index, Local<Value> value, const PropertyCallbackInfo<Value>& info);
typedef void (*IndexedPropertyQueryCallback)(
  uint32_t index, const PropertyCallbackInfo<Integer>& info);
typedef void (*IndexedPropertyDeleterCallback)(
  uint32_t index, const PropertyCallbackInfo<Boolean>& info);
typedef void (*IndexedPropertyEnumeratorCallback)(
  const PropertyCallbackInfo<Array>& info);
typedef void (*IndexedPropertyDefinerCallback)(
  uint32_t index, const PropertyDescriptor& desc,
  const PropertyCallbackInfo<Value>& info);
typedef void (*IndexedPropertyDescriptorCallback)(
  uint32_t index, const PropertyCallbackInfo<Value>& info);

typedef bool (*EntropySource)(unsigned char* buffer, size_t length);
typedef void (*FatalErrorCallback)(const char *location, const char *message);
typedef void (*JitCodeEventHandler)(const JitCodeEvent *event);

typedef MaybeLocal<Promise>(*HostImportModuleDynamicallyCallback)(
    Local<Context> context, Local<ScriptOrModule> referrer,
    Local<String> specifier);

typedef void (*HostInitializeImportMetaObjectCallback)(
    Local<Context> context,
    Local<Module> module,
    Local<Object> meta);

template <class T>
class Local {
 public:
  V8_INLINE Local() : val_(0) {}

  template <class S>
  V8_INLINE Local(Local<S> that)
      : val_(reinterpret_cast<T*>(*that)) {
    TYPE_CHECK(T, S);
  }

  V8_INLINE bool IsEmpty() const { return val_ == 0; }
  V8_INLINE void Clear() { val_ = 0; }
  V8_INLINE T* operator->() const { return val_; }
  V8_INLINE T* operator*() const { return val_; }

  template <class S>
  V8_INLINE bool operator==(const Local<S>& that) const {
    return val_ == that.val_;
  }

  template <class S>
  V8_INLINE bool operator==(const PersistentBase<S>& that) const {
    return val_ == that.val_;
  }

  template <class S>
  V8_INLINE bool operator!=(const Local<S>& that) const {
    return !operator==(that);
  }

  template <class S>
  V8_INLINE bool operator!=(const PersistentBase<S>& that) const {
    return !operator==(that);
  }

  template <class S>
  V8_INLINE static Local<T> Cast(Local<S> that) {
    return Local<T>(T::Cast(*that));
  }

  template <class S>
  V8_INLINE Local<S> As() {
    return Local<S>::Cast(*this);
  }

  V8_INLINE static Local<T> New(Isolate* isolate, Local<T> that);
  V8_INLINE static Local<T> New(Isolate* isolate,
                                const PersistentBase<T>& that);

 private:
  friend struct AcessorExternalDataType;
  friend class AccessorSignature;
  friend class Array;
  friend class ArrayBuffer;
  friend class ArrayBufferView;
  friend class Boolean;
  friend class BooleanObject;
  friend class Context;
  friend class Date;
  friend class Debug;
  friend class External;
  friend class Function;
  friend class FunctionCallbackData;
  friend class FunctionTemplate;
  friend class FunctionTemplateData;
  friend class HandleScope;
  friend class Integer;
  friend class Map;
  friend class Message;
  friend class Number;
  friend class NumberObject;
  friend class Object;
  friend class ObjectTemplate;
  friend class Private;
  friend class PropertyDescriptor;
  friend class Proxy;
  friend class RegExp;
  friend class Promise;
  friend class Set;
  friend class Signature;
  friend class Script;
  friend class StackFrame;
  friend class StackTrace;
  friend class String;
  friend class StringObject;
  friend class Symbol;
  friend class SymbolObject;
  friend class Utils;
  friend class TryCatch;
  friend class UnboundScript;
  friend class Value;
  friend class JSON;
  friend class uvimpl::Work;
  friend JsErrorCode jsrt::CreateV8PropertyDescriptor(
    JsValueRef descriptor,
    v8::PropertyDescriptor* result);
  friend class jsrt::IsolateShim;
  template <class F> friend class FunctionCallbackInfo;
  template <class F> friend class MaybeLocal;
  template <class F> friend class PersistentBase;
  template <class F, class M> friend class Persistent;
  template <class F> friend class Local;
  friend V8_EXPORT Local<Primitive> Undefined(Isolate* isolate);
  friend V8_EXPORT Local<Primitive> Null(Isolate* isolate);
  friend V8_EXPORT Local<Boolean> True(Isolate* isolate);
  friend V8_EXPORT Local<Boolean> False(Isolate* isolate);

  explicit V8_INLINE Local(T* that)
      : val_(that) {}
  V8_INLINE static Local<T> New(Isolate* isolate, T* that) {
    return New(that);
  }

  V8_INLINE Local(JsValueRef that)
    : val_(static_cast<T*>(that)) {}
  V8_INLINE Local(const PersistentBase<T>& that)
    : val_(that.val_) {
  }
  V8_INLINE static Local<T> New(T* that);
  V8_INLINE static Local<T> New(JsValueRef ref) {
    return New(static_cast<T*>(ref));
  }

  T* val_;
};


// Handle is an alias for Local for historical reasons.
template <class T>
using Handle = Local<T>;


template <class T>
class MaybeLocal {
 public:
  MaybeLocal() : val_(nullptr) {}
  template <class S>
  MaybeLocal(Local<S> that)  // NOLINT(runtime/explicit)
    : val_(reinterpret_cast<T*>(*that)) {
    TYPE_CHECK(T, S);
  }

  bool IsEmpty() const { return val_ == nullptr; }

  template <class S>
  bool ToLocal(Local<S>* out) const {
    out->val_ = IsEmpty() ? nullptr : this->val_;
    return !IsEmpty();
  }

  V8_INLINE Local<T> ToLocalChecked();

  template <class S>
  Local<S> FromMaybe(Local<S> default_value) const {
    return IsEmpty() ? default_value : Local<S>(val_);
  }

 private:
  T* val_;
};


static const int kInternalFieldsInWeakCallback = 2;


template <typename T>
class WeakCallbackInfo {
 public:
  typedef void (*Callback)(const WeakCallbackInfo<T>& data);

  WeakCallbackInfo(Isolate* isolate, T* parameter,
                   void* internal_fields[kInternalFieldsInWeakCallback],
                   Callback* callback)
      : isolate_(isolate), parameter_(parameter), callback_(callback) {
    for (int i = 0; i < kInternalFieldsInWeakCallback; ++i) {
      internal_fields_[i] = internal_fields[i];
    }
  }

  V8_INLINE Isolate* GetIsolate() const { return isolate_; }
  V8_INLINE T* GetParameter() const { return parameter_; }
  V8_INLINE void* GetInternalField(int index) const {
    return internal_fields_[index];
  }

  V8_INLINE V8_DEPRECATE_SOON("use indexed version",
                              void* GetInternalField1()) const {
    return internal_fields_[0];
  }
  V8_INLINE V8_DEPRECATE_SOON("use indexed version",
                              void* GetInternalField2()) const {
    return internal_fields_[1];
  }

  bool IsFirstPass() const { return callback_ != nullptr; }
  void SetSecondPassCallback(Callback callback) const { *callback_ = callback; }

 private:
  Isolate* isolate_;
  T* parameter_;
  Callback* callback_;
  void* internal_fields_[kInternalFieldsInWeakCallback];
};


template <class T, class P>
class WeakCallbackData {
 public:
  typedef void (*Callback)(const WeakCallbackData<T, P>& data);

  WeakCallbackData(Isolate* isolate, P* parameter, Local<T> handle)
      : isolate_(isolate), parameter_(parameter), handle_(handle) {}

  V8_INLINE Isolate* GetIsolate() const { return isolate_; }
  V8_INLINE P* GetParameter() const { return parameter_; }
  V8_INLINE Local<T> GetValue() const { return handle_; }

 private:
  Isolate* isolate_;
  P* parameter_;
  Local<T> handle_;
};


namespace chakrashim {
struct WeakReferenceCallbackWrapper {
  void *parameters;
  union {
    WeakCallbackInfo<void>::Callback infoCallback;
    WeakCallbackData<Value, void>::Callback dataCallback;
  };
  bool isWeakCallbackInfo;
};

// A helper method for setting an object with a WeakReferenceCallback. The
// callback will be called before the object is released.
V8_EXPORT void SetObjectWeakReferenceCallback(
  JsValueRef object,
  WeakCallbackInfo<void>::Callback callback,
  void* parameters,
  WeakReferenceCallbackWrapper** weakWrapper);
V8_EXPORT void SetObjectWeakReferenceCallback(
  JsValueRef object,
  WeakCallbackData<Value, void>::Callback callback,
  void* parameters,
  WeakReferenceCallbackWrapper** weakWrapper);
// A helper method for turning off the WeakReferenceCallback that was set using
// the previous method
V8_EXPORT void ClearObjectWeakReferenceCallback(JsValueRef object, bool revive);
}  // namespace chakrashim

enum class WeakCallbackType { kParameter, kInternalFields };

template <class T>
class PersistentBase {
 public:
  V8_INLINE void Reset();

  template <class S>
  V8_INLINE void Reset(Isolate* isolate, const Handle<S>& other);

  template <class S>
  V8_INLINE void Reset(Isolate* isolate, const PersistentBase<S>& other);

  V8_INLINE bool IsEmpty() const { return val_ == nullptr; }
  V8_INLINE void Empty() { Reset(); }

  V8_INLINE Local<T> Get(Isolate* isolate) const {
    return Local<T>::New(isolate, *this);
  }

  template <class S>
  V8_INLINE bool operator==(const PersistentBase<S>& that) const {
    return val_ == that.val_;
  }

  template <class S>
  V8_INLINE bool operator==(const Handle<S>& that) const {
    return val_ == that.val_;
  }

  template <class S>
  V8_INLINE bool operator!=(const PersistentBase<S>& that) const {
    return !operator==(that);
  }

  template <class S>
  V8_INLINE bool operator!=(const Handle<S>& that) const {
    return !operator==(that);
  }

  template <typename P>
  V8_INLINE V8_DEPRECATE_SOON(
      "use WeakCallbackInfo version",
      void SetWeak(P* parameter,
                   typename WeakCallbackData<T, P>::Callback callback));

  template <typename P>
  V8_INLINE void SetWeak(P* parameter,
                         typename WeakCallbackInfo<P>::Callback callback,
                         WeakCallbackType type);

  template<typename P>
  V8_INLINE P* ClearWeak();

  V8_INLINE void ClearWeak() { ClearWeak<void>(); }
  V8_INLINE void MarkIndependent();
  V8_INLINE void MarkPartiallyDependent();
  V8_INLINE bool IsIndependent() const;
  V8_INLINE bool IsNearDeath() const;
  V8_INLINE bool IsWeak() const;
  V8_INLINE void SetWrapperClassId(uint16_t class_id);

 private:
  template<class F> friend class Local;
  template<class F> friend class Global;
  template<class F1, class F2> friend class Persistent;
  template <class F> friend class Global;

  explicit V8_INLINE PersistentBase(T* val)
      : val_(val), _weakWrapper(nullptr) {}
  PersistentBase(PersistentBase& other) = delete;  // NOLINT
  void operator=(PersistentBase&) = delete;
  V8_INLINE static T* New(Isolate* isolate, T* that);

  template <typename P, typename Callback>
  void SetWeakCommon(P* parameter, Callback callback);

  T* val_;
  chakrashim::WeakReferenceCallbackWrapper* _weakWrapper;
};


template<class T>
class NonCopyablePersistentTraits {
 public:
  typedef Persistent<T, NonCopyablePersistentTraits<T> > NonCopyablePersistent;
  static const bool kResetInDestructor = true;  // chakra: changed to true!
  template<class S, class M>
  V8_INLINE static void Copy(const Persistent<S, M>& source,
                             NonCopyablePersistent* dest) {
    Uncompilable<Object>();
  }
  template<class O> V8_INLINE static void Uncompilable() {
    TYPE_CHECK(O, Primitive);
  }
};


template<class T>
struct CopyablePersistentTraits {
  typedef Persistent<T, CopyablePersistentTraits<T> > CopyablePersistent;
  static const bool kResetInDestructor = true;
  template<class S, class M>
  static V8_INLINE void Copy(const Persistent<S, M>& source,
                             CopyablePersistent* dest) {
    // do nothing, just allow copy
  }
};


template <class T, class M>
class Persistent : public PersistentBase<T> {
 public:
  V8_INLINE Persistent() : PersistentBase<T>(0) { }

  template <class S>
  V8_INLINE Persistent(Isolate* isolate, Handle<S> that)
      : PersistentBase<T>(PersistentBase<T>::New(isolate, *that)) {
    TYPE_CHECK(T, S);
  }

  template <class S, class M2>
  V8_INLINE Persistent(Isolate* isolate, const Persistent<S, M2>& that)
    : PersistentBase<T>(PersistentBase<T>::New(isolate, *that)) {
    TYPE_CHECK(T, S);
  }

  V8_INLINE Persistent(const Persistent& that) : PersistentBase<T>(0) {
    Copy(that);
  }

  template <class S, class M2>
  V8_INLINE Persistent(const Persistent<S, M2>& that) : PersistentBase<T>(0) {
    Copy(that);
  }

  V8_INLINE Persistent& operator=(const Persistent& that) { // NOLINT
    Copy(that);
    return *this;
  }

  template <class S, class M2>
  V8_INLINE Persistent& operator=(const Persistent<S, M2>& that) { // NOLINT
    Copy(that);
    return *this;
  }

  V8_INLINE ~Persistent() {
    if (M::kResetInDestructor) this->Reset();
  }

  template <class S>
  V8_INLINE static Persistent<T>& Cast(Persistent<S>& that) { // NOLINT
    return reinterpret_cast<Persistent<T>&>(that);
  }

  template <class S>
  V8_INLINE Persistent<S>& As() { // NOLINT
    return Persistent<S>::Cast(*this);
  }

 private:
  friend class Object;
  friend class ObjectTemplate;
  friend class ObjectTemplateData;
  friend class TemplateData;
  friend class FunctionCallbackData;
  friend class FunctionTemplate;
  friend class FunctionTemplateData;
  friend class Utils;
  template <class F> friend class Local;
  template <class F> friend class ReturnValue;
  friend class PromiseResolverData;

  V8_INLINE Persistent(T* that)
    : PersistentBase<T>(PersistentBase<T>::New(nullptr, that)) { }

  V8_INLINE T* operator*() const { return this->val_; }
  V8_INLINE T* operator->() const { return this->val_; }

  template<class S, class M2>
  V8_INLINE void Copy(const Persistent<S, M2>& that);

  template <class S>
  V8_INLINE Persistent& operator=(const Local<S>& other) {
    this->Reset(nullptr, other);
    return *this;
  }
  V8_INLINE Persistent& operator=(JsRef other) {
    return operator=(Local<T>(static_cast<T*>(other)));
  }
};


template <class T>
class Global : public PersistentBase<T> {
 public:
  V8_INLINE Global() : PersistentBase<T>(nullptr) {}

  template <class S>
  V8_INLINE Global(Isolate* isolate, Handle<S> that)
    : PersistentBase<T>(PersistentBase<T>::New(isolate, *that)) {
    TYPE_CHECK(T, S);
  }

  template <class S>
  V8_INLINE Global(Isolate* isolate, const PersistentBase<S>& that)
    : PersistentBase<T>(PersistentBase<T>::New(isolate, that.val_)) {
    TYPE_CHECK(T, S);
  }

  V8_INLINE Global(Global&& other) : PersistentBase<T>(other.val_) {
    this->_weakWrapper = other._weakWrapper;
    other.val_ = nullptr;
    other._weakWrapper = nullptr;
  }

  V8_INLINE ~Global() { this->Reset(); }

  template <class S>
  V8_INLINE Global& operator=(Global<S>&& rhs) {
    TYPE_CHECK(T, S);
    if (this != &rhs) {
      this->Reset();
      this->val_ = rhs.val_;
      this->_weakWrapper = rhs._weakWrapper;
      rhs.val_ = nullptr;
      rhs._weakWrapper = nullptr;
    }
    return *this;
  }

  Global Pass() { return static_cast<Global&&>(*this); }

 private:
  Global(const Global&) = delete;
  void operator=(const Global&) = delete;
};


template <class T>
class Eternal : protected Persistent<T> {
 public:
  Eternal() {}

  template<class S>
  Eternal(Isolate* isolate, Local<S> handle) {
    Set(isolate, handle);
  }

  Local<T> Get(Isolate* isolate) {
    return Local<T>::New(isolate, *this);
  }

  bool IsEmpty() const {
    return Persistent<T>::IsEmpty();
  }

  template<class S> void Set(Isolate* isolate, Local<S> handle) {
    this->Reset(isolate, handle);
  }
};

// CHAKRA: Chakra's GC behavior does not exactly match up with V8's GC behavior.
// V8 uses a HandleScope to keep Local references alive, which means that as
// long as the HandleScope is on the stack, the Local references will not be
// collected. Chakra, on the other hand, directly walks the stack and has no
// HandleScope mechanism. It requires hosts to keep "local" references on the
// stack or else turn them into "persistent" references through
// JsAddRef/JsRelease. To paper over this difference, the bridge HandleScope
// will create a JS array and will hold that reference on the stack. Any local
// values created will then be added to that array. So the GC will see the array
// on the stack and then keep those local references alive.
class V8_EXPORT HandleScope {
 public:
  HandleScope(Isolate* isolate);
  ~HandleScope();

  static int NumberOfHandles(Isolate* isolate);

  Isolate* GetIsolate() const;

 private:
  friend class EscapableHandleScope;
  template <class T> friend class Local;
  static const int kOnStackLocals = 8;  // Arbitrary number of refs on stack

  HandleScope *_prev;

  // Save some refs on stack. 1st element on stack
  // is the JavascriptArray where other refs go.
  JsValueRef _locals[kOnStackLocals + 1];
  int _count;
  JsContextRef _contextRef;
  struct AddRefRecord {
    JsRef _ref;
    AddRefRecord *  _next;
  } *_addRefRecordHead;

  bool AddLocal(JsValueRef value);
  bool AddLocalContext(JsContextRef value);
  bool AddLocalAddRef(JsRef value);

  static HandleScope *GetCurrent();

  template <class T>
  Local<T> Close(Handle<T> value);
};

class V8_EXPORT EscapableHandleScope : public HandleScope {
 public:
  EscapableHandleScope(Isolate* isolate) : HandleScope(isolate) {}

  template <class T>
  Local<T> Escape(Handle<T> value) { return Close(value); }
};

typedef HandleScope SealHandleScope;

class V8_EXPORT Data {
 public:
};

class V8_EXPORT ScriptOrModule {
 public:
  Local<Value> GetResourceName();
};

class ScriptOrigin {
 public:
  explicit ScriptOrigin(
    Local<Value> resource_name,
    Local<Integer> resource_line_offset = Local<Integer>(),
    Local<Integer> resource_column_offset = Local<Integer>(),
    Local<Boolean> resource_is_shared_cross_origin = Local<Boolean>(),
    Local<Integer> script_id = Local<Integer>(),
    Local<Value> source_map_url = Local<Value>(),
    Local<Boolean> resource_is_opaque = Local<Boolean>(),
    Local<Boolean> is_wasm = Local<Boolean>(),
    Local<Boolean> is_module = Local<Boolean>())
    : resource_name_(resource_name),
      resource_line_offset_(resource_line_offset),
      resource_column_offset_(resource_column_offset),
      script_id_(script_id) {}
  Local<Value> ResourceName() const {
    return resource_name_;
  }
  Local<Integer> ResourceLineOffset() const {
    return resource_line_offset_;
  }
  Local<Integer> ResourceColumnOffset() const {
    return resource_column_offset_;
  }
  V8_INLINE Local<Integer> ScriptID() const {
    return script_id_;
  }

 private:
  Local<Value> resource_name_;
  Local<Integer> resource_line_offset_;
  Local<Integer> resource_column_offset_;
  Local<Integer> script_id_;
};

class V8_EXPORT UnboundScript {
 public:
  Local<Script> BindToCurrentContext();

  int GetId();

  static const int kNoScriptId = 0;
};

class V8_EXPORT Script {
 public:
  static V8_DEPRECATE_SOON(
      "Use maybe version",
      Local<Script> Compile(Handle<String> source,
                            ScriptOrigin* origin = nullptr));
  static V8_WARN_UNUSED_RESULT MaybeLocal<Script> Compile(
      Local<Context> context, Handle<String> source,
      ScriptOrigin* origin = nullptr);

  static Local<Script> V8_DEPRECATE_SOON("Use maybe version",
                                         Compile(Handle<String> source,
                                                 Handle<String> file_name));

  V8_DEPRECATE_SOON("Use maybe version", Local<Value> Run());
  V8_WARN_UNUSED_RESULT MaybeLocal<Value> Run(Local<Context> context);

  Local<UnboundScript> GetUnboundScript();
};

class V8_EXPORT ScriptCompiler {
 public:
  struct CachedData {
    // CHAKRA-TODO: Not implemented
    enum BufferPolicy {
      BufferNotOwned,
      BufferOwned
    };

    const uint8_t* data;
    int length;
    bool rejected = true;
    BufferPolicy buffer_policy;

    CachedData();
    CachedData(const uint8_t* data, int length,
               BufferPolicy buffer_policy = BufferNotOwned) {
    }
  };

  class Source {
   public:
    Source(
      Local<String> source_string,
      const ScriptOrigin& origin,
      CachedData * cached_data = nullptr)
      : source_string(source_string), resource_name(origin.ResourceName()) {
    }

    Source(Local<String> source_string,  // NOLINT(runtime/explicit)
           CachedData * cached_data = nullptr)
      : source_string(source_string) {
    }

    const CachedData* GetCachedData() const { return nullptr; }

   private:
    friend ScriptCompiler;
    Local<String> source_string;
    Handle<Value> resource_name;
  };

  enum CompileOptions {
    kNoCompileOptions = 0,
    kProduceParserCache,
    kConsumeParserCache,
    kProduceCodeCache,
    kConsumeCodeCache
  };

  static V8_DEPRECATE_SOON("Use maybe version",
                           Local<UnboundScript> CompileUnbound(
                             Isolate* isolate, Source* source,
                             CompileOptions options = kNoCompileOptions));
  static V8_WARN_UNUSED_RESULT MaybeLocal<UnboundScript> CompileUnboundScript(
    Isolate* isolate, Source* source,
    CompileOptions options = kNoCompileOptions);

  static V8_DEPRECATE_SOON(
    "Use maybe version",
    Local<Script> Compile(Isolate* isolate, Source* source,
                          CompileOptions options = kNoCompileOptions));
  static V8_WARN_UNUSED_RESULT MaybeLocal<Script> Compile(
    Local<Context> context, Source* source,
    CompileOptions options = kNoCompileOptions);

  static uint32_t CachedDataVersionTag();

  static V8_WARN_UNUSED_RESULT MaybeLocal<Module> CompileModule(
    Isolate* isolate, Source* source);

  static CachedData* CreateCodeCache(Local<UnboundScript> unbound_script,
                                     Local<String> source) {
    return nullptr;
  }
};

class V8_EXPORT Message {
 public:
  Local<String> Get() const;

  V8_DEPRECATE_SOON("Use maybe version", Local<String> GetSourceLine()) const;
  V8_WARN_UNUSED_RESULT MaybeLocal<String> GetSourceLine(
      Local<Context> context) const;

  ScriptOrigin GetScriptOrigin() const;

  Handle<Value> GetScriptResourceName() const;

  Local<StackTrace> GetStackTrace() const;

  V8_DEPRECATE_SOON("Use maybe version", int GetLineNumber()) const;
  V8_WARN_UNUSED_RESULT Maybe<int> GetLineNumber(Local<Context> context) const;

  V8_DEPRECATE_SOON("Use maybe version", int GetStartColumn()) const;
  V8_WARN_UNUSED_RESULT Maybe<int> GetStartColumn(Local<Context> context) const;

  V8_DEPRECATE_SOON("Use maybe version", int GetEndColumn()) const;
  V8_WARN_UNUSED_RESULT Maybe<int> GetEndColumn(Local<Context> context) const;

  static const int kNoLineNumberInfo = 0;
  static const int kNoColumnInfo = 0;
  static const int kNoScriptIdInfo = 0;
};

typedef void (*MessageCallback)(Handle<Message> message, Handle<Value> error);

class V8_EXPORT StackTrace {
 public:
  enum StackTraceOptions {
    kLineNumber = 1,
    kColumnOffset = 1 << 1 | kLineNumber,
    kScriptName = 1 << 2,
    kFunctionName = 1 << 3,
    kIsEval = 1 << 4,
    kIsConstructor = 1 << 5,
    kScriptNameOrSourceURL = 1 << 6,
    kScriptId = 1 << 7,
    kExposeFramesAcrossSecurityOrigins = 1 << 8,
    kOverview = kLineNumber | kColumnOffset | kScriptName | kFunctionName,
    kDetailed = kOverview | kIsEval | kIsConstructor | kScriptNameOrSourceURL
  };

  Local<StackFrame> GetFrame(uint32_t index) const;
  int GetFrameCount() const;
  Local<Array> AsArray();

  static Local<StackTrace> CurrentStackTrace(
    Isolate* isolate,
    int frame_limit,
    StackTraceOptions options = kOverview);
};

class V8_EXPORT StackFrame {
 public:
  int GetLineNumber() const;
  int GetColumn() const;
  int GetScriptId() const;
  Local<String> GetScriptName() const;
  Local<String> GetScriptNameOrSourceURL() const;
  Local<String> GetFunctionName() const;
  bool IsEval() const;
  bool IsConstructor() const;
};

enum class PromiseHookType { kInit, kResolve, kBefore, kAfter };

typedef void(*PromiseHook)(PromiseHookType type,
                           Local<Promise> promise,
                           Local<Value> parent);


class V8_EXPORT Value : public Data {
 public:
  bool IsUndefined() const;
  bool IsNull() const;
  bool IsNullOrUndefined() const;
  bool IsTrue() const;
  bool IsFalse() const;
  bool IsName() const;
  bool IsString() const;
  bool IsSymbol() const;
  bool IsFunction() const;
  bool IsArray() const;
  bool IsObject() const;
  bool IsBoolean() const;
  bool IsNumber() const;
  bool IsInt32() const;
  bool IsUint32() const;
  bool IsDate() const;
  bool IsArgumentsObject() const;
  bool IsBooleanObject() const;
  bool IsNumberObject() const;
  bool IsStringObject() const;
  bool IsSymbolObject() const;
  bool IsNativeError() const;
  bool IsRegExp() const;
  bool IsAsyncFunction() const;
  bool IsGeneratorObject() const;
  bool IsGeneratorFunction() const;
  bool IsWebAssemblyCompiledModule() const;
  bool IsExternal() const;
  bool IsArrayBuffer() const;
  bool IsArrayBufferView() const;
  bool IsTypedArray() const;
  bool IsUint8Array() const;
  bool IsUint8ClampedArray() const;
  bool IsInt8Array() const;
  bool IsUint16Array() const;
  bool IsInt16Array() const;
  bool IsUint32Array() const;
  bool IsInt32Array() const;
  bool IsFloat32Array() const;
  bool IsFloat64Array() const;
  bool IsDataView() const;
  bool IsSharedArrayBuffer() const;
  bool IsMapIterator() const;
  bool IsSetIterator() const;
  bool IsMap() const;
  bool IsSet() const;
  bool IsWeakMap() const;
  bool IsWeakSet() const;
  bool IsPromise() const;
  bool IsProxy() const;

  V8_WARN_UNUSED_RESULT MaybeLocal<Boolean> ToBoolean(
    Local<Context> context) const;
  V8_WARN_UNUSED_RESULT MaybeLocal<Number> ToNumber(
    Local<Context> context) const;
  V8_WARN_UNUSED_RESULT MaybeLocal<String> ToString(
    Local<Context> context) const;
  V8_WARN_UNUSED_RESULT MaybeLocal<String> ToDetailString(
    Local<Context> context) const;
  V8_WARN_UNUSED_RESULT MaybeLocal<Object> ToObject(
    Local<Context> context) const;
  V8_WARN_UNUSED_RESULT MaybeLocal<Integer> ToInteger(
    Local<Context> context) const;
  V8_WARN_UNUSED_RESULT MaybeLocal<Uint32> ToUint32(
    Local<Context> context) const;
  V8_WARN_UNUSED_RESULT MaybeLocal<Int32> ToInt32(Local<Context> context) const;

  Local<Boolean> ToBoolean(Isolate* isolate = nullptr) const;
  Local<Number> ToNumber(Isolate* isolate = nullptr) const;
  Local<String> ToString(Isolate* isolate = nullptr) const;
  Local<String> ToDetailString(Isolate* isolate = nullptr) const;
  Local<Object> ToObject(Isolate* isolate = nullptr) const;
  Local<Integer> ToInteger(Isolate* isolate = nullptr) const;
  Local<Uint32> ToUint32(Isolate* isolate = nullptr) const;
  Local<Int32> ToInt32(Isolate* isolate = nullptr) const;

  V8_DEPRECATE_SOON("Use maybe version", Local<Uint32> ToArrayIndex()) const;
  V8_WARN_UNUSED_RESULT MaybeLocal<Uint32> ToArrayIndex(
    Local<Context> context) const;

  V8_WARN_UNUSED_RESULT Maybe<bool> BooleanValue(Local<Context> context) const;
  V8_WARN_UNUSED_RESULT Maybe<double> NumberValue(Local<Context> context) const;
  V8_WARN_UNUSED_RESULT Maybe<int64_t> IntegerValue(
    Local<Context> context) const;
  V8_WARN_UNUSED_RESULT Maybe<uint32_t> Uint32Value(
    Local<Context> context) const;
  V8_WARN_UNUSED_RESULT Maybe<int32_t> Int32Value(Local<Context> context) const;

  V8_DEPRECATE_SOON("Use maybe version", bool BooleanValue()) const;
  V8_DEPRECATE_SOON("Use maybe version", double NumberValue()) const;
  V8_DEPRECATE_SOON("Use maybe version", int64_t IntegerValue()) const;
  V8_DEPRECATE_SOON("Use maybe version", uint32_t Uint32Value()) const;
  V8_DEPRECATE_SOON("Use maybe version", int32_t Int32Value()) const;

  V8_DEPRECATE_SOON("Use maybe version", bool Equals(Handle<Value> that)) const;
  V8_WARN_UNUSED_RESULT Maybe<bool> Equals(Local<Context> context,
                                           Handle<Value> that) const;

  bool StrictEquals(Handle<Value> that) const;

  template <class T> static Value* Cast(T* value) {
    return static_cast<Value*>(value);
  }
};

class V8_EXPORT Private : public Data {
 public:
  Local<Value> Name() const;
  static Local<Private> New(Isolate* isolate,
                            Local<String> name = Local<String>());
  static Local<Private> ForApi(Isolate* isolate, Local<String> name);

 private:
  Private();
};

class V8_EXPORT Primitive : public Value {
 public:
};

class V8_EXPORT Boolean : public Primitive {
 public:
  bool Value() const;
  static Handle<Boolean> New(Isolate* isolate, bool value);
  static Boolean* Cast(v8::Value* obj);

 private:
  friend class BooleanObject;
  template <class F> friend class ReturnValue;
  static Local<Boolean> From(bool value);
};

class V8_EXPORT Name : public Primitive {
 public:
  int GetIdentityHash();
  static Name* Cast(v8::Value* obj);
 private:
  static void CheckCast(v8::Value* obj);
};

enum class NewStringType { kNormal, kInternalized };

class V8_EXPORT String : public Name {
 public:
  static const int kMaxLength = (1 << 28) - 16;

  int Length() const;
  int Utf8Length() const;
  bool IsOneByte() const { return false; }
  bool ContainsOnlyOneByte() const { return false; }

  enum WriteOptions {
    NO_OPTIONS = 0,
    HINT_MANY_WRITES_EXPECTED = 1,
    NO_NULL_TERMINATION = 2,
    PRESERVE_ONE_BYTE_NULL = 4,
    REPLACE_INVALID_UTF8 = 8
  };

  int Write(uint16_t* buffer,
            int start = 0,
            int length = -1,
            int options = NO_OPTIONS) const;
  int WriteOneByte(uint8_t* buffer,
                   int start = 0,
                   int length = -1,
                   int options = NO_OPTIONS) const;
  int WriteUtf8(char* buffer,
                int length = -1,
                int* nchars_ref = nullptr,
                int options = NO_OPTIONS) const;

  static Local<String> Empty(Isolate* isolate);
  bool IsExternal() const { return false; }
  bool IsExternalOneByte() const { return false; }

  class V8_EXPORT ExternalOneByteStringResource {
   public:
    virtual ~ExternalOneByteStringResource() {}
    virtual const char *data() const = 0;
    virtual size_t length() const = 0;
    virtual void Dispose() { delete this; }
  };

  class V8_EXPORT ExternalStringResource {
   public:
    virtual ~ExternalStringResource() {}
    virtual const uint16_t* data() const = 0;
    virtual size_t length() const = 0;
    virtual void Dispose() { delete this; }
  };

  ExternalStringResource* GetExternalStringResource() const { return nullptr; }
  const ExternalOneByteStringResource*
    GetExternalOneByteStringResource() const {
    return nullptr;
  }

  static String *Cast(v8::Value *obj);

  enum NewStringType {
    kNormalString = static_cast<int>(v8::NewStringType::kNormal),
    kInternalizedString = static_cast<int>(v8::NewStringType::kInternalized)
  };

  static V8_DEPRECATE_SOON(
    "Use maybe version",
    Local<String> NewFromUtf8(Isolate* isolate, const char* data,
                              NewStringType type = kNormalString,
                              int length = -1));
  static V8_WARN_UNUSED_RESULT MaybeLocal<String> NewFromUtf8(
    Isolate* isolate, const char* data, v8::NewStringType type,
    int length = -1);

  static V8_DEPRECATE_SOON(
    "Use maybe version",
    Local<String> NewFromOneByte(Isolate* isolate, const uint8_t* data,
                                 NewStringType type = kNormalString,
                                 int length = -1));
  static V8_WARN_UNUSED_RESULT MaybeLocal<String> NewFromOneByte(
    Isolate* isolate, const uint8_t* data, v8::NewStringType type,
    int length = -1);

  static V8_DEPRECATE_SOON(
    "Use maybe version",
    Local<String> NewFromTwoByte(Isolate* isolate, const uint16_t* data,
                                 NewStringType type = kNormalString,
                                 int length = -1));
  static V8_WARN_UNUSED_RESULT MaybeLocal<String> NewFromTwoByte(
    Isolate* isolate, const uint16_t* data, v8::NewStringType type,
    int length = -1);

  static Local<String> Concat(Handle<String> left, Handle<String> right);

  static V8_DEPRECATE_SOON(
    "Use maybe version",
    Local<String> NewExternal(Isolate* isolate,
                              ExternalStringResource* resource));
  static V8_WARN_UNUSED_RESULT MaybeLocal<String> NewExternalTwoByte(
    Isolate* isolate, ExternalStringResource* resource);

  static V8_DEPRECATE_SOON(
    "Use maybe version",
    Local<String> NewExternal(Isolate* isolate,
                              ExternalOneByteStringResource* resource));
  static V8_WARN_UNUSED_RESULT MaybeLocal<String> NewExternalOneByte(
    Isolate* isolate, ExternalOneByteStringResource* resource);

  class V8_EXPORT Utf8Value {
   public:
    explicit Utf8Value(Handle<v8::Value> obj);
    ~Utf8Value();
    char *operator*() { return _str; }
    const char *operator*() const { return _str; }
    int length() const { return static_cast<int>(_length); }
   private:
    Utf8Value(const Utf8Value&);
    void operator=(const Utf8Value&);

    char* _str;
    int _length;
  };

  class V8_EXPORT Value {
   public:
    explicit Value(Handle<v8::Value> obj);
    ~Value();
    uint16_t *operator*() { return _str; }
    const uint16_t *operator*() const { return _str; }
    int length() const { return _length; }
   private:
    Value(const Value&);
    void operator=(const Value&);

    uint16_t* _str;
    int _length;
  };
};

class V8_EXPORT Symbol : public Name {
 public:
  // Returns the print name string of the symbol, or undefined if none.
  Local<Value> Name() const;
  static Local<Symbol> New(Isolate* isolate,
      Local<String> name = Local<String>());
  static Symbol* Cast(Value* obj);

 private:
  static Local<Symbol> From(Local<String> name);
  Symbol();
};

class V8_EXPORT Number : public Primitive {
 public:
  double Value() const;
  static Local<Number> New(Isolate* isolate, double value);
  static Number *Cast(v8::Value *obj);

 private:
  friend class Integer;
  template <class F> friend class ReturnValue;
  static Local<Number> From(double value);
};

class V8_EXPORT Integer : public Number {
 public:
  static Local<Integer> New(Isolate* isolate, int32_t value);
  static Local<Integer> NewFromUnsigned(Isolate* isolate, uint32_t value);
  static Integer *Cast(v8::Value *obj);

  int64_t Value() const;

 private:
  friend class Utils;
  template <class F> friend class ReturnValue;
  static Local<Integer> From(int32_t value);
  static Local<Integer> From(uint32_t value);
};

class V8_EXPORT Int32 : public Integer {
 public:
  int32_t Value() const;
  static Int32* Cast(v8::Value* obj);
};

class V8_EXPORT Uint32 : public Integer {
 public:
  uint32_t Value() const;
  static Uint32* Cast(v8::Value* obj);
};

class V8_EXPORT Object : public Value {
 public:
  V8_DEPRECATE_SOON("Use maybe version",
                    bool Set(Handle<Value> key, Handle<Value> value));
  V8_WARN_UNUSED_RESULT Maybe<bool> Set(Local<Context> context,
                                        Local<Value> key, Local<Value> value);

  V8_DEPRECATE_SOON("Use maybe version",
                    bool Set(uint32_t index, Handle<Value> value));
  V8_WARN_UNUSED_RESULT Maybe<bool> Set(Local<Context> context, uint32_t index,
                                        Local<Value> value);

  V8_WARN_UNUSED_RESULT Maybe<bool> CreateDataProperty(Local<Context> context,
                                                       Local<Name> key,
                                                       Local<Value> value);
  V8_WARN_UNUSED_RESULT Maybe<bool> CreateDataProperty(Local<Context> context,
                                                       uint32_t index,
                                                       Local<Value> value);

  V8_WARN_UNUSED_RESULT Maybe<bool> DefineOwnProperty(
      Local<Context> context, Local<Name> key, Local<Value> value,
      PropertyAttribute attributes = None);

  V8_WARN_UNUSED_RESULT Maybe<bool> DefineProperty(Local<Context> context,
      Local<Name>,
      PropertyDescriptor& decriptor);  // NOLINT(runtime/references)

  V8_DEPRECATE_SOON("Use maybe version",
                    bool ForceSet(Handle<Value> key, Handle<Value> value,
                                  PropertyAttribute attribs = None));
  V8_WARN_UNUSED_RESULT Maybe<bool> ForceSet(Local<Context> context,
                                             Local<Value> key,
                                             Local<Value> value,
                                             PropertyAttribute attribs = None);

  V8_DEPRECATE_SOON("Use maybe version", Local<Value> Get(Handle<Value> key));
  V8_WARN_UNUSED_RESULT MaybeLocal<Value> Get(Local<Context> context,
                                              Local<Value> key);

  V8_DEPRECATE_SOON("Use maybe version", Local<Value> Get(uint32_t index));
  V8_WARN_UNUSED_RESULT MaybeLocal<Value> Get(Local<Context> context,
                                              uint32_t index);

  V8_DEPRECATE_SOON("Use maybe version",
                    PropertyAttribute GetPropertyAttributes(Handle<Value> key));
  V8_WARN_UNUSED_RESULT Maybe<PropertyAttribute> GetPropertyAttributes(
      Local<Context> context, Local<Value> key);

  V8_DEPRECATE_SOON("Use maybe version",
                    Local<Value> GetOwnPropertyDescriptor(Local<Name> key));
  V8_WARN_UNUSED_RESULT MaybeLocal<Value> GetOwnPropertyDescriptor(
    Local<Context> context, Local<Name> key);

  V8_DEPRECATE_SOON("Use maybe version", bool Has(Handle<Value> key));
  V8_WARN_UNUSED_RESULT Maybe<bool> Has(Local<Context> context,
                                        Local<Value> key);

  V8_DEPRECATE_SOON("Use maybe version", bool Delete(Handle<Value> key));
  V8_WARN_UNUSED_RESULT Maybe<bool> Delete(Local<Context> context,
                                           Local<Value> key);

  V8_DEPRECATE_SOON("Use maybe version", bool Has(uint32_t index));
  V8_WARN_UNUSED_RESULT Maybe<bool> Has(Local<Context> context, uint32_t index);

  V8_DEPRECATE_SOON("Use maybe version", bool Delete(uint32_t index));
  V8_WARN_UNUSED_RESULT Maybe<bool> Delete(Local<Context> context,
                                           uint32_t index);

  V8_DEPRECATE_SOON("Use maybe version",
                    bool SetAccessor(Handle<String> name,
                                     AccessorGetterCallback getter,
                                     AccessorSetterCallback setter = 0,
                                     Handle<Value> data = Handle<Value>(),
                                     AccessControl settings = DEFAULT,
                                     PropertyAttribute attribute = None));
  V8_DEPRECATE_SOON("Use maybe version",
                    bool SetAccessor(Handle<Name> name,
                                     AccessorNameGetterCallback getter,
                                     AccessorNameSetterCallback setter = 0,
                                     Handle<Value> data = Handle<Value>(),
                                     AccessControl settings = DEFAULT,
                                     PropertyAttribute attribute = None));
  V8_WARN_UNUSED_RESULT
  Maybe<bool> SetAccessor(Local<Context> context,
                          Local<Name> name,
                          AccessorNameGetterCallback getter,
                          AccessorNameSetterCallback setter = 0,
                          MaybeLocal<Value> data = MaybeLocal<Value>(),
                          AccessControl settings = DEFAULT,
                          PropertyAttribute attribute = None);

  V8_DEPRECATE_SOON("Use maybe version", Local<Array> GetPropertyNames());
  V8_WARN_UNUSED_RESULT MaybeLocal<Array> GetPropertyNames(
    Local<Context> context);

  V8_DEPRECATE_SOON("Use maybe version", Local<Array> GetOwnPropertyNames());
  V8_WARN_UNUSED_RESULT MaybeLocal<Array> GetOwnPropertyNames(
    Local<Context> context);

  Local<Value> GetPrototype();

  V8_DEPRECATE_SOON("Use maybe version",
                    bool SetPrototype(Handle<Value> prototype));
  V8_WARN_UNUSED_RESULT Maybe<bool> SetPrototype(Local<Context> context,
                                                 Local<Value> prototype);

  V8_DEPRECATE_SOON("Use maybe version", Local<String> ObjectProtoToString());
  V8_WARN_UNUSED_RESULT MaybeLocal<String> ObjectProtoToString(
    Local<Context> context);

  Local<String> GetConstructorName();
  int InternalFieldCount();
  Local<Value> GetInternalField(int index);
  void SetInternalField(int index, Handle<Value> value);
  void* GetAlignedPointerFromInternalField(int index);
  void SetAlignedPointerInInternalField(int index, void* value);

  V8_DEPRECATE_SOON("Use maybe version",
                    bool HasOwnProperty(Handle<String> key));
  V8_WARN_UNUSED_RESULT Maybe<bool> HasOwnProperty(Local<Context> context,
                                                   Local<Name> key);
  V8_DEPRECATE_SOON("Use maybe version",
                    bool HasRealNamedProperty(Handle<String> key));
  V8_WARN_UNUSED_RESULT Maybe<bool> HasRealNamedProperty(Local<Context> context,
                                                         Local<Name> key);
  V8_DEPRECATE_SOON("Use maybe version",
                    bool HasRealIndexedProperty(uint32_t index));
  V8_WARN_UNUSED_RESULT Maybe<bool> HasRealIndexedProperty(
    Local<Context> context, uint32_t index);
  V8_DEPRECATE_SOON("Use maybe version",
                    bool HasRealNamedCallbackProperty(Handle<String> key));
  V8_WARN_UNUSED_RESULT Maybe<bool> HasRealNamedCallbackProperty(
    Local<Context> context, Local<Name> key);

  V8_DEPRECATE_SOON(
    "Use maybe version",
    Local<Value> GetRealNamedPropertyInPrototypeChain(Handle<String> key));
  V8_WARN_UNUSED_RESULT MaybeLocal<Value> GetRealNamedPropertyInPrototypeChain(
    Local<Context> context, Local<Name> key);

  V8_DEPRECATE_SOON("Use maybe version",
                    Local<Value> GetRealNamedProperty(Handle<String> key));
  V8_WARN_UNUSED_RESULT MaybeLocal<Value> GetRealNamedProperty(
    Local<Context> context, Local<Name> key);

  V8_DEPRECATE_SOON("Use maybe version",
                    Maybe<PropertyAttribute> GetRealNamedPropertyAttributes(
                      Handle<String> key));
  V8_WARN_UNUSED_RESULT Maybe<PropertyAttribute> GetRealNamedPropertyAttributes(
    Local<Context> context, Local<Name> key);

  V8_DEPRECATE_SOON("Use v8::Object::SetPrivate instead.",
                    bool SetHiddenValue(Handle<String> key,
                                        Handle<Value> value));
  V8_DEPRECATE_SOON("Use v8::Object::GetPrivate instead.",
                    Local<Value> GetHiddenValue(Handle<String> key));

  void SetAccessorProperty(Local<Name> name, Local<Function> getter,
                           Local<Function> setter = Local<Function>(),
                           PropertyAttribute attribute = None,
                           AccessControl settings = DEFAULT);

  Maybe<bool> HasPrivate(Local<Context> context, Local<Private> key);
  Maybe<bool> SetPrivate(Local<Context> context, Local<Private> key,
                         Local<Value> value);
  Maybe<bool> DeletePrivate(Local<Context> context, Local<Private> key);
  MaybeLocal<Value> GetPrivate(Local<Context> context, Local<Private> key);

  Local<Object> Clone();
  Local<Context> CreationContext();

  V8_DEPRECATE_SOON("Use maybe version",
                    Local<Value> CallAsFunction(Handle<Value> recv, int argc,
                                                Handle<Value> argv[]));
  V8_WARN_UNUSED_RESULT MaybeLocal<Value> CallAsFunction(Local<Context> context,
                                                         Handle<Value> recv,
                                                         int argc,
                                                         Handle<Value> argv[]);
  V8_DEPRECATE_SOON("Use maybe version",
                    Local<Value> CallAsConstructor(int argc,
                                                   Handle<Value> argv[]));
  V8_WARN_UNUSED_RESULT MaybeLocal<Value> CallAsConstructor(
    Local<Context> context, int argc, Local<Value> argv[]);

  Isolate* GetIsolate();
  static Local<Object> New(Isolate* isolate = nullptr);
  static Object *Cast(Value *obj);

  Maybe<bool> SetIntegrityLevel(Local<Context> context, IntegrityLevel level);

 private:
  friend class ObjectTemplate;
  friend class Utils;

  Maybe<bool> Set(Handle<Value> key, Handle<Value> value,
                  PropertyAttribute attribs, bool force);
  Maybe<bool> SetAccessor(Handle<Name> name,
                          AccessorNameGetterCallback getter,
                          AccessorNameSetterCallback setter,
                          Handle<Value> data,
                          AccessControl settings,
                          PropertyAttribute attribute,
                          Handle<AccessorSignature> signature);

  ObjectTemplate* GetObjectTemplate();
};

class V8_EXPORT Array : public Object {
 public:
  uint32_t Length() const;

  V8_DEPRECATE_SOON("Use maybe version",
                    Local<Object> CloneElementAt(uint32_t index));
  V8_WARN_UNUSED_RESULT MaybeLocal<Object> CloneElementAt(
    Local<Context> context, uint32_t index);

  static Local<Array> New(Isolate* isolate = nullptr, int length = 0);
  static Array *Cast(Value *obj);
};

class V8_EXPORT BooleanObject : public Object {
 public:
  static Local<Value> New(Isolate* isolate, bool value);
  V8_DEPRECATED("Pass an isolate", static Local<Value> New(bool value));
  bool ValueOf() const;
  static BooleanObject* Cast(Value* obj);
};


class V8_EXPORT StringObject : public Object {
 public:
  static Local<Value> New(Handle<String> value);
  Local<String> ValueOf() const;
  static StringObject* Cast(Value* obj);
};

class V8_EXPORT SymbolObject : public Object {
 public:
  static Local<Value> New(Isolate* isolate, Local<Symbol> value);
  Local<Symbol> ValueOf() const;
  static SymbolObject* Cast(v8::Value* obj);
};

class V8_EXPORT NumberObject : public Object {
 public:
  static Local<Value> New(Isolate * isolate, double value);
  double ValueOf() const;
  static NumberObject* Cast(Value* obj);
};

class V8_EXPORT Date : public Object {
 public:
  static V8_DEPRECATE_SOON("Use maybe version.",
                           Local<Value> New(Isolate* isolate, double time));
  static V8_WARN_UNUSED_RESULT MaybeLocal<Value> New(Local<Context> context,
                                                     double time);

  static Date *Cast(Value *obj);
};

class V8_EXPORT RegExp : public Object {
 public:
  enum Flags {
    kNone = 0,
    kGlobal = 1,
    kIgnoreCase = 2,
    kMultiline = 4
  };

  static V8_DEPRECATE_SOON("Use maybe version",
                           Local<RegExp> New(Handle<String> pattern,
                                             Flags flags));
  static V8_WARN_UNUSED_RESULT MaybeLocal<RegExp> New(Local<Context> context,
                                                      Handle<String> pattern,
                                                      Flags flags);
  Local<String> GetSource() const;
  static RegExp *Cast(v8::Value *obj);
};


class V8_EXPORT Map : public Object {
 public:
  size_t Size() const;
  void Clear();
  V8_WARN_UNUSED_RESULT MaybeLocal<Value> Get(Local<Context> context,
                                              Local<Value> key);
  V8_WARN_UNUSED_RESULT MaybeLocal<Map> Set(Local<Context> context,
                                            Local<Value> key,
                                            Local<Value> value);
  V8_WARN_UNUSED_RESULT Maybe<bool> Has(Local<Context> context,
                                        Local<Value> key);
  V8_WARN_UNUSED_RESULT Maybe<bool> Delete(Local<Context> context,
                                           Local<Value> key);

  Local<Array> AsArray() const;
  static Local<Map> New(Isolate* isolate);
  static Map* Cast(Value* obj);

 private:
  Map();
};

class V8_EXPORT Set : public Object {
 public:
  size_t Size() const;
  void Clear();
  V8_WARN_UNUSED_RESULT MaybeLocal<Set> Add(Local<Context> context,
                                            Local<Value> key);
  V8_WARN_UNUSED_RESULT Maybe<bool> Has(Local<Context> context,
                                        Local<Value> key);
  V8_WARN_UNUSED_RESULT Maybe<bool> Delete(Local<Context> context,
                                           Local<Value> key);

  Local<Array> AsArray() const;
  static Local<Set> New(Isolate* isolate);
  static Set* Cast(Value* obj);

 private:
  Set();
};

template<typename T>
class ReturnValue {
 public:
  // Handle setters
  template <typename S> void Set(const Persistent<S>& handle) {
    *_value = static_cast<Value*>(*handle);
  }
  template <typename S> void Set(const Handle<S> handle) {
    *_value = static_cast<Value*>(*handle);
  }
  // Fast primitive setters
  void Set(bool value) { Set(Boolean::From(value)); }
  void Set(double value) { Set(Number::From(value)); }
  void Set(int32_t value) { Set(Integer::From(value)); }
  void Set(uint32_t value) { Set(Integer::From(value)); }
  // Fast JS primitive setters
  V8_INLINE void SetNull();
  V8_INLINE void SetUndefined();
  V8_INLINE void SetEmptyString();
  // Convenience getter for Isolate
  V8_INLINE Isolate* GetIsolate();

  Value* Get() const { return *_value; }

 private:
  explicit ReturnValue(Value** value)
    : _value(value) {
  }

  Value** _value;
  template <typename F> friend class FunctionCallbackInfo;
  template <typename F> friend class PropertyCallbackInfo;
};

template<typename T>
class FunctionCallbackInfo {
 public:
  int Length() const { return _length; }
  V8_INLINE Local<Value> operator[](int i) const;
  Local<Function> Callee() const { return _callee; }
  Local<Object> This() const { return _thisPointer; }
  Local<Value> NewTarget() const { return _newTargetPointer; }
  Local<Object> Holder() const { return _holder; }
  bool IsConstructCall() const { return _isConstructorCall; }
  Local<Value> Data() const { return _data; }
  V8_INLINE Isolate* GetIsolate() const;
  ReturnValue<T> GetReturnValue() const {
    return ReturnValue<T>(
      &(const_cast<FunctionCallbackInfo<T>*>(this)->_returnValue));
  }

  FunctionCallbackInfo(
    Value** args,
    int length,
    Local<Object> _this,
    Local<Object> _newTarget,
    Local<Object> holder,
    bool isConstructorCall,
    Local<Value> data,
    Local<Function> callee)
       : _args(args),
         _length(length),
         _thisPointer(_this),
         _newTargetPointer(_newTarget),
         _holder(holder),
         _isConstructorCall(isConstructorCall),
         _data(data),
         _callee(callee),
         _returnValue(static_cast<Value*>(JS_INVALID_REFERENCE)) {
  }

 private:
  Value** _args;
  int _length;
  Local<Object> _thisPointer;
  Local<Object> _newTargetPointer;
  Local<Object> _holder;
  bool _isConstructorCall;
  Local<Value> _data;
  Local<Function> _callee;
  Value* _returnValue;
};


template<typename T>
class PropertyCallbackInfo {
 public:
  V8_INLINE Isolate* GetIsolate() const;
  Local<Value> Data() const { return _data; }
  Local<Object> This() const { return _thisObject; }
  Local<Object> Holder() const { return _holder; }
  // CHAKRA-TODO
  bool ShouldThrowOnError() const { return true; }
  ReturnValue<T> GetReturnValue() const {
    return ReturnValue<T>(
      &(const_cast<PropertyCallbackInfo<T>*>(this)->_returnValue));
  }

  PropertyCallbackInfo(
    Local<Value> data, Local<Object> thisObject, Local<Object> holder)
       : _data(data),
         _thisObject(thisObject),
         _holder(holder),
         _returnValue(static_cast<Value*>(JS_INVALID_REFERENCE)) {
  }

 private:
  Local<Value> _data;
  Local<Object> _thisObject;
  Local<Object> _holder;
  Value* _returnValue;
};

typedef void (*FunctionCallback)(const FunctionCallbackInfo<Value>& info);

enum class ConstructorBehavior { kThrow, kAllow };

class V8_EXPORT Function : public Object {
 public:
  static MaybeLocal<Function> New(
      Local<Context> context,
      FunctionCallback callback,
      Local<Value> data = Local<Value>(),
      int length = 0,
      ConstructorBehavior behavior = ConstructorBehavior::kAllow);
  static V8_DEPRECATE_SOON("Use maybe version",
    Local<Function> New(Isolate* isolate,
                        FunctionCallback callback,
                        Local<Value> data = Local<Value>(),
                        int length = 0));

  V8_DEPRECATE_SOON("Use maybe version",
                    Local<Object> NewInstance(int argc,
                                              Handle<Value> argv[])) const;
  V8_WARN_UNUSED_RESULT MaybeLocal<Object> NewInstance(
    Local<Context> context, int argc, Handle<Value> argv[]) const;

  V8_DEPRECATE_SOON("Use maybe version", Local<Object> NewInstance()) const;
  V8_WARN_UNUSED_RESULT MaybeLocal<Object> NewInstance(
    Local<Context> context) const {
    return NewInstance(context, 0, nullptr);
  }

  V8_DEPRECATE_SOON("Use maybe version",
                    Local<Value> Call(Handle<Value> recv, int argc,
                                      Handle<Value> argv[]));
  V8_WARN_UNUSED_RESULT MaybeLocal<Value> Call(Local<Context> context,
                                               Handle<Value> recv, int argc,
                                               Handle<Value> argv[]);

  void SetName(Handle<String> name);
  Local<Value> GetName() const;

  Local<Value> GetInferredName() const;
  Local<Value> GetDebugName() const;

  int GetScriptLineNumber() const;
  int GetScriptColumnNumber() const;

  int ScriptId() const;
  Local<Value> GetBoundFunction() const;

  static Function *Cast(Value *obj);
  static const int kLineOffsetNotFound;
};

class V8_EXPORT Promise : public Object {
 public:
  enum PromiseState { kPending, kFulfilled, kRejected };

  Local<Value> Result();
  PromiseState State();

  class V8_EXPORT Resolver : public Object {
   public:
    static V8_DEPRECATE_SOON("Use maybe version",
      Local<Resolver> New(Isolate* isolate));
    static V8_WARN_UNUSED_RESULT MaybeLocal<Resolver> New(
      Local<Context> context);
    Local<Promise> GetPromise();
    V8_DEPRECATE_SOON("Use maybe version", void Resolve(Local<Value> value));
    Maybe<bool> Resolve(Local<Context> context, Local<Value> value);

    V8_DEPRECATE_SOON("Use maybe version", void Reject(Local<Value> value));
    Maybe<bool> Reject(Local<Context> context, Local<Value> value);
    static Resolver* Cast(Value* obj);
   private:
    Resolver();
    static void CheckCast(Value* obj);
  };

  Local<Promise> Chain(Handle<Function> handler);

  V8_DEPRECATED("Use maybe version",
                Local<Promise> Catch(Local<Function> handler));
  V8_WARN_UNUSED_RESULT MaybeLocal<Promise> Catch(Local<Context> context,
                                                  Local<Function> handler);

  V8_DEPRECATED("Use maybe version",
                Local<Promise> Then(Local<Function> handler));
  V8_WARN_UNUSED_RESULT MaybeLocal<Promise> Then(Local<Context> context,
                                                 Local<Function> handler);

  bool HasHandler();
  static Promise* Cast(Value* obj);

 private:
  Promise();
};

class V8_EXPORT PropertyDescriptor {
 public:
  PropertyDescriptor();
  PropertyDescriptor(Local<Value> value);
  PropertyDescriptor(Local<Value> value, bool writable);
  PropertyDescriptor(Local<Value> get, Local<Value> set);
  ~PropertyDescriptor();

  Local<Value> value() const;
  bool has_value() const;

  Local<Value> get() const;
  bool has_get() const;
  Local<Value> set() const;
  bool has_set() const;

  void set_enumerable(bool enumerable);
  bool enumerable() const;
  bool has_enumerable() const;

  void set_configurable(bool configurable);
  bool configurable() const;
  bool has_configurable() const;

  bool writable() const;
  bool has_writable() const;

  struct PrivateData;
  PrivateData* get_private() const { return private_; }

  PropertyDescriptor & operator=(PropertyDescriptor &&);

  PropertyDescriptor(const PropertyDescriptor&) = delete;
  void operator=(const PropertyDescriptor&) = delete;

 private:
  PrivateData* private_;
};

class V8_EXPORT Proxy : public Object {
 public:
  Local<Object> GetTarget();
  Local<Value> GetHandler();
  bool IsRevoked();
  void Revoke();

  static MaybeLocal<Proxy> New(Local<Context> context,
                               Local<Object> local_target,
                               Local<Object> local_handler);

  static Proxy* Cast(Value* obj);

 private:
  Proxy();
  static void CheckCast(Value* obj);
};


enum class ArrayBufferCreationMode { kInternalized, kExternalized };

class V8_EXPORT ArrayBuffer : public Object {
 public:
  class V8_EXPORT Allocator {  // NOLINT
   public:
    virtual ~Allocator() {}
    virtual void* Allocate(size_t length) = 0;
    virtual void* AllocateUninitialized(size_t length) = 0;
    virtual void Free(void* data, size_t length) = 0;
    static Allocator* NewDefaultAllocator();
  };

  class V8_EXPORT Contents {  // NOLINT
   public:
    Contents() : data_(nullptr), byte_length_(0) {}
    void* Data() const { return data_; }
    size_t ByteLength() const { return byte_length_; }

   private:
    void* data_;
    size_t byte_length_;
    friend class ArrayBuffer;
  };

  size_t ByteLength() const;
  static Local<ArrayBuffer> New(Isolate* isolate, size_t byte_length);
  static Local<ArrayBuffer> New(
    Isolate* isolate, void* data, size_t byte_length,
    ArrayBufferCreationMode mode = ArrayBufferCreationMode::kExternalized);

  bool IsExternal() const;
  bool IsNeuterable() const;
  void Neuter();
  Contents Externalize();
  Contents GetContents();

// #if ENABLE_TTD_NODE
  void TTDRawBufferNotifyRegisterForModification(byte* initialModPosition);
  static void TTDRawBufferAsyncModifyComplete(byte* finalModPosition);
  void TTDRawBufferModifyNotifySync(UINT32 index, UINT32 count);
  static void TTDRawBufferCopyNotify(Local<ArrayBuffer> dst, UINT32 dstindex,
                                     Local<ArrayBuffer> src, UINT32 srcIndex,
                                     UINT32 count);
// #endif

  static ArrayBuffer* Cast(Value* obj);

 private:
  ArrayBuffer();
};

class V8_EXPORT ArrayBufferView : public Object {
 public:
  Local<ArrayBuffer> Buffer();
  size_t ByteOffset();
  size_t ByteLength();
  size_t CopyContents(void* dest, size_t byte_length);
  bool HasBuffer() const;

  static ArrayBufferView* Cast(Value* obj);
 private:
  ArrayBufferView();
};

class V8_EXPORT TypedArray : public ArrayBufferView {
 public:
  size_t Length();
  static TypedArray* Cast(Value* obj);
 private:
  TypedArray();
};

class V8_EXPORT Uint8Array : public TypedArray {
 public:
  static Local<Uint8Array> New(Handle<ArrayBuffer> array_buffer,
                               size_t byte_offset, size_t length);
  static Uint8Array* Cast(Value* obj);
 private:
  Uint8Array();
};

class V8_EXPORT Uint8ClampedArray : public TypedArray {
 public:
  static Local<Uint8ClampedArray> New(Handle<ArrayBuffer> array_buffer,
                                      size_t byte_offset, size_t length);
  static Uint8ClampedArray* Cast(Value* obj);
 private:
  Uint8ClampedArray();
};

class V8_EXPORT Int8Array : public TypedArray {
 public:
  static Local<Int8Array> New(Handle<ArrayBuffer> array_buffer,
                              size_t byte_offset, size_t length);
  static Int8Array* Cast(Value* obj);
 private:
  Int8Array();
};

class V8_EXPORT Uint16Array : public TypedArray {
 public:
  static Local<Uint16Array> New(Handle<ArrayBuffer> array_buffer,
                                size_t byte_offset, size_t length);
  static Uint16Array* Cast(Value* obj);
 private:
  Uint16Array();
};

class V8_EXPORT Int16Array : public TypedArray {
 public:
  static Local<Int16Array> New(Handle<ArrayBuffer> array_buffer,
                               size_t byte_offset, size_t length);
  static Int16Array* Cast(Value* obj);
 private:
  Int16Array();
};

class V8_EXPORT Uint32Array : public TypedArray {
 public:
  static Local<Uint32Array> New(Handle<ArrayBuffer> array_buffer,
                                size_t byte_offset, size_t length);
  static Uint32Array* Cast(Value* obj);
 private:
  Uint32Array();
};

class V8_EXPORT Int32Array : public TypedArray {
 public:
  static Local<Int32Array> New(Handle<ArrayBuffer> array_buffer,
                               size_t byte_offset, size_t length);
  static Int32Array* Cast(Value* obj);
 private:
  Int32Array();
};

class V8_EXPORT Float32Array : public TypedArray {
 public:
  static Local<Float32Array> New(Handle<ArrayBuffer> array_buffer,
                                 size_t byte_offset, size_t length);
  static Float32Array* Cast(Value* obj);
 private:
  Float32Array();
};

class V8_EXPORT Float64Array : public TypedArray {
 public:
  static Local<Float64Array> New(Handle<ArrayBuffer> array_buffer,
                                 size_t byte_offset, size_t length);
  static Float64Array* Cast(Value* obj);
 private:
  Float64Array();
};

class V8_EXPORT SharedArrayBuffer : public Object {
 public:
  static SharedArrayBuffer* Cast(Value* obj);

 private:
  SharedArrayBuffer();
};

class V8_EXPORT JSON {
 public:
  static V8_DEPRECATED("Use the maybe version taking context",
    Local<Value> Parse(Local<String> json_string));
  static V8_DEPRECATE_SOON("Use the maybe version taking context",
    MaybeLocal<Value> Parse(Isolate* isolate,
    Local<String> json_string));
  static V8_WARN_UNUSED_RESULT MaybeLocal<Value> Parse(
    Local<Context> context, Local<String> json_string);

  static V8_WARN_UNUSED_RESULT MaybeLocal<String> Stringify(
    Local<Context> context, Local<Object> json_object,
    Local<String> gap = Local<String>());
};

class V8_EXPORT ValueSerializer {
 public:
  class V8_EXPORT Delegate {
   public:
    virtual ~Delegate() {}

    virtual void ThrowDataCloneError(Local<String> message) = 0;
    virtual Maybe<bool> WriteHostObject(Isolate* isolate, Local<Object> object);
    virtual Maybe<uint32_t> GetSharedArrayBufferId(
      Isolate* isolate, Local<SharedArrayBuffer> shared_array_buffer);
    virtual void* ReallocateBufferMemory(void* old_buffer, size_t size,
      size_t* actual_size);
    virtual void FreeBufferMemory(void* buffer);
  };

  explicit ValueSerializer(Isolate* isolate);
  ValueSerializer(Isolate* isolate, Delegate* delegate);
  ~ValueSerializer();

  void WriteHeader();
  V8_WARN_UNUSED_RESULT Maybe<bool> WriteValue(Local<Context> context,
    Local<Value> value);
  V8_WARN_UNUSED_RESULT std::pair<uint8_t*, size_t> Release();
  void TransferArrayBuffer(uint32_t transfer_id,
    Local<ArrayBuffer> array_buffer);
  void SetTreatArrayBufferViewsAsHostObjects(bool mode);
  void WriteUint32(uint32_t value);
  void WriteUint64(uint64_t value);
  void WriteDouble(double value);
  void WriteRawBytes(const void* source, size_t length);

 private:
  ValueSerializer(const ValueSerializer&) = delete;
  void operator=(const ValueSerializer&) = delete;
};

class V8_EXPORT ValueDeserializer {
 public:
  class V8_EXPORT Delegate {
   public:
    virtual ~Delegate() {}

    virtual MaybeLocal<Object> ReadHostObject(Isolate* isolate);
  };

  ValueDeserializer(Isolate* isolate, const uint8_t* data, size_t size,
    Delegate* delegate);
  ~ValueDeserializer();

  V8_WARN_UNUSED_RESULT Maybe<bool> ReadHeader(Local<Context> context);
  V8_WARN_UNUSED_RESULT MaybeLocal<Value> ReadValue(Local<Context> context);
  void TransferArrayBuffer(uint32_t transfer_id,
    Local<ArrayBuffer> array_buffer);
  void TransferSharedArrayBuffer(uint32_t id,
    Local<SharedArrayBuffer> shared_array_buffer);
  uint32_t GetWireFormatVersion() const;
  V8_WARN_UNUSED_RESULT bool ReadUint32(uint32_t* value);
  V8_WARN_UNUSED_RESULT bool ReadUint64(uint64_t* value);
  V8_WARN_UNUSED_RESULT bool ReadDouble(double* value);
  V8_WARN_UNUSED_RESULT bool ReadRawBytes(size_t length, const void** data);

 private:
  ValueDeserializer(const ValueDeserializer&) = delete;
  void operator=(const ValueDeserializer&) = delete;
};

enum AccessType {
  ACCESS_GET,
  ACCESS_SET,
  ACCESS_HAS,
  ACCESS_DELETE,
  ACCESS_KEYS
};

typedef bool (*NamedSecurityCallback)(
  Local<Object> host, Local<Value> key, AccessType type, Local<Value> data);
typedef bool (*IndexedSecurityCallback)(
  Local<Object> host, uint32_t index, AccessType type, Local<Value> data);

class V8_EXPORT Template : public Data {
 public:
  void Set(Local<Name> name, Local<Data> value,
           PropertyAttribute attributes = None);
  V8_INLINE void Set(Isolate* isolate, const char* name, Local<Data> value) {
    Set(v8::String::NewFromUtf8(isolate, name), value);
  }

  void SetAccessorProperty(
      Local<Name> name,
      Local<FunctionTemplate> getter = Local<FunctionTemplate>(),
      Local<FunctionTemplate> setter = Local<FunctionTemplate>(),
      PropertyAttribute attribute = None,
      AccessControl settings = DEFAULT);

 private:
  Template();
};

class V8_EXPORT FunctionTemplate : public Template {
 public:
  static Local<FunctionTemplate> New(
      Isolate* isolate, FunctionCallback callback = 0,
      Local<Value> data = Local<Value>(),
      Local<Signature> signature = Local<Signature>(), int length = 0);

  V8_DEPRECATE_SOON("Use maybe version", Local<Function> GetFunction());
  V8_WARN_UNUSED_RESULT MaybeLocal<Function> GetFunction(
    Local<Context> context);

  Local<ObjectTemplate> InstanceTemplate();
  Local<ObjectTemplate> PrototypeTemplate();
  void SetClassName(Handle<String> name);
  void SetHiddenPrototype(bool value);
  void SetCallHandler(FunctionCallback callback,
                      Handle<Value> data = Handle<Value>());
  bool HasInstance(Handle<Value> object);
  void Inherit(Handle<FunctionTemplate> parent);
  void RemovePrototype();
};

enum class PropertyHandlerFlags {
  kNone = 0,
  kAllCanRead = 1,
  kNonMasking = 1 << 1,
  kOnlyInterceptStrings = 1 << 2,
};

struct NamedPropertyHandlerConfiguration {
  NamedPropertyHandlerConfiguration(
    GenericNamedPropertyGetterCallback getter = 0,
    GenericNamedPropertySetterCallback setter = 0,
    GenericNamedPropertyQueryCallback query = 0,
    GenericNamedPropertyDeleterCallback deleter = 0,
    GenericNamedPropertyEnumeratorCallback enumerator = 0,
    Handle<Value> data = Handle<Value>(),
    PropertyHandlerFlags flags = PropertyHandlerFlags::kNone)
    : getter(getter),
      setter(setter),
      query(query),
      deleter(deleter),
      enumerator(enumerator),
      definer(0),
      descriptor(0),
      data(data),
      flags(flags) {}

    NamedPropertyHandlerConfiguration(
      GenericNamedPropertyGetterCallback getter,
      GenericNamedPropertySetterCallback setter,
      GenericNamedPropertyDescriptorCallback descriptor,
      GenericNamedPropertyDeleterCallback deleter,
      GenericNamedPropertyEnumeratorCallback enumerator,
      GenericNamedPropertyDefinerCallback definer,
      Local<Value> data = Local<Value>(),
      PropertyHandlerFlags flags = PropertyHandlerFlags::kNone)
      : getter(getter),
        setter(setter),
        query(0),
        deleter(deleter),
        enumerator(enumerator),
        definer(definer),
        descriptor(descriptor),
        data(data),
        flags(flags) {}

    GenericNamedPropertyGetterCallback getter;
    GenericNamedPropertySetterCallback setter;
    GenericNamedPropertyQueryCallback query;
    GenericNamedPropertyDeleterCallback deleter;
    GenericNamedPropertyEnumeratorCallback enumerator;
    GenericNamedPropertyDefinerCallback definer;
    GenericNamedPropertyDescriptorCallback descriptor;
    Local<Value> data;
    PropertyHandlerFlags flags;
};

struct IndexedPropertyHandlerConfiguration {
  IndexedPropertyHandlerConfiguration(
    IndexedPropertyGetterCallback getter = 0,
    IndexedPropertySetterCallback setter = 0,
    IndexedPropertyQueryCallback query = 0,
    IndexedPropertyDeleterCallback deleter = 0,
    IndexedPropertyEnumeratorCallback enumerator = 0,
    Handle<Value> data = Handle<Value>(),
    PropertyHandlerFlags flags = PropertyHandlerFlags::kNone)
    : getter(getter),
      setter(setter),
      query(query),
      deleter(deleter),
      enumerator(enumerator),
      definer(0),
      descriptor(0),
      data(data),
      flags(flags) {}

  IndexedPropertyHandlerConfiguration(
    IndexedPropertyGetterCallback getter = 0,
    IndexedPropertySetterCallback setter = 0,
    IndexedPropertyDescriptorCallback descriptor = 0,
    IndexedPropertyDeleterCallback deleter = 0 ,
    IndexedPropertyEnumeratorCallback enumerator = 0,
    IndexedPropertyDefinerCallback definer = 0,
    Local<Value> data = Local<Value>(),
    PropertyHandlerFlags flags = PropertyHandlerFlags::kNone)
    : getter(getter),
      setter(setter),
      query(0),
      deleter(deleter),
      enumerator(enumerator),
      definer(definer),
      descriptor(descriptor),
      data(data),
      flags(flags) {}

  IndexedPropertyGetterCallback getter;
  IndexedPropertySetterCallback setter;
  IndexedPropertyQueryCallback query;
  IndexedPropertyDeleterCallback deleter;
  IndexedPropertyEnumeratorCallback enumerator;
  IndexedPropertyDefinerCallback definer;
  IndexedPropertyDescriptorCallback descriptor;
  Handle<Value> data;
  PropertyHandlerFlags flags;
};

class V8_EXPORT ObjectTemplate : public Template {
 public:
  static Local<ObjectTemplate> New(
      Isolate* isolate,
      Local<FunctionTemplate> constructor = Local<FunctionTemplate>());

  V8_DEPRECATE_SOON("Use maybe version", Local<Object> NewInstance());
  V8_WARN_UNUSED_RESULT MaybeLocal<Object> NewInstance(Local<Context> context);

  void SetAccessor(Handle<String> name,
                   AccessorGetterCallback getter,
                   AccessorSetterCallback setter = 0,
                   Handle<Value> data = Handle<Value>(),
                   AccessControl settings = DEFAULT,
                   PropertyAttribute attribute = None,
                   Handle<AccessorSignature> signature =
                       Handle<AccessorSignature>());
  void SetAccessor(Handle<Name> name,
                   AccessorNameGetterCallback getter,
                   AccessorNameSetterCallback setter = 0,
                   Handle<Value> data = Handle<Value>(),
                   AccessControl settings = DEFAULT,
                   PropertyAttribute attribute = None,
                   Handle<AccessorSignature> signature =
                       Handle<AccessorSignature>());

  void SetNamedPropertyHandler(
    NamedPropertyGetterCallback getter,
    NamedPropertySetterCallback setter = 0,
    NamedPropertyQueryCallback query = 0,
    NamedPropertyDeleterCallback deleter = 0,
    NamedPropertyEnumeratorCallback enumerator = 0,
    NamedPropertyDefinerCallback definer = 0,
    NamedPropertyDescriptorCallback descriptor = 0,
    Handle<Value> data = Handle<Value>());
  void SetHandler(const NamedPropertyHandlerConfiguration& configuration);

  void SetHandler(const IndexedPropertyHandlerConfiguration& configuration);
  void SetIndexedPropertyHandler(
    IndexedPropertyGetterCallback getter,
    IndexedPropertySetterCallback setter = 0,
    IndexedPropertyQueryCallback query = 0,
    IndexedPropertyDeleterCallback deleter = 0,
    IndexedPropertyEnumeratorCallback enumerator = 0,
    IndexedPropertyDefinerCallback definer = 0,
    IndexedPropertyDescriptorCallback descriptor = 0,
    Handle<Value> data = Handle<Value>());

  void SetAccessCheckCallbacks(
    NamedSecurityCallback named_handler,
    IndexedSecurityCallback indexed_handler,
    Handle<Value> data = Handle<Value>(),
    bool turned_on_by_default = true);

  void SetInternalFieldCount(int value);
  void SetCallAsFunctionHandler(FunctionCallback callback,
                                Handle<Value> data = Handle<Value>());

 private:
  friend class FunctionTemplate;
  friend class FunctionCallbackData;
  friend class FunctionTemplateData;
  friend class Utils;

  Local<Object> NewInstance(Handle<Function> constructor);
  void SetConstructor(Handle<FunctionTemplate> constructor);
};

class V8_EXPORT External : public Value {
 public:
  static Local<Value> Wrap(void* data);
  static inline void* Unwrap(Handle<Value> obj);
  static bool IsExternal(const Value* obj);

  static Local<External> New(Isolate* isolate, void* value);
  static External* Cast(Value* obj);
  void* Value() const;
};

class V8_EXPORT Signature : public Data {
 public:
  static Local<Signature> New(Isolate* isolate,
                              Handle<FunctionTemplate> receiver =
                                Handle<FunctionTemplate>(),
                              int argc = 0,
                              Handle<FunctionTemplate> argv[] = nullptr);
 private:
  Signature();
};

class V8_EXPORT AccessorSignature : public Data {
 public:
  static Local<AccessorSignature> New(
    Isolate* isolate,
    Handle<FunctionTemplate> receiver = Handle<FunctionTemplate>());
};


V8_EXPORT Handle<Primitive> Undefined(Isolate* isolate);
V8_EXPORT Handle<Primitive> Null(Isolate* isolate);
V8_EXPORT Handle<Boolean> True(Isolate* isolate);
V8_EXPORT Handle<Boolean> False(Isolate* isolate);
V8_EXPORT bool SetResourceConstraints(ResourceConstraints *constraints);


class V8_EXPORT ResourceConstraints {
 public:
  void set_stack_limit(uint32_t *value) {}
};

class V8_EXPORT Exception {
 public:
  static Local<Value> RangeError(Handle<String> message);
  static Local<Value> ReferenceError(Handle<String> message);
  static Local<Value> SyntaxError(Handle<String> message);
  static Local<Value> TypeError(Handle<String> message);
  static Local<Value> Error(Handle<String> message);
};

class V8_EXPORT MicrotasksScope {
 public:
    enum Type { kRunMicrotasks, kDoNotRunMicrotasks };

    MicrotasksScope(Isolate* isolate, Type type);
    ~MicrotasksScope();

    // Prevent copying.
    MicrotasksScope(const MicrotasksScope&) = delete;
    MicrotasksScope& operator=(const MicrotasksScope&) = delete;
};

enum GCType {
  kGCTypeScavenge = 1 << 0,
  kGCTypeMarkSweepCompact = 1 << 1,
  kGCTypeIncrementalMarking = 1 << 2,
  kGCTypeProcessWeakCallbacks = 1 << 3,
  kGCTypeAll = kGCTypeScavenge | kGCTypeMarkSweepCompact |
               kGCTypeIncrementalMarking | kGCTypeProcessWeakCallbacks
};

enum GCCallbackFlags {
  kNoGCCallbackFlags = 0,
  kGCCallbackFlagCompacted = 1 << 0,
  kGCCallbackFlagConstructRetainedObjectInfos = 1 << 1,
  kGCCallbackFlagForced = 1 << 2
};

typedef void (*GCPrologueCallback)(GCType type, GCCallbackFlags flags);
typedef void (*GCEpilogueCallback)(GCType type, GCCallbackFlags flags);

// --- Promise Reject Callback ---
enum PromiseRejectEvent {
  kPromiseRejectWithNoHandler = 0,
  kPromiseHandlerAddedAfterReject = 1
};

class PromiseRejectMessage {
 public:
  PromiseRejectMessage(Handle<Promise> promise, PromiseRejectEvent event,
                       Handle<Value> value, Handle<StackTrace> stack_trace)
    : promise_(promise),
    event_(event),
    value_(value),
    stack_trace_(stack_trace) {
  }

  Handle<Promise> GetPromise() const { return promise_; }
  PromiseRejectEvent GetEvent() const { return event_; }
  Handle<Value> GetValue() const { return value_; }

  // DEPRECATED. Use v8::Exception::CreateMessage(GetValue())->GetStackTrace()
  Handle<StackTrace> GetStackTrace() const { return stack_trace_; }

 private:
  Handle<Promise> promise_;
  PromiseRejectEvent event_;
  Handle<Value> value_;
  Handle<StackTrace> stack_trace_;
};

typedef void (*PromiseRejectCallback)(PromiseRejectMessage message);
typedef void(*MicrotaskCallback)(void* data);

/**
* Collection of V8 heap information.
*
* Instances of this class can be passed to v8::V8::HeapStatistics to
* get heap statistics from V8.
*/
class V8_EXPORT HeapStatistics {
 public:
  HeapStatistics();

  void set_heap_size(size_t heap_size) {
    total_heap_size_ = heap_size;
  }

  size_t total_heap_size() { return total_heap_size_; }
  size_t total_heap_size_executable() { return total_heap_size_executable_; }
  size_t total_physical_size() { return total_physical_size_; }
  size_t total_available_size() { return total_available_size_; }
  size_t used_heap_size() { return used_heap_size_; }
  size_t heap_size_limit() { return heap_size_limit_; }
  size_t malloced_memory() { return malloced_memory_; }
  size_t peak_malloced_memory() { return peak_malloced_memory_; }
  size_t does_zap_garbage() { return does_zap_garbage_; }

 private:
  size_t total_heap_size_;
  size_t total_heap_size_executable_;
  size_t total_physical_size_;
  size_t total_available_size_;
  size_t used_heap_size_;
  size_t heap_size_limit_;
  size_t malloced_memory_;
  size_t peak_malloced_memory_;
  bool does_zap_garbage_;

  friend class V8;
  friend class Isolate;
};

class V8_EXPORT HeapSpaceStatistics {
 public:
  HeapSpaceStatistics() {}
  const char* space_name() { return ""; }
  size_t space_size() { return 0; }
  size_t space_used_size() { return 0; }
  size_t space_available_size() { return 0; }
  size_t physical_space_size() { return 0; }

 private:
  const char* space_name_;
  size_t space_size_;
  size_t space_used_size_;
  size_t space_available_size_;
  size_t physical_space_size_;

  friend class Isolate;
};

typedef void(*FunctionEntryHook)(uintptr_t function,
                                 uintptr_t return_addr_location);
typedef int* (*CounterLookupCallback)(const char* name);
typedef void* (*CreateHistogramCallback)(
  const char* name, int min, int max, size_t buckets);
typedef void (*AddHistogramSampleCallback)(void* histogram, int sample);

typedef void (*InterruptCallback)(Isolate* isolate, void* data);

class V8_EXPORT Isolate {
 public:
  struct CreateParams {
    CreateParams()
       : entry_hook(nullptr),
         code_event_handler(nullptr),
         snapshot_blob(nullptr),
         counter_lookup_callback(nullptr),
         create_histogram_callback(nullptr),
         add_histogram_sample_callback(nullptr),
         array_buffer_allocator(nullptr) {
    }

    FunctionEntryHook entry_hook;
    JitCodeEventHandler code_event_handler;
    ResourceConstraints constraints;
    StartupData* snapshot_blob;
    CounterLookupCallback counter_lookup_callback;
    CreateHistogramCallback create_histogram_callback;
    AddHistogramSampleCallback add_histogram_sample_callback;
    ArrayBuffer::Allocator* array_buffer_allocator;
  };

  class V8_EXPORT Scope {
   public:
    explicit Scope(Isolate* isolate) : isolate_(isolate) { isolate->Enter(); }
    ~Scope() { isolate_->Exit(); }
   private:
    Isolate* const isolate_;
    Scope(const Scope&);
    Scope& operator=(const Scope&);
  };

  class V8_EXPORT DisallowJavascriptExecutionScope {
   public:
    enum OnFailure { CRASH_ON_FAILURE, THROW_ON_FAILURE };

    DisallowJavascriptExecutionScope(Isolate* isolate, OnFailure on_failure);
    ~DisallowJavascriptExecutionScope();

    DisallowJavascriptExecutionScope(const DisallowJavascriptExecutionScope&) =
        delete;
    DisallowJavascriptExecutionScope& operator=(
        const DisallowJavascriptExecutionScope&) = delete;
  };

  enum GarbageCollectionType {
    kFullGarbageCollection,
    kMinorGarbageCollection
  };

  static Isolate* NewWithTTDSupport(const CreateParams& params,
                                    size_t optReplayUriLength,
                                    const char* optReplayUri,
                                    bool doRecord,
                                    bool doReplay,
                                    bool doDebug,
                                    uint32_t snapInterval,
                                    uint32_t snapHistoryLength);
  static Isolate* New(const CreateParams& params);

  static Isolate* New();
  static Isolate* GetCurrent();
  typedef bool(*AbortOnUncaughtExceptionCallback)(Isolate*);
  void SetAbortOnUncaughtExceptionCallback(
    AbortOnUncaughtExceptionCallback callback);

  void SetHostImportModuleDynamicallyCallback(
      HostImportModuleDynamicallyCallback callback);
  void SetHostInitializeImportMetaObjectCallback(
      HostInitializeImportMetaObjectCallback callback);

  void Enter();
  void Exit();
  void Dispose();

  void GetHeapStatistics(HeapStatistics *heap_statistics);
  size_t NumberOfHeapSpaces();
  bool GetHeapSpaceStatistics(HeapSpaceStatistics* space_statistics,
                              size_t index);
  int64_t AdjustAmountOfExternalAllocatedMemory(int64_t change_in_bytes);
  void SetData(uint32_t slot, void* data);
  void* GetData(uint32_t slot);

  static bool RunSingleStepOfReverseMoveLoop(v8::Isolate* isolate,
                                             uint64_t* moveMode,
                                             int64_t* nextEventTime);

  static uint32_t GetNumberOfDataSlots();
  bool InContext();
  Local<Context> GetCurrentContext();
  void SetPromiseHook(PromiseHook hook);
  void SetPromiseRejectCallback(PromiseRejectCallback callback);
  void RunMicrotasks();
  void EnqueueMicrotask(MicrotaskCallback microtask, void* data = nullptr);
  void SetMicrotasksPolicy(MicrotasksPolicy policy);
  void SetAutorunMicrotasks(bool autorun);
  void SetFatalErrorHandler(FatalErrorCallback that);
  void SetJitCodeEventHandler(
    JitCodeEventOptions options, JitCodeEventHandler event_handler);
  bool AddMessageListener(
    MessageCallback that, Handle<Value> data = Handle<Value>());
  void RemoveMessageListeners(MessageCallback that);
  Local<Value> ThrowException(Local<Value> exception);
  HeapProfiler* GetHeapProfiler();
  CpuProfiler* GetCpuProfiler();

  typedef void (*GCPrologueCallback)(
    Isolate* isolate, GCType type, GCCallbackFlags flags);
  typedef void (*GCEpilogueCallback)(
    Isolate* isolate, GCType type, GCCallbackFlags flags);
  typedef void(*GCCallback)(Isolate* isolate, GCType type,
                            GCCallbackFlags flags);
  typedef void (*GCCallbackWithData)(Isolate* isolate, GCType type,
                            GCCallbackFlags flags, void* data);

  void AddGCPrologueCallback(
    GCCallbackWithData callback, void* data = nullptr,
    GCType gc_type_filter = kGCTypeAll);
  void AddGCPrologueCallback(
    GCCallback callback, GCType gc_type_filter = kGCTypeAll);
  void RemoveGCPrologueCallback(
    GCCallbackWithData callback, void* data = nullptr);
  void RemoveGCPrologueCallback(GCCallback callback);
  void AddGCEpilogueCallback(
    GCCallbackWithData callback, void* data = nullptr,
    GCType gc_type_filter = kGCTypeAll);
  void AddGCEpilogueCallback(
    GCCallback callback, GCType gc_type_filter = kGCTypeAll);
  void RemoveGCEpilogueCallback(
    GCCallbackWithData callback, void* data = nullptr);
  void RemoveGCEpilogueCallback(GCCallback callback);

  void CancelTerminateExecution();
  void RequestInterrupt(InterruptCallback callback, void* data);
  void TerminateExecution();
  void RequestGarbageCollectionForTesting(GarbageCollectionType type);

  void SetCounterFunction(CounterLookupCallback);
  void SetCreateHistogramFunction(CreateHistogramCallback);
  void SetAddHistogramSampleFunction(AddHistogramSampleCallback);

  bool IdleNotificationDeadline(double deadline_in_seconds);
  V8_DEPRECATE_SOON("use IdleNotificationDeadline()",
                    bool IdleNotification(int idle_time_in_ms));

  void LowMemoryNotification();
  int ContextDisposedNotification();
  void SetCaptureStackTraceForUncaughtExceptions(
      bool capture, int frame_limit = 10,
      StackTrace::StackTraceOptions options = StackTrace::kOverview);
};

class V8_EXPORT JitCodeEvent {
 public:
  enum EventType {
    CODE_ADDED,
    CODE_MOVED,
    CODE_REMOVED,
  };

  EventType type;
  void * code_start;
  size_t code_len;
  union {
    struct {
      const char* str;
      size_t len;
    } name;
    void* new_code_start;
  };
};

class V8_EXPORT StartupData {
};

class V8_EXPORT V8 {
 public:
  static bool IsDead();
  static void SetFlagsFromString(const char* str, int length);
  static void SetFlagsFromCommandLine(
    int *argc, char **argv, bool remove_flags);
  static const char *GetVersion();
  static bool Initialize();
  static void SetEntropySource(EntropySource source);
  static void TerminateExecution(Isolate* isolate);
  static bool IsExeuctionDisabled(Isolate* isolate = nullptr);
  static void CancelTerminateExecution(Isolate* isolate);
  static bool Dispose();
  static void InitializePlatform(Platform* platform) {}
  static void FromJustIsNothing();
  static void ToLocalEmpty();
  static void ShutdownPlatform() {}
};

template <class T>
class Maybe {
 public:
  bool IsNothing() const { return !has_value; }
  bool IsJust() const { return has_value; }

  T ToChecked() const { return FromJust(); }

  bool To(T* out) const {
    if (V8_LIKELY(IsJust())) *out = value;
    return IsJust();
  }

  // Will crash if the Maybe<> is nothing.
  T FromJust() const {
    if (!IsJust()) {
      V8::FromJustIsNothing();
    }
    return value;
  }

  T FromMaybe(const T& default_value) const {
    return has_value ? value : default_value;
  }

  bool operator==(const Maybe& other) const {
    return (IsJust() == other.IsJust()) &&
      (!IsJust() || FromJust() == other.FromJust());
  }

  bool operator!=(const Maybe& other) const {
    return !operator==(other);
  }

 private:
  Maybe() : has_value(false) {}
  explicit Maybe(const T& t) : has_value(true), value(t) {}

  bool has_value;
  T value;

  template <class U>
  friend Maybe<U> Nothing();
  template <class U>
  friend Maybe<U> Just(const U& u);
};

template <class T>
inline Maybe<T> Nothing() {
  return Maybe<T>();
}


template <class T>
inline Maybe<T> Just(const T& t) {
  return Maybe<T>(t);
}

class V8_EXPORT TryCatch {
 public:
  TryCatch(Isolate* isolate = nullptr);
  ~TryCatch();

  bool HasCaught() const;
  bool CanContinue() const;
  bool HasTerminated() const;
  Handle<Value> ReThrow();
  Local<Value> Exception() const;

  V8_DEPRECATE_SOON("Use maybe version.", Local<Value> StackTrace()) const;
  V8_WARN_UNUSED_RESULT MaybeLocal<Value> StackTrace(
    Local<Context> context) const;

  Local<v8::Message> Message() const;
  void Reset();
  void SetVerbose(bool value);
  bool IsVerbose() const;
  void SetCaptureMessage(bool value);

 private:
  friend class Function;

  void SetNonUser() { user = false; }
  void GetAndClearException();
  void CheckReportExternalException();
  JsValueRef EnsureException() const;


  JsValueRef metadata;
  TryCatch* prev;
  bool rethrow;
  bool user;
  bool verbose;
};

class V8_EXPORT ExtensionConfiguration {
};

class V8_EXPORT Context {
 public:
  class V8_EXPORT Scope {
   private:
    Scope * previous;
    void * context;
   public:
    Scope(Handle<Context> context);
    ~Scope();
  };

  Local<Object> Global();

  static Local<Context> New(
    Isolate* isolate,
    bool useGlobalTTState = false,
    ExtensionConfiguration* extensions = nullptr,
    Handle<ObjectTemplate> global_template = Handle<ObjectTemplate>(),
    Handle<Value> global_object = Handle<Value>());
  static Local<Context> New(
    Isolate* isolate,
    ExtensionConfiguration* extensions,
    Handle<ObjectTemplate> global_template = Handle<ObjectTemplate>(),
    Handle<Value> global_object = Handle<Value>()) {
    return New(isolate, false, extensions, global_template, global_object);
  }
  static Local<Context> GetCurrent();

  Isolate* GetIsolate();

  void Enter();
  void Exit();

  enum EmbedderDataFields { kDebugIdIndex = 0 };
  void* GetAlignedPointerFromEmbedderData(int index);
  void SetAlignedPointerInEmbedderData(int index, void* value);
  void SetEmbedderData(int index, Local<Value> value);
  Local<Value> GetEmbedderData(int index);
  void SetSecurityToken(Handle<Value> token);
  Handle<Value> GetSecurityToken();
};

class V8_EXPORT Locker {
  // Don't need to implement this for Chakra
 public:
  explicit Locker(Isolate* isolate) {}
};

class V8_EXPORT Module {
 public:
  /**
   * The different states a module can be in.
   */
  enum Status {
    kUninstantiated,
    kInstantiating,
    kInstantiated,
    kEvaluating,
    kEvaluated,
    kErrored
  };

  /**
   * Returns the module's current status.
   */
  Status GetStatus() const;

  /**
   * For a module in kErrored status, this returns the corresponding exception.
   */
  Local<Value> GetException() const;

  /**
   * Returns the number of modules requested by this module.
   */
  int GetModuleRequestsLength() const;

  /**
   * Returns the ith module specifier in this module.
   * i must be < GetModuleRequestsLength() and >= 0.
   */
  Local<String> GetModuleRequest(int i) const;

  /**
   * Returns the identity hash for this object.
   */
  int GetIdentityHash() const;

  typedef MaybeLocal<Module> (*ResolveCallback)(Local<Context> context,
                                                Local<String> specifier,
                                                Local<Module> referrer);

  /**
   * ModuleDeclarationInstantiation
   *
   * Returns false if an exception occurred during instantiation. (In the case
   * where the callback throws an exception, that exception is propagated.)
   */
  V8_WARN_UNUSED_RESULT bool Instantiate(Local<Context> context,
                                         ResolveCallback callback);

  Maybe<bool> InstantiateModule(Local<Context> context,
                                ResolveCallback callback);
  /**
   * ModuleEvaluation
   *
   * Returns the completion value.
   */
  V8_WARN_UNUSED_RESULT MaybeLocal<Value> Evaluate(Local<Context> context);

  /**
   * Returns the namespace object of this module.
   * The module's status must be kEvaluated.
   */
  Local<Value> GetModuleNamespace();
};



//
// Local<T> members
//

template <class T>
Local<T> Local<T>::New(T* that) {
  if (!HandleScope::GetCurrent()->AddLocal(that)) {
    return Local<T>();
  }
  return Local<T>(that);
}

// Context are not javascript values, so we need to specialize them
template <>
V8_INLINE Local<Context> Local<Context>::New(Context* that) {
  if (!HandleScope::GetCurrent()->AddLocalContext(that)) {
    return Local<Context>();
  }
  return Local<Context>(that);
}

template <class T>
Local<T> Local<T>::New(Isolate* isolate, Local<T> that) {
  return New(isolate, *that);
}

template <class T>
Local<T> Local<T>::New(Isolate* isolate, const PersistentBase<T>& that) {
  return New(isolate, that.val_);
}

template <class T>
Local<T> MaybeLocal<T>::ToLocalChecked() {
  if (V8_UNLIKELY(val_ == nullptr)) V8::ToLocalEmpty();
  return Local<T>(val_);
}

//
// Persistent<T> members
//

template <class T>
T* PersistentBase<T>::New(Isolate* isolate, T* that) {
  if (that) {
    JsAddRef(static_cast<JsRef>(that), nullptr);
  }
  return that;
}

template <class T, class M>
template <class S, class M2>
void Persistent<T, M>::Copy(const Persistent<S, M2>& that) {
  TYPE_CHECK(T, S);
  this->Reset();
  if (that.IsEmpty()) return;

  this->val_ = that.val_;
  this->_weakWrapper = that._weakWrapper;
  if (this->val_ && !this->IsWeak()) {
    JsAddRef(this->val_, nullptr);
  }

  M::Copy(that, this);
}

template <class T>
bool PersistentBase<T>::IsNearDeath() const {
  return true;
}

template <class T>
bool PersistentBase<T>::IsWeak() const {
  return _weakWrapper != nullptr;
}

template <class T>
void PersistentBase<T>::Reset() {
  if (this->IsEmpty() || V8::IsDead()) return;

  if (IsWeak()) {
    if (_weakWrapper) {
      chakrashim::ClearObjectWeakReferenceCallback(val_, /*revive*/false);
      delete _weakWrapper;
      _weakWrapper = nullptr;
    }
  } else {
    JsRelease(val_, nullptr);
  }

  val_ = nullptr;
}

template <class T>
template <class S>
void PersistentBase<T>::Reset(Isolate* isolate, const Handle<S>& other) {
  TYPE_CHECK(T, S);
  Reset();
  if (other.IsEmpty()) return;
  this->val_ = New(isolate, other.val_);
}

template <class T>
template <class S>
void PersistentBase<T>::Reset(Isolate* isolate,
                              const PersistentBase<S>& other) {
  TYPE_CHECK(T, S);
  Reset();
  if (other.IsEmpty()) return;
  this->val_ = New(isolate, other.val_);
}

template <class T>
template <typename P, typename Callback>
void PersistentBase<T>::SetWeakCommon(P* parameter, Callback callback) {
  if (this->IsEmpty()) return;

  bool wasStrong = !IsWeak();
  chakrashim::SetObjectWeakReferenceCallback(val_, callback, parameter,
                                             &_weakWrapper);
  if (wasStrong) {
    JsRelease(val_, nullptr);
  }
}

template <class T>
template <typename P>
void PersistentBase<T>::SetWeak(
    P* parameter,
    typename WeakCallbackData<T, P>::Callback callback) {
  typedef typename WeakCallbackData<Value, void>::Callback Callback;
  SetWeakCommon(parameter, reinterpret_cast<Callback>(callback));
}

template <class T>
template <typename P>
void PersistentBase<T>::SetWeak(P* parameter,
                                typename WeakCallbackInfo<P>::Callback callback,
                                WeakCallbackType type) {
  typedef typename WeakCallbackInfo<void>::Callback Callback;
  SetWeakCommon(parameter, reinterpret_cast<Callback>(callback));
}

template <class T>
template <typename P>
P* PersistentBase<T>::ClearWeak() {
  if (!IsWeak()) return nullptr;

  P* parameters = reinterpret_cast<P*>(_weakWrapper->parameters);
  if (_weakWrapper) {
    chakrashim::ClearObjectWeakReferenceCallback(val_, /*revive*/true);
    delete _weakWrapper;
    _weakWrapper = nullptr;
  }

  JsAddRef(val_, nullptr);
  return parameters;
}

template <class T>
void PersistentBase<T>::MarkIndependent() {
}

template <class T>
void PersistentBase<T>::SetWrapperClassId(uint16_t class_id) {
}


//
// HandleScope template members
//

template <class T>
Local<T> HandleScope::Close(Handle<T> value) {
  if (_prev == nullptr || !_prev->AddLocal(*value)) {
    return Local<T>();
  }

  return Local<T>(*value);
}

// Context are not javascript values, so we need to specialize them
template <>
inline Local<Context> HandleScope::Close(Handle<Context> value) {
  if (_prev == nullptr || !_prev->AddLocalContext(*value)) {
    return Local<Context>();
  }

  return Local<Context>(*value);
}

template<typename T>
void ReturnValue<T>::SetNull() {
  Set(Null(nullptr));
}

template<typename T>
void ReturnValue<T>::SetUndefined() {
  Set(Undefined(nullptr));
}

template<typename T>
void ReturnValue<T>::SetEmptyString() {
  Set(String::Empty(nullptr));
}

template<typename T>
Isolate* ReturnValue<T>::GetIsolate() {
  return Isolate::GetCurrent();
}

template<typename T>
Local<Value> FunctionCallbackInfo<T>::operator[](int i) const {
  return (i >= 0 && i < _length) ?
    _args[i] : Undefined(nullptr).As<Value>();
}

template<typename T>
Isolate* FunctionCallbackInfo<T>::GetIsolate() const {
  return Isolate::GetCurrent();
}

template<typename T>
Isolate* PropertyCallbackInfo<T>::GetIsolate() const {
  return Isolate::GetCurrent();
}

}  // namespace v8

#endif  // DEPS_CHAKRASHIM_INCLUDE_V8_H_
