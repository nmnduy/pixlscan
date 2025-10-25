# Coding guidelines

* Prefer composition over inheritance.
* Use templates or concepts for polymorphism when possible.
* Use `std::variant` or `std::visit` instead of class hierarchies for type alternatives.
* Avoid smart pointer cycles (especially with `std::shared_ptr`).
* Prefer smart pointers (`std::unique_ptr`, `std::shared_ptr`) over raw pointers.
* Avoid manual `new` and `delete` unless absolutely necessary.
* Use RAII (Resource Acquisition Is Initialization) for resource management.
* Avoid naked resource handles; wrap them in classes.
* Use `std::vector` or `std::array` instead of dynamic arrays.
* Use references (`&`) instead of pointers when ownership is not needed.
* Avoid returning raw pointers to internal data.
* Do not use dangling pointers or references to destroyed objects.
* Initialize all variables upon declaration.
* Use `std::optional` instead of nullable pointers when possible.
* Avoid `reinterpret_cast` unless absolutely necessary.
* Use `std::move` only when you understand ownership semantics.
* Never access memory after freeing or moving it.
* Use `std::span` for safe view over contiguous data.
* Enable compiler sanitizers (`-fsanitize=address,undefined`).
* Avoid global mutable state.
* Make destructors virtual in polymorphic base classes.
* Prefer `std::make_unique` / `std::make_shared` for allocation.
* Use `const` and `constexpr` aggressively to prevent unintended modifications.
* Review ownership semantics in every API (who frees what).
