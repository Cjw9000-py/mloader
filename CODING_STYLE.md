# CODING_STYLE.md

A concise, opinionated C++ style guide meant to be reusable across projects and libraries (including but not limited to MTL).

Goals:

* **Consistency** – one coherent “mini-language” on top of C++.
* **Clarity** – short names, minimal ceremony.
* **Contracts** – encode intent with attributes/macros, not comments.
* **Pragmatism** – safety where it matters, performance where it counts.

---

## Quick Reference (TL;DR)

* **Namespaces:** lowercase, single word (`core`, `math`, `memory`).
* **Types:**

    * Primitive/utility-like → lowercase one word (`u32`, `str`, `vec<T>`, `buffer`, `handle`).
    * Complex/descriptive → PascalCase (`SceneGraph`, `RenderPipeline`).
* **Functions:** lowercase one word where possible; allow `_` only in canonical patterns: `to_string`, `from_string`, `is_ready`, `get_value`/`set_value`. No camelCase.
* **Members:** public → lowercase; non-public → `m_` prefix + lowercase.
* **Class layout:** use `struct`; public first, protected last; avoid `final`; use `private` only for fragile representation/invariants you may change.
* **Contracts:**

    * `prop` for simple accessors (`[[nodiscard]]` + inline),
    * `cnex` = `const noexcept`, `nex` = `noexcept`,
    * assert invariants (`fassert`, `xassert`),
    * `unreachable()` in impossible paths.
* **Containers/Aliases:** short aliases (`vec`, `umap`, `uset`, `opt`, `sptr`, `uptr`, etc.) keep naming uniform.

---

## 1) Naming

### Namespaces

* Lowercase single words only. No underscores, no CamelCase.

  ```cpp
  namespace core { namespace math { /* ... */ } }
  ```

### Types

* **Primitive-like / frequently used:** one lowercase word (`u32`, `str`, `vec<T>`, `buffer`, `handle`).
* **Complex / multi-word concepts:** PascalCase (`QuadraticCurve`, `SceneGraph`).
* Do **not** use snake_case for type names.

### Functions

* Prefer a single lowercase verb/noun: `encode`, `normalize`, `reserve`.
* Allow `_` **only** for standard idioms with semantic prefixes:

    * Conversions: `to_string`, `from_string`.
    * Boolean state: `is_ready` (avoid camelCase like `isReady`).
    * Accessors when paired: `get_value` / `set_value` (don’t use `get_` unless there’s a matching `set_`).
* Avoid arbitrary glue underscores like `some_util` and avoid camelCase altogether.

### Members

* Public fields: lowercase one word (rare; prefer functions).
* Non-public: `m_` prefix, lowercase one word: `m_size`, `m_data`.

### Macros/Attributes (keyword-like)

* Use terse, mnemonic macros that read like language keywords.

  ```cpp
  #define ctor  inline explicit
  #define dtor
  #define virt  virtual
  #define finline __attribute__((always_inline))
  #define ninline __attribute__((noinline))
  #define noret __attribute__((noreturn))
  #define hot   __attribute__((hot))
  #define unused __attribute__((unused))
  #define use   [[nodiscard]]
  #define prop  use finline
  #define cnex  const noexcept
  #define nex   noexcept
  ```

---

## 2) Class & Inheritance Discipline

* Use `struct` for everything (classes and PODs). Public API first; protected implementation details last.
* **Default stance:** enable extension, avoid `final`.
* **When to use `private`:** if a member is part of the *representation* or *invariants* and you want freedom to refactor without breaking subclasses. If it’s `protected`, you are implicitly promising stability to subclassers.
* Prefer the **NVI pattern** (Non-Virtual Interface): public non-virtual method that calls protected virtual hook(s) to enforce sequencing and invariants.

  ```cpp
  struct Drawable {
      void draw() { prepare(); on_draw(); cleanup(); }
  protected:
      virt void on_draw() = 0;
      void prepare();
      void cleanup();
  };
  ```

---

## 3) Exceptions, Errors, and Invariants

* Use `fassert` (always-on) / `xassert` (debug-only) for internal invariants.
* Throw exceptions when rejecting invalid input is cheap and correctness-critical.
* Use `unreachable()` in logically impossible paths (asserts in debug, optimizer hints in release).
* Performance-critical hot paths may skip checks (`operator[]` on a trusted range).

---

## 4) Contracts via Attributes

* Accessors/queries return by value/reference and **must** be `cnex` and `[[nodiscard]]` via `prop`.
* Non-throwing mutators should be `nex`.
* Missing `noexcept`/`[[nodiscard]]` on eligible methods is a **style violation**.

Examples:

```cpp
prop usize size() cnex { return m_size; }
prop T* data() { return m_data; }
prop const T* data() cnex { return m_data; }
```

---

## 5) Aliases & STL Interop

Keep naming uniform and visually lightweight.

* `vec<T>`  → `std::vector<T>`
* `umap<K,V>` → `std::unordered_map<K,V>`
* `uset<T>` → `std::unordered_set<T>`
* `opt<T>` → `std::optional<T>`
* `sptr<T>`/`uptr<T>` → smart pointers
* `make_sptr<T>(...)` / `make_uptr<T>(...)` → construction helpers
* Strings and paths live as `str`, `wstr`, `cstr`, with `fs` for `std::filesystem`.

Primitive aliases:

* Signed: `i8, i16, i32, i64`
* Unsigned: `u8, u16, u32, u64`
* Floats: `f32, f64, f128`
* Sizes: `usize, isize`

---

## 6) Examples

### Light, non-owning buffer view

**Design intent:** view over contiguous memory; fast iteration; checked slicing; raw-pointer iterators.

```cpp
template<typename T>
struct LightBuffer {
    static_assert(std::is_trivially_copyable_v<T>, "LightBuffer requires a trivially copyable type");

    ctor LightBuffer() = default;
    ctor LightBuffer(T* ptr, usize n) : m_data(ptr), m_size(n) {}

    template<typename VT>
    ctor LightBuffer(VT& data) : LightBuffer((T*)&data, sizeof data) {}

    // Accessors: cnex via prop
    prop usize size() cnex { return m_size; }
    prop T* data() { return m_data; }
    prop const T* data() cnex { return m_data; }

    // Raw pointer iteration
    prop T* begin() { return m_data; }
    prop T* end()   { return m_data + m_size; }
    prop const T* begin() cnex { return m_data; }
    prop const T* end()   cnex { return m_data + m_size; }

    // Indexing: intentionally unchecked for hot path
    use T& operator[](usize i) { return m_data[i]; }
    use const T& operator[](usize i) const { return m_data[i]; }

    // Slicing: checked, loud failure
    use LightBuffer<T> view(usize start, usize end) {
        if (start > end || end > m_size) throw OutOfRangeError("LightBuffer::view range is invalid");
        return LightBuffer<T>(m_data + start, end - start);
    }
    use LightBuffer<const T> view(usize start, usize end) const {
        if (start > end || end > m_size) throw OutOfRangeError("LightBuffer::view range is invalid");
        return LightBuffer<const T>(m_data + start, end - start);
    }

    use LightBuffer<T> view(usize start) { return view(start, m_size); }
    use LightBuffer<const T> view(usize start) const { return view(start, m_size); }

private: // representation & invariants: keep refactor freedom
    T* m_data = nullptr;
    usize m_size = 0;
};
```

### NVI pattern for controlled extension

```cpp
struct Processor {
    void run() { setup(); on_run(); teardown(); }
protected:
    virt void on_run() = 0;
    void setup();
    void teardown();
};
```

---

## 7) Practical Checklist

* **Naming:**

    * Namespace = lowercase word.
    * Core types lowercase; complex types PascalCase.
    * Functions lowercase; allow `_` only for `to_/from_`, `is_`, and paired `get_/set_`.
* **Members:**

    * Non-public use `m_` prefix.
* **Methods:**

    * Queries/accessors = `prop ... cnex`.
    * Non-throwing mutators = `nex`.
    * Throw only on clear invalid input; assert invariants internally.
* **Inheritance:**

    * Use NVI. Expose explicit hooks.
    * Use `private` for representation/invariants. Avoid `final`.
* **Performance:**

    * Hot paths may skip checks; slow paths must be safe.
* **Impossible paths:**

    * Call `unreachable()`.

---

## 8) Rationale (Short)

This style deliberately reshapes C++ into a compact, uniform dialect:

* Lowercase primitives and aliases read like keywords.
* PascalCase signals higher-order abstractions.
* Attributes/macros encode API contracts (nodiscard, noexcept) so the compiler enforces discipline.
* Inheritance is opt-in through explicit hooks to prevent fragile bases while still enabling extension.

Keep the code lean, sharp, and predictable.
