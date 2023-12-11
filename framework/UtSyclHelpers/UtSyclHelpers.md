# UtSyclHelpers
### SYCL-related helper functions for UTs. Don't use outside UTs.
---

## copyVariableFromBufferToHost

Copies value from a single variable buffer `from` to a variable referenced by `to` in host code.

```cpp
template<typename Type, typename BufferType>
void copyVariableFromBufferToHost(BufferType& from, Type& to);
```
